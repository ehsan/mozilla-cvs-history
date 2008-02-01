/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
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
 * The Original Code is Mozilla.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications.  Portions created by Netscape Communications are
 * Copyright (C) 2001 by Netscape Communications.  All
 * Rights Reserved.
 * 
 * Contributor(s): 
 *   Vidur Apparao <vidur@netscape.com> (original author)
 */

#include "nsSchemaPrivate.h"

////////////////////////////////////////////////////////////
//
// nsSchemaAttribute implementation
//
////////////////////////////////////////////////////////////
nsSchemaAttribute::nsSchemaAttribute(nsISchema* aSchema, 
                                     const nsAReadableString& aName)
  : nsSchemaComponentBase(aSchema), mName(aName)
{
  NS_INIT_ISUPPORTS();
}

nsSchemaAttribute::~nsSchemaAttribute()
{
}

NS_IMPL_ISUPPORTS3(nsSchemaAttribute,
                   nsISchemaComponent,
                   nsISchemaAttributeComponent,
                   nsISchemaAttribute)


/* void resolve (); */
NS_IMETHODIMP 
nsSchemaAttribute::Resolve()
{
  return NS_OK;
}

/* void clear (); */
NS_IMETHODIMP 
nsSchemaAttribute::Clear()
{
  if (mIsClearing) {
    return NS_OK;
  }

  mIsClearing = PR_TRUE;
  mType->Clear();
  mType = nsnull;
  mIsClearing = PR_FALSE;

  return NS_OK;
}

/* readonly attribute AString name; */
NS_IMETHODIMP 
nsSchemaAttribute::GetName(nsAWritableString & aName)
{
  aName.Assign(mName);

  return NS_OK;
}

/* readonly attribute unsigned short componentType; */
NS_IMETHODIMP 
nsSchemaAttribute::GetComponentType(PRUint16 *aComponentType)
{
  NS_ENSURE_ARG_POINTER(aComponentType);

  *aComponentType = nsISchemaAttributeComponent::COMPONENT_TYPE_ATTRIBUTE;

  return NS_OK;
}

/* readonly attribute nsISchemaSimpleType type; */
NS_IMETHODIMP 
nsSchemaAttribute::GetType(nsISchemaSimpleType * *aType)
{
  NS_ENSURE_ARG_POINTER(aType);

  *aType = mType;
  NS_IF_ADDREF(*aType);

  return NS_OK;
}

/* readonly attribute AString defaultValue; */
NS_IMETHODIMP 
nsSchemaAttribute::GetDefaultValue(nsAWritableString & aDefaultValue)
{
  aDefaultValue.Assign(mDefaultValue);
  
  return NS_OK;
}

/* readonly attribute AString fixedValue; */
NS_IMETHODIMP 
nsSchemaAttribute::GetFixedValue(nsAWritableString & aFixedValue)
{
  aFixedValue.Assign(mFixedValue);
  
  return NS_OK;
}

/* readonly attribute unsigned short use; */
NS_IMETHODIMP 
nsSchemaAttribute::GetUse(PRUint16 *aUse)
{
  NS_ENSURE_ARG_POINTER(aUse);

  *aUse = mUse;

  return NS_OK;
}

NS_IMETHODIMP 
nsSchemaAttribute::SetType(nsISchemaSimpleType* aType)
{
  NS_ENSURE_ARG(aType);
  
  mType = aType;

  return NS_OK;
}

NS_IMETHODIMP
nsSchemaAttribute::SetConstraints(const nsAReadableString& aDefaultValue,
                                  const nsAReadableString& aFixedValue)
{
  mDefaultValue.Assign(aDefaultValue);
  mFixedValue.Assign(aFixedValue);

  return NS_OK;
}
 
NS_IMETHODIMP
nsSchemaAttribute::SetUse(PRUint16 aUse)
{
  mUse = aUse;

  return NS_OK;
}

////////////////////////////////////////////////////////////
//
// nsSchemaAttributeRef implementation
//
////////////////////////////////////////////////////////////
nsSchemaAttributeRef::nsSchemaAttributeRef(nsISchema* aSchema, 
                                           const nsAReadableString& aRef)
  : nsSchemaComponentBase(aSchema), mRef(aRef)
{
  NS_INIT_ISUPPORTS();
}

nsSchemaAttributeRef::~nsSchemaAttributeRef()
{
}

NS_IMPL_ISUPPORTS3(nsSchemaAttributeRef,
                   nsISchemaComponent,
                   nsISchemaAttributeComponent,
                   nsISchemaAttribute)


