/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * Copyright (C) 1999 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

// this file implements the nsMsgDatabase interface using the MDB Interface.


#include "nsMsgDatabase.h"
#include "nsDBFolderInfo.h"
#include "nsNewsSet.h"

#ifdef WE_HAVE_MDBINTERFACES
static NS_DEFINE_IID(kIMDBIID, NS_IMDB_IID);
static NS_DEFINE_IID(kIMBBCID, NS_IMBB_IID);



	nsresult rv = nsRepository::CreateInstance(kMDBCID, nsnull, kIMDBIID, (void **)&gMDBInterface);

	if (nsnull != devSpec){
	  if (NS_OK == ((nsDeviceContextSpecMac *)devSpec)->Init(aQuiet)){
	    aNewSpec = devSpec;
	    rv = NS_OK;
	  }
	}
#endif

nsMsgDatabaseArray *nsMsgDatabase::m_dbCache = NULL;

nsMsgDatabaseArray::nsMsgDatabaseArray()
{
}

//----------------------------------------------------------------------
// GetDBCache
//----------------------------------------------------------------------
nsMsgDatabaseArray *
nsMsgDatabase::GetDBCache()
{
	if (!m_dbCache)
	{
		m_dbCache = new nsMsgDatabaseArray();
	}
	return m_dbCache;
	
}

void
nsMsgDatabase::CleanupCache()
{
	if (m_dbCache) // clean up memory leak
	{
		for (int i = 0; i < GetDBCache()->Count(); i++)
		{
			nsMsgDatabase* pMessageDB = GetDBCache()->GetAt(i);
			if (pMessageDB)
			{
#ifdef DEBUG_bienvenu
				XP_Trace("closing %s\n", pMessageDB->m_dbName);
#endif
				pMessageDB->ForceClosed();
				i--;	// back up array index, since closing removes db from cache.
			}
		}
		XP_ASSERT(GetNumInCache() == 0);	// better not be any open db's.
		delete m_dbCache;
	}
	m_dbCache = NULL; // Need to reset to NULL since it's a
			  // static global ptr and maybe referenced 
			  // again in other places.
}

//----------------------------------------------------------------------
// FindInCache
//----------------------------------------------------------------------
nsMsgDatabase* nsMsgDatabase::FindInCache(nsFilePath &dbName)
{
	for (int i = 0; i < GetDBCache()->Count(); i++)
	{
		nsMsgDatabase* pMessageDB = GetDBCache()->GetAt(i);
		if (pMessageDB->MatchDbName(dbName))
		{
			return(pMessageDB);
		}
	}
	return(NULL);
}

//----------------------------------------------------------------------
// FindInCache
//----------------------------------------------------------------------
int nsMsgDatabase::FindInCache(nsMsgDatabase* pMessageDB)
{
	for (int i = 0; i < GetDBCache()->Count(); i++)
	{
		if (GetDBCache()->GetAt(i) == pMessageDB)
		{
			return(i);
		}
	}
	return(-1);
}

PRBool nsMsgDatabase::MatchDbName(nsFilePath &dbName)	// returns TRUE if they match
{
	// ### we need equality operators for nsFileSpec...
	return strcmp(dbName, m_dbName); 
}

//----------------------------------------------------------------------
// RemoveFromCache
//----------------------------------------------------------------------
void nsMsgDatabase::RemoveFromCache(nsMsgDatabase* pMessageDB)
{
	int i = FindInCache(pMessageDB);
	if (i != -1)
	{
		GetDBCache()->RemoveElementAt(i);
	}
}


#ifdef DEBUG
void nsMsgDatabase::DumpCache()
{
	for (int i = 0; i < GetDBCache()->Count(); i++)
	{
#ifdef DEBUG_bienvenu
		nsMsgDatabase* pMessageDB = 
#endif
        GetDBCache()->GetAt(i);
#ifdef DEBUG_bienvenu
		XP_Trace("db %s in cache use count = %d\n", pMessageDB->m_dbName, pMessageDB->mRefCnt);
#endif
	}
}
#endif /* DEBUG */

nsMsgDatabase::nsMsgDatabase() : m_dbName("")
{
	m_mdbEnv = NULL;
	m_mdbStore = NULL;
	m_mdbAllMsgHeadersTable = NULL;
	m_mdbTokensInitialized = FALSE;
}

nsMsgDatabase::~nsMsgDatabase()
{
}

