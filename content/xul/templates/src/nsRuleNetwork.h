/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 2000 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Original Author(s):
 *   Chris Waterson <waterson@netscape.com>
 *
 * Contributor(s): 
 */

/*

  A rule discrimination network implementation based on ideas from
  RETE and TREAT.

  RETE is described in Charles Forgy, "Rete: A Fast Algorithm for the
  Many Patterns/Many Objects Match Problem", Artificial Intelligence
  19(1): pp. 17-37, 1982.

  TREAT is described in Daniel P. Miranker, "TREAT: A Better Match
  Algorithm for AI Production System Matching", AAAI 1987: pp. 42-47.

 */

#ifndef nsRuleNetwork_h__
#define nsRuleNetwork_h__

#include "nsCOMPtr.h"
#include "nsIContent.h"
#include "plhash.h"

class nsIRDFResource;
class nsIRDFNode;

//----------------------------------------------------------------------

/**
 * A type-safe value that can be bound to a variable in the rule
 * network.
 */
class Value {
public:
    enum Type { eUndefined, eISupports, eString };

protected:
    Type mType;

    union {
        nsISupports* mISupports;
        PRUnichar*   mString;
    };

    PRBool Equals(const Value& aValue) const;
    PRBool Equals(nsISupports* aISupports) const;
    PRBool Equals(const PRUnichar* aString) const;

    void Clear();

public:
    Value() : mType(eUndefined) {
        MOZ_COUNT_CTOR(Value); }

    Value(const Value& aValue);
    Value(nsISupports* aISupports);
    Value(const PRUnichar* aString);

    Value& operator=(const Value& aValue);
    Value& operator=(nsISupports* aISupports);
    Value& operator=(const PRUnichar* aString);

    ~Value();

    PRBool operator==(const Value& aValue) const { return Equals(aValue); }
    PRBool operator==(nsISupports* aISupports) const { return Equals(aISupports); }
    PRBool operator==(const PRUnichar* aString) const { return Equals(aString); }

    PRBool operator!=(const Value& aValue) const { return !Equals(aValue); }
    PRBool operator!=(nsISupports* aISupports) const { return !Equals(aISupports); }
    PRBool operator!=(const PRUnichar* aString) const { return !Equals(aString); }

    Type GetType() const { return mType; }

    operator nsISupports*() const;
    operator const PRUnichar*() const;

    PLHashNumber Hash() const;
};

//----------------------------------------------------------------------

/**
 * A set of variables
 */
class VariableSet
{
public:
    VariableSet();
    ~VariableSet();

    nsresult Add(PRInt32 aVariable);
    nsresult Remove(PRInt32 aVariable);
    PRBool Contains(PRInt32 aVariable);
    PRInt32 GetCount() { return mCount; }
    PRInt32 GetVariableAt(PRInt32 aIndex) { return mVariables[aIndex]; }

protected:
    PRInt32* mVariables;
    PRInt32 mCount;
    PRInt32 mCapacity;
};

//----------------------------------------------------------------------

/**
 * A memory element that supports an instantiation
 */
class MemoryElement {
public:
    MemoryElement() {}
    virtual ~MemoryElement() {}

    virtual const char* Type() const = 0;
    virtual PLHashNumber Hash() const = 0;
    virtual PRBool Equals(const MemoryElement& aElement) const = 0;
    virtual MemoryElement* Clone() const = 0;

    PRBool operator==(const MemoryElement& aMemoryElement) const {
        return Equals(aMemoryElement);
    }

    PRBool operator!=(const MemoryElement& aMemoryElement) const {
        return !Equals(aMemoryElement);
    }
};

//----------------------------------------------------------------------

/**
 * A collection of memory elements
 */
class MemoryElementSet {
public:
    class ConstIterator;
    friend class ConstIterator;

protected:
    class List {
    public:
        List() { MOZ_COUNT_CTOR(MemoryElementSet::List); }

        ~List() {
            MOZ_COUNT_DTOR(MemoryElementSet::List);
            delete mElement;
            NS_IF_RELEASE(mNext); }

