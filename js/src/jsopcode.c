/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set sw=4 ts=8 et tw=78:
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/*
 * JS bytecode descriptors, disassemblers, and decompilers.
 */
#include "jsstddef.h"
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jstypes.h"
#include "jsarena.h" /* Added by JSIFY */
#include "jsutil.h" /* Added by JSIFY */
#include "jsdtoa.h"
#include "jsprf.h"
#include "jsapi.h"
#include "jsarray.h"
#include "jsatom.h"
#include "jscntxt.h"
#include "jsconfig.h"
#include "jsdbgapi.h"
#include "jsemit.h"
#include "jsfun.h"
#include "jslock.h"
#include "jsobj.h"
#include "jsopcode.h"
#include "jsregexp.h"
#include "jsscan.h"
#include "jsscope.h"
#include "jsscript.h"
#include "jsstr.h"

static const char js_incop_strs[][3] = {"++", "--"};

/* Pollute the namespace locally for MSVC Win16, but not for WatCom.  */
#ifdef __WINDOWS_386__
    #ifdef FAR
        #undef FAR
    #endif
#else  /* !__WINDOWS_386__ */
#ifndef FAR
#define FAR
#endif
#endif /* !__WINDOWS_386__ */

const JSCodeSpec FAR js_CodeSpec[] = {
#define OPDEF(op,val,name,token,length,nuses,ndefs,prec,format) \
    {name,token,length,nuses,ndefs,prec,format},
#include "jsopcode.tbl"
#undef OPDEF
};

uintN js_NumCodeSpecs = sizeof (js_CodeSpec) / sizeof js_CodeSpec[0];

/************************************************************************/

static ptrdiff_t
GetJumpOffset(jsbytecode *pc, jsbytecode *pc2)
{
    uint32 type;

    type = (js_CodeSpec[*pc].format & JOF_TYPEMASK);
    if (JOF_TYPE_IS_EXTENDED_JUMP(type))
        return GET_JUMPX_OFFSET(pc2);
    return GET_JUMP_OFFSET(pc2);
}

#ifdef DEBUG

JS_FRIEND_API(JSBool)
js_Disassemble(JSContext *cx, JSScript *script, JSBool lines, FILE *fp)
{
    jsbytecode *pc, *end;
    uintN len;

    pc = script->code;
    end = pc + script->length;
    while (pc < end) {
        if (pc == script->main)
            fputs("main:\n", fp);
        len = js_Disassemble1(cx, script, pc,
                              PTRDIFF(pc, script->code, jsbytecode),
                              lines, fp);
        if (!len)
            return JS_FALSE;
        pc += len;
    }
    return JS_TRUE;
}

const char *
ToDisassemblySource(JSContext *cx, jsval v)
{
    JSObject *obj;
    JSScopeProperty *sprop;
    char *source;
    const char *bytes;
    JSString *str;

    if (!JSVAL_IS_PRIMITIVE(v)) {
        obj = JSVAL_TO_OBJECT(v);
        if (OBJ_GET_CLASS(cx, obj) == &js_BlockClass) {
            source = JS_sprintf_append(NULL, "depth %d {",
                                       OBJ_BLOCK_DEPTH(cx, obj));
            for (sprop = OBJ_SCOPE(obj)->lastProp; sprop;
                 sprop = sprop->parent) {
                bytes = js_AtomToPrintableString(cx, JSID_TO_ATOM(sprop->id));
                if (!bytes)
                    return NULL;
                source = JS_sprintf_append(source, "%s: %d%s",
                                           bytes, sprop->shortid,
                                           sprop->parent ? ", " : "");
            }
            source = JS_sprintf_append(source, "}");
            if (!source)
                return NULL;
            str = JS_NewString(cx, source, strlen(source));
            if (!str)
                return NULL;
            return JS_GetStringBytes(str);
        }
    }
    return js_ValueToPrintableSource(cx, v);
}

JS_FRIEND_API(uintN)
js_Disassemble1(JSContext *cx, JSScript *script, jsbytecode *pc, uintN loc,
                JSBool lines, FILE *fp)
{
    JSOp op;
    const JSCodeSpec *cs;
    ptrdiff_t len, off, jmplen;
    uint32 type;
    JSAtom *atom;
    const char *bytes;

    op = (JSOp)*pc;
    if (op >= JSOP_LIMIT) {
        char numBuf1[12], numBuf2[12];
        JS_snprintf(numBuf1, sizeof numBuf1, "%d", op);
        JS_snprintf(numBuf2, sizeof numBuf2, "%d", JSOP_LIMIT);
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL,
                             JSMSG_BYTECODE_TOO_BIG, numBuf1, numBuf2);
        return 0;
    }
    cs = &js_CodeSpec[op];
    len = (ptrdiff_t) cs->length;
    fprintf(fp, "%05u:", loc);
    if (lines)
        fprintf(fp, "%4u", JS_PCToLineNumber(cx, script, pc));
    fprintf(fp, "  %s", cs->name);
    type = cs->format & JOF_TYPEMASK;
    switch (type) {
      case JOF_BYTE:
        if (op == JSOP_TRAP) {
            op = JS_GetTrapOpcode(cx, script, pc);
            if (op == JSOP_LIMIT)
                return 0;
            len = (ptrdiff_t) js_CodeSpec[op].length;
        }
        break;

      case JOF_JUMP:
      case JOF_JUMPX:
        off = GetJumpOffset(pc, pc);
        fprintf(fp, " %u (%d)", loc + off, off);
        break;

      case JOF_CONST:
        atom = GET_ATOM(cx, script, pc);
        bytes = ToDisassemblySource(cx, ATOM_KEY(atom));
        if (!bytes)
            return 0;
        fprintf(fp, " %s", bytes);
        break;

      case JOF_UINT16:
      case JOF_LOCAL:
        fprintf(fp, " %u", GET_UINT16(pc));
        break;

      case JOF_TABLESWITCH:
      case JOF_TABLESWITCHX:
      {
        jsbytecode *pc2;
        jsint i, low, high;

        jmplen = (type == JOF_TABLESWITCH) ? JUMP_OFFSET_LEN
                                           : JUMPX_OFFSET_LEN;
        pc2 = pc;
        off = GetJumpOffset(pc, pc2);
        pc2 += jmplen;
        low = GET_JUMP_OFFSET(pc2);
        pc2 += JUMP_OFFSET_LEN;
        high = GET_JUMP_OFFSET(pc2);
        pc2 += JUMP_OFFSET_LEN;
        fprintf(fp, " defaultOffset %d low %d high %d", off, low, high);
        for (i = low; i <= high; i++) {
            off = GetJumpOffset(pc, pc2);
            fprintf(fp, "\n\t%d: %d", i, off);
            pc2 += jmplen;
        }
        len = 1 + pc2 - pc;
        break;
      }

      case JOF_LOOKUPSWITCH:
      case JOF_LOOKUPSWITCHX:
      {
        jsbytecode *pc2;
        jsatomid npairs;

        jmplen = (type == JOF_LOOKUPSWITCH) ? JUMP_OFFSET_LEN
                                            : JUMPX_OFFSET_LEN;
        pc2 = pc;
        off = GetJumpOffset(pc, pc2);
        pc2 += jmplen;
        npairs = GET_ATOM_INDEX(pc2);
        pc2 += ATOM_INDEX_LEN;
        fprintf(fp, " offset %d npairs %u", off, (uintN) npairs);
        while (npairs) {
            atom = GET_ATOM(cx, script, pc2);
            pc2 += ATOM_INDEX_LEN;
            off = GetJumpOffset(pc, pc2);
            pc2 += jmplen;

            bytes = ToDisassemblySource(cx, ATOM_KEY(atom));
            if (!bytes)
                return 0;
            fprintf(fp, "\n\t%s: %d", bytes, off);
            npairs--;
        }
        len = 1 + pc2 - pc;
        break;
      }

      case JOF_QARG:
        fprintf(fp, " %u", GET_ARGNO(pc));
        break;

      case JOF_QVAR:
        fprintf(fp, " %u", GET_VARNO(pc));
        break;

      case JOF_INDEXCONST:
        fprintf(fp, " %u", GET_VARNO(pc));
        pc += VARNO_LEN;
        atom = GET_ATOM(cx, script, pc);
        bytes = ToDisassemblySource(cx, ATOM_KEY(atom));
        if (!bytes)
            return 0;
        fprintf(fp, " %s", bytes);
        break;

      case JOF_UINT24:
        if (op == JSOP_FINDNAME) {
            /* Special case to avoid a JOF_FINDNAME just for this op. */
            atom = js_GetAtom(cx, &script->atomMap, GET_UINT24(pc));
            bytes = ToDisassemblySource(cx, ATOM_KEY(atom));
            if (!bytes)
                return 0;
            fprintf(fp, " %s", bytes);
            break;
        }

        JS_ASSERT(op == JSOP_UINT24 || op == JSOP_LITERAL);
        fprintf(fp, " %u", GET_UINT24(pc));
        break;

      case JOF_LITOPX:
        atom = js_GetAtom(cx, &script->atomMap, GET_LITERAL_INDEX(pc));
        bytes = ToDisassemblySource(cx, ATOM_KEY(atom));
        if (!bytes)
            return 0;

        /*
         * Bytecode: JSOP_LITOPX <uint24> op [<varno> if JSOP_DEFLOCALFUN].
         * Advance pc to point at op.
         */
        pc += 1 + LITERAL_INDEX_LEN;
        op = *pc;
        cs = &js_CodeSpec[op];
        fprintf(fp, " %s op %s", bytes, cs->name);
        if ((cs->format & JOF_TYPEMASK) == JOF_INDEXCONST)
            fprintf(fp, " %u", GET_VARNO(pc));

        /*
         * Set len to advance pc to skip op and any other immediates (namely,
         * <varno> if JSOP_DEFLOCALFUN).
         */
        JS_ASSERT(cs->length > ATOM_INDEX_LEN);
        len = cs->length - ATOM_INDEX_LEN;
        break;

      default: {
        char numBuf[12];
        JS_snprintf(numBuf, sizeof numBuf, "%lx", (unsigned long) cs->format);
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL,
                             JSMSG_UNKNOWN_FORMAT, numBuf);
        return 0;
      }
    }
    fputs("\n", fp);
    return len;
}

#endif /* DEBUG */

/************************************************************************/

/*
 * Sprintf, but with unlimited and automatically allocated buffering.
 */
typedef struct Sprinter {
    JSContext       *context;       /* context executing the decompiler */
    JSArenaPool     *pool;          /* string allocation pool */
    char            *base;          /* base address of buffer in pool */
    size_t          size;           /* size of buffer allocated at base */
    ptrdiff_t       offset;         /* offset of next free char in buffer */
} Sprinter;

#define INIT_SPRINTER(cx, sp, ap, off) \
    ((sp)->context = cx, (sp)->pool = ap, (sp)->base = NULL, (sp)->size = 0,  \
     (sp)->offset = off)

#define OFF2STR(sp,off) ((sp)->base + (off))
#define STR2OFF(sp,str) ((str) - (sp)->base)
#define RETRACT(sp,str) ((sp)->offset = STR2OFF(sp, str))

static JSBool
SprintAlloc(Sprinter *sp, size_t nb)
{
    if (!sp->base) {
        JS_ARENA_ALLOCATE_CAST(sp->base, char *, sp->pool, nb);
    } else {
        JS_ARENA_GROW_CAST(sp->base, char *, sp->pool, sp->size, nb);
    }
    if (!sp->base) {
        JS_ReportOutOfMemory(sp->context);
        return JS_FALSE;
    }
    sp->size += nb;
    return JS_TRUE;
}

static ptrdiff_t
SprintPut(Sprinter *sp, const char *s, size_t len)
{
    ptrdiff_t nb, offset;
    char *bp;

    /* Allocate space for s, including the '\0' at the end. */
    nb = (sp->offset + len + 1) - sp->size;
    if (nb > 0 && !SprintAlloc(sp, nb))
        return -1;

    /* Advance offset and copy s into sp's buffer. */
    offset = sp->offset;
    sp->offset += len;
    bp = sp->base + offset;
    memmove(bp, s, len);
    bp[len] = 0;
    return offset;
}

static ptrdiff_t
SprintCString(Sprinter *sp, const char *s)
{
    return SprintPut(sp, s, strlen(s));
}

static ptrdiff_t
Sprint(Sprinter *sp, const char *format, ...)
{
    va_list ap;
    char *bp;
    ptrdiff_t offset;

    va_start(ap, format);
    bp = JS_vsmprintf(format, ap);      /* XXX vsaprintf */
    va_end(ap);
    if (!bp) {
        JS_ReportOutOfMemory(sp->context);
        return -1;
    }
    offset = SprintCString(sp, bp);
    free(bp);
    return offset;
}

const jschar js_EscapeMap[] = {
    '\b', 'b',
    '\f', 'f',
    '\n', 'n',
    '\r', 'r',
    '\t', 't',
    '\v', 'v',
    '"',  '"',
    '\'', '\'',
    '\\', '\\',
    0
};

#define QUOTE_IN_XML    0x10000

static char *
QuoteString(Sprinter *sp, JSString *str, uint32 quote)
{
    JSBool inXML, ok;
    jschar qc, c;
    ptrdiff_t off, len, nb;
    const jschar *s, *t, *u, *z;
    char *bp;

    /* Sample off first for later return value pointer computation. */
    inXML = (quote & QUOTE_IN_XML) != 0;
    qc = (jschar) quote;
    off = sp->offset;
    if (qc && Sprint(sp, "%c", (char)qc) < 0)
        return NULL;

    /* Loop control variables: z points at end of string sentinel. */
    s = JSSTRING_CHARS(str);
    z = s + JSSTRING_LENGTH(str);
    for (t = s; t < z; s = ++t) {
        /* Move t forward from s past un-quote-worthy characters. */
        c = *t;
        while (JS_ISPRINT(c) && c != qc && c != '\\' && !(c >> 8)) {
            c = *++t;
            if (t == z)
                break;
        }
        len = PTRDIFF(t, s, jschar);

        /* Allocate space for s, including the '\0' at the end. */
        nb = (sp->offset + len + 1) - sp->size;
        if (nb > 0 && !SprintAlloc(sp, nb))
            return NULL;

        /* Advance sp->offset and copy s into sp's buffer. */
        bp = sp->base + sp->offset;
        sp->offset += len;
        while (--len >= 0)
            *bp++ = (char) *s++;
        *bp = '\0';

        if (t == z)
            break;

        /* Use js_EscapeMap, \u, or \x only if necessary. */
        if ((u = js_strchr(js_EscapeMap, c)) != NULL) {
            ok = inXML
                 ? Sprint(sp, "%c", (char)c) >= 0
                 : Sprint(sp, "\\%c", (char)u[1]) >= 0;
        } else {
#ifdef JS_C_STRINGS_ARE_UTF8
            /* If this is a surrogate pair, make sure to print the pair. */
            if (c >= 0xD800 && c <= 0xDBFF) {
                jschar buffer[3];
                buffer[0] = c;
                buffer[1] = *++t;
                buffer[2] = 0;
                if (t == z) {
                    char numbuf[10];
                    JS_snprintf(numbuf, sizeof numbuf, "0x%x", c);
                    JS_ReportErrorFlagsAndNumber(cx, JSREPORT_ERROR,
                                                 js_GetErrorMessage, NULL,
                                                 JSMSG_BAD_SURROGATE_CHAR,
                                                 numbuf);
                    ok = JS_FALSE;
                    break;
                }
                ok = Sprint(sp, "%hs", buffer) >= 0;
            } else {
                /* Print as UTF-8 string. */
                ok = Sprint(sp, "%hc", c) >= 0;
            }
#else
            /* Use \uXXXX or \xXX  if the string can't be displayed as UTF-8. */
            ok = Sprint(sp, (c >> 8) ? "\\u%04X" : "\\x%02X", c) >= 0;
#endif
        }
        if (!ok)
            return NULL;
    }

    /* Sprint the closing quote and return the quoted string. */
    if (qc && Sprint(sp, "%c", (char)qc) < 0)
        return NULL;

    /*
     * If we haven't Sprint'd anything yet, Sprint an empty string so that
     * the OFF2STR below gives a valid result.
     */
    if (off == sp->offset && Sprint(sp, "") < 0)
        return NULL;
    return OFF2STR(sp, off);
}

