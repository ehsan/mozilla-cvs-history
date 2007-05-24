/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#ifndef nsCycleCollectionParticipant_h__
#define nsCycleCollectionParticipant_h__

#include "nsISupports.h"

#define NS_CYCLECOLLECTIONPARTICIPANT_IID                                      \
{                                                                              \
    0x8057ad9e,                                                                \
    0x3ecc,                                                                    \
    0x4e89,                                                                    \
    { 0x89, 0xac, 0x0a, 0x71, 0xf3, 0x8b, 0x14, 0xc2 }                         \
}

/**
 * Special IID to get at the base nsISupports for a class. Usually this is the
 * canonical nsISupports pointer, but in the case of tearoffs for example it is
 * the base nsISupports pointer of the tearoff. This allow the cycle collector
 * to have separate nsCycleCollectionParticipant's for tearoffs or aggregated
 * classes.
 */
#define NS_CYCLECOLLECTIONISUPPORTS_IID                                        \
{                                                                              \
    0xc61eac14,                                                                \
    0x5f7a,                                                                    \
    0x4481,                                                                    \
    { 0x96, 0x5e, 0x7e, 0xaa, 0x6e, 0xff, 0xa8, 0x5f }                         \
}

/**
 * Just holds the IID so NS_GET_IID works.
 */
class NS_COM nsCycleCollectionISupports
{
public:
    NS_DECLARE_STATIC_IID_ACCESSOR(NS_CYCLECOLLECTIONISUPPORTS_IID)
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsCycleCollectionISupports, 
                              NS_CYCLECOLLECTIONISUPPORTS_IID)

#undef  IMETHOD_VISIBILITY
#define IMETHOD_VISIBILITY NS_VISIBILITY_DEFAULT

struct nsCycleCollectionParticipant;

struct nsCycleCollectionTraversalCallback
{
    // You must call DescribeNode() with an accurate refcount,
    // otherwise cycle collection will fail, and probably crash.
#ifdef DEBUG_CC
    virtual void DescribeNode(size_t refcount,
                              size_t objsz,
                              const char *objname) = 0;
#else
    virtual void DescribeNode(size_t refcount) = 0;
#endif
    virtual void NoteScriptChild(PRUint32 langID, void *child) = 0;
    virtual void NoteXPCOMChild(nsISupports *child) = 0;
    virtual void NoteNativeChild(void *child,
                                 nsCycleCollectionParticipant *helper) = 0;
};

class NS_NO_VTABLE nsCycleCollectionParticipant
{
public:
    NS_IMETHOD Traverse(void *p, nsCycleCollectionTraversalCallback &cb) = 0;

    NS_IMETHOD Root(void *p) = 0;
    NS_IMETHOD Unlink(void *p) = 0;
    NS_IMETHOD Unroot(void *p) = 0;
};

class NS_COM nsXPCOMCycleCollectionParticipant
    : public nsCycleCollectionParticipant
{
public:
    NS_DECLARE_STATIC_IID_ACCESSOR(NS_CYCLECOLLECTIONPARTICIPANT_IID)

    NS_IMETHOD Traverse(void *p, nsCycleCollectionTraversalCallback &cb);

    NS_IMETHOD Root(void *p);
    NS_IMETHOD Unlink(void *p);
    NS_IMETHOD Unroot(void *p);

    NS_IMETHOD_(void) UnmarkPurple(nsISupports *p);

#ifdef DEBUG
    NS_EXTERNAL_VIS_(PRBool) CheckForRightISupports(nsISupports *s);
#endif
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsXPCOMCycleCollectionParticipant, 
                              NS_CYCLECOLLECTIONPARTICIPANT_IID)

#undef  IMETHOD_VISIBILITY
#define IMETHOD_VISIBILITY NS_VISIBILITY_HIDDEN


///////////////////////////////////////////////////////////////////////////////
// Helpers for implementing a QI to nsXPCOMCycleCollectionParticipant
///////////////////////////////////////////////////////////////////////////////

#define NS_CYCLE_COLLECTION_INNERCLASS                                         \
        cycleCollection

#define NS_CYCLE_COLLECTION_CLASSNAME(_class)                                  \
        _class::NS_CYCLE_COLLECTION_INNERCLASS

#define NS_CYCLE_COLLECTION_NAME(_class)                                       \
        _class##_cycleCollectorGlobal

#define NS_IMPL_QUERY_CYCLE_COLLECTION(_class)                                 \
  if ( aIID.Equals(NS_GET_IID(nsXPCOMCycleCollectionParticipant)) ) {          \
    *aInstancePtr = & NS_CYCLE_COLLECTION_NAME(_class);                        \
    return NS_OK;                                                              \
  } else