        PRInt32 AddRef() { return ++mRefCnt; }

        PRInt32 Release() {
            PRInt32 refcnt = --mRefCnt;
            if (refcnt == 0) delete this;
            return refcnt; }

        MemoryElement* mElement;
        PRInt32        mRefCnt;
        List*          mNext;
    };

    List* mElements;

public:
    MemoryElementSet() : mElements(nsnull) {
        MOZ_COUNT_CTOR(MemoryElementSet); }

    MemoryElementSet(const MemoryElementSet& aSet) : mElements(aSet.mElements) {
        MOZ_COUNT_CTOR(MemoryElementSet);
        NS_IF_ADDREF(mElements); }

    MemoryElementSet& operator=(const MemoryElementSet& aSet) {
        NS_IF_RELEASE(mElements);
        mElements = aSet.mElements;
        NS_IF_ADDREF(mElements);
        return *this; }
        
    ~MemoryElementSet() {
        MOZ_COUNT_DTOR(MemoryElementSet);
        NS_IF_RELEASE(mElements); }

public:
    class ConstIterator {
    public:
        ConstIterator(List* aElementList) : mCurrent(aElementList) {
            NS_IF_ADDREF(mCurrent); }

        ConstIterator(const ConstIterator& aConstIterator)
            : mCurrent(aConstIterator.mCurrent) {
            NS_IF_ADDREF(mCurrent); }

        ConstIterator& operator=(const ConstIterator& aConstIterator) {
            NS_IF_RELEASE(mCurrent);
            mCurrent = aConstIterator.mCurrent;
            NS_IF_ADDREF(mCurrent);
            return *this; }

        ~ConstIterator() { NS_IF_RELEASE(mCurrent); }

        ConstIterator& operator++() {
            List* next = mCurrent->mNext;
            NS_RELEASE(mCurrent);
            mCurrent = next;
            NS_IF_ADDREF(mCurrent);
            return *this; }

        ConstIterator operator++(int) {
            ConstIterator result(*this);
            List* next = mCurrent->mNext;
            NS_RELEASE(mCurrent);
            mCurrent = next;
            NS_IF_ADDREF(mCurrent);
            return result; }

        const MemoryElement& operator*() const {
            return *mCurrent->mElement; }

        const MemoryElement* operator->() const {
            return mCurrent->mElement; }

        PRBool operator==(const ConstIterator& aConstIterator) const {
            return mCurrent == aConstIterator.mCurrent; }

        PRBool operator!=(const ConstIterator& aConstIterator) const {
            return mCurrent != aConstIterator.mCurrent; }

    protected:
        List* mCurrent;
    };

    ConstIterator First() const { return ConstIterator(mElements); }
    ConstIterator Last() const { return ConstIterator(nsnull); }

    // N.B. that the set assumes ownership of the element
    nsresult Add(MemoryElement* aElement);
};

//----------------------------------------------------------------------

/**
 * An assignment of a value to a variable
 */
class nsBinding {
public:
    PRInt32 mVariable;
    Value   mValue;

    nsBinding() : mVariable(-1), mValue()
        { MOZ_COUNT_CTOR(nsBinding); }

    nsBinding(PRInt32 aVariable, const Value& aValue)
        : mVariable(aVariable),
          mValue(aValue)
        { MOZ_COUNT_CTOR(nsBinding); }

    nsBinding(const nsBinding& aBinding)
        : mVariable(aBinding.mVariable),
          mValue(aBinding.mValue)
        { MOZ_COUNT_CTOR(nsBinding); }

    ~nsBinding() { MOZ_COUNT_DTOR(nsBinding); }

    nsBinding& operator=(const nsBinding& aBinding) {
        mVariable = aBinding.mVariable;
        mValue    = aBinding.mValue;
        return *this; }

    PRBool operator==(const nsBinding& aBinding) const {
        return mVariable == aBinding.mVariable && mValue == aBinding.mValue; }

