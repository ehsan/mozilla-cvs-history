/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
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
 

#include "nsDTDUtils.h"
#include "CNavDTD.h" 
#include "nsIObserverService.h"
#include "nsIServiceManager.h"
#include "nsIElementObserver.h"

static NS_DEFINE_IID(kIObserverServiceIID, NS_IOBSERVERSERVICE_IID);
static NS_DEFINE_IID(kObserverServiceCID, NS_OBSERVERSERVICE_CID);

 
/***************************************************************
  First, define the tagstack class
 ***************************************************************/


/**
 * Default constructor
 * @update	harishd 04/04/99
 * @update  gess 04/22/99
 */
nsEntryStack::nsEntryStack()  {
  mCapacity=0;
  mCount=0;
  mEntries=0;
}

/**
 * Default destructor
 * @update  harishd 04/04/99
 * @update  gess 04/22/99
 */
nsEntryStack::~nsEntryStack() {
  if(mEntries)
    delete [] mEntries;
}

/**
 * Resets state of stack to be empty.
 * @update harishd 04/04/99
 */
void nsEntryStack::Empty(void) {
  mCount=0;
}

/**
 * 
 * @update  gess 04/22/99
 */
void nsEntryStack::Push(eHTMLTags aTag) {
  if(mCount==mCapacity){ 
    nsTagEntry* temp=new nsTagEntry[mCapacity+=50]; 
    PRUint32 index=0; 
    for(index=0;index<mCount;index++) {
      temp[index]=mEntries[index];
    }
    delete [] mEntries;
    mEntries=temp;
  }
  mEntries[mCount].mTag=aTag;
  mEntries[mCount].mBankIndex=-1;
  mEntries[mCount++].mStyleIndex=-1;
}


/**
 * 
 * @update  harishd 04/04/99
 * @update  gess 04/21/99
 */
eHTMLTags nsEntryStack::Pop() {
  eHTMLTags result=eHTMLTag_unknown;
  if(0<mCount) {
    result=mEntries[--mCount].mTag;
  }
  return result;
} 
 
/**
 * 
 * @update  harishd 04/04/99
 * @update  gess 04/21/99
 */
eHTMLTags nsEntryStack::First() const {
  eHTMLTags result=eHTMLTag_unknown;
  if(0<mCount){
    result=mEntries[0].mTag;
  }
  return result;
}

/**
 * 
 * @update  harishd 04/04/99
 * @update  gess 04/21/99
 */
eHTMLTags nsEntryStack::TagAt(PRUint32 anIndex) const {
  eHTMLTags result=eHTMLTag_unknown;
  if(anIndex<mCount) {
    result=mEntries[anIndex].mTag;
  }
  return result;
}

/**
 * 
 * @update  gess 04/21/99
 */
nsTagEntry& nsEntryStack::EntryAt(PRUint32 anIndex) const {
  static nsTagEntry gSentinel;
  if(anIndex<mCount) {
    return mEntries[anIndex];
  }
  return gSentinel;
}


/**
 * 
 * @update  harishd 04/04/99
 * @update  gess 04/21/99
 */
eHTMLTags nsEntryStack::operator[](PRUint32 anIndex) const {
  eHTMLTags result=eHTMLTag_unknown;
  if(anIndex<mCount) {
    result=mEntries[anIndex].mTag;
  }
  return result;
}


/**
 * 
 * @update  harishd 04/04/99
 * @update  gess 04/21/99
 */
eHTMLTags nsEntryStack::Last() const {
  return TagAt(mCount-1);
}

/**
 * 
 * @update  harishd 04/04/99
 * @update  gess 04/21/99
 */
PRInt32 nsEntryStack::GetTopmostIndexOf(eHTMLTags aTag) const {
  int theIndex=0;
  for(theIndex=mCount-1;theIndex>=0;theIndex--){
    if(aTag==TagAt(theIndex))
      return theIndex;
  }
  return kNotFound;

}




/***************************************************************
  Now define the dtdcontext class
 ***************************************************************/


/**
 * 
 * @update	gess9/10/98
 * @update  gess 4/26/99
 */
nsDTDContext::nsDTDContext() : mStack(), mSkipped(0), mStyles(0) {
#ifdef  NS_DEBUG
  nsCRT::zero(mTags,sizeof(mTags));
#endif
} 
 

/**
 * 
 * @update	gess9/10/98
 * @update  gess 4/26/99
 */
nsDTDContext::~nsDTDContext() {
}

/**
 * 
 * @update  gess7/9/98, harishd 04/04/99 
 * @update  gess 4/26/99
 */
PRInt32 nsDTDContext::GetCount(void) {
  return mStack.GetSize();
}

