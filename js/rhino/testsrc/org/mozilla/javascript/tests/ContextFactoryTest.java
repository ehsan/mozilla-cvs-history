/**
 * 
 */
package org.mozilla.javascript.tests;

import junit.framework.TestCase;

import org.mozilla.javascript.Context;
import org.mozilla.javascript.ContextFactory;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.RhinoException;

/**
 * @author Norris Boyd
 */
public class ContextFactoryTest extends TestCase {
    static class MyFactory extends ContextFactory {
        @Override
        public boolean hasFeature(Context cx, int featureIndex)
        {
            switch (featureIndex) {
                case Context.FEATURE_MEMBER_EXPR_AS_FUNCTION_NAME:
                    return true;
            }
            return super.hasFeature(cx, featureIndex);
        }
    }

    public void testCustomContextFactory() {
        ContextFactory.initGlobal(new MyFactory());
        Context cx = Context.enter();
        try {
            Scriptable globalScope = cx.initStandardObjects();
            // Test that FEATURE_MEMBER_EXPR_AS_FUNCTION_NAME is enabled
            Object result = cx.evaluateString(globalScope,
                    "var obj = {};" +
                    "function obj.foo() { return 'bar'; }" +
                    "obj.foo();",
                    "test source", 1, null);
            assertEquals("bar", result);
        } catch (RhinoException e) {
            fail(e.toString());
        } finally {
            Context.exit();
        }
    }
 }