// ref counting methods - if we inherit from nsISupports, we won't need these,
// and we can take advantage of the nsISupports ref-counting tracing methods
nsrefcnt nsMsgDatabase::AddRef(void)
{
  return ++mRefCnt;
}

nsrefcnt nsMsgDatabase::Release(void)
{
	NS_PRECONDITION(0 != mRefCnt, "dup release");
	if (--mRefCnt == 0) 
	{
		delete this;
		return 0;
	}
	return mRefCnt;
}

/* static */ mdbFactory *nsMsgDatabase::GetMDBFactory()
{
	static mdbFactory *gMDBFactory = NULL;
	if (!gMDBFactory)
	{
		// ### hook up class factory code when it's working
//		gMDBFactory = new mdbFactory;
	}
	return gMDBFactory;
}

// Open the MDB database synchronously. If successful, this routine
// will set up the m_mdbStore and m_mdbEnv of the database object 
// so other database calls can work.
nsresult nsMsgDatabase::OpenMDB(const char *dbName, PRBool create)
{
	nsresult ret = NS_OK;
	mdbFactory *myMDBFactory = GetMDBFactory();
	if (myMDBFactory)
	{
		ret = myMDBFactory->MakeEnv(&m_mdbEnv);
		if (NS_SUCCEEDED(ret))
		{
			mdbThumb *thumb;
			ret = myMDBFactory->OpenFileStore(m_mdbEnv, dbName, NULL, /* const mdbOpenPolicy* inOpenPolicy */
				&thumb); 
			if (NS_SUCCEEDED(ret))
			{
				mdb_count outTotal;    // total somethings to do in operation
				mdb_count outCurrent;  // subportion of total completed so far
				mdb_bool outDone = PR_FALSE;      // is operation finished?
				mdb_bool outBroken;     // is operation irreparably dead and broken?
				do
				{
					ret = thumb->DoMore(m_mdbEnv, &outTotal, &outCurrent, &outDone, &outBroken);
				}
				while (NS_SUCCEEDED(ret) && !outBroken && !outDone);
				if (NS_SUCCEEDED(ret) && outDone)
				{
					ret = myMDBFactory->ThumbToOpenStore(m_mdbEnv, thumb, &m_mdbStore);
					if (ret == NS_OK)
						ret = InitExistingDB();
				}
			}
			else if (create)	// ### need error code saying why open file store failed
			{
				ret = myMDBFactory->CreateNewFileStore(m_mdbEnv, dbName, NULL, &m_mdbStore);
				if (ret == NS_OK)
					ret = InitNewDB();
			}
		}
	}
	return ret;
}

nsresult		nsMsgDatabase::CloseMDB(PRBool commit /* = TRUE */)
{
	--mRefCnt;
	PR_ASSERT(mRefCnt >= 0);
	if (mRefCnt == 0)
	{
		RemoveFromCache(this);
#ifdef DEBUG_bienvenu1
		if (GetNumInCache() != 0)
		{
			XP_Trace("closing %s\n", m_dbName);
			DumpCache();
		}
		XP_Trace("On close - cache used = %lx", CNeoPersist::FCacheUsed);
#endif
		// if this terrifies you, we can make it a static method
		delete this;	
		return(NS_OK);
	}
	else
	{
		return(NS_OK);
	}
}

// force the database to close - this'll flush out anybody holding onto
// a database without having a listener!
// This is evil in the com world, but there are times we need to delete the file.
nsresult nsMsgDatabase::ForceClosed()
{
	nsresult	err = NS_OK;

	while (mRefCnt > 0 && NS_SUCCEEDED(err))
	{
		int32 saveUseCount = mRefCnt;
		err = CloseMDB();
		if (saveUseCount == 1)
			break;
	}
	return err;
}

const char *kMsgHdrsScope = "ns:msg:db:row:scope:msgs:all";
const char *kMsgHdrsTableKind = "ns:msg:db:table:kind:msgs";
const char *kSubjectColumnName = "subject";
const char *kSenderColumnName = "sender";
const char *kMessageIdColumnName = "message-id";
const char *kReferencesColumnName = "references";
const char *kRecipientsColumnName = "recipients";
const char *kDateColumnName = "date";
const char *kMessageSizeColumnName = "size";
const char *kFlagsColumnName = "flags";
const char *kPriorityColumnName = "priority";

struct mdbOid gAllMsgHdrsTableOID;

