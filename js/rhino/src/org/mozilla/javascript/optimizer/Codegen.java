/*
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Rhino code, released
 * May 6, 1999.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1997-2000 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s):
 * Norris Boyd
 * Kemal Bayram
 * Igor Bukanov
 * Roger Lawrence
 * Andi Vajda
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */


package org.mozilla.javascript.optimizer;

import org.mozilla.javascript.*;
import org.mozilla.classfile.*;
import java.util.*;
import java.io.IOException;
import java.lang.reflect.Constructor;

/**
 * This class generates code for a given IR tree.
 *
 * @author Norris Boyd
 * @author Roger Lawrence
 */

public class Codegen extends Interpreter {

    public Codegen()
    {
    }

    public IRFactory createIRFactory(Context cx, TokenStream ts)
    {
        if (nameHelper == null) {
            nameHelper = (OptClassNameHelper)ClassNameHelper.get(cx);
            itsUseDynamicScope = cx.hasCompileFunctionsWithDynamicScope();
            generateDebugInfo = (!cx.isGeneratingDebugChanged()
                                 || cx.isGeneratingDebug());
            languageVersion = cx.getLanguageVersion();
        }
        return new IRFactory(this, ts);
    }

    public FunctionNode createFunctionNode(IRFactory irFactory, String name)
    {
        return new OptFunctionNode(name);
    }

    public ScriptOrFnNode transform(Context cx, IRFactory irFactory,
                                    ScriptOrFnNode tree)
    {
        int optLevel = cx.getOptimizationLevel();
        Hashtable possibleDirectCalls = null;
        if (optLevel > 0) {
           /*
            * Collect all of the contained functions into a hashtable
            * so that the call optimizer can access the class name & parameter
            * count for any call it encounters
            */
            if (tree.getType() == Token.SCRIPT) {
                int functionCount = tree.getFunctionCount();
                for (int i = 0; i != functionCount; ++i) {
                    OptFunctionNode fn;
                    fn = (OptFunctionNode)tree.getFunctionNode(i);
                    if (fn.getFunctionType()
                        == FunctionNode.FUNCTION_STATEMENT)
                    {
                        String name = fn.getFunctionName();
                        if (name.length() != 0) {
                            if (possibleDirectCalls == null) {
                                possibleDirectCalls = new Hashtable();
                            }
                            possibleDirectCalls.put(name, fn);
                        }
                    }
                }
            }
        }

        if (possibleDirectCalls != null) {
            directCallTargets = new ObjArray();
        }

        OptTransformer ot = new OptTransformer(irFactory, possibleDirectCalls,
                                               directCallTargets);
        ot.transform(tree);

        if (optLevel > 0) {
            (new Optimizer(irFactory)).optimize(tree, optLevel);
        }

        return tree;
    }

    public Object compile(Context cx, Scriptable scope,
                          ScriptOrFnNode scriptOrFn,
                          SecurityController securityController,
                          Object securityDomain, String encodedSource)
    {
        initScriptOrFnNodesData(scriptOrFn);
        initClassNames();

        generateCode(encodedSource);

        boolean onlySave = false;
        ClassRepository repository = nameHelper.getClassRepository();
        if (repository != null) {
            try {
                if (!repository.storeClass(mainClassName, mainClassBytes,
                                           true))
                {
                    onlySave = true;
                }
                if (generateHelperClass) {
                    if (!repository.storeClass(helperClassName,
                                               helperClassBytes,
                                               false))
                    {
                        onlySave = true;
                    }
                }
            } catch (IOException iox) {
                throw ScriptRuntime.throwAsUncheckedException(iox);
            }

            Class[] interfaces = nameHelper.getTargetImplements();
            Class superClass = nameHelper.getTargetExtends();
            if (interfaces != null || superClass != null) {
                String adapterClassName = getScriptClassName(true);
                ScriptableObject obj = new NativeObject();
                int functionCount = scriptOrFn.getFunctionCount();
                for (int i = 0; i != functionCount; ++i) {
                    OptFunctionNode fn;
                    fn = (OptFunctionNode)scriptOrFn.getFunctionNode(i);
                    String name = fn.getFunctionName();
                    if (name != null && name.length() != 0) {
                        obj.put(fn.getFunctionName(), obj, fn);
                    }
                }
                if (superClass == null) {
                    superClass = ScriptRuntime.ObjectClass;
                }
                byte[] classFile = JavaAdapter.createAdapterCode(
                                       cx, obj, adapterClassName,
                                       superClass, interfaces,
                                       mainClassName);
                try {
                    if (!repository.storeClass(adapterClassName, classFile,
                                               true))
                    {
                        onlySave = true;
                    }
                } catch (IOException iox) {
                    throw ScriptRuntime.throwAsUncheckedException(iox);
                }
            }
        }

        if (onlySave) { return null; }

        Exception e = null;
        Class result = null;
        ClassLoader parentLoader = cx.getApplicationClassLoader();
        GeneratedClassLoader loader;
        if (securityController == null) {
            loader = cx.createClassLoader(parentLoader);
        } else {
            loader = securityController.createClassLoader(parentLoader,
                                                          securityDomain);
        }

        try {
            result = loader.defineClass(mainClassName, mainClassBytes);
            if (generateHelperClass) {
                loader.defineClass(helperClassName, helperClassBytes);
            }
            loader.linkClass(result);
        } catch (SecurityException x) {
            e = x;
        } catch (IllegalArgumentException x) {
            e = x;
        }
        if (e != null)
            throw new RuntimeException("Malformed optimizer package " + e);

        if (scriptOrFn.getType() == Token.FUNCTION) {
            NativeFunction f;
            try {
                Constructor ctor = result.getConstructors()[0];
                Object[] initArgs = { scope, cx, new Integer(0) };
                f = (NativeFunction)ctor.newInstance(initArgs);
            } catch (Exception ex) {
                throw new RuntimeException
                    ("Unable to instantiate compiled class:"+ex.toString());
            }
            int ftype = ((OptFunctionNode)scriptOrFn).getFunctionType();
            OptRuntime.initFunction(f, ftype, scope, cx);
            return f;
        } else {
            Script script;
            try {
                script = (Script) result.newInstance();
            } catch (Exception ex) {
                throw new RuntimeException
                    ("Unable to instantiate compiled class:"+ex.toString());
            }
            return script;
        }
    }

    public void notifyDebuggerCompilationDone(Context cx,
                                              Object scriptOrFunction,
                                              String debugSource)
    {
        // Not supported
    }

    private void initScriptOrFnNodesData(ScriptOrFnNode scriptOrFn)
    {
        ObjArray x = new ObjArray();
        collectScriptOrFnNodes_r(scriptOrFn, x);

        int count = x.size();
        scriptOrFnNodes = new ScriptOrFnNode[count];
        x.toArray(scriptOrFnNodes);

        scriptOrFnIndexes = new ObjToIntMap(count);
        for (int i = 0; i != count; ++i) {
            scriptOrFnIndexes.put(scriptOrFnNodes[i], i);
        }

        if (scriptOrFnNodes.length == 1
            ||scriptOrFn.getType() == Token.FUNCTION)
        {
            generateHelperClass = false;
        } else {
            generateHelperClass = true;
        }
    }

    private static void collectScriptOrFnNodes_r(ScriptOrFnNode n,
                                                 ObjArray x)
    {
        x.add(n);
        int nestedCount = n.getFunctionCount();
        for (int i = 0; i != nestedCount; ++i) {
            collectScriptOrFnNodes_r(n.getFunctionNode(i), x);
        }
    }

    private void initClassNames()
    {
        boolean isPrimary = (nameHelper.getTargetExtends() == null
                             && nameHelper.getTargetImplements() == null);
        mainClassName = getScriptClassName(isPrimary);
        mainClassSignature
            = ClassFileWriter.classNameToSignature(mainClassName);
        if (generateHelperClass) {
            helperClassName = getScriptClassName(false);
            functionClassName = helperClassName;
            functionClassSignature
                = ClassFileWriter.classNameToSignature(helperClassName);
        } else {
            functionClassName = mainClassName;
            functionClassSignature = mainClassSignature;
        }
    }

    private String getScriptClassName(boolean primary)
    {
        return nameHelper.getScriptClassName(null, primary);
    }

    private void generateCode(String encodedSource)
    {
        String sourceFile = null;
        if (generateDebugInfo) {
            sourceFile = scriptOrFnNodes[0].getSourceName();
        }

        ClassFileWriter cfwMain;
        if (!generateHelperClass) {
            if (getFunctionCount() != 0) {
                cfwMain = generateFunctionSpecific(sourceFile, encodedSource);
            } else {
                cfwMain = generateScriptSpecific(sourceFile, encodedSource);
            }
        } else {
            ClassFileWriter cfwHelper;
            cfwHelper = generateFunctionSpecific(sourceFile, encodedSource);
            helperClassBytes = cfwHelper.toByteArray();
            cfwMain = generateScriptSpecific(sourceFile, encodedSource);
        }

        generateBodyMethods(cfwMain, encodedSource);

        mainClassBytes = cfwMain.toByteArray();
    }

    private ClassFileWriter generateScriptSpecific(String sourceFile,
                                                   String encodedSource)
    {
        ClassFileWriter cfw = new ClassFileWriter(mainClassName,
                                                  SUPER_CLASS_NAME,
                                                  sourceFile);
        cfw.addInterface("org/mozilla/javascript/Script");
        cfw.addField(REGEXP_ARRAY_FIELD_NAME, REGEXP_ARRAY_FIELD_TYPE,
                     (short)0);

        generateScriptCtor(cfw);
        generateMain(cfw);
        generateExecute(cfw);
        if (encodedSource != null) {
            generateGetEncodedSource(cfw, true);
        }

        return cfw;
    }

    private ClassFileWriter generateFunctionSpecific(String sourceFile,
                                                     String encodedSource)
    {
        ClassFileWriter cfw = new ClassFileWriter(functionClassName,
                                                  SUPER_CLASS_NAME,
                                                  sourceFile);
        cfw.addField(ID_FIELD_NAME, "I",
                     ClassFileWriter.ACC_PRIVATE);
        cfw.addField(DIRECT_CALL_PARENT_FIELD, mainClassSignature,
                     (short)0);
        cfw.addField(REGEXP_ARRAY_FIELD_NAME, REGEXP_ARRAY_FIELD_TYPE,
                     (short)0);

        generateFunctionConstructor(cfw);
        generateFunctionCallMethod(cfw);
        if (encodedSource != null) {
            generateGetEncodedSource(cfw, false);
        }

        return cfw;
    }

    private void generateBodyMethods(ClassFileWriter cfw, String encodedSource)
    {
        int count = scriptOrFnNodes.length;
        for (int i = 0; i != count; ++i) {
            ScriptOrFnNode n = scriptOrFnNodes[i];

            boolean isFunction = (n.getType() == Token.FUNCTION);

            String zeroArgClassName
                = (isFunction) ? functionClassName: mainClassName;

            generateInitImpl(cfw, n, zeroArgClassName);

            BodyCodegen bodygen = new BodyCodegen();
            bodygen.cfw = cfw;
            bodygen.codegen = this;
            bodygen.zeroArgClassName = zeroArgClassName;
            bodygen.scriptOrFn = n;

            bodygen.generateBodyCode();

            if (n.getType() == Token.FUNCTION) {
                OptFunctionNode fn = (OptFunctionNode)n;
                if (fn.isTargetOfDirectCall()) {
                    emitDirectConstructor(cfw, fn);
                }
            }
        }

        if (directCallTargets != null) {
            int N = directCallTargets.size();
            for (int j = 0; j != N; ++j) {
                OptFunctionNode
                    fn = (OptFunctionNode)directCallTargets.get(j);
                cfw.addField(getDirectTargetFieldName(j),
                             functionClassSignature,
                             (short)0);
            }
        }

        if (encodedSource != null) {
            generateGetEncodedSourceImpl(cfw, encodedSource);
        }

        emitConstantDudeInitializers(cfw);
    }

    private void emitDirectConstructor(ClassFileWriter cfw,
                                       OptFunctionNode fn)
    {
/*
    we generate ..
        Scriptable directConstruct(<directCallArgs>) {
            Scriptable newInstance = createObject(cx, scope);
            Object val = <body-name>(cx, scope, newInstance, <directCallArgs>);
            if (val instanceof Scriptable && val != Undefined.instance) {
                return (Scriptable) val;
            }
            return newInstance;
        }
*/
        cfw.startMethod(getDirectCtorName(fn),
                        getBodyMethodSignature(fn),
                        ClassFileWriter.ACC_STATIC);

        int argCount = fn.getParamCount();
        int firstLocal = (4 + argCount * 3) + 1;

        cfw.addALoad(0); // this
        cfw.addALoad(1); // cx
        cfw.addALoad(2); // scope
        cfw.addInvoke(ByteCode.INVOKEVIRTUAL,
                      "org/mozilla/javascript/BaseFunction",
                      "createObject",
                      "(Lorg/mozilla/javascript/Context;"
                      +"Lorg/mozilla/javascript/Scriptable;"
                      +")Lorg/mozilla/javascript/Scriptable;");
        cfw.addAStore(firstLocal);

        cfw.addALoad(0);
        cfw.addALoad(1);
        cfw.addALoad(2);
        cfw.addALoad(firstLocal);
        for (int i = 0; i < argCount; i++) {
            cfw.addALoad(4 + (i * 3));
            cfw.addDLoad(5 + (i * 3));
        }
        cfw.addALoad(4 + argCount * 3);
        cfw.addInvoke(ByteCode.INVOKESTATIC,
                      mainClassName,
                      getBodyMethodName(fn),
                      getBodyMethodSignature(fn));
        int exitLabel = cfw.acquireLabel();
        cfw.add(ByteCode.DUP); // make a copy of direct call result
        cfw.add(ByteCode.INSTANCEOF, "org/mozilla/javascript/Scriptable");
        cfw.add(ByteCode.IFEQ, exitLabel);
        cfw.add(ByteCode.DUP); // make a copy of direct call result
        pushUndefined(cfw);
        cfw.add(ByteCode.IF_ACMPEQ, exitLabel);
        // cast direct call result
        cfw.add(ByteCode.CHECKCAST, "org/mozilla/javascript/Scriptable");
        cfw.add(ByteCode.ARETURN);
        cfw.markLabel(exitLabel);

        cfw.addALoad(firstLocal);
        cfw.add(ByteCode.ARETURN);

        cfw.stopMethod((short)(firstLocal + 1), null);

    }

    private void generateFunctionCallMethod(ClassFileWriter cfw)
    {
        cfw.startMethod("call",
                        "(Lorg/mozilla/javascript/Context;" +
                        "Lorg/mozilla/javascript/Scriptable;" +
                        "Lorg/mozilla/javascript/Scriptable;" +
                        "[Ljava/lang/Object;)Ljava/lang/Object;",
                        (short)(ClassFileWriter.ACC_PUBLIC
                                | ClassFileWriter.ACC_FINAL));

        cfw.addALoad(0);
        cfw.addALoad(1);
        cfw.addALoad(2);
        cfw.addALoad(3);
        cfw.addALoad(4);

        int start = (scriptOrFnNodes[0].getType() == Token.SCRIPT) ? 1 : 0;
        int end = scriptOrFnNodes.length;
        boolean generateSwitch = (2 <= end - start);

        int switchStart = 0;
        int switchStackTop = 0;
        if (generateSwitch) {
            cfw.addALoad(0);
            cfw.add(ByteCode.GETFIELD, cfw.getClassName(), ID_FIELD_NAME, "I");
            // do switch from (start + 1,  end - 1) mapping start to
            // the default case
            switchStart = cfw.addTableSwitch(start + 1, end - 1);
        }

        for (int i = start; i != end; ++i) {
            if (generateSwitch) {
                if (i == start) {
                    cfw.markTableSwitchDefault(switchStart);
                    switchStackTop = cfw.getStackTop();
                } else {
                    cfw.markTableSwitchCase(switchStart, i - 1 - start,
                                            switchStackTop);
                }
            }
            OptFunctionNode fn = (OptFunctionNode)scriptOrFnNodes[i];
            if (fn.isTargetOfDirectCall()) {
                int pcount = fn.getParamCount();
                if (pcount != 0) {
                    // loop invariant: stack top == arguments array from aload4
                    for (int p = 0; p != pcount; ++p) {
                        cfw.add(ByteCode.ARRAYLENGTH);
                        cfw.addPush(p);
                        int undefArg = cfw.acquireLabel();
                        int beyond = cfw.acquireLabel();
                        cfw.add(ByteCode.IF_ICMPLE, undefArg);
                        // get array[p]
                        cfw.addALoad(4);
                        cfw.addPush(p);
                        cfw.add(ByteCode.AALOAD);
                        cfw.add(ByteCode.GOTO, beyond);
                        cfw.markLabel(undefArg);
                        pushUndefined(cfw);
                        cfw.markLabel(beyond);
                        // Only one push
                        cfw.adjustStackTop(-1);
                        cfw.addPush(0.0);
                        // restore invariant
                        cfw.addALoad(4);
                    }
                }
            }
            cfw.addInvoke(ByteCode.INVOKESTATIC,
                          mainClassName,
                          getBodyMethodName(fn),
                          getBodyMethodSignature(fn));
            cfw.add(ByteCode.ARETURN);
        }
        cfw.stopMethod((short)5, null);
        // 5: this, cx, scope, js this, args[]
    }

