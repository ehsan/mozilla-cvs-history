/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Marco Pesenti Gritti <marco@gnome.org>.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

/**
 * We support two builds of the directory service provider.
 * One, linked into the profile component, uses the internal
 * string API. The other can be used by standalone embedding
 * clients, and uses embed strings.
 * To keep the code clean, we are using typedefs to equate
 * embed/internal string types. We are also defining some
 * internal macros in terms of the embedding strings API.
 *
 * When modifying the profile directory service provider, be
 * sure to use methods supported by both the internal and
 * embed strings APIs.
 */

#ifdef STANDALONE_PROFILEDIRSERVICE
#define MOZILLA_STRICT_API
#include "nsEmbedString.h"

typedef nsEmbedString nsString;
typedef nsEmbedCString nsPromiseFlatCString;
typedef nsEmbedCString nsCAutoString;

#define NS_LITERAL_CSTRING nsEmbedCString
#define EmptyString nsEmbedString
#define PromiseFlatCString nsEmbedCString
#define NS_NAMED_LITERAL_STRING(stringname, filename) \
  nsEmbedString stringname; \
  NS_CStringToUTF16(nsEmbedCString(filename), \
                    NS_CSTRING_ENCODING_ASCII, stringname);
#else
#include "nsString.h"
#include "nsPromiseFlatString.h"
#endif