JSString *
js_QuoteString(JSContext *cx, JSString *str, jschar quote)
{
    void *mark;
    Sprinter sprinter;
    char *bytes;
    JSString *escstr;

    mark = JS_ARENA_MARK(&cx->tempPool);
    INIT_SPRINTER(cx, &sprinter, &cx->tempPool, 0);
    bytes = QuoteString(&sprinter, str, quote);
    escstr = bytes ? JS_NewStringCopyZ(cx, bytes) : NULL;
    JS_ARENA_RELEASE(&cx->tempPool, mark);
    return escstr;
}

/************************************************************************/

#if JS_HAS_BLOCK_SCOPE
typedef enum JSBraceState {
    ALWAYS_BRACE,
    MAYBE_BRACE,
    DONT_BRACE
} JSBraceState;
#endif

struct JSPrinter {
    Sprinter        sprinter;       /* base class state */
    JSArenaPool     pool;           /* string allocation pool */
    uintN           indent;         /* indentation in spaces */
    JSPackedBool    pretty;         /* pretty-print: indent, use newlines */
    JSPackedBool    grouped;        /* in parenthesized expression context */
    JSScript        *script;        /* script being printed */
    JSScope         *scope;         /* script function scope */
#if JS_HAS_BLOCK_SCOPE
    JSBraceState    braceState;     /* remove braces around let declaration */
    ptrdiff_t       spaceOffset;    /* -1 or offset of space before maybe-{ */
#endif
};

/*
 * Hack another flag, a la JS_DONT_PRETTY_PRINT, into uintN indent parameters
 * to functions such as js_DecompileFunction and js_NewPrinter.  This time, as
 * opposed to JS_DONT_PRETTY_PRINT back in the dark ages, we can assume that a
 * uintN is at least 32 bits.
 */
#define JS_IN_GROUP_CONTEXT 0x10000

JSPrinter *
js_NewPrinter(JSContext *cx, const char *name, uintN indent, JSBool pretty)
{
    JSPrinter *jp;

    jp = (JSPrinter *) JS_malloc(cx, sizeof(JSPrinter));
    if (!jp)
        return NULL;
    INIT_SPRINTER(cx, &jp->sprinter, &jp->pool, 0);
    JS_InitArenaPool(&jp->pool, name, 256, 1);
    jp->indent = indent & ~JS_IN_GROUP_CONTEXT;
    jp->pretty = pretty;
    jp->grouped = (indent & JS_IN_GROUP_CONTEXT) != 0;
    jp->script = NULL;
    jp->scope = NULL;
#if JS_HAS_BLOCK_SCOPE
    jp->braceState = ALWAYS_BRACE;
    jp->spaceOffset = -1;
#endif
    return jp;
}

void
js_DestroyPrinter(JSPrinter *jp)
{
    JS_FinishArenaPool(&jp->pool);
    JS_free(jp->sprinter.context, jp);
}

JSString *
js_GetPrinterOutput(JSPrinter *jp)
{
    JSContext *cx;
    JSString *str;

    cx = jp->sprinter.context;
    if (!jp->sprinter.base)
        return cx->runtime->emptyString;
    str = JS_NewStringCopyZ(cx, jp->sprinter.base);
    if (!str)
        return NULL;
    JS_FreeArenaPool(&jp->pool);
    INIT_SPRINTER(cx, &jp->sprinter, &jp->pool, 0);
    return str;
}

#if !JS_HAS_BLOCK_SCOPE
# define SET_MAYBE_BRACE(jp)    jp
# define CLEAR_MAYBE_BRACE(jp)  jp
#else
# define SET_MAYBE_BRACE(jp)    ((jp)->braceState = MAYBE_BRACE, (jp))
# define CLEAR_MAYBE_BRACE(jp)  ((jp)->braceState = ALWAYS_BRACE, (jp))

static void
SetDontBrace(JSPrinter *jp)
{
    ptrdiff_t offset;

    /* When not pretty-printing, newline after brace is chopped. */
    JS_ASSERT(jp->spaceOffset < 0);
    offset = jp->sprinter.offset - (jp->pretty ? 3 : 2);

    /* The shortest case is "if (x) {". */
    JS_ASSERT(offset >= 6);
    JS_ASSERT(jp->sprinter.base[offset+0] == ' ');
    JS_ASSERT(jp->sprinter.base[offset+1] == '{');
    JS_ASSERT(!jp->pretty || jp->sprinter.base[offset+2] == '\n');

    jp->spaceOffset = offset;
    jp->braceState = DONT_BRACE;
}
#endif

int
js_printf(JSPrinter *jp, const char *format, ...)
{
    va_list ap;
    char *bp, *fp;
    int cc;

    if (*format == '\0')
        return 0;

    va_start(ap, format);

    /* If pretty-printing, expand magic tab into a run of jp->indent spaces. */
    if (*format == '\t') {
        format++;

#if JS_HAS_BLOCK_SCOPE
        if (*format == '}' && jp->braceState != ALWAYS_BRACE) {
            JSBraceState braceState;

            braceState = jp->braceState;
            jp->braceState = ALWAYS_BRACE;
            if (braceState == DONT_BRACE) {
                ptrdiff_t offset, delta, from;

                JS_ASSERT(format[1] == '\n' || format[1] == ' ');
                offset = jp->spaceOffset;
                JS_ASSERT(offset >= 6);

                /* Replace " {\n" at the end of jp->sprinter with "\n". */
                bp = jp->sprinter.base;
                JS_ASSERT(bp[offset+0] == ' ');
                JS_ASSERT(bp[offset+1] == '{');
                delta = 2;
                if (jp->pretty) {
                    /* If pretty, we don't have to worry about 'else'. */
                    JS_ASSERT(bp[offset+2] == '\n');
                } else if (bp[offset-1] != ')') {
                    /* Must keep ' ' to avoid 'dolet' or 'elselet'. */
                    ++offset;
                    delta = 1;
                }

                from = offset + delta;
                memmove(bp + offset, bp + from, jp->sprinter.offset - from);
                jp->sprinter.offset -= delta;
                jp->spaceOffset = -1;

                format += 2;
                if (*format == '\0')
                    return 0;
            }
        }
#endif

        if (jp->pretty && Sprint(&jp->sprinter, "%*s", jp->indent, "") < 0)
            return -1;
    }

    /* Suppress newlines (must be once per format, at the end) if not pretty. */
    fp = NULL;
    if (!jp->pretty && format[cc = strlen(format) - 1] == '\n') {
        fp = JS_strdup(jp->sprinter.context, format);
        if (!fp)
            return -1;
        fp[cc] = '\0';
        format = fp;
    }

    /* Allocate temp space, convert format, and put. */
    bp = JS_vsmprintf(format, ap);      /* XXX vsaprintf */
    if (fp) {
        JS_free(jp->sprinter.context, fp);
        format = NULL;
    }
    if (!bp) {
        JS_ReportOutOfMemory(jp->sprinter.context);
        return -1;
    }

    cc = strlen(bp);
    if (SprintPut(&jp->sprinter, bp, (size_t)cc) < 0)
        cc = -1;
    free(bp);

    va_end(ap);
    return cc;
}

JSBool
js_puts(JSPrinter *jp, const char *s)
{
    return SprintCString(&jp->sprinter, s) >= 0;
}

/************************************************************************/

typedef struct SprintStack {
    Sprinter    sprinter;       /* sprinter for postfix to infix buffering */
    ptrdiff_t   *offsets;       /* stack of postfix string offsets */
    jsbytecode  *opcodes;       /* parallel stack of JS opcodes */
    uintN       top;            /* top of stack index */
    uintN       inArrayInit;    /* array initialiser/comprehension level */
    JSPrinter   *printer;       /* permanent output goes here */
} SprintStack;

/* Gap between stacked strings to allow for insertion of parens and commas. */
#define PAREN_SLOP      (2 + 1)

/*
 * These pseudo-ops help js_DecompileValueGenerator decompile JSOP_SETNAME,
 * JSOP_SETPROP, and JSOP_SETELEM, respectively.  See the first assertion in
 * PushOff.
 */
#define JSOP_GETPROP2   254
#define JSOP_GETELEM2   255

static JSBool
PushOff(SprintStack *ss, ptrdiff_t off, JSOp op)
{
    uintN top;

#if JSOP_LIMIT > JSOP_GETPROP2
#error JSOP_LIMIT must be <= JSOP_GETPROP2
#endif
    if (!SprintAlloc(&ss->sprinter, PAREN_SLOP))
        return JS_FALSE;

    /* ss->top points to the next free slot; be paranoid about overflow. */
    top = ss->top;
    JS_ASSERT(top < ss->printer->script->depth);
    if (top >= ss->printer->script->depth) {
        JS_ReportOutOfMemory(ss->sprinter.context);
        return JS_FALSE;
    }

    /* The opcodes stack must contain real bytecodes that index js_CodeSpec. */
    ss->offsets[top] = off;
    ss->opcodes[top] = (op == JSOP_GETPROP2) ? JSOP_GETPROP
                     : (op == JSOP_GETELEM2) ? JSOP_GETELEM
                     : (jsbytecode) op;
    ss->top = ++top;
    ss->sprinter.offset += PAREN_SLOP;
    return JS_TRUE;
}

static ptrdiff_t
PopOff(SprintStack *ss, JSOp op)
{
    uintN top;
    const JSCodeSpec *cs, *topcs;
    ptrdiff_t off;

    /* ss->top points to the next free slot; be paranoid about underflow. */
    top = ss->top;
    JS_ASSERT(top != 0);
    if (top == 0)
        return 0;

    ss->top = --top;
    topcs = &js_CodeSpec[ss->opcodes[top]];
    cs = &js_CodeSpec[op];
    if (topcs->prec != 0 && topcs->prec < cs->prec) {
        ss->offsets[top] -= 2;
        ss->sprinter.offset = ss->offsets[top];
        off = Sprint(&ss->sprinter, "(%s)",
                     OFF2STR(&ss->sprinter, ss->sprinter.offset + 2));
    } else {
        off = ss->sprinter.offset = ss->offsets[top];
    }
    return off;
}

typedef struct TableEntry {
    jsval       key;
    ptrdiff_t   offset;
    JSAtom      *label;
    jsint       order;          /* source order for stable tableswitch sort */
} TableEntry;

static JSBool
CompareOffsets(void *arg, const void *v1, const void *v2, int *result)
{
    ptrdiff_t offset_diff;
    const TableEntry *te1 = (const TableEntry *) v1,
                     *te2 = (const TableEntry *) v2;

    offset_diff = te1->offset - te2->offset;
    *result = (offset_diff == 0 ? te1->order - te2->order
               : offset_diff < 0 ? -1
               : 1);
    return JS_TRUE;
}

static JSBool
Decompile(SprintStack *ss, jsbytecode *pc, intN nb);

static JSBool
DecompileSwitch(SprintStack *ss, TableEntry *table, uintN tableLength,
                jsbytecode *pc, ptrdiff_t switchLength,
                ptrdiff_t defaultOffset, JSBool isCondSwitch)
{
    JSContext *cx;
    JSPrinter *jp;
    ptrdiff_t off, off2, diff, caseExprOff;
    char *lval, *rval;
    uintN i;
    jsval key;
    JSString *str;

    cx = ss->sprinter.context;
    jp = ss->printer;

    /* JSOP_CONDSWITCH doesn't pop, unlike JSOP_{LOOKUP,TABLE}SWITCH. */
    off = isCondSwitch ? ss->offsets[ss->top-1] : PopOff(ss, JSOP_NOP);
    lval = OFF2STR(&ss->sprinter, off);

    js_printf(CLEAR_MAYBE_BRACE(jp), "\tswitch (%s) {\n", lval);

    if (tableLength) {
        diff = table[0].offset - defaultOffset;
        if (diff > 0) {
            jp->indent += 2;
            js_printf(jp, "\t%s:\n", js_default_str);
            jp->indent += 2;
            if (!Decompile(ss, pc + defaultOffset, diff))
                return JS_FALSE;
            jp->indent -= 4;
        }

        caseExprOff = isCondSwitch ? JSOP_CONDSWITCH_LENGTH : 0;

        for (i = 0; i < tableLength; i++) {
            off = table[i].offset;
            off2 = (i + 1 < tableLength) ? table[i + 1].offset : switchLength;

            key = table[i].key;
            if (isCondSwitch) {
                ptrdiff_t nextCaseExprOff;

                /*
                 * key encodes the JSOP_CASE bytecode's offset from switchtop.
                 * The next case expression follows immediately, unless we are
                 * at the last case.
                 */
                nextCaseExprOff = (ptrdiff_t)JSVAL_TO_INT(key);
                nextCaseExprOff += js_CodeSpec[pc[nextCaseExprOff]].length;
                jp->indent += 2;
                if (!Decompile(ss, pc + caseExprOff,
                               nextCaseExprOff - caseExprOff)) {
                    return JS_FALSE;
                }
                caseExprOff = nextCaseExprOff;

                /* Balance the stack as if this JSOP_CASE matched. */
                --ss->top;
            } else {
                /*
                 * key comes from an atom, not the decompiler, so we need to
                 * quote it if it's a string literal.  But if table[i].label
                 * is non-null, key was constant-propagated and label is the
                 * name of the const we should show as the case label.  We set
                 * key to undefined so this identifier is escaped, if required
                 * by non-ASCII characters, but not quoted, by QuoteString.
                 */
                if (table[i].label) {
                    str = ATOM_TO_STRING(table[i].label);
                    key = JSVAL_VOID;
                } else {
                    str = js_ValueToString(cx, key);
                    if (!str)
                        return JS_FALSE;
                }
                rval = QuoteString(&ss->sprinter, str,
                                   (jschar)(JSVAL_IS_STRING(key) ? '"' : 0));
                if (!rval)
                    return JS_FALSE;
                RETRACT(&ss->sprinter, rval);
                jp->indent += 2;
                js_printf(jp, "\tcase %s:\n", rval);
            }

            jp->indent += 2;
            if (off <= defaultOffset && defaultOffset < off2) {
                diff = defaultOffset - off;
                if (diff != 0) {
                    if (!Decompile(ss, pc + off, diff))
                        return JS_FALSE;
                    off = defaultOffset;
                }
                jp->indent -= 2;
                js_printf(jp, "\t%s:\n", js_default_str);
                jp->indent += 2;
            }
            if (!Decompile(ss, pc + off, off2 - off))
                return JS_FALSE;
            jp->indent -= 4;

            /* Re-balance as if last JSOP_CASE or JSOP_DEFAULT mismatched. */
            if (isCondSwitch)
                ++ss->top;
        }
    }

    if (defaultOffset == switchLength) {
        jp->indent += 2;
        js_printf(jp, "\t%s:;\n", js_default_str);
        jp->indent -= 2;
    }
    js_printf(jp, "\t}\n");

    /* By the end of a JSOP_CONDSWITCH, the discriminant has been popped. */
    if (isCondSwitch)
        --ss->top;
    return JS_TRUE;
}

static JSAtom *
GetSlotAtom(JSPrinter *jp, JSPropertyOp getter, uintN slot)
{
    JSScope *scope;
    JSScopeProperty *sprop;
    JSObject *obj, *proto;

    scope = jp->scope;
    while (scope) {
        for (sprop = SCOPE_LAST_PROP(scope); sprop; sprop = sprop->parent) {
            if (sprop->getter != getter)
                continue;
            JS_ASSERT(sprop->flags & SPROP_HAS_SHORTID);
            JS_ASSERT(JSID_IS_ATOM(sprop->id));
            if ((uintN) sprop->shortid == slot)
                return JSID_TO_ATOM(sprop->id);
        }
        obj = scope->object;
        if (!obj)
            break;
        proto = OBJ_GET_PROTO(jp->sprinter.context, obj);
        if (!proto)
            break;
        scope = OBJ_SCOPE(proto);
    }
    return NULL;
}

/*
 * NB: Indexed by SRC_DECL_* defines from jsemit.h.
 */
static const char * const var_prefix[] = {"var ", "const ", "let "};

static const char *
VarPrefix(jssrcnote *sn)
{
    if (sn && SN_TYPE(sn) == SRC_DECL) {
        ptrdiff_t type = js_GetSrcNoteOffset(sn, 0);
        if ((uintN)type <= SRC_DECL_LET)
            return var_prefix[type];
    }
    return "";
}