/* void resolve (); */
NS_IMETHODIMP 
nsSchemaAttributeRef::Resolve()
{
  nsresult rv = NS_OK;
  if (mIsResolving) {
    return NS_OK;
  }
  
  mIsResolving = PR_TRUE;
  if (!mAttribute && mSchema) {
    mSchema->GetAttributeByName(mRef, getter_AddRefs(mAttribute));
  }

  if (mAttribute) {
    rv = mAttribute->Resolve();
  }
  mIsResolving = PR_FALSE;

  return rv;
}

/* void clear (); */
NS_IMETHODIMP 
nsSchemaAttributeRef::Clear()
{
  if (mIsClearing) {
    return NS_OK;
  }

  mIsClearing = PR_TRUE;
  mAttribute->Clear();
  mAttribute = nsnull;
  mIsClearing = PR_FALSE;

  return NS_OK;
}

/* readonly attribute AString name; */
NS_IMETHODIMP 
nsSchemaAttributeRef::GetName(nsAWritableString & aName)
{
  if (!mAttribute) {
    return NS_ERROR_NOT_INITIALIZED;
  }

  return mAttribute->GetName(aName);
}

/* readonly attribute unsigned short componentType; */
NS_IMETHODIMP 
nsSchemaAttributeRef::GetComponentType(PRUint16 *aComponentType)
{
  NS_ENSURE_ARG_POINTER(aComponentType);

  *aComponentType = nsISchemaAttributeComponent::COMPONENT_TYPE_ATTRIBUTE;

  return NS_OK;
}

/* readonly attribute nsISchemaSimpleType type; */
NS_IMETHODIMP 
nsSchemaAttributeRef::GetType(nsISchemaSimpleType * *aType)
{
  NS_ENSURE_ARG_POINTER(aType);

  if (!mAttribute) {
    return NS_ERROR_NOT_INITIALIZED;
  }

  return mAttribute->GetType(aType);
}

/* readonly attribute AString defaultValue; */
NS_IMETHODIMP 
nsSchemaAttributeRef::GetDefaultValue(nsAWritableString & aDefaultValue)
{
  aDefaultValue.Assign(mDefaultValue);
  
  return NS_OK;
}

/* readonly attribute AString fixedValue; */
NS_IMETHODIMP 
nsSchemaAttributeRef::GetFixedValue(nsAWritableString & aFixedValue)
{
  aFixedValue.Assign(mFixedValue);
  
  return NS_OK;
}

/* readonly attribute unsigned short use; */
NS_IMETHODIMP 
nsSchemaAttributeRef::GetUse(PRUint16 *aUse)
{
  NS_ENSURE_ARG_POINTER(aUse);

  *aUse = mUse;

  return NS_OK;
}

NS_IMETHODIMP
nsSchemaAttributeRef::SetConstraints(const nsAReadableString& aDefaultValue,
                                     const nsAReadableString& aFixedValue)
{
  mDefaultValue.Assign(aDefaultValue);
  mFixedValue.Assign(aFixedValue);

  return NS_OK;
}
 
NS_IMETHODIMP
nsSchemaAttributeRef::SetUse(PRUint16 aUse)
{
  mUse = aUse;

  return NS_OK;
}

////////////////////////////////////////////////////////////
//
// nsSchemaAttributeGroup implementation
//
////////////////////////////////////////////////////////////
nsSchemaAttributeGroup::nsSchemaAttributeGroup(nsISchema* aSchema,
                                               const nsAReadableString& aName)
  : nsSchemaComponentBase(aSchema), mName(aName)
{
  NS_INIT_ISUPPORTS();
}

nsSchemaAttributeGroup::~nsSchemaAttributeGroup()
{
}

NS_IMPL_ISUPPORTS3(nsSchemaAttributeGroup,
                   nsISchemaComponent,
                   nsISchemaAttributeComponent,
                   nsISchemaAttributeGroup)

/* void resolve (); */
NS_IMETHODIMP 
nsSchemaAttributeGroup::Resolve()
{
  if (mIsResolving) {
    return NS_OK;
  }

  mIsResolving = PR_TRUE;
  nsresult rv;
  PRUint32 i, count;

  mAttributes.Count(&count);
  for (i = 0; i < count; i++) {
    nsCOMPtr<nsISchemaAttributeComponent> attribute;
    
    rv = mAttributes.QueryElementAt(i, NS_GET_IID(nsISchemaAttributeComponent),
                                    getter_AddRefs(attribute));
    if (NS_SUCCEEDED(rv)) {
      rv = attribute->Resolve();
      if (NS_FAILED(rv)) {
        mIsResolving = PR_FALSE;
        return rv;
      }
    }
  }
  mIsResolving = PR_FALSE;
  
  return NS_OK;
}

