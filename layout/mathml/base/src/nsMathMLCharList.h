/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla MathML Project.
 *
 * The Initial Developer of the Original Code is The University Of
 * Queensland.  Portions created by The University Of Queensland are
 * Copyright (C) 1999 The University Of Queensland.  All Rights Reserved.
 *
 * Contributor(s):
 *   Roger B. Sidje <rbs@maths.uq.edu.au>
 *   Shyjan Mahamud <mahamud@cs.cmu.edu>
 */

/******

  This file contains the list of all stretchy MathML operators.

  It is designed to be used as inline input to nsMathMLChar.cpp *only*
  through the magic of C preprocessing.

  All entries must be enclosed in the macro MATHML_CHAR which will have cruel
  and unusual things done to them.

  How it works
  ============
  
  * Stretchy chars that come from the same font are grouped together.
    For example, stretchy chars that can be build with the Symbol font are enclosed in:
    #if defined(WANT_SYMBOL_DATA)  
      ...
    #endif
     
  * To add support for a <b>new stretchy char within a font</b>, you should 
    add its MATHML_CHAR entry in the appropriate list. Each relevant row looks like:

    MATHML_CHAR(index, enum, top _ middle _ bottom _ glue, size0 _ ... _ size{N-1})

    o The second argument of MATHML_CHAR will be converted in an 'enum' of the form,
      e.g., eMathMLChar_LeftParenthesis, which will be used in the code. 

    o The first argument 'index' represents the previous index augmented with:
      1 + number of glyphs listed in the previous row. In general therefore,
      the k+1st index after the k-th index must satisfy the relationship:

         index[k+1] = index[k] + 5 + N. 

      where:
         5 represents the 4 partial glyphs: top (or left), middle, bottom (or right), glue,
                      *plus* the null separator that is inserted after the list of glyphs!
         N represents the total number of bigger sizes: size0, ..., size{N-1}.

  * To add support for a <b>new font</b>, you should create the list of stretchy chars
    that can be rendered with that font, and then declare an instance of nsGlyphTable
    associated to that font in nsMathMLChar.cpp. Then update the font-family list
    of :-moz-math-font-style-stretchy in mathml.css to instruct the Style System to
    pass your font in your preferred order.

  * There can be two types of data: either with Unicode points, or with direct
    glyph indices within the fonts. XXX The second variant is not yet supported.
 
 ******/

#if defined(WANT_CHAR_ENUM)
   #define MATHML_CHAR(_unicode, _enum, _direction) eMathMLChar_##_enum,

#elif defined(WANT_CHAR_INFO)
   #define MATHML_CHAR(_unicode, _enum, _direction) {_unicode, _direction, nsnull},

#elif defined(WANT_CHAR_DATA)
   #define MATHML_CHAR(_index, _enum, _parts, _sizes) {eMathMLChar_##_enum, _index},

#elif defined(WANT_GLYPH_DATA)
   #define MATHML_CHAR(_index, _enum, _parts, _sizes) _parts, _sizes, 0x0000,

#endif


#if defined(WANT_CHAR_ENUM) || defined(WANT_CHAR_INFO)

// short names for a short while...
#define STRETCH_UNSUPPORTED NS_STRETCH_DIRECTION_UNSUPPORTED
#define STRETCH_HORIZONTAL  NS_STRETCH_DIRECTION_HORIZONTAL
#define STRETCH_VERTICAL    NS_STRETCH_DIRECTION_VERTICAL

// List of all stretchy chars --------------------------------------------------------
// Includes extra stretchy operators that are not in the MathML REC Operator Dictionary
// The list is *sorted* so that we can use binary search for quick lookup.
// For stretching to work, chars must also be added in nsMathMLOperators.
// Duplicate stretchy chars (i.e., with same Unicode points) that are in the
// MathML REC Operator Dictionary shouldn't be added to the list, otherwise
// they get different enums and these play havoc with the setup
// -----------------------------------------------------------------------------------