static JSBool
Decompile(SprintStack *ss, jsbytecode *pc, intN nb)
{
    JSContext *cx;
    JSPrinter *jp, *jp2;
    jsbytecode *endpc, *pc2, *done, *forelem_tail, *forelem_done;
    ptrdiff_t tail, todo, len, oplen, cond, next;
    JSOp op, lastop, saveop;
    const JSCodeSpec *cs;
    jssrcnote *sn, *sn2;
    const char *lval, *rval, *xval, *fmt;
    jsint i, argc;
    char **argv;
    jsatomid atomIndex;
    JSAtom *atom;
    JSObject *obj;
    JSFunction *fun;
    JSString *str;
    JSBool ok;
#if JS_HAS_XML_SUPPORT
    JSBool foreach, inXML, quoteAttr;
#else
#define inXML JS_FALSE
#endif
    jsval val;
    int stackDummy;

    static const char exception_cookie[] = "/*EXCEPTION*/";
    static const char retsub_pc_cookie[] = "/*RETSUB_PC*/";
    static const char iter_cookie[]      = "/*ITER*/";
    static const char forelem_cookie[]   = "/*FORELEM*/";
    static const char with_cookie[]      = "/*WITH*/";
    static const char dot_format[]       = "%s.%s";
    static const char index_format[]     = "%s[%s]";
    static const char predot_format[]    = "%s%s.%s";
    static const char postdot_format[]   = "%s.%s%s";
    static const char preindex_format[]  = "%s%s[%s]";
    static const char postindex_format[] = "%s[%s]%s";
    static const char ss_format[]        = "%s%s";

/*
 * Local macros
 */
#define DECOMPILE_CODE(pc,nb)   if (!Decompile(ss, pc, nb)) return JS_FALSE
#define POP_STR()               OFF2STR(&ss->sprinter, PopOff(ss, op))
#define LOCAL_ASSERT(expr)      JS_BEGIN_MACRO                                \
                                    JS_ASSERT(expr);                          \
                                    if (!(expr)) return JS_FALSE;             \
                                JS_END_MACRO

/*
 * Callers know that ATOM_IS_STRING(atom), and we leave it to the optimizer to
 * common ATOM_TO_STRING(atom) here and near the call sites.
 */
#define ATOM_IS_IDENTIFIER(atom) js_IsIdentifier(ATOM_TO_STRING(atom))

/*
 * Given an atom already fetched from jp->script's atom map, quote/escape its
 * string appropriately into rval, and select fmt from the quoted and unquoted
 * alternatives.
 */
#define GET_QUOTE_AND_FMT(qfmt, ufmt, rval)                                   \
    JS_BEGIN_MACRO                                                            \
        jschar quote_;                                                        \
        if (!ATOM_IS_IDENTIFIER(atom)) {                                      \
            quote_ = '\'';                                                    \
            fmt = qfmt;                                                       \
        } else {                                                              \
            quote_ = 0;                                                       \
            fmt = ufmt;                                                       \
        }                                                                     \
        rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), quote_);      \
        if (!rval)                                                            \
            return JS_FALSE;                                                  \
    JS_END_MACRO

/*
 * Get atom from jp->script's atom map, quote/escape its string appropriately
 * into rval, and select fmt from the quoted and unquoted alternatives.
 */
#define GET_ATOM_QUOTE_AND_FMT(qfmt, ufmt, rval)                              \
    JS_BEGIN_MACRO                                                            \
        atom = GET_ATOM(cx, jp->script, pc);                                  \
        GET_QUOTE_AND_FMT(qfmt, ufmt, rval);                                  \
    JS_END_MACRO

    cx = ss->sprinter.context;
    if (!JS_CHECK_STACK_SIZE(cx, stackDummy)) {
        JS_ReportErrorNumber(cx, js_GetErrorMessage, NULL, JSMSG_OVER_RECURSED);
        return JS_FALSE;
    }

    jp = ss->printer;
    endpc = pc + nb;
    forelem_tail = forelem_done = NULL;
    tail = -1;
    todo = -2;                  /* NB: different from Sprint() error return. */
    saveop = JSOP_NOP;
    sn = NULL;
    rval = NULL;
#if JS_HAS_XML_SUPPORT
    foreach = inXML = quoteAttr = JS_FALSE;