/* void clear (); */
NS_IMETHODIMP 
nsSchemaAttributeGroup::Clear()
{
  if (mIsClearing) {
    return NS_OK;
  }

  mIsClearing = PR_TRUE;
  nsresult rv;
  PRUint32 i, count;
  mAttributes.Count(&count);
  for (i = 0; i < count; i++) {
    nsCOMPtr<nsISchemaAttributeComponent> attribute;
    
    rv = mAttributes.QueryElementAt(i, NS_GET_IID(nsISchemaAttributeComponent),
                                    getter_AddRefs(attribute));
    if (NS_SUCCEEDED(rv)) {
      attribute->Clear();
    }
  }
  mAttributes.Clear();
  mAttributesHash.Reset();
  mIsClearing = PR_FALSE;

  return NS_OK;
}

/* readonly attribute AString name; */
NS_IMETHODIMP 
nsSchemaAttributeGroup::GetName(nsAWritableString & aName)
{
  aName.Assign(mName);

  return NS_OK;
}

/* readonly attribute unsigned short componentType; */
NS_IMETHODIMP 
nsSchemaAttributeGroup::GetComponentType(PRUint16 *aComponentType)
{
  NS_ENSURE_ARG_POINTER(aComponentType);

  *aComponentType = nsISchemaAttributeComponent::COMPONENT_TYPE_GROUP;

  return NS_OK;
}

/* readonly attribute PRUint32 attributeCount; */
NS_IMETHODIMP 
nsSchemaAttributeGroup::GetAttributeCount(PRUint32 *aAttributeCount)
{
  NS_ENSURE_ARG_POINTER(aAttributeCount);
  
  return mAttributes.Count(aAttributeCount);
}

/* nsISchemaAttributeComponent getAttributeByIndex (in PRUint32 index); */
NS_IMETHODIMP 
nsSchemaAttributeGroup::GetAttributeByIndex(PRUint32 index, 
                                            nsISchemaAttributeComponent **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);

  return mAttributes.QueryElementAt(index, 
                                    NS_GET_IID(nsISchemaAttributeComponent),
                                    (void**)_retval);
}

/* nsISchemaAttributeComponent getAttributeByName (in AString name); */
NS_IMETHODIMP 
nsSchemaAttributeGroup::GetAttributeByName(const nsAReadableString & name, 
                                           nsISchemaAttributeComponent **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);

  nsStringKey key(name);
  nsCOMPtr<nsISupports> sup = dont_AddRef(mAttributesHash.Get(&key));

  if (sup) {
    return CallQueryInterface(sup, _retval);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsSchemaAttributeGroup::AddAttribute(nsISchemaAttributeComponent* aAttribute)
{
  NS_ENSURE_ARG_POINTER(aAttribute);

  nsAutoString name;
  aAttribute->GetName(name);

  mAttributes.AppendElement(aAttribute);
  nsStringKey key(name);
  mAttributesHash.Put(&key, aAttribute);

  return NS_OK;    
}

////////////////////////////////////////////////////////////
//
// nsSchemaAttributeGroupRef implementation
//
////////////////////////////////////////////////////////////
nsSchemaAttributeGroupRef::nsSchemaAttributeGroupRef(nsISchema* aSchema,
                                                     const nsAReadableString& aRef)
  : nsSchemaComponentBase(aSchema), mRef(aRef)
{
  NS_INIT_ISUPPORTS();
}

nsSchemaAttributeGroupRef::~nsSchemaAttributeGroupRef()
{
}

NS_IMPL_ISUPPORTS3(nsSchemaAttributeGroupRef,
                   nsISchemaComponent,
                   nsISchemaAttributeComponent,
                   nsISchemaAttributeGroup)

/* void resolve (); */
NS_IMETHODIMP 
nsSchemaAttributeGroupRef::Resolve()
{
  nsresult rv = NS_OK;
  if (mIsResolving) {
    return NS_OK;
  }

  mIsResolving = PR_TRUE;
  if (!mAttributeGroup && mSchema) {
    mSchema->GetAttributeGroupByName(mRef, getter_AddRefs(mAttributeGroup));
  }

  if (mAttributeGroup) {
    rv = mAttributeGroup->Resolve();
  }
  mIsResolving = PR_FALSE;
  
  return rv;
}

/* void clear (); */
NS_IMETHODIMP 
nsSchemaAttributeGroupRef::Clear()
{
  if (mIsClearing) {
    return NS_OK;
  }

  mIsClearing = PR_TRUE;
  mAttributeGroup->Clear();
  mAttributeGroup = nsnull;
  mIsClearing = PR_FALSE;

  return NS_OK;
}

/* readonly attribute AString name; */
NS_IMETHODIMP 
nsSchemaAttributeGroupRef::GetName(nsAWritableString & aName)
{
  if (!mAttributeGroup) {
    return NS_ERROR_NOT_INITIALIZED;
  }

  return mAttributeGroup->GetName(aName);
}

/* readonly attribute unsigned short componentType; */
NS_IMETHODIMP 
nsSchemaAttributeGroupRef::GetComponentType(PRUint16 *aComponentType)
{
  NS_ENSURE_ARG_POINTER(aComponentType);

  *aComponentType = nsISchemaAttributeComponent::COMPONENT_TYPE_GROUP;

  return NS_OK;
}

/* readonly attribute PRUint32 attributeCount; */
NS_IMETHODIMP 
nsSchemaAttributeGroupRef::GetAttributeCount(PRUint32 *aAttributeCount)
{
  NS_ENSURE_ARG_POINTER(aAttributeCount);

  if (!mAttributeGroup) {
    return NS_ERROR_NOT_INITIALIZED;
  }
  
  return mAttributeGroup->GetAttributeCount(aAttributeCount);
}

/* nsISchemaAttributeComponent getAttributeByIndex (in PRUint32 index); */
NS_IMETHODIMP 
nsSchemaAttributeGroupRef::GetAttributeByIndex(PRUint32 index, 
                                               nsISchemaAttributeComponent **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);

  if (!mAttributeGroup) {
    return NS_ERROR_NOT_INITIALIZED;
  }
  
  return mAttributeGroup->GetAttributeByIndex(index, _retval);
}