    PRBool operator!=(const nsBinding& aBinding) const {
        return mVariable != aBinding.mVariable || mValue != aBinding.mValue; }

    PLHashNumber Hash() const {
        // XXX I have no idea if this hashing function is good or not
        return (mValue.Hash() & 0xffff) | (mVariable << 16); }
};


//----------------------------------------------------------------------

/**
 * A collection of value-to-variable assignments
 */
class nsBindingSet {
public:
    class ConstIterator;
    friend class ConstIterator;

protected:
    class List {
    public:
        List() { MOZ_COUNT_CTOR(nsBindingSet::List); }

        ~List() {
            MOZ_COUNT_DTOR(nsBindingSet::List);
            NS_IF_RELEASE(mNext); }

        PRInt32 AddRef() { return ++mRefCnt; }

        PRInt32 Release() {
            PRInt32 refcnt = --mRefCnt;
            if (refcnt == 0) delete this;
            return refcnt; }

        nsBinding mBinding;
        PRInt32 mRefCnt;
        List*   mNext;
    };

    List* mBindings;

public:
    nsBindingSet() : mBindings(nsnull) {
        MOZ_COUNT_CTOR(nsBindingSet); }

    nsBindingSet(const nsBindingSet& aSet) : mBindings(aSet.mBindings) {
        MOZ_COUNT_CTOR(nsBindingSet);
        NS_IF_ADDREF(mBindings); }

    nsBindingSet& operator=(const nsBindingSet& aSet) {
        NS_IF_RELEASE(mBindings);
        mBindings = aSet.mBindings;
        NS_IF_ADDREF(mBindings);
        return *this; }
        
    ~nsBindingSet() {
        MOZ_COUNT_DTOR(nsBindingSet);
        NS_IF_RELEASE(mBindings); }

public:
    class ConstIterator {
    public:
        ConstIterator(List* aBindingList) : mCurrent(aBindingList) {
            NS_IF_ADDREF(mCurrent); }

        ConstIterator(const ConstIterator& aConstIterator)
            : mCurrent(aConstIterator.mCurrent) {
            NS_IF_ADDREF(mCurrent); }

        ConstIterator& operator=(const ConstIterator& aConstIterator) {
            NS_IF_RELEASE(mCurrent);
            mCurrent = aConstIterator.mCurrent;
            NS_IF_ADDREF(mCurrent);
            return *this; }

        ~ConstIterator() { NS_IF_RELEASE(mCurrent); }

        ConstIterator& operator++() {
            List* next = mCurrent->mNext;
            NS_RELEASE(mCurrent);
            mCurrent = next;
            NS_IF_ADDREF(mCurrent);
            return *this; }

        ConstIterator operator++(int) {
            ConstIterator result(*this);
            List* next = mCurrent->mNext;
            NS_RELEASE(mCurrent);
            mCurrent = next;
            NS_IF_ADDREF(mCurrent);
            return result; }

        const nsBinding& operator*() const {
            return mCurrent->mBinding; }

        const nsBinding* operator->() const {
            return &mCurrent->mBinding; }

        PRBool operator==(const ConstIterator& aConstIterator) const {
            return mCurrent == aConstIterator.mCurrent; }

        PRBool operator!=(const ConstIterator& aConstIterator) const {
            return mCurrent != aConstIterator.mCurrent; }

    protected:
        List* mCurrent;
    };

    ConstIterator First() const { return ConstIterator(mBindings); }
    ConstIterator Last() const { return ConstIterator(nsnull); }

public:
    nsresult Add(const nsBinding& aElement);

    PRBool HasBinding(PRInt32 aVariable, const Value& aValue) const;
    PRBool HasBindingFor(PRInt32 aVariable) const;
    PRBool GetBindingFor(PRInt32 aVariable, Value* aValue) const;

    PRInt32 Count() const;

    PRBool Equals(const nsBindingSet& aSet) const;
    PRBool operator==(const nsBindingSet& aSet) const { return Equals(aSet); }
    PRBool operator!=(const nsBindingSet& aSet) const { return !Equals(aSet); }
};


