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

#ifndef nsHTMLEditor_h__
#define nsHTMLEditor_h__

#include "nsITextEditor.h"
#include "nsTextEditor.h"
#include "nsIHTMLEditor.h"
#include "nsCOMPtr.h"
#include "nsIDOMEventListener.h"
#include "InsertTableTxn.h"

/**
 * The HTML editor implementation.<br>
 * Use to edit HTML document represented as a DOM tree. 
 */
class nsHTMLEditor  : public nsTextEditor, public nsIHTMLEditor
{

  typedef enum {eNoOp=0, eReplaceParent=1, eInsertParent=2} BlockTransformationType;

public:
  // see nsIHTMLEditor for documentation

//Interfaces for addref and release and queryinterface
//NOTE macro used is for classes that inherit from 
// another class. Only the base class should use NS_DECL_ISUPPORTS
  NS_DECL_ISUPPORTS_INHERITED

  nsHTMLEditor();
  virtual ~nsHTMLEditor();

//Initialization
  NS_IMETHOD Init(nsIDOMDocument *aDoc, nsIPresShell *aPresShell);

//============================================================================
// Methods that are duplicates of nsTextEditor -- exposed here for convenience

// Editing Operations
  NS_IMETHOD SetTextProperty(nsIAtom *aProperty, 
                             const nsString *aAttribute,
                             const nsString *aValue);
  NS_IMETHOD GetTextProperty(nsIAtom *aProperty, 
                             const nsString *aAttribute, const nsString *aValue,
                             PRBool &aFirst, PRBool &aAny, PRBool &aAll);
  NS_IMETHOD RemoveTextProperty(nsIAtom *aProperty, const nsString *aAttribute);
  NS_IMETHOD DeleteSelection(nsIEditor::ECollapsedSelectionAction aAction);
  NS_IMETHOD InsertText(const nsString& aStringToInsert);
  NS_IMETHOD InsertBreak();
  NS_IMETHOD CopyAttributes(nsIDOMNode *aDestNode, nsIDOMNode *aSourceNode);

// Transaction control
  NS_IMETHOD EnableUndo(PRBool aEnable);
  NS_IMETHOD Undo(PRUint32 aCount);
  NS_IMETHOD CanUndo(PRBool &aIsEnabled, PRBool &aCanUndo);
  NS_IMETHOD Redo(PRUint32 aCount);
  NS_IMETHOD CanRedo(PRBool &aIsEnabled, PRBool &aCanRedo);
  NS_IMETHOD BeginTransaction();
  NS_IMETHOD EndTransaction();

// Selection and navigation
  NS_IMETHOD MoveSelectionUp(nsIAtom *aIncrement, PRBool aExtendSelection);
  NS_IMETHOD MoveSelectionDown(nsIAtom *aIncrement, PRBool aExtendSelection);
  NS_IMETHOD MoveSelectionNext(nsIAtom *aIncrement, PRBool aExtendSelection);
  NS_IMETHOD MoveSelectionPrevious(nsIAtom *aIncrement, PRBool aExtendSelection);
  NS_IMETHOD SelectNext(nsIAtom *aIncrement, PRBool aExtendSelection); 
  NS_IMETHOD SelectPrevious(nsIAtom *aIncrement, PRBool aExtendSelection);
  NS_IMETHOD SelectAll();
  NS_IMETHOD ScrollUp(nsIAtom *aIncrement);
  NS_IMETHOD ScrollDown(nsIAtom *aIncrement);
  NS_IMETHOD ScrollIntoView(PRBool aScrollToBegin);

// file handling
  NS_IMETHOD Save();
  NS_IMETHOD SaveAs(PRBool aSavingCopy);

// cut, copy & paste
  NS_IMETHOD Cut();
  NS_IMETHOD Copy();
  NS_IMETHOD Paste();
  NS_IMETHOD PasteAsQuotation();
  NS_IMETHOD PasteAsCitedQuotation(nsString& aCitation);
  NS_IMETHOD InsertAsQuotation(const nsString& aQuotedText);
  NS_IMETHOD InsertAsCitedQuotation(const nsString& aQuotedText, nsString& aCitation);

// Input/Output
  NS_IMETHOD InsertHTML(const nsString& aInputString);

  NS_IMETHOD BeginComposition(void);
  NS_IMETHOD SetCompositionString(const nsString& aCompositionString);
  NS_IMETHOD EndComposition(void);
  NS_IMETHOD OutputText(nsString& aOutputString);
  NS_IMETHOD OutputHTML(nsString& aOutputString);
  NS_IMETHOD OutputText(nsIOutputStream* aOutputStream,nsString* aCharsetOverride = nsnull);
  NS_IMETHOD OutputHTML(nsIOutputStream* aOutputStream,nsString* aCharsetOverride = nsnull);

// End of methods implemented in nsEditor
//=============================================================
// HTML Editing methods
  NS_IMETHOD SetBackgroundColor(const nsString& aColor);
  NS_IMETHOD SetBodyAttribute(const nsString& aAttr, const nsString& aValue);
  NS_IMETHOD GetParagraphStyle(nsStringArray *aTagList);
  NS_IMETHOD AddBlockParent(nsString& aParentTag);
  NS_IMETHOD ReplaceBlockParent(nsString& aParentTag);
  NS_IMETHOD RemoveParagraphStyle();
  NS_IMETHOD RemoveParent(const nsString &aParentTag);