#endif

    while (pc < endpc) {
        /*
         * Move saveop to lastop so prefixed bytecodes can take special action
         * while sharing maximal code.  Set op and saveop to the new bytecode,
         * use op in POP_STR to trigger automatic parenthesization, but push
         * saveop at the bottom of the loop if this op pushes.  Thus op may be
         * set to nop or otherwise mutated to suppress auto-parens.
         */
        lastop = saveop;
        op = saveop = (JSOp) *pc;
        if (op >= JSOP_LIMIT) {
            switch (op) {
              case JSOP_GETPROP2:
                saveop = JSOP_GETPROP;
                break;
              case JSOP_GETELEM2:
                saveop = JSOP_GETELEM;
                break;
              default:;
            }
        }
        cs = &js_CodeSpec[saveop];
        len = oplen = cs->length;

        if (cs->token) {
            switch (cs->nuses) {
              case 2:
                sn = js_GetSrcNote(jp->script, pc);
                if (sn && SN_TYPE(sn) == SRC_ASSIGNOP) {
                    /*
                     * Avoid over-parenthesizing y in x op= y based on its
                     * expansion: x = x op y (replace y by z = w to see the
                     * problem).
                     */
                    op = pc[1];
                    JS_ASSERT(op != saveop);
                }
                rval = POP_STR();
                lval = POP_STR();
                if (op != saveop) {
                    /* Print only the right operand of the assignment-op. */
                    todo = SprintCString(&ss->sprinter, rval);
                    op = saveop;
                } else if (!inXML) {
                    todo = Sprint(&ss->sprinter, "%s %s %s",
                                  lval, cs->token, rval);
                } else {
                    /* In XML, just concatenate the two operands. */
                    JS_ASSERT(op == JSOP_ADD);
                    todo = Sprint(&ss->sprinter, ss_format, lval, rval);
                }
                break;

              case 1:
                rval = POP_STR();
                todo = Sprint(&ss->sprinter, ss_format, cs->token, rval);
                break;

              case 0:
                todo = SprintCString(&ss->sprinter, cs->token);
                break;

              default:
                todo = -2;
                break;
            }
        } else {
            switch (op) {
#define BEGIN_LITOPX_CASE(OP)                                                 \
              case OP:                                                        \
                atomIndex = GET_ATOM_INDEX(pc);                               \
              do_##OP:                                                        \
                atom = js_GetAtom(cx, &jp->script->atomMap, atomIndex);

#define END_LITOPX_CASE                                                       \
                break;

              case JSOP_NOP:
                /*
                 * Check for a do-while loop, a for-loop with an empty
                 * initializer part, a labeled statement, a function
                 * definition, or try/finally.
                 */
                sn = js_GetSrcNote(jp->script, pc);
                todo = -2;
                switch (sn ? SN_TYPE(sn) : SRC_NULL) {
                  case SRC_WHILE:
                    js_printf(SET_MAYBE_BRACE(jp), "\tdo {\n");
                    jp->indent += 4;
                    break;

                  case SRC_FOR:
                    rval = "";

                  do_forloop:
                    /* Skip the JSOP_NOP or JSOP_POP bytecode. */
                    pc++;

                    /* Get the cond, next, and loop-closing tail offsets. */
                    cond = js_GetSrcNoteOffset(sn, 0);
                    next = js_GetSrcNoteOffset(sn, 1);
                    tail = js_GetSrcNoteOffset(sn, 2);
                    LOCAL_ASSERT(tail + GetJumpOffset(pc+tail, pc+tail) == 0);

                    /* Print the keyword and the possibly empty init-part. */
                    js_printf(jp, "\tfor (%s;", rval);

                    if (pc[cond] == JSOP_IFEQ || pc[cond] == JSOP_IFEQX) {
                        /* Decompile the loop condition. */
                        DECOMPILE_CODE(pc, cond);
                        js_printf(jp, " %s", POP_STR());
                    }

                    /* Need a semicolon whether or not there was a cond. */
                    js_puts(jp, ";");

                    if (pc[next] != JSOP_GOTO && pc[next] != JSOP_GOTOX) {
                        /* Decompile the loop updater. */
                        DECOMPILE_CODE(pc + next, tail - next - 1);
                        js_printf(jp, " %s", POP_STR());
                    }

                    /* Do the loop body. */
                    js_printf(SET_MAYBE_BRACE(jp), ") {\n");
                    jp->indent += 4;
                    oplen = (cond) ? js_CodeSpec[pc[cond]].length : 0;
                    DECOMPILE_CODE(pc + cond + oplen, next - cond - oplen);
                    jp->indent -= 4;
                    js_printf(jp, "\t}\n");

                    /* Set len so pc skips over the entire loop. */
                    len = tail + js_CodeSpec[pc[tail]].length;
                    break;

                  case SRC_LABEL:
                    atom = js_GetAtom(cx, &jp->script->atomMap,
                                      (jsatomid) js_GetSrcNoteOffset(sn, 0));
                    jp->indent -= 4;
                    rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                    if (!rval)
                        return JS_FALSE;
                    RETRACT(&ss->sprinter, rval);
                    js_printf(jp, "\t%s:\n", rval);
                    jp->indent += 4;
                    break;

                  case SRC_LABELBRACE:
                    atom = js_GetAtom(cx, &jp->script->atomMap,
                                      (jsatomid) js_GetSrcNoteOffset(sn, 0));
                    rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                    if (!rval)
                        return JS_FALSE;
                    RETRACT(&ss->sprinter, rval);
                    js_printf(CLEAR_MAYBE_BRACE(jp), "\t%s: {\n", rval);
                    jp->indent += 4;
                    break;

                  case SRC_ENDBRACE:
                    jp->indent -= 4;
                    js_printf(jp, "\t}\n");
                    break;

                  case SRC_FUNCDEF:
                    atom = js_GetAtom(cx, &jp->script->atomMap,
                                      (jsatomid) js_GetSrcNoteOffset(sn, 0));
                    JS_ASSERT(ATOM_IS_OBJECT(atom));
                  do_function:
                    obj = ATOM_TO_OBJECT(atom);
                    fun = (JSFunction *) JS_GetPrivate(cx, obj);
                    jp2 = js_NewPrinter(cx, JS_GetFunctionName(fun),
                                        jp->indent, jp->pretty);
                    if (!jp2)
                        return JS_FALSE;
                    jp2->scope = jp->scope;
                    js_puts(jp2, "\n");
                    ok = js_DecompileFunction(jp2, fun);
                    if (ok) {
                        js_puts(jp2, "\n");
                        str = js_GetPrinterOutput(jp2);
                        if (str)
                            js_printf(jp, "%s\n", JS_GetStringBytes(str));
                        else
                            ok = JS_FALSE;
                    }
                    js_DestroyPrinter(jp2);
                    if (!ok)
                        return JS_FALSE;

                    break;

                  case SRC_BRACE:
                    js_printf(CLEAR_MAYBE_BRACE(jp), "\t{\n");
                    jp->indent += 4;
                    len = js_GetSrcNoteOffset(sn, 0);
                    DECOMPILE_CODE(pc + oplen, len - oplen);
                    jp->indent -= 4;
                    js_printf(jp, "\t}\n");
                    break;

                  default:;
                }
              case JSOP_RETRVAL:
                break;

              case JSOP_GROUP:
                cs = &js_CodeSpec[lastop];
                if ((cs->prec != 0 && cs->prec == js_CodeSpec[pc[1]].prec) ||
                    pc[1] == JSOP_PUSHOBJ) {
                    /*
                     * Force parens if this JSOP_GROUP forced re-association
                     * against precedence, or if this is a call or constructor
                     * expression.
                     *
                     * This is necessary to handle the operator new grammar,
                     * by which new x(y).z means (new x(y))).z.  For example
                     * new (x(y).z) must decompile with the constructor
                     * parenthesized, but normal precedence has JSOP_GETPROP
                     * (for the final .z) higher than JSOP_NEW.  In general,
                     * if the call or constructor expression is parenthesized,
                     * we preserve parens.
                     */
                    op = JSOP_NAME;
                    rval = POP_STR();
                    todo = SprintCString(&ss->sprinter, rval);
                } else {
                    /*
                     * Don't explicitly parenthesize -- just fix the top
                     * opcode so that the auto-parens magic in PopOff can do
                     * its thing.
                     */
                    LOCAL_ASSERT(ss->top != 0);
                    ss->opcodes[ss->top-1] = saveop = lastop;
                    todo = -2;
                }
                break;

              case JSOP_STARTITER:
                if (ss->inArrayInit) {
                    ss->offsets[ss->top++] = ss->sprinter.offset;
                    ss->offsets[ss->top++] = ss->sprinter.offset;
                    ss->opcodes[ss->top-1] = ss->opcodes[ss->top-2] = op;
                    break;
                }
                todo = Sprint(&ss->sprinter, iter_cookie);
                if (todo < 0 || !PushOff(ss, todo, op))
                    return JS_FALSE;
                /* FALL THROUGH */

              case JSOP_PUSH:
              case JSOP_PUSHOBJ:
              case JSOP_BINDNAME:
              do_JSOP_BINDNAME:
                todo = Sprint(&ss->sprinter, "");
                break;

              case JSOP_TRY:
                js_printf(CLEAR_MAYBE_BRACE(jp), "\ttry {\n");
                jp->indent += 4;
                todo = -2;
                break;

              case JSOP_FINALLY:
                jp->indent -= 4;
                js_printf(CLEAR_MAYBE_BRACE(jp), "\t} finally {\n");
                jp->indent += 4;

                /*
                 * We must push an empty string placeholder for gosub's return
                 * address, popped by JSOP_RETSUB and counted by script->depth
                 * but not by ss->top (see JSOP_SETSP, below).
                 */
                todo = Sprint(&ss->sprinter, exception_cookie);
                if (todo < 0 || !PushOff(ss, todo, op))
                    return JS_FALSE;
                todo = Sprint(&ss->sprinter, retsub_pc_cookie);
                break;

              case JSOP_RETSUB:
                rval = POP_STR();
                LOCAL_ASSERT(strcmp(rval, retsub_pc_cookie) == 0);
                lval = POP_STR();
                LOCAL_ASSERT(strcmp(lval, exception_cookie) == 0);
                todo = -2;
                break;

              case JSOP_SWAP:
                /*
                 * We don't generate this opcode currently, and previously we
                 * did not need to decompile it.  If old, serialized bytecode
                 * uses it still, we should fall through and set todo = -2.
                 */
                /* FALL THROUGH */

              case JSOP_GOSUB:
              case JSOP_GOSUBX:
                /*
                 * JSOP_GOSUB and GOSUBX have no effect on the decompiler's
                 * string stack because the next op in bytecode order finds
                 * the stack balanced by a JSOP_RETSUB executed elsewhere.
                 */
                todo = -2;
                break;

              case JSOP_SETSP:
                /*
                 * The compiler models operand stack depth and fixes the stack
                 * pointer on entry to a catch clause based on its depth model.
                 * The decompiler must match the code generator's model, which
                 * is why JSOP_FINALLY pushes a cookie that JSOP_RETSUB pops.
                 */
                LOCAL_ASSERT(ss->top >= (uintN) GET_ATOM_INDEX(pc));
                ss->top = (uintN) GET_ATOM_INDEX(pc);
                break;

              case JSOP_EXCEPTION:
                /*
                 * The only other JSOP_EXCEPTION cases occur as part of a code
                 * sequence that follows a SRC_CATCH-annotated JSOP_ENTERBLOCK
                 * or that precedes a SRC_HIDDEN-annotated JSOP_POP emitted
                 * when returning from within a finally clause.
                 */
                sn = js_GetSrcNote(jp->script, pc);
                LOCAL_ASSERT(sn && SN_TYPE(sn) == SRC_HIDDEN);
                todo = -2;
                break;

              case JSOP_POP:
                /*
                 * By default, do not automatically parenthesize when popping
                 * a stacked expression decompilation.  We auto-parenthesize
                 * only when JSOP_POP is annotated with SRC_PCDELTA, meaning
                 * comma operator.
                 */
                op = JSOP_POPV;
                /* FALL THROUGH */

              case JSOP_POPV:
                sn = js_GetSrcNote(jp->script, pc);
                switch (sn ? SN_TYPE(sn) : SRC_NULL) {
                  case SRC_FOR:
                    /* Force parens around 'in' expression at 'for' front. */
                    if (ss->opcodes[ss->top-1] == JSOP_IN)
                        op = JSOP_LSH;
                    rval = POP_STR();
                    todo = -2;
                    goto do_forloop;

                  case SRC_PCDELTA:
                    /* Comma operator: use JSOP_POP for correct precedence. */
                    op = JSOP_POP;

                    /* Pop and save to avoid blowing stack depth budget. */
                    lval = JS_strdup(cx, POP_STR());
                    if (!lval)
                        return JS_FALSE;

                    /*
                     * The offset tells distance to the end of the right-hand
                     * operand of the comma operator.
                     */
                    done = pc + len;
                    pc += js_GetSrcNoteOffset(sn, 0);
                    len = 0;

                    if (!Decompile(ss, done, pc - done)) {
                        JS_free(cx, (char *)lval);
                        return JS_FALSE;
                    }

                    /* Pop Decompile result and print comma expression. */
                    rval = POP_STR();
                    todo = Sprint(&ss->sprinter, "%s, %s", lval, rval);
                    JS_free(cx, (char *)lval);
                    break;

                  case SRC_HIDDEN:
                    /* Hide this pop, it's from a goto in a with or for/in. */
                    todo = -2;
                    break;

                  case SRC_DECL:
                    /* This pop is at the end of the head of a let form. */
                    pc += JSOP_POP_LENGTH;
                    len = js_GetSrcNoteOffset(sn, 0);
                    if (pc[len] == JSOP_LEAVEBLOCK) {
                        js_printf(CLEAR_MAYBE_BRACE(jp), "\tlet (%s) {\n",
                                  POP_STR());
                        jp->indent += 4;
                        DECOMPILE_CODE(pc, len);
                        jp->indent -= 4;
                        js_printf(jp, "\t}\n");
                        todo = -2;
                    } else {
                        JS_ASSERT(pc[len] == JSOP_LEAVEBLOCKEXPR);

                        lval = JS_strdup(cx, POP_STR());
                        if (!lval)
                            return JS_FALSE;

                        if (!Decompile(ss, pc, len)) {
                            JS_free(cx, (char *)lval);
                            return JS_FALSE;
                        }
                        rval = POP_STR();
                        todo = Sprint(&ss->sprinter,
                                      (*rval == '{')
                                      ? "let (%s) (%s)"
                                      : "let (%s) %s",
                                      lval, rval);
                        JS_free(cx, (char *)lval);
                    }
                    break;

                  default:
                    /* Turn off parens around a yield statement. */
                    if (ss->opcodes[ss->top-1] == JSOP_YIELD)
                        op = JSOP_NOP;

                    rval = POP_STR();
                    if (*rval != '\0') {
#if JS_HAS_BLOCK_SCOPE
                        /*
                         * If a let declaration is the only child of a control
                         * structure that does not require braces, it must not
                         * be braced.  If it were braced explicitly, it would
                         * be bracketed by JSOP_ENTERBLOCK/JSOP_LEAVEBLOCK.
                         */
                        if (jp->braceState == MAYBE_BRACE &&
                            pc + JSOP_POP_LENGTH == endpc &&
                            !strncmp(rval, var_prefix[SRC_DECL_LET], 4) &&
                            rval[4] != '(') {
                            SetDontBrace(jp);
                        }
#endif
                        js_printf(jp,
                                  (*rval == '{' ||
                                   (strncmp(rval, js_function_str, 8) == 0 &&
                                    rval[8] == ' '))
                                  ? "\t(%s);\n"
                                  : "\t%s;\n",
                                  rval);
                    }
                    todo = -2;
                    break;
                }
                break;

              case JSOP_POP2:
              case JSOP_ENDITER:
                sn = js_GetSrcNote(jp->script, pc);
                todo = -2;
                if (sn && SN_TYPE(sn) == SRC_HIDDEN)
                    break;
                (void) PopOff(ss, op);
                if (ss->inArrayInit) {
                    ss->top -= 2;
                    break;
                }
                (void) PopOff(ss, op);
                if (op == JSOP_ENDITER) {
                    rval = POP_STR();
                    JS_ASSERT(!strcmp(rval, iter_cookie));
                }
                break;

              case JSOP_ENTERWITH:
                JS_ASSERT(!js_GetSrcNote(jp->script, pc));
                rval = POP_STR();
                js_printf(SET_MAYBE_BRACE(jp), "\twith (%s) {\n", rval);
                jp->indent += 4;
                todo = Sprint(&ss->sprinter, with_cookie);
                break;

              case JSOP_LEAVEWITH:
                sn = js_GetSrcNote(jp->script, pc);
                todo = -2;
                if (sn && SN_TYPE(sn) == SRC_HIDDEN)
                    break;
                rval = POP_STR();
                LOCAL_ASSERT(strcmp(rval, with_cookie) == 0);
                jp->indent -= 4;
                js_printf(jp, "\t}\n");
                break;

              BEGIN_LITOPX_CASE(JSOP_ENTERBLOCK)
              {
                JSAtom **atomv, *smallv[5];
                JSScopeProperty *sprop;

                atom = GET_ATOM(cx, jp->script, pc);
                obj = ATOM_TO_OBJECT(atom);
                argc = OBJ_BLOCK_COUNT(cx, obj);
                if ((size_t)argc <= sizeof smallv / sizeof smallv[0]) {
                    atomv = smallv;
                } else {
                    atomv = (JSAtom **) JS_malloc(cx, argc * sizeof(JSAtom *));
                    if (!atomv)
                        return JS_FALSE;
                }

                /* From here on, control must flow through enterblock_out. */
                for (sprop = OBJ_SCOPE(obj)->lastProp; sprop;
                     sprop = sprop->parent) {
                    if (!(sprop->flags & SPROP_HAS_SHORTID))
                        continue;
                    JS_ASSERT(sprop->shortid < argc);
                    atomv[sprop->shortid] = JSID_TO_ATOM(sprop->id);
                }
                ok = JS_TRUE;
                for (i = 0; i < argc; i++) {
                    atom = atomv[i];
                    rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                    if (!rval ||
                        !PushOff(ss, STR2OFF(&ss->sprinter, rval), op)) {
                        ok = JS_FALSE;
                        goto enterblock_out;
                    }
                }

                sn = js_GetSrcNote(jp->script, pc);
                switch (sn ? SN_TYPE(sn) : SRC_NULL) {
#if JS_HAS_BLOCK_SCOPE
                  case SRC_BRACE:
                    js_printf(CLEAR_MAYBE_BRACE(jp), "\t{\n");
                    jp->indent += 4;
                    len = js_GetSrcNoteOffset(sn, 0);
                    ok = Decompile(ss, pc + oplen, len - oplen);
                    if (!ok)
                        goto enterblock_out;
                    jp->indent -= 4;
                    js_printf(jp, "\t}\n");
                    break;
#endif

                  case SRC_CATCH:
                    jp->indent -= 4;
                    js_printf(CLEAR_MAYBE_BRACE(jp), "\t} catch (");

                    pc2 = pc;
                    pc += oplen;
                    LOCAL_ASSERT(*pc == JSOP_EXCEPTION);
                    pc += JSOP_EXCEPTION_LENGTH;
                    LOCAL_ASSERT(*pc == JSOP_SETLOCALPOP);
                    i = GET_UINT16(pc);
                    pc += JSOP_SETLOCALPOP_LENGTH;
                    str = ATOM_TO_STRING(atomv[i - OBJ_BLOCK_DEPTH(cx, obj)]);
                    rval = QuoteString(&ss->sprinter, str, 0);
                    if (!rval) {
                        ok = JS_FALSE;
                        goto enterblock_out;
                    }
                    RETRACT(&ss->sprinter, rval);
                    js_printf(jp, "%s", rval);

                    len = js_GetSrcNoteOffset(sn, 0);
                    if (len) {
                        len -= PTRDIFF(pc, pc2, jsbytecode);
                        JS_ASSERT(len > 0);
                        js_printf(jp, " if ");
                        ok = Decompile(ss, pc, len);
                        if (!ok)
                            goto enterblock_out;
                        js_printf(jp, "%s", POP_STR());
                        pc += len;
                        LOCAL_ASSERT(*pc == JSOP_IFEQ || *pc == JSOP_IFEQX);
                        pc += js_CodeSpec[*pc].length;
                    }

                    js_printf(jp, ") {\n");
                    jp->indent += 4;
                    len = 0;
                    break;
                }

                todo = -2;

              enterblock_out:
                if (atomv != smallv)
                    JS_free(cx, atomv);
                if (!ok)
                    return JS_FALSE;
              }
              END_LITOPX_CASE

              case JSOP_LEAVEBLOCK:
              case JSOP_LEAVEBLOCKEXPR:
              {
                uintN top, depth;

                sn = js_GetSrcNote(jp->script, pc);
                todo = -2;
                if (sn) {
                    JS_ASSERT(op == JSOP_LEAVEBLOCK);
                    if (SN_TYPE(sn) == SRC_HIDDEN)
                        break;
                    LOCAL_ASSERT(SN_TYPE(sn) == SRC_CATCH);
                    LOCAL_ASSERT((uintN)js_GetSrcNoteOffset(sn, 0) == ss->top);
                }
                if (op == JSOP_LEAVEBLOCKEXPR)
                    rval = POP_STR();
                top = ss->top;
                depth = GET_UINT16(pc);
                JS_ASSERT(top >= depth);
                top -= depth;
                ss->top = top;
                ss->sprinter.offset = ss->offsets[top];
                if (op == JSOP_LEAVEBLOCKEXPR)
                    todo = SprintCString(&ss->sprinter, rval);
                break;
              }

              case JSOP_GETLOCAL:
                i = GET_UINT16(pc);
                sn = js_GetSrcNote(jp->script, pc);
                if ((uintN)i < ss->top) {
                    rval = OFF2STR(&ss->sprinter, ss->offsets[i]);
                } else {
                    jsatomid j, n;
                    JSScript *script;
                    jsint depth, count;
                    JSScopeProperty *sprop;

                    /*
                     * We must be called from js_DecompileValueGenerator when
                     * dereferencing a local that's undefined or null.  Search
                     * jp->script->atomMap for the block containing this local
                     * by its stack index, i.
                     */
                    script = jp->script;
                    for (j = 0, n = script->atomMap.length; j < n; j++) {
                        atom = script->atomMap.vector[j];
                        if (ATOM_IS_OBJECT(atom)) {
                            obj = ATOM_TO_OBJECT(atom);
                            if (OBJ_GET_CLASS(cx, obj) == &js_BlockClass) {
                                depth = OBJ_BLOCK_DEPTH(cx, obj);
                                count = OBJ_BLOCK_COUNT(cx, obj);
                                if ((jsuint)(i - depth) < (jsuint)count)
                                    break;
                            }
                        }
                    }

                    LOCAL_ASSERT(j < n);
                    i -= depth;
                    for (sprop = OBJ_SCOPE(obj)->lastProp; sprop;
                         sprop = sprop->parent) {
                        if (sprop->shortid == i)
                            break;
                    }

                    LOCAL_ASSERT(sprop && JSID_IS_ATOM(sprop->id));
                    atom = JSID_TO_ATOM(sprop->id);
                    rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                    if (!rval)
                        return JS_FALSE;
                    RETRACT(&ss->sprinter, rval);
                }
                todo = Sprint(&ss->sprinter, ss_format, VarPrefix(sn), rval);
                break;

              case JSOP_SETLOCAL:
                i = GET_UINT16(pc);
                lval = OFF2STR(&ss->sprinter, ss->offsets[i]);
                rval = POP_STR();
                goto do_setlval;

              case JSOP_INCLOCAL:
              case JSOP_DECLOCAL:
                i = GET_UINT16(pc);
                lval = OFF2STR(&ss->sprinter, ss->offsets[i]);
                goto do_inclval;

              case JSOP_LOCALINC:
              case JSOP_LOCALDEC:
                i = GET_UINT16(pc);
                lval = OFF2STR(&ss->sprinter, ss->offsets[i]);
                goto do_lvalinc;

              case JSOP_FORLOCAL:
                i = GET_UINT16(pc);
                lval = OFF2STR(&ss->sprinter, ss->offsets[i]);
                atom = NULL;
                goto do_forlvalinloop;

              case JSOP_SETRVAL:
              case JSOP_RETURN:
                rval = POP_STR();
                if (*rval != '\0')
                    js_printf(jp, "\t%s %s;\n", js_return_str, rval);
                else
                    js_printf(jp, "\t%s;\n", js_return_str);
                todo = -2;
                break;

#if JS_HAS_GENERATORS
              case JSOP_YIELD:
                op = JSOP_SETNAME;      /* turn off most parens */
                rval = POP_STR();
                todo = (*rval != '\0')
                       ? Sprint(&ss->sprinter,
                                (strncmp(rval, js_yield_str, 5) == 0 &&
                                 (rval[5] == ' ' || rval[5] == '\0'))
                                ? "%s (%s)"
                                : "%s %s",
                                js_yield_str, rval)
                       : SprintCString(&ss->sprinter, js_yield_str);
                break;

              case JSOP_ARRAYPUSH:
              {
                uintN pos, blockpos, startpos;
                ptrdiff_t start;

                rval = POP_STR();
                pos = ss->top;
                while (ss->opcodes[--pos] != JSOP_ENTERBLOCK)
                    LOCAL_ASSERT(pos != 0);
                blockpos = pos;
                while (ss->opcodes[--pos] == JSOP_ENTERBLOCK) {
                    if (pos == 0)
                        break;
                }
                LOCAL_ASSERT(ss->opcodes[pos] == JSOP_NEWINIT);
                startpos = pos;
                start = ss->offsets[pos];
                LOCAL_ASSERT(ss->sprinter.base[start] == '[' ||
                             ss->sprinter.base[start] == '#');
                pos = blockpos;
                while (ss->opcodes[++pos] == JSOP_STARTITER)
                    LOCAL_ASSERT(pos < ss->top);
                LOCAL_ASSERT(pos < ss->top);
                xval = OFF2STR(&ss->sprinter, ss->offsets[pos]);
                lval = OFF2STR(&ss->sprinter, start);
                RETRACT(&ss->sprinter, lval);
                todo = Sprint(&ss->sprinter, "%s%s%.*s",
                              lval, rval, rval - xval, xval);
                if (todo < 0)
                    return JS_FALSE;
                ss->offsets[startpos] = todo;
                todo = -2;
                break;
              }
#endif

              case JSOP_THROWING:
                todo = -2;
                break;

              case JSOP_THROW:
                sn = js_GetSrcNote(jp->script, pc);
                todo = -2;
                if (sn && SN_TYPE(sn) == SRC_HIDDEN)
                    break;
                rval = POP_STR();
                js_printf(jp, "\t%s %s;\n", cs->name, rval);
                break;

              case JSOP_GOTO:
              case JSOP_GOTOX:
                sn = js_GetSrcNote(jp->script, pc);
                switch (sn ? SN_TYPE(sn) : SRC_NULL) {
                  case SRC_CONT2LABEL:
                    atom = js_GetAtom(cx, &jp->script->atomMap,
                                      (jsatomid) js_GetSrcNoteOffset(sn, 0));
                    rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                    if (!rval)
                        return JS_FALSE;
                    RETRACT(&ss->sprinter, rval);
                    js_printf(jp, "\tcontinue %s;\n", rval);
                    break;
                  case SRC_CONTINUE:
                    js_printf(jp, "\tcontinue;\n");
                    break;
                  case SRC_BREAK2LABEL:
                    atom = js_GetAtom(cx, &jp->script->atomMap,
                                      (jsatomid) js_GetSrcNoteOffset(sn, 0));
                    rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                    if (!rval)
                        return JS_FALSE;
                    RETRACT(&ss->sprinter, rval);
                    js_printf(jp, "\tbreak %s;\n", rval);
                    break;
                  case SRC_HIDDEN:
                    break;
                  default:
                    js_printf(jp, "\tbreak;\n");
                    break;
                }
                todo = -2;
                break;

              case JSOP_IFEQ:
              case JSOP_IFEQX:
              {
                JSBool elseif = JS_FALSE;

              if_again:
                len = GetJumpOffset(pc, pc);
                sn = js_GetSrcNote(jp->script, pc);

                switch (sn ? SN_TYPE(sn) : SRC_NULL) {
                  case SRC_IF:
                  case SRC_IF_ELSE:
                    op = JSOP_NOP;              /* turn off parens */
                    rval = POP_STR();
                    if (ss->inArrayInit) {
                        LOCAL_ASSERT(SN_TYPE(sn) == SRC_IF);
                        if (Sprint(&ss->sprinter, " if (%s)", rval) < 0)
                            return JS_FALSE;
                    } else {
                        js_printf(SET_MAYBE_BRACE(jp),
                                  elseif ? " if (%s) {\n" : "\tif (%s) {\n",
                                  rval);
                        jp->indent += 4;
                    }

                    if (SN_TYPE(sn) == SRC_IF) {
                        DECOMPILE_CODE(pc + oplen, len - oplen);
                    } else {
                        LOCAL_ASSERT(!ss->inArrayInit);
                        tail = js_GetSrcNoteOffset(sn, 0);
                        DECOMPILE_CODE(pc + oplen, tail - oplen);
                        jp->indent -= 4;
                        pc += tail;
                        LOCAL_ASSERT(*pc == JSOP_GOTO || *pc == JSOP_GOTOX);
                        oplen = js_CodeSpec[*pc].length;
                        len = GetJumpOffset(pc, pc);
                        js_printf(jp, "\t} else");

                        /*
                         * If the second offset for sn is non-zero, it tells
                         * the distance from the goto around the else, to the
                         * ifeq for the if inside the else that forms an "if
                         * else if" chain.  Thus cond spans the condition of
                         * the second if, so we simply decompile it and start
                         * over at label if_again.
                         */
                        cond = js_GetSrcNoteOffset(sn, 1);
                        if (cond != 0) {
                            DECOMPILE_CODE(pc + oplen, cond - oplen);
                            pc += cond;
                            elseif = JS_TRUE;
                            goto if_again;
                        }

                        js_printf(SET_MAYBE_BRACE(jp), " {\n");
                        jp->indent += 4;
                        DECOMPILE_CODE(pc + oplen, len - oplen);
                    }

                    if (!ss->inArrayInit) {
                        jp->indent -= 4;
                        js_printf(jp, "\t}\n");
                    }
                    todo = -2;
                    break;

                  case SRC_WHILE:
                    rval = POP_STR();
                    js_printf(SET_MAYBE_BRACE(jp), "\twhile (%s) {\n", rval);
                    jp->indent += 4;
                    tail = js_GetSrcNoteOffset(sn, 0);
                    DECOMPILE_CODE(pc + oplen, tail - oplen);
                    jp->indent -= 4;
                    js_printf(jp, "\t}\n");
                    todo = -2;
                    break;

                  case SRC_COND:
                    xval = JS_strdup(cx, POP_STR());
                    if (!xval)
                        return JS_FALSE;
                    len = js_GetSrcNoteOffset(sn, 0);
                    DECOMPILE_CODE(pc + oplen, len - oplen);
                    lval = JS_strdup(cx, POP_STR());
                    if (!lval) {
                        JS_free(cx, (void *)xval);
                        return JS_FALSE;
                    }
                    pc += len;
                    LOCAL_ASSERT(*pc == JSOP_GOTO || *pc == JSOP_GOTOX);
                    oplen = js_CodeSpec[*pc].length;
                    len = GetJumpOffset(pc, pc);
                    DECOMPILE_CODE(pc + oplen, len - oplen);
                    rval = POP_STR();
                    todo = Sprint(&ss->sprinter, "%s ? %s : %s",
                                  xval, lval, rval);
                    JS_free(cx, (void *)xval);
                    JS_free(cx, (void *)lval);
                    break;

                  default:
                    break;
                }
                break;
              }

              case JSOP_IFNE:
              case JSOP_IFNEX:
                /* Currently, this must be a do-while loop's upward branch. */
                jp->indent -= 4;
                js_printf(jp, "\t} while (%s);\n", POP_STR());
                todo = -2;
                break;

              case JSOP_OR:
              case JSOP_ORX:
                xval = "||";

              do_logical_connective:
                /* Top of stack is the first clause in a disjunction (||). */
                lval = JS_strdup(cx, POP_STR());
                if (!lval)
                    return JS_FALSE;
                done = pc + GetJumpOffset(pc, pc);
                pc += len;
                len = PTRDIFF(done, pc, jsbytecode);
                DECOMPILE_CODE(pc, len);
                rval = POP_STR();
                if (jp->pretty &&
                    jp->indent + 4 + strlen(lval) + 4 + strlen(rval) > 75) {
                    rval = JS_strdup(cx, rval);
                    if (!rval) {
                        tail = -1;
                    } else {
                        todo = Sprint(&ss->sprinter, "%s %s\n", lval, xval);
                        tail = Sprint(&ss->sprinter, "%*s%s",
                                      jp->indent + 4, "", rval);
                        JS_free(cx, (char *)rval);
                    }
                    if (tail < 0)
                        todo = -1;
                } else {
                    todo = Sprint(&ss->sprinter, "%s %s %s", lval, xval, rval);
                }
                JS_free(cx, (char *)lval);
                break;

              case JSOP_AND:
              case JSOP_ANDX:
                xval = "&&";
                goto do_logical_connective;

              case JSOP_FORARG:
                atom = GetSlotAtom(jp, js_GetArgument, GET_ARGNO(pc));
                LOCAL_ASSERT(atom);
                goto do_fornameinloop;

              case JSOP_FORVAR:
                atom = GetSlotAtom(jp, js_GetLocalVariable, GET_VARNO(pc));
                LOCAL_ASSERT(atom);
                goto do_fornameinloop;

              case JSOP_FORNAME:
                atom = GET_ATOM(cx, jp->script, pc);

              do_fornameinloop:
                lval = "";
              do_forlvalinloop:
                sn = js_GetSrcNote(jp->script, pc);
                xval = NULL;
                goto do_forinloop;

              case JSOP_FORPROP:
                xval = NULL;
                atom = GET_ATOM(cx, jp->script, pc);
                if (!ATOM_IS_IDENTIFIER(atom)) {
                    xval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom),
                                       (jschar)'\'');
                    if (!xval)
                        return JS_FALSE;
                    atom = NULL;
                }
                lval = POP_STR();
                sn = NULL;

              do_forinloop:
                pc += oplen;
                LOCAL_ASSERT(*pc == JSOP_IFEQ || *pc == JSOP_IFEQX);
                oplen = js_CodeSpec[*pc].length;
                len = GetJumpOffset(pc, pc);
                sn2 = js_GetSrcNote(jp->script, pc);
                tail = js_GetSrcNoteOffset(sn2, 0);

              do_forinhead:
                if (!atom && xval) {
                    /*
                     * If xval is not a dummy empty string, we have to strdup
                     * it to save it from being clobbered by the first Sprint
                     * below.  Standard dumb decompiler operating procedure!
                     */
                    if (*xval == '\0') {
                        xval = NULL;
                    } else {
                        xval = JS_strdup(cx, xval);
                        if (!xval)
                            return JS_FALSE;
                    }
                }

#if JS_HAS_XML_SUPPORT
                if (foreach) {
                    foreach = JS_FALSE;
                    todo = Sprint(&ss->sprinter, "for %s (%s%s",
                                  js_each_str, VarPrefix(sn), lval);
                } else
#endif
                {
                    todo = Sprint(&ss->sprinter, "for (%s%s",
                                  VarPrefix(sn), lval);
                }
                if (atom) {
                    if (*lval && SprintPut(&ss->sprinter, ".", 1) < 0)
                        return JS_FALSE;
                    xval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                    if (!xval)
                        return JS_FALSE;
                } else if (xval) {
                    JS_ASSERT(*xval != '\0');
                    ok = (Sprint(&ss->sprinter,
                                 (js_CodeSpec[lastop].format & JOF_XMLNAME)
                                 ? ".%s"
                                 : "[%s]",
                                 xval)
                          >= 0);
                    JS_free(cx, (char *)xval);
                    if (!ok)
                        return JS_FALSE;
                }
                if (todo < 0)
                    return JS_FALSE;

                lval = OFF2STR(&ss->sprinter, todo);
                rval = OFF2STR(&ss->sprinter, ss->offsets[ss->top-1]);
                RETRACT(&ss->sprinter, rval);
                if (ss->inArrayInit) {
                    todo = Sprint(&ss->sprinter, " %s in %s)", lval, rval);
                    if (todo < 0)
                        return JS_FALSE;
                    ss->offsets[ss->top-1] = todo;
                    ss->sprinter.offset += PAREN_SLOP;
                    DECOMPILE_CODE(pc + oplen, tail - oplen);
                } else {
                    js_printf(SET_MAYBE_BRACE(jp), "\t%s in %s) {\n",
                              lval, rval);
                    jp->indent += 4;
                    DECOMPILE_CODE(pc + oplen, tail - oplen);
                    jp->indent -= 4;
                    js_printf(jp, "\t}\n");
                }
                todo = -2;
                break;

              case JSOP_FORELEM:
                pc++;
                LOCAL_ASSERT(*pc == JSOP_IFEQ || *pc == JSOP_IFEQX);
                len = js_CodeSpec[*pc].length;

                /*
                 * Arrange for the JSOP_ENUMELEM case to set tail for use by
                 * do_forinhead: code that uses on it to find the loop-closing
                 * jump (whatever its format, normal or extended), in order to
                 * bound the recursively decompiled loop body.
                 */
                sn = js_GetSrcNote(jp->script, pc);
                LOCAL_ASSERT(!forelem_tail);
                forelem_tail = pc + js_GetSrcNoteOffset(sn, 0);

                /*
                 * This gets a little wacky.  Only the length of the for loop
                 * body PLUS the element-indexing expression is known here, so
                 * we pass the after-loop pc to the JSOP_ENUMELEM case, which
                 * is immediately below, to decompile that helper bytecode via
                 * the 'forelem_done' local.
                 *
                 * Since a for..in loop can't nest in the head of another for
                 * loop, we can use forelem_{tail,done} singletons to remember
                 * state from JSOP_FORELEM to JSOP_ENUMELEM, thence (via goto)
                 * to label do_forinhead.
                 */
                LOCAL_ASSERT(!forelem_done);
                forelem_done = pc + GetJumpOffset(pc, pc);

                /* Our net stack balance after forelem;ifeq is +1. */
                todo = SprintCString(&ss->sprinter, forelem_cookie);
                break;

              case JSOP_ENUMELEM:
                /*
                 * The stack has the object under the (top) index expression.
                 * The "rval" property id is underneath those two on the stack.
                 * The for loop body net and gross lengths can now be adjusted
                 * to account for the length of the indexing expression that
                 * came after JSOP_FORELEM and before JSOP_ENUMELEM.
                 */
                atom = NULL;
                xval = POP_STR();
                op = JSOP_GETELEM;      /* lval must have high precedence */
                lval = POP_STR();
                op = saveop;
                rval = POP_STR();
                LOCAL_ASSERT(strcmp(rval, forelem_cookie) == 0);
                LOCAL_ASSERT(forelem_tail > pc);
                tail = forelem_tail - pc;
                forelem_tail = NULL;
                LOCAL_ASSERT(forelem_done > pc);
                len = forelem_done - pc;
                forelem_done = NULL;
                goto do_forinhead;

#if JS_HAS_GETTER_SETTER
              case JSOP_GETTER:
              case JSOP_SETTER:
                todo = -2;
                break;
#endif

              case JSOP_DUP2:
                rval = OFF2STR(&ss->sprinter, ss->offsets[ss->top-2]);
                todo = SprintCString(&ss->sprinter, rval);
                if (todo < 0 || !PushOff(ss, todo, ss->opcodes[ss->top-2]))
                    return JS_FALSE;
                /* FALL THROUGH */

              case JSOP_DUP:
                rval = OFF2STR(&ss->sprinter, ss->offsets[ss->top-1]);
                saveop = ss->opcodes[ss->top-1];
                todo = SprintCString(&ss->sprinter, rval);
                break;

              case JSOP_SETARG:
                atom = GetSlotAtom(jp, js_GetArgument, GET_ARGNO(pc));
                LOCAL_ASSERT(atom);
                goto do_setname;

              case JSOP_SETVAR:
                atom = GetSlotAtom(jp, js_GetLocalVariable, GET_VARNO(pc));
                LOCAL_ASSERT(atom);
                goto do_setname;

              case JSOP_SETCONST:
              case JSOP_SETNAME:
              case JSOP_SETGVAR:
                atomIndex = GET_ATOM_INDEX(pc);

              do_JSOP_SETCONST:
                atom = js_GetAtom(cx, &jp->script->atomMap, atomIndex);

              do_setname:
                lval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                if (!lval)
                    return JS_FALSE;
                rval = POP_STR();
                if (op == JSOP_SETNAME)
                    (void) PopOff(ss, op);

              do_setlval:
                sn = js_GetSrcNote(jp->script, pc - 1);
                if (sn && SN_TYPE(sn) == SRC_ASSIGNOP) {
                    todo = Sprint(&ss->sprinter, "%s %s= %s",
                                  lval,
                                  (lastop == JSOP_GETTER)
                                  ? js_getter_str
                                  : (lastop == JSOP_SETTER)
                                  ? js_setter_str
                                  : js_CodeSpec[lastop].token,
                                  rval);
                } else {
                    sn = js_GetSrcNote(jp->script, pc);
                    todo = Sprint(&ss->sprinter, "%s%s = %s",
                                  VarPrefix(sn), lval, rval);
                }
                break;

              case JSOP_NEW:
              case JSOP_CALL:
              case JSOP_EVAL:
#if JS_HAS_LVALUE_RETURN
              case JSOP_SETCALL:
#endif
                op = JSOP_SETNAME;      /* turn off most parens */
                argc = GET_ARGC(pc);
                argv = (char **)
                    JS_malloc(cx, (size_t)(argc + 1) * sizeof *argv);
                if (!argv)
                    return JS_FALSE;

                ok = JS_TRUE;
                for (i = argc; i > 0; i--) {
                    argv[i] = JS_strdup(cx, POP_STR());
                    if (!argv[i]) {
                        ok = JS_FALSE;
                        break;
                    }
                }

                /* Skip the JSOP_PUSHOBJ-created empty string. */
                LOCAL_ASSERT(ss->top >= 2);
                (void) PopOff(ss, op);

                op = saveop;
                argv[0] = JS_strdup(cx, POP_STR());
                if (!argv[i])
                    ok = JS_FALSE;

                lval = "(", rval = ")";
                if (op == JSOP_NEW) {
                    if (argc == 0)
                        lval = rval = "";
                    todo = Sprint(&ss->sprinter, "%s %s%s",
                                  js_new_str, argv[0], lval);
                } else {
                    todo = Sprint(&ss->sprinter, ss_format,
                                  argv[0], lval);
                }
                if (todo < 0)
                    ok = JS_FALSE;

                for (i = 1; i <= argc; i++) {
                    if (!argv[i] ||
                        Sprint(&ss->sprinter, ss_format,
                               argv[i], (i < argc) ? ", " : "") < 0) {
                        ok = JS_FALSE;
                        break;
                    }
                }
                if (Sprint(&ss->sprinter, rval) < 0)
                    ok = JS_FALSE;

                for (i = 0; i <= argc; i++) {
                    if (argv[i])
                        JS_free(cx, argv[i]);
                }
                JS_free(cx, argv);
                if (!ok)
                    return JS_FALSE;
#if JS_HAS_LVALUE_RETURN
                if (op == JSOP_SETCALL) {
                    if (!PushOff(ss, todo, op))
                        return JS_FALSE;
                    todo = Sprint(&ss->sprinter, "");
                }
#endif
                break;

              case JSOP_DELNAME:
                atom = GET_ATOM(cx, jp->script, pc);
                lval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                if (!lval)
                    return JS_FALSE;
                RETRACT(&ss->sprinter, lval);
              do_delete_lval:
                todo = Sprint(&ss->sprinter, "%s %s", js_delete_str, lval);
                break;

              case JSOP_DELPROP:
                GET_ATOM_QUOTE_AND_FMT("%s %s[%s]", "%s %s.%s", rval);
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, fmt, js_delete_str, lval, rval);
                break;

              case JSOP_DELELEM:
                op = JSOP_NOP;          /* turn off parens */
                xval = POP_STR();
                op = saveop;
                lval = POP_STR();
                if (*xval == '\0')
                    goto do_delete_lval;
                todo = Sprint(&ss->sprinter,
                              (js_CodeSpec[lastop].format & JOF_XMLNAME)
                              ? "%s %s.%s"
                              : "%s %s[%s]",
                              js_delete_str, lval, xval);
                break;