//----------------------------------------------------------------------

/**
 * A set of bindings with associated memory element support.
 */
class Instantiation
{
public:
    nsBindingSet       mBindings;
    MemoryElementSet mSupport;

    Instantiation() { MOZ_COUNT_CTOR(Instantiation); }

    Instantiation(const Instantiation& aInstantiation)
        : mBindings(aInstantiation.mBindings),
          mSupport(aInstantiation.mSupport) {
        MOZ_COUNT_CTOR(Instantiation); }

    Instantiation& operator=(const Instantiation& aInstantiation) {
        mBindings = aInstantiation.mBindings;
        mSupport  = aInstantiation.mSupport;
        return *this; }

    ~Instantiation() { MOZ_COUNT_DTOR(Instantiation); }

    nsresult AddBinding(PRInt32 aVariable, const Value& aValue) {
        mBindings.Add(nsBinding(aVariable, aValue));
        return NS_OK; }

    nsresult AddSupportingElement(MemoryElement* aMemoryElement) {
        mSupport.Add(aMemoryElement);
        return NS_OK; }

    PRBool Equals(const Instantiation& aInstantiation) const {
        return mBindings == aInstantiation.mBindings; }

    PRBool operator==(const Instantiation& aInstantiation) const {
        return Equals(aInstantiation); }

    PRBool operator!=(const Instantiation& aInstantiation) const {
        return !Equals(aInstantiation); }

    static PLHashNumber Hash(const void* aKey);
    static PRIntn Compare(const void* aLeft, const void* aRight);
};


//----------------------------------------------------------------------

/**
 * A collection of intantiations
 */
class InstantiationSet
{
public:
    InstantiationSet();
    InstantiationSet(const InstantiationSet& aInstantiationSet);
    InstantiationSet& operator=(const InstantiationSet& aInstantiationSet);

    ~InstantiationSet() {
        MOZ_COUNT_DTOR(InstantiationSet);
        Clear(); }

    class ConstIterator;
    friend class ConstIterator;

    class Iterator;
    friend class Iterator;

protected:
    class List {
    public:
        Instantiation mInstantiation;
        List*         mNext;
        List*         mPrev;

        List() { MOZ_COUNT_CTOR(InstantiationSet::List); }
        ~List() { MOZ_COUNT_DTOR(InstantiationSet::List); }
    };

    List mHead;

public:
    class ConstIterator {
    protected:
        friend class Iterator; // XXXwaterson so broken.
        List* mCurrent;

    public:
        ConstIterator(List* aList) : mCurrent(aList) {}

        ConstIterator(const ConstIterator& aConstIterator)
            : mCurrent(aConstIterator.mCurrent) {}

        ConstIterator& operator=(const ConstIterator& aConstIterator) {
            mCurrent = aConstIterator.mCurrent;
            return *this; }

        ConstIterator& operator++() {
            mCurrent = mCurrent->mNext;
            return *this; }

        ConstIterator operator++(int) {
            ConstIterator result(*this);
            mCurrent = mCurrent->mNext;
            return result; }

        ConstIterator& operator--() {
            mCurrent = mCurrent->mPrev;
            return *this; }

        ConstIterator operator--(int) {
            ConstIterator result(*this);
            mCurrent = mCurrent->mPrev;
            return result; }

        const Instantiation& operator*() const {
            return mCurrent->mInstantiation; }

        const Instantiation* operator->() const {
            return &mCurrent->mInstantiation; }

        PRBool operator==(const ConstIterator& aConstIterator) const {
            return mCurrent == aConstIterator.mCurrent; }

        PRBool operator!=(const ConstIterator& aConstIterator) const {
            return mCurrent != aConstIterator.mCurrent; }
    };

    ConstIterator First() const { return ConstIterator(mHead.mNext); }
    ConstIterator Last() const { return ConstIterator(NS_CONST_CAST(List*, &mHead)); }