#define NS_IMPL_QUERY_CYCLE_COLLECTION_ISUPPORTS(_class)                       \
  if ( aIID.Equals(NS_GET_IID(nsCycleCollectionISupports)) )                   \
    foundInterface = NS_CYCLE_COLLECTION_CLASSNAME(_class)::Upcast(this);      \
  else

#define NS_INTERFACE_MAP_ENTRY_CYCLE_COLLECTION(_class)                        \
  NS_IMPL_QUERY_CYCLE_COLLECTION(_class)

#define NS_INTERFACE_MAP_ENTRY_CYCLE_COLLECTION_ISUPPORTS(_class)              \
  NS_IMPL_QUERY_CYCLE_COLLECTION_ISUPPORTS(_class)

#define NS_INTERFACE_MAP_ENTRIES_CYCLE_COLLECTION(_class)                      \
  NS_INTERFACE_MAP_ENTRY_CYCLE_COLLECTION(_class)                              \
  NS_INTERFACE_MAP_ENTRY_CYCLE_COLLECTION_ISUPPORTS(_class)

#define NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(_class)                        \
  NS_INTERFACE_MAP_BEGIN(_class)                                               \
    NS_INTERFACE_MAP_ENTRY_CYCLE_COLLECTION(_class)                            \
    NS_INTERFACE_MAP_ENTRY_CYCLE_COLLECTION_ISUPPORTS(_class)

#define NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(_class)              \
  NS_INTERFACE_MAP_BEGIN(_class)                                               \
    NS_INTERFACE_MAP_ENTRY_CYCLE_COLLECTION(_class)

///////////////////////////////////////////////////////////////////////////////
// Helpers for implementing nsCycleCollectionParticipant::Unlink
///////////////////////////////////////////////////////////////////////////////