/**
 * 
 * @update  gess7/9/98, harishd 04/04/99
 * @update  gess 4/26/99
 */
void nsDTDContext::Push(eHTMLTags aTag) {
#ifdef  NS_DEBUG
  if(mStack.mCount<sizeof(mTags))
    mTags[mStack.mCount]=aTag;
#endif

  mStack.Push(aTag);
}

/** 
 * @update  gess7/9/98, harishd 04/04/99
 * @update  gess 4/26/99
 */
eHTMLTags nsDTDContext::Pop() {
#ifdef  NS_DEBUG
  if(mStack.mCount>0)
    mTags[mStack.mCount-1]=eHTMLTag_unknown;
#endif

  eHTMLTags result=mStack.Pop();
  return result;
}

/**
 * 
 * @update  gess7/9/98
 * @update  gess 4/26/99
 */
eHTMLTags nsDTDContext::First() const {
  return mStack.First();
}

/**
 * 
 * @update  gess7/9/98
 * @update  gess 4/26/99
 */
eHTMLTags nsDTDContext::TagAt(PRInt32 anIndex) const {
  return mStack.TagAt(anIndex);
}


/**
 * 
 * @update  gess7/9/98
 * @update  gess 4/26/99
 */
eHTMLTags nsDTDContext::operator[](PRInt32 anIndex) const {
  return mStack[anIndex];
}

/**
 * 
 * @update  gess7/9/98
 * @update  gess 4/26/99
 */
eHTMLTags nsDTDContext::Last() const {
  return mStack.Last();
}

/**
 * 
 * @update  gess7/9/98
 * @update  gess 4/26/99
 */
nsEntryStack* nsDTDContext::GetStyles(void) const {
  nsEntryStack* result=0;
  if(0<mStack.mCount){
    PRInt32 theIndex=mStack.mEntries[mStack.mCount-1].mStyleIndex;  
    if(-1<theIndex){
      result=(nsEntryStack*)mStyles.ObjectAt(theIndex);
    }
  }
  return result;
}

/**
 * 
 * @update  harishd 04/04/99
 * @update  gess 04/21/99
 */
void nsDTDContext::SaveToken(CToken* aToken, PRInt32 aID)
{ 
  NS_PRECONDITION(aID <= mStack.GetSize() && aID > -1,"Out of bounds");

  if(aToken) {
    nsTagEntry& theEntry=mStack.EntryAt(aID);
    //ok, now go get the right tokenbank deque...
    nsDeque* theDeque=0;
    if(-1<theEntry.mBankIndex)
      theDeque=(nsDeque*)mSkipped.ObjectAt(theEntry.mBankIndex);
    if(!theDeque){
      //time to make a databank for this element...
      theDeque=new nsDeque(0);
      mSkipped.Push(theDeque);
      theEntry.mBankIndex=mSkipped.GetSize()-1;
    }
    theDeque->Push(aToken);
  }
}

/**
 * 
 * @update  harishd 04/04/99
 * @update  gess 04/21/99
 */
CToken*  nsDTDContext::RestoreTokenFrom(PRInt32 aID)
{ 
  NS_PRECONDITION(aID <= mStack.GetSize() && aID > -1,"Out of bounds");
  CToken* result=0;
  if(0<mStack.GetSize()) {
    nsTagEntry theEntry=mStack.EntryAt(aID);
    nsDeque* theDeque=(nsDeque*)mSkipped.ObjectAt(theEntry.mBankIndex);
    if(theDeque){
      result=(CToken*)theDeque->PopFront();
    }
  }
  return result;
}

/**
 * 
 * @update  harishd 04/04/99
 * @update  gess 04/21/99
 */
PRInt32  nsDTDContext::TokenCountAt(PRInt32 aID) 
{ 
  NS_PRECONDITION(aID <= mStack.GetSize(),"Out of bounds");

  nsTagEntry theEntry=mStack.EntryAt(aID);
  nsDeque* theDeque=(nsDeque*)mSkipped.ObjectAt(theEntry.mBankIndex);
  if(theDeque){
    return theDeque->GetSize();
  }

  return kNotFound;
}

/**************************************************************
  Now define the tokenrecycler class...
 **************************************************************/

/**
 * 
 * @update  gess7/25/98
 * @param 
 */
CTokenRecycler::CTokenRecycler() : nsITokenRecycler() {
  int i=0;
  for(i=0;i<eToken_last-1;i++) {
    mTokenCache[i]=new nsDeque(new CTokenDeallocator());
#ifdef NS_DEBUG
    mTotals[i]=0;
#endif
  }
}

/**
 * Destructor for the token factory
 * @update  gess7/25/98
 */
