/* -*- Mode: java; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
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
 * Frank Mitchell
 * Mike Shaver
 * Kurt Westerfeld
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

package org.mozilla.javascript;

import java.lang.reflect.*;
import java.util.Hashtable;
import java.util.Enumeration;

/**
 *
 * @author Mike Shaver
 * @author Norris Boyd
 * @see NativeJavaObject
 * @see NativeJavaClass
 */
class JavaMembers
{

    JavaMembers(Scriptable scope, Class cl)
    {
        this.members = new Hashtable(23);
        this.staticMembers = new Hashtable(7);
        this.cl = cl;
        reflect(scope);
    }

    boolean has(String name, boolean isStatic)
    {
        Hashtable ht = isStatic ? staticMembers : members;
        Object obj = ht.get(name);
        if (obj != null) {
            return true;
        } else {
            return null != findExplicitFunction(name, isStatic);
        }
    }

    Object get(Scriptable scope, String name, Object javaObject,
               boolean isStatic)
    {
        Hashtable ht = isStatic ? staticMembers : members;
        Object member = ht.get(name);
        if (!isStatic && member == null) {
            // Try to get static member from instance (LC3)
            member = staticMembers.get(name);
        }
        if (member == null) {
            member = this.getExplicitFunction(scope, name,
                                              javaObject, isStatic);
            if (member == null)
                return Scriptable.NOT_FOUND;
        }
        if (member instanceof Scriptable)
            return member;      // why is this here?
        Context cx = Context.getContext();
        Object rval;
        Class type;
        try {
            if (member instanceof BeanProperty) {
                BeanProperty bp = (BeanProperty) member;
                rval = bp.getter.invoke(javaObject, null);
                type = bp.getter.method().getReturnType();
            } else {
                Field field = (Field) member;
                rval = field.get(isStatic ? null : javaObject);
                type = field.getType();
            }
        } catch (Exception ex) {
            throw ScriptRuntime.throwAsUncheckedException(ex);
        }
        // Need to wrap the object before we return it.
        scope = ScriptableObject.getTopLevelScope(scope);
        return cx.getWrapFactory().wrap(cx, scope, rval, type);
    }

    public void put(Scriptable scope, String name, Object javaObject,
                    Object value, boolean isStatic)
    {
        Hashtable ht = isStatic ? staticMembers : members;
        Object member = ht.get(name);
        if (!isStatic && member == null) {
            // Try to get static member from instance (LC3)
            member = staticMembers.get(name);
        }
        if (member == null)
            throw reportMemberNotFound(name);
        if (member instanceof FieldAndMethods) {
            FieldAndMethods fam = (FieldAndMethods) ht.get(name);
            member = fam.field;
        }

        // Is this a bean property "set"?
        if (member instanceof BeanProperty) {
            BeanProperty bp = (BeanProperty)member;
            if (bp.setter == null) {
                throw reportMemberNotFound(name);
            }
            Class setType = bp.setter.argTypes[0];
            Object[] args = { NativeJavaObject.coerceType(setType, value,
                                                          true) };
            try {
                bp.setter.invoke(javaObject, args);
            } catch (Exception ex) {
                throw ScriptRuntime.throwAsUncheckedException(ex);
            }
        }
        else {
            if (!(member instanceof Field)) {
                String str = (member == null) ? "msg.java.internal.private"
                                              : "msg.java.method.assign";
                throw Context.reportRuntimeError1(str, name);
            }
            Field field = (Field)member;
            Object javaValue = NativeJavaObject.coerceType(field.getType(),
                                                           value, true);
            try {
                field.set(javaObject, javaValue);
            } catch (IllegalAccessException accessEx) {
                throw new RuntimeException("unexpected IllegalAccessException "+
                                           "accessing Java field");
            } catch (IllegalArgumentException argEx) {
                throw Context.reportRuntimeError3(
                    "msg.java.internal.field.type",
                    value.getClass().getName(), field,
                    javaObject.getClass().getName());
            }
        }
    }

    Object[] getIds(boolean isStatic)
    {
        Hashtable ht = isStatic ? staticMembers : members;
        int len = ht.size();
        Object[] result = new Object[len];
        Enumeration keys = ht.keys();
        for (int i=0; i < len; i++)
            result[i] = keys.nextElement();
        return result;
    }