    private static void generateMain(ClassFileWriter cfw)
    {
        cfw.startMethod("main", "([Ljava/lang/String;)V",
                        (short)(ClassFileWriter.ACC_PUBLIC
                                | ClassFileWriter.ACC_STATIC));
        cfw.addPush(cfw.getClassName());  // load the name of this class
        cfw.addInvoke(ByteCode.INVOKESTATIC,
                      "java/lang/Class",
                      "forName",
                      "(Ljava/lang/String;)Ljava/lang/Class;");
        cfw.add(ByteCode.ALOAD_0); // load 'args'
        cfw.addInvoke(ByteCode.INVOKESTATIC,
                      "org/mozilla/javascript/ScriptRuntime",
                      "main",
                      "(Ljava/lang/Class;[Ljava/lang/String;)V");
        cfw.add(ByteCode.RETURN);
        // 1 = String[] args
        cfw.stopMethod((short)1, null);
    }

    private static void generateExecute(ClassFileWriter cfw)
    {
        cfw.startMethod("exec",
                        "(Lorg/mozilla/javascript/Context;"
                        +"Lorg/mozilla/javascript/Scriptable;"
                        +")Ljava/lang/Object;",
                        (short)(ClassFileWriter.ACC_PUBLIC
                                | ClassFileWriter.ACC_FINAL));

        final byte ALOAD_CONTEXT = ByteCode.ALOAD_1;
        final byte ALOAD_SCOPE = ByteCode.ALOAD_2;

        // to begin a script, call the initScript method
        cfw.add(ByteCode.ALOAD_0); // load 'this'
        cfw.add(ALOAD_SCOPE);
        cfw.add(ALOAD_CONTEXT);
        cfw.addInvoke(ByteCode.INVOKEVIRTUAL, cfw.getClassName(),
                      "initScript", SCRIPT_INIT_SIGNATURE);

        cfw.add(ByteCode.ALOAD_0); // load 'this'
        cfw.add(ALOAD_CONTEXT);
        cfw.add(ALOAD_SCOPE);
        cfw.add(ByteCode.DUP);
        cfw.add(ByteCode.ACONST_NULL);
        cfw.addInvoke(ByteCode.INVOKEVIRTUAL,
                      cfw.getClassName(),
                      "call",
                      "(Lorg/mozilla/javascript/Context;"
                      +"Lorg/mozilla/javascript/Scriptable;"
                      +"Lorg/mozilla/javascript/Scriptable;"
                      +"[Ljava/lang/Object;"
                      +")Ljava/lang/Object;");

        cfw.add(ByteCode.ARETURN);
        // 3 = this + context + scope
        cfw.stopMethod((short)3, null);
    }

    private static void generateScriptCtor(ClassFileWriter cfw)
    {
        cfw.startMethod("<init>", "()V", ClassFileWriter.ACC_PUBLIC);
        cfw.add(ByteCode.ALOAD_0);
        cfw.addInvoke(ByteCode.INVOKESPECIAL, SUPER_CLASS_NAME,
                      "<init>", "()V");
        cfw.add(ByteCode.RETURN);
        // 1 parameter = this
        cfw.stopMethod((short)1, null);
    }

    private void generateFunctionConstructor(ClassFileWriter cfw)
    {
        final byte SCOPE_ARG = 1;
        final byte CONTEXT_ARG = 2;
        final byte ID_ARG = 3;

        cfw.startMethod("<init>", FUNCTION_CONSTRUCTOR_SIGNATURE,
                        ClassFileWriter.ACC_PUBLIC);
        cfw.addALoad(0);
        cfw.addInvoke(ByteCode.INVOKESPECIAL, SUPER_CLASS_NAME,
                      "<init>", "()V");

        cfw.addLoadThis();
        cfw.addILoad(ID_ARG);
        cfw.add(ByteCode.PUTFIELD, cfw.getClassName(), ID_FIELD_NAME, "I");

        cfw.addLoadThis();
        cfw.addALoad(CONTEXT_ARG);
        cfw.addALoad(SCOPE_ARG);

        int start = (scriptOrFnNodes[0].getType() == Token.SCRIPT) ? 1 : 0;
        int end = scriptOrFnNodes.length;
        boolean generateSwitch = (2 <= end - start);

        int switchStart = 0;
        int switchStackTop = 0;
        if (generateSwitch) {
            cfw.addILoad(ID_ARG);
            // do switch from (start + 1,  end - 1) mapping start to
            // the default case
            switchStart = cfw.addTableSwitch(start + 1, end - 1);
        }

        for (int i = start; i != end; ++i) {
            if (generateSwitch) {
                if (i == start) {
                    cfw.markTableSwitchDefault(switchStart);
                    switchStackTop = cfw.getStackTop();
                } else {
                    cfw.markTableSwitchCase(switchStart, i - 1 - start,
                                            switchStackTop);
                }
            }
            OptFunctionNode fn = (OptFunctionNode)scriptOrFnNodes[i];
            cfw.addInvoke(ByteCode.INVOKESTATIC,
                          mainClassName,
                          getInitImplMethodName(fn),
                          getInitImplMethodSignature(fn));
            cfw.add(ByteCode.RETURN);
        }

        // 4 = this + scope + context + id
        cfw.stopMethod((short)4, null);
    }

    private void generateInitImpl(ClassFileWriter cfw,
                                  ScriptOrFnNode scriptOrFn,
                                  String zeroArgClassName)
    {
        boolean isScript = (scriptOrFn.getType() == Token.SCRIPT);
        OptFunctionNode fn;
        if (!isScript) {
            fn = (OptFunctionNode)scriptOrFn;
        } else {
            fn = null;
        }

        final byte SELF_ARG = 0; // scriptThis/functionRef
        final byte CONTEXT_ARG;
        final byte SCOPE_ARG;

        if (fn != null) {
            CONTEXT_ARG = 1;
            SCOPE_ARG = 2;
            cfw.startMethod(getInitImplMethodName(fn),
                            getInitImplMethodSignature(fn),
                            ClassFileWriter.ACC_STATIC);
        } else {
            CONTEXT_ARG = 2;
            SCOPE_ARG = 1;
            cfw.startMethod("initScript", SCRIPT_INIT_SIGNATURE,
                            ClassFileWriter.ACC_PUBLIC);
        }

        // Prepare stack to call NativeFunction.initScriptFunction
        cfw.addALoad(SELF_ARG);
        cfw.addPush(languageVersion);

        if (fn != null) {
            cfw.addPush(fn.getFunctionName());
        } else {
            cfw.addPush("");
        }

        // Push string array with the names of the parameters and the vars.
        int paramAndVarCount = scriptOrFn.getParamAndVarCount();
        if (paramAndVarCount == 0) {
            cfw.add(ByteCode.GETSTATIC,
                    "org/mozilla/javascript/ScriptRuntime",
                    "emptyStrings", "[Ljava/lang/String;");
        } else {
            cfw.addPush(paramAndVarCount);
            cfw.add(ByteCode.ANEWARRAY, "java/lang/String");
            for (int i = 0; i != paramAndVarCount; ++i) {
                cfw.add(ByteCode.DUP);
                cfw.addPush(i);
                cfw.addPush(scriptOrFn.getParamOrVarName(i));
                cfw.add(ByteCode.AASTORE);
            }
        }

        cfw.addPush(scriptOrFn.getParamCount());

        cfw.addInvoke(ByteCode.INVOKEVIRTUAL,
                    "org/mozilla/javascript/NativeFunction",
                    "initScriptFunction",
                    "(ILjava/lang/String;[Ljava/lang/String;I)V");

        if (fn != null) {
            cfw.addALoad(SELF_ARG);
            cfw.addALoad(SCOPE_ARG);
            cfw.addInvoke(ByteCode.INVOKEVIRTUAL,
                          "org/mozilla/javascript/ScriptableObject",
                          "setParentScope",
                          "(Lorg/mozilla/javascript/Scriptable;)V");
        }

        // precompile all regexp literals
        int regexpCount = scriptOrFn.getRegexpCount();
        if (regexpCount != 0) {
            cfw.addALoad(SELF_ARG);
            cfw.addPush(regexpCount);
            cfw.add(ByteCode.ANEWARRAY,
                    "org/mozilla/javascript/regexp/NativeRegExp");
            // Loop invariant: it keeps stack this, new-array-ref
            for (int i = 0; i != regexpCount; ++i) {
                cfw.add(ByteCode.DUP);
                cfw.addPush(i);

                cfw.add(ByteCode.NEW,
                            "org/mozilla/javascript/regexp/NativeRegExp");
                cfw.add(ByteCode.DUP);

                cfw.addALoad(CONTEXT_ARG);
                cfw.addALoad(SCOPE_ARG);
                cfw.addPush(scriptOrFn.getRegexpString(i));
                String regexpFlags = scriptOrFn.getRegexpFlags(i);
                if (regexpFlags == null) {
                    cfw.add(ByteCode.ACONST_NULL);
                } else {
                    cfw.addPush(regexpFlags);
                }
                cfw.addPush(0);
                cfw.addInvoke(ByteCode.INVOKESPECIAL,
                              "org/mozilla/javascript/regexp/NativeRegExp",
                              "<init>",
                              "(Lorg/mozilla/javascript/Context;"
                              +"Lorg/mozilla/javascript/Scriptable;"
                              +"Ljava/lang/String;Ljava/lang/String;"
                              +"Z"
                              +")V");

                cfw.add(ByteCode.AASTORE);
            }
            cfw.add(ByteCode.PUTFIELD, zeroArgClassName,
                    REGEXP_ARRAY_FIELD_NAME, REGEXP_ARRAY_FIELD_TYPE);
        }

        cfw.add(ByteCode.RETURN);
        // 3 = (scriptThis/functionRef) + scope + context
        cfw.stopMethod((short)3, null);
    }

    private void generateGetEncodedSource(ClassFileWriter cfw,
                                          boolean forScript)
    {
        // Override NativeFunction.getEncodedSourceg() with
        // public String getEncodedSource()
        // {
        //     return main_class.getEncodedSourceImpl(scriptOrFnIndex);
        // }
        cfw.startMethod("getEncodedSource", "()Ljava/lang/Object;",
                        ClassFileWriter.ACC_PUBLIC);
        if (forScript) {
            cfw.addPush(0);
        } else {
            cfw.addALoad(0);
            cfw.add(ByteCode.GETFIELD, cfw.getClassName(), ID_FIELD_NAME, "I");
        }
        cfw.addInvoke(ByteCode.INVOKESTATIC,
                      mainClassName,
                      "getEncodedSourceImpl",
                      "(I)Ljava/lang/String;");
        cfw.add(ByteCode.ARETURN);
        // 1: this and no argument or locals
        cfw.stopMethod((short)1, null);
    }

    private void generateGetEncodedSourceImpl(ClassFileWriter cfw,
                                              String encodedSource)
    {
        // generate
        // static String getEncodedSourceImpl(int index)
        // {
        //     int start, end;
        //     switch (index) {
        //       case 1: start, end = embedded_constants_for_function_1;
        //       case 2: start, end = embedded_constants_for_function_2;
        //       ...
        //       default: start, end = embedded_constants_for_function_0;
        //     }
        //     return ENCODED.substring(start, end);
        // }
        cfw.startMethod("getEncodedSourceImpl", "(I)Ljava/lang/String;",
                        ClassFileWriter.ACC_STATIC);

        cfw.addPush(encodedSource);

        int count = scriptOrFnNodes.length;
        if (count == 1) {
            // do not generate switch in this case
            ScriptOrFnNode n = scriptOrFnNodes[0];
            cfw.addPush(n.getEncodedSourceStart());
            cfw.addPush(n.getEncodedSourceEnd());
        } else {
            cfw.addILoad(0);
            // do switch from 1 .. count - 1 mapping 0 to the default case
            int switchStart = cfw.addTableSwitch(1, count - 1);
            int afterSwitch = cfw.acquireLabel();
            int switchStackTop = 0;
            for (int i = 0; i != count; ++i) {
                ScriptOrFnNode n = scriptOrFnNodes[i];
                if (i == 0) {
                    cfw.markTableSwitchDefault(switchStart);
                    switchStackTop = cfw.getStackTop();
                } else {
                    cfw.markTableSwitchCase(switchStart, i - 1,
                                            switchStackTop);
                }
                cfw.addPush(n.getEncodedSourceStart());
                cfw.addPush(n.getEncodedSourceEnd());
                // Add goto past switch code unless the last statement
                if (i + 1 != count) {
                    cfw.add(ByteCode.GOTO, afterSwitch);
                }
            }
            cfw.markLabel(afterSwitch);
        }

        cfw.addInvoke(ByteCode.INVOKEVIRTUAL,
                      "java/lang/String",
                      "substring",
                      "(II)Ljava/lang/String;");
        cfw.add(ByteCode.ARETURN);
        cfw.stopMethod((short)1, null);
    }

    private void emitConstantDudeInitializers(ClassFileWriter cfw)
    {
        int N = itsConstantListSize;
        if (N == 0)
            return;

        cfw.startMethod("<clinit>", "()V",
            (short)(ClassFileWriter.ACC_STATIC | ClassFileWriter.ACC_FINAL));

        double[] array = itsConstantList;
        for (int i = 0; i != N; ++i) {
            double num = array[i];
            String constantName = "jsK_" + i;
            String constantType = getStaticConstantWrapperType(num);
            cfw.addField(constantName, constantType,
                         ClassFileWriter.ACC_STATIC);
            pushNewNumberObject(cfw, num);
            cfw.add(ByteCode.PUTSTATIC, mainClassName,
                    constantName, constantType);
        }

        cfw.add(ByteCode.RETURN);
        cfw.stopMethod((short)0, null);
    }

    void pushNumberAsObject(ClassFileWriter cfw, double num)
    {
        if (num != num) {
            // Add NaN object
            cfw.add(ByteCode.GETSTATIC,
                    "org/mozilla/javascript/ScriptRuntime",
                    "NaNobj", "Ljava/lang/Double;");
        } else if (itsConstantListSize >= 2000) {
            // There appears to be a limit in the JVM on either the number
            // of static fields in a class or the size of the class
            // initializer. Either way, we can't have any more than 2000
            // statically init'd constants.
            pushNewNumberObject(cfw, num);
        } else {
            String constantName = "jsK_" + addNumberConstant(num);
            String constantType = getStaticConstantWrapperType(num);
            cfw.add(ByteCode.GETSTATIC, mainClassName,
                    constantName, constantType);
        }
    }

    private static void pushNewNumberObject(ClassFileWriter cfw, double num)
    {
        // Generate code to create the new numeric constant
        //
        // new java/lang/<WrapperType>
        // dup
        // push <number>
        // invokestatic java/lang/<WrapperType>/<init>(X)V

        String wrapperType;
        String signature;
        boolean isInteger;
        int inum = (int)num;
        if (inum == num) {
            isInteger = true;
            wrapperType = "java/lang/Integer";
            signature = "(I)V";
        } else {
            isInteger = false;
            wrapperType = "java/lang/Double";
            signature = "(D)V";
        }

        cfw.add(ByteCode.NEW, wrapperType);
        cfw.add(ByteCode.DUP);
        if (isInteger) { cfw.addPush(inum); }
        else { cfw.addPush(num); }
        cfw.addInvoke(ByteCode.INVOKESPECIAL, wrapperType, "<init>", signature);
    }