/* nsISchemaAttributeComponent getAttributeByName (in AString name); */
NS_IMETHODIMP 
nsSchemaAttributeGroupRef::GetAttributeByName(const nsAReadableString & name, 
                                              nsISchemaAttributeComponent **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);

  if (!mAttributeGroup) {
    return NS_ERROR_NOT_INITIALIZED;
  }

  return mAttributeGroup->GetAttributeByName(name, _retval);
}

////////////////////////////////////////////////////////////
//
// nsSchemaAnyAttribute implementation
//
////////////////////////////////////////////////////////////
nsSchemaAnyAttribute::nsSchemaAnyAttribute(nsISchema* aSchema)
  : nsSchemaComponentBase(aSchema), mProcess(PROCESS_STRICT)
{
  NS_INIT_ISUPPORTS();
}

nsSchemaAnyAttribute::~nsSchemaAnyAttribute()
{
}

NS_IMPL_ISUPPORTS3(nsSchemaAnyAttribute,
                   nsISchemaComponent,
                   nsISchemaAttributeComponent,
                   nsISchemaAnyAttribute)

/* void resolve (); */
NS_IMETHODIMP 
nsSchemaAnyAttribute::Resolve()
{
  return NS_OK;
}

/* void clear (); */
NS_IMETHODIMP 
nsSchemaAnyAttribute::Clear()
{
  return NS_OK;
}

/* readonly attribute unsigned short componentType; */
NS_IMETHODIMP 
nsSchemaAnyAttribute::GetComponentType(PRUint16 *aComponentType)
{
  NS_ENSURE_ARG_POINTER(aComponentType);

  *aComponentType = nsISchemaAttributeComponent::COMPONENT_TYPE_ANY;

  return NS_OK;
}

/* readonly attribute AString name; */
NS_IMETHODIMP 
nsSchemaAnyAttribute::GetName(nsAWritableString & aName)
{
  aName.Assign(NS_LITERAL_STRING("anyAttribute"));

  return NS_OK;
}

/* readonly attribute unsigned short process; */
NS_IMETHODIMP 
nsSchemaAnyAttribute::GetProcess(PRUint16 *aProcess)
{
  NS_ENSURE_ARG_POINTER(aProcess);

  *aProcess = mProcess;

  return NS_OK;
}

/* readonly attribute AString namespace; */
NS_IMETHODIMP 
nsSchemaAnyAttribute::GetNamespace(nsAWritableString & aNamespace)
{
  aNamespace.Assign(mNamespace);

  return NS_OK;
}

NS_IMETHODIMP 
nsSchemaAnyAttribute::SetProcess(PRUint16 aProcess)
{
  mProcess = aProcess;
  
  return NS_OK;
}
 
NS_IMETHODIMP 
nsSchemaAnyAttribute::SetNamespace(const nsAReadableString& aNamespace)
{
  mNamespace.Assign(aNamespace);

  return NS_OK;
}