MATHML_CHAR(0x0028, LeftParenthesis,                 STRETCH_VERTICAL)
MATHML_CHAR(0x0029, RightParenthesis,                STRETCH_VERTICAL)
MATHML_CHAR(0x002F, Slash,                           STRETCH_VERTICAL)
MATHML_CHAR(0x005B, LeftSquareBracket,               STRETCH_VERTICAL)
MATHML_CHAR(0x005D, RightSquareBracket,              STRETCH_VERTICAL)
MATHML_CHAR(0x007B, LeftCurlyBracket,                STRETCH_VERTICAL)
MATHML_CHAR(0x007C, VertBar,                         STRETCH_VERTICAL) // '|'
MATHML_CHAR(0x007D, RightCurlyBracket,               STRETCH_VERTICAL)
MATHML_CHAR(0x00AF, OverBar,                         STRETCH_HORIZONTAL)
MATHML_CHAR(0x02C7, Hacek,                           STRETCH_HORIZONTAL)
MATHML_CHAR(0x02DC, DiacriticalTilde,                STRETCH_HORIZONTAL)
MATHML_CHAR(0x0302, Hat,                             STRETCH_HORIZONTAL)
MATHML_CHAR(0x0332, UnderBar,                        STRETCH_HORIZONTAL)
MATHML_CHAR(0x20D0, LeftHarpoonAccent,               STRETCH_HORIZONTAL)
MATHML_CHAR(0x20D1, RightHarpoonAccent,              STRETCH_HORIZONTAL)         
MATHML_CHAR(0x20D6, LeftArrowAccent,                 STRETCH_HORIZONTAL)
MATHML_CHAR(0x20D7, RightArrowAccent,                STRETCH_HORIZONTAL)
MATHML_CHAR(0x20E1, LeftRightArrowAccent,            STRETCH_HORIZONTAL)
MATHML_CHAR(0x2190, LeftArrow,                       STRETCH_HORIZONTAL)
MATHML_CHAR(0x2191, UpArrow,                         STRETCH_VERTICAL)
MATHML_CHAR(0x2192, RightArrow,                      STRETCH_HORIZONTAL)
MATHML_CHAR(0x2193, DownArrow,                       STRETCH_VERTICAL)
MATHML_CHAR(0x2194, LeftRightArrow,                  STRETCH_HORIZONTAL)
MATHML_CHAR(0x2195, UpDownArrow,                     STRETCH_VERTICAL)
MATHML_CHAR(0x2196, UpperLeftArrow,                  STRETCH_VERTICAL)
MATHML_CHAR(0x2197, UpperRightArrow,                 STRETCH_VERTICAL)
MATHML_CHAR(0x2198, LowerRightArrow,                 STRETCH_HORIZONTAL)
MATHML_CHAR(0x2199, LowerLeftArrow,                  STRETCH_HORIZONTAL)
MATHML_CHAR(0x21A4, LeftTeeArrow,                    STRETCH_HORIZONTAL)
MATHML_CHAR(0x21A5, UpTeeArrow,                      STRETCH_VERTICAL)
MATHML_CHAR(0x21A6, RightTeeArrow,                   STRETCH_HORIZONTAL)
MATHML_CHAR(0x21A7, DownTeeArrow,                    STRETCH_VERTICAL)
MATHML_CHAR(0x21BC, LeftVector,                      STRETCH_HORIZONTAL)
MATHML_CHAR(0x21BD, DownLeftVector,                  STRETCH_VERTICAL)
MATHML_CHAR(0x21BE, RightUpVector,                   STRETCH_VERTICAL)
MATHML_CHAR(0x21BF, LeftUpVector,                    STRETCH_VERTICAL)
MATHML_CHAR(0x21C0, RightVector,                     STRETCH_HORIZONTAL)
MATHML_CHAR(0x21C1, DownRightVector,                 STRETCH_VERTICAL)
MATHML_CHAR(0x21C2, RightDownVector,                 STRETCH_VERTICAL)
MATHML_CHAR(0x21C3, LeftDownVector,                  STRETCH_VERTICAL)
MATHML_CHAR(0x21C4, RightArrowLeftArrow,             STRETCH_HORIZONTAL)
MATHML_CHAR(0x21C5, UpArrowDownArrow,                STRETCH_VERTICAL)
MATHML_CHAR(0x21C6, LeftArrowRightArrow,             STRETCH_HORIZONTAL)
MATHML_CHAR(0x21CB, ReverseEquilibrium,              STRETCH_HORIZONTAL)
MATHML_CHAR(0x21CC, Equilibrium,                     STRETCH_HORIZONTAL)
MATHML_CHAR(0x21D0, DoubleLeftArrow,                 STRETCH_HORIZONTAL)
MATHML_CHAR(0x21D1, DoubleUpArrow,                   STRETCH_VERTICAL)
MATHML_CHAR(0x21D2, DoubleRightArrow,                STRETCH_HORIZONTAL)
MATHML_CHAR(0x21D3, DoubleDownArrow,                 STRETCH_VERTICAL)
MATHML_CHAR(0x21D4, DoubleLeftRightArrow,            STRETCH_HORIZONTAL)
MATHML_CHAR(0x21D5, DoubleUpDownArrow,               STRETCH_VERTICAL)
MATHML_CHAR(0x21E4, LeftArrowBar,                    STRETCH_HORIZONTAL)
MATHML_CHAR(0x21E5, RightArrowBar,                   STRETCH_HORIZONTAL)
MATHML_CHAR(0x220F, Product,                         STRETCH_VERTICAL)
MATHML_CHAR(0x2210, Coproduct,                       STRETCH_VERTICAL)
MATHML_CHAR(0x2211, Sum,                             STRETCH_VERTICAL)
MATHML_CHAR(0x2216, Backslash,                       STRETCH_VERTICAL)
MATHML_CHAR(0x221A, Sqrt,                            STRETCH_VERTICAL)
MATHML_CHAR(0x2223, VerticalBar,                     STRETCH_VERTICAL)
MATHML_CHAR(0x2225, DoubleVerticalBar,               STRETCH_VERTICAL)
MATHML_CHAR(0x2227, And,                             STRETCH_HORIZONTAL)
MATHML_CHAR(0x2228, Or,                              STRETCH_HORIZONTAL)
MATHML_CHAR(0x222B, Integral,                        STRETCH_VERTICAL)
MATHML_CHAR(0x222E, ContourIntegral,                 STRETCH_VERTICAL)
MATHML_CHAR(0x222F, DoubleContourIntegral,           STRETCH_VERTICAL)
MATHML_CHAR(0x2232, ClockwiseContourIntegral,        STRETCH_VERTICAL)
MATHML_CHAR(0x2233, CounterClockwiseContourIntegral, STRETCH_VERTICAL)
MATHML_CHAR(0x223C, Tilde,                           STRETCH_HORIZONTAL)
MATHML_CHAR(0x228E, UnionPlus,                       STRETCH_VERTICAL)
MATHML_CHAR(0x2293, SquareIntersection,              STRETCH_VERTICAL)
MATHML_CHAR(0x2294, SquareUnion,                     STRETCH_VERTICAL)
MATHML_CHAR(0x2295, CirclePlus,                      STRETCH_VERTICAL)
MATHML_CHAR(0x2297, CircleMultiply,                  STRETCH_VERTICAL)
MATHML_CHAR(0x2299, CircleDot,                       STRETCH_VERTICAL)
MATHML_CHAR(0x22C0, Wedge,                           STRETCH_VERTICAL)
MATHML_CHAR(0x22C1, Vee,                             STRETCH_VERTICAL)
MATHML_CHAR(0x22C2, Intersection,                    STRETCH_VERTICAL)
MATHML_CHAR(0x22C3, Union,                           STRETCH_VERTICAL)
MATHML_CHAR(0x2308, LeftCeiling,                     STRETCH_VERTICAL)
MATHML_CHAR(0x2309, RightCeiling,                    STRETCH_VERTICAL)
MATHML_CHAR(0x230A, LeftFloor,                       STRETCH_VERTICAL)
MATHML_CHAR(0x230B, RightFloor,                      STRETCH_VERTICAL)
MATHML_CHAR(0x3008, LeftAngleBracket,                STRETCH_VERTICAL)
MATHML_CHAR(0x3009, RightAngleBracket,               STRETCH_VERTICAL)
MATHML_CHAR(0x301A, LeftDoubleBracket,               STRETCH_VERTICAL)
MATHML_CHAR(0x301B, RightDoubleBracket,              STRETCH_VERTICAL)
MATHML_CHAR(0xE200, DoubleLongLeftArrow,             STRETCH_HORIZONTAL)
MATHML_CHAR(0xE201, LongLeftArrow,                   STRETCH_HORIZONTAL)
MATHML_CHAR(0xE202, DoubleLongLeftRightArrow,        STRETCH_HORIZONTAL)
MATHML_CHAR(0xE203, LongLeftRightArrow,              STRETCH_HORIZONTAL)
MATHML_CHAR(0xE204, DoubleLongRightArrow,            STRETCH_HORIZONTAL)
MATHML_CHAR(0xE205, LongRightArrow,                  STRETCH_HORIZONTAL)
MATHML_CHAR(0xE216, DownArrowUpArrow,                STRETCH_VERTICAL)
MATHML_CHAR(0xE217, ReverseUpEquilibrium,            STRETCH_VERTICAL)
MATHML_CHAR(0xE218, UpEquilibrium,                   STRETCH_VERTICAL)
MATHML_CHAR(0xE859, HorizontalLine,                  STRETCH_HORIZONTAL)
MATHML_CHAR(0xE85A, VerticalLine,                    STRETCH_VERTICAL)
MATHML_CHAR(0xE85C, VerticalSeparator,               STRETCH_VERTICAL)
MATHML_CHAR(0xF503, UpArrowBar,                      STRETCH_VERTICAL)
MATHML_CHAR(0xF504, DownArrowBar,                    STRETCH_VERTICAL)
MATHML_CHAR(0xF505, LeftRightVector,                 STRETCH_HORIZONTAL)
MATHML_CHAR(0xF507, LeftVectorBar,                   STRETCH_HORIZONTAL)
MATHML_CHAR(0xF508, RightVectorBar,                  STRETCH_HORIZONTAL)
MATHML_CHAR(0xF509, LeftTeeVector,                   STRETCH_HORIZONTAL)
MATHML_CHAR(0xF50A, RightTeeVector,                  STRETCH_HORIZONTAL)
MATHML_CHAR(0xF50B, DownLeftRightVector,             STRETCH_VERTICAL)
MATHML_CHAR(0xF50C, DownLeftVectorBar,               STRETCH_VERTICAL)
MATHML_CHAR(0xF50D, DownRightVectorBar,              STRETCH_VERTICAL)
MATHML_CHAR(0xF50E, DownLeftTeeVector,               STRETCH_VERTICAL)
MATHML_CHAR(0xF50F, DownRightTeeVector,              STRETCH_VERTICAL)
MATHML_CHAR(0xF510, RightUpDownVector,               STRETCH_VERTICAL)
MATHML_CHAR(0xF511, RightUpVectorBar,                STRETCH_VERTICAL)
MATHML_CHAR(0xF512, RightDownVectorBar,              STRETCH_VERTICAL)
MATHML_CHAR(0xF513, RightUpTeeVector,                STRETCH_VERTICAL)
MATHML_CHAR(0xF514, RightDownTeeVector,              STRETCH_VERTICAL)
MATHML_CHAR(0xF515, LeftUpDownVector,                STRETCH_VERTICAL)
MATHML_CHAR(0xF516, LeftUpVectorBar,                 STRETCH_VERTICAL)
MATHML_CHAR(0xF517, LeftDownVectorBar,               STRETCH_VERTICAL)
MATHML_CHAR(0xF518, LeftUpTeeVector,                 STRETCH_VERTICAL)
MATHML_CHAR(0xF519, LeftDownTeeVector,               STRETCH_VERTICAL)
MATHML_CHAR(0xF603, LeftBracketingBar,               STRETCH_VERTICAL)
MATHML_CHAR(0xF604, RightBracketingBar,              STRETCH_VERTICAL)
MATHML_CHAR(0xF605, LeftDoubleBracketingBar,         STRETCH_VERTICAL)
MATHML_CHAR(0xF606, RightDoubleBracketingBar,        STRETCH_VERTICAL)
MATHML_CHAR(0xF610, OverParenthesis,                 STRETCH_HORIZONTAL)
MATHML_CHAR(0xF611, UnderParenthesis,                STRETCH_HORIZONTAL)
MATHML_CHAR(0xF612, OverCurlyBracket,                STRETCH_HORIZONTAL)
MATHML_CHAR(0xF613, UnderCurlyBracket,               STRETCH_HORIZONTAL)
MATHML_CHAR(0xF614, OverSquareBracket,               STRETCH_HORIZONTAL)
MATHML_CHAR(0xF615, UnderSquareBracket,              STRETCH_HORIZONTAL)