    class Iterator : public ConstIterator {
    public:
        Iterator(List* aList) : ConstIterator(aList) {}

        Iterator& operator++() {
            mCurrent = mCurrent->mNext;
            return *this; }

        Iterator operator++(int) {
            Iterator result(*this);
            mCurrent = mCurrent->mNext;
            return result; }

        Iterator& operator--() {
            mCurrent = mCurrent->mPrev;
            return *this; }

        Iterator operator--(int) {
            Iterator result(*this);
            mCurrent = mCurrent->mPrev;
            return result; }

        Instantiation& operator*() const {
            return mCurrent->mInstantiation; }

        Instantiation* operator->() const {
            return &mCurrent->mInstantiation; }

        PRBool operator==(const ConstIterator& aConstIterator) const {
            return mCurrent == aConstIterator.mCurrent; }

        PRBool operator!=(const ConstIterator& aConstIterator) const {
            return mCurrent != aConstIterator.mCurrent; }

        friend class InstantiationSet;
    };

    Iterator First() { return Iterator(mHead.mNext); }
    Iterator Last() { return Iterator(&mHead); }

    PRBool Empty() const { return First() == Last(); }

    Iterator Append(const Instantiation& aInstantiation) {
        return Insert(Last(), aInstantiation); }

    Iterator Insert(Iterator aBefore, const Instantiation& aInstantiation);

    Iterator Erase(Iterator aElement);

    void Clear();

    PRBool HasBindingFor(PRInt32 aVariable) const;

};

//----------------------------------------------------------------------

/**
 * A abstract base class for all nodes in the rule network
 */
class ReteNode
{
public:
    ReteNode() {}
    virtual ~ReteNode() {}

    // "downward" propogations
    virtual nsresult Propogate(const InstantiationSet& aInstantiations, void* aClosure) = 0;
};

//----------------------------------------------------------------------

/**
 * A collection of nodes in the rule network
 */
class NodeSet
{
public:
    NodeSet();
    ~NodeSet();

    nsresult Add(ReteNode* aNode);
    nsresult Clear();

    class Iterator;

    class ConstIterator {
    public:
        ConstIterator(ReteNode** aNode) : mCurrent(aNode) {}

        ConstIterator(const ConstIterator& aConstIterator)
            : mCurrent(aConstIterator.mCurrent) {}

        ConstIterator& operator=(const ConstIterator& aConstIterator) {
            mCurrent = aConstIterator.mCurrent;
            return *this; }

        ConstIterator& operator++() {
            ++mCurrent;
            return *this; }

        ConstIterator operator++(int) {
            ConstIterator result(*this);
            ++mCurrent;
            return result; }

        const ReteNode* operator*() const {
            return *mCurrent; }

        const ReteNode* operator->() const {
            return *mCurrent; }

        PRBool operator==(const ConstIterator& aConstIterator) const {
            return mCurrent == aConstIterator.mCurrent; }

        PRBool operator!=(const ConstIterator& aConstIterator) const {
            return mCurrent != aConstIterator.mCurrent; }

    protected:
        friend class Iterator; // XXXwaterson this is so wrong!
        ReteNode** mCurrent;
    };

    ConstIterator First() const { return ConstIterator(mNodes); }
    ConstIterator Last() const { return ConstIterator(mNodes + mCount); }

    class Iterator : public ConstIterator {
    public:
        Iterator(ReteNode** aNode) : ConstIterator(aNode) {}

        Iterator& operator++() {
            ++mCurrent;
            return *this; }

        Iterator operator++(int) {
            Iterator result(*this);
            ++mCurrent;
            return result; }

        ReteNode* operator*() const {
            return *mCurrent; }

        ReteNode* operator->() const {
            return *mCurrent; }

        PRBool operator==(const ConstIterator& aConstIterator) const {
            return mCurrent == aConstIterator.mCurrent; }

        PRBool operator!=(const ConstIterator& aConstIterator) const {
            return mCurrent != aConstIterator.mCurrent; }
    };

