/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

#ifndef nsRDFDataModelItem_h__
#define nsRDFDataModelItem_h__

#include "rdf.h"
#include "nsIDMItem.h"
#include "nsIRDFResource.h"
#include "nsVector.h"

class nsRDFDataModel;

////////////////////////////////////////////////////////////////////////

class nsRDFDataModelItem : public nsIDMItem, public nsIRDFResource {
private:
    nsRDFDataModel&     mDataModel;
    RDF_Resource        mResource;
    PRBool              mOpen;
    nsVector            mChildren;
    nsRDFDataModelItem* mParent;

    /**
     * A cached value for the size of this item's subtree. Zero means "invalid"
     * or "unknown", and the next call to GetSubtreeSize() will force it to be
     * recomputed.
     */
    mutable PRUint32 mCachedSubtreeSize;

    PRUint32 GetSubtreeSize(void) const;
    void InvalidateCachedSubtreeSize(void);

public:
    nsRDFDataModelItem(nsRDFDataModel& model, RDF_Resource resource);
    virtual ~nsRDFDataModelItem(void);

    ////////////////////////////////////////////////////////////////////////
    // nsISupports interface

    NS_DECL_ISUPPORTS

    ////////////////////////////////////////////////////////////////////////
    // nsIDMItem interface

    // Inspectors
    NS_IMETHOD GetIconImage(nsIImage*& pImage, nsIImageGroup* pGroup) const;
    NS_IMETHOD GetOpenState(PRBool& answer) const;

    // Methods for iterating over children.
    NS_IMETHOD GetChildCount(PRUint32& count) const;
    NS_IMETHOD GetNthChild(nsIDMItem*& pItem, PRUint32 item) const;

    // Parent access
    NS_IMETHOD GetParent(nsIDMItem*& pItem) const;

    // Setters
    NS_IMETHOD SetOpenState(PRBool state); // XXX not there yet...

    // Methods to query the data model for a specific item displayed within the widget.
    NS_IMETHOD GetStringPropertyValue(nsString& value, const nsString& itemProperty) const;
    NS_IMETHOD GetIntPropertyValue(PRInt32& value, const nsString& itemProperty) const;

    ////////////////////////////////////////////////////////////////////////
    // nsIRDFResource interface

    NS_IMETHOD GetResource(RDF_Resource& resource /* out */) const;

    ////////////////////////////////////////////////////////////////////////
    // Implementation methods
    
    nsRDFDataModel& GetDataModel(void) const {
        return mDataModel;
    }

    PRBool IsOpen(void) const {
        return mOpen;
    }

    void Open(void);
    void Close(void);

    nsRDFDataModelItem* ChildAt(PRUint32 index) const {
        return static_cast<nsRDFDataModelItem*>(mChildren[index]);
    }

    void AddChild(nsRDFDataModelItem* child);

    PRUint32 GetChildCount(void) const {
        return mChildren.GetSize();
    }

    nsRDFDataModelItem* GetParent(void) const {
        return mParent;
    }

    nsRDFDataModelItem* GetNth(PRUint32 n) const;
};

////////////////////////////////////////////////////////////////////////


#endif // nsRDFDataModelItem_h__