CTokenRecycler::~CTokenRecycler() {
  //begin by deleting all the known (recycled) tokens...
  //We're also deleting the cache-deques themselves.
  int i;
  for(i=0;i<eToken_last-1;i++) {
    if(0!=mTokenCache[i]) {
      delete mTokenCache[i];
      mTokenCache[i]=0;
    }
  }
}

class CTokenFinder: public nsDequeFunctor{
public:
  CTokenFinder(CToken* aToken) {mToken=aToken;}
  virtual void* operator()(void* anObject) {
    if(anObject==mToken) {
      return anObject;
    }
    return 0;
  }
  CToken* mToken;
};

/**
 * This method gets called when someone wants to recycle a token
 * @update  gess7/24/98
 * @param   aToken -- token to be recycled.
 * @return  nada
 */
void CTokenRecycler::RecycleToken(CToken* aToken) {
  if(aToken) {
    PRInt32 theType=aToken->GetTokenType();
    CTokenFinder finder(aToken);
    CToken* theMatch=(CToken*)mTokenCache[theType-1]->FirstThat(finder);
    if(theMatch) {
      int x=5;
    }
    mTokenCache[theType-1]->Push(aToken);
  }
}


/**
 * 
 * @update	vidur 11/12/98
 * @param 
 * @return
 */
CToken* CTokenRecycler::CreateTokenOfType(eHTMLTokenTypes aType,eHTMLTags aTag, const nsString& aString) {

  CToken* result=(CToken*)mTokenCache[aType-1]->Pop();

  if(result) {
    result->Reinitialize(aTag,aString);
  }
  else {
#ifdef  NS_DEBUG
    mTotals[aType-1]++;
#endif
    switch(aType){
      case eToken_start:      result=new CStartToken(aTag); break;
      case eToken_end:        result=new CEndToken(aTag); break;
      case eToken_comment:    result=new CCommentToken(); break;
      case eToken_entity:     result=new CEntityToken(); break;
      case eToken_whitespace: result=new CWhitespaceToken(); break;
      case eToken_newline:    result=new CNewlineToken(); break;
      case eToken_text:       result=new CTextToken(aString); break;
      case eToken_attribute:  result=new CAttributeToken(); break;
      case eToken_script:     result=new CScriptToken(); break;
      case eToken_style:      result=new CStyleToken(); break;
      case eToken_skippedcontent: result=new CSkippedContentToken(aString); break;
      case eToken_instruction:result=new CInstructionToken(); break;
      case eToken_cdatasection:result=new CCDATASectionToken(); break;
        default:
          break;
    }
  }
  return result;
}

/**
 * 
 * @update	vidur 11/12/98
 * @param 
 * @return
 */
CToken* CTokenRecycler::CreateTokenOfType(eHTMLTokenTypes aType,eHTMLTags aTag) {

  CToken* result=(CToken*)mTokenCache[aType-1]->Pop();

  static nsAutoString theEmpty;
  if(result) {
    result->Reinitialize(aTag,theEmpty);
  }
  else {
#ifdef  NS_DEBUG
    mTotals[aType-1]++;
#endif
    switch(aType){
      case eToken_start:            result=new CStartToken(aTag); break;
      case eToken_end:              result=new CEndToken(aTag); break;
      case eToken_comment:          result=new CCommentToken(); break;
      case eToken_attribute:        result=new CAttributeToken(); break;
      case eToken_entity:           result=new CEntityToken(); break;
      case eToken_whitespace:       result=new CWhitespaceToken(); break;
      case eToken_newline:          result=new CNewlineToken(); break;
      case eToken_text:             result=new CTextToken(theEmpty); break;
      case eToken_script:           result=new CScriptToken(); break;
      case eToken_style:            result=new CStyleToken(); break;
      case eToken_skippedcontent:   result=new CSkippedContentToken(theEmpty); break;
      case eToken_instruction:      result=new CInstructionToken(); break;
      case eToken_cdatasection:     result=new CCDATASectionToken(); break;
      case eToken_error:            result=new CErrorToken(); break;
        default:
          break;
    }
  }
  return result;
}

void DebugDumpContainmentRules(nsIDTD& theDTD,const char* aFilename,const char* aTitle) {
  const char* prefix="     ";
  fstream out(aFilename,ios::out);
  out << "==================================================" << endl;
  out << aTitle << endl;
  out << "==================================================";
  int i,j=0;
  int written;
  for(i=1;i<eHTMLTag_text;i++){
    const char* tag=NS_EnumToTag((eHTMLTags)i);
    out << endl << endl << "Tag: <" << tag << ">" << endl;
    out << prefix;
    written=0;
    if(theDTD.IsContainer(i)) {
      for(j=1;j<eHTMLTag_text;j++){
        if(15==written) {
          out << endl << prefix;
          written=0;
        }
        if(theDTD.CanContain(i,j)){
          tag=NS_EnumToTag((eHTMLTags)j);
          if(tag) {
            out<< tag << ", ";
            written++;
          }
        }
      }//for
    }
    else out<<"(not container)" << endl;
  }
}