#undef STRETCH_UNSUPPORTED
#undef STRETCH_HORIZONTAL
#undef STRETCH_VERTICAL

#elif defined(WANT_CHAR_DATA) || defined(WANT_GLYPH_DATA)
#define _ , // dirty trick to make the macro handle a variable number of glyphs...

// Data for stretchy chars that are supported by the Symbol font ---------------------
// -----------------------------------------------------------------------------------
   #if defined(WANT_SYMBOL_DATA)//[top/left][middle][bot/right][glue]  [size0 ... size{N-1}]
MATHML_CHAR( 0, LeftParenthesis,    0xF8EB _ 0x0000 _ 0xF8ED _ 0xF8EC, 0x0028)
MATHML_CHAR( 6, RightParenthesis,   0xF8F6 _ 0x0000 _ 0xF8F8 _ 0xF8F7, 0x0029)
MATHML_CHAR(12, Integral,           0x2320 _ 0x0000 _ 0x2321 _ 0xF8F5, 0x222B)
MATHML_CHAR(18, LeftSquareBracket,  0xF8EE _ 0xF8EF _ 0xF8F0 _ 0xF8EF, 0x005B)
MATHML_CHAR(24, RightSquareBracket, 0xF8F9 _ 0xF8FA _ 0xF8FB _ 0xF8FA, 0x005D)
MATHML_CHAR(30, LeftCurlyBracket,   0xF8F1 _ 0xF8F2 _ 0xF8F3 _ 0xF8F4, 0x007B)
MATHML_CHAR(36, RightCurlyBracket,  0xF8FC _ 0xF8FD _ 0xF8FE _ 0xF8F4, 0x007D)
MATHML_CHAR(42, UpArrow,            0x2191 _ 0x0000 _ 0x0000 _ 0xF8E6, 0x2191)
MATHML_CHAR(48, DownArrow,          0x0000 _ 0x0000 _ 0x2193 _ 0xF8E6, 0x2193)
MATHML_CHAR(54, UpDownArrow,        0x2191 _ 0x0000 _ 0x2193 _ 0xF8E6, 0x2195)
MATHML_CHAR(60, LeftArrow,          0x2190 _ 0x0000 _ 0x0000 _ 0xF8E7, 0x2190)
MATHML_CHAR(66, RightArrow,         0x0000 _ 0x0000 _ 0x2192 _ 0xF8E7, 0x2192)
MATHML_CHAR(72, LeftRightArrow,     0x2190 _ 0x0000 _ 0x2192 _ 0xF8E7, 0x2194)
MATHML_CHAR(78, OverBar,            0x0000 _ 0x0000 _ 0x0000 _ 0x00AF, 0x00AF)
MATHML_CHAR(84, VertBar,            0x0000 _ 0x0000 _ 0x0000 _ 0x007C, 0x007C)
   #endif // defined(WANT_SYMBOL_DATA)