// set up empty tables, dbFolderInfo, etc.
nsresult nsMsgDatabase::InitNewDB()
{
	nsresult err = NS_OK;

	err = InitMDBInfo();
	if (err == NS_OK)
	{
		nsDBFolderInfo *dbFolderInfo = new nsDBFolderInfo(this);
		if (dbFolderInfo)
		{
			err = dbFolderInfo->AddToNewMDB();
			mdbStore *store = GetStore();
			// create the unique table for the dbFolderInfo.
			mdb_err err = store->NewTable(GetEnv(), m_hdrRowScopeToken, 
				m_hdrTableKindToken, PR_FALSE, &m_mdbAllMsgHeadersTable);

		}
		else
			err = NS_ERROR_OUT_OF_MEMORY;
	}
	return err;
}

nsresult nsMsgDatabase::InitExistingDB()
{
	nsresult err = NS_OK;

	err = InitMDBInfo();
	if (err == NS_OK)
	{
		err = GetStore()->GetTable(GetEnv(), &gAllMsgHdrsTableOID, &m_mdbAllMsgHeadersTable);
	}
	return err;
}

// initialize the various tokens and tables in our db's env
nsresult nsMsgDatabase::InitMDBInfo()
{
	nsresult err = NS_OK;

	if (!m_mdbTokensInitialized && GetStore())
	{
		m_mdbTokensInitialized = TRUE;
		err	= GetStore()->StringToToken(GetEnv(), kMsgHdrsScope, &m_hdrRowScopeToken); 
		if (err == NS_OK)
		{
			GetStore()->StringToToken(GetEnv(),  kSubjectColumnName, &m_subjectColumnToken);
			GetStore()->StringToToken(GetEnv(),  kSenderColumnName, &m_senderColumnToken);
			GetStore()->StringToToken(GetEnv(),  kMessageIdColumnName, &m_messageIdColumnToken);
			// if we just store references as a string, we won't get any savings from the
			// fact there's a lot of duplication. So we may want to break them up into
			// multiple columns, r1, r2, etc.
			GetStore()->StringToToken(GetEnv(),  kReferencesColumnName, &m_referencesColumnToken);
			// similarly, recipients could be tokenized properties
			GetStore()->StringToToken(GetEnv(),  kRecipientsColumnName, &m_recipientsColumnToken);
			GetStore()->StringToToken(GetEnv(),  kDateColumnName, &m_dateColumnToken);
			GetStore()->StringToToken(GetEnv(),  kMessageSizeColumnName, &m_messageSizeColumnToken);
			GetStore()->StringToToken(GetEnv(),  kFlagsColumnName, &m_flagsColumnToken);
			GetStore()->StringToToken(GetEnv(),  kPriorityColumnName, &m_priorityColumnToken);

			err = GetStore()->StringToToken(GetEnv(), kMsgHdrsTableKind, &m_hdrTableKindToken); 
			if (err == NS_OK)
			{
				// The table of all message hdrs will have table id 1.
				gAllMsgHdrsTableOID.mOid_Scope = m_hdrRowScopeToken;
				gAllMsgHdrsTableOID.mOid_Id = 1;
			}
		}
	}
	return err;
}

// get a message header for the given key. Caller must release()!
nsresult nsMsgDatabase::GetMsgHdrForKey(MessageKey messageKey, nsMsgHdr **pmsgHdr)
{
	nsresult	err = NS_OK;
	mdb_pos		rowPos;
	mdbOid		rowObjectId;


	if (!pmsgHdr || !m_mdbAllMsgHeadersTable)
		return NS_ERROR_NULL_POINTER;

	*pmsgHdr = NULL;
	rowObjectId.mOid_Id = messageKey;
	rowObjectId.mOid_Scope = m_hdrRowScopeToken;
	err = m_mdbAllMsgHeadersTable->HasOid(GetEnv(), &rowObjectId, &rowPos);
	if (err == NS_OK)
	{
		mdbTableRowCursor *rowCursor;
		err = m_mdbAllMsgHeadersTable->GetTableRowCursor(GetEnv(), rowPos, &rowCursor);
		if (err == NS_OK && rowPos >= 0) // ### is rowPos > 0 the right thing to check?
		{
			mdbRow	*hdrRow;
			err = rowCursor->NextRow(GetEnv(), &hdrRow, &rowPos);
			if (err == NS_OK)
			{
				*pmsgHdr = new nsMsgHdr(this, hdrRow);
			}
			rowCursor->Release();
		}
	}

	return err;
}

