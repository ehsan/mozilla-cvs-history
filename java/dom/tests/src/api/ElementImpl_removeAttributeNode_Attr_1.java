/**
 *
 *  @version 1.00 
 *  @author Raju Pallath
 *
 *  TESTID 
 * 
 *  Tests out the ElementImpl->removeattributeNode(dummyattr) method.
 *
 */
package org.mozilla.dom.test;

import java.util.*;
import java.io.*;
import org.mozilla.dom.test.*;
import org.mozilla.dom.*;
import org.w3c.dom.*;

public class ElementImpl_removeAttributeNode_Attr_1 extends BWBaseTest implements Execution
{

   /**
    *
    ***********************************************************
    *  Constructor
    ***********************************************************
    *
    */
   public ElementImpl_removeAttributeNode_Attr_1()
   {
   }


   /**
    *
    ***********************************************************
    *  Starting point of application
    *
    *  @param   args    Array of command line arguments
    *
    ***********************************************************
    */
   public static void main(String[] args)
   {
   }

   /**
    ***********************************************************
    *
    *  Execute Method 
    *
    *  @param   tobj    Object reference (Node/Document/...)
    *  @return          true or false  depending on whether test passed or failed.
    *
    ***********************************************************
    */
   public boolean execute(Object tobj)
   {
      if (tobj == null)  {
           TestLoader.logPrint("Object is NULL...");
           return BWBaseTest.FAILED;
      }

      String os = System.getProperty("OS");
      osRoutine(os);

      Document d = (Document)tobj;
      if (d != null)
      {
	     Attr a = d.createAttribute("dummyattr_1");
	     if (a == null) {
                TestLoader.logErrPrint("Document createAttribute FAILED... ");
                return BWBaseTest.FAILED;
	     }

             Element e = d.getDocumentElement();
	     if (e == null) {
                TestLoader.logErrPrint("Document Element is  NULL..");
                return BWBaseTest.FAILED;
             } else {
                Attr tattr = e.setAttributeNode(a);
                tattr = e.getAttributeNode("dummyattr_1");
		if (tattr == null) {
                  TestLoader.logErrPrint("Element 'getAttributeNode(dummy_attr_1) FAILED... ");
                  return BWBaseTest.FAILED;
                }
 
                Attr rattr = e.removeAttributeNode(tattr);
		if (rattr == null) {
                  TestLoader.logErrPrint("Element 'removeAttributeNode(dummy_attr_1) FAILED... ");
                  return BWBaseTest.FAILED;
                } 
             }
      } else {
             System.out.println("Document is  NULL..");
             return BWBaseTest.FAILED;
      }

      return BWBaseTest.PASSED;
   }

   /**
    *
    ***********************************************************
    *  Routine where OS specific checks are made. 
    *
    *  @param   os      OS Name (SunOS/Linus/MacOS/...)
    ***********************************************************
    *
    */
   private void osRoutine(String os)
   {
     if(os == null) return;

     os = os.trim();
     if(os.compareTo("SunOS") == 0) {}
     else if(os.compareTo("Linux") == 0) {}
     else if(os.compareTo("Windows") == 0) {}
     else if(os.compareTo("MacOS") == 0) {}
     else {}
   }
}