// Data for stretchy chars that are supported by the MT Extra font -------------------
// -----------------------------------------------------------------------------------
   #if defined(WANT_MTEXTRA_DATA)//[top/left][middle][bot/right][glue] [size0 ... size{N-1}]
MATHML_CHAR( 0, UnderCurlyBracket,   0xEC00 _ 0xEC01 _ 0xEC02 _ 0xEC03, 0xF613)
MATHML_CHAR( 6, OverCurlyBracket,    0xEC04 _ 0xEC05 _ 0xEC06 _ 0xEC03, 0xF612)
//disable for now as they appear too small and don't align properly at default font-size 
//MATHML_CHAR(12, LeftArrowAccent,     0x20D6 _ 0x0000 _ 0x0000 _ 0xEB00, 0x20D6)
//MATHML_CHAR(18, RightArrowAccent,    0x0000 _ 0x0000 _ 0x20D7 _ 0xEB00, 0x20D7)
//MATHML_CHAR(24, LeftRightArrowAccent,0x20D6 _ 0x0000 _ 0x20D7 _ 0xEB00, 0x20E1)
//MATHML_CHAR(30, LeftHarpoonAccent,   0x20D0 _ 0x0000 _ 0x0000 _ 0xEB00, 0x20D0)
//MATHML_CHAR(36, RightHarpoonAccent,  0x0000 _ 0x0000 _ 0x20D1 _ 0xEB00, 0x20D1)
   #endif // defined(WANT_MTEXTRA_DATA)