nsresult nsMsgDatabase::DeleteMessages(nsMsgKeyArray &messageKeys, nsIDBChangeListener *instigator)
{
	nsresult	err = NS_OK;

	for (PRUint32 index = 0; index < messageKeys.GetSize(); index++)
	{
		MessageKey messageKey = messageKeys.GetAt(index);
		nsMsgHdr *msgHdr = NULL;
		
		err = GetMsgHdrForKey(messageKey, &msgHdr);
		if (msgHdr == NULL || err != NS_OK)
		{
			err = NS_MSG_MESSAGE_NOT_FOUND;
			break;
		}
		err = DeleteHeader(msgHdr, instigator, index % 300 == 0);
		if (err != NS_OK)
			break;
	}
//	Commit();
	return err;
}


nsresult nsMsgDatabase::DeleteHeader(nsMsgHdr *msgHdr, nsIDBChangeListener *instigator, PRBool commit, PRBool notify /* = TRUE */)
{
	MessageKey	messageKey = msgHdr->GetMessageKey();
	// only need to do this for mail - will this speed up news expiration? Is dirtying objects
	// we're about to delete slow for neoaccess? But are we short circuiting some
	// notifications that we need?
//	if (GetMailDB())
		SetHdrFlag(msgHdr, TRUE, MSG_FLAG_EXPUNGED);	// tell mailbox (mail)

	if (m_newSet)	// if it's in the new set, better get rid of it.
		m_newSet->Remove(msgHdr->GetMessageKey());

	if (m_dbFolderInfo != NULL)
	{
		PRBool isRead;
		m_dbFolderInfo->ChangeNumMessages(-1);
		m_dbFolderInfo->ChangeNumVisibleMessages(-1);
		IsRead(msgHdr->GetMessageKey(), &isRead);
		if (!isRead)
			m_dbFolderInfo->ChangeNumNewMessages(-1);

		m_dbFolderInfo->m_expungedBytes += msgHdr->GetMessageSize();

	}	

	if (notify)
		NotifyKeyChangeAll(messageKey, msgHdr->GetFlags(), instigator); // tell listeners

//	if (!onlyRemoveFromThread)	// to speed up expiration, try this. But really need to do this in RemoveHeaderFromDB
		RemoveHeaderFromDB(msgHdr);
//	if (commit)
//		Commit();			// ### dmb is this a good time to commit?
	msgHdr->Release();	// even though we're deleting it from the db, need to unrefer.
	return NS_OK;
}

// This is a lower level routine which doesn't send notifcations or
// update folder info. One use is when a rule fires moving a header
// from one db to another, to remove it from the first db.

nsresult nsMsgDatabase::RemoveHeaderFromDB(nsMsgHdr *msgHdr)
{
	return NS_OK;
}

nsresult nsMsgDatabase::IsRead(MessageKey messageKey, PRBool *pRead)
{
	nsresult	err = NS_OK;
	nsMsgHdr *msgHdr;

	err = GetMsgHdrForKey(messageKey, &msgHdr);
	if (err == NS_OK && msgHdr != NULL)
	{
		err = IsHeaderRead(msgHdr, pRead);
		msgHdr->Release();
		return err;
	}
	else
	{
		return NS_MSG_MESSAGE_NOT_FOUND;
	}
}


// header iteration methods.

class ListContext
{
public:
	ListContext();
	virtual ~ListContext();
	mdbTableRowCursor *m_rowCursor;
};

ListContext::ListContext()
{
	m_rowCursor = NULL;
}

ListContext::~ListContext()
{
	if (m_rowCursor)
	{
		m_rowCursor->Release();
	}
}

nsresult	nsMsgDatabase::ListFirst(ListContext **ppContext, nsMsgHdr **pResultHdr)
{
	nsresult	err = NS_OK;

	if (!pResultHdr || !ppContext)
		return NS_ERROR_NULL_POINTER;

	*pResultHdr = NULL;
	*ppContext = NULL;

	ListContext *pContext = new ListContext;
	if (!pContext)
		return NS_ERROR_OUT_OF_MEMORY;

	err = m_mdbAllMsgHeadersTable->GetTableRowCursor(GetEnv(), -1, &pContext->m_rowCursor);
	if (err == NS_OK)
	{
		*ppContext = pContext;

		err = ListNext(pContext, pResultHdr);
	}
	return err;
}