    static String javaSignature(Class type)
    {
        if (!type.isArray()) {
            return type.getName();
        } else {
            int arrayDimension = 0;
            do {
                ++arrayDimension;
                type = type.getComponentType();
            } while (type.isArray());
            String name = type.getName();
            String suffix = "[]";
            if (arrayDimension == 1) {
                return name.concat(suffix);
            } else {
                int length = name.length() + arrayDimension * suffix.length();
                StringBuffer sb = new StringBuffer(length);
                sb.append(name);
                while (arrayDimension != 0) {
                    --arrayDimension;
                    sb.append(suffix);
                }
                return sb.toString();
            }
        }
    }

    static String liveConnectSignature(Class[] argTypes)
    {
        int N = argTypes.length;
        if (N == 0) { return "()"; }
        StringBuffer sb = new StringBuffer();
        sb.append('(');
        for (int i = 0; i != N; ++i) {
            if (i != 0) {
                sb.append(',');
            }
            sb.append(javaSignature(argTypes[i]));
        }
        sb.append(')');
        return sb.toString();
    }

    private MemberBox findExplicitFunction(String name, boolean isStatic)
    {
        int sigStart = name.indexOf('(');
        if (sigStart < 0) { return null; }

        Hashtable ht = isStatic ? staticMembers : members;
        MemberBox[] methodsOrCtors = null;
        boolean isCtor = (isStatic && sigStart == 0);

        if (isCtor) {
            // Explicit request for an overloaded constructor
            methodsOrCtors = ctors;
        } else {
            // Explicit request for an overloaded method
            String trueName = name.substring(0,sigStart);
            Object obj = ht.get(trueName);
            if (!isStatic && obj == null) {
                // Try to get static member from instance (LC3)
                obj = staticMembers.get(trueName);
            }
            if (obj instanceof NativeJavaMethod) {
                NativeJavaMethod njm = (NativeJavaMethod)obj;
                methodsOrCtors = njm.methods;
            }
        }

        if (methodsOrCtors != null) {
            for (int i = 0; i < methodsOrCtors.length; i++) {
                Class[] type = methodsOrCtors[i].argTypes;
                String sig = liveConnectSignature(type);
                if (sigStart + sig.length() == name.length()
                    && name.regionMatches(sigStart, sig, 0, sig.length()))
                {
                    return methodsOrCtors[i];
                }
            }
        }

        return null;
    }

    private Object getExplicitFunction(Scriptable scope, String name,
                                       Object javaObject, boolean isStatic)
    {
        Hashtable ht = isStatic ? staticMembers : members;
        Object member = null;
        MemberBox methodOrCtor = findExplicitFunction(name, isStatic);

        if (methodOrCtor != null) {
            Scriptable prototype =
                ScriptableObject.getFunctionPrototype(scope);

            if (methodOrCtor.isCtor()) {
                NativeJavaConstructor fun =
                    new NativeJavaConstructor(methodOrCtor);
                fun.setPrototype(prototype);
                member = fun;
                ht.put(name, fun);
            } else {
                String trueName = methodOrCtor.getName();
                member = ht.get(trueName);

                if (member instanceof NativeJavaMethod &&
                    ((NativeJavaMethod)member).methods.length > 1 ) {
                    NativeJavaMethod fun =
                        new NativeJavaMethod(methodOrCtor, name);
                    fun.setPrototype(prototype);
                    ht.put(name, fun);
                    member = fun;
                }
            }
        }

        return member;
    }

    private void reflect(Scriptable scope)
    {
        // We reflect methods first, because we want overloaded field/method
        // names to be allocated to the NativeJavaMethod before the field
        // gets in the way.
        reflectMethods(scope);
        reflectFields(scope);
        makeBeanProperties(scope, false);
        makeBeanProperties(scope, true);

        reflectCtors();
    }