#if JS_HAS_XML_SUPPORT
              case JSOP_DELDESC:
                xval = POP_STR();
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, "%s %s..%s",
                              js_delete_str, lval, xval);
                break;
#endif

              case JSOP_TYPEOFEXPR:
              case JSOP_TYPEOF:
              case JSOP_VOID:
                rval = POP_STR();
                todo = Sprint(&ss->sprinter, "%s %s", cs->name, rval);
                break;

              case JSOP_INCARG:
              case JSOP_DECARG:
                atom = GetSlotAtom(jp, js_GetArgument, GET_ARGNO(pc));
                LOCAL_ASSERT(atom);
                goto do_incatom;

              case JSOP_INCVAR:
              case JSOP_DECVAR:
                atom = GetSlotAtom(jp, js_GetLocalVariable, GET_VARNO(pc));
                LOCAL_ASSERT(atom);
                goto do_incatom;

              case JSOP_INCNAME:
              case JSOP_DECNAME:
              case JSOP_INCGVAR:
              case JSOP_DECGVAR:
                atom = GET_ATOM(cx, jp->script, pc);
              do_incatom:
                lval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                if (!lval)
                    return JS_FALSE;
                RETRACT(&ss->sprinter, lval);
              do_inclval:
                todo = Sprint(&ss->sprinter, ss_format,
                              js_incop_strs[!(cs->format & JOF_INC)], lval);
                break;

              case JSOP_INCPROP:
              case JSOP_DECPROP:
                GET_ATOM_QUOTE_AND_FMT(preindex_format, predot_format, rval);

                /*
                 * Force precedence below the numeric literal opcodes, so that
                 * 42..foo or 10000..toString(16), e.g., decompile with parens
                 * around the left-hand side of dot.
                 */
                op = JSOP_GETPROP;
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, fmt,
                              js_incop_strs[!(cs->format & JOF_INC)],
                              lval, rval);
                break;

              case JSOP_INCELEM:
              case JSOP_DECELEM:
                op = JSOP_NOP;          /* turn off parens */
                xval = POP_STR();
                op = saveop;
                lval = POP_STR();
                if (*xval != '\0') {
                    todo = Sprint(&ss->sprinter,
                                  (js_CodeSpec[lastop].format & JOF_XMLNAME)
                                  ? predot_format
                                  : preindex_format,
                                  js_incop_strs[!(cs->format & JOF_INC)],
                                  lval, xval);
                } else {
                    todo = Sprint(&ss->sprinter, ss_format,
                                  js_incop_strs[!(cs->format & JOF_INC)], lval);
                }
                break;

              case JSOP_ARGINC:
              case JSOP_ARGDEC:
                atom = GetSlotAtom(jp, js_GetArgument, GET_ARGNO(pc));
                LOCAL_ASSERT(atom);
                goto do_atominc;

              case JSOP_VARINC:
              case JSOP_VARDEC:
                atom = GetSlotAtom(jp, js_GetLocalVariable, GET_VARNO(pc));
                LOCAL_ASSERT(atom);
                goto do_atominc;

              case JSOP_NAMEINC:
              case JSOP_NAMEDEC:
              case JSOP_GVARINC:
              case JSOP_GVARDEC:
                atom = GET_ATOM(cx, jp->script, pc);
              do_atominc:
                lval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                if (!lval)
                    return JS_FALSE;
                RETRACT(&ss->sprinter, lval);
              do_lvalinc:
                todo = Sprint(&ss->sprinter, ss_format,
                              lval, js_incop_strs[!(cs->format & JOF_INC)]);
                break;

              case JSOP_PROPINC:
              case JSOP_PROPDEC:
                GET_ATOM_QUOTE_AND_FMT(postindex_format, postdot_format, rval);

                /*
                 * Force precedence below the numeric literal opcodes, so that
                 * 42..foo or 10000..toString(16), e.g., decompile with parens
                 * around the left-hand side of dot.
                 */
                op = JSOP_GETPROP;
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, fmt, lval, rval,
                              js_incop_strs[!(cs->format & JOF_INC)]);
                break;

              case JSOP_ELEMINC:
              case JSOP_ELEMDEC:
                op = JSOP_NOP;          /* turn off parens */
                xval = POP_STR();
                op = saveop;
                lval = POP_STR();
                if (*xval != '\0') {
                    todo = Sprint(&ss->sprinter,
                                  (js_CodeSpec[lastop].format & JOF_XMLNAME)
                                  ? postdot_format
                                  : postindex_format,
                                  lval, xval,
                                  js_incop_strs[!(cs->format & JOF_INC)]);
                } else {
                    todo = Sprint(&ss->sprinter, ss_format,
                                  lval, js_incop_strs[!(cs->format & JOF_INC)]);
                }
                break;

              case JSOP_GETPROP2:
                op = JSOP_GETPROP;
                (void) PopOff(ss, lastop);
                /* FALL THROUGH */

              case JSOP_GETPROP:
              case JSOP_GETXPROP:
                atom = GET_ATOM(cx, jp->script, pc);

              do_getprop:
                GET_QUOTE_AND_FMT(index_format, dot_format, rval);

              do_getprop_lval:
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, fmt, lval, rval);
                break;