nsresult	nsMsgDatabase::ListNext(ListContext *pContext, nsMsgHdr **pResultHdr)
{
	nsresult	err = NS_OK;
	mdbRow	*hdrRow;
	mdb_pos		rowPos;

	if (!pResultHdr || !pContext)
		return NS_ERROR_NULL_POINTER;

	*pResultHdr = NULL;
	err = pContext->m_rowCursor->NextRow(GetEnv(), &hdrRow, &rowPos);
	if (err == NS_OK)
		*pResultHdr = new nsMsgHdr(this, hdrRow);

	return err;
}

nsresult	nsMsgDatabase::ListDone(ListContext *pContext)
{
	nsresult	err = NS_OK;
	delete pContext;
	return err;
}

nsresult nsMsgDatabase::ListAllKeys(nsMsgKeyArray &outputKeys)
{
	nsresult	err = NS_OK;
	mdbTableRowCursor *rowCursor;
	err = m_mdbAllMsgHeadersTable->GetTableRowCursor(GetEnv(), -1, &rowCursor);
	while (err == NS_OK)
	{
		mdbOid outOid;
		mdb_pos	outPos;

		err = rowCursor->NextRowOid(GetEnv(), &outOid, &outPos);
		if (outPos < 0)	// is this right?
			break;
		if (err == NS_OK)
			outputKeys.Add(outOid.mOid_Id);
	}
	return err;
}

nsresult nsMsgDatabase::CreateNewHdr(PRBool *newThread, MessageHdrStruct *hdrStruct, nsMsgHdr **pnewHdr, PRBool notify /* = FALSE */)
{
	nsresult	err = NS_OK;
	mdbRow		*hdrRow;
	struct mdbOid allMsgHdrsTableOID;

	if (!pnewHdr || !m_mdbAllMsgHeadersTable)
		return NS_ERROR_NULL_POINTER;

	allMsgHdrsTableOID.mOid_Scope = m_hdrRowScopeToken;
	allMsgHdrsTableOID.mOid_Id = hdrStruct->m_messageKey;

	err  = GetStore()->NewRowWithOid(GetEnv(), m_hdrRowScopeToken,
		&allMsgHdrsTableOID, &hdrRow);

	// add the row to the singleton table.
	if (NS_SUCCEEDED(err))
	{
		struct mdbYarn yarn;
		char	int32StrBuf[20];

		yarn.mYarn_Grow = NULL;
		hdrRow->AddColumn(GetEnv(),  m_subjectColumnToken, nsStringToYarn(&yarn, &hdrStruct->m_subject));
		delete[] yarn.mYarn_Buf;	// won't need this when we have nsCString
		
		hdrRow->AddColumn(GetEnv(),  m_senderColumnToken, nsStringToYarn(&yarn, &hdrStruct->m_author));
		delete[] yarn.mYarn_Buf;	// won't need this when we have nsCString

		hdrRow->AddColumn(GetEnv(),  m_messageIdColumnToken, nsStringToYarn(&yarn, &hdrStruct->m_messageId));
		delete[] yarn.mYarn_Buf;	// won't need this when we have nsCString

		hdrRow->AddColumn(GetEnv(),  m_referencesColumnToken, nsStringToYarn(&yarn, &hdrStruct->m_references));
		delete[] yarn.mYarn_Buf;	// won't need this when we have nsCString

		hdrRow->AddColumn(GetEnv(),  m_recipientsColumnToken, nsStringToYarn(&yarn, &hdrStruct->m_recipients));
		delete[] yarn.mYarn_Buf;	// won't need this when we have nsCString

		yarn.mYarn_Buf = int32StrBuf;
		yarn.mYarn_Size = sizeof(int32StrBuf);
		yarn.mYarn_Fill = sizeof(int32StrBuf);

		hdrRow->AddColumn(GetEnv(),  m_dateColumnToken, UInt32ToYarn(&yarn, hdrStruct->m_date));

		hdrRow->AddColumn(GetEnv(),  m_messageSizeColumnToken, UInt32ToYarn(&yarn, hdrStruct->m_messageSize));

		hdrRow->AddColumn(GetEnv(),  m_flagsColumnToken, UInt32ToYarn(&yarn, hdrStruct->m_flags));

		hdrRow->AddColumn(GetEnv(),  m_priorityColumnToken, UInt32ToYarn(&yarn, hdrStruct->m_priority));

		err = m_mdbAllMsgHeadersTable->AddRow(GetEnv(), hdrRow);
	}

	if (err == NS_OK)
		*pnewHdr = new nsMsgHdr(this, hdrRow);

	return err;
}

	// extract info from an nsMsgHdr into a MessageHdrStruct