    private void reflectMethods(Scriptable scope)
    {
        Method[] methods = cl.getMethods();
        for (int i = 0; i < methods.length; i++) {
            Method method = methods[i];
            int mods = method.getModifiers();
            if (!Modifier.isPublic(mods)) {
                continue;
            }
            boolean isStatic = Modifier.isStatic(mods);
            Hashtable ht = isStatic ? staticMembers : members;
            String name = method.getName();
            Object value = ht.get(name);
            if (value == null) {
                ht.put(name, method);
            } else {
                ObjArray overloadedMethods;
                if (value instanceof ObjArray) {
                    overloadedMethods = (ObjArray)value;
                } else {
                    if (!(value instanceof Method)) Context.codeBug();
                    // value should be instance of Method as reflectMethods is
                    // called when staticMembers and members are empty
                    overloadedMethods = new ObjArray();
                    overloadedMethods.add(value);
                    ht.put(name, overloadedMethods);
                }
                overloadedMethods.add(method);
            }
        }
        initNativeMethods(staticMembers, scope);
        initNativeMethods(members, scope);
    }

    private void reflectFields(Scriptable scope)
    {
        Field[] fields = cl.getFields();
        for (int i = 0; i < fields.length; i++) {
            Field field = fields[i];
            int mods = field.getModifiers();
            if (!Modifier.isPublic(mods)) {
                continue;
            }
            boolean isStatic = Modifier.isStatic(mods);
            Hashtable ht = isStatic ? staticMembers : members;
            String name = field.getName();
            Object member = ht.get(name);
            if (member == null) {
                ht.put(name, field);
            } else if (member instanceof NativeJavaMethod) {
                NativeJavaMethod method = (NativeJavaMethod) member;
                FieldAndMethods fam = new FieldAndMethods(method.methods,
                                                          field);
                fam.setPrototype(ScriptableObject.getFunctionPrototype(scope));
                getFieldAndMethodsTable(isStatic).put(name, fam);
                ht.put(name, fam);
            } else if (member instanceof Field) {
                Field oldField = (Field) member;
                // If this newly reflected field shadows an inherited field,
                // then replace it. Otherwise, since access to the field
                // would be ambiguous from Java, no field should be reflected.
                // For now, the first field found wins, unless another field
                // explicitly shadows it.
                if (oldField.getDeclaringClass().
                        isAssignableFrom(field.getDeclaringClass()))
                {
                    ht.put(name, field);
                }
            } else {
                // "unknown member type"
                Context.codeBug();
            }
        }
    }


    private void makeBeanProperties(Scriptable scope, boolean isStatic)
    {
        Hashtable ht = isStatic ? staticMembers : members;
        Hashtable toAdd = new Hashtable();

        // Now, For each member, make "bean" properties.
        for (Enumeration e = ht.keys(); e.hasMoreElements(); ) {

            // Is this a getter?
            String name = (String) e.nextElement();
            boolean memberIsGetMethod = name.startsWith("get");
            boolean memberIsIsMethod = name.startsWith("is");
            if (memberIsGetMethod || memberIsIsMethod) {
                // Double check name component.
                String nameComponent = name.substring(memberIsGetMethod ? 3 : 2);
                if (nameComponent.length() == 0)
                    continue;

                // Make the bean property name.
                String beanPropertyName = nameComponent;
                char ch0 = nameComponent.charAt(0);
                if (Character.isUpperCase(ch0)) {
                    if (nameComponent.length() == 1) {
                        beanPropertyName = nameComponent.toLowerCase();
                    } else {
                        char ch1 = nameComponent.charAt(1);
                        if (!Character.isUpperCase(ch1)) {
                            beanPropertyName = Character.toLowerCase(ch0)
                                               +nameComponent.substring(1);
                        }
                    }
                }

                // If we already have a member by this name, don't do this
                // property.
                if (ht.containsKey(beanPropertyName))
                    continue;

                // Get the method by this name.
                Object member = ht.get(name);
                if (!(member instanceof NativeJavaMethod))
                    continue;

                NativeJavaMethod njmGet = (NativeJavaMethod)member;
                MemberBox getter = extractGetMethod(njmGet.methods, isStatic);
                if (getter != null) {

                    // We have a getter.  Now, do we have a setter?
                    NativeJavaMethod njmSet = null;
                    MemberBox setter = null;
                    String setterName = "set".concat(nameComponent);
                    if (ht.containsKey(setterName)) {
                        // Is this value a method?
                        member = ht.get(setterName);
                        if (member instanceof NativeJavaMethod) {
                            njmSet = (NativeJavaMethod)member;
                            Class type = getter.method().getReturnType();
                            setter = extractSetMethod(type, njmSet.methods,
                                                      isStatic);
                        }
                    }

                    // Make the property.
                    BeanProperty bp = new BeanProperty(getter, setter);
                    toAdd.put(beanPropertyName, bp);
                }
            }
        }

        // Add the new bean properties.
        for (Enumeration e = toAdd.keys(); e.hasMoreElements();) {
            String key = (String) e.nextElement();
            Object value = toAdd.get(key);
            ht.put(key, value);
        }
    }