#if JS_HAS_XML_SUPPORT
              BEGIN_LITOPX_CASE(JSOP_GETMETHOD)
                sn = js_GetSrcNote(jp->script, pc);
                if (sn && SN_TYPE(sn) == SRC_PCBASE)
                    goto do_getprop;
                GET_QUOTE_AND_FMT("%s.function::[%s]", "%s.function::%s", rval);
                goto do_getprop_lval;

              BEGIN_LITOPX_CASE(JSOP_SETMETHOD)
                sn = js_GetSrcNote(jp->script, pc);
                if (sn && SN_TYPE(sn) == SRC_PCBASE)
                    goto do_setprop;
                GET_QUOTE_AND_FMT("%s.function::[%s] %s= %s",
                                  "%s.function::%s %s= %s",
                                  xval);
                goto do_setprop_rval;
#endif

              case JSOP_SETPROP:
                atom = GET_ATOM(cx, jp->script, pc);

              do_setprop:
                GET_QUOTE_AND_FMT("%s[%s] %s= %s", "%s.%s %s= %s", xval);

              do_setprop_rval:
                rval = POP_STR();

                /*
                 * Force precedence below the numeric literal opcodes, so that
                 * 42..foo or 10000..toString(16), e.g., decompile with parens
                 * around the left-hand side of dot.
                 */
                op = JSOP_GETPROP;
                lval = POP_STR();
                sn = js_GetSrcNote(jp->script, pc - 1);
                todo = Sprint(&ss->sprinter, fmt, lval, xval,
                              (sn && SN_TYPE(sn) == SRC_ASSIGNOP)
                              ? (lastop == JSOP_GETTER)
                                ? js_getter_str
                                : (lastop == JSOP_SETTER)
                                ? js_setter_str
                                : js_CodeSpec[lastop].token
                              : "",
                              rval);
                break;

              case JSOP_GETELEM2:
                op = JSOP_GETELEM;
                (void) PopOff(ss, lastop);
                /* FALL THROUGH */

              case JSOP_GETELEM:
              case JSOP_GETXELEM:
                op = JSOP_NOP;          /* turn off parens */
                xval = POP_STR();
                op = saveop;
                lval = POP_STR();
                if (*xval == '\0') {
                    todo = Sprint(&ss->sprinter, "%s", lval);
                } else {
                    todo = Sprint(&ss->sprinter,
                                  (js_CodeSpec[lastop].format & JOF_XMLNAME)
                                  ? dot_format
                                  : index_format,
                                  lval, xval);
                }
                break;

              case JSOP_SETELEM:
                rval = POP_STR();
                op = JSOP_NOP;          /* turn off parens */
                xval = POP_STR();
                cs = &js_CodeSpec[ss->opcodes[ss->top]];
                op = JSOP_GETELEM;      /* lval must have high precedence */
                lval = POP_STR();
                op = saveop;
                if (*xval == '\0')
                    goto do_setlval;
                sn = js_GetSrcNote(jp->script, pc - 1);
                todo = Sprint(&ss->sprinter,
                              (cs->format & JOF_XMLNAME)
                              ? "%s.%s %s= %s"
                              : "%s[%s] %s= %s",
                              lval, xval,
                              (sn && SN_TYPE(sn) == SRC_ASSIGNOP)
                              ? (lastop == JSOP_GETTER)
                                ? js_getter_str
                                : (lastop == JSOP_SETTER)
                                ? js_setter_str
                                : js_CodeSpec[lastop].token
                              : "",
                              rval);
                break;

              case JSOP_ARGSUB:
                i = (jsint) GET_ATOM_INDEX(pc);
                todo = Sprint(&ss->sprinter, "%s[%d]",
                              js_arguments_str, (int) i);
                break;

              case JSOP_ARGCNT:
                todo = Sprint(&ss->sprinter, dot_format,
                              js_arguments_str, js_length_str);
                break;

              case JSOP_GETARG:
                atom = GetSlotAtom(jp, js_GetArgument, GET_ARGNO(pc));
                LOCAL_ASSERT(atom);
                goto do_name;

              case JSOP_GETVAR:
                atom = GetSlotAtom(jp, js_GetLocalVariable, GET_VARNO(pc));
                LOCAL_ASSERT(atom);
                goto do_name;

              case JSOP_NAME:
              case JSOP_GETGVAR:
                atom = GET_ATOM(cx, jp->script, pc);
              do_name:
                lval = "";
              do_qname:
                sn = js_GetSrcNote(jp->script, pc);
                rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                if (!rval)
                    return JS_FALSE;
                RETRACT(&ss->sprinter, rval);
                todo = Sprint(&ss->sprinter, "%s%s%s",
                              VarPrefix(sn), lval, rval);
                break;

              case JSOP_UINT16:
                i = (jsint) GET_ATOM_INDEX(pc);
                goto do_sprint_int;

              case JSOP_UINT24:
                i = (jsint) GET_UINT24(pc);
              do_sprint_int:
                todo = Sprint(&ss->sprinter, "%u", (unsigned) i);
                break;

              case JSOP_LITERAL:
                atomIndex = GET_LITERAL_INDEX(pc);
                goto do_JSOP_STRING;

              case JSOP_FINDNAME:
                atomIndex = GET_LITERAL_INDEX(pc);
                todo = Sprint(&ss->sprinter, "");
                if (todo < 0 || !PushOff(ss, todo, op))
                    return JS_FALSE;
                atom = js_GetAtom(cx, &jp->script->atomMap, atomIndex);
                goto do_name;

              case JSOP_LITOPX:
                atomIndex = GET_LITERAL_INDEX(pc);
                pc2 = pc + 1 + LITERAL_INDEX_LEN;
                op = saveop = *pc2;
                pc += len - (1 + ATOM_INDEX_LEN);
                cs = &js_CodeSpec[op];
                len = cs->length;
                switch (op) {
                  case JSOP_ANONFUNOBJ:   goto do_JSOP_ANONFUNOBJ;
                  case JSOP_BINDNAME:     goto do_JSOP_BINDNAME;
                  case JSOP_CLOSURE:      goto do_JSOP_CLOSURE;
#if JS_HAS_EXPORT_IMPORT
                  case JSOP_EXPORTNAME:   goto do_JSOP_EXPORTNAME;
#endif
#if JS_HAS_XML_SUPPORT
                  case JSOP_GETMETHOD:    goto do_JSOP_GETMETHOD;
                  case JSOP_SETMETHOD:    goto do_JSOP_SETMETHOD;
#endif
                  case JSOP_NAMEDFUNOBJ:  goto do_JSOP_NAMEDFUNOBJ;
                  case JSOP_NUMBER:       goto do_JSOP_NUMBER;
                  case JSOP_OBJECT:       goto do_JSOP_OBJECT;
#if JS_HAS_XML_SUPPORT
                  case JSOP_QNAMECONST:   goto do_JSOP_QNAMECONST;
                  case JSOP_QNAMEPART:    goto do_JSOP_QNAMEPART;
#endif
                  case JSOP_REGEXP:       goto do_JSOP_REGEXP;
                  case JSOP_SETCONST:     goto do_JSOP_SETCONST;
                  case JSOP_STRING:       goto do_JSOP_STRING;
#if JS_HAS_XML_SUPPORT
                  case JSOP_XMLCDATA:     goto do_JSOP_XMLCDATA;
                  case JSOP_XMLCOMMENT:   goto do_JSOP_XMLCOMMENT;
                  case JSOP_XMLOBJECT:    goto do_JSOP_XMLOBJECT;
                  case JSOP_XMLPI:        goto do_JSOP_XMLPI;
#endif
                  case JSOP_ENTERBLOCK:   goto do_JSOP_ENTERBLOCK;
                  default:                JS_ASSERT(0);
                }
                /* NOTREACHED */
                break;

              BEGIN_LITOPX_CASE(JSOP_NUMBER)
                val = ATOM_KEY(atom);
                if (JSVAL_IS_INT(val)) {
                    long ival = (long)JSVAL_TO_INT(val);
                    todo = Sprint(&ss->sprinter, "%ld", ival);
                } else {
                    char buf[DTOSTR_STANDARD_BUFFER_SIZE];
                    char *numStr = JS_dtostr(buf, sizeof buf, DTOSTR_STANDARD,
                                             0, *JSVAL_TO_DOUBLE(val));
                    if (!numStr) {
                        JS_ReportOutOfMemory(cx);
                        return JS_FALSE;
                    }
                    todo = Sprint(&ss->sprinter, numStr);
                }
              END_LITOPX_CASE

              BEGIN_LITOPX_CASE(JSOP_STRING)
                rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom),
                                   inXML ? QUOTE_IN_XML : '"');
                if (!rval)
                    return JS_FALSE;
                todo = STR2OFF(&ss->sprinter, rval);
              END_LITOPX_CASE

              case JSOP_OBJECT:
              case JSOP_REGEXP:
              case JSOP_ANONFUNOBJ:
              case JSOP_NAMEDFUNOBJ:
                atomIndex = GET_ATOM_INDEX(pc);

              do_JSOP_OBJECT:
              do_JSOP_REGEXP:
              do_JSOP_ANONFUNOBJ:
              do_JSOP_NAMEDFUNOBJ:
                atom = js_GetAtom(cx, &jp->script->atomMap, atomIndex);
                if (op == JSOP_OBJECT || op == JSOP_REGEXP) {
                    if (!js_regexp_toString(cx, ATOM_TO_OBJECT(atom), 0, NULL,
                                            &val)) {
                        return JS_FALSE;
                    }
                } else {
                    if (!js_fun_toString(cx, ATOM_TO_OBJECT(atom),
                                         JS_IN_GROUP_CONTEXT |
                                         JS_DONT_PRETTY_PRINT,
                                         0, NULL, &val)) {
                        return JS_FALSE;
                    }
                }
                str = JSVAL_TO_STRING(val);
                todo = SprintPut(&ss->sprinter, JS_GetStringBytes(str),
                                 JSSTRING_LENGTH(str));
                break;

              case JSOP_TABLESWITCH:
              case JSOP_TABLESWITCHX:
              {
                ptrdiff_t jmplen, off, off2;
                jsint j, n, low, high;
                TableEntry *table, pivot;

                sn = js_GetSrcNote(jp->script, pc);
                JS_ASSERT(sn && SN_TYPE(sn) == SRC_SWITCH);
                len = js_GetSrcNoteOffset(sn, 0);
                jmplen = (op == JSOP_TABLESWITCH) ? JUMP_OFFSET_LEN
                                                  : JUMPX_OFFSET_LEN;
                pc2 = pc;
                off = GetJumpOffset(pc, pc2);
                pc2 += jmplen;
                low = GET_JUMP_OFFSET(pc2);
                pc2 += JUMP_OFFSET_LEN;
                high = GET_JUMP_OFFSET(pc2);
                pc2 += JUMP_OFFSET_LEN;

                n = high - low + 1;
                if (n == 0) {
                    table = NULL;
                    j = 0;
                } else {
                    table = (TableEntry *)
                            JS_malloc(cx, (size_t)n * sizeof *table);
                    if (!table)
                        return JS_FALSE;
                    for (i = j = 0; i < n; i++) {
                        table[j].label = NULL;
                        off2 = GetJumpOffset(pc, pc2);
                        if (off2) {
                            sn = js_GetSrcNote(jp->script, pc2);
                            if (sn) {
                                JS_ASSERT(SN_TYPE(sn) == SRC_LABEL);
                                table[j].label =
                                    js_GetAtom(cx, &jp->script->atomMap,
                                               (jsatomid)
                                               js_GetSrcNoteOffset(sn, 0));
                            }
                            table[j].key = INT_TO_JSVAL(low + i);
                            table[j].offset = off2;
                            table[j].order = j;
                            j++;
                        }
                        pc2 += jmplen;
                    }
                    js_HeapSort(table, (size_t) j, &pivot, sizeof(TableEntry),
                                CompareOffsets, NULL);
                }

                ok = DecompileSwitch(ss, table, (uintN)j, pc, len, off,
                                     JS_FALSE);
                JS_free(cx, table);
                if (!ok)
                    return ok;
                todo = -2;
                break;
              }

              case JSOP_LOOKUPSWITCH:
              case JSOP_LOOKUPSWITCHX:
              {
                ptrdiff_t jmplen, off, off2;
                jsatomid npairs, k;
                TableEntry *table;

                sn = js_GetSrcNote(jp->script, pc);
                JS_ASSERT(sn && SN_TYPE(sn) == SRC_SWITCH);
                len = js_GetSrcNoteOffset(sn, 0);
                jmplen = (op == JSOP_LOOKUPSWITCH) ? JUMP_OFFSET_LEN
                                                   : JUMPX_OFFSET_LEN;
                pc2 = pc;
                off = GetJumpOffset(pc, pc2);
                pc2 += jmplen;
                npairs = GET_ATOM_INDEX(pc2);
                pc2 += ATOM_INDEX_LEN;

                table = (TableEntry *)
                    JS_malloc(cx, (size_t)npairs * sizeof *table);
                if (!table)
                    return JS_FALSE;
                for (k = 0; k < npairs; k++) {
                    sn = js_GetSrcNote(jp->script, pc2);
                    if (sn) {
                        JS_ASSERT(SN_TYPE(sn) == SRC_LABEL);
                        table[k].label =
                            js_GetAtom(cx, &jp->script->atomMap, (jsatomid)
                                       js_GetSrcNoteOffset(sn, 0));
                    } else {
                        table[k].label = NULL;
                    }
                    atom = GET_ATOM(cx, jp->script, pc2);
                    pc2 += ATOM_INDEX_LEN;
                    off2 = GetJumpOffset(pc, pc2);
                    pc2 += jmplen;
                    table[k].key = ATOM_KEY(atom);
                    table[k].offset = off2;
                }

                ok = DecompileSwitch(ss, table, (uintN)npairs, pc, len, off,
                                     JS_FALSE);
                JS_free(cx, table);
                if (!ok)
                    return ok;
                todo = -2;
                break;
              }

              case JSOP_CONDSWITCH:
              {
                ptrdiff_t off, off2, caseOff;
                jsint ncases;
                TableEntry *table;

                sn = js_GetSrcNote(jp->script, pc);
                JS_ASSERT(sn && SN_TYPE(sn) == SRC_SWITCH);
                len = js_GetSrcNoteOffset(sn, 0);
                off = js_GetSrcNoteOffset(sn, 1);

                /*
                 * Count the cases using offsets from switch to first case,
                 * and case to case, stored in srcnote immediates.
                 */
                pc2 = pc;
                off2 = off;
                for (ncases = 0; off2 != 0; ncases++) {
                    pc2 += off2;
                    JS_ASSERT(*pc2 == JSOP_CASE || *pc2 == JSOP_DEFAULT ||
                              *pc2 == JSOP_CASEX || *pc2 == JSOP_DEFAULTX);
                    if (*pc2 == JSOP_DEFAULT || *pc2 == JSOP_DEFAULTX) {
                        /* End of cases, but count default as a case. */
                        off2 = 0;
                    } else {
                        sn = js_GetSrcNote(jp->script, pc2);
                        JS_ASSERT(sn && SN_TYPE(sn) == SRC_PCDELTA);
                        off2 = js_GetSrcNoteOffset(sn, 0);
                    }
                }

                /*
                 * Allocate table and rescan the cases using their srcnotes,
                 * stashing each case's delta from switch top in table[i].key,
                 * and the distance to its statements in table[i].offset.
                 */
                table = (TableEntry *)
                    JS_malloc(cx, (size_t)ncases * sizeof *table);
                if (!table)
                    return JS_FALSE;
                pc2 = pc;
                off2 = off;
                for (i = 0; i < ncases; i++) {
                    pc2 += off2;
                    JS_ASSERT(*pc2 == JSOP_CASE || *pc2 == JSOP_DEFAULT ||
                              *pc2 == JSOP_CASEX || *pc2 == JSOP_DEFAULTX);
                    caseOff = pc2 - pc;
                    table[i].key = INT_TO_JSVAL((jsint) caseOff);
                    table[i].offset = caseOff + GetJumpOffset(pc2, pc2);
                    if (*pc2 == JSOP_CASE || *pc2 == JSOP_CASEX) {
                        sn = js_GetSrcNote(jp->script, pc2);
                        JS_ASSERT(sn && SN_TYPE(sn) == SRC_PCDELTA);
                        off2 = js_GetSrcNoteOffset(sn, 0);
                    }
                }

                /*
                 * Find offset of default code by fetching the default offset
                 * from the end of table.  JSOP_CONDSWITCH always has a default
                 * case at the end.
                 */
                off = JSVAL_TO_INT(table[ncases-1].key);
                pc2 = pc + off;
                off += GetJumpOffset(pc2, pc2);

                ok = DecompileSwitch(ss, table, (uintN)ncases, pc, len, off,
                                     JS_TRUE);
                JS_free(cx, table);
                if (!ok)
                    return ok;
                todo = -2;
                break;
              }

              case JSOP_CASE:
              case JSOP_CASEX:
              {
                lval = POP_STR();
                if (!lval)
                    return JS_FALSE;
                js_printf(jp, "\tcase %s:\n", lval);
                todo = -2;
                break;
              }

              case JSOP_NEW_EQ:
              case JSOP_NEW_NE:
                rval = POP_STR();
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, "%s %c== %s",
                              lval, (op == JSOP_NEW_EQ) ? '=' : '!', rval);
                break;

              BEGIN_LITOPX_CASE(JSOP_CLOSURE)
                JS_ASSERT(ATOM_IS_OBJECT(atom));
                todo = -2;
                goto do_function;
              END_LITOPX_CASE