nsresult nsMsgDatabase::GetMsgHdrStructFromnsMsgHdr(nsMsgHdr *msgHdr, MessageHdrStruct &hdrStruct)
{
	nsresult	err = NS_OK;

	if (msgHdr)
	{
		err = RowCellColumnTonsString(msgHdr->GetMDBRow(), m_subjectColumnToken, hdrStruct.m_subject);
		err = RowCellColumnTonsString(msgHdr->GetMDBRow(), m_senderColumnToken, hdrStruct.m_author);
		err = RowCellColumnTonsString(msgHdr->GetMDBRow(), m_messageIdColumnToken, hdrStruct.m_messageId);
		err = RowCellColumnTonsString(msgHdr->GetMDBRow(), m_referencesColumnToken, hdrStruct.m_references);
		err = RowCellColumnTonsString(msgHdr->GetMDBRow(), m_recipientsColumnToken, hdrStruct.m_recipients);
		err = RowCellColumnToUInt32(msgHdr->GetMDBRow(), m_messageSizeColumnToken, &hdrStruct.m_messageSize);
		hdrStruct.m_messageKey = msgHdr->GetMessageKey();
	}
	return err;
}

nsresult nsMsgDatabase::RowCellColumnTonsString(mdbRow *hdrRow, mdb_token columnToken, nsString &resultStr)
{
	nsresult	err = NS_OK;
	mdbCell	*hdrCell;

	err = hdrRow->GetCell(GetEnv(), columnToken, &hdrCell);
	if (err == NS_OK)
	{
		struct mdbYarn yarn;
		hdrCell->AliasYarn(GetEnv(), &yarn);
		YarnTonsString(&yarn, &resultStr);
	}
	return err;
}

nsresult nsMsgDatabase::RowCellColumnToUInt32(mdbRow *hdrRow, mdb_token columnToken, PRUint32 *uint32Result)
{
	nsresult	err = NS_OK;
	mdbCell	*hdrCell;

	err = hdrRow->GetCell(GetEnv(), columnToken, &hdrCell);
	if (err == NS_OK)
	{
		struct mdbYarn yarn;
		hdrCell->AliasYarn(GetEnv(), &yarn);
		YarnToUInt32(&yarn, uint32Result);
	}
	return err;
}


/* static */struct mdbYarn *nsMsgDatabase::nsStringToYarn(struct mdbYarn *yarn, nsString *str)
{
	yarn->mYarn_Buf = str->ToNewCString();
	yarn->mYarn_Size = PL_strlen((const char *) yarn->mYarn_Buf) + 1;
	yarn->mYarn_Fill = yarn->mYarn_Size;
	yarn->mYarn_Form = 0;	// what to do with this? Should be parsed out of the mime2 header?
	return yarn;
}

/* static */struct mdbYarn *nsMsgDatabase::UInt32ToYarn(struct mdbYarn *yarn, PRUint32 i)
{
	PR_snprintf((char *) yarn->mYarn_Buf, yarn->mYarn_Size, "%uld", i);
	yarn->mYarn_Fill = PL_strlen((const char *) yarn->mYarn_Buf) + 1;
	yarn->mYarn_Form = 0;	// what to do with this? Should be parsed out of the mime2 header?
	return yarn;
}

/* static */void nsMsgDatabase::YarnTonsString(struct mdbYarn *yarn, nsString *str)
{
	str->SetString((const char *) yarn->mYarn_Buf, yarn->mYarn_Fill);
}

/* static */void nsMsgDatabase::YarnToUInt32(struct mdbYarn *yarn, PRUint32 *i)
{
	char *endPtr;
	*i = XP_STRTOUL((char *) yarn->mYarn_Buf, &endPtr, yarn->mYarn_Fill); 
}

nsresult nsMsgDatabase::SetSummaryValid(PRBool valid /* = TRUE */)
{
	// setting the version to -1 ought to make it pretty invalid.
	if (!valid)
		m_dbFolderInfo->SetVersion(-1);

	// for default db (and news), there's no nothing to set to make it it valid
	return NS_OK;
}