    private void reflectCtors()
    {
        Constructor[] constructors = cl.getConstructors();
        int N = constructors.length;
        ctors = new MemberBox[N];
        for (int i = 0; i != N; ++i) {
            ctors[i] = new MemberBox(constructors[i]);
        }
    }

    private static void initNativeMethods(Hashtable ht, Scriptable scope)
    {
        Enumeration e = ht.keys();
        while (e.hasMoreElements()) {
            String name = (String)e.nextElement();
            MemberBox[] methods;
            Object value = ht.get(name);
            if (value instanceof Method) {
                methods = new MemberBox[1];
                methods[0] = new MemberBox((Method)value);
            } else {
                ObjArray overloadedMethods = (ObjArray)value;
                int N = overloadedMethods.size();
                if (N < 2) Context.codeBug();
                methods = new MemberBox[N];
                for (int i = 0; i != N; ++i) {
                    Method method = (Method)overloadedMethods.get(i);
                    methods[i] = new MemberBox(method);
                }
            }
            NativeJavaMethod fun = new NativeJavaMethod(methods);
            if (scope != null) {
                fun.setPrototype(ScriptableObject.getFunctionPrototype(scope));
            }
            ht.put(name, fun);
        }
    }

    private Hashtable getFieldAndMethodsTable(boolean isStatic)
    {
        Hashtable fmht = isStatic ? staticFieldAndMethods
                                  : fieldAndMethods;
        if (fmht == null) {
            fmht = new Hashtable(11);
            if (isStatic)
                staticFieldAndMethods = fmht;
            else
                fieldAndMethods = fmht;
        }

        return fmht;
    }

    private static MemberBox extractGetMethod(MemberBox[] methods,
                                              boolean isStatic)
    {
         // Grab and inspect the getter method; does it have an empty parameter
         // list with a return value (eg. a getSomething() or isSomething())?
        if (methods.length == 1) {
            MemberBox method = methods[0];
            // Make sure the method static-ness is preserved for this property.
            if (!isStatic || method.isStatic()) {
                if (method.argTypes.length == 0) {
                    Class type = method.method().getReturnType();
                    if (type != Void.TYPE) {
                        return method;
                    }
                }
            }
        }
        return null;
    }

    private static MemberBox extractSetMethod(Class type, MemberBox[] methods,
                                              boolean isStatic)
    {
        //
        // Note: it may be preferable to allow NativeJavaMethod.findFunction()
        //       to find the appropriate setter; unfortunately, it requires an
        //       instance of the target arg to determine that.
        //

        // Make two passes: one to find a method with direct type assignment,
        // and one to find a widening conversion.
        for (int pass = 1; pass <= 2; ++pass) {
            for (int i = 0; i < methods.length; ++i) {
                MemberBox method = methods[i];
                if (!isStatic || method.isStatic()) {
                    if (method.method().getReturnType() == Void.TYPE) {
                        Class[] params = method.argTypes;
                        if (params.length == 1) {
                            if (pass == 1) {
                                if (params[0] == type) {
                                    return method;
                                }
                            } else {
                                if (pass != 2) Context.codeBug();
                                if (params[0].isAssignableFrom(type)) {
                                    return method;
                                }
                            }
                        }
                    }
                }
            }
        }
        return null;
    }