    private static String getStaticConstantWrapperType(double num)
    {
        String constantType;
        int inum = (int)num;
        if (inum == num) {
            return "Ljava/lang/Integer;";
        } else {
            return "Ljava/lang/Double;";
        }
    }

    private int addNumberConstant(double num)
    {
        // NaN is provided via ScriptRuntime.NaNobj
        if (num != num) Context.codeBug();
        int N = itsConstantListSize;
        if (N == 0) {
            itsConstantList = new double[128];
        } else {
            double[] array = itsConstantList;
            for (int i = 0; i != N; ++i) {
                if (array[i] == num) { return i; }
            }
            if (N == array.length) {
                array = new double[N * 2];
                System.arraycopy(itsConstantList, 0, array, 0, N);
                itsConstantList = array;
            }
        }
        itsConstantList[N] = num;
        itsConstantListSize = N + 1;
        return N;
    }

    static void pushUndefined(ClassFileWriter cfw)
    {
        cfw.add(ByteCode.GETSTATIC, "org/mozilla/javascript/Undefined",
                "instance", "Lorg/mozilla/javascript/Scriptable;");
    }

    private int getFunctionCount()
    {
        return (scriptOrFnNodes[0].getType() == Token.SCRIPT)
               ? scriptOrFnNodes.length - 1
               : scriptOrFnNodes.length;
    }

    int getIndex(ScriptOrFnNode n)
    {
        return scriptOrFnIndexes.getExisting(n);
    }

    static String getDirectTargetFieldName(int i)
    {
        return "_dt" + i;
    }

    String getDirectCtorName(ScriptOrFnNode n)
    {
        return "_n"+getIndex(n);
    }

    String getBodyMethodName(OptFunctionNode fn)
    {
        return "_c"+getIndex(fn);
    }

    String getBodyMethodSignature(OptFunctionNode fn)
    {
        StringBuffer sb = new StringBuffer();
        sb.append('(');
        sb.append(functionClassSignature);
        sb.append("Lorg/mozilla/javascript/Context;"
                  +"Lorg/mozilla/javascript/Scriptable;"
                  +"Lorg/mozilla/javascript/Scriptable;");
        if (fn.isTargetOfDirectCall()) {
            int pCount = fn.getParamCount();
            for (int i = 0; i != pCount; i++) {
                sb.append("Ljava/lang/Object;D");
            }
        }
        sb.append("[Ljava/lang/Object;)Ljava/lang/Object;");
        return sb.toString();
    }

    String getInitImplMethodName(OptFunctionNode fn)
    {
        return "_i"+getIndex(fn);
    }

    String getInitImplMethodSignature(OptFunctionNode fn)
    {
        StringBuffer sb = new StringBuffer();
        sb.append('(');
        sb.append(functionClassSignature);
        sb.append("Lorg/mozilla/javascript/Context;"
                  +"Lorg/mozilla/javascript/Scriptable;"
                  +")V");
        return sb.toString();
    }

    static RuntimeException badTree()
    {
        throw new RuntimeException("Bad tree in codegen");
    }

    private static final String SUPER_CLASS_NAME
        = "org.mozilla.javascript.NativeFunction";

    static final String DIRECT_CALL_PARENT_FIELD = "_dcp";

    private static final String ID_FIELD_NAME = "_id";

    static final String REGEXP_ARRAY_FIELD_NAME = "_re";
    static final String REGEXP_ARRAY_FIELD_TYPE
        = "[Lorg/mozilla/javascript/regexp/NativeRegExp;";

   static final String SCRIPT_INIT_SIGNATURE
        = "(Lorg/mozilla/javascript/Scriptable;"
          +"Lorg/mozilla/javascript/Context;)V";

   static final String FUNCTION_CONSTRUCTOR_SIGNATURE
        = "(Lorg/mozilla/javascript/Scriptable;"
          +"Lorg/mozilla/javascript/Context;I)V";

    private OptClassNameHelper nameHelper;
    private ObjArray directCallTargets;
    ScriptOrFnNode[] scriptOrFnNodes;
    private ObjToIntMap scriptOrFnIndexes;

    private boolean generateHelperClass;

    String mainClassName;
    String mainClassSignature;

    private String helperClassName;

    String functionClassName;
    String functionClassSignature;

    private byte[] mainClassBytes;
    private byte[] helperClassBytes;

    boolean itsUseDynamicScope;
    boolean generateDebugInfo;
    int languageVersion;

    private double[] itsConstantList;
    private int itsConstantListSize;
}


class BodyCodegen
{
    void generateBodyCode()
    {
        initBodyGeneration();

        if (fnCurrent != null) {
            cfw.startMethod(codegen.getBodyMethodName(fnCurrent),
                            codegen.getBodyMethodSignature(fnCurrent),
                            ClassFileWriter.ACC_STATIC);
        } else {
            cfw.startMethod("call",
                            "(Lorg/mozilla/javascript/Context;" +
                            "Lorg/mozilla/javascript/Scriptable;" +
                            "Lorg/mozilla/javascript/Scriptable;" +
                            "[Ljava/lang/Object;)Ljava/lang/Object;",
                            (short)(ClassFileWriter.ACC_PUBLIC
                                    | ClassFileWriter.ACC_FINAL));
        }

        generatePrologue();

        Node treeTop;
        if (fnCurrent != null) {
            treeTop = scriptOrFn.getLastChild();
        } else {
            treeTop = scriptOrFn;
        }
        generateCodeFromNode(treeTop, null);

        generateEpilogue();

        cfw.stopMethod((short)(localsMax + 1), debugVars);
    }

    private void initBodyGeneration()
    {
        if (scriptOrFn.getType() == Token.FUNCTION) {
            fnCurrent = (OptFunctionNode)scriptOrFn;
        } else {
            fnCurrent = null;
        }

        isTopLevel = (scriptOrFn == codegen.scriptOrFnNodes[0]);

        inDirectCallFunction = (fnCurrent == null) ? false
                                   : fnCurrent.isTargetOfDirectCall();

        hasVarsInRegs = (fnCurrent != null && !fnCurrent.requiresActivation());

        locals = new boolean[MAX_LOCALS];

        funObjLocal = 0;
        contextLocal = 1;
        variableObjectLocal = 2;
        thisObjLocal = 3;
        localsMax = (short) 4;  // number of parms + "this"
        firstFreeLocal = 4;

        scriptResultLocal = -1;
        argsLocal = -1;
        itsZeroArgArray = -1;
        itsOneArgArray = -1;
        epilogueLabel = -1;
    }

    /**
     * Generate the prologue for a function or script.
     */
    private void generatePrologue()
    {
        int directParameterCount = -1;
        if (inDirectCallFunction) {
            directParameterCount = scriptOrFn.getParamCount();
            // 0 is reserved for function Object 'this'
            // 1 is reserved for context
            // 2 is reserved for parentScope
            // 3 is reserved for script 'this'
            short jReg = 4;
            for (int i = 0; i != directParameterCount; ++i) {
                OptLocalVariable lVar = fnCurrent.getVar(i);
                lVar.assignJRegister(jReg);
                jReg += 3;  // 3 is 1 for Object parm and 2 for double parm
            }
            if (!fnCurrent.getParameterNumberContext()) {
                // make sure that all parameters are objects
                itsForcedObjectParameters = true;
                for (int i = 0; i != directParameterCount; ++i) {
                    OptLocalVariable lVar = fnCurrent.getVar(i);
                    cfw.addALoad(lVar.getJRegister());
                    cfw.add(ByteCode.GETSTATIC,
                            "java/lang/Void",
                            "TYPE",
                            "Ljava/lang/Class;");
                    int isObjectLabel = cfw.acquireLabel();
                    cfw.add(ByteCode.IF_ACMPNE, isObjectLabel);
                    cfw.add(ByteCode.NEW,"java/lang/Double");
                    cfw.add(ByteCode.DUP);
                    cfw.addDLoad(lVar.getJRegister() + 1);
                    addDoubleConstructor();
                    cfw.addAStore(lVar.getJRegister());
                    cfw.markLabel(isObjectLabel);
                }
            }
        }

        if (fnCurrent != null && !codegen.itsUseDynamicScope
            && directParameterCount == -1)
        {
            // Unless we're either using dynamic scope or we're in a
            // direct call, use the enclosing scope of the function as our
            // variable object.
            cfw.addALoad(funObjLocal);
            cfw.addInvoke(ByteCode.INVOKEINTERFACE,
                          "org/mozilla/javascript/Scriptable",
                          "getParentScope",
                          "()Lorg/mozilla/javascript/Scriptable;");
            cfw.addAStore(variableObjectLocal);
        }

        if (directParameterCount > 0) {
            for (int i = 0; i < (3 * directParameterCount); i++)
                reserveWordLocal(i + 4);               // reserve 'args'
        }
        // reserve 'args[]'
        argsLocal = reserveWordLocal(directParameterCount <= 0
                                     ? 4 : (3 * directParameterCount) + 4);

        // These locals are to be pre-allocated since they need function scope.
        // They are primarily used by the exception handling mechanism
        int localCount = scriptOrFn.getLocalCount();
        if (localCount != 0) {
            itsLocalAllocationBase = (short)(argsLocal + 1);
            for (int i = 0; i < localCount; i++) {
                reserveWordLocal(itsLocalAllocationBase + i);
            }
        }

        if (fnCurrent != null && fnCurrent.getCheckThis()) {
            // Nested functions must check their 'this' value to
            //  insure it is not an activation object:
            //  see 10.1.6 Activation Object
            cfw.addALoad(thisObjLocal);
            addScriptRuntimeInvoke("getThis",
                                   "(Lorg/mozilla/javascript/Scriptable;"
                                   +")Lorg/mozilla/javascript/Scriptable;");
            cfw.addAStore(thisObjLocal);
        }

        if (hasVarsInRegs) {
            // No need to create activation. Pad arguments if need be.
            int parmCount = scriptOrFn.getParamCount();
            if (parmCount > 0 && directParameterCount < 0) {
                // Set up args array
                // check length of arguments, pad if need be
                cfw.addALoad(argsLocal);
                cfw.add(ByteCode.ARRAYLENGTH);
                cfw.addPush(parmCount);
                int label = cfw.acquireLabel();
                cfw.add(ByteCode.IF_ICMPGE, label);
                cfw.addALoad(argsLocal);
                cfw.addPush(parmCount);
                addScriptRuntimeInvoke("padArguments",
                                       "([Ljava/lang/Object;I"
                                       +")[Ljava/lang/Object;");
                cfw.addAStore(argsLocal);
                cfw.markLabel(label);
            }

            // REMIND - only need to initialize the vars that don't get a value
            // before the next call and are used in the function
            short firstUndefVar = -1;
            for (int i = 0; i < fnCurrent.getVarCount(); i++) {
                OptLocalVariable lVar = fnCurrent.getVar(i);
                if (lVar.isNumber()) {
                    lVar.assignJRegister(getNewWordPairLocal());
                    cfw.addPush(0.0);
                    cfw.addDStore(lVar.getJRegister());
                } else if (lVar.isParameter()) {
                    if (directParameterCount < 0) {
                        lVar.assignJRegister(getNewWordLocal());
                        cfw.addALoad(argsLocal);
                        cfw.addPush(i);
                        cfw.add(ByteCode.AALOAD);
                        cfw.addAStore(lVar.getJRegister());
                    }
                } else {
                    lVar.assignJRegister(getNewWordLocal());
                    if (firstUndefVar == -1) {
                        Codegen.pushUndefined(cfw);
                        firstUndefVar = lVar.getJRegister();
                    } else {
                        cfw.addALoad(firstUndefVar);
                    }
                    cfw.addAStore(lVar.getJRegister());
                }
                lVar.setStartPC(cfw.getCurrentCodeOffset());
            }

            // Indicate that we should generate debug information for
            // the variable table. (If we're generating debug info at
            // all.)
            debugVars = fnCurrent.getVarsArray();

            // Skip creating activation object.
            return;
        }

        if (directParameterCount > 0) {
            // We're going to create an activation object, so we
            // need to get an args array with all the arguments in it.

            cfw.addALoad(argsLocal);
            cfw.addPush(directParameterCount);
            addOptRuntimeInvoke("padStart",
                                "([Ljava/lang/Object;I)[Ljava/lang/Object;");
            cfw.addAStore(argsLocal);
            for (int i=0; i < directParameterCount; i++) {
                cfw.addALoad(argsLocal);
                cfw.addPush(i);
                // "3" is 1 for Object parm and 2 for double parm, and
                // "4" is to account for the context, etc. parms
                cfw.addALoad(3 * i + 4);
                cfw.add(ByteCode.AASTORE);
            }
        }

        String debugVariableName;
        if (fnCurrent != null) {
            cfw.addALoad(contextLocal);
            cfw.addALoad(variableObjectLocal);
            cfw.addALoad(funObjLocal);
            cfw.addALoad(thisObjLocal);
            cfw.addALoad(argsLocal);
            addScriptRuntimeInvoke("initVarObj",
                                   "(Lorg/mozilla/javascript/Context;"
                                   +"Lorg/mozilla/javascript/Scriptable;"
                                   +"Lorg/mozilla/javascript/NativeFunction;"
                                   +"Lorg/mozilla/javascript/Scriptable;"
                                   +"[Ljava/lang/Object;"
                                   +")Lorg/mozilla/javascript/Scriptable;");
            debugVariableName = "activation";
        } else {
            cfw.addALoad(contextLocal);
            cfw.addALoad(variableObjectLocal);
            cfw.addALoad(funObjLocal);
            cfw.addALoad(thisObjLocal);
            cfw.addPush(0);
            addScriptRuntimeInvoke("initScript",
                                   "(Lorg/mozilla/javascript/Context;"
                                   +"Lorg/mozilla/javascript/Scriptable;"
                                   +"Lorg/mozilla/javascript/NativeFunction;"
                                   +"Lorg/mozilla/javascript/Scriptable;"
                                   +"Z"
                                   +")Lorg/mozilla/javascript/Scriptable;");
            debugVariableName = "global";
        }
        cfw.addAStore(variableObjectLocal);

        int functionCount = scriptOrFn.getFunctionCount();
        for (int i = 0; i != functionCount; i++) {
            OptFunctionNode fn = (OptFunctionNode)scriptOrFn.getFunctionNode(i);
            if (fn.getFunctionType() == FunctionNode.FUNCTION_STATEMENT) {
                visitFunction(fn, FunctionNode.FUNCTION_STATEMENT);
            }
        }

        // default is to generate debug info
        if (codegen.generateDebugInfo) {
            OptLocalVariable lv = new OptLocalVariable(debugVariableName,
                                                       false);
            lv.assignJRegister(variableObjectLocal);
            lv.setStartPC(cfw.getCurrentCodeOffset());

            debugVars = new OptLocalVariable[1];
            debugVars[0] = lv;
        }

        if (fnCurrent == null) {
            // OPT: use dataflow to prove that this assignment is dead
            scriptResultLocal = getNewWordLocal();
            Codegen.pushUndefined(cfw);
            cfw.addAStore(scriptResultLocal);

            int linenum = scriptOrFn.getEndLineno();
            if (linenum != -1)
              cfw.addLineNumberEntry((short)linenum);

        } else {
            if (fnCurrent.itsContainsCalls0) {
                itsZeroArgArray = getNewWordLocal();
                cfw.add(ByteCode.GETSTATIC,
                        "org/mozilla/javascript/ScriptRuntime",
                        "emptyArgs", "[Ljava/lang/Object;");
                cfw.addAStore(itsZeroArgArray);
            }
            if (fnCurrent.itsContainsCalls1) {
                itsOneArgArray = getNewWordLocal();
                cfw.addPush(1);
                cfw.add(ByteCode.ANEWARRAY, "java/lang/Object");
                cfw.addAStore(itsOneArgArray);
            }
        }
    }

    private void generateEpilogue() {
        if (epilogueLabel != -1) {
            cfw.markLabel(epilogueLabel);
        }
        if (fnCurrent == null || !hasVarsInRegs) {
            // restore caller's activation
            cfw.addALoad(contextLocal);
            addScriptRuntimeInvoke("popActivation",
                                   "(Lorg/mozilla/javascript/Context;)V");
            if (fnCurrent == null) {
                cfw.addALoad(scriptResultLocal);
            }
        }
        cfw.add(ByteCode.ARETURN);
    }