// Data for stretchy chars that are supported by TeX's CMSY font ---------------------
// -----------------------------------------------------------------------------------

   #if defined(WANT_CMSY_DATA)   //[top/left][middle][bot/right][glue] [size0 ... size{N-1}]
MATHML_CHAR( 0, Sqrt,                0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x221A)
MATHML_CHAR( 6, DoubleLeftArrow,     0x21D0 _ 0x0000 _ 0x0000 _    '=', 0x21D0)
MATHML_CHAR(12, DoubleRightArrow,    0x0000 _ 0x0000 _ 0x21D2 _    '=', 0x21D2)
MATHML_CHAR(18, DoubleLeftRightArrow,0x21D0 _ 0x0000 _ 0x21D2 _    '=', 0x21D4)
MATHML_CHAR(24, DoubleLongLeftArrow, 0x21D0 _ 0x0000 _ 0x0000 _    '=', 0xE200)
MATHML_CHAR(30, DoubleLongRightArrow,0x0000 _ 0x0000 _ 0x21D2 _    '=', 0xE204)
MATHML_CHAR(36, DoubleLongLeftRightArrow, 0x21D0 _ 0x0000 _ 0x21D2 _    '=', 0xE202)
MATHML_CHAR(42, VerticalBar,        0x0000 _ 0x0000 _ 0x0000 _ 0x2223, 0x2223)
MATHML_CHAR(48, DoubleVerticalBar,  0x0000 _ 0x0000 _ 0x0000 _ 0x2225, 0x2225)
   #endif // defined(WANT_CMSY_DATA)