    Hashtable getFieldAndMethodsObjects(Scriptable scope, Object javaObject,
                                        boolean isStatic)
    {
        Hashtable ht = isStatic ? staticFieldAndMethods : fieldAndMethods;
        if (ht == null)
            return null;
        int len = ht.size();
        Hashtable result = new Hashtable(len);
        Enumeration e = ht.elements();
        while (len-- > 0) {
            FieldAndMethods fam = (FieldAndMethods) e.nextElement();
            FieldAndMethods famNew = new FieldAndMethods(fam.methods,
                                                         fam.field);
            famNew.javaObject = javaObject;
            result.put(fam.field.getName(), famNew);
        }
        return result;
    }

    static JavaMembers lookupClass(Scriptable scope, Class dynamicType,
                                   Class staticType)
    {
        Hashtable ct = classTable;  // use local reference to avoid synchronize
        JavaMembers members = (JavaMembers) ct.get(dynamicType);
        if (members != null)
            return members;

        if (staticType == dynamicType) {
            staticType = null;
        }

        Class cl = dynamicType;
        if (!Modifier.isPublic(dynamicType.getModifiers())) {
            if (staticType == null
                || !Modifier.isPublic(staticType.getModifiers()))
            {
                cl = getPublicSuperclass(dynamicType);
                if (cl == null) {
                    // Can happen if dynamicType is interface
                    cl = dynamicType;
                }
            } else if (staticType.isInterface()) {
                // If the static type is an interface, use it
                cl = staticType;
            } else {
                // We can use the static type, and that is OK, but we'll trace
                // back the java class chain here to look for public superclass
                // comming before staticType
                cl = getPublicSuperclass(dynamicType);
                if (cl == null) {
                    // Can happen if dynamicType is interface
                    cl = dynamicType;
                }
            }
        }
        for (;;) {
            try {
                members = new JavaMembers(scope, cl);
                break;
            } catch (SecurityException e) {
                // Reflection may fail for objects that are in a restricted
                // access package (e.g. sun.*).  If we get a security
                // exception, try again with the static type if it is interface.
                // Otherwise, try public superclass
                if (staticType != null && staticType.isInterface()) {
                    cl = staticType;
                    staticType = null; // try staticType only once
                    continue;
                }
                Class parent = getPublicSuperclass(cl);
                if (parent == null) {
                    if (cl.isInterface()) {
                        // last resort
                        parent = ScriptRuntime.ObjectClass;
                    } else {
                        throw e;
                    }
                }
                cl = parent;
            }
        }

        if (Context.isCachingEnabled)
            ct.put(cl, members);
        return members;
    }

    private static Class getPublicSuperclass(Class cl)
    {
        if (cl == ScriptRuntime.ObjectClass) {
            return null;
        }
        do {
            cl = cl.getSuperclass();
            if (cl == null || cl == ScriptRuntime.ObjectClass) {
                break;
            }
        } while (!Modifier.isPublic(cl.getModifiers()));
        return cl;
    }

    RuntimeException reportMemberNotFound(String memberName)
    {
        return Context.reportRuntimeError2(
            "msg.java.member.not.found", cl.getName(), memberName);
    }

    static Hashtable classTable = new Hashtable();

    private Class cl;
    private Hashtable members;
    private Hashtable fieldAndMethods;
    private Hashtable staticMembers;
    private Hashtable staticFieldAndMethods;
    MemberBox[] ctors;
}

class BeanProperty
{
    BeanProperty(MemberBox getter, MemberBox setter)
    {
        this.getter = getter;
        this.setter = setter;
    }

    MemberBox getter;
    MemberBox setter;
}

class FieldAndMethods extends NativeJavaMethod
{

    FieldAndMethods(MemberBox[] methods, Field field)
    {
        super(methods);
        this.field = field;
    }

    public Object getDefaultValue(Class hint)
    {
        if (hint == ScriptRuntime.FunctionClass)
            return this;
        Object rval;
        Class type;
        try {
            rval = field.get(javaObject);
            type = field.getType();
        } catch (IllegalAccessException accEx) {
            throw Context.reportRuntimeError1(
                "msg.java.internal.private", field.getName());
        }
        Context cx  = Context.getContext();
        rval = cx.getWrapFactory().wrap(cx, this, rval, type);
        if (rval instanceof Scriptable) {
            rval = ((Scriptable) rval).getDefaultValue(hint);
        }
        return rval;
    }

    Field field;
    Object javaObject;
}