    private void generateCodeFromNode(Node node, Node parent)
    {
        // System.out.println("gen code for " + node.toString());

        int type = node.getType();
        Node child = node.getFirstChild();
        switch (type) {
              case Token.LOOP:
              case Token.WITH:
              case Token.LABEL:
                visitStatement(node);
                while (child != null) {
                    generateCodeFromNode(child, node);
                    child = child.getNext();
                }
                break;

              case Token.CASE:
              case Token.DEFAULT:
                // XXX shouldn't these be StatementNodes?

              case Token.SCRIPT:
              case Token.BLOCK:
              case Token.EMPTY:
              case Token.NOP:
                // no-ops.
                visitStatement(node);
                while (child != null) {
                    generateCodeFromNode(child, node);
                    child = child.getNext();
                }
                break;

              case Token.FUNCTION:
                if (fnCurrent != null || parent.getType() != Token.SCRIPT) {
                    int fnIndex = node.getExistingIntProp(Node.FUNCTION_PROP);
                    OptFunctionNode fn;
                    fn = (OptFunctionNode)scriptOrFn.getFunctionNode(fnIndex);
                    int t = fn.getFunctionType();
                    if (t != FunctionNode.FUNCTION_STATEMENT) {
                        visitFunction(fn, t);
                    }
                }
                break;

              case Token.NAME:
                visitName(node);
                break;

              case Token.NEW:
              case Token.CALL:
                visitCall(node, type, child);
                break;

              case Token.NUMBER:
              case Token.STRING:
                visitLiteral(node);
                break;

              case Token.THIS:
                cfw.addALoad(thisObjLocal);
                break;

              case Token.THISFN:
                cfw.add(ByteCode.ALOAD_0);
                break;

              case Token.NULL:
                cfw.add(ByteCode.ACONST_NULL);
                break;

              case Token.TRUE:
                cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                        "TRUE", "Ljava/lang/Boolean;");
                break;

              case Token.FALSE:
                cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                        "FALSE", "Ljava/lang/Boolean;");
                break;

              case Token.UNDEFINED:
                Codegen.pushUndefined(cfw);
                break;

              case Token.REGEXP:
                visitRegexp(node);
                break;

              case Token.TRY:
                visitTryCatchFinally(node, child);
                break;

              case Token.THROW:
                visitThrow(node, child);
                break;

              case Token.RETURN:
                visitReturn(node, child);
                break;

              case Token.SWITCH:
                visitSwitch(node, child);
                break;

              case Token.COMMA: {
                Node next = child.getNext();
                while (next != null) {
                    generateCodeFromNode(child, node);
                    cfw.add(ByteCode.POP);
                    child = next;
                    next = next.getNext();
                }
                generateCodeFromNode(child, node);
                break;
              }

              case Token.NEWSCOPE:
                addScriptRuntimeInvoke("newScope",
                                       "()Lorg/mozilla/javascript/Scriptable;");
                break;

              case Token.ENTERWITH:
                visitEnterWith(node, child);
                break;

              case Token.LEAVEWITH:
                visitLeaveWith(node, child);
                break;

              case Token.ENUMINIT:
                visitEnumInit(node, child);
                break;

              case Token.ENUMNEXT:
                visitEnumNext(node, child);
                break;

              case Token.ENUMDONE:
                visitEnumDone(node, child);
                break;

              case Token.POP:
                visitStatement(node);
                if (child.getType() == Token.SETVAR) {
                    /* special case this so as to avoid unnecessary
                    load's & pop's */
                    visitSetVar(child, child.getFirstChild(), false);
                }
                else {
                    while (child != null) {
                        generateCodeFromNode(child, node);
                        child = child.getNext();
                    }
                    if (node.getIntProp(Node.ISNUMBER_PROP, -1) != -1)
                        cfw.add(ByteCode.POP2);
                    else
                        cfw.add(ByteCode.POP);
                }
                break;

              case Token.POPV:
                visitStatement(node);
                while (child != null) {
                    generateCodeFromNode(child, node);
                    child = child.getNext();
                }
                cfw.addAStore(scriptResultLocal);
                break;

              case Token.TARGET:
                visitTarget(node);
                break;

              case Token.JSR:
              case Token.GOTO:
              case Token.IFEQ:
              case Token.IFNE:
                visitGOTO(node, type, child);
                break;