// Data for stretchy chars that are supported by TeX's CMEX font ---------------------
// -----------------------------------------------------------------------------------
   #if defined(WANT_CMEX_DATA)  // [top/left][middle][bot/right][glue]  [size0 ... size{N-1}]
MATHML_CHAR(  0, LeftParenthesis,    0xE030 _ 0x0000 _ 0xE040 _ 0xE042, 0x0028 _ 0xE07F _ 0xE08F _ 0xE091 _ 0xE09F)
MATHML_CHAR( 10, RightParenthesis,   0xE031 _ 0x0000 _ 0xE041 _ 0xE043, 0x0029 _ 0xE080 _ 0xE090 _ 0xE092 _ 0xE021)
MATHML_CHAR( 20, LeftSquareBracket,  0xE032 _ 0x0000 _ 0xE034 _ 0xE036, 0x005B _ 0xE081 _ 0xE093 _ 0xE068 _ 0xE022)
MATHML_CHAR( 30, RightSquareBracket, 0xE033 _ 0x0000 _ 0xE035 _ 0xE037, 0x005D _ 0xE082 _ 0xE094 _ 0xE069 _ 0xE023)
MATHML_CHAR( 40, LeftFloor,          0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x230A _ 0xE083 _ 0xE095 _ 0xE06A _ 0xE024)
MATHML_CHAR( 50, RightFloor,         0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x230B _ 0xE084 _ 0xE096 _ 0xE06B _ 0xE025)
MATHML_CHAR( 60, LeftCeiling,        0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2308 _ 0xE085 _ 0xE097 _ 0xE06C _ 0xE026)
MATHML_CHAR( 70, RightCeiling,       0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2309 _ 0xE086 _ 0xE098 _ 0xE06D _ 0xE027)
MATHML_CHAR( 80, LeftCurlyBracket,   0xE038 _ 0xE03C _ 0xE03A _ 0xE03E, 0x007B _ 0xE087 _ 0xE099 _ 0xE06E _ 0xE028)
MATHML_CHAR( 90, RightCurlyBracket,  0xE039 _ 0xE03D _ 0xE03B _ 0xE03E, 0x007D _ 0xE088 _ 0xE09A _ 0xE06F _ 0xE029)
MATHML_CHAR(100, LeftAngleBracket,   0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x3008 _ 0xE089 _ 0xE09B _ 0xE044 _ 0xE02A)
MATHML_CHAR(110, RightAngleBracket,  0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x3009 _ 0xE08A _ 0xE09C _ 0xE045 _ 0xE02B)
MATHML_CHAR(120, SquareUnion,        0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2294 _ 0xE046 _ 0xE047)
MATHML_CHAR(128, ContourIntegral,    0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x222E _ 0xE048 _ 0xE049)
MATHML_CHAR(136, CircleDot,          0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2299 _ 0xE04A _ 0xE04B)
MATHML_CHAR(144, CirclePlus,         0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2295 _ 0xE04C _ 0xE04D)
MATHML_CHAR(152, CircleMultiply,     0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2297 _ 0xE04E _ 0xE04F)
MATHML_CHAR(160, Sum,                0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2211 _ 0xE050 _ 0xE058)
MATHML_CHAR(168, Product,            0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x220F _ 0xE051 _ 0xE059)
MATHML_CHAR(176, Slash,              0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x002F _ 0xE02E _ 0xE09D _ 0xE08D _ 0xE02C)
MATHML_CHAR(186, Union,              0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x22C3 _ 0xE053 _ 0xE05B)
MATHML_CHAR(194, Intersection,       0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x22C2 _ 0xE054 _ 0xE05C)
MATHML_CHAR(202, UnionPlus,          0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x228E _ 0xE055 _ 0xE05D)
MATHML_CHAR(210, And,                0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2227 _ 0xE056 _ 0xE05E)
MATHML_CHAR(218, Or,                 0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2228 _ 0xE057 _ 0xE05F)
MATHML_CHAR(226, Coproduct,          0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x2210 _ 0xE060 _ 0xE061) 
MATHML_CHAR(234, Hat,                0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x0302 _ 0xE062 _ 0xE063 _ 0xE064)
MATHML_CHAR(243, Tilde,              0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x223C _ 0xE065 _ 0xE066 _ 0xE067)
MATHML_CHAR(252, Sqrt,               0xE076 _ 0x0000 _ 0xE074 _ 0xE075, 0x221A _ 0xE070 _ 0xE071 _ 0xE072 _ 0xE073)
MATHML_CHAR(262, UpArrow,            0xE078 _ 0x0000 _ 0x0000 _ 0xE03F, 0x2191)
MATHML_CHAR(268, DownArrow,          0x0000 _ 0x0000 _ 0xE079 _ 0xE03F, 0x2193)
MATHML_CHAR(274, UpDownArrow,        0xE078 _ 0x0000 _ 0xE079 _ 0xE03F, 0x2195)
MATHML_CHAR(280, DoubleUpArrow,      0xE07E _ 0x0000 _ 0x0000 _ 0xE077, 0x21D1)
MATHML_CHAR(286, DoubleDownArrow,    0x0000 _ 0x0000 _ 0xE0A0 _ 0xE077, 0x21D3)
MATHML_CHAR(292, DoubleUpDownArrow,  0xE07E _ 0x0000 _ 0xE0A0 _ 0xE077, 0x21D5)
//XXX awaiting the italic correction
//MATHML_CHAR(298, Integral,           0x0000 _ 0x0000 _ 0x0000 _ 0x0000, 0x222B _ 0xE052 _ 0xE05A)
   #endif // defined(WANT_CMEX_DATA)

