/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Sun Microsystems,
 * Inc. Portions created by Sun are
 * Copyright (C) 1999 Sun Microsystems, Inc. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
#include "JNIEnvTests.h"
#include "ArrayOperations.h"

JNI_OJIAPITest(JNIEnv_GetCharArrayElements_1)
{
  GET_JNI_FOR_TEST

  jcharArray arr = env->NewCharArray(5);

  jboolean isCopy = JNI_TRUE;
  jchar *val = env->GetCharArrayElements(NULL, &isCopy);
  printf("Function GetCharArrayElements(NULL, isCopy) is not crushed!\n\n");
  if(val==NULL){
     return TestResult::PASS("GetCharArrayElements(NULL, NOT_NULL) returns correct value");
  }else{
     return TestResult::FAIL("GetCharArrayElements(NULL, NOT_NULL) returns incorrect value");
  }

}