              case Token.NOT: {
                int trueTarget = cfw.acquireLabel();
                int falseTarget = cfw.acquireLabel();
                int beyond = cfw.acquireLabel();
                generateIfJump(child, node, trueTarget, falseTarget);

                cfw.markLabel(trueTarget);
                cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                        "FALSE", "Ljava/lang/Boolean;");
                cfw.add(ByteCode.GOTO, beyond);
                cfw.markLabel(falseTarget);
                cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                        "TRUE", "Ljava/lang/Boolean;");
                cfw.markLabel(beyond);
                cfw.adjustStackTop(-1);
                break;
              }

              case Token.BITNOT:
                cfw.add(ByteCode.NEW, "java/lang/Double");
                cfw.add(ByteCode.DUP);
                generateCodeFromNode(child, node);
                addScriptRuntimeInvoke("toInt32", "(Ljava/lang/Object;)I");
                cfw.addPush(-1);         // implement ~a as (a ^ -1)
                cfw.add(ByteCode.IXOR);
                cfw.add(ByteCode.I2D);
                addDoubleConstructor();
                break;

              case Token.VOID:
                generateCodeFromNode(child, node);
                cfw.add(ByteCode.POP);
                Codegen.pushUndefined(cfw);
                break;

              case Token.TYPEOF:
                generateCodeFromNode(child, node);
                addScriptRuntimeInvoke("typeof",
                                       "(Ljava/lang/Object;"
                                       +")Ljava/lang/String;");
                break;

              case Token.TYPEOFNAME:
                visitTypeofname(node);
                break;

              case Token.INC:
                visitIncDec(node, true);
                break;

              case Token.DEC:
                visitIncDec(node, false);
                break;

              case Token.OR:
              case Token.AND: {
                    generateCodeFromNode(child, node);
                    cfw.add(ByteCode.DUP);
                    addScriptRuntimeInvoke("toBoolean",
                                           "(Ljava/lang/Object;)Z");
                    int falseTarget = cfw.acquireLabel();
                    if (type == Token.AND)
                        cfw.add(ByteCode.IFEQ, falseTarget);
                    else
                        cfw.add(ByteCode.IFNE, falseTarget);
                    cfw.add(ByteCode.POP);
                    generateCodeFromNode(child.getNext(), node);
                    cfw.markLabel(falseTarget);
                }
                break;

              case Token.ADD: {
                    generateCodeFromNode(child, node);
                    generateCodeFromNode(child.getNext(), node);
                    switch (node.getIntProp(Node.ISNUMBER_PROP, -1)) {
                        case Node.BOTH:
                            cfw.add(ByteCode.DADD);
                            break;
                        case Node.LEFT:
                            addOptRuntimeInvoke("add",
                                "(DLjava/lang/Object;)Ljava/lang/Object;");
                            break;
                        case Node.RIGHT:
                            addOptRuntimeInvoke("add",
                                "(Ljava/lang/Object;D)Ljava/lang/Object;");
                            break;
                        default:
                        addScriptRuntimeInvoke("add",
                                               "(Ljava/lang/Object;"
                                               +"Ljava/lang/Object;"
                                               +")Ljava/lang/Object;");
                    }
                }
                break;

              case Token.MUL:
                visitArithmetic(node, ByteCode.DMUL, child, parent);
                break;

              case Token.SUB:
                visitArithmetic(node, ByteCode.DSUB, child, parent);
                break;

              case Token.DIV:
              case Token.MOD:
                visitArithmetic(node, type == Token.DIV
                                      ? ByteCode.DDIV
                                      : ByteCode.DREM, child, parent);
                break;

              case Token.BITOR:
              case Token.BITXOR:
              case Token.BITAND:
              case Token.LSH:
              case Token.RSH:
              case Token.URSH:
                visitBitOp(node, type, child);
                break;

              case Token.POS:
              case Token.NEG:
                cfw.add(ByteCode.NEW, "java/lang/Double");
                cfw.add(ByteCode.DUP);
                generateCodeFromNode(child, node);
                addScriptRuntimeInvoke("toNumber", "(Ljava/lang/Object;)D");
                if (type == Token.NEG) {
                    cfw.add(ByteCode.DNEG);
                }
                addDoubleConstructor();
                break;

              case Optimizer.TO_DOUBLE:
                // cnvt to double (not Double)
                generateCodeFromNode(child, node);
                addScriptRuntimeInvoke("toNumber", "(Ljava/lang/Object;)D");
                break;

              case Optimizer.TO_OBJECT: {
                // convert from double
                int prop = -1;
                if (child.getType() == Token.NUMBER) {
                    prop = child.getIntProp(Node.ISNUMBER_PROP, -1);
                }
                if (prop != -1) {
                    child.removeProp(Node.ISNUMBER_PROP);
                    generateCodeFromNode(child, node);
                    child.putIntProp(Node.ISNUMBER_PROP, prop);
                } else {
                    cfw.add(ByteCode.NEW, "java/lang/Double");
                    cfw.add(ByteCode.DUP);
                    generateCodeFromNode(child, node);
                    addDoubleConstructor();
                }
                break;
              }

              case Token.IN:
              case Token.INSTANCEOF:
              case Token.LE:
              case Token.LT:
              case Token.GE:
              case Token.GT:
                // need a result Object
                visitRelOp(node, child);
                break;

              case Token.EQ:
              case Token.NE:
              case Token.SHEQ:
              case Token.SHNE:
                visitEqOp(node, child);
                break;

              case Token.GETPROP:
                visitGetProp(node, child);
                break;

              case Token.GETELEM:
                while (child != null) {
                    generateCodeFromNode(child, node);
                    child = child.getNext();
                }
                cfw.addALoad(variableObjectLocal);
                if (node.getIntProp(Node.ISNUMBER_PROP, -1) != -1) {
                    addOptRuntimeInvoke(
                        "getElem",
                        "(Ljava/lang/Object;D"
                        +"Lorg/mozilla/javascript/Scriptable;"
                        +")Ljava/lang/Object;");
                }
                else {
                    addScriptRuntimeInvoke(
                        "getElem",
                        "(Ljava/lang/Object;"
                        +"Ljava/lang/Object;"
                        +"Lorg/mozilla/javascript/Scriptable;"
                        +")Ljava/lang/Object;");
                }
                break;

              case Token.GETVAR: {
                OptLocalVariable lVar
                        = (OptLocalVariable)(node.getProp(Node.VARIABLE_PROP));
                visitGetVar(lVar,
                            node.getIntProp(Node.ISNUMBER_PROP, -1) != -1,
                            node.getString());
              }
              break;

              case Token.SETVAR:
                visitSetVar(node, child, true);
                break;

              case Token.SETNAME:
                visitSetName(node, child);
                break;

              case Token.SETPROP:
                visitSetProp(node, child);
                break;

              case Token.SETELEM:
                while (child != null) {
                    generateCodeFromNode(child, node);
                    child = child.getNext();
                }
                cfw.addALoad(variableObjectLocal);
                if (node.getIntProp(Node.ISNUMBER_PROP, -1) != -1) {
                    addOptRuntimeInvoke(
                        "setElem",
                        "(Ljava/lang/Object;"
                        +"D"
                        +"Ljava/lang/Object;"
                        +"Lorg/mozilla/javascript/Scriptable;"
                        +")Ljava/lang/Object;");
                }
                else {
                    addScriptRuntimeInvoke(
                        "setElem",
                        "(Ljava/lang/Object;"
                        +"Ljava/lang/Object;"
                        +"Ljava/lang/Object;"
                        +"Lorg/mozilla/javascript/Scriptable;"
                        +")Ljava/lang/Object;");
                }
                break;

              case Token.DELPROP:
                cfw.addALoad(contextLocal);
                cfw.addALoad(variableObjectLocal);
                while (child != null) {
                    generateCodeFromNode(child, node);
                    child = child.getNext();
                }
                addScriptRuntimeInvoke("delete",
                                       "(Lorg/mozilla/javascript/Context;"
                                       +"Lorg/mozilla/javascript/Scriptable;"
                                       +"Ljava/lang/Object;"
                                       +"Ljava/lang/Object;"
                                       +")Ljava/lang/Object;");
                break;

              case Token.BINDNAME:
              case Token.GETBASE:
                visitBind(node, type, child);
                break;

              case Token.GETTHIS:
                generateCodeFromNode(child, node);
                addScriptRuntimeInvoke("getThis",
                                       "(Lorg/mozilla/javascript/Scriptable;"
                                       +")Lorg/mozilla/javascript/Scriptable;");
                break;

              case Token.PARENT:
                generateCodeFromNode(child, node);
                addScriptRuntimeInvoke("getParent",
                                       "(Ljava/lang/Object;"
                                       +")Lorg/mozilla/javascript/Scriptable;");
                break;

              case Token.NEWTEMP:
                visitNewTemp(node, child);
                break;

              case Token.USETEMP:
                visitUseTemp(node, child);
                break;

              case Token.NEWLOCAL:
                visitNewLocal(node, child);
                break;

              case Token.USELOCAL:
                visitUseLocal(node, child);
                break;

              default:
                throw new RuntimeException("Unexpected node type "+type);
        }

    }

    private void generateIfJump(Node node, Node parent,
                                int trueLabel, int falseLabel)
    {
        // System.out.println("gen code for " + node.toString());

        int type = node.getType();
        Node child = node.getFirstChild();

        switch (type) {
          case Token.NOT:
            generateIfJump(child, node, falseLabel, trueLabel);
            break;

          case Token.OR:
          case Token.AND: {
            int interLabel = cfw.acquireLabel();
            if (type == Token.AND) {
                generateIfJump(child, node, interLabel, falseLabel);
            }
            else {
                generateIfJump(child, node, trueLabel, interLabel);
            }
            cfw.markLabel(interLabel);
            child = child.getNext();
            generateIfJump(child, node, trueLabel, falseLabel);
            break;
          }

          case Token.IN:
          case Token.INSTANCEOF:
          case Token.LE:
          case Token.LT:
          case Token.GE:
          case Token.GT:
            visitIfJumpRelOp(node, child, trueLabel, falseLabel);
            break;

          case Token.EQ:
          case Token.NE:
          case Token.SHEQ:
          case Token.SHNE:
            visitIfJumpEqOp(node, child, trueLabel, falseLabel);
            break;

          default:
            // Generate generic code for non-optimized jump
            generateCodeFromNode(node, parent);
            addScriptRuntimeInvoke("toBoolean", "(Ljava/lang/Object;)Z");
            cfw.add(ByteCode.IFNE, trueLabel);
            cfw.add(ByteCode.GOTO, falseLabel);
        }
    }

    private void visitFunction(OptFunctionNode fn, int functionType)
    {
        String fnClassName = codegen.functionClassName;
        int fnIndex = codegen.getIndex(fn);
        cfw.add(ByteCode.NEW, fnClassName);
        // Call function constructor
        cfw.add(ByteCode.DUP);
        cfw.addALoad(variableObjectLocal);
        cfw.addALoad(contextLocal);           // load 'cx'
        cfw.addPush(fnIndex);
        cfw.addInvoke(ByteCode.INVOKESPECIAL, fnClassName,
                      "<init>", Codegen.FUNCTION_CONSTRUCTOR_SIGNATURE);

        // Init mainScript field;
        cfw.add(ByteCode.DUP);
        if (isTopLevel) {
            cfw.add(ByteCode.ALOAD_0);
        } else {
            cfw.add(ByteCode.ALOAD_0);
            cfw.add(ByteCode.GETFIELD,
                    zeroArgClassName,
                    Codegen.DIRECT_CALL_PARENT_FIELD,
                    codegen.mainClassSignature);
        }
        cfw.add(ByteCode.PUTFIELD,
                fnClassName,
                Codegen.DIRECT_CALL_PARENT_FIELD,
                codegen.mainClassSignature);

        int directTargetIndex = fn.getDirectTargetIndex();
        if (directTargetIndex >= 0) {
            cfw.add(ByteCode.DUP);
            if (isTopLevel) {
                cfw.add(ByteCode.ALOAD_0);
            } else {
                cfw.add(ByteCode.ALOAD_0);
                cfw.add(ByteCode.GETFIELD,
                        zeroArgClassName,
                        Codegen.DIRECT_CALL_PARENT_FIELD,
                        codegen.mainClassSignature);
            }
            cfw.add(ByteCode.SWAP);
            cfw.add(ByteCode.PUTFIELD,
                    codegen.mainClassName,
                    Codegen.getDirectTargetFieldName(directTargetIndex),
                    codegen.functionClassSignature);
        }

        // Dup function reference for function expressions to have it
        // on top of the stack when initFunction returns
        if (functionType != FunctionNode.FUNCTION_STATEMENT) {
            cfw.add(ByteCode.DUP);
        }
        cfw.addPush(functionType);
        cfw.addALoad(variableObjectLocal);
        cfw.addALoad(contextLocal);           // load 'cx'
        addOptRuntimeInvoke("initFunction",
                            "(Lorg/mozilla/javascript/NativeFunction;"
                            +"I"
                            +"Lorg/mozilla/javascript/Scriptable;"
                            +"Lorg/mozilla/javascript/Context;"
                            +")V");
    }

    private void visitTarget(Node node)
    {
        int label = node.getIntProp(Node.LABEL_PROP, -1);
        if (label == -1) {
            label = cfw.acquireLabel();
            node.putIntProp(Node.LABEL_PROP, label);
        }
        cfw.markLabel(label);
    }

    private void visitGOTO(Node node, int type, Node child)
    {
        Node target = (Node)(node.getProp(Node.TARGET_PROP));
        int targetLabel = target.getIntProp(Node.LABEL_PROP, -1);
        if (targetLabel == -1) {
            targetLabel = cfw.acquireLabel();
            target.putIntProp(Node.LABEL_PROP, targetLabel);
        }
        int fallThruLabel = cfw.acquireLabel();

        if ((type == Token.IFEQ) || (type == Token.IFNE)) {
            if (child == null) {
                // can have a null child from visitSwitch which
                // has already generated the code for the child
                // and just needs the GOTO code emitted
                addScriptRuntimeInvoke("toBoolean",
                                       "(Ljava/lang/Object;)Z");
                if (type == Token.IFEQ)
                    cfw.add(ByteCode.IFNE, targetLabel);
                else
                    cfw.add(ByteCode.IFEQ, targetLabel);
            }
            else {
                if (type == Token.IFEQ)
                    generateIfJump(child, node, targetLabel, fallThruLabel);
                else
                    generateIfJump(child, node, fallThruLabel, targetLabel);
            }
        }
        else {
            while (child != null) {
                generateCodeFromNode(child, node);
                child = child.getNext();
            }
            if (type == Token.JSR)
                cfw.add(ByteCode.JSR, targetLabel);
            else
                cfw.add(ByteCode.GOTO, targetLabel);
        }
        cfw.markLabel(fallThruLabel);
    }

    private void visitEnumInit(Node node, Node child)
    {
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        cfw.addALoad(variableObjectLocal);
        addScriptRuntimeInvoke("initEnum",
                               "(Ljava/lang/Object;"
                               +"Lorg/mozilla/javascript/Scriptable;"
                               +")Ljava/lang/Object;");
        short x = getNewWordLocal();
        cfw.addAStore(x);
        node.putIntProp(Node.LOCAL_PROP, x);
    }

    private void visitEnumNext(Node node, Node child)
    {
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        Node init = (Node) node.getProp(Node.ENUM_PROP);
        int local = init.getExistingIntProp(Node.LOCAL_PROP);
        cfw.addALoad(local);
        addScriptRuntimeInvoke("nextEnum",
                               "(Ljava/lang/Object;)Ljava/lang/Object;");
    }

    private void visitEnumDone(Node node, Node child)
    {
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        Node init = (Node) node.getProp(Node.ENUM_PROP);
        int local = init.getExistingIntProp(Node.LOCAL_PROP);
        releaseWordLocal((short)local);
    }

    private void visitEnterWith(Node node, Node child)
    {
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        cfw.addALoad(variableObjectLocal);
        addScriptRuntimeInvoke("enterWith",
                               "(Ljava/lang/Object;"
                               +"Lorg/mozilla/javascript/Scriptable;"
                               +")Lorg/mozilla/javascript/Scriptable;");
        cfw.addAStore(variableObjectLocal);
    }

    private void visitLeaveWith(Node node, Node child)
    {
        cfw.addALoad(variableObjectLocal);
        addScriptRuntimeInvoke("leaveWith",
                               "(Lorg/mozilla/javascript/Scriptable;"
                               +")Lorg/mozilla/javascript/Scriptable;");
        cfw.addAStore(variableObjectLocal);
    }

    private void resetTargets(Node node)
    {
        if (node.getType() == Token.TARGET) {
            node.removeProp(Node.LABEL_PROP);
        }
        Node child = node.getFirstChild();
        while (child != null) {
            resetTargets(child);
            child = child.getNext();
        }
    }

    private void visitCall(Node node, int type, Node child)
    {
        /*
         * Generate code for call.
         */

        Node chelsea = child;      // remember the first child for later
        OptFunctionNode
            target = (OptFunctionNode)node.getProp(Node.DIRECTCALL_PROP);
        if (target != null) {
            generateCodeFromNode(child, node);
            int regularCall = cfw.acquireLabel();

            int directTargetIndex = target.getDirectTargetIndex();
            if (isTopLevel) {
                cfw.add(ByteCode.ALOAD_0);
            } else {
                cfw.add(ByteCode.ALOAD_0);
                cfw.add(ByteCode.GETFIELD, zeroArgClassName,
                        Codegen.DIRECT_CALL_PARENT_FIELD,
                        codegen.mainClassSignature);
            }
            cfw.add(ByteCode.GETFIELD, codegen.mainClassName,
                    Codegen.getDirectTargetFieldName(directTargetIndex),
                    codegen.functionClassSignature);

            short stackHeight = cfw.getStackTop();

            cfw.add(ByteCode.DUP2);
            cfw.add(ByteCode.IF_ACMPNE, regularCall);
            cfw.add(ByteCode.SWAP);
            cfw.add(ByteCode.POP);

            if (!codegen.itsUseDynamicScope) {
                cfw.add(ByteCode.DUP);
                cfw.addInvoke(ByteCode.INVOKEINTERFACE,
                              "org/mozilla/javascript/Scriptable",
                              "getParentScope",
                              "()Lorg/mozilla/javascript/Scriptable;");
            } else {
                cfw.addALoad(variableObjectLocal);
            }
            cfw.addALoad(contextLocal);
            cfw.add(ByteCode.SWAP);

            if (type == Token.NEW)
                cfw.add(ByteCode.ACONST_NULL);
            else {
                child = child.getNext();
                generateCodeFromNode(child, node);
            }
/*
    Remember that directCall parameters are paired in 1 aReg and 1 dReg
    If the argument is an incoming arg, just pass the orginal pair thru.
    Else, if the argument is known to be typed 'Number', pass Void.TYPE
    in the aReg and the number is the dReg
    Else pass the JS object in the aReg and 0.0 in the dReg.
*/
            child = child.getNext();
            while (child != null) {
                boolean handled = false;
                if ((child.getType() == Token.GETVAR)
                        && inDirectCallFunction) {
                    OptLocalVariable lVar
                        = (OptLocalVariable)(child.getProp(Node.VARIABLE_PROP));
                    if (lVar != null && lVar.isParameter()) {
                        handled = true;
                        cfw.addALoad(lVar.getJRegister());
                        cfw.addDLoad(lVar.getJRegister() + 1);
                    }
                }
                if (!handled) {
                    int childNumberFlag
                                = child.getIntProp(Node.ISNUMBER_PROP, -1);
                    if (childNumberFlag == Node.BOTH) {
                        cfw.add(ByteCode.GETSTATIC,
                                "java/lang/Void",
                                "TYPE",
                                "Ljava/lang/Class;");
                        generateCodeFromNode(child, node);
                    }
                    else {
                        generateCodeFromNode(child, node);
                        cfw.addPush(0.0);
                    }
                }
                resetTargets(child);
                child = child.getNext();
            }

            cfw.add(ByteCode.GETSTATIC,
                    "org/mozilla/javascript/ScriptRuntime",
                    "emptyArgs", "[Ljava/lang/Object;");
            cfw.addInvoke(ByteCode.INVOKESTATIC,
                          codegen.mainClassName,
                          (type == Token.NEW)
                              ? codegen.getDirectCtorName(target)
                              : codegen.getBodyMethodName(target),
                          codegen.getBodyMethodSignature(target));

            int beyond = cfw.acquireLabel();
            cfw.add(ByteCode.GOTO, beyond);
            cfw.markLabel(regularCall, stackHeight);
            cfw.add(ByteCode.POP);

            visitRegularCall(node, type, chelsea, true);
            cfw.markLabel(beyond);
        }
        else {
            visitRegularCall(node, type, chelsea, false);
        }
   }


    private String getSimpleCallName(Node callNode)
    {
    /*
        Find call trees that look like this :
        (they arise from simple function invocations)

            CALL                     <-- this is the callNode node
                GETPROP
                    NEWTEMP [USES: 1]
                        GETBASE 'name'
                    STRING 'name'
                GETTHIS
                    USETEMP [TEMP: NEWTEMP [USES: 1]]
                <-- arguments would be here

        and return the name found.

    */
        Node callBase = callNode.getFirstChild();
        if (callBase.getType() == Token.GETPROP) {
            Node callBaseChild = callBase.getFirstChild();
            if (callBaseChild.getType() == Token.NEWTEMP) {
                Node callBaseID = callBaseChild.getNext();
                Node tempChild = callBaseChild.getFirstChild();
                if (tempChild.getType() == Token.GETBASE) {
                    String functionName = tempChild.getString();
                    if (callBaseID != null
                        && callBaseID.getType() == Token.STRING)
                    {
                        if (functionName.equals(callBaseID.getString())) {
                            Node thisChild = callBase.getNext();
                            if (thisChild.getType() == Token.GETTHIS) {
                                Node useChild = thisChild.getFirstChild();
                                if (useChild.getType() == Token.USETEMP) {
                                    if (useChild.getProp(Node.TEMP_PROP)
                                         == callBaseChild)
                                    {
                                        return functionName;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return null;
    }

    private void constructArgArray(int argCount)
    {
        if (argCount == 0) {
            if (itsZeroArgArray >= 0)
                cfw.addALoad(itsZeroArgArray);
            else {
                cfw.add(ByteCode.GETSTATIC,
                        "org/mozilla/javascript/ScriptRuntime",
                        "emptyArgs", "[Ljava/lang/Object;");
            }
        }
        else {
            if (argCount == 1) {
                if (itsOneArgArray >= 0)
                    cfw.addALoad(itsOneArgArray);
                else {
                    cfw.addPush(1);
                    cfw.add(ByteCode.ANEWARRAY, "java/lang/Object");
                }
            }
            else {
                cfw.addPush(argCount);
                cfw.add(ByteCode.ANEWARRAY, "java/lang/Object");
            }
        }
    }

    private void visitRegularCall(Node node, int type,
                                  Node child, boolean firstArgDone)
    {
        /*
         * Generate code for call.
         *
         * push <arity>
         * anewarray Ljava/lang/Object;
         * // "initCount" instances of code from here...
         * dup
         * push <i>
         * <gen code for child>
         * acfw.addAStore
         * //...to here
         * invokestatic call
         */

        OptFunctionNode target = (OptFunctionNode)node.getProp(Node.DIRECTCALL_PROP);
        Node chelsea = child;
        int childCount = 0;
        int argSkipCount = (type == Token.NEW) ? 1 : 2;
        while (child != null) {
            childCount++;
            child = child.getNext();
        }

        child = chelsea;    // re-start the iterator from the first child,
                    // REMIND - too bad we couldn't just back-patch the count ?

        int argIndex = -argSkipCount;
        if (firstArgDone && (child != null)) {
            child = child.getNext();
            argIndex++;
            cfw.addALoad(contextLocal);
            cfw.add(ByteCode.SWAP);
        }
        else
            cfw.addALoad(contextLocal);

        if (firstArgDone && (type == Token.NEW))
            constructArgArray(childCount - argSkipCount);

        int callType = node.getIntProp(Node.SPECIALCALL_PROP,
                                       Node.NON_SPECIALCALL);
        boolean isSimpleCall = false;
        if (!firstArgDone && type != Token.NEW) {
            String simpleCallName = getSimpleCallName(node);
            if (simpleCallName != null && callType == Node.NON_SPECIALCALL) {
                isSimpleCall = true;
                cfw.addPush(simpleCallName);
                cfw.addALoad(variableObjectLocal);
                child = child.getNext().getNext();
                argIndex = 0;
                constructArgArray(childCount - argSkipCount);
            }
        }

        while (child != null) {
            if (argIndex < 0)       // not moving these arguments to the array
                generateCodeFromNode(child, node);
            else {
                cfw.add(ByteCode.DUP);
                cfw.addPush(argIndex);
                if (target != null) {
/*
    If this has also been a directCall sequence, the Number flag will
    have remained set for any parameter so that the values could be
    copied directly into the outgoing args. Here we want to force it
    to be treated as not in a Number context, so we set the flag off.
*/
                    boolean handled = false;
                    if ((child.getType() == Token.GETVAR)
                            && inDirectCallFunction) {
                        OptLocalVariable lVar
                          = (OptLocalVariable)(child.getProp(Node.VARIABLE_PROP));
                        if (lVar != null && lVar.isParameter()) {
                            child.removeProp(Node.ISNUMBER_PROP);
                            generateCodeFromNode(child, node);
                            handled = true;
                        }
                    }
                    if (!handled) {
                        int childNumberFlag
                                = child.getIntProp(Node.ISNUMBER_PROP, -1);
                        if (childNumberFlag == Node.BOTH) {
                            cfw.add(ByteCode.NEW,"java/lang/Double");
                            cfw.add(ByteCode.DUP);
                            generateCodeFromNode(child, node);
                            addDoubleConstructor();
                        }
                        else
                            generateCodeFromNode(child, node);
                    }
                }
                else
                    generateCodeFromNode(child, node);
                cfw.add(ByteCode.AASTORE);
            }
            argIndex++;
            if (argIndex == 0) {
                constructArgArray(childCount - argSkipCount);
            }
            child = child.getNext();
        }

        String className;
        String methodName;
        String callSignature;

        if (callType != Node.NON_SPECIALCALL) {
            className = "org/mozilla/javascript/optimizer/OptRuntime";
            if (type == Token.NEW) {
                methodName = "newObjectSpecial";
                callSignature = "(Lorg/mozilla/javascript/Context;"
                                +"Ljava/lang/Object;"
                                +"[Ljava/lang/Object;"
                                +"Lorg/mozilla/javascript/Scriptable;"
                                +"Lorg/mozilla/javascript/Scriptable;"
                                +"I" // call type
                                +")Ljava/lang/Object;";
                cfw.addALoad(variableObjectLocal);
                cfw.addALoad(thisObjLocal);
                cfw.addPush(callType);
            } else {
                methodName = "callSpecial";
                callSignature = "(Lorg/mozilla/javascript/Context;"
                                +"Ljava/lang/Object;"
                                +"Ljava/lang/Object;"
                                +"[Ljava/lang/Object;"
                                +"Lorg/mozilla/javascript/Scriptable;"
                                +"Lorg/mozilla/javascript/Scriptable;"
                                +"I" // call type
                                +"Ljava/lang/String;I"  // filename, linenumber
                                +")Ljava/lang/Object;";
                cfw.addALoad(variableObjectLocal);
                cfw.addALoad(thisObjLocal);
                cfw.addPush(callType);
                String sourceName = scriptOrFn.getSourceName();
                cfw.addPush(sourceName == null ? "" : sourceName);
                cfw.addPush(itsLineNumber);
            }
        } else if (isSimpleCall) {
            className = "org/mozilla/javascript/optimizer/OptRuntime";
            methodName = "callSimple";
            callSignature = "(Lorg/mozilla/javascript/Context;"
                            +"Ljava/lang/String;"
                            +"Lorg/mozilla/javascript/Scriptable;"
                            +"[Ljava/lang/Object;"
                            +")Ljava/lang/Object;";
        } else {
            className = "org/mozilla/javascript/ScriptRuntime";
            cfw.addALoad(variableObjectLocal);
            if (type == Token.NEW) {
                methodName = "newObject";
                callSignature = "(Lorg/mozilla/javascript/Context;"
                                +"Ljava/lang/Object;"
                                +"[Ljava/lang/Object;"
                                +"Lorg/mozilla/javascript/Scriptable;"
                                +")Lorg/mozilla/javascript/Scriptable;";
            } else {
                methodName = "call";
                callSignature = "(Lorg/mozilla/javascript/Context;"
                                 +"Ljava/lang/Object;"
                                 +"Ljava/lang/Object;"
                                 +"[Ljava/lang/Object;"
                                 +"Lorg/mozilla/javascript/Scriptable;"
                                 +")Ljava/lang/Object;";
            }
        }

        cfw.addInvoke(ByteCode.INVOKESTATIC,
                      className, methodName, callSignature);
    }

    private void visitStatement(Node node)
    {
        itsLineNumber = node.getLineno();
        if (itsLineNumber == -1)
            return;
        cfw.addLineNumberEntry((short)itsLineNumber);
    }


    private void visitTryCatchFinally(Node node, Node child)
    {
        /* Save the variable object, in case there are with statements
         * enclosed by the try block and we catch some exception.
         * We'll restore it for the catch block so that catch block
         * statements get the right scope.
         */

        // OPT we only need to do this if there are enclosed WITH
        // statements; could statically check and omit this if there aren't any.

        // XXX OPT Maybe instead do syntactic transforms to associate
        // each 'with' with a try/finally block that does the exitwith.

        // For that matter:  Why do we have leavewith?

        // XXX does Java have any kind of MOV(reg, reg)?
        short savedVariableObject = getNewWordLocal();
        cfw.addALoad(variableObjectLocal);
        cfw.addAStore(savedVariableObject);

        /*
         * Generate the code for the tree; most of the work is done in IRFactory
         * and NodeTransformer;  Codegen just adds the java handlers for the
         * javascript catch and finally clauses.  */
        // need to set the stack top to 1 to account for the incoming exception
        int startLabel = cfw.acquireLabel();
        cfw.markLabel(startLabel, (short)1);

        visitStatement(node);
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }

        Node catchTarget = (Node)node.getProp(Node.TARGET_PROP);
        Node finallyTarget = (Node)node.getProp(Node.FINALLY_PROP);

        // control flow skips the handlers
        int realEnd = cfw.acquireLabel();
        cfw.add(ByteCode.GOTO, realEnd);


        // javascript handler; unwrap exception and GOTO to javascript
        // catch area.
        if (catchTarget != null) {
            // get the label to goto
            int catchLabel = catchTarget.getExistingIntProp(Node.LABEL_PROP);

            generateCatchBlock(JAVASCRIPT_EXCEPTION, savedVariableObject,
                               catchLabel, startLabel);
            /*
             * catch WrappedExceptions, see if they are wrapped
             * JavaScriptExceptions. Otherwise, rethrow.
             */
            generateCatchBlock(WRAPPED_EXCEPTION, savedVariableObject,
                               catchLabel, startLabel);

            /*
                we also need to catch EcmaErrors and feed the
                associated error object to the handler
            */
            generateCatchBlock(ECMAERROR_EXCEPTION, savedVariableObject,
                               catchLabel, startLabel);
        }

        // finally handler; catch all exceptions, store to a local; JSR to
        // the finally, then re-throw.
        if (finallyTarget != null) {
            int finallyHandler = cfw.acquireLabel();
            cfw.markHandler(finallyHandler);

            // reset the variable object local
            cfw.addALoad(savedVariableObject);
            cfw.addAStore(variableObjectLocal);

            short exnLocal = itsLocalAllocationBase++;
            cfw.addAStore(exnLocal);

            // get the label to JSR to
            int finallyLabel =
                finallyTarget.getExistingIntProp(Node.LABEL_PROP);
            cfw.add(ByteCode.JSR, finallyLabel);

            // rethrow
            cfw.addALoad(exnLocal);
            cfw.add(ByteCode.ATHROW);

            // mark the handler
            cfw.addExceptionHandler(startLabel, finallyLabel,
                                          finallyHandler, null); // catch any
        }
        releaseWordLocal(savedVariableObject);
        cfw.markLabel(realEnd);
    }

    private final int JAVASCRIPT_EXCEPTION = 0;
    private final int WRAPPED_EXCEPTION    = 2;
    private final int ECMAERROR_EXCEPTION  = 3;

    private void generateCatchBlock(int exceptionType,
                                    short savedVariableObject,
                                    int catchLabel,
                                    int startLabel)
    {
        int handler = cfw.acquireLabel();
        cfw.markHandler(handler);

        // MS JVM gets cranky if the exception object is left on the stack
        short exceptionObject = getNewWordLocal();
        cfw.addAStore(exceptionObject);

        // reset the variable object local
        cfw.addALoad(savedVariableObject);
        cfw.addAStore(variableObjectLocal);

        cfw.addALoad(contextLocal);
        cfw.addALoad(variableObjectLocal);
        cfw.addALoad(exceptionObject);
        releaseWordLocal(exceptionObject);

        // unwrap the exception...
        addScriptRuntimeInvoke(
            "getCatchObject",
            "(Lorg/mozilla/javascript/Context;"
            +"Lorg/mozilla/javascript/Scriptable;"
            +"Ljava/lang/Throwable;"
            +")Ljava/lang/Object;");

        String exceptionName;

        if (exceptionType == JAVASCRIPT_EXCEPTION) {
            exceptionName = "org/mozilla/javascript/JavaScriptException";
        } else if (exceptionType == WRAPPED_EXCEPTION) {
            exceptionName = "org/mozilla/javascript/WrappedException";
        } else {
            if (exceptionType != ECMAERROR_EXCEPTION) Context.codeBug();
            exceptionName = "org/mozilla/javascript/EcmaError";
        }

        // mark the handler
        cfw.addExceptionHandler(startLabel, catchLabel, handler,
                                      exceptionName);

        cfw.add(ByteCode.GOTO, catchLabel);
    }

    private void visitThrow(Node node, Node child)
    {
        visitStatement(node);
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }

        cfw.add(ByteCode.NEW,
                      "org/mozilla/javascript/JavaScriptException");
        cfw.add(ByteCode.DUP_X1);
        cfw.add(ByteCode.SWAP);
        cfw.addPush(scriptOrFn.getSourceName());
        cfw.addPush(itsLineNumber);
        cfw.addInvoke(ByteCode.INVOKESPECIAL,
                      "org/mozilla/javascript/JavaScriptException",
                      "<init>",
                      "(Ljava/lang/Object;Ljava/lang/String;I)V");

        cfw.add(ByteCode.ATHROW);
    }

    private void visitReturn(Node node, Node child)
    {
        visitStatement(node);
        if (child != null) {
            do {
                generateCodeFromNode(child, node);
                child = child.getNext();
            } while (child != null);
        } else if (fnCurrent != null) {
            Codegen.pushUndefined(cfw);
        } else {
            cfw.addALoad(scriptResultLocal);
        }

        if (epilogueLabel == -1)
            epilogueLabel = cfw.acquireLabel();
        cfw.add(ByteCode.GOTO, epilogueLabel);
    }

    private void visitSwitch(Node node, Node child)
    {
        visitStatement(node);
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }

        // save selector value
        short selector = getNewWordLocal();
        cfw.addAStore(selector);

        ObjArray cases = (ObjArray) node.getProp(Node.CASES_PROP);
        for (int i=0; i < cases.size(); i++) {
            Node thisCase = (Node) cases.get(i);
            Node first = thisCase.getFirstChild();
            generateCodeFromNode(first, thisCase);
            cfw.addALoad(selector);
            addScriptRuntimeInvoke("seqB",
                                   "(Ljava/lang/Object;"
                                   +"Ljava/lang/Object;"
                                   +")Ljava/lang/Boolean;");
            Node target = new Node(Token.TARGET);
            thisCase.replaceChild(first, target);
            generateGOTO(Token.IFEQ, target);
        }

        Node defaultNode = (Node) node.getProp(Node.DEFAULT_PROP);
        if (defaultNode != null) {
            Node defaultTarget = new Node(Token.TARGET);
            defaultNode.getFirstChild().addChildToFront(defaultTarget);
            generateGOTO(Token.GOTO, defaultTarget);
        }

        Node breakTarget = (Node) node.getProp(Node.BREAK_PROP);
        generateGOTO(Token.GOTO, breakTarget);
    }

    private void generateGOTO(int type, Node target)
    {
        Node GOTO = new Node(type);
        GOTO.putProp(Node.TARGET_PROP, target);
        visitGOTO(GOTO, type, null);
    }

    private void visitTypeofname(Node node)
    {
        String name = node.getString();
        if (hasVarsInRegs) {
            OptLocalVariable lVar = fnCurrent.getVar(name);
            if (lVar != null) {
                if (lVar.isNumber()) {
                    cfw.addPush("number");
                    return;
                }
                visitGetVar(lVar, false, name);
                addScriptRuntimeInvoke("typeof",
                                       "(Ljava/lang/Object;"
                                       +")Ljava/lang/String;");
                return;
            }
        }
        cfw.addALoad(variableObjectLocal);
        cfw.addPush(name);
        addScriptRuntimeInvoke("typeofName",
                               "(Lorg/mozilla/javascript/Scriptable;"
                               +"Ljava/lang/String;"
                               +")Ljava/lang/String;");
    }

    private void visitIncDec(Node node, boolean isInc)
    {
        Node child = node.getFirstChild();
        if (node.getIntProp(Node.ISNUMBER_PROP, -1) != -1) {
            OptLocalVariable lVar
                    = (OptLocalVariable)(child.getProp(Node.VARIABLE_PROP));
            if (lVar.getJRegister() == -1)
                lVar.assignJRegister(getNewWordPairLocal());
            cfw.addDLoad(lVar.getJRegister());
            cfw.add(ByteCode.DUP2);
            cfw.addPush(1.0);
            cfw.add((isInc) ? ByteCode.DADD : ByteCode.DSUB);
            cfw.addDStore(lVar.getJRegister());
        } else {
            OptLocalVariable lVar
                    = (OptLocalVariable)(child.getProp(Node.VARIABLE_PROP));
            String routine = (isInc) ? "postIncrement" : "postDecrement";
            int childType = child.getType();
            if (hasVarsInRegs && childType == Token.GETVAR) {
                if (lVar == null)
                    lVar = fnCurrent.getVar(child.getString());
                if (lVar.getJRegister() == -1)
                    lVar.assignJRegister(getNewWordLocal());
                cfw.addALoad(lVar.getJRegister());
                cfw.add(ByteCode.DUP);
                addScriptRuntimeInvoke(routine,
                                       "(Ljava/lang/Object;"
                                       +")Ljava/lang/Object;");
                cfw.addAStore(lVar.getJRegister());
            } else if (childType == Token.GETPROP) {
                Node getPropChild = child.getFirstChild();
                generateCodeFromNode(getPropChild, node);
                generateCodeFromNode(getPropChild.getNext(), node);
                cfw.addALoad(variableObjectLocal);
                addScriptRuntimeInvoke(routine,
                                       "(Ljava/lang/Object;"
                                       +"Ljava/lang/String;"
                                       +"Lorg/mozilla/javascript/Scriptable;"
                                       +")Ljava/lang/Object;");
            } else if (childType == Token.GETELEM) {
                routine += "Elem";
                Node getPropChild = child.getFirstChild();
                generateCodeFromNode(getPropChild, node);
                generateCodeFromNode(getPropChild.getNext(), node);
                cfw.addALoad(variableObjectLocal);
                addScriptRuntimeInvoke(routine,
                                       "(Ljava/lang/Object;"
                                       +"Ljava/lang/Object;"
                                       +"Lorg/mozilla/javascript/Scriptable;"
                                       +")Ljava/lang/Object;");
            } else {
                cfw.addALoad(variableObjectLocal);
                cfw.addPush(child.getString());          // push name
                addScriptRuntimeInvoke(routine,
                                       "(Lorg/mozilla/javascript/Scriptable;"
                                       +"Ljava/lang/String;"
                                       +")Ljava/lang/Object;");
            }
        }
    }

    private static boolean isArithmeticNode(Node node)
    {
        int type = node.getType();
        return (type == Token.SUB)
                  || (type == Token.MOD)
                        || (type == Token.DIV)
                              || (type == Token.MUL);
    }

    private void visitArithmetic(Node node, byte opCode, Node child,
                                 Node parent)
    {
        int childNumberFlag = node.getIntProp(Node.ISNUMBER_PROP, -1);
        if (childNumberFlag != -1) {
            generateCodeFromNode(child, node);
            generateCodeFromNode(child.getNext(), node);
            cfw.add(opCode);
        }
        else {
            boolean childOfArithmetic = isArithmeticNode(parent);
            if (!childOfArithmetic) {
                cfw.add(ByteCode.NEW, "java/lang/Double");
                cfw.add(ByteCode.DUP);
            }
            generateCodeFromNode(child, node);
            if (!isArithmeticNode(child))
                addScriptRuntimeInvoke("toNumber", "(Ljava/lang/Object;)D");
            generateCodeFromNode(child.getNext(), node);
            if (!isArithmeticNode(child.getNext()))
                  addScriptRuntimeInvoke("toNumber", "(Ljava/lang/Object;)D");
            cfw.add(opCode);
            if (!childOfArithmetic) {
                addDoubleConstructor();
            }
        }
    }

    private void visitBitOp(Node node, int type, Node child)
    {
        int childNumberFlag = node.getIntProp(Node.ISNUMBER_PROP, -1);
        if (childNumberFlag == -1) {
            cfw.add(ByteCode.NEW, "java/lang/Double");
            cfw.add(ByteCode.DUP);
        }
        generateCodeFromNode(child, node);

        // special-case URSH; work with the target arg as a long, so
        // that we can return a 32-bit unsigned value, and call
        // toUint32 instead of toInt32.
        if (type == Token.URSH) {
            addScriptRuntimeInvoke("toUint32", "(Ljava/lang/Object;)J");
            generateCodeFromNode(child.getNext(), node);
            addScriptRuntimeInvoke("toInt32", "(Ljava/lang/Object;)I");
            // Looks like we need to explicitly mask the shift to 5 bits -
            // LUSHR takes 6 bits.
            cfw.addPush(31);
            cfw.add(ByteCode.IAND);
            cfw.add(ByteCode.LUSHR);
            cfw.add(ByteCode.L2D);
            addDoubleConstructor();
            return;
        }
        if (childNumberFlag == -1) {
            addScriptRuntimeInvoke("toInt32", "(Ljava/lang/Object;)I");
            generateCodeFromNode(child.getNext(), node);
            addScriptRuntimeInvoke("toInt32", "(Ljava/lang/Object;)I");
        }
        else {
            addScriptRuntimeInvoke("toInt32", "(D)I");
            generateCodeFromNode(child.getNext(), node);
            addScriptRuntimeInvoke("toInt32", "(D)I");
        }
        switch (type) {
          case Token.BITOR:
            cfw.add(ByteCode.IOR);
            break;
          case Token.BITXOR:
            cfw.add(ByteCode.IXOR);
            break;
          case Token.BITAND:
            cfw.add(ByteCode.IAND);
            break;
          case Token.RSH:
            cfw.add(ByteCode.ISHR);
            break;
          case Token.LSH:
            cfw.add(ByteCode.ISHL);
            break;
          default:
            Codegen.badTree();
        }
        cfw.add(ByteCode.I2D);
        if (childNumberFlag == -1) {
            addDoubleConstructor();
        }
    }

    private boolean nodeIsDirectCallParameter(Node node)
    {
        if (node.getType() == Token.GETVAR) {
            OptLocalVariable lVar
                    = (OptLocalVariable)(node.getProp(Node.VARIABLE_PROP));
            if (lVar != null && lVar.isParameter() && inDirectCallFunction &&
                !itsForcedObjectParameters)
            {
                return true;
            }
        }
        return false;
    }

    private void genSimpleCompare(int type, int trueGOTO, int falseGOTO)
    {
        switch (type) {
            case Token.LE :
                cfw.add(ByteCode.DCMPG);
                cfw.add(ByteCode.IFLE, trueGOTO);
                break;
            case Token.GE :
                cfw.add(ByteCode.DCMPL);
                cfw.add(ByteCode.IFGE, trueGOTO);
                break;
            case Token.LT :
                cfw.add(ByteCode.DCMPG);
                cfw.add(ByteCode.IFLT, trueGOTO);
                break;
            case Token.GT :
                cfw.add(ByteCode.DCMPL);
                cfw.add(ByteCode.IFGT, trueGOTO);
                break;
            default :
                Codegen.badTree();

        }
        if (falseGOTO != -1)
            cfw.add(ByteCode.GOTO, falseGOTO);
    }

    private void visitIfJumpRelOp(Node node, Node child,
                                  int trueGOTO, int falseGOTO)
    {
        int type = node.getType();
        if (type == Token.INSTANCEOF || type == Token.IN) {
            generateCodeFromNode(child, node);
            generateCodeFromNode(child.getNext(), node);
            cfw.addALoad(variableObjectLocal);
            addScriptRuntimeInvoke(
                (type == Token.INSTANCEOF) ? "instanceOf" : "in",
                "(Ljava/lang/Object;"
                +"Ljava/lang/Object;"
                +"Lorg/mozilla/javascript/Scriptable;"
                +")Z");
            cfw.add(ByteCode.IFNE, trueGOTO);
            cfw.add(ByteCode.GOTO, falseGOTO);
            return;
        }
        int childNumberFlag = node.getIntProp(Node.ISNUMBER_PROP, -1);
        if (childNumberFlag == Node.BOTH) {
            generateCodeFromNode(child, node);
            generateCodeFromNode(child.getNext(), node);
            genSimpleCompare(type, trueGOTO, falseGOTO);
        } else {
            Node rChild = child.getNext();
            boolean leftIsDCP = nodeIsDirectCallParameter(child);
            boolean rightIsDCP = nodeIsDirectCallParameter(rChild);
            if (leftIsDCP || rightIsDCP) {
                if (leftIsDCP) {
                    if (rightIsDCP) {
                        OptLocalVariable lVar1, lVar2;
                        lVar1 = (OptLocalVariable)child.getProp(
                                    Node.VARIABLE_PROP);
                        cfw.addALoad(lVar1.getJRegister());
                        cfw.add(ByteCode.GETSTATIC,
                                "java/lang/Void",
                                "TYPE",
                                "Ljava/lang/Class;");
                        int notNumbersLabel = cfw.acquireLabel();
                        cfw.add(ByteCode.IF_ACMPNE, notNumbersLabel);
                        lVar2 = (OptLocalVariable)rChild.getProp(
                                    Node.VARIABLE_PROP);
                        cfw.addALoad(lVar2.getJRegister());
                        cfw.add(ByteCode.GETSTATIC,
                                "java/lang/Void",
                                "TYPE",
                                "Ljava/lang/Class;");
                        cfw.add(ByteCode.IF_ACMPNE, notNumbersLabel);
                        cfw.addDLoad(lVar1.getJRegister() + 1);
                        cfw.addDLoad(lVar2.getJRegister() + 1);
                        genSimpleCompare(type, trueGOTO, falseGOTO);
                        cfw.markLabel(notNumbersLabel);
                        // fall thru to generic handling
                    } else {
                        // just the left child is a DCP, if the right child
                        // is a number it's worth testing the left
                        if (childNumberFlag == Node.RIGHT) {
                            OptLocalVariable lVar1;
                            lVar1 = (OptLocalVariable)child.getProp(
                                        Node.VARIABLE_PROP);
                            cfw.addALoad(lVar1.getJRegister());
                            cfw.add(ByteCode.GETSTATIC,
                                    "java/lang/Void",
                                    "TYPE",
                                    "Ljava/lang/Class;");
                            int notNumbersLabel = cfw.acquireLabel();
                            cfw.add(ByteCode.IF_ACMPNE,
                                        notNumbersLabel);
                            cfw.addDLoad(lVar1.getJRegister() + 1);
                            generateCodeFromNode(rChild, node);
                            genSimpleCompare(type, trueGOTO, falseGOTO);
                            cfw.markLabel(notNumbersLabel);
                            // fall thru to generic handling
                        }
                    }
                } else {
                    //  just the right child is a DCP, if the left child
                    //  is a number it's worth testing the right
                    if (childNumberFlag == Node.LEFT) {
                        OptLocalVariable lVar2;
                        lVar2 = (OptLocalVariable)rChild.getProp(
                                    Node.VARIABLE_PROP);
                        cfw.addALoad(lVar2.getJRegister());
                        cfw.add(ByteCode.GETSTATIC,
                                "java/lang/Void",
                                "TYPE",
                                "Ljava/lang/Class;");
                        int notNumbersLabel = cfw.acquireLabel();
                        cfw.add(ByteCode.IF_ACMPNE, notNumbersLabel);
                        generateCodeFromNode(child, node);
                        cfw.addDLoad(lVar2.getJRegister() + 1);
                        genSimpleCompare(type, trueGOTO, falseGOTO);
                        cfw.markLabel(notNumbersLabel);
                        // fall thru to generic handling
                    }
                }
            }
            generateCodeFromNode(child, node);
            generateCodeFromNode(rChild, node);
            if (childNumberFlag == -1) {
                if (type == Token.GE || type == Token.GT) {
                    cfw.add(ByteCode.SWAP);
                }
                String routine = ((type == Token.LT)
                          || (type == Token.GT)) ? "cmp_LT" : "cmp_LE";
                addScriptRuntimeInvoke(routine,
                                       "(Ljava/lang/Object;"
                                       +"Ljava/lang/Object;"
                                       +")I");
            } else {
                boolean doubleThenObject = (childNumberFlag == Node.LEFT);
                if (type == Token.GE || type == Token.GT) {
                    if (doubleThenObject) {
                        cfw.add(ByteCode.DUP_X2);
                        cfw.add(ByteCode.POP);
                        doubleThenObject = false;
                    } else {
                        cfw.add(ByteCode.DUP2_X1);
                        cfw.add(ByteCode.POP2);
                        doubleThenObject = true;
                    }
                }
                String routine = ((type == Token.LT)
                         || (type == Token.GT)) ? "cmp_LT" : "cmp_LE";
                if (doubleThenObject)
                    addOptRuntimeInvoke(routine, "(DLjava/lang/Object;)I");
                else
                    addOptRuntimeInvoke(routine, "(Ljava/lang/Object;D)I");
            }
            cfw.add(ByteCode.IFNE, trueGOTO);
            cfw.add(ByteCode.GOTO, falseGOTO);
        }
    }

    private void visitRelOp(Node node, Node child)
    {
        /*
            this is the version that returns an Object result
        */
        int type = node.getType();
        if (type == Token.INSTANCEOF || type == Token.IN) {
            generateCodeFromNode(child, node);
            generateCodeFromNode(child.getNext(), node);
            cfw.addALoad(variableObjectLocal);
            addScriptRuntimeInvoke(
                (type == Token.INSTANCEOF) ? "instanceOf" : "in",
                "(Ljava/lang/Object;"
                +"Ljava/lang/Object;"
                +"Lorg/mozilla/javascript/Scriptable;"
                +")Z");
            int trueGOTO = cfw.acquireLabel();
            int skip = cfw.acquireLabel();
            cfw.add(ByteCode.IFNE, trueGOTO);
            cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                    "FALSE", "Ljava/lang/Boolean;");
            cfw.add(ByteCode.GOTO, skip);
            cfw.markLabel(trueGOTO);
            cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                    "TRUE", "Ljava/lang/Boolean;");
            cfw.markLabel(skip);
            cfw.adjustStackTop(-1);   // only have 1 of true/false
            return;
        }

        int childNumberFlag = node.getIntProp(Node.ISNUMBER_PROP, -1);
        if (childNumberFlag == Node.BOTH) {
            generateCodeFromNode(child, node);
            generateCodeFromNode(child.getNext(), node);
            int trueGOTO = cfw.acquireLabel();
            int skip = cfw.acquireLabel();
            genSimpleCompare(type, trueGOTO, -1);
            cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                    "FALSE", "Ljava/lang/Boolean;");
            cfw.add(ByteCode.GOTO, skip);
            cfw.markLabel(trueGOTO);
            cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                    "TRUE", "Ljava/lang/Boolean;");
            cfw.markLabel(skip);
            cfw.adjustStackTop(-1);   // only have 1 of true/false
        }
        else {
            String routine = (type == Token.LT || type == Token.GT)
                             ? "cmp_LTB" : "cmp_LEB";
            generateCodeFromNode(child, node);
            generateCodeFromNode(child.getNext(), node);
            if (childNumberFlag == -1) {
                if (type == Token.GE || type == Token.GT) {
                    cfw.add(ByteCode.SWAP);
                }
                addScriptRuntimeInvoke(routine,
                                       "(Ljava/lang/Object;"
                                       +"Ljava/lang/Object;"
                                       +")Ljava/lang/Boolean;");
            }
            else {
                boolean doubleThenObject = (childNumberFlag == Node.LEFT);
                if (type == Token.GE || type == Token.GT) {
                    if (doubleThenObject) {
                        cfw.add(ByteCode.DUP_X2);
                        cfw.add(ByteCode.POP);
                        doubleThenObject = false;
                    }
                    else {
                        cfw.add(ByteCode.DUP2_X1);
                        cfw.add(ByteCode.POP2);
                        doubleThenObject = true;
                    }
                }
                if (doubleThenObject)
                    addOptRuntimeInvoke(routine,
                                        "(DLjava/lang/Object;"
                                        +")Ljava/lang/Boolean;");
                else
                    addOptRuntimeInvoke(routine,
                                        "(Ljava/lang/Object;D"
                                        +")Ljava/lang/Boolean;");
            }
        }
    }

    private Node getConvertToObjectOfNumberNode(Node node)
    {
        if (node.getType() == Optimizer.TO_OBJECT) {
            Node convertChild = node.getFirstChild();
            if (convertChild.getType() == Token.NUMBER) {
                return convertChild;
            }
        }
        return null;
    }

    private void visitEqOp(Node node, Node child)
    {
        int type = node.getType();
        Node rightChild = child.getNext();
        boolean isStrict = type == Token.SHEQ ||
                           type == Token.SHNE;
        if (rightChild.getType() == Token.NULL) {
            generateCodeFromNode(child, node);
            if (isStrict) {
                cfw.add(ByteCode.IFNULL, 9);
            } else {
                cfw.add(ByteCode.DUP);
                cfw.add(ByteCode.IFNULL, 15);
                Codegen.pushUndefined(cfw);
                cfw.add(ByteCode.IF_ACMPEQ, 10);
            }
            if ((type == Token.EQ) || (type == Token.SHEQ))
                cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                        "FALSE", "Ljava/lang/Boolean;");
            else
                cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                        "TRUE", "Ljava/lang/Boolean;");
            if (isStrict) {
                cfw.add(ByteCode.GOTO, 6);
            } else {
                cfw.add(ByteCode.GOTO, 7);
                cfw.add(ByteCode.POP);
            }
            if ((type == Token.EQ) || (type == Token.SHEQ))
                cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                        "TRUE", "Ljava/lang/Boolean;");
            else
                cfw.add(ByteCode.GETSTATIC, "java/lang/Boolean",
                                        "FALSE", "Ljava/lang/Boolean;");
            return;
        }

        generateCodeFromNode(child, node);
        generateCodeFromNode(child.getNext(), node);

        String name;
        switch (type) {
          case Token.EQ:
            name = "eqB";
            break;

          case Token.NE:
            name = "neB";
            break;

          case Token.SHEQ:
            name = "seqB";
            break;

          case Token.SHNE:
            name = "sneB";
            break;

          default:
            name = null;
            Codegen.badTree();
        }
        addScriptRuntimeInvoke(name,
                               "(Ljava/lang/Object;"
                               +"Ljava/lang/Object;"
                               +")Ljava/lang/Boolean;");
    }

    private void visitIfJumpEqOp(Node node, Node child,
                                 int trueGOTO, int falseGOTO)
    {
        int type = node.getType();
        Node rightChild = child.getNext();
        boolean isStrict = type == Token.SHEQ ||
                           type == Token.SHNE;

        if (rightChild.getType() == Token.NULL) {
            if (type != Token.EQ && type != Token.SHEQ) {
                // invert true and false.
                int temp = trueGOTO;
                trueGOTO = falseGOTO;
                falseGOTO = temp;
            }

            generateCodeFromNode(child, node);
            if (isStrict) {
                cfw.add(ByteCode.IFNULL, trueGOTO);
                cfw.add(ByteCode.GOTO, falseGOTO);
                return;
            }
            /*
                since we have to test for null && undefined we end up
                having to push the operand twice and so have to GOTO to
                a pop site if the first test passes.
                We can avoid that for operands that are 'simple', i.e.
                don't generate a lot of code and don't have side-effects.
                For now, 'simple' means GETVAR
            */
            boolean simpleChild = (child.getType() == Token.GETVAR);
            if (!simpleChild) cfw.add(ByteCode.DUP);
            int popGOTO = cfw.acquireLabel();
            cfw.add(ByteCode.IFNULL,
                            (simpleChild) ? trueGOTO : popGOTO);
            short popStack = cfw.getStackTop();
            if (simpleChild) generateCodeFromNode(child, node);
            Codegen.pushUndefined(cfw);
            cfw.add(ByteCode.IF_ACMPEQ, trueGOTO);
            cfw.add(ByteCode.GOTO, falseGOTO);
            if (!simpleChild) {
                cfw.markLabel(popGOTO, popStack);
                cfw.add(ByteCode.POP);
                cfw.add(ByteCode.GOTO, trueGOTO);
            }
            return;
        }

        Node rChild = child.getNext();

        if (nodeIsDirectCallParameter(child)) {
            Node convertChild = getConvertToObjectOfNumberNode(rChild);
            if (convertChild != null) {
                OptLocalVariable lVar1
                    = (OptLocalVariable)(child.getProp(Node.VARIABLE_PROP));
                cfw.addALoad(lVar1.getJRegister());
                cfw.add(ByteCode.GETSTATIC,
                        "java/lang/Void",
                        "TYPE",
                        "Ljava/lang/Class;");
                int notNumbersLabel = cfw.acquireLabel();
                cfw.add(ByteCode.IF_ACMPNE, notNumbersLabel);
                cfw.addDLoad(lVar1.getJRegister() + 1);
                cfw.addPush(convertChild.getDouble());
                cfw.add(ByteCode.DCMPL);
                if (type == Token.EQ)
                    cfw.add(ByteCode.IFEQ, trueGOTO);
                else
                    cfw.add(ByteCode.IFNE, trueGOTO);
                cfw.add(ByteCode.GOTO, falseGOTO);
                cfw.markLabel(notNumbersLabel);
                // fall thru into generic handling
            }
        }

        generateCodeFromNode(child, node);
        generateCodeFromNode(rChild, node);

        String name;
        switch (type) {
          case Token.EQ:
            name = "eq";
            addScriptRuntimeInvoke(name,
                                   "(Ljava/lang/Object;"
                                   +"Ljava/lang/Object;"
                                   +")Z");
            break;

          case Token.NE:
            name = "neq";
            addOptRuntimeInvoke(name,
                                "(Ljava/lang/Object;"
                                +"Ljava/lang/Object;"
                                +")Z");
            break;

          case Token.SHEQ:
            name = "shallowEq";
            addScriptRuntimeInvoke(name,
                                   "(Ljava/lang/Object;"
                                   +"Ljava/lang/Object;"
                                   +")Z");
            break;

          case Token.SHNE:
            name = "shallowNeq";
            addOptRuntimeInvoke(name,
                                "(Ljava/lang/Object;"
                                +"Ljava/lang/Object;"
                                +")Z");
            break;

          default:
            name = null;
            Codegen.badTree();
        }
        cfw.add(ByteCode.IFNE, trueGOTO);
        cfw.add(ByteCode.GOTO, falseGOTO);
    }

    private void visitLiteral(Node node)
    {
        if (node.getType() == Token.STRING) {
            // just load the string constant
            cfw.addPush(node.getString());
        } else {
            double num = node.getDouble();
            if (node.getIntProp(Node.ISNUMBER_PROP, -1) != -1) {
                cfw.addPush(num);
            } else {
                codegen.pushNumberAsObject(cfw, num);
            }
        }
    }

    private void visitRegexp(Node node)
    {
        int i = node.getExistingIntProp(Node.REGEXP_PROP);
        cfw.addALoad(funObjLocal);
        cfw.add(ByteCode.GETFIELD, zeroArgClassName,
                Codegen.REGEXP_ARRAY_FIELD_NAME,
                Codegen.REGEXP_ARRAY_FIELD_TYPE);
        cfw.addPush(i);
        cfw.add(ByteCode.AALOAD);
    }

    private void visitName(Node node)
    {
        cfw.addALoad(variableObjectLocal);             // get variable object
        cfw.addPush(node.getString());                 // push name
        addScriptRuntimeInvoke(
            "name",
            "(Lorg/mozilla/javascript/Scriptable;"
            +"Ljava/lang/String;"
            +")Ljava/lang/Object;");
    }

    private void visitSetName(Node node, Node child)
    {
        String name = node.getFirstChild().getString();
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        cfw.addALoad(variableObjectLocal);
        cfw.addPush(name);
        addScriptRuntimeInvoke(
            "setName",
            "(Lorg/mozilla/javascript/Scriptable;"
            +"Ljava/lang/Object;"
            +"Lorg/mozilla/javascript/Scriptable;"
            +"Ljava/lang/String;"
            +")Ljava/lang/Object;");
    }

    private void visitGetVar(OptLocalVariable lVar, boolean isNumber,
                             String name)
    {
        // TODO: Clean up use of lVar here and in set.
        if (hasVarsInRegs && lVar == null)
            lVar = fnCurrent.getVar(name);
        if (lVar != null) {
            if (lVar.getJRegister() == -1)
                if (lVar.isNumber())
                    lVar.assignJRegister(getNewWordPairLocal());
                else
                    lVar.assignJRegister(getNewWordLocal());
            if (lVar.isParameter() && inDirectCallFunction &&
                !itsForcedObjectParameters)
            {
/*
    Remember that here the isNumber flag means that we want to
    use the incoming parameter in a Number context, so test the
    object type and convert the value as necessary.

*/
                if (isNumber) {
                    cfw.addALoad(lVar.getJRegister());
                    cfw.add(ByteCode.GETSTATIC,
                            "java/lang/Void",
                            "TYPE",
                            "Ljava/lang/Class;");
                    int isNumberLabel = cfw.acquireLabel();
                    int beyond = cfw.acquireLabel();
                    cfw.add(ByteCode.IF_ACMPEQ, isNumberLabel);
                    cfw.addALoad(lVar.getJRegister());
                    addScriptRuntimeInvoke("toNumber", "(Ljava/lang/Object;)D");
                    cfw.add(ByteCode.GOTO, beyond);
                    cfw.markLabel(isNumberLabel);
                    cfw.addDLoad(lVar.getJRegister() + 1);
                    cfw.markLabel(beyond);
                } else {
                    cfw.addALoad(lVar.getJRegister());
                    cfw.add(ByteCode.GETSTATIC,
                            "java/lang/Void",
                            "TYPE",
                            "Ljava/lang/Class;");
                    int isNumberLabel = cfw.acquireLabel();
                    int beyond = cfw.acquireLabel();
                    cfw.add(ByteCode.IF_ACMPEQ, isNumberLabel);
                    cfw.addALoad(lVar.getJRegister());
                    cfw.add(ByteCode.GOTO, beyond);
                    cfw.markLabel(isNumberLabel);
                    cfw.add(ByteCode.NEW,"java/lang/Double");
                    cfw.add(ByteCode.DUP);
                    cfw.addDLoad(lVar.getJRegister() + 1);
                    addDoubleConstructor();
                    cfw.markLabel(beyond);
                }
            } else {
                if (lVar.isNumber())
                    cfw.addDLoad(lVar.getJRegister());
                else
                    cfw.addALoad(lVar.getJRegister());
            }
            return;
        }

        cfw.addALoad(variableObjectLocal);
        cfw.addPush(name);
        cfw.addALoad(variableObjectLocal);
        addScriptRuntimeInvoke(
            "getProp",
            "(Ljava/lang/Object;"
            +"Ljava/lang/String;"
            +"Lorg/mozilla/javascript/Scriptable;"
            +")Ljava/lang/Object;");
    }

    private void visitSetVar(Node node, Node child, boolean needValue)
    {
        OptLocalVariable lVar;
        lVar = (OptLocalVariable)(node.getProp(Node.VARIABLE_PROP));
        // XXX is this right? If so, clean up.
        if (hasVarsInRegs && lVar == null)
            lVar = fnCurrent.getVar(child.getString());
        if (lVar != null) {
            generateCodeFromNode(child.getNext(), node);
            if (lVar.getJRegister() == -1) {
                if (lVar.isNumber())
                    lVar.assignJRegister(getNewWordPairLocal());
                else
                    lVar.assignJRegister(getNewWordLocal());
            }
            if (lVar.isParameter()
                        && inDirectCallFunction
                        && !itsForcedObjectParameters) {
                if (node.getIntProp(Node.ISNUMBER_PROP, -1) != -1) {
                    if (needValue) cfw.add(ByteCode.DUP2);
                    cfw.addALoad(lVar.getJRegister());
                    cfw.add(ByteCode.GETSTATIC,
                            "java/lang/Void",
                            "TYPE",
                            "Ljava/lang/Class;");
                    int isNumberLabel = cfw.acquireLabel();
                    int beyond = cfw.acquireLabel();
                    cfw.add(ByteCode.IF_ACMPEQ, isNumberLabel);
                    cfw.add(ByteCode.NEW,"java/lang/Double");
                    cfw.add(ByteCode.DUP);
                    cfw.add(ByteCode.DUP2_X2);
                    cfw.add(ByteCode.POP2);
                    addDoubleConstructor();
                    cfw.addAStore(lVar.getJRegister());
                    cfw.add(ByteCode.GOTO, beyond);
                    cfw.markLabel(isNumberLabel);
                    cfw.addDStore(lVar.getJRegister() + 1);
                    cfw.markLabel(beyond);
                }
                else {
                    if (needValue) cfw.add(ByteCode.DUP);
                    cfw.addAStore(lVar.getJRegister());
                }
            }
            else {
                if (node.getIntProp(Node.ISNUMBER_PROP, -1) != -1) {
                      cfw.addDStore(lVar.getJRegister());
                      if (needValue) cfw.addDLoad(lVar.getJRegister());
                }
                else {
                    cfw.addAStore(lVar.getJRegister());
                    if (needValue) cfw.addALoad(lVar.getJRegister());
                }
            }
            return;
        }

        // default: just treat like any other name lookup
        child.setType(Token.BINDNAME);
        node.setType(Token.SETNAME);
        visitSetName(node, child);
        if (!needValue)
            cfw.add(ByteCode.POP);
    }

    private void visitGetProp(Node node, Node child)
    {
        String s = (String) node.getProp(Node.SPECIAL_PROP_PROP);
        if (s != null) {
            while (child != null) {
                generateCodeFromNode(child, node);
                child = child.getNext();
            }
            cfw.addALoad(variableObjectLocal);
            String runtimeMethod = null;
            if (s.equals("__proto__")) {
                runtimeMethod = "getProto";
            } else if (s.equals("__parent__")) {
                runtimeMethod = "getParent";
            } else {
                Codegen.badTree();
            }
            addScriptRuntimeInvoke(
                runtimeMethod,
                "(Ljava/lang/Object;"
                +"Lorg/mozilla/javascript/Scriptable;"
                +")Lorg/mozilla/javascript/Scriptable;");
            return;
        }
        Node nameChild = child.getNext();
        /*
            for 'this.foo' we call thisGet which can skip some
            casting overhead.

        */
        generateCodeFromNode(child, node);      // the object
        generateCodeFromNode(nameChild, node);  // the name
        if (nameChild.getType() == Token.STRING) {
            if ((child.getType() == Token.THIS)
                || (child.getType() == Token.NEWTEMP
                    && child.getFirstChild().getType() == Token.THIS))
            {
                cfw.addALoad(variableObjectLocal);
                addOptRuntimeInvoke(
                    "thisGet",
                    "(Lorg/mozilla/javascript/Scriptable;"
                    +"Ljava/lang/String;"
                    +"Lorg/mozilla/javascript/Scriptable;"
                    +")Ljava/lang/Object;");
            }
            else {
                cfw.addALoad(variableObjectLocal);
                addScriptRuntimeInvoke(
                    "getProp",
                    "(Ljava/lang/Object;"
                    +"Ljava/lang/String;"
                    +"Lorg/mozilla/javascript/Scriptable;"
                    +")Ljava/lang/Object;");
            }
        }
        else {
            cfw.addALoad(variableObjectLocal);
            addScriptRuntimeInvoke(
                "getProp",
                "(Ljava/lang/Object;"
                +"Ljava/lang/String;"
                +"Lorg/mozilla/javascript/Scriptable;"
                +")Ljava/lang/Object;");
        }
    }

    private void visitSetProp(Node node, Node child)
    {
        String s = (String) node.getProp(Node.SPECIAL_PROP_PROP);
        if (s != null) {
            while (child != null) {
                generateCodeFromNode(child, node);
                child = child.getNext();
            }
            cfw.addALoad(variableObjectLocal);
            String runtimeMethod = null;
            if (s.equals("__proto__")) {
                runtimeMethod = "setProto";
            } else if (s.equals("__parent__")) {
                runtimeMethod = "setParent";
            } else {
                Codegen.badTree();
            }
            addScriptRuntimeInvoke(
                runtimeMethod,
                "(Ljava/lang/Object;"
                +"Ljava/lang/Object;"
                +"Lorg/mozilla/javascript/Scriptable;"
                +")Ljava/lang/Object;");
            return;
        }
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        cfw.addALoad(variableObjectLocal);
        addScriptRuntimeInvoke(
            "setProp",
            "(Ljava/lang/Object;"
            +"Ljava/lang/String;"
            +"Ljava/lang/Object;"
            +"Lorg/mozilla/javascript/Scriptable;"
            +")Ljava/lang/Object;");
    }

    private void visitBind(Node node, int type, Node child)
    {
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        // Generate code for "ScriptRuntime.bind(varObj, "s")"
        cfw.addALoad(variableObjectLocal);             // get variable object
        cfw.addPush(node.getString());                 // push name
        addScriptRuntimeInvoke(
            type == Token.BINDNAME ? "bind" : "getBase",
            "(Lorg/mozilla/javascript/Scriptable;"
            +"Ljava/lang/String;"
            +")Lorg/mozilla/javascript/Scriptable;");
    }

    private short getLocalFromNode(Node node)
    {
        int local = node.getIntProp(Node.LOCAL_PROP, -1);
        if (local == -1) {
            // for NEWLOCAL & USELOCAL, use the next pre-allocated
            // register, otherwise for NEWTEMP & USETEMP, get the
            // next available from the pool
            local = ((node.getType() == Token.NEWLOCAL)
                                || (node.getType() == Token.USELOCAL)) ?
                            itsLocalAllocationBase++ : getNewWordLocal();

            node.putIntProp(Node.LOCAL_PROP, local);
        }
        return (short)local;
    }

    private void visitNewTemp(Node node, Node child)
    {
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        short local = getLocalFromNode(node);
        cfw.add(ByteCode.DUP);
        cfw.addAStore(local);
        if (node.getIntProp(Node.USES_PROP, 0) == 0)
            releaseWordLocal(local);
    }

    private void visitUseTemp(Node node, Node child)
    {
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        Node temp = (Node) node.getProp(Node.TEMP_PROP);
        short local = getLocalFromNode(temp);

        // if the temp node has a magic TARGET property,
        // treat it as a RET to that temp.
        if (node.getProp(Node.TARGET_PROP) != null)
            cfw.add(ByteCode.RET, local);
        else
            cfw.addALoad(local);
        int n = temp.getIntProp(Node.USES_PROP, 0);
        if (n <= 1) {
                    releaseWordLocal(local);
            }
        if (n != 0 && n != Integer.MAX_VALUE) {
            temp.putIntProp(Node.USES_PROP, n - 1);
        }
    }

    private void visitNewLocal(Node node, Node child)
    {
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        short local = getLocalFromNode(node);
        cfw.add(ByteCode.DUP);
        cfw.addAStore(local);
    }

    private void visitUseLocal(Node node, Node child)
    {
        while (child != null) {
            generateCodeFromNode(child, node);
            child = child.getNext();
        }
        Node temp = (Node) node.getProp(Node.LOCAL_PROP);
        short local = getLocalFromNode(temp);

        // if the temp node has a magic TARGET property,
        // treat it as a RET to that temp.
        if (node.getProp(Node.TARGET_PROP) != null)
            cfw.add(ByteCode.RET, local);
        else
            cfw.addALoad(local);
    }

    private void addScriptRuntimeInvoke(String methodName,
                                        String methodSignature)
    {
        cfw.addInvoke(ByteCode.INVOKESTATIC,
                      "org/mozilla/javascript/ScriptRuntime",
                      methodName,
                      methodSignature);
    }

    private void addOptRuntimeInvoke(String methodName,
                                     String methodSignature)
    {
        cfw.addInvoke(ByteCode.INVOKESTATIC,
                      "org/mozilla/javascript/optimizer/OptRuntime",
                      methodName,
                      methodSignature);
    }

    private void addDoubleConstructor()
    {
        cfw.addInvoke(ByteCode.INVOKESPECIAL,
                      "java/lang/Double", "<init>", "(D)V");
    }

    private short getNewWordPairLocal()
    {
        short result = firstFreeLocal;
        while (true) {
            if (result >= (MAX_LOCALS - 1))
                break;
            if (!locals[result]
                    && !locals[result + 1])
                break;
            result++;
        }
        if (result < (MAX_LOCALS - 1)) {
            locals[result] = true;
            locals[result + 1] = true;
            if (result == firstFreeLocal) {
                for (int i = firstFreeLocal + 2; i < MAX_LOCALS; i++) {
                    if (!locals[i]) {
                        firstFreeLocal = (short) i;
                        if (localsMax < firstFreeLocal)
                            localsMax = firstFreeLocal;
                        return result;
                    }
                }
            }
            else {
                return result;
            }
        }
        throw Context.reportRuntimeError("Program too complex " +
                                         "(out of locals)");
    }

    private short reserveWordLocal(int local)
    {
        if (getNewWordLocal() != local)
            throw new RuntimeException("Local allocation error");
        return (short) local;
    }

    private short getNewWordLocal()
    {
        short result = firstFreeLocal;
        locals[result] = true;
        for (int i = firstFreeLocal + 1; i < MAX_LOCALS; i++) {
            if (!locals[i]) {
                firstFreeLocal = (short) i;
                if (localsMax < firstFreeLocal)
                    localsMax = firstFreeLocal;
                return result;
            }
        }
        throw Context.reportRuntimeError("Program too complex " +
                                         "(out of locals)");
    }

    private void releaseWordpairLocal(short local)
    {
        if (local < firstFreeLocal)
            firstFreeLocal = local;
        locals[local] = false;
        locals[local + 1] = false;
    }

    private void releaseWordLocal(short local)
    {
        if (local < firstFreeLocal)
            firstFreeLocal = local;
        locals[local] = false;
    }

    ClassFileWriter cfw;
    Codegen codegen;
    String zeroArgClassName;
    ScriptOrFnNode scriptOrFn;

    private OptFunctionNode fnCurrent;
    private boolean isTopLevel;

    private static final int MAX_LOCALS = 256;
    private boolean[] locals;
    private short firstFreeLocal;
    private short localsMax;

    private OptLocalVariable[] debugVars;
    private int itsLineNumber;

    private boolean hasVarsInRegs;
    private boolean inDirectCallFunction;
    private boolean itsForcedObjectParameters;
    private short itsLocalAllocationBase;
    private int epilogueLabel;

    // special known locals. If you add a new local here, be sure
    // to initialize it to -1 in initBodyGeneration
    private short variableObjectLocal;
    private short scriptResultLocal;
    private short contextLocal;
    private short argsLocal;
    private short thisObjLocal;
    private short funObjLocal;
    private short itsZeroArgArray;
    private short itsOneArgArray;
}