#define NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(_class)                          \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Unlink(void *p)                       \
  {                                                                            \
    nsISupports *s = NS_STATIC_CAST(nsISupports*, p);                          \
    NS_ASSERTION(CheckForRightISupports(s),                                    \
                 "not the nsISupports pointer we expect");                     \
    _class *tmp = Downcast(s);

#define NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN_INHERITED(_class, _base_class)   \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Unlink(void *p)                       \
  {                                                                            \
    nsISupports *s = NS_STATIC_CAST(nsISupports*, p);                          \
    NS_ASSERTION(CheckForRightISupports(s),                                    \
                 "not the nsISupports pointer we expect");                     \
    _class *tmp = NS_STATIC_CAST(_class*, Downcast(s));                        \
    NS_CYCLE_COLLECTION_CLASSNAME(_base_class)::Unlink(s);

#define NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN_NATIVE(_class)                   \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Unlink(void *p)                       \
  {                                                                            \
    _class *tmp = NS_STATIC_CAST(_class*, p);

#define NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(_field)                       \
    tmp->_field = NULL;    

#define NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMARRAY(_field)                     \
    tmp->_field.Clear();

#define NS_IMPL_CYCLE_COLLECTION_UNLINK_NSTARRAY(_field)                       \
    tmp->_field.Clear();

#define NS_IMPL_CYCLE_COLLECTION_UNLINK_END                                    \
    return NS_OK;                                                              \
  }

#define NS_IMPL_CYCLE_COLLECTION_UNLINK_0(_class)                              \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Unlink(void *p)                       \
  {                                                                            \
    NS_ASSERTION(CheckForRightISupports(NS_STATIC_CAST(nsISupports*, p)),      \
                 "not the nsISupports pointer we expect");                     \
    return NS_OK;                                                              \
  }

#define NS_IMPL_CYCLE_COLLECTION_UNLINK_NATIVE_0(_class)                       \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Unlink(void *p)                       \
  {                                                                            \
    return NS_OK;                                                              \
  }


///////////////////////////////////////////////////////////////////////////////
// Helpers for implementing nsCycleCollectionParticipant::Traverse
///////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_CC
#define NS_IMPL_CYCLE_COLLECTION_DESCRIBE(_class)                              \
    cb.DescribeNode(tmp->mRefCnt.get(), sizeof(_class), #_class);
#else
#define NS_IMPL_CYCLE_COLLECTION_DESCRIBE(_class)                              \
    cb.DescribeNode(tmp->mRefCnt.get());
#endif

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(_class)                        \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Traverse                              \
                         (void *p,                                             \
                          nsCycleCollectionTraversalCallback &cb)              \
  {                                                                            \
    nsISupports *s = NS_STATIC_CAST(nsISupports*, p);                          \
    NS_ASSERTION(CheckForRightISupports(s),                                    \
                 "not the nsISupports pointer we expect");                     \
    _class *tmp = Downcast(s);                                                 \
    NS_IMPL_CYCLE_COLLECTION_DESCRIBE(_class)

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN_INHERITED(_class, _base_class) \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Traverse                              \
                         (void *p,                                             \
                          nsCycleCollectionTraversalCallback &cb)              \
  {                                                                            \
    nsISupports *s = NS_STATIC_CAST(nsISupports*, p);                          \
    NS_ASSERTION(CheckForRightISupports(s),                                    \
                 "not the nsISupports pointer we expect");                     \
    _class *tmp = NS_STATIC_CAST(_class*, Downcast(s));                        \
    NS_CYCLE_COLLECTION_CLASSNAME(_base_class)::Traverse(s, cb);

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NATIVE_BEGIN(_class)                 \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Traverse                              \
                         (void *p,                                             \
                          nsCycleCollectionTraversalCallback &cb)              \
  {                                                                            \
    _class *tmp = NS_STATIC_CAST(_class*, p);                                  \
    NS_IMPL_CYCLE_COLLECTION_DESCRIBE(_class)

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_RAWPTR(_field)                       \
    cb.NoteXPCOMChild(tmp->_field);

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(_field)                     \
    cb.NoteXPCOMChild(tmp->_field.get());

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR_AMBIGUOUS(_field, _base)    \
    cb.NoteXPCOMChild(NS_ISUPPORTS_CAST(_base*, tmp->_field));

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMARRAY(_field)                   \
    {                                                                          \
      PRInt32 i;                                                               \
      for (i = 0; i < tmp->_field.Count(); ++i)                                \
        cb.NoteXPCOMChild(tmp->_field[i]);                                     \
    }

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NATIVE_PTR(_ptr, _ptr_class)         \
  cb.NoteNativeChild(_ptr, &NS_CYCLE_COLLECTION_NATIVE_NAME(_ptr_class));

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NATIVE_MEMBER(_field, _field_class)  \
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NATIVE_PTR(tmp->_field, _field_class)

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSTARRAY(_array, _element_class)     \
    {                                                                          \
      PRUint32 i, length = (_array).Length();                                  \
      for (i = 0; i < length; ++i)                                             \
        NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NATIVE_PTR((_array)[i],              \
                                                     _element_class);          \
    }

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSTARRAY_MEMBER(_field,              \
                                                          _element_class)      \
    NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSTARRAY(tmp->_field, _element_class)

#define NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END                                  \
    return NS_OK;                                                              \
  }

///////////////////////////////////////////////////////////////////////////////
// Helpers for implementing a concrete nsCycleCollectionParticipant 
///////////////////////////////////////////////////////////////////////////////

#define NS_DECL_CYCLE_COLLECTION_CLASS_AMBIGUOUS(_class, _base)                \
class NS_CYCLE_COLLECTION_INNERCLASS                                           \
 : public nsXPCOMCycleCollectionParticipant                                    \
{                                                                              \
public:                                                                        \
  NS_IMETHOD Unlink(void *p);                                                  \
  NS_IMETHOD Traverse(void *p,                                                 \
                      nsCycleCollectionTraversalCallback &cb);                 \
  NS_IMETHOD_(void) UnmarkPurple(nsISupports *s)                               \
  {                                                                            \
    Downcast(s)->UnmarkPurple();                                               \
  }                                                                            \
  static _class* Downcast(nsISupports* s)                                      \
  {                                                                            \
    return NS_STATIC_CAST(_class*, NS_STATIC_CAST(_base*, s));                 \
  }                                                                            \
  static nsISupports* Upcast(_class *p)                                        \
  {                                                                            \
    return NS_ISUPPORTS_CAST(_base*, p);                                       \
  }                                                                            \
};                                                           

#define NS_DECL_CYCLE_COLLECTION_CLASS(_class)                                 \
  NS_DECL_CYCLE_COLLECTION_CLASS_AMBIGUOUS(_class, _class)

#define NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(_class, _base_class)          \
class NS_CYCLE_COLLECTION_INNERCLASS                                           \
 : public NS_CYCLE_COLLECTION_CLASSNAME(_base_class)                           \
{                                                                              \
public:                                                                        \
  NS_IMETHOD Unlink(void *p);                                                  \
  NS_IMETHOD Traverse(void *p,                                                 \
                      nsCycleCollectionTraversalCallback &cb);                 \
  static _class* Downcast(nsISupports* s)                                      \
  {                                                                            \
    return NS_STATIC_CAST(_class*, NS_STATIC_CAST(_base_class*,                \
      NS_CYCLE_COLLECTION_CLASSNAME(_base_class)::Downcast(s)));               \
  }                                                                            \
};

#define NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED_NO_UNLINK(_class,             \
                                                           _base_class)        \
class NS_CYCLE_COLLECTION_INNERCLASS                                           \
 : public NS_CYCLE_COLLECTION_CLASSNAME(_base_class)                           \
{                                                                              \
public:                                                                        \
  NS_IMETHOD Traverse(void *p,                                                 \
                      nsCycleCollectionTraversalCallback &cb);                 \
  static _class* Downcast(nsISupports* s)                                      \
  {                                                                            \
    return NS_STATIC_CAST(_class*, NS_STATIC_CAST(_base_class*,                \
      NS_CYCLE_COLLECTION_CLASSNAME(_base_class)::Downcast(s)));               \
  }                                                                            \
};

/**
 * This implements a stub UnmarkPurple function for classes that want to be
 * traversed but whose AddRef/Release functions don't add/remove them to/from
 * the purple buffer. If you're just using NS_DECL_CYCLE_COLLECTING_ISUPPORTS
 * then you don't need this.
 */
#define NS_DECL_CYCLE_COLLECTION_UNMARK_PURPLE_STUB(_class)                    \
  NS_IMETHODIMP_(void) UnmarkPurple()                                          \
  {                                                                            \
  }                                                                            \

#define NS_IMPL_CYCLE_COLLECTION_CLASS(_class)                                 \
  static NS_CYCLE_COLLECTION_CLASSNAME(_class)                                 \
    NS_CYCLE_COLLECTION_NAME(_class);

#define NS_CYCLE_COLLECTION_NATIVE_INNERNAME                                   \
        _cycleCollectorGlobal

#define NS_CYCLE_COLLECTION_NATIVE_NAME(_class)                                \
        _class::NS_CYCLE_COLLECTION_NATIVE_INNERNAME

#define NS_DECL_CYCLE_COLLECTION_NATIVE_CLASS(_class)                          \
  class NS_CYCLE_COLLECTION_INNERCLASS                                         \
   : public nsCycleCollectionParticipant                                       \
  {                                                                            \
  public:                                                                      \
    NS_IMETHOD Root(void *n);                                                  \
    NS_IMETHOD Unlink(void *n);                                                \
    NS_IMETHOD Unroot(void *n);                                                \
    NS_IMETHOD Traverse(void *n,                                               \
                      nsCycleCollectionTraversalCallback &cb);                 \
  };                                                                           \
  static NS_CYCLE_COLLECTION_INNERCLASS                                        \
      NS_CYCLE_COLLECTION_NATIVE_INNERNAME;

#define NS_IMPL_CYCLE_COLLECTION_NATIVE_CLASS(_class)                          \
  NS_CYCLE_COLLECTION_CLASSNAME(_class) NS_CYCLE_COLLECTION_NATIVE_NAME(_class);

#define NS_IMPL_CYCLE_COLLECTION_ROOT_NATIVE(_class, _root_function)           \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Root(void *p)                         \
  {                                                                            \
    _class *tmp = NS_STATIC_CAST(_class*, p);                                  \
    tmp->_root_function();                                                     \
    return NS_OK;                                                              \
  }

#define NS_IMPL_CYCLE_COLLECTION_UNROOT_NATIVE(_class, _unroot_function)       \
  NS_IMETHODIMP                                                                \
  NS_CYCLE_COLLECTION_CLASSNAME(_class)::Unroot(void *p)                       \
  {                                                                            \
    _class *tmp = NS_STATIC_CAST(_class*, p);                                  \
    tmp->_unroot_function();                                                   \
    return NS_OK;                                                              \
  }

#define NS_IMPL_CYCLE_COLLECTION_0(_class)                                     \
 NS_IMPL_CYCLE_COLLECTION_CLASS(_class)                                        \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(_class)                                 \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_END                                           \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(_class)                               \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

#define NS_IMPL_CYCLE_COLLECTION_1(_class, _f)                                 \
 NS_IMPL_CYCLE_COLLECTION_CLASS(_class)                                        \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(_class)                                 \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(_f)                                  \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_END                                           \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(_class)                               \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(_f)                                \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

#define NS_IMPL_CYCLE_COLLECTION_2(_class, _f1, _f2)                           \
 NS_IMPL_CYCLE_COLLECTION_CLASS(_class)                                        \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(_class)                                 \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(_f1)                                 \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(_f2)                                 \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_END                                           \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(_class)                               \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(_f1)                               \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(_f2)                               \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

#define NS_IMPL_CYCLE_COLLECTION_3(_class, _f1, _f2, _f3)                      \
 NS_IMPL_CYCLE_COLLECTION_CLASS(_class)                                        \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(_class)                                 \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(_f1)                                 \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(_f2)                                 \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(_f3)                                 \
 NS_IMPL_CYCLE_COLLECTION_UNLINK_END                                           \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(_class)                               \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(_f1)                               \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(_f2)                               \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(_f3)                               \
 NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

#endif // nsCycleCollectionParticipant_h__
