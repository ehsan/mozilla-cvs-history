/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 */

#include "nsCOMPtr.h"
#include "nsTreeFrame.h"
#include "nsIStyleContext.h"
#include "nsIContent.h"
#include "nsCSSRendering.h"
#include "nsTreeCellFrame.h"
#include "nsCellMap.h"
#include "nsIDOMXULTreeElement.h"
#include "nsINameSpaceManager.h"
#include "nsTreeRowGroupFrame.h"
#include "nsXULAtoms.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMXULTreeElement.h"
//
// NS_NewTreeFrame
//
// Creates a new tree frame
//
nsresult
NS_NewTreeFrame (nsIFrame** aNewFrame)
{
  NS_PRECONDITION(aNewFrame, "null OUT ptr");
  if (nsnull == aNewFrame) {
    return NS_ERROR_NULL_POINTER;
  }
  nsTreeFrame* it = new nsTreeFrame;
  if (!it)
    return NS_ERROR_OUT_OF_MEMORY;

  *aNewFrame = it;
  return NS_OK;
  
} // NS_NewTreeFrame


// Constructor
nsTreeFrame::nsTreeFrame()
:nsTableFrame(),mSlatedForReflow(PR_FALSE) { }

// Destructor
nsTreeFrame::~nsTreeFrame()
{
}

void nsTreeFrame::SetSelection(nsIPresContext& aPresContext, nsTreeCellFrame* aFrame)
{
  nsCOMPtr<nsIContent> cellContent;
  aFrame->GetContent(getter_AddRefs(cellContent));

  nsCOMPtr<nsIContent> rowContent;
  cellContent->GetParent(*getter_AddRefs(rowContent));

  nsCOMPtr<nsIContent> itemContent;
  rowContent->GetParent(*getter_AddRefs(itemContent));

  nsCOMPtr<nsIDOMXULTreeElement> treeElement = do_QueryInterface(mContent);
  nsCOMPtr<nsIDOMXULElement> cellElement = do_QueryInterface(cellContent);
  nsCOMPtr<nsIDOMXULElement> itemElement = do_QueryInterface(itemContent);

  nsCOMPtr<nsIAtom> kSuppressSelectChange = dont_AddRef(NS_NewAtom("suppressonselect"));
  mContent->SetAttribute(kNameSpaceID_None, kSuppressSelectChange, "true", PR_FALSE);
  treeElement->SelectItem(itemElement);
  mContent->UnsetAttribute(kNameSpaceID_None, kSuppressSelectChange, PR_FALSE);
  treeElement->SelectCell(cellElement);
}

void nsTreeFrame::ToggleSelection(nsIPresContext& aPresContext, nsTreeCellFrame* aFrame)
{
	nsCOMPtr<nsIContent> cellContent;
  aFrame->GetContent(getter_AddRefs(cellContent));

  nsCOMPtr<nsIContent> rowContent;
  cellContent->GetParent(*getter_AddRefs(rowContent));

  nsCOMPtr<nsIContent> itemContent;
  rowContent->GetParent(*getter_AddRefs(itemContent));

  nsCOMPtr<nsIDOMXULTreeElement> treeElement = do_QueryInterface(mContent);
  nsCOMPtr<nsIDOMXULElement> cellElement = do_QueryInterface(cellContent);
  nsCOMPtr<nsIDOMXULElement> itemElement = do_QueryInterface(itemContent);

  nsCOMPtr<nsIAtom> kSuppressSelectChange = dont_AddRef(NS_NewAtom("suppressonselect"));
  mContent->SetAttribute(kNameSpaceID_None, kSuppressSelectChange, "true", PR_FALSE);
  treeElement->ToggleItemSelection(itemElement);
  mContent->UnsetAttribute(kNameSpaceID_None, kSuppressSelectChange, PR_FALSE);
  treeElement->ToggleCellSelection(cellElement);
}

