/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#ifndef nsISpellChecker_h__
#define nsISpellChecker_h__

#include "nsISupports.h"

#define NS_ISPELLCHECKER_IID                    \
{ /* F72A52F1-F83B-11d2-8D54-000064657374 */    \
0xf72a52f1, 0xf83b, 0x11d2,                     \
{ 0x8d, 0x54, 0x0, 0x0, 0x64, 0x65, 0x73, 0x74 } }

class nsITextServicesDocument;
class nsString;
class nsVoidArray;

/**
 * A generic interface for a spelling checker.
 */
class nsISpellChecker  : public nsISupports{
public:

  static const nsIID& GetIID() { static nsIID iid = NS_ISPELLCHECKER_IID; return iid; }

  /**
   * Tells the spellchecker what document to check.
   * @param aDoc is the document to check.
   * @param aFromStartOfDoc If true, start check from beginning of document,
   * if false, start check from current cursor position.
   */
  NS_IMETHOD SetDocument(nsITextServicesDocument *aDoc, PRBool aFromStartofDoc) = 0;

  /**
   * Selects (hilites) the next misspelled word in the document.
   * @param aWord will contain the misspelled word.
   * @param aSuggestions is an array of nsStrings, that represent the
   * suggested replacements for the misspelled word. The caller is
   * responsible for calling delete for each nsString entry in this array.
   */
  NS_IMETHOD NextMisspelledWord(nsString *aWord, nsVoidArray *aSuggestions) = 0;

  /**
   * Checks if a word is misspelled. No document is required to use this method.
   * @param aWord is the word to check.
   * @param aIsMisspelled will be set to true if the word is misspelled.
   * @param aSuggestions is an array of nsStrings which represent the
   * suggested replacements for the misspelled word. The array will be empty
   * if there aren't any suggestions. The caller is responsible for calling
   * delete for each nsString entry in this array.
   */
  NS_IMETHOD CheckWord(nsString *aWord, PRBool *aIsMisspelled, nsVoidArray *aSuggestions) = 0;

  /**
   * Replaces the old word with the specified new word.
   * @param aOldWord is the word to be replaced.
   * @param aNewWord is the word that is to replace old word.
   * @param aAllOccurrences will replace all occurrences of old
   * word, in the document, with new word when it is true. If
   * false, it will replace the 1st occurrence only!
   */
  NS_IMETHOD Replace(nsString *aOldWord, nsString *aNewWord, PRBool aAllOccurrences) = 0;

  /**
   * Ignores all occurrences of the specified word in the document.
   * @param aWord is the word to ignore.
   */
  NS_IMETHOD IgnoreAll(nsString *aWord) = 0;

  /**
   * Add a word to the user's personal dictionary.
   * @param aWord is the word to add.
   */
  NS_IMETHOD AddWordToPersonalDictionary(nsString *aWord);

  /**
   * Remove a word from the user's personal dictionary.
   * @param aWord is the word to remove.
   */
  NS_IMETHOD RemoveWordFromPersonalDictionary(nsString *aWord);

  /**
   * Returns the list of words in the user's personal dictionary.
   * @param aWordList is an array of nsStrings that represent the
   * list of words in the user's personal dictionary. The caller is
   * responsible for calling delete for each nsString entry in this array.
   */
  NS_IMETHOD GetPersonalDictionary(nsVoidArray *aWordList) = 0;
};

#endif // nsISpellChecker_h__