    Iterator First() { return Iterator(mNodes); }
    Iterator Last() { return Iterator(mNodes + mCount); }

protected:
    ReteNode** mNodes;
    PRInt32 mCount;
    PRInt32 mCapacity;
};

//----------------------------------------------------------------------

/**
 * An abstract base class for an "inner node" in the rule
 * network. Adds support for children and "upward" queries.
 */
class InnerNode : public ReteNode
{
public:
    // "upward" propogations
    virtual nsresult Constrain(InstantiationSet& aInstantiations) = 0;

    virtual nsresult GetAncestorVariables(VariableSet& aVariables) const = 0;

    virtual PRBool HasAncestor(const ReteNode* aNode) const = 0;

    nsresult AddChild(ReteNode* aNode) { return mKids.Add(aNode); }
    nsresult RemoveAllChildren() { return mKids.Clear(); }

protected:
    NodeSet mKids;
};

//----------------------------------------------------------------------

/**
 * The root node in the rule network.
 */
class RootNode : public InnerNode
{
public:
    // "downward" propogations
    virtual nsresult Propogate(const InstantiationSet& aInstantiations, void* aClosure);

    // "upward" propogations
    virtual nsresult Constrain(InstantiationSet& aInstantiations);

    virtual nsresult GetAncestorVariables(VariableSet& aVariables) const;

    virtual PRBool HasAncestor(const ReteNode* aNode) const;
};

//----------------------------------------------------------------------

/**
 * A node that joins to paths from the root node, and binds a
 * variable from the left ancestor to a variable in the right
 * ancestor.
 */
class JoinNode : public InnerNode
{
public:
    enum Operator { eEquality };

    JoinNode(InnerNode* aLeftParent,
             PRInt32 aLeftVariable,
             InnerNode* aRightParent,
             PRInt32 aRightVariable,
             Operator aOperator);

    // "downward" propogations
    virtual nsresult Propogate(const InstantiationSet& aInstantiations, void* aClosure);

    // "upward" propogations
    virtual nsresult Constrain(InstantiationSet& aInstantiations);

    virtual nsresult GetAncestorVariables(VariableSet& aVariables) const;

    virtual PRBool HasAncestor(const ReteNode* aNode) const;

protected:
    InnerNode* mLeftParent;
    PRInt32 mLeftVariable;
    InnerNode* mRightParent;
    PRInt32 mRightVariable;
    Operator mOperator;

    static nsresult GetNumBound(InnerNode* aAncestor, const InstantiationSet& aInstantiations, PRInt32* aBoundCount);

    nsresult Bind(InstantiationSet& aInstantiations, PRBool* aDidBind);
};


//----------------------------------------------------------------------

/**
 * A node that applies a test condition to a set of instantiations.
 */
class TestNode : public InnerNode
{
public:
    TestNode(InnerNode* aParent);

    InnerNode* GetParent() const { return mParent; }

    // "downward" propogations
    virtual nsresult Propogate(const InstantiationSet& aInstantiations, void* aClosure);

    // "upward" propogations
    virtual nsresult Constrain(InstantiationSet& aInstantiations);

    // instantiation filtering
    virtual nsresult FilterInstantiations(InstantiationSet& aInstantiations) const = 0; //XXX probably better named "ApplyConstraints" or "Discrminiate" or something

    virtual nsresult GetAncestorVariables(VariableSet& aVariables) const;

    virtual PRBool HasAncestor(const ReteNode* aNode) const;

protected:
    InnerNode* mParent;
};

//----------------------------------------------------------------------

class nsRuleNetwork
{
public:
    nsRuleNetwork();
    ~nsRuleNetwork();

    nsresult Clear();
    nsresult AddNode(ReteNode* aNode) { return mNodes.Add(aNode); }

    nsresult CreateVariable(PRInt32* aVariable);

    RootNode* GetRoot() { return &mRoot; };

protected:
    NodeSet mNodes;
    PRInt32 mNextVariable;

    RootNode mRoot;
};



#endif // nsRuleNetwork_h__