/*************************************************************************
 *  The table lookup technique was adapted from the algorithm described  *
 *  by Avram Perez, Byte-wise CRC Calculations, IEEE Micro 3, 40 (1983). *
 *************************************************************************/

#define POLYNOMIAL 0x04c11db7L

static PRUint32 crc_table[256];

static
void gen_crc_table() {
 /* generate the table of CRC remainders for all possible bytes */
  int i, j;  
  PRUint32 crc_accum;
  for ( i = 0;  i < 256;  i++ ) { 
    crc_accum = ( (unsigned long) i << 24 );
    for ( j = 0;  j < 8;  j++ ) { 
      if ( crc_accum & 0x80000000L )
        crc_accum = ( crc_accum << 1 ) ^ POLYNOMIAL;
      else crc_accum = ( crc_accum << 1 ); 
    }
    crc_table[i] = crc_accum; 
  }
  return; 
}

class CRCInitializer {
  public: 
    CRCInitializer() {
      gen_crc_table();
    }
};
CRCInitializer gCRCInitializer;


PRUint32 AccumulateCRC(PRUint32 crc_accum, char *data_blk_ptr, int data_blk_size)  {
 /* update the CRC on the data block one byte at a time */
  int i, j;
  for ( j = 0;  j < data_blk_size;  j++ ) { 
    i = ( (int) ( crc_accum >> 24) ^ *data_blk_ptr++ ) & 0xff;
    crc_accum = ( crc_accum << 8 ) ^ crc_table[i]; 
  }
  return crc_accum; 
}

/******************************************************************************
  This class is used to store ref's to tag observers during the parse phase.
  Note that for simplicity, this is a singleton that is constructed in the
  CNavDTD and shared for the duration of the application session. Later on it
  might be nice to use a more dynamic approach that would permit observers to
  come and go on a document basis.
 ******************************************************************************/

CObserverDictionary::CObserverDictionary() {
  nsCRT::zero(mObservers,sizeof(mObservers));
  RegisterObservers();
}

CObserverDictionary::~CObserverDictionary() {
  UnregisterObservers();
}

/**************************************************************
  Define the nsIElementObserver release class...
 **************************************************************/
class nsObserverReleaser: public nsDequeFunctor{
public:
  virtual void* operator()(void* anObject) {
    nsIElementObserver* theObserver= (nsIElementObserver*)anObject;
    NS_RELEASE(theObserver);
    return 0;
  }
};

void CObserverDictionary::UnregisterObservers() {
  int theIndex=0;
  nsObserverReleaser theReleaser;
  for(theIndex=0;theIndex<NS_HTML_TAG_MAX;theIndex++){
    if(mObservers[theIndex]){
      nsIElementObserver* theElementObserver=0;
      mObservers[theIndex]->ForEach(theReleaser);
      delete mObservers[theIndex];
    }
  }
}

void CObserverDictionary::RegisterObservers() {
  nsresult result = NS_OK;
  nsIObserverService* theObserverService = nsnull;
  result = nsServiceManager::GetService(kObserverServiceCID, kIObserverServiceIID,
                                      (nsISupports**) &theObserverService, nsnull);
  if(result == NS_OK){
    nsString  theTopic("htmlparser");
    nsIEnumerator* theEnum;
    result = theObserverService->EnumerateObserverList(&theEnum, &theTopic);
    if(result == NS_OK){
      nsIElementObserver* theElementObserver;
      nsISupports *inst;
      
      for (theEnum->First(); theEnum->IsDone() != NS_OK; theEnum->Next()) {
        result = theEnum->CurrentItem(&inst);
        if (NS_SUCCEEDED(result))
          result = inst->QueryInterface(nsIElementObserver::GetIID(), (void**)&theElementObserver);
        if(result == NS_OK) {
          const char* theTagStr;
          theTagStr = theElementObserver->GetTagName();
          if(theTagStr != nsnull) {
            eHTMLTags theTag = NS_TagToEnum(theTagStr);
            if(eHTMLTag_userdefined!=theTag){
              if(mObservers[theTag] == nsnull) {
                 mObservers[theTag] = new nsDeque(0);
              }
              NS_ADDREF(theElementObserver);
              mObservers[theTag]->Push(theElementObserver);
            }
          }
        }
      }
    }
  }
}

nsDeque* CObserverDictionary::GetObserversForTag(eHTMLTags aTag) {
  return mObservers[aTag];
}