#if JS_HAS_EXPORT_IMPORT
              case JSOP_EXPORTALL:
                js_printf(jp, "\texport *;\n");
                todo = -2;
                break;

              BEGIN_LITOPX_CASE(JSOP_EXPORTNAME)
                rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                if (!rval)
                    return JS_FALSE;
                RETRACT(&ss->sprinter, rval);
                js_printf(jp, "\texport %s;\n", rval);
                todo = -2;
              END_LITOPX_CASE

              case JSOP_IMPORTALL:
                lval = POP_STR();
                js_printf(jp, "\timport %s.*;\n", lval);
                todo = -2;
                break;

              case JSOP_IMPORTPROP:
              do_importprop:
                GET_ATOM_QUOTE_AND_FMT("\timport %s[%s];\n",
                                       "\timport %s.%s;\n",
                                       rval);
                lval = POP_STR();
                js_printf(jp, fmt, lval, rval);
                todo = -2;
                break;

              case JSOP_IMPORTELEM:
                xval = POP_STR();
                op = JSOP_GETELEM;
                if (js_CodeSpec[lastop].format & JOF_XMLNAME)
                    goto do_importprop;
                lval = POP_STR();
                js_printf(jp, "\timport %s[%s];\n", lval, xval);
                todo = -2;
                break;
#endif /* JS_HAS_EXPORT_IMPORT */

              case JSOP_TRAP:
                op = JS_GetTrapOpcode(cx, jp->script, pc);
                if (op == JSOP_LIMIT)
                    return JS_FALSE;
                saveop = op;
                *pc = op;
                cs = &js_CodeSpec[op];
                len = cs->length;
                DECOMPILE_CODE(pc, len);
                *pc = JSOP_TRAP;
                todo = -2;
                break;

              case JSOP_NEWINIT:
                LOCAL_ASSERT(ss->top >= 2);
                (void) PopOff(ss, op);
                lval = POP_STR();
                todo = ss->sprinter.offset;
#if JS_HAS_SHARP_VARS
                op = (JSOp)pc[len];
                if (op == JSOP_DEFSHARP) {
                    pc += len;
                    cs = &js_CodeSpec[op];
                    len = cs->length;
                    i = (jsint) GET_ATOM_INDEX(pc);
                    if (Sprint(&ss->sprinter, "#%u=", (unsigned) i) < 0)
                        return JS_FALSE;
                }
#endif /* JS_HAS_SHARP_VARS */
                if (*lval == 'A') {
                    ++ss->inArrayInit;
                    if (SprintCString(&ss->sprinter, "[") < 0)
                        return JS_FALSE;
                } else {
                    if (SprintCString(&ss->sprinter, "{") < 0)
                        return JS_FALSE;
                }
                break;

              case JSOP_ENDINIT:
                op = JSOP_NOP;           /* turn off parens */
                rval = POP_STR();
                sn = js_GetSrcNote(jp->script, pc);
                if (*rval == '[')
                    --ss->inArrayInit;
                todo = Sprint(&ss->sprinter, "%s%s%c",
                              rval,
                              (sn && SN_TYPE(sn) == SRC_CONTINUE) ? ", " : "",
                              (*rval == '[') ? ']' : '}');
                break;

              case JSOP_INITPROP:
                atom = GET_ATOM(cx, jp->script, pc);
                xval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom),
                                   (jschar)
                                   (ATOM_IS_IDENTIFIER(atom) ? 0 : '\''));
                if (!xval)
                    return JS_FALSE;
                rval = POP_STR();
                lval = POP_STR();
              do_initprop:
#ifdef OLD_GETTER_SETTER
                todo = Sprint(&ss->sprinter, "%s%s%s%s%s:%s",
                              lval,
                              (lval[1] != '\0') ? ", " : "",
                              xval,
                              (lastop == JSOP_GETTER || lastop == JSOP_SETTER)
                              ? " " : "",
                              (lastop == JSOP_GETTER) ? js_getter_str :
                              (lastop == JSOP_SETTER) ? js_setter_str :
                              "",
                              rval);
#else
                if (lastop == JSOP_GETTER || lastop == JSOP_SETTER) {
                    JS_ASSERT(strncmp(rval, js_function_str, 8) == 0 &&
                              rval[8] == ' ');
                    rval += 8 + 1;
                    JS_ASSERT(rval[strlen(rval)-1] == '}');
                    todo = Sprint(&ss->sprinter, "%s%s%s %s%s",
                                  lval,
                                  (lval[1] != '\0') ? ", " : "",
                                  (lastop == JSOP_GETTER)
                                  ? js_get_str : js_set_str,
                                  xval,
                                  rval);
                } else {
                    todo = Sprint(&ss->sprinter, "%s%s%s:%s",
                                  lval,
                                  (lval[1] != '\0') ? ", " : "",
                                  xval,
                                  rval);
                }
#endif
                break;

              case JSOP_INITELEM:
                rval = POP_STR();
                xval = POP_STR();
                lval = POP_STR();
                sn = js_GetSrcNote(jp->script, pc);
                if (sn && SN_TYPE(sn) == SRC_INITPROP)
                    goto do_initprop;
                todo = Sprint(&ss->sprinter, "%s%s%s",
                              lval,
                              (lval[1] != '\0' || *xval != '0') ? ", " : "",
                              rval);
                break;

#if JS_HAS_SHARP_VARS
              case JSOP_DEFSHARP:
                i = (jsint) GET_ATOM_INDEX(pc);
                rval = POP_STR();
                todo = Sprint(&ss->sprinter, "#%u=%s", (unsigned) i, rval);
                break;

              case JSOP_USESHARP:
                i = (jsint) GET_ATOM_INDEX(pc);
                todo = Sprint(&ss->sprinter, "#%u#", (unsigned) i);
                break;
#endif /* JS_HAS_SHARP_VARS */

#if JS_HAS_DEBUGGER_KEYWORD
              case JSOP_DEBUGGER:
                js_printf(jp, "\tdebugger;\n");
                todo = -2;
                break;
#endif /* JS_HAS_DEBUGGER_KEYWORD */

#if JS_HAS_XML_SUPPORT
              case JSOP_STARTXML:
              case JSOP_STARTXMLEXPR:
                inXML = op == JSOP_STARTXML;
                todo = -2;
                break;

              case JSOP_DEFXMLNS:
                rval = POP_STR();
                js_printf(jp, "\t%s %s %s = %s;\n",
                          js_default_str, js_xml_str, js_namespace_str, rval);
                todo = -2;
                break;

              case JSOP_ANYNAME:
                if (pc[JSOP_ANYNAME_LENGTH] == JSOP_TOATTRNAME) {
                    len += JSOP_TOATTRNAME_LENGTH;
                    todo = SprintPut(&ss->sprinter, "@*", 2);
                } else {
                    todo = SprintPut(&ss->sprinter, "*", 1);
                }
                break;

              BEGIN_LITOPX_CASE(JSOP_QNAMEPART)
                if (pc[JSOP_QNAMEPART_LENGTH] == JSOP_TOATTRNAME) {
                    saveop = JSOP_TOATTRNAME;
                    len += JSOP_TOATTRNAME_LENGTH;
                    lval = "@";
                    goto do_qname;
                }
                goto do_name;
              END_LITOPX_CASE

              BEGIN_LITOPX_CASE(JSOP_QNAMECONST)
                rval = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0);
                if (!rval)
                    return JS_FALSE;
                RETRACT(&ss->sprinter, rval);
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, "%s::%s", lval, rval);
              END_LITOPX_CASE

              case JSOP_QNAME:
                rval = POP_STR();
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, "%s::[%s]", lval, rval);
                break;

              case JSOP_TOATTRNAME:
                op = JSOP_NOP;           /* turn off parens */
                rval = POP_STR();
                todo = Sprint(&ss->sprinter, "@[%s]", rval);
                break;

              case JSOP_TOATTRVAL:
                todo = -2;
                break;

              case JSOP_ADDATTRNAME:
                rval = POP_STR();
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, "%s %s", lval, rval);
                /* This gets reset by all XML tag expressions. */
                quoteAttr = JS_TRUE;
                break;

              case JSOP_ADDATTRVAL:
                rval = POP_STR();
                lval = POP_STR();
                if (quoteAttr)
                    todo = Sprint(&ss->sprinter, "%s=\"%s\"", lval, rval);
                else
                    todo = Sprint(&ss->sprinter, "%s=%s", lval, rval);
                break;

              case JSOP_BINDXMLNAME:
                /* Leave the name stacked and push a dummy string. */
                todo = Sprint(&ss->sprinter, "");
                break;

              case JSOP_SETXMLNAME:
                /* Pop the r.h.s., the dummy string, and the name. */
                rval = POP_STR();
                (void) PopOff(ss, op);
                lval = POP_STR();
                goto do_setlval;

              case JSOP_XMLELTEXPR:
              case JSOP_XMLTAGEXPR:
                todo = Sprint(&ss->sprinter, "{%s}", POP_STR());
                inXML = JS_TRUE;
                /* If we're an attribute value, we shouldn't quote this. */
                quoteAttr = JS_FALSE;
                break;

              case JSOP_TOXMLLIST:
                op = JSOP_NOP;           /* turn off parens */
                todo = Sprint(&ss->sprinter, "<>%s</>", POP_STR());
                inXML = JS_FALSE;
                break;

              case JSOP_FOREACH:
                foreach = JS_TRUE;
                todo = -2;
                break;

              case JSOP_TOXML:
                inXML = JS_FALSE;
                /* FALL THROUGH */

              case JSOP_XMLNAME:
              case JSOP_FILTER:
                /* Conversion and prefix ops do nothing in the decompiler. */
                todo = -2;
                break;

              case JSOP_ENDFILTER:
                rval = POP_STR();
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, "%s.(%s)", lval, rval);
                break;

              case JSOP_DESCENDANTS:
                rval = POP_STR();
                lval = POP_STR();
                todo = Sprint(&ss->sprinter, "%s..%s", lval, rval);
                break;

              BEGIN_LITOPX_CASE(JSOP_XMLOBJECT)
                atom = GET_ATOM(cx, jp->script, pc);
                todo = Sprint(&ss->sprinter, "<xml address='%p'>",
                              ATOM_TO_OBJECT(atom));
              END_LITOPX_CASE

              BEGIN_LITOPX_CASE(JSOP_XMLCDATA)
                todo = SprintPut(&ss->sprinter, "<![CDATA[", 9);
                if (!QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0))
                    return JS_FALSE;
                SprintPut(&ss->sprinter, "]]>", 3);
              END_LITOPX_CASE

              BEGIN_LITOPX_CASE(JSOP_XMLCOMMENT)
                todo = SprintPut(&ss->sprinter, "<!--", 4);
                if (!QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0))
                    return JS_FALSE;
                SprintPut(&ss->sprinter, "-->", 3);
              END_LITOPX_CASE

              BEGIN_LITOPX_CASE(JSOP_XMLPI)
                rval = JS_strdup(cx, POP_STR());
                if (!rval)
                    return JS_FALSE;
                todo = SprintPut(&ss->sprinter, "<?", 2);
                ok = QuoteString(&ss->sprinter, ATOM_TO_STRING(atom), 0) &&
                     (*rval == '\0' ||
                      (SprintPut(&ss->sprinter, " ", 1) >= 0 &&
                       SprintCString(&ss->sprinter, rval)));
                JS_free(cx, (char *)rval);
                if (!ok)
                    return JS_FALSE;
                SprintPut(&ss->sprinter, "?>", 2);
              END_LITOPX_CASE

              case JSOP_GETFUNNS:
                todo = SprintPut(&ss->sprinter, js_function_str, 8);
                break;