  NS_IMETHOD GetParagraphFormat(nsString& aParagraphFormat);
  NS_IMETHOD SetParagraphFormat(const nsString& aParagraphFormat);

  NS_IMETHOD Indent(const nsString& aIndent);
  NS_IMETHOD Align(const nsString& aAlign);
  NS_IMETHOD InsertLink(nsString& aURL);
  NS_IMETHOD InsertImage(nsString& aURL,
                         nsString& aWidth, nsString& aHeight,
                         nsString& aHspace, nsString& aVspace,
                         nsString& aBorder,
                         nsString& aAlt, nsString& aAlignment);
  NS_IMETHOD InsertList(const nsString& aListType);

  // This should replace InsertLink and InsertImage once it is working
  NS_IMETHOD GetSelectedElement(const nsString& aTagName, nsIDOMElement** aReturn);
  NS_IMETHOD CreateElementWithDefaults(const nsString& aTagName, nsIDOMElement** aReturn);
  NS_IMETHOD InsertElement(nsIDOMElement* aElement, PRBool aDeleteSelection, nsIDOMElement** aReturn);
  NS_IMETHOD InsertLinkAroundSelection(nsIDOMElement* aAnchorElement);
  PRBool     IsElementInBody(nsIDOMElement* aElement);
  NS_IMETHOD SelectElement(nsIDOMElement* aElement);
  NS_IMETHOD SetCaretAfterElement(nsIDOMElement* aElement);

// Table Editing (implemented in EditTable.cpp)
  NS_IMETHOD CreateTxnForInsertTable(const nsIDOMElement *aTableNode, InsertTableTxn ** aTxn);
  NS_IMETHOD GetColIndexForCell(nsIDOMNode *aCellNode, PRInt32 &aCellIndex);
  NS_IMETHOD GetRowIndexForCell(nsIDOMNode *aCellNode, PRInt32 &aCellIndex);
  NS_IMETHOD GetFirstCellInColumn(nsIDOMNode *aCurrentCellNode, nsIDOMNode* &aFirstCellNode);
  NS_IMETHOD GetNextCellInColumn(nsIDOMNode *aCurrentCellNode, nsIDOMNode* &aNextCellNode);
  NS_IMETHOD GetFirstCellInRow(nsIDOMNode *aCurrentCellNode, nsIDOMNode* &aCellNode);
  NS_IMETHOD GetNextCellInRow(nsIDOMNode *aCurrentCellNode, nsIDOMNode* &aNextCellNode);
  NS_IMETHOD InsertTable();
  NS_IMETHOD InsertTableCell(PRInt32 aNumber, PRBool aAfter);
  NS_IMETHOD InsertTableColumn(PRInt32 aNumber, PRBool aAfter);
  NS_IMETHOD InsertTableRow(PRInt32 aNumber, PRBool aAfter);
  NS_IMETHOD DeleteTable();
  NS_IMETHOD DeleteTableCell(PRInt32 aNumber);
  NS_IMETHOD DeleteTableColumn(PRInt32 aNumber);
  NS_IMETHOD DeleteTableRow(PRInt32 aNumber);
  NS_IMETHOD JoinTableCells(PRBool aCellToRight);

  NS_IMETHOD GetLocalFileURL(nsIDOMWindow* aParent, const nsString& aFilterType, nsString& aReturn);

// Data members

protected:

// rules initialization

  virtual void  InitRules();

  NS_IMETHOD ReParentContentOfNode(nsIDOMNode *aNode, 
                                   nsString   &aParentTag,
                                   BlockTransformationType aTranformation);

  NS_IMETHOD ReParentBlockContent(nsIDOMNode  *aNode, 
                                  nsString    &aParentTag,
                                  nsIDOMNode  *aBlockParentNode,
                                  nsString    &aBlockParentTag,
                                  BlockTransformationType aTranformation,
                                  nsIDOMNode **aNewParentNode);
  
  NS_IMETHOD ReParentContentOfRange(nsIDOMRange *aRange, 
                                    nsString    &aParentTag,
                                    BlockTransformationType aTranformation);

  NS_IMETHOD RemoveParagraphStyleFromRange(nsIDOMRange *aRange);
  
  NS_IMETHOD RemoveParagraphStyleFromBlockContent(nsIDOMRange *aRange);

  NS_IMETHOD RemoveParentFromRange(const nsString &aParentTag, nsIDOMRange *aRange);
  
  NS_IMETHOD RemoveParentFromBlockContent(const nsString &aParentTag, nsIDOMRange *aRange);

  PRBool     CanContainTag(nsIDOMNode* aParent, const nsString &aTag);
  
  NS_IMETHOD IsRootTag(nsString &aTag, PRBool &aIsTag);

  NS_IMETHOD IsSubordinateBlock(nsString &aTag, PRBool &aIsTag);


// EVENT LISTENERS AND COMMAND ROUTING NEEDS WORK
// For now, the listners are tied to the nsTextEditor class
//
//  nsCOMPtr<nsIDOMEventListener> mKeyListenerP;
//  nsCOMPtr<nsIDOMEventListener> mMouseListenerP;



};

#endif //nsHTMLEditor_h__