void nsTreeFrame::RangedSelection(nsIPresContext& aPresContext, nsTreeCellFrame* pEndFrame)
{
 // XXX Re-implement!
}

void
nsTreeFrame::GetTreeBody(nsTreeRowGroupFrame** aResult)
{
  nsIFrame* curr = mFrames.FirstChild();
  while (curr) {
    nsCOMPtr<nsIContent> content;
    curr->GetContent(getter_AddRefs(content));
    if (content) {
      nsCOMPtr<nsIAtom> tag;
      content->GetTag(*getter_AddRefs(tag));
      if (tag && tag.get() == nsXULAtoms::treechildren) {
        // This is our actual treechildren frame.
        nsTreeRowGroupFrame* rowGroup = (nsTreeRowGroupFrame*)curr; // XXX I am evil.
        *aResult = rowGroup;
        return;
      }
    }

    curr->GetNextSibling(&curr);
  }
}

NS_IMETHODIMP 
nsTreeFrame::HandleEvent(nsIPresContext& aPresContext, 
                             nsGUIEvent*     aEvent,
                             nsEventStatus&  aEventStatus)
{
  aEventStatus = nsEventStatus_eConsumeDoDefault;
  if (aEvent->message == NS_KEY_DOWN) {
    nsKeyEvent* keyEvent = (nsKeyEvent*)aEvent;
    PRUint32 keyCode = keyEvent->keyCode;
    if (keyCode == NS_VK_UP ||
        keyCode == NS_VK_DOWN ||
        keyCode == NS_VK_LEFT ||
        keyCode == NS_VK_RIGHT ||
        keyCode == NS_VK_ENTER) {

      // Get our treechildren child frame.
      nsTreeRowGroupFrame* treeRowGroup = nsnull;
      GetTreeBody(&treeRowGroup);

      if (!treeRowGroup)
        return NS_OK; // No tree body. Just bail.

      nsCOMPtr<nsIDOMXULTreeElement> treeElement = do_QueryInterface(mContent);
      nsCOMPtr<nsIDOMNodeList> itemNodeList;
      nsCOMPtr<nsIDOMNodeList> cellNodeList;
      treeElement->GetSelectedItems(getter_AddRefs(itemNodeList));
      treeElement->GetSelectedCells(getter_AddRefs(cellNodeList));
      PRUint32 itemLength;
      PRUint32 cellLength;
      itemNodeList->GetLength(&itemLength);
      cellNodeList->GetLength(&cellLength);

      PRInt32 rowIndex = -1;
      PRInt32 cellIndex = 0;

      if (cellLength != 0 && itemLength == 0) {
        nsCOMPtr<nsIDOMNode> node;
        cellNodeList->Item(0, getter_AddRefs(node));
        nsCOMPtr<nsIContent> content = do_QueryInterface(node);
        treeRowGroup->IndexOfCell(aPresContext, content, rowIndex, cellIndex);
      }
      else if (cellLength == 0 && itemLength != 0) {
        nsCOMPtr<nsIDOMNode> node;
        itemNodeList->Item(0, getter_AddRefs(node));
        nsCOMPtr<nsIContent> content = do_QueryInterface(node);
        treeRowGroup->IndexOfRow(aPresContext, content, rowIndex);
      }
      else if (cellLength != 0 && itemLength != 0) {
        nsCOMPtr<nsIDOMNode> node;
        cellNodeList->Item(0, getter_AddRefs(node));
        nsCOMPtr<nsIContent> content = do_QueryInterface(node);
        treeRowGroup->IndexOfCell(aPresContext, content, rowIndex, cellIndex);
      }
      
      // We now have a valid row and cell index for the current selection.  Based on the
      // direction, let's adjust the row and column index.
      if (rowIndex == -1)
        rowIndex = 0;
      else if (keyCode == NS_VK_DOWN)
        rowIndex++;
      else if (keyCode == NS_VK_UP)
        rowIndex--;
      
      if (!treeRowGroup->IsValidRow(rowIndex))
        return NS_OK;

      // Ensure that the required index is visible.
      treeRowGroup->EnsureRowIsVisible(rowIndex);

      // Now that the row is scrolled into view, we have a frame created. We can retrieve the cell.
      nsTreeCellFrame* cellFrame;
      treeRowGroup->GetCellFrameAtIndex(rowIndex, cellIndex, &cellFrame);
      if (!cellFrame)
        return NS_OK; // No cell. Whatever. Bail.

      // We got it! Perform the selection on an up/down.
      if (keyCode == NS_VK_UP || keyCode == NS_VK_DOWN)
        SetSelection(aPresContext, cellFrame);
      else if (keyCode == NS_VK_ENTER || keyCode == NS_VK_RETURN)
        cellFrame->ToggleOpenClose();
      else if (keyCode == NS_VK_LEFT)
        cellFrame->Close();
      else if (keyCode == NS_VK_RIGHT)
        cellFrame->Open();
    }
  }
  return NS_OK;
}
  