#endif /* JS_HAS_XML_SUPPORT */

              default:
                todo = -2;
                break;

#undef BEGIN_LITOPX_CASE
#undef END_LITOPX_CASE
            }
        }

        if (todo < 0) {
            /* -2 means "don't push", -1 means reported error. */
            if (todo == -1)
                return JS_FALSE;
        } else {
            if (!PushOff(ss, todo, saveop))
                return JS_FALSE;
        }
        pc += len;
    }

/*
 * Undefine local macros.
 */
#undef inXML
#undef DECOMPILE_CODE
#undef POP_STR
#undef LOCAL_ASSERT
#undef ATOM_IS_IDENTIFIER
#undef GET_QUOTE_AND_FMT
#undef GET_ATOM_QUOTE_AND_FMT

    return JS_TRUE;
}


JSBool
js_DecompileCode(JSPrinter *jp, JSScript *script, jsbytecode *pc, uintN len)
{
    SprintStack ss;
    JSContext *cx;
    void *mark, *space;
    size_t offsetsz, opcodesz;
    JSBool ok;
    JSScript *oldscript;
    char *last;

    /* Initialize a sprinter for use with the offset stack. */
    ss.printer = jp;
    cx = jp->sprinter.context;
    mark = JS_ARENA_MARK(&cx->tempPool);
    INIT_SPRINTER(cx, &ss.sprinter, &cx->tempPool, PAREN_SLOP);

    /* Allocate the parallel (to avoid padding) offset and opcode stacks. */
    offsetsz = script->depth * sizeof(ptrdiff_t);
    opcodesz = script->depth * sizeof(jsbytecode);
    JS_ARENA_ALLOCATE(space, &cx->tempPool, offsetsz + opcodesz);
    if (!space) {
        ok = JS_FALSE;
        goto out;
    }
    ss.offsets = (ptrdiff_t *) space;
    ss.opcodes = (jsbytecode *) ((char *)space + offsetsz);
    ss.top = ss.inArrayInit = 0;

    /* Call recursive subroutine to do the hard work. */
    oldscript = jp->script;
    jp->script = script;
    ok = Decompile(&ss, pc, len);
    jp->script = oldscript;

    /* If the given code didn't empty the stack, do it now. */
    if (ss.top) {
        do {
            last = OFF2STR(&ss.sprinter, PopOff(&ss, JSOP_NOP));
        } while (ss.top);
        js_printf(jp, "%s", last);
    }

out:
    /* Free all temporary stuff allocated under this call. */
    JS_ARENA_RELEASE(&cx->tempPool, mark);
    return ok;
}

JSBool
js_DecompileScript(JSPrinter *jp, JSScript *script)
{
    return js_DecompileCode(jp, script, script->code, (uintN)script->length);
}

static const char native_code_str[] = "\t[native code]\n";

JSBool
js_DecompileFunctionBody(JSPrinter *jp, JSFunction *fun)
{
    JSScript *script;
    JSScope *scope, *save;
    JSBool ok;

    if (!FUN_INTERPRETED(fun)) {
        js_printf(jp, native_code_str);
        return JS_TRUE;
    }
    script = fun->u.i.script;
    scope = fun->object ? OBJ_SCOPE(fun->object) : NULL;
    save = jp->scope;
    jp->scope = scope;
    ok = js_DecompileCode(jp, script, script->code, (uintN)script->length);
    jp->scope = save;
    return ok;
}

JSBool
js_DecompileFunction(JSPrinter *jp, JSFunction *fun)
{
    JSContext *cx;
    uintN i, nargs, indent;
    void *mark;
    JSAtom **params;
    JSScope *scope, *oldscope;
    JSScopeProperty *sprop;
    JSBool ok;

    /*
     * If pretty, conform to ECMA-262 Edition 3, 15.3.4.2, by decompiling a
     * FunctionDeclaration.  Otherwise, check the JSFUN_LAMBDA flag and force
     * an expression by parenthesizing.
     */
    if (jp->pretty) {
        js_printf(jp, "\t");
    } else {
        if (!jp->grouped && (fun->flags & JSFUN_LAMBDA))
            js_puts(jp, "(");
    }
    if (JSFUN_GETTER_TEST(fun->flags))
        js_printf(jp, "%s ", js_getter_str);
    else if (JSFUN_SETTER_TEST(fun->flags))
        js_printf(jp, "%s ", js_setter_str);

    js_printf(jp, "%s ", js_function_str);
    if (fun->atom && !QuoteString(&jp->sprinter, ATOM_TO_STRING(fun->atom), 0))
        return JS_FALSE;
    js_puts(jp, "(");

    if (FUN_INTERPRETED(fun) && fun->object) {
        size_t paramsize;

        /*
         * Print the parameters.
         *
         * This code is complicated by the need to handle duplicate parameter
         * names, as required by ECMA (bah!).  A duplicate parameter is stored
         * as another node with the same id (the parameter name) but different
         * shortid (the argument index) along the property tree ancestor line
         * starting at SCOPE_LAST_PROP(scope).  Only the last duplicate param
         * is mapped by the scope's hash table.
         */
        cx = jp->sprinter.context;
        nargs = fun->nargs;
        mark = JS_ARENA_MARK(&cx->tempPool);
        paramsize = nargs * sizeof(JSAtom *);
        JS_ARENA_ALLOCATE_CAST(params, JSAtom **, &cx->tempPool, paramsize);
        if (!params) {
            JS_ReportOutOfMemory(cx);
            return JS_FALSE;
        }
        memset(params, 0, paramsize);
        scope = OBJ_SCOPE(fun->object);
        for (sprop = SCOPE_LAST_PROP(scope); sprop; sprop = sprop->parent) {
            if (sprop->getter != js_GetArgument)
                continue;
            JS_ASSERT(sprop->flags & SPROP_HAS_SHORTID);
            JS_ASSERT((uint16) sprop->shortid < nargs);
            JS_ASSERT(JSID_IS_ATOM(sprop->id));
            params[(uint16) sprop->shortid] = JSID_TO_ATOM(sprop->id);
        }
        for (i = 0; i < nargs; i++) {
            if (i > 0)
                js_puts(jp, ", ");
            if (!QuoteString(&jp->sprinter, ATOM_TO_STRING(params[i]), 0))
                return JS_FALSE;
        }
        JS_ARENA_RELEASE(&cx->tempPool, mark);
#ifdef __GNUC__
    } else {
        scope = NULL;
#endif
    }

    js_printf(jp, ") {\n");
    indent = jp->indent;
    jp->indent += 4;
    if (FUN_INTERPRETED(fun) && fun->object) {
        oldscope = jp->scope;
        jp->scope = scope;
        ok = js_DecompileScript(jp, fun->u.i.script);
        jp->scope = oldscope;
        if (!ok) {
            jp->indent = indent;
            return JS_FALSE;
        }
    } else {
        js_printf(jp, native_code_str);
    }
    jp->indent -= 4;
    js_printf(jp, "\t}");

    if (!jp->pretty) {
        if (!jp->grouped && (fun->flags & JSFUN_LAMBDA))
            js_puts(jp, ")");
    }
    return JS_TRUE;
}

JSString *
js_DecompileValueGenerator(JSContext *cx, intN spindex, jsval v,
                           JSString *fallback)
{
    JSStackFrame *fp, *down;
    jsbytecode *pc, *begin, *end, *tmp;
    jsval *sp, *base, *limit;
    JSScript *script;
    JSOp op;
    const JSCodeSpec *cs;
    uint32 format, mode, type;
    intN depth;
    jssrcnote *sn;
    uintN len, off;
    JSPrinter *jp;
    JSString *name;

    for (fp = cx->fp; fp && !fp->script; fp = fp->down)
        continue;
    if (!fp)
        goto do_fallback;

    /* Try to find sp's generating pc depth slots under it on the stack. */
    pc = fp->pc;
    if (spindex == JSDVG_SEARCH_STACK) {
        if (!pc) {
            /*
             * Current frame is native: look under it for a scripted call
             * in which a decompilable bytecode string that generated the
             * value as an actual argument might exist.
             */
            JS_ASSERT(!fp->script && !(fp->fun && FUN_INTERPRETED(fp->fun)));
            down = fp->down;
            if (!down)
                goto do_fallback;
            script = down->script;
            base = fp->argv;
            limit = base + fp->argc;
        } else {
            /*
             * This should be a script activation, either a top-level
             * script or a scripted function.  But be paranoid about calls
             * to js_DecompileValueGenerator from code that hasn't fully
             * initialized a (default-all-zeroes) frame.
             */
            script = fp->script;
            base = fp->spbase;
            limit = fp->sp;
        }

        /*
         * Pure paranoia about default-zeroed frames being active while
         * js_DecompileValueGenerator is called.  It can't hurt much now;
         * error reporting performance is not an issue.
         */
        if (!script || !base || !limit)
            goto do_fallback;

        /*
         * Try to find operand-generating pc depth slots below sp.
         *
         * In the native case, we know the arguments have generating pc's
         * under them, on account of fp->down->script being non-null: all
         * compiled scripts get depth slots for generating pc's allocated
         * upon activation, at the top of js_Interpret.
         *
         * In the script or scripted function case, the same reasoning
         * applies to fp rather than to fp->down.
         *
         * We search from limit to base to find the most recently calculated
         * value matching v under assumption that it is it that caused
         * exception, see bug 328664.
         */
        for (sp = limit; sp > base; ) {
            --sp;
            if (*sp == v) {
                depth = (intN)script->depth;
                pc = (jsbytecode *) sp[-depth];
                break;
            }
        }
    } else {
        /*
         * At this point, pc may or may not be null, i.e., we could be in
         * a script activation, or we could be in a native frame that was
         * called by another native function.  Check pc and script.
         */
        if (!pc)
            goto do_fallback;
        script = fp->script;
        if (!script)
            goto do_fallback;

        if (spindex != JSDVG_IGNORE_STACK) {
            JS_ASSERT(spindex < 0);
            depth = (intN)script->depth;
#if !JS_HAS_NO_SUCH_METHOD
            JS_ASSERT(-depth <= spindex);
#endif
            spindex -= depth;

            base = (jsval *) cx->stackPool.current->base;
            limit = (jsval *) cx->stackPool.current->avail;
            sp = fp->sp + spindex;
            if (JS_UPTRDIFF(sp, base) < JS_UPTRDIFF(limit, base))
                pc = (jsbytecode *) *sp;
        }
    }

    /*
     * Again, be paranoid, this time about possibly loading an invalid pc
     * from sp[-(1+depth)].
     */
    if (JS_UPTRDIFF(pc, script->code) >= (jsuword)script->length) {
        pc = fp->pc;
        if (!pc)
            goto do_fallback;
    }
    op = (JSOp) *pc;
    if (op == JSOP_TRAP)
        op = JS_GetTrapOpcode(cx, script, pc);

    /* XXX handle null as a special case, to avoid calling null "object" */
    if (op == JSOP_NULL)
        return ATOM_TO_STRING(cx->runtime->atomState.nullAtom);

    cs = &js_CodeSpec[op];
    format = cs->format;
    mode = (format & JOF_MODEMASK);

    /* NAME ops are self-contained, but others require left context. */
    if (mode == JOF_NAME) {
        begin = pc;
    } else {
        sn = js_GetSrcNote(script, pc);
        if (!sn || (SN_TYPE(sn) != SRC_PCBASE && SN_TYPE(sn) != SRC_PCDELTA)) {
            if (cs->token)
                return JS_NewStringCopyZ(cx, cs->token);
            goto do_fallback;
        }
        begin = pc - js_GetSrcNoteOffset(sn, 0);
    }
    end = pc + cs->length;
    len = PTRDIFF(end, begin, jsbytecode);

    if (format & (JOF_SET | JOF_DEL | JOF_INCDEC | JOF_IMPORT | JOF_FOR)) {
        tmp = (jsbytecode *) JS_malloc(cx, len * sizeof(jsbytecode));
        if (!tmp)
            return NULL;
        memcpy(tmp, begin, len * sizeof(jsbytecode));
        if (mode == JOF_NAME) {
            /*
             * JOF_NAME does not imply JOF_CONST, so we must check for the
             * QARG and QVAR format types and translate those to JSOP_GETARG
             * or JSOP_GETVAR appropriately, instead of JSOP_NAME.
             */
            type = format & JOF_TYPEMASK;
            tmp[0] = (type == JOF_QARG)
                     ? JSOP_GETARG
                     : (type == JOF_QVAR)
                     ? JSOP_GETVAR
                     : JSOP_NAME;
        } else {
            /*
             * We must replace the faulting pc's bytecode with a corresponding
             * JSOP_GET* code.  For JSOP_SET{PROP,ELEM}, we must use the "2nd"
             * form of JSOP_GET{PROP,ELEM}, to throw away the assignment op's
             * right-hand operand and decompile it as if it were a GET of its
             * left-hand operand.
             */
            off = len - cs->length;
            JS_ASSERT(off == (uintN) PTRDIFF(pc, begin, jsbytecode));
            if (mode == JOF_PROP) {
                tmp[off] = (format & JOF_SET) ? JSOP_GETPROP2 : JSOP_GETPROP;
            } else if (mode == JOF_ELEM) {
                tmp[off] = (format & JOF_SET) ? JSOP_GETELEM2 : JSOP_GETELEM;
            } else {
                /*
                 * A zero mode means precisely that op is uncategorized for our
                 * purposes, so we must write per-op special case code here.
                 */
                switch (op) {
                  case JSOP_ENUMELEM:
                    tmp[off] = JSOP_GETELEM;
                    break;
#if JS_HAS_LVALUE_RETURN
                  case JSOP_SETCALL:
                    tmp[off] = JSOP_CALL;
                    break;
#endif
                  default:
                    JS_ASSERT(0);
                }
            }
        }
        begin = tmp;
    } else {
        /* No need to revise script bytecode. */
        tmp = NULL;
    }

    name = NULL;
    jp = js_NewPrinter(cx, "js_DecompileValueGenerator", 0, JS_FALSE);
    if (jp) {
        if (fp->fun && fp->fun->object) {
            JS_ASSERT(OBJ_IS_NATIVE(fp->fun->object));
            jp->scope = OBJ_SCOPE(fp->fun->object);
        }
        if (js_DecompileCode(jp, script, begin, len))
            name = js_GetPrinterOutput(jp);
        js_DestroyPrinter(jp);
    }
    if (tmp)
        JS_free(cx, tmp);
    return name;

  do_fallback:
    return fallback ? fallback : js_ValueToSource(cx, v);
}
