/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Mozilla Password Manager.
 *
 * The Initial Developer of the Original Code is
 * Brian Ryner.
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsCPasswordManager.h"
#include "nsClassHashtable.h"
#include "nsDataHashtable.h"
#include "nsCOMPtr.h"
#include "nsIAuthPromptWrapper.h"
#include "nsIObserver.h"
#include "nsWeakReference.h"
#include "nsIPrompt.h"
#include "nsIFormSubmitObserver.h"
#include "nsIWebProgressListener.h"
#include "nsIDOMFormListener.h"

/* 360565c4-2ef3-4f6a-bab9-94cca891b2a7 */
#define NS_PASSWORDMANAGER_CID \
{0x360565c4, 0x2ef3, 0x4f6a, {0xba, 0xb9, 0x94, 0xcc, 0xa8, 0x91, 0xb2, 0xa7}}

/* 1baf3398-f759-4a72-a21f-0abdc9cc9960 */
#define NS_SINGLE_SIGNON_PROMPT_CID \
{0x1baf3398, 0xf759, 0x4a72, {0xa2, 0x1f, 0x0a, 0xbd, 0xc9, 0xcc, 0x99, 0x60}}


class nsIFile;
class nsIPrompt;
class nsIStringBundle;
class nsIComponentManager;
class nsIContent;
class nsIDOMWindowInternal;
class nsIURI;
struct nsModuleComponentInfo;

class nsPasswordManager : public nsIPasswordManager,
                          public nsIPasswordManagerInternal,
                          public nsIObserver,
                          public nsIFormSubmitObserver,
                          public nsIWebProgressListener,
                          public nsIDOMFormListener,
                          public nsSupportsWeakReference
{
public:
  class SignonDataEntry;
  class SignonHashEntry;
  class PasswordEntry;

  nsPasswordManager();
  virtual ~nsPasswordManager();

  nsresult Init();
  static PRBool SingleSignonEnabled();

  static NS_METHOD Register(nsIComponentManager* aCompMgr,
                            nsIFile* aPath,
                            const char* aRegistryLocation,
                            const char* aComponentType,
                            const nsModuleComponentInfo* aInfo);

  static NS_METHOD Unregister(nsIComponentManager* aCompMgr,
                              nsIFile* aPath,
                              const char* aRegistryLocation,
                              const nsModuleComponentInfo* aInfo);

  static void Shutdown();


  static nsresult DecryptData(const nsAString& aData, nsAString& aPlaintext);
  static nsresult EncryptData(const nsAString& aPlaintext,
                              nsACString& aEncrypted);
  static nsresult EncryptDataUCS2(const nsAString& aPlaintext,
                                  nsAString& aEncrypted);


  NS_DECL_ISUPPORTS
  NS_DECL_NSIPASSWORDMANAGER
  NS_DECL_NSIPASSWORDMANAGERINTERNAL
  NS_DECL_NSIOBSERVER
  NS_DECL_NSIWEBPROGRESSLISTENER

  // nsIFormSubmitObserver
  NS_IMETHOD Notify(nsIContent* aFormNode,
                    nsIDOMWindowInternal* aWindow,
                    nsIURI* aActionURL,
                    PRBool* aCancelSubmit);

  // nsIDOMFormListener
  NS_IMETHOD Submit(nsIDOMEvent* aEvent);
  NS_IMETHOD Reset(nsIDOMEvent* aEvent);
  NS_IMETHOD Change(nsIDOMEvent* aEvent);
  NS_IMETHOD Select(nsIDOMEvent* aEvent);
  NS_IMETHOD Input(nsIDOMEvent* aEvent);

  // nsIDOMEventListener
  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent);


protected:
  void ReadSignonFile();
  void WriteSignonFile();
  void AddSignonData(const nsACString& aRealm, SignonDataEntry* aEntry);

  nsresult FindPasswordEntryInternal(const SignonDataEntry* aEntry,
                                     const nsAString&  aUser,
                                     const nsAString&  aPassword,
                                     const nsAString&  aUserField,
                                     SignonDataEntry** aResult);


  static PLDHashOperator PR_CALLBACK FindEntryEnumerator(const nsACString& aKey,
                                                         SignonHashEntry* aEntry,
                                                         void* aUserData);

  static PLDHashOperator PR_CALLBACK WriteRejectEntryEnumerator(const nsACString& aKey,
                                                                PRInt32 aEntry,
                                                                void* aUserData);

  static PLDHashOperator PR_CALLBACK WriteSignonEntryEnumerator(const nsACString& aKey,
                                                                SignonHashEntry* aEntry,
                                                                void* aUserData);

  static PLDHashOperator PR_CALLBACK BuildArrayEnumerator(const nsACString& aKey,
                                                          SignonHashEntry* aEntry,
                                                          void* aUserData);

  static PLDHashOperator PR_CALLBACK BuildRejectArrayEnumerator(const nsACString& aKey,
                                                                PRInt32 aEntry,
                                                                void* aUserData);

  static void EnsureDecoderRing();

  nsClassHashtable<nsCStringHashKey,SignonHashEntry> mSignonTable;
  nsDataHashtable<nsCStringHashKey,PRInt32> mRejectTable;

  nsCOMPtr<nsIFile> mSignonFile;
};

// Our wrapper for username/password prompts - this allows us to prefill
// the password dialog and add a "remember this password" checkbox.

class nsSingleSignonPrompt : public nsIAuthPromptWrapper
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAUTHPROMPT
  NS_DECL_NSIAUTHPROMPTWRAPPER

  nsSingleSignonPrompt() { }
  virtual ~nsSingleSignonPrompt() { }

protected:
  void GetLocalizedString(const nsAString& aKey, nsAString& aResult);

  nsCOMPtr<nsIPrompt> mPrompt;
  nsCOMPtr<nsIStringBundle> mBundle;
};