void nsTreeFrame::MoveUp(nsIPresContext& aPresContext, nsTreeCellFrame* pFrame)
{
	PRInt32 rowIndex;
  pFrame->GetRowIndex(rowIndex);
	PRInt32 colIndex;
  pFrame->GetColIndex(colIndex);
	if (rowIndex > 0)
	{
		MoveToRowCol(aPresContext, rowIndex-1, colIndex);
	}
}

void nsTreeFrame::MoveDown(nsIPresContext& aPresContext, nsTreeCellFrame* pFrame)
{
	PRInt32 rowIndex;
  pFrame->GetRowIndex(rowIndex);
	PRInt32 colIndex;
  pFrame->GetColIndex(colIndex);
	PRInt32 totalRows = mCellMap->GetRowCount();

	if (rowIndex < totalRows-1)
	{
		MoveToRowCol(aPresContext, rowIndex+1, colIndex);
	}
}

void nsTreeFrame::MoveLeft(nsIPresContext& aPresContext, nsTreeCellFrame* pFrame)
{
	PRInt32 rowIndex;
  pFrame->GetRowIndex(rowIndex);
	PRInt32 colIndex;
  pFrame->GetColIndex(colIndex);
	if (colIndex > 0)
	{
		MoveToRowCol(aPresContext, rowIndex, colIndex-1);
	}
}

void nsTreeFrame::MoveRight(nsIPresContext& aPresContext, nsTreeCellFrame* aFrame)
{
	PRInt32 rowIndex;
  aFrame->GetRowIndex(rowIndex);
	PRInt32 colIndex;
  aFrame->GetColIndex(colIndex);
	PRInt32 totalCols = mCellMap->GetColCount();

	if (colIndex < totalCols-1)
	{
		MoveToRowCol(aPresContext, rowIndex, colIndex+1);
	}
}

void nsTreeFrame::MoveToRowCol(nsIPresContext& aPresContext, PRInt32 aRow, PRInt32 aCol)
{
	nsTableCellFrame* cellFrame = mCellMap->GetCellInfoAt(aRow, aCol);

	// We now have the cell that should be selected. 
	nsTreeCellFrame* treeCell = NS_STATIC_CAST(nsTreeCellFrame*, cellFrame);
	SetSelection(aPresContext, treeCell);
}

NS_IMETHODIMP 
nsTreeFrame::Destroy(nsIPresContext& aPresContext)
{
  return nsTableFrame::Destroy(aPresContext);
}

NS_IMETHODIMP
nsTreeFrame::Reflow(nsIPresContext&          aPresContext,
							      nsHTMLReflowMetrics&     aMetrics,
							      const nsHTMLReflowState& aReflowState,
							      nsReflowStatus&          aStatus)
{
  mSlatedForReflow = PR_FALSE;
  return nsTableFrame::Reflow(aPresContext, aMetrics, aReflowState, aStatus);
}