// Data for stretchy chars that are supported by the Math4 font ----------------------
// -----------------------------------------------------------------------------------
   #if defined(WANT_MATH4_DATA) // [top/left][middle][bot/right][glue] [size0 ... size{N-1}]
MATHML_CHAR( 0, UnderCurlyBracket,   0xEC00 _ 0xEC01 _ 0xEC02 _ 0xEC03, 0xF613 _ 0xEC29 _ 0xEC2A _ 0xEC2B)
MATHML_CHAR( 9, OverCurlyBracket,    0xEC04 _ 0xEC05 _ 0xEC06 _ 0xEC07, 0xF612 _ 0xEC25 _ 0xEC26 _ 0xEC27)
MATHML_CHAR(18, UnderSquareBracket,  0xEC08 _ 0x0000 _ 0xEC0A _ 0xEC09, 0xF615 _ 0xEC19 _ 0xEC1A _ 0xEC1B)
MATHML_CHAR(27, OverSquareBracket,   0xEC0B _ 0x0000 _ 0xEC0D _ 0xEC0C, 0xF614 _ 0xEC15 _ 0xEC16 _ 0xEC17)
MATHML_CHAR(36, UnderParenthesis,    0xEC0E _ 0x0000 _ 0xEC10 _ 0xEC0F, 0xF611 _ 0xEC21 _ 0xEC22 _ 0xEC23)
MATHML_CHAR(45, OverParenthesis,     0xEC11 _ 0x0000 _ 0xEC13 _ 0xEC12, 0xF610 _ 0xEC1D _ 0xEC1E _ 0xEC1F)
   #endif // defined(WANT_MATH4_DATA)

#undef _
#endif // defined(WANT_CHAR_DATA) || defined(WANT_GLYPH_DATA)


#undef MATHML_CHAR


#if 0
// awaiting PUA codes from Math4
    0xD0      0x002F [Forward slash (solidus)]
    0xD1      0x002F [Forward slash (solidus)]
    0xD2      0x002F [Forward slash (solidus)]
    0xD3      0x002F [Forward slash (solidus)]
    0xD4      0x002F [Forward slash (solidus)]
    0xD5      0x002F [Forward slash (solidus)]
#endif
