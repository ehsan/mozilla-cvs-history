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
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

// edview2.cpp : 2nd implementation file of the CNetscapeEditView class
//
//

#include "stdafx.h"
#ifdef EDITOR
#include "edview.h"
#include "edprops.h"
#include "edt.h"
#include "mainfrm.h"
#include "edframe.h"
#include "edres2.h"
#include "property.h"
#include "edttypes.h"
#include "edtable.h"
#include "prefapi.h"
#include "genframe.h"
#include "intl_csi.h"
#include "abdefn.h"
#include "spellcli.h"


#ifdef _IME_COMPOSITION
#define CLEARBIT(A, N)	A&=~N
#define SETBIT(A, N)	A|=N

    #ifdef XP_WIN16
        #include "ime16.h"
    #else
        #include "intlwin.h"
    #endif //XP_WIN16 else XP_WIN32
#endif



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#ifdef XP_WIN32
#include "shlobj.h"
#endif

#ifdef _IME_COMPOSITION
#define CLEARBIT(A, N)	A&=~N
#define SETBIT(A, N)	A|=N
#endif

extern char *EDT_NEW_DOC_NAME;

// editor plugin info. this is the structure of the array elements stored in m_pPluginInfo.
typedef struct _PluginInfo
{
    uint32  CategoryId;
    uint32  PluginId;
}   PluginInfo;

// implemented in edframe.cpp.I changed this to be WFE_FindMenu in winproto.h SP 4/3/97
//int FindMenu(CMenu *pMenu, CString menuItemName);

// Global align toolbar 
CDropdownToolbar *pTB = NULL;

// Be sure no other timer uses this ID!
#define FEED_IMAGE_LOAD_PAUSE 10

// We do this alot
#define GET_MWCONTEXT  (GetContext() == NULL ? NULL : GetContext()->GetContext())

// This is TRUE only if we have a context and are not an Editor or 
//   we are an editor that has a good buffer and is not blocked 
// THIS MUST BE THE SAME AS IN EDVIEW2.CPP
#define CAN_INTERACT  (GetContext() != NULL && GetContext()->GetContext() != NULL \
                       && !GetContext()->GetContext()->waitingMode \
                       && (!EDT_IS_EDITOR(GetContext()->GetContext()) \
                           || (EDT_HaveEditBuffer(GetContext()->GetContext()) && !EDT_IsBlocked(GetContext()->GetContext()))))

extern BOOL wfe_bUseLastFrameLocation;

// Helper for graying out unselecting before insert objects
BOOL NonLinkObjectIsSelected(MWContext *pMWContext)
{
    ASSERT(pMWContext );
    ED_ElementType type = EDT_GetCurrentElementType(pMWContext);
    return ( type == ED_ELEMENT_HRULE ||
             type == ED_ELEMENT_UNKNOWN_TAG ||
             type == ED_ELEMENT_TARGET ||
             type == ED_ELEMENT_TABLE);
    // Note: This is always followed by EDT_ClearSelection(),
    //  so we can ignore Table/Cell selection
}

/////////////////////////////////////////////////////////////////////////
// Array used to map TagIDs onto listbox indexes for Paragraph styles
//
// These must match order of items in editor.rc2 -- menu and listbox text
TagType FEED_nParagraphTags[] =
{
	P_NSDT,
	P_HEADER_1,
	P_HEADER_2,
	P_HEADER_3,
	P_HEADER_4,
	P_HEADER_5,
	P_HEADER_6,
	P_ADDRESS,
	P_PREFORMAT,
	P_LIST_ITEM,
	P_DESC_TITLE,
	P_DESC_TEXT,
    P_UNKNOWN       // This must be last - we use it for end-of-loop
};


void CNetscapeEditView::OnInsertNonbreakingSpace(){
    EDT_InsertNonbreakingSpace( GET_MWCONTEXT );
}

void CNetscapeEditView::OnInsertTarget()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( pMWContext ){
        // Edit name of existing target if it is selected object
        if( ED_ELEMENT_TARGET == EDT_GetCurrentElementType(pMWContext) ) {
            OnTargetProperties();
            return;
        }
        // Unselect so we insert after after the object
        if( NonLinkObjectIsSelected(pMWContext) )
            EDT_ClearSelection(pMWContext);

        // Default for 3rd param will force inserting new target
        CTargetDlg dlg(this, GET_MWCONTEXT);
        dlg.DoModal();
    }
}

void CNetscapeEditView::OnTargetProperties()
{
    char * pName = NULL;
    MWContext * pMWContext = GET_MWCONTEXT;
    if( !pMWContext )
        return;

    if( ED_ELEMENT_TARGET == EDT_GetCurrentElementType(pMWContext) ) {
        pName = EDT_GetTargetData(pMWContext);
    }

    CTargetDlg dlg(this, pMWContext, pName);
    dlg.DoModal();

    if(pName) XP_FREE(pName);
	// If we automatically caused a selection, then clear it now
	//  so user doesn't accidentally delete our selected object.
	if( m_bAutoSelectObject && EDT_IsSelected(pMWContext) ){
		EDT_ClearSelection(pMWContext);
        m_bAutoSelectObject = FALSE;
    }
}

void CNetscapeEditView::OnUpdateTargetProperties(CCmdUI* pCmdUI)
{
    // Must be on an existing link:
    pCmdUI->Enable( CAN_INTERACT && 
                    ED_ELEMENT_TARGET == EDT_GetCurrentElementType(GET_MWCONTEXT) );
}

void CNetscapeEditView::OnInsertTag()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( pMWContext ){
        // Edit properties of existing tag if it is the selected object
        if( ED_ELEMENT_UNKNOWN_TAG == EDT_GetCurrentElementType(pMWContext) ) {
            OnTagProperties();
            return;
        }
        // Unselect so we insert after after the object
        if( NonLinkObjectIsSelected(pMWContext) )
            EDT_ClearSelection(pMWContext);

        // NULL default for 3rd param will force inserting new tag
        CTagDlg dlg(this, pMWContext);
        dlg.DoModal();
    }
}

void CNetscapeEditView::OnTagProperties()
{
    char * pTag = NULL;
    if( ED_ELEMENT_UNKNOWN_TAG == EDT_GetCurrentElementType(GET_MWCONTEXT) ) {
        pTag = EDT_GetUnknownTagData(GET_MWCONTEXT);
    }

    CTagDlg dlg(this, GET_MWCONTEXT, pTag);
    dlg.DoModal();

    if(pTag) XP_FREE(pTag);
}

void CNetscapeEditView::OnUpdateTagProperties(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( CAN_INTERACT && 
                    ED_ELEMENT_UNKNOWN_TAG == EDT_GetCurrentElementType(GET_MWCONTEXT) );
}

///////////////////////////////////////////////////////////////
// Edit menu
// Find is shared with Navigator - uses common dialog
// Replase only seems to be called from outside (by the bookmark window?)
//   via a registered message
//
void CNetscapeEditView::OnEditFindReplace()
{
    // TODO: something like:  CMainFrame::OnFindReplace();
}

void CNetscapeEditView::OnPasteCharacterStyle()
{
  	MWContext *pMWContext = GET_MWCONTEXT;
    if( pMWContext && EDT_CanPasteStyle(pMWContext) )
        //Actually paste. FALSE would clear stored styles
        EDT_PasteStyle(pMWContext, TRUE); 
}

void CNetscapeEditView::OnUpdatePasteCharacterStyle(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( CAN_INTERACT && EDT_CanPasteStyle(GET_MWCONTEXT) );
}

///////////////////////////////////////////////////////////////
// Insert menu

///////////////////////////////////////////////////////////////
// Format menu

void CNetscapeEditView::OnFormatParagraph( UINT nID )
{
  	MWContext *pMWContext = GET_MWCONTEXT;
    if( pMWContext )
    {
        if( nID == ID_FORMAT_PARAGRAPH_BASE+P_BLOCKQUOTE )
        {
            // Block quote is a special case - its is handled by list
            EDT_ToggleList(GET_MWCONTEXT, TagType(nID - ID_FORMAT_PARAGRAPH_BASE));
        }
        else
        {
	        // Should be current, but lets get style to be sure
	        UINT nParagraphFormat = (UINT)EDT_GetParagraphFormatting( pMWContext );
	        UINT nNewParagraphFormat = CASTUINT(nID-ID_FORMAT_PARAGRAPH_BASE);
            EDT_MorphContainer( pMWContext, nNewParagraphFormat);
            if( nNewParagraphFormat == P_DESC_TEXT || nNewParagraphFormat == P_DESC_TITLE ){
                // We should have a description list
                EDT_MorphContainer(pMWContext, P_DESC_LIST);
            }
        }
    }
}

void CNetscapeEditView::SetPointSize(int iPointSize)
{
	// If Selected, we can't be sure of multible attributes, so set it
	if ( iPointSize > 0  && iPointSize != m_EditState.iFontSize ||
		 EDT_IsSelected(GET_MWCONTEXT) )
	{
        EDT_SetFontPointSize(GET_MWCONTEXT, iPointSize);
   	    // Triggers OnUpdate... to get fontsize set and update combobox
        m_EditState.bFontSizeMaybeChanged = TRUE;
    }
}

void CNetscapeEditView::OnPointSize(UINT nID)
{
    int iSize = 0;
    switch( nID )
    {
        case ID_FORMAT_POINTSIZE_BASE: 
            iSize = 8;
            break;
        case ID_FORMAT_POINTSIZE_BASE+1:
            iSize = 9;
            break;
        case ID_FORMAT_POINTSIZE_BASE+2:
            iSize = 10;
            break;
        case ID_FORMAT_POINTSIZE_BASE+3:
            iSize = 11;
            break;
        case ID_FORMAT_POINTSIZE_BASE+4:
            iSize = 12;
            break;
        case ID_FORMAT_POINTSIZE_BASE+5:
            iSize = 14;
            break;
        case ID_FORMAT_POINTSIZE_BASE+6:
            iSize = 16;
            break;
        case ID_FORMAT_POINTSIZE_BASE+7:
            iSize = 8;
            break;
        case ID_FORMAT_POINTSIZE_BASE+8:
            iSize = 20;
            break;
        case ID_FORMAT_POINTSIZE_BASE+9:
            iSize = 22;
            break;
        case ID_FORMAT_POINTSIZE_BASE+10:
            iSize = 24;
            break;
        case ID_FORMAT_POINTSIZE_BASE+11:
            iSize = 28;
            break;
        case ID_FORMAT_POINTSIZE_BASE+12:
            iSize = 36;
            break;
        case ID_FORMAT_POINTSIZE_BASE+13:
            iSize = 48;
            break;
        case ID_FORMAT_POINTSIZE_BASE+14:
            iSize = 72;
            break;
    }
    if( iSize > 0 )
        SetPointSize(iSize);
}

void CNetscapeEditView::OnCharacterNoTextStyles()
{
    if ( GET_MWCONTEXT ) {
        // Clear all styles EXCEPT HREF
        EDT_CharacterData *pData = EDT_GetCharacterData(GET_MWCONTEXT);
        if( pData ){
            // Set all bits except href's
            pData->mask = ~TF_HREF;
            pData->values = TF_NONE;
            EDT_SetCharacterData(GET_MWCONTEXT, pData);
            EDT_FreeCharacterData(pData);
            return;
        }
    }
}

void CNetscapeEditView::OnCharacterNone()
{
	XP_Bool prefBool;
	PREF_GetBoolPref("editor.hints.removelinks",&prefBool);
    // Warn user if any links are included 
	if ( EDT_SelectionContainsLink(GET_MWCONTEXT) &&
         prefBool ){
        // Show a "Hint" dialog to warn user that
        //   default behavior is to remove links
        //   Use "Yes/No" mode (last param=TRUE)
        //   to clear all styles but leave links alone
        
        CEditHintDlg dlg(this, IDS_UNLINK_WARNING,
                         IDS_CLEAR_CHAR_STYLES_CAPTION, TRUE);

        UINT nRetVal = dlg.DoModal();
        if ( dlg.m_bDontShowAgain ) {
            // Suppress showing this again in futures
			PREF_SetBoolPref("editor.hints.removelinks",FALSE);
        }
        if( nRetVal != IDOK ){
            OnCharacterNoTextStyles();
            return;
        }
    }
    EDT_FormatCharacter(GET_MWCONTEXT, TF_NONE);
}

void CNetscapeEditView::OnCharacterFixedWidth()
{
    // Set fixed width as a character attribute
    // Note: This acts like a toggle, unlike "Fixed Width" from Font Menu or toolbar
    EDT_FormatCharacter(GET_MWCONTEXT, TF_FIXED);
}


// Local helper used to get character style state
void SetCharacterCheck(MWContext * pMWContext, CCmdUI* pCmdUI, CComboToolBar* pToolbar, ED_TextFormat tf )
{
    EDT_CharacterData * pData = EDT_GetCharacterData(pMWContext);
    int iCheck = 0;
    if( pData) {
        if( pData->mask & tf ) {
            iCheck = (pData->values & tf ) ? 1 : 0;
        } else if( pToolbar && pToolbar->m_pInfo ) {
            iCheck = 2; // Style is indeterminate
        } else {
            // indeterminate style does not show up in menu - looks checked
            iCheck = 0; // Style is indeterminate
        }
        EDT_FreeCharacterData(pData);
    }

    if ( pCmdUI->m_pMenu )              // Always change menu items
        pCmdUI->SetCheck( iCheck );
    else if ( pToolbar && pToolbar->m_pInfo ) {
        pToolbar->SetCheck( pCmdUI->m_nID, iCheck );
    }
}

void CNetscapeEditView::OnCharacterStyle(UINT nID)
{
    ED_TextFormat iStyle = -1;
    switch( nID )
    {
        case ID_FORMAT_CHAR_BOLD:
            iStyle = TF_BOLD;
            break;
        case ID_FORMAT_CHAR_ITALIC:
            iStyle = TF_ITALIC;
            break;
        case ID_FORMAT_CHAR_NOBREAKS:
            iStyle = TF_NOBREAK;
            break;
        case ID_FORMAT_CHAR_UNDERLINE:
            iStyle = TF_UNDERLINE;
            break;
        case ID_FORMAT_CHAR_SUPER:
            iStyle = TF_SUPER;
            break;
        case ID_FORMAT_CHAR_SUB:
            iStyle = TF_SUB;
            break;
        case ID_FORMAT_CHAR_STRIKEOUT:
            iStyle = TF_STRIKEOUT;
            break;
        case ID_FORMAT_CHAR_BLINK:
            iStyle = TF_BLINK;
            break;
    }
    if( iStyle != -1 ){
        EDT_FormatCharacter(GET_MWCONTEXT, iStyle);
    }
}

void SetCharacterCheck(MWContext * pMWContext, CCmdUI* pCmdUI, CWnd* pToolbar, ED_TextFormat tf )
{
    EDT_CharacterData * pData = EDT_GetCharacterData(pMWContext);
    int iCheck = 0;
    if( pData) {
        if( pData->mask & tf ) {
            iCheck = (pData->values & tf ) ? 1 : 0;
        } else if( pToolbar ) {
            iCheck = 2; // Style is indeterminate
        } else {
            // indeterminate style does not show up in menu - looks checked
            iCheck = 0; // Style is indeterminate
        }
        EDT_FreeCharacterData(pData);
    }

    if ( pCmdUI->m_pMenu )              // Always change menu items
        pCmdUI->SetCheck( iCheck );
    else if ( pToolbar ) {
        pCmdUI->SetCheck( iCheck );
    }
}

void CNetscapeEditView::OnUpdateCharacterStyle(UINT nID, CCmdUI* pCmdUI)
{
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController) {
        ED_TextFormat iStyle = -1;
        switch( nID )
        {
            case ID_FORMAT_CHAR_BOLD:
                iStyle = TF_BOLD;
                break;
            case ID_FORMAT_CHAR_ITALIC:
                iStyle = TF_ITALIC;
                break;
            case ID_FORMAT_CHAR_NOBREAKS:
                iStyle = TF_NOBREAK;
                break;
            case ID_FORMAT_CHAR_UNDERLINE:
                iStyle = TF_UNDERLINE;
                break;
            case ID_FORMAT_CHAR_SUPER:
                iStyle = TF_SUPER;
                break;
            case ID_FORMAT_CHAR_SUB:
                iStyle = TF_SUB;
                break;
            case ID_FORMAT_CHAR_STRIKEOUT:
                iStyle = TF_STRIKEOUT;
                break;
            case ID_FORMAT_CHAR_BLINK:
                iStyle = TF_BLINK;
                break;
        }
        if( iStyle != -1 ){
            if ( pController->GetCNSToolbar())
    	    	SetCharacterCheck(GET_MWCONTEXT, pCmdUI, pController->GetCNSToolbar(), iStyle);
            else if (pController->GetCharacterBar())
    	    	SetCharacterCheck(GET_MWCONTEXT, pCmdUI, pController->GetCharacterBar(), iStyle);
	    	pCmdUI->Enable( EDT_CanSetCharacterAttribute(GET_MWCONTEXT) );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void CNetscapeEditView::OnSetFocusParagraphStyle()
{
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);


	if (pController) {
		pController->GetParagraphCombo()->SetFocus();
        pController->GetParagraphCombo()->ShowDropDown();
	}
}

void CNetscapeEditView::OnSetFocusFontFace()
{
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController) {
		pController->GetFontFaceCombo()->SetFocus();
        pController->GetFontFaceCombo()->ShowDropDown();
	}
}

void CNetscapeEditView::OnSetFocusFontSize()
{
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController) {
		pController->GetFontSizeCombo()->SetFocus();
        pController->GetFontSizeCombo()->ShowDropDown();
	}
}

static BOOL bBusy = FALSE;

void CNetscapeEditView::OnGetFontColor()
{
    // Prevent recursion
    if( bBusy )
        return;

    bBusy = TRUE;

    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController)
    {
        pController->GetFontColorCombo()->SendMessage(WM_LBUTTONUP, 0,0);// ShowDropDown(FALSE);
        // NEW: Color Combo is only for showing color in toolbar,
        //   popup dialog to get color to set
    	MWContext *pMWContext = GET_MWCONTEXT;
        CComboBox * pCombo = pController->GetFontColorCombo();
        if( !pMWContext || !pCombo )
            return;
        
        // Get the combobox location so we popup new dialog just under it
        RECT rect = {0,0,0,0};
        if( pCombo->IsWindowVisible() )
            pCombo->GetWindowRect(&rect);

        // "Hidden" feature: If Alt key is pressed, set the background color
        // (Note: Shift+Ctrl+C launches color picker via keyboard,
        //  so we can't use Shift or Control!)
        XP_Bool bBackground = (GetAsyncKeyState(VK_MENU) < 0);
        
        LO_Color LoColor;
        COLORREF crBackground;
        UINT nIDCaption = IDS_TEXT_COLOR;
        if( bBackground )
        {
            // Get color of current cell, table, or the page
            ED_ElementType type = EDT_GetBackgroundColor(pMWContext, &LoColor);
            crBackground = WFE_LO2COLORREF( &LoColor, LO_COLOR_BG );

            // Set caption string according to type
            if( type == ED_ELEMENT_TABLE )
                nIDCaption = IDS_TABLE_BACKGROUND;
            else if( type == ED_ELEMENT_CELL )
                nIDCaption = IDS_CELL_BACKGROUND;
            else
                nIDCaption = IDS_PAGE_BACKGROUND;
        }
        CColorPicker dlg(GET_DLG_PARENT(this), pMWContext, 
                         bBackground ? crBackground : m_EditState.crFontColor, 
                         bBackground ? BACKGROUND_COLORREF : DEFAULT_COLORREF, 
                         nIDCaption, &rect); 

        COLORREF crNew = dlg.GetColor();
        if( crNew != CANCEL_COLORREF )
        {
            WFE_SetLO_Color(crNew, &LoColor);
            if( bBackground )
            {
                if( crNew != crBackground )
                {
                    // Set the BACKGROUND color:
                    // If "Default", no color attribute is written
                    EDT_SetBackgroundColor(pMWContext, (crNew == DEFAULT_COLORREF) ? NULL : &LoColor);
                }
            }
            else
            {
                // Set the FONT color
		        if ( crNew != m_EditState.crFontColor ||
			         EDT_IsSelected(pMWContext) )
		        {
                    EDT_SetFontColor(pMWContext, (crNew == DEFAULT_COLORREF) ? NULL : &LoColor);

                    // Trigger update of color in combo display
                    m_EditState.bFontColorMaybeChanged = TRUE;
                }
            }
        }
        // Return focus to the view
        SetFocus();
	}
    bBusy = FALSE;
}

////////////////////////////////////////////////////////////////////////////////

// User presses Esc key while in any combobox - return focus to main view
void CNetscapeEditView::OnCancelComboBox()
{
	// Return focus to view
	SetFocus();
}

void CNetscapeEditView::OnSelendokParagraphCombo()
{
	// TODO: ADD CODE TO TEST IF ALLOWED TO CHANGE STYLE!

	// Get index to selected item in Paragraph styles list
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController) {
		UINT	nSelected = pController->GetParagraphCombo()->GetCurSel();

		if ( nSelected >= 0 && 
		    m_EditState.nParagraphFormat != FEED_nParagraphTags[nSelected] )
	    {
			OnFormatParagraph(CASTUINT(FEED_nParagraphTags[nSelected]+ID_FORMAT_PARAGRAPH_BASE));
			m_EditState.bParaFormatMaybeChanged = TRUE;
	    }
	
		// Return focus to view
		SetFocus();
	}
}


void CNetscapeEditView::OnUpdateParagraphComboBox(CCmdUI* pCmdUI)
{
	if ( m_EditState.bParaFormatMaybeChanged && 
		 GetFocus() == this )
	{
		TagType nParagraphFormat = EDT_GetParagraphFormatting( GET_MWCONTEXT );

		if ( m_EditState.nParagraphFormat != nParagraphFormat )
		{
            CEditToolBarController *pController;
            CWnd *t_parent;
            if (!GetEmbedded())
                t_parent = (CWnd *)GetParentFrame();
            else
                t_parent = GetParent();
            if (t_parent)
                pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
			if (pController) {
				if ( nParagraphFormat == P_UNKNOWN ) 
					pController->GetParagraphCombo()->SetCurSel(-1);
				else
					for ( int i = 0; FEED_nParagraphTags[i] != P_UNKNOWN; i++ )
					{
						if ( FEED_nParagraphTags[i] == nParagraphFormat )
						{	
							pController->GetParagraphCombo()->SetCurSel(i);
							break;
						}
					}
			
				// Note that we save state only for Toolbar
				m_EditState.nParagraphFormat = nParagraphFormat;
			}
		}
	    m_EditState.bParaFormatMaybeChanged = FALSE;
	}
    pCmdUI->Enable( EDT_CanSetCharacterAttribute(GET_MWCONTEXT) );
}

/////////////////////////////////////////////////////////////////////////
// Font Face controls

void CNetscapeEditView::OnUpdateFontFaceComboBox(CCmdUI* pCmdUI)
{
	if ( m_EditState.bFontFaceMaybeChanged && 
		 GetFocus() == this )
	{
    	MWContext *pMWContext = GET_MWCONTEXT;
        if(pMWContext){
            CEditToolBarController *pController;
            CWnd *t_parent;
            if (!GetEmbedded())
                t_parent = (CWnd *)GetParentFrame();
            else
                t_parent = GetParent();
            if (t_parent)
                pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
			if (pController) {
                CNSComboBox * pCombo = pController->GetFontFaceCombo();
                char * pFace = EDT_GetFontFace(pMWContext);
                int iFontIndex;
                if( pFace ){
                    iFontIndex = pCombo->FindSelectedOrSetText(pFace);
                } else {
                    iFontIndex = -1;
                    pCombo->SetCurSel(iFontIndex);
                }
                // Note: We always update this before size combox, thus
                //   we can see if font base size has changed and we are
                //   changing from FixedWidth to not-FixedWidth or vice versa.
                //   If we are changing, then we must force update of font size value
                //     when we are converting relative size to points
                if( wfe_iFontSizeMode == ED_FONTSIZE_POINTS &&
                    iFontIndex != m_EditState.iFontIndex &&
                    (iFontIndex == 1 || m_EditState.iFontIndex == 1) ){
                    m_EditState.iFontSize = -1;    
                    m_EditState.bFontSizeMaybeChanged = TRUE;
                }
                m_EditState.iFontIndex = iFontIndex;
                m_EditState.bFontFaceMaybeChanged = FALSE;
            }
        }
    }
    pCmdUI->Enable( EDT_CanSetCharacterAttribute(GET_MWCONTEXT) );
}

void CNetscapeEditView::OnSelendokFontFaceCombo()
{
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController) {
        int iFontIndex = pController->GetSelectedFontFaceIndex();
        // Process "Other..." to get font from dialog and set it
        if( iFontIndex == INDEX_OTHER ){
            OnSetLocalFontFace();
            return;
        } else {
            // Let cross platform code process font face string from combobox
            EDT_SetFontFace(GET_MWCONTEXT, NULL, 0, 
                            (char*)pController->GetFontFaceCombo()->GetItemData(iFontIndex));
        }
		SetFocus();
	}
}

void CNetscapeEditView::OnRemoveFontFace()
{
    // Set font to the "default proportional font",
    //  which effectively removes the tag
    EDT_SetFontFace(GET_MWCONTEXT, NULL, 0, NULL);
}

void CNetscapeEditView::OnSetLocalFontFace()
{
    // Popup Window's font face picker dialog
    CFontDialog dlg(NULL, CF_SCREENFONTS | CF_TTONLY | CF_NOSIZESEL | CF_NOSTYLESEL | CF_NOSCRIPTSEL, NULL, this);
    if( dlg.DoModal() ){
        CString csNewFont = dlg.GetFaceName();
        // Set the arbitrary font face 
        EDT_SetFontFace(GET_MWCONTEXT, NULL, 0, (char*)LPCSTR(csNewFont));
        // Trigger update of fontname in toolbar combobox
        m_EditState.bFontFaceMaybeChanged = TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////
// Font Size controls

// Menu only:
void CNetscapeEditView::OnFontSize(UINT nID)
{
    int	iNewFontSize = (int)(nID - ID_FORMAT_FONTSIZE_BASE + 1);

	// If Selected, we can't be sure of multible attributes, so always set it
	if ( iNewFontSize != EDT_GetFontSize( GET_MWCONTEXT ) ||
	     EDT_IsSelected(GET_MWCONTEXT) )
    {
        // Set the size using relative font scale (1 to 7)
        EDT_SetFontSize( GET_MWCONTEXT, iNewFontSize );

        // Triggers OnUpdate... to get fontsize set and update combobox
        m_EditState.bFontSizeMaybeChanged = TRUE;
    }
}

// Menu only:
void CNetscapeEditView::OnSelendokFontSizeCombo()
{
    MWContext *pMWContext = GET_MWCONTEXT;
    if(!pMWContext){
        return;
    }
    
	// Font size is 1 more than index to selected item
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController) {
        int iSel = pController->GetFontSizeCombo()->GetCurSel();
        int iNewFontSize =  0;

		if( iSel < MAX_FONT_SIZE ){
            iNewFontSize =  iSel + 1;
        } else {
            char pSize[16] = "";
            char *pEnd;
            strcpy(pSize, (char*)pController->GetFontSizeCombo()->GetItemData(iSel));
            // We will trust that the strings in the combobox 
            //   begin with a valid integer, so strings like "8 pts"
            //   will yield "8" and we don't check for 
            //   "bad" string
            iNewFontSize = (int)strtol( pSize, &pEnd, 10 );
        }
		// If Selected, we can't be sure of multible attributes, so set it
		if ( iNewFontSize > 0  && iNewFontSize != m_EditState.iFontSize ||
			 EDT_IsSelected(pMWContext) )
		{
            if( iSel < MAX_FONT_SIZE ){
                EDT_SetFontSize(pMWContext, iNewFontSize);
            } else {
                EDT_SetFontPointSize(pMWContext, iNewFontSize);
            }
    	    // Triggers OnUpdate... to get fontsize set and update combobox
	        m_EditState.bFontSizeMaybeChanged = TRUE;
		}
		SetFocus();
	}
}

void CNetscapeEditView::OnFontSizeDropDown()
{
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController) {
        CNSComboBox * pCombo = pController->GetFontSizeCombo();
        int iSel = pCombo->GetCurSel();
        // Refill font sizes in case font basesize changed
        //  (when switching from variable to fixed width font or vice versa)
        // 3rd param = TRUE when using Fixed Width font
        wfe_FillFontSizeCombo(GET_MWCONTEXT, pCombo, (EDT_GetFontFaceIndex(GET_MWCONTEXT) == 1));
        pCombo->SetCurSel(iSel);
        
        // Allow search function, but doesn't make much sense in pure relative mode
        if( wfe_iFontSizeMode != ED_FONTSIZE_RELATIVE ){
            pCombo->InitSearch();
        }
    }
}

void CNetscapeEditView::OnUpdateFontSizeComboBox(CCmdUI* pCmdUI)
{
	if ( m_EditState.bFontSizeMaybeChanged &&
		 GetFocus() == this )
    {
    	MWContext *pMWContext = GET_MWCONTEXT;
        int	iFontSize = 0;
    	if( pMWContext )
        {
            char * pSize = NULL;
            char pSizeNotInList[16];

            EDT_CharacterData * pData = EDT_GetCharacterData(pMWContext);
            if(pData)
            {
                if( (pData->mask & TF_FONT_SIZE) && (pData->mask & TF_FONT_POINT_SIZE) )
                {
                    iFontSize = pData->iPointSize ? pData->iPointSize : pData->iSize;
                    int iFontIndex = EDT_GetFontFaceIndex(pMWContext);
                    // Point sizes in lower range overlap with relative size numbers (1 through 7)
                    //  and combobox item uses "8 pts" but we only want to display "8"
                    //  when combobox is closed
                    BOOL bSmallPointSize = (pData->iPointSize > 0 && pData->iPointSize <= 8 );

                    if( iFontSize != m_EditState.iFontSize ||
                        bSmallPointSize ||
                        iFontIndex != m_EditState.iFontIndex || 
                        iFontIndex > 1 )
                    {
                        CEditToolBarController *pController;
                        CWnd *t_parent;
                        if (!GetEmbedded())
                            t_parent = (CWnd *)GetParentFrame();
                        else
                            t_parent = GetParent();
                        if (t_parent)
                            pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
			            if (pController)
                        {
                            char * pSize = NULL;
                            char pSizeNotInList[16];
                            if( !bSmallPointSize && iFontSize > 0 && iFontSize <= MAX_FONT_SIZE )
                            {
                                pController->GetFontSizeCombo()->SetCurSel(iFontSize-1);
                                pSize = wfe_GetFontSizeString(pMWContext, iFontSize, iFontIndex == 1);
                            } else {
                                wsprintf(pSizeNotInList, "%d", iFontSize);
                                pSize = pSizeNotInList;
                            }
                            if(pSize)
                                pController->GetFontSizeCombo()->FindSelectedOrSetText(pSize, MAX_FONT_SIZE);
                        }
                    }
                }
                else
                {
                    CEditToolBarController *pController;
                    CWnd *t_parent;
                    if (!GetEmbedded())
                        t_parent = (CWnd *)GetParentFrame();
                    else
                        t_parent = GetParent();
                    if (t_parent)
                        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
			        if (pController)
                        pController->GetFontSizeCombo()->FindSelectedOrSetText(NULL);
                }
                m_EditState.iFontSize = iFontSize;
                EDT_FreeCharacterData(pData);
           }
           m_EditState.bFontSizeMaybeChanged = FALSE;
        }
        if( pCmdUI )
            pCmdUI->Enable( EDT_CanSetCharacterAttribute(pMWContext) );
    }
}

void CNetscapeEditView::UpdateFontSizeCombo()
{
    // Change current state to force updating the combo
    m_EditState.bFontSizeMaybeChanged = TRUE;
    m_EditState.iFontSize = -2;
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController) {
        wfe_FillFontSizeCombo(GET_MWCONTEXT, pController->GetFontSizeCombo(), 
                              (EDT_GetFontFaceIndex(GET_MWCONTEXT) == 1));
    }
    OnUpdateFontSizeComboBox(NULL);
}

void CNetscapeEditView::OnIncreaseFontSize()
{
    EDT_IncreaseFontSize(GET_MWCONTEXT);
    // Force update of size controls on toolbar
    m_EditState.bFontSizeMaybeChanged = TRUE;
}

void CNetscapeEditView::OnDecreaseFontSize()
{
    EDT_DecreaseFontSize(GET_MWCONTEXT);
    // Force update of size controls on toolbar
    m_EditState.bFontSizeMaybeChanged = TRUE;
}

/////////////////////////////////////////////////////////////////////////
// Font Color controls


// From the Menu only -- CURRENTLY NO SUBMENU FOR COLOR
void CNetscapeEditView::OnFontColorMenu(UINT nID)
{
	int i = CASTINT(nID - ID_FORMAT_FONTCOLOR_BASE);

    // Figure out current color at cursor
    COLORREF crColor = WFE_GetCurrentFontColor(GET_MWCONTEXT);

	// If Selected, we can't be sure of multible attributes, so set it
	if ( crColor != wfe_CustomPalette[i] ||
	     EDT_IsSelected(GET_MWCONTEXT) )
	{
        // Set the Color 
        LO_Color FontColor;
    	FontColor.red = GetRValue(wfe_CustomPalette[i]);
	    FontColor.green = GetGValue(wfe_CustomPalette[i]);
	    FontColor.blue = GetBValue(wfe_CustomPalette[i]);
	        
	    EDT_SetFontColor( GET_MWCONTEXT, &FontColor );

        m_EditState.bFontColorMaybeChanged = TRUE;
	}
}

void CNetscapeEditView::OnUpdateFontColorComboBox(CCmdUI* pCmdUI)
{
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
    if (pController) {
	    if ( m_EditState.bFontColorMaybeChanged && 
	         GetFocus() == this )
	    {
            // Figure out current color at cursor
            COLORREF cr = WFE_GetCurrentFontColor(GET_MWCONTEXT);
            if( cr != m_EditState.crFontColor ){
                // Set color in combobox
                pController->GetFontColorCombo()->SetColor(cr);
                m_EditState.crFontColor = cr;
            }
            m_EditState.bFontColorMaybeChanged = FALSE;
        }
    }
    // We can allow this to be active if not in text because
    //   we allow access to background color as well
    pCmdUI->Enable(CAN_INTERACT);
}

//////////////////////////////////////////////////////////////////
// Insert Objects and Properties

void CNetscapeEditView::OnLButtonDblClk(UINT nFlags, CPoint cPoint) 
// Trap double-click on objects that we want to pop-up property dialogs
// (Note that we do NOT call CGenericView::OnLButtonDblClk)
{
    CView::OnLButtonDblClk(nFlags, cPoint);
    MWContext *pMWContext = GET_MWCONTEXT;
    if( !pMWContext )
        return;

    XY Point;
    GetContext()->ResolvePoint(Point, cPoint);

    BOOL bInLink = !EDT_IsSelected(pMWContext) && EDT_CanSetHREF(pMWContext);
    if( !bInLink )
    {
        switch( EDT_GetCurrentElementType(pMWContext) )
        {
            case ED_ELEMENT_IMAGE:
                OnImageProperties();
                return;
            case ED_ELEMENT_HRULE:
                OnHRuleProperties();
                return;
            case ED_ELEMENT_TARGET:
                OnTargetProperties();
                return;
            case ED_ELEMENT_UNKNOWN_TAG:
                OnTagProperties();
                return;
            default:
                // Check for table hit areas - do table region properties
                LO_Element *pTableElement = NULL;
                ED_HitType iHit =  EDT_GetTableHitRegion(pMWContext, Point.x, Point.y, &pTableElement, (nFlags & MK_CONTROL));
                if( !iHit && pTableElement && pTableElement->type == LO_TABLE )
                {
                    iHit = ED_HIT_SEL_TABLE;
                }
                int iStartPage = -1;
                switch( iHit )
                {
                    case ED_HIT_SEL_TABLE:
                    case ED_HIT_ADD_ROWS:
                    case ED_HIT_ADD_COLS:
                    case ED_HIT_SIZE_TABLE_WIDTH:
                        // Allow any of 4 corners to select table properties
                        iStartPage = 0;
                        break;
                    case ED_HIT_SEL_ROW:
                        iStartPage = 1;
                        break;
                    case ED_HIT_SEL_COL:
                        iStartPage = 1;
                        break;
                    case ED_HIT_SEL_CELL:
                    case ED_HIT_SEL_ALL_CELLS:
                        iStartPage = 1;
                        break;
                }
                if( iStartPage >= 0 )
                {
                    // Select the region double-clicked on. 
                    //  1st FALSE = erase existing table selection, 2nd = don't extend selection
                    EDT_SelectTableElement(pMWContext, Point.x, Point.y, pTableElement, iHit, FALSE, FALSE);
                    OnTableProperties(iStartPage);
                    return;
                }
                break;
        }
    }
    // This will select a link or word
    if(GetContext() != NULL && GetContext()->IsDestroyed() == FALSE)
    {
        //	Pass it off to the context to handle.
	    GetContext()->OnLButtonDblClkCX(nFlags, cPoint);
    }
    // If on a link, popup Link Properties
    if( bInLink )
    {
        OnLinkProperties();
    }
}

// Hook into button up to trigger Update of UI elements after 
//   dragging a selection
void CNetscapeEditView::OnLButtonUp(UINT nFlags, CPoint point)
{
    CNetscapeView::OnLButtonUp(nFlags, point);
    SetEditChanged();
}

void CNetscapeEditView::OnDisplayTables()
{
    EDT_SetDisplayTables(GET_MWCONTEXT, !EDT_GetDisplayTables(GET_MWCONTEXT));
}

void CNetscapeEditView::OnUpdateDisplayTables(CCmdUI* pCmdUI)
{
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if( pController && pCmdUI->m_pMenu ){
        pCmdUI->m_pMenu->ModifyMenu(ID_DISPLAY_TABLES, MF_BYCOMMAND | MF_STRING, ID_DISPLAY_TABLES,
                                    szLoadString(EDT_GetDisplayTables(GET_MWCONTEXT) ?
                                                 IDS_HIDE_TABLE_BORDERS : IDS_SHOW_TABLE_BORDERS) );
    }
    pCmdUI->Enable(CAN_INTERACT);
}

void CNetscapeEditView::OnRButtonDown(UINT uFlags, CPoint cpPoint)
//	Purpose:	Bring up the popup menu - extremely state-dependent on: element type, selection, clipboard, is in table, etc
//	Arguments:	uFlags	What meta keys are currently pressed, ignored.
//				cpPoint	The point at which the mouse was clicked in relative to the upper left corner of the window.
//	Revision History:
//		11-03-95	created CLM
//      --> today   One of the most revised pieces of code in Composer!
//
{
	MWContext *pMWContext = GET_MWCONTEXT;
    if( pMWContext == NULL || pMWContext->waitingMode || EDT_IsBlocked(pMWContext) ){
        return;
    }
    // We are getting occasional caret artifacts - clear it here?
    FE_DestroyCaret(pMWContext);

	//Note: DON'T Call the base class to handle the click.
    //  We don't need to do anything in GenView or CWinCX

    int32 x, y;
    ClientToDocXY(cpPoint, &x, &y);
	LO_Element *pElement = NULL;
    BOOL bSelectTableOrCell = FALSE;

    m_bAutoSelectObject = FALSE;

    // Check if clicking on a Table or Cell selection point
    LO_Element * pTableElement = NULL;
    BOOL bSizeTable = FALSE;
    BOOL bIsInSelectedText = GetContext()->PtInSelectedRegion(cpPoint, TRUE);

    ED_HitType iTableHit = ED_HIT_NONE;

    // Right Button down: Check if we can select or size a Table or Cell,
    //   but not if Alt key is pressed, ignore the table
    //   to allow sizing objects tightly surrounded by Cell border
    if( !(uFlags & MK_ALT))
    {
        iTableHit = EDT_GetTableHitRegion(pMWContext, x, y, &pTableElement, (uFlags & MK_CONTROL) );
    }

    if( pTableElement && iTableHit != ED_HIT_NONE )
    {
        // We are selecting a table or cell
        pElement = pTableElement;
        // Mouse is in a selectable region for table, row, column, or cell.
        // If Ctrl key is down and a cell is selected, it is appended to other table cells selected
        EDT_SelectTableElement(pMWContext, x, y, pTableElement, iTableHit,
                               (uFlags & MK_CONTROL), (uFlags & MK_SHIFT) );
        bSelectTableOrCell = TRUE;

        // Don't select anything else -- just position caret in closest table cell
        EDT_PositionCaret(pMWContext, x, y);
    } else {
        if( !bIsInSelectedText )
        {
            // EDT_GetTableHitRegion kindly returns the cell we clicked in 
            //  even if we didn't hit one of the edges.
            // Check if we clicked inbetween cells (pTableElement = LO_TABLE)
            //  or within a cell that is selected.
            if( pTableElement && (pTableElement->type == LO_TABLE ||
                (pTableElement->lo_cell.ele_attrmask & LO_ELE_SELECTED)) )
            {
                // Don't select internal object, just
                //  position the caret within the selected cell
                //  or in closest cell if pTableElement is a table
                // Use x, y from element to be sure we position within the table
                if( pTableElement->type == LO_TABLE )
                {
                    // We hit between cells - select the table
                    iTableHit = ED_HIT_SEL_TABLE;

                    // We need to adjust hit spot so caret will be placed
                    //  within the nearest cell (bellow/right)
                    // If we don't, caret will be postitioned before the table!
                    // TODO: Write an EDT_ function to find closest cell.
                    x += (pTableElement->lo_table.inter_cell_space + 2);
                    y += (pTableElement->lo_table.inter_cell_space + 2);

                    EDT_PositionCaret(pMWContext, x, y);
                    EDT_SelectTableElement(pMWContext, 0, 0, pTableElement, iTableHit, FALSE, FALSE);
                    bSelectTableOrCell = TRUE;
                } else {
                    // Get last-selected table region type so 
                    //  we can show region-specific properties item,
                    //  even though we will also show character and other property menu items above it
                    iTableHit = EDT_GetSelectedTableElement(pMWContext, NULL );
                    EDT_PositionCaret(pMWContext, x, y);
                }
            } 
            else
            {
                // We are not over an existing selection
                //   and not inside a selected cell
                // Remove any existing selection, move caret to cursor position,
                //  then start selection. This will select single objects
                //  such as image or HRule. If no object selected,
                //  the caret is moved to inside the cell.
                //  Note: THIS DOES NOT REMOVE TABLE SELECTION!
                EDT_SelectObject(pMWContext, x, y);
                if( EDT_IsSelected(pMWContext) )
                {
                    // Set flag so we can undo the selection when done with property dialog
                    m_bAutoSelectObject = TRUE;
                }
                // Clear any cell selection if caret isn't inside a selected cell
                // By not clearing selection if we ARE inside,
                //   user can do properties on selected cells
                EDT_ClearCellSelectionIfNotInside(pMWContext);
            }
        }
    }

	//	Save the point of the click.
	//	Look in popup handlers for the real usage of the cached point.
	//	Obtain the layout element at that point.
	m_ptRBDown = cpPoint;	
    if( !bSelectTableOrCell )
    {
#ifdef LAYERS
	    // BUGBUG We need to compositor-based event dispatch in 
	    // this case too.
        //TODO This is temorary fix for the devcon demo. I'll change this code to dispatch a compositor event when I figure
        // out how to do it. - kamal 10/4/96 
        CL_Layer *Layer;
    
        // Get the "content" layer handle from the root layer.
        Layer = CL_GetCompositorRoot(pMWContext->compositor);
        Layer = CL_GetLayerChildByName(Layer, LO_BODY_LAYER_NAME);
        Layer = CL_GetLayerChildByName(Layer, LO_CONTENT_LAYER_NAME);
        pElement = GetLayoutElement(cpPoint, Layer);
#else
    	pElement = GetLayoutElement(cpPoint);
#endif
        if (pElement)
       	    m_csRBLink = GetAnchorHref(pElement);
    }

	//	Create the popup.
	CMenu cmPopup;
	if(cmPopup.CreatePopupMenu() == 0)	{
		return;
	}
	
    BOOL bIsSelected = bSelectTableOrCell || EDT_IsSelected(pMWContext);
    BOOL bInTable = bSelectTableOrCell || EDT_IsInsertPointInTable(pMWContext);

    // Get what types are available on the clipboard
    BOOL bHaveText, bHaveLink, bHaveImage, bHaveTable;
    BOOL bCanPaste = wfe_GetClipboardTypes(pMWContext, bHaveText, bHaveImage, bHaveLink, bHaveTable );
    BOOL bIsLink = !m_csRBLink.IsEmpty();
    char *pLinkURL = bIsLink ? (char*)LPCSTR(m_csRBLink) : 0;

    // Pasting a table in a table offers a submenu of options
    BOOL bCanPasteTableInTable = bInTable ? bHaveTable : FALSE;

    // This gives us lots of usefull info, but ONLY if caret is in text
    //   or selection contains some text
    EDT_CharacterData *pCharData = EDT_GetCharacterData(pMWContext);

	// Some of following block is extracted from popup-up handling in Browser
    // Editor will move caret and do most things differently,
    //   but we want to use some link and image functions
	//TODO: DO ANYTHING ON EMBEDED ITEMS???
    m_csRBEmbed.Empty();

	UINT uState = MF_ENABLED;
    UINT uMailtoState = MF_ENABLED; 
    CString csEntry;
    UINT nID = 0;
    UINT nIDS;
    ED_ElementType type = ED_ELEMENT_NONE;
    BOOL bOneLink = FALSE;
    BOOL bNoLinks = FALSE;
    BOOL bMayHaveManyLinks = FALSE;
    BOOL bLinkProps = FALSE;

    if( !bSelectTableOrCell )
    {
        // Save link info so we can edit or browse
        if( pElement )
            m_csRBImage = GetImageHref(pElement);

        csEntry.LoadString(IDS_POPUP_LOAD_LINK_EDT);
        CString csAppend = m_csRBLink;
    
	    if(bIsLink) {
    	    WFE_CondenseURL(csAppend, 25);
    	    csEntry += csAppend;		
    	    csAppend.Empty();
        }

	    //	Need to figure out mailto state, and any other URLs
	    //		that won't make sense in a new window, or with
	    //		save as.
	    if(strnicmp(m_csRBLink, "mailto:", 7) == 0 || strnicmp(m_csRBLink, "telnet:", 7) == 0 ||
	        strnicmp(m_csRBLink, "tn3270:", 7) == 0 || strnicmp(m_csRBLink, "rlogin:", 7) == 0)	{
		    uMailtoState = MF_GRAYED;
	    }

        // The Context-sensitive items - what object are we on
        type = EDT_GetCurrentElementType(pMWContext);
        // If a Table or Cell is selected,
        //  there can be no other element selected,
        //  so caret must be at text element
        if( type >= ED_ELEMENT_TABLE )
            type = ED_ELEMENT_TEXT;

        // Get HREF state
        // We are certain (same HREF across selection or
        // caret in a link) if mask bit is set
        if( pCharData && pCharData->mask & TF_HREF )
        {
            // We are certain that we have a single HREF 
            //  throughout entire selection or none at all
            if( pCharData->values & TF_HREF)
            {
                bOneLink = TRUE;
            } else {
                bNoLinks = TRUE;
            }
        } else if( type == ED_ELEMENT_IMAGE && bIsSelected )
        {
            // TODO: ltabb bug -- a selected image with no HREF
            //  should have TF_HREF mask bit set
            if( bIsLink )
            {
                bOneLink = TRUE;
            } else {
                bNoLinks = TRUE;
            }
        } else {
            // We are uncertain (mixed selection with 
            // at least 1 HREF found) if mask bit is clear
            // THIS IS NOT CORRECT if we have only images selected
            bMayHaveManyLinks = TRUE;
            // csRBLink should be EMPTY, right? TEST THIS!
        } 

        switch( type )
        {
            case ED_ELEMENT_IMAGE:
        	    if(bIsLink)
                {
            	    nIDS = IDS_PROPS_IMAGE_LINK;
                } else {
            	    nIDS = IDS_PROPS_IMAGE;
                }
                nID = ID_PROPS_IMAGE;
                break;
            case ED_ELEMENT_HRULE:
        	    nIDS = IDS_PROPS_HRULE;
                nID = ID_PROPS_HRULE;
                break;
            case ED_ELEMENT_TARGET:
        	    nIDS = IDS_PROPS_TARGET;
                nID = ID_PROPS_TARGET;
                break;
                // TODO: ADD TARGET, JAVA, PLUG-IN and FORM ELEMENTS
            case ED_ELEMENT_UNKNOWN_TAG:
                nIDS = IDS_PROPS_TAG;
                nID =ID_PROPS_TAG;
                break;
            default:    // This is also used when table or cell is element type
            //case ED_ELEMENT_TEXT:
            //case ED_ELEMENT_SELECTION:

                // We automatically select a link with right mouse down,
                //  so check if we are in a link. BUT WHAT ABOUT MIXED SELECTION!
                // TODO: TEST THIS IF SELECTION STARTS IN LINK VS.
                //       SELECTION ENDS IN LINKS
                // TODO: How to Test for IMAGE-ONLY selected block?
        	    if(bIsLink)
                {
                    nIDS = IDS_PROPS_LINK; 
                    nID = ID_PROPS_LINK;
                    bLinkProps = FALSE;
                } else if( pCharData )
                {    
                    nIDS = IDS_PROPS_CHARACTER; 
                    nID = ID_PROPS_CHARACTER;
                }
                break;            
        }

        if ( nID != ID_PROPS_CHARACTER &&
             !EDT_IsSelected(pMWContext) )
        {
	        // If no selection, caret is just before an object,
            //    so also include text properties
	        cmPopup.AppendMenu(MF_ENABLED, ID_PROPS_CHARACTER, szLoadString(IDS_PROPS_CHARACTER));
        }

        if( nID )
        {
            cmPopup.AppendMenu(MF_ENABLED, nID, szLoadString(nIDS));
        }
        // Paragraph props are always available if not table selecting
	    cmPopup.AppendMenu(MF_ENABLED, ID_PROPS_PARAGRAPH,
	                       szLoadString(IDS_PROPS_PARAGRAPH));  
    
        // Show "Background and Page Properties" only if not in table
        if( !bInTable )
        {
           cmPopup.AppendMenu(MF_ENABLED, ID_PROPS_DOC_COLOR, szLoadString(IDS_POPUP_PROPS_PAGE));
        }

	    cmPopup.AppendMenu(MF_SEPARATOR);
    }
    HMENU hSelectMenu = 0;
    HMENU hInsertMenu = 0;
    HMENU hInsertTableMenu = 0;
    HMENU hDeleteMenu = 0;
    HMENU hPasteTableMenu = 0;
    HMENU hPasteMenu = 0;

    if( bInTable )
    {
        // After using this for awhile, it seems hard to right click to 
        //  get table properties, so lets always include both Table and Cell items
        cmPopup.AppendMenu(uState, ID_PROPS_TABLE, szLoadString(IDS_POPUP_TABLE_PROPS));
        cmPopup.AppendMenu(uState, ID_PROPS_TABLE_CELL, szLoadString(IDS_POPUP_TABLE_CELL_PROPS));
        ED_MergeType MergeType = EDT_GetMergeTableCellsType(pMWContext);
        if( MergeType != ED_MERGE_NONE )
        {
            cmPopup.AppendMenu(MF_ENABLED, ID_MERGE_TABLE_CELLS, 
                               szLoadString(MergeType == ED_MERGE_NEXT_CELL ? IDS_MERGE_NEXT_CELL : IDS_MERGE_SELECTED_CELLS) );
        }
// TODO: IMPLEMENT SPLIT CELL
        if( EDT_CanSplitTableCell(pMWContext) )
            cmPopup.AppendMenu(MF_ENABLED, ID_SPLIT_TABLE_CELL, szLoadString(IDS_SPLIT_TABLE_CELL));


        // We are in a cell, so we can do Select, Insert and Delete table commands,
        // Use menus shared with Frame's Table Menu
        hInsertTableMenu = ::LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDM_COMPOSER_TABLE_INSERTMENU));
        hDeleteMenu = ::LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDM_COMPOSER_TABLE_DELETEMENU));
        hSelectMenu = ::LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDM_COMPOSER_TABLE_SELECTMENU));
        if( hInsertTableMenu )
            cmPopup.AppendMenu(MF_POPUP, (UINT)hInsertTableMenu, szLoadString(IDS_SUBMENU_INSERT_TABLE));
        if( hDeleteMenu )
            cmPopup.AppendMenu(MF_POPUP, (UINT)hDeleteMenu, szLoadString(IDS_SUBMENU_DELETE_TABLE));
        if( hSelectMenu )
            cmPopup.AppendMenu(MF_POPUP, (UINT)hSelectMenu, szLoadString(IDS_SUBMENU_SELECT_TABLE));
        
        if( bCanPasteTableInTable )
        {
            hPasteTableMenu = ::LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDM_COMPOSER_TABLE_PASTEMENU));
            if( hPasteTableMenu )
            {
                // If cells are selected, replace last item with "Replace Selected Cells"
                if( bSelectTableOrCell )
                {
                    ::ModifyMenu(hPasteTableMenu, ID_PASTE_TABLE_REPLACE, MF_BYCOMMAND | MF_STRING, 
                                 ID_PASTE_TABLE_REPLACE,
                                 szLoadString(IDS_REPLACE_SELECTED_CELLS));
                }
                cmPopup.AppendMenu(MF_POPUP, (UINT)hPasteTableMenu, szLoadString(IDS_SUBMENU_PASTE_TABLE));
            }
        }

        cmPopup.AppendMenu(MF_SEPARATOR);
    }

    // Link-related items:

    BOOL bCanSetHREF = EDT_CanSetHREF(pMWContext);
    BOOL bLinkPopup = FALSE;

	if( (type == ED_ELEMENT_TEXT || 
	     type == ED_ELEMENT_SELECTION) &&
	     bIsLink)
    {
        // Open link into a browse window
    	cmPopup.AppendMenu(uMailtoState, ID_POPUP_LOADLINKNEWWINDOW, csEntry);

        // Jump to internal Target or open link into an edit window
        cmPopup.AppendMenu(uMailtoState, ID_POPUP_EDIT_LINK,
    	                   szLoadString(EDT_IsInternalLink(pMWContext, pLinkURL) ? 
                                            IDS_EDIT_JUMP_TARGET : IDS_POPUP_EDIT_LINK));
        bLinkPopup = TRUE;
    }
    if ( bCanSetHREF && bIsSelected  && bNoLinks )
    {
        cmPopup.AppendMenu(MF_ENABLED, ID_MAKE_LINK, szLoadString(IDS_POPUP_CREATE_LINK));
        bLinkPopup = TRUE;
    }
	if( (type == ED_ELEMENT_TEXT ||
	     type == ED_ELEMENT_SELECTION) )
    {
        if( bOneLink )
        {
            // We are in an existing link with TEXT, not image
            cmPopup.AppendMenu(uState, ID_POPUP_ADDLINK2BOOKMARKS, 
                               szLoadString(IDS_POPUP_ADDLINK2BOOKMARKS_EDT));
            cmPopup.AppendMenu(uState, ID_POPUP_COPYLINKCLIPBOARD,
                               szLoadString(IDS_POPUP_COPYLINKCLIPBOARD_EDT));
            // "Remove Link" for single text link
            cmPopup.AppendMenu(uState, ID_REMOVE_LINKS,
                               szLoadString(IDS_REMOVE_LINK));
            bLinkPopup = TRUE;
        } else if( bMayHaveManyLinks )
        {
            // "Remove All Links" for mixed selection
            cmPopup.AppendMenu(uState, ID_REMOVE_LINKS,
                               szLoadString(IDS_REMOVE_ALL_LINKS));
            bLinkPopup = TRUE;
        } else if( type == ED_ELEMENT_IMAGE )
        {
                // "Remove Link" for single image link
            cmPopup.AppendMenu(uState, ID_REMOVE_LINKS,
                               szLoadString(IDS_REMOVE_LINK));
        }
    }
    if(bLinkPopup)
    {    
        // End of Link stuff
        cmPopup.AppendMenu(MF_SEPARATOR);
    }

    // Image items:
    if ( type == ED_ELEMENT_IMAGE )
    {
        char *pImageEditor = NULL;
		PREF_CopyCharPref("editor.image_editor",&pImageEditor);
	    
        // Add "Edit Image..." if editor was designated
	    if( pImageEditor && XP_STRLEN(pImageEditor) > 0 )
        {
    	    cmPopup.AppendMenu(MF_ENABLED, ID_POPUP_EDIT_IMAGE,
    	                       szLoadString(IDS_EDIT_IMAGE));
        }
        // "Save image as..."
	    cmPopup.AppendMenu(MF_ENABLED, ID_POPUP_SAVEIMAGEAS,
	                       szLoadString(IDS_EDIT_SAVE_IMAGE));

        // We don't support background image in the Mail compose window
        if( !pMWContext->bIsComposeWindow )
        {
            // Make the selected image into the background of entire page
            cmPopup.AppendMenu(MF_ENABLED, ID_MAKE_IMAGE_BACKGROUND,
                               szLoadString(IDS_POPUP_MAKE_IMAGE_BACKGROUND));
        }
        // Add separator from copy/cut/paste items
        // It will be removed if none of those are used
        cmPopup.AppendMenu(MF_SEPARATOR);
		XP_FREEIF(pImageEditor);
    }

    if ( bIsSelected )
    {
        cmPopup.AppendMenu(MF_ENABLED, ID_EDIT_CUT, szLoadString(IDS_EDIT_CUT));
	    cmPopup.AppendMenu(MF_ENABLED, ID_EDIT_COPY, szLoadString(IDS_EDIT_COPY));
    }
    else
    {
        hInsertMenu = ::LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDM_COMPOSER_INSERTMENU));
        if( hInsertMenu )
        {
            cmPopup.AppendMenu(MF_POPUP, (UINT)hInsertMenu, szLoadString(IDS_POPUP_INSERT));
  // Should we do this? If we don't then insert Table is on 2 submenus
            // Delete the "Table" item -- its already in the Table Insert submenu
  //          if( hInsertTableMenu )
  //              ::DeleteMenu(hInsertMenu, ID_INSERT_TABLE, MF_BYCOMMAND);
        }
    }

    if( bHaveText && bHaveImage )
    {
        if( bCanPasteTableInTable || !bHaveTable )
        {
    	    // Since the "Table" item is handled in table submen or there's no table available,
            //  there's no point in a 2-item submenu,
            //  just add separate Text and Image paste items 
            cmPopup.AppendMenu(MF_ENABLED, ID_PASTE_TEXT, szLoadString(IDS_EDIT_PASTE_TEXT));
    	    cmPopup.AppendMenu(MF_ENABLED, ID_PASTE_IMAGE, szLoadString(IDS_EDIT_PASTE_IMAGE));
        }
        else
        {
            // Caret is not inside a table, so we can only paste
            // This submenu lets user choose between Table, Text and Image versions
            //   (most like spreadsheet-style data)
            hPasteMenu = ::LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDM_COMPOSER_PASTEMENU));
            if( hPasteMenu )
                cmPopup.AppendMenu(MF_POPUP, (UINT)hPasteMenu, szLoadString(IDS_EDIT_PASTE));
        }
    }
    else if( bCanPaste && !bCanPasteTableInTable ) // All table items we can paste are covered above
    {
        nID = bHaveLink ? IDS_EDIT_PASTE_LINK : IDS_EDIT_PASTE;
	    cmPopup.AppendMenu(MF_ENABLED, ID_EDIT_PASTE, szLoadString(nID));
    }
    if( EDT_CanPasteStyle(pMWContext) )
    {
	    cmPopup.AppendMenu(MF_ENABLED, ID_PASTE_CHARACTER_STYLE, 
                           szLoadString(IDS_PASTE_CHARACTER_SYLE_POPUP));
    }
    // Remove last item if its a separator
    int iLastItem = cmPopup.GetMenuItemCount() - 1;
    if( iLastItem > 0 && 0 == cmPopup.GetMenuItemID(iLastItem) )
    {
        cmPopup.RemoveMenu(iLastItem, MF_BYPOSITION);
    }

	//	Track the popup now.
	ClientToScreen(&cpPoint);
    
    // This flag needed to prevent mouse-over message 
    //    to CWinCX from changing cursor (menu should change it to default arrow)
    GetContext()->m_bInPopupMenu = TRUE;

    cmPopup.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, cpPoint.x, cpPoint.y, 
                           GetParentFrame(), NULL);

    GetContext()->m_bInPopupMenu = FALSE;

    if( hSelectMenu) ::DestroyMenu(hSelectMenu);
    if( hInsertTableMenu) ::DestroyMenu(hInsertTableMenu);
    if( hDeleteMenu) ::DestroyMenu(hDeleteMenu);
    if( hPasteTableMenu) ::DestroyMenu(hPasteTableMenu);
    if( hInsertMenu) ::DestroyMenu(hInsertMenu);
    if( hPasteMenu) ::DestroyMenu(hPasteMenu);
    if( pCharData ) EDT_FreeCharacterData(pCharData);
}

////////////////////////////////////////////////////////////////////////////
// This is called when loading images
// Does not show up if image is already in cache or local file,
//   needed primarily for remote file loading
void FE_ImageLoadDialog( MWContext *pMWContext )
{
    if ( !pMWContext ){
        return;
    }

    CNetscapeEditView* pView = (CNetscapeEditView*)WINCX(pMWContext)->GetView();
    // Initialize start time
    if ( pView->m_nLoadingImageCount == 0) {
        pView->m_ctStartLoadingImageTime = CTime::GetCurrentTime();
	
        // Disable clicking during image loading
        // CAN'T DO THIS? Seems to interfere with image loading sometimes.
        pMWContext->waitingMode = TRUE;

        // Start a timer to pause 3 seconds before 
        //  poping up dialog
        ::SetTimer(PANECX(pMWContext)->GetPane(), 
                    FEED_IMAGE_LOAD_PAUSE, 3000, NULL);
    }
    pView->m_nLoadingImageCount++;
}

void FE_ImageLoadDialogDestroy( MWContext *pMWContext )
{
    if ( !pMWContext ){
        return;
    }
    CNetscapeEditView* pView = (CNetscapeEditView*)WINCX(pMWContext)->GetView();
    if ( pView->m_nLoadingImageCount ){
        pView->m_nLoadingImageCount--;

        if ( pView->m_nLoadingImageCount == 0 && 
             pView->m_pLoadingImageDlg )
        {
            // We're done loading - kill dialog if it exists
            if( ::IsWindow(pView->m_pLoadingImageDlg->m_hWnd) ){
                pView->m_pLoadingImageDlg->DestroyWindow();
            }
            pView->m_pLoadingImageDlg = NULL;
            // Enable clicking
            pMWContext->waitingMode = FALSE;
            ::KillTimer(PANECX(pMWContext)->GetPane(), FEED_IMAGE_LOAD_PAUSE);
        }
    }
}

void FE_FinishedSave( MWContext *pMWContext, int status, 
                      char *pURL, int m_iCurFile )
{
    CNetscapeView* pView = (CNetscapeView*)WINCX(pMWContext)->GetView();
    if( !pView )
        return;

    if( EDT_IS_EDITOR(pMWContext)){ 
        CNetscapeEditView * pEditView = (CNetscapeEditView*)pView;

        if (status == ED_ERROR_NONE || status == ED_ERROR_FILE_EXISTS) {
            if( pEditView->m_pImagePage ){
                pEditView->m_pImagePage->SetImageFileSaved(pURL, m_iCurFile);
            } else if( pEditView->m_pColorPage ){
                pEditView->m_pColorPage->SetImageFileSaved(pURL);
            }

        }
    }

#if 0
        // *** TODO: We need to inform SiteManager about all URLs published
        // Tell site manager when each file is published
        if ( bSiteManagerIsActive &&
             pView->m_pSaveFileDlg &&
             ::IsWindow(pView->m_pSaveFileDlg->m_hWnd) &&
             pView->m_pSaveFileDlg->m_bUpload )
        {
            pITalkSMClient->SavedURL(pURL);
        }
#endif
}

// Note: This is used by Navigator's HTP UPLOAD as well as Composer's file saving
//  DON'T ASSUME ANY EDITOR FUNCTIONALITY!
// Dialog to give feedback and allow canceling, overwrite protection
//   when downloading remote files 
//
void FE_SaveDialogCreate( MWContext *pMWContext, int iFileCount, ED_SaveDialogType saveType  )
{
    CNetscapeView* pView = (CNetscapeView*)WINCX(pMWContext)->GetView();
    if( !pView )
        return;

    // Get the active child of the current view,
    //  since we might be called while another dialog box is active
    CWnd *pParent = pView->GetFrame()->GetFrameWnd()->GetLastActivePopup();

    pView->m_pSaveFileDlg = new CSaveFileDlg(pParent, pMWContext, iFileCount, saveType);
}

void FE_SaveDialogSetFilename( MWContext *pMWContext, char *pFilename )
{
    CNetscapeEditView* pView = (CNetscapeEditView*)WINCX(pMWContext)->GetView();
    
    // Write the filename into the dialog, but check if it still exists first
    if ( pView && pView->m_pSaveFileDlg && ::IsWindow(pView->m_pSaveFileDlg->m_hWnd) ){
        pView->m_pSaveFileDlg->StartFileSave(pFilename);
    }
}

void FE_SaveDialogDestroy( MWContext *pMWContext, int status, char *pFileURL )
{
    CNetscapeEditView* pView = (CNetscapeEditView*)WINCX(pMWContext)->GetView();
    if( !pView ){
        return;
    }

    if( EDT_IS_EDITOR(pMWContext) ) {
        // Set status value -- Asynchronous start of SaveFile is waiting for this
        ((CNetscapeEditView*)pView)->SetFileSaveStatus((ED_FileError)status);
    }

    int  iCount = 0;
    BOOL bUpload =  FALSE;
    
    if( pView->m_pSaveFileDlg ) {

        // This gets destroyed if parent wasn't the view
        //  (e.g., DocProperties dialog)
        if( ::IsWindow(pView->m_pSaveFileDlg->m_hWnd) ){
            iCount = pView->m_pSaveFileDlg->m_iFileCount;
            bUpload = pView->m_pSaveFileDlg->m_bUpload;
            pView->m_pSaveFileDlg->DestroyWindow();
        }
        pView->m_pSaveFileDlg = NULL;
    }
#ifdef XP_WIN32
    if( !bUpload && status == 0 || status == MK_DATA_LOADED ){
        // Always tell Site Manager about any local files saved
        // Tell SiteManager we saved a file
        if( bSiteMgrIsActive &&  status == ED_ERROR_NONE ){
            pITalkSMClient->SavedURL(pFileURL);
        }
    }
#endif
}

ED_SaveOption FE_SaveFileExistsDialog( MWContext *pMWContext, char* pFilename )
{

    CNetscapeEditView* pView = (CNetscapeEditView*)WINCX(pMWContext)->GetView();
    ASSERT(pView && pView->GetFrame() && EDT_IS_EDITOR(pMWContext));

    CSaveFileOverwriteDlg dlg( pView->GetFrame()->GetFrameWnd()->GetLastActivePopup(), pFilename);
    dlg.DoModal();
    return dlg.m_Result;
}

// NO LONGER USED NEED TO REMOVE FROM fe_proto.h and all front ends at same time.
Bool FE_SaveErrorContinueDialog( MWContext *pMWContext, char* pFileName, 
                                 ED_FileError error )
{
    return FALSE;
}

/* 
 * LTNOTE: this should probably be introduced at the usual fifty layers....
*/
// This doesn't seem to be called at all?
#if 0
void FE_ClearBackgroundImage( MWContext *pMWContext ){
    CNetscapeEditView* pView = (CNetscapeEditView*)WINCX(pMWContext)->GetView();
    pView->GetContext()->m_pBackdropLOImage = NULL;
    pView->GetContext()->m_pBackdropILImage = NULL;
}
#endif

PUBLIC Bool FE_EditorPrefConvertFileCaseOnWrite( ){
    return TRUE;
}

void CNetscapeEditView::OnTimer(UINT nIDEvent)
{
    CNetscapeView::OnTimer(nIDEvent);
    if ( nIDEvent == FEED_IMAGE_LOAD_PAUSE &&
         GET_MWCONTEXT != NULL && 
         m_nLoadingImageCount > 0 &&
         m_pLoadingImageDlg == NULL )      // Dialog must not already exist
    {
        // Don't need timer any more
        ::KillTimer(this->m_hWnd, FEED_IMAGE_LOAD_PAUSE);

        // Create our dialog
        m_pLoadingImageDlg = new CLoadingImageDlg(GET_DLG_PARENT(this), GET_MWCONTEXT);
    }
}

// This will insert a new image if current element is not an image
void CNetscapeEditView::OnImagePropDlg()
{
    EDT_ImageData* pData = NULL;
    
    if (EDT_GetCurrentElementType(GET_MWCONTEXT) == ED_ELEMENT_IMAGE){
        pData = EDT_GetImageData(GET_MWCONTEXT);
    }
}

/////////////////////////////////////////////////
void CNetscapeEditView::OnInsertHRule()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( pMWContext ){
        // Edit properties of existing HRule if it is the selected object
        if( ED_ELEMENT_HRULE == EDT_GetCurrentElementType(pMWContext) ) {
            OnHRuleProperties();
            return;
        }
        // Hrule is so simple, just use default values
        //   instead of bringing up properties dialog
        EDT_HorizRuleData *pData = EDT_NewHorizRuleData();
        if ( pData ){
            // Unselect so we insert after after the object
            if( NonLinkObjectIsSelected(pMWContext) )
                EDT_ClearSelection(pMWContext);
            
            // Get defaults from preferences
			int32 iWidth,iHeight;

			PREF_GetIntPref("editor.hrule.height",&iHeight);
			PREF_GetIntPref("editor.hrule.width",&iWidth);
			PREF_GetBoolPref("editor.hrule.width_percent",&(pData->bWidthPercent));
			// Confusing! Pref and UI use "Shading" but tag param is NOSHADE
            BOOL bShading;
            PREF_GetBoolPref("editor.hrule.shading",&bShading);
            pData->bNoShade = !bShading;

			pData->size = iHeight;
			pData->iWidth = iWidth;

			int32 align;
			PREF_GetIntPref("editor.hrule.align",&align);
            
			if( align == ED_ALIGN_RIGHT ){
                pData->align = ED_ALIGN_RIGHT;
            } else if( align == ED_ALIGN_LEFT  ){
                pData->align = ED_ALIGN_LEFT;
            } else {
                pData->align = ED_ALIGN_CENTER;
            }

            EDT_InsertHorizRule(pMWContext, pData);
	        EDT_FreeHorizRuleData(pData);
        }
    }
#if 0
    // Save this in case we want to popu properties dialog after
    //   inserting new HRULE - maybe a user preference?
    // All the work is done by the dialog
    // supply no data to insert new rule
    CHRuleDlg dlg(GET_DLG_PARENT(this), pMWContext);
    dlg.DoModal();
#endif
}

// This will insert a new HRule if current element is not a HRule
void CNetscapeEditView::OnHRuleProperties()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( !pMWContext )
        return;
    EDT_HorizRuleData* pData = NULL;
    if (EDT_GetCurrentElementType(pMWContext) == ED_ELEMENT_HRULE){
        pData = EDT_GetHorizRuleData(pMWContext);
    }
    CHRuleDlg dlg(GET_DLG_PARENT(this), pMWContext, pData);
    dlg.DoModal();

	// If we automatically caused a selection, then clear it now
	//  so user doesn't accidentally delete our selected object.
	if( m_bAutoSelectObject && EDT_IsSelected(pMWContext) ){
		EDT_ClearSelection(pMWContext);
        m_bAutoSelectObject = FALSE;
    }
}

void CNetscapeEditView::OnUpdateHRuleProperties(CCmdUI* pCmdUI)
{
    // In menu, properties does not allow insert,
    //  so we check object type
    // Toolbar button does Insert or Change existing properties
    if (pCmdUI->m_pMenu ) {
        pCmdUI->Enable(CAN_INTERACT && 
                       EDT_GetCurrentElementType(GET_MWCONTEXT) == ED_ELEMENT_HRULE);
    } else {
        pCmdUI->Enable(CAN_INTERACT);
    }
}

////////////////////////////////////////////////////
// Tabbed Dialog Property UI:

#define PROPS_PAGE_TEXT_OR_IMAGE  0
#define PROPS_PAGE_LINK           1
#define PROPS_PAGE_PARA           2

void CNetscapeEditView::OnUpdatePropsLocal(CCmdUI* pCmdUI)
{
    // We only have this on the menu
    if( !pCmdUI->m_pMenu )
        return;

    MWContext *pMWContext = GET_MWCONTEXT;
    if( !pMWContext )
        return;

    UINT nID = 0;
    ED_ElementType ElementType = ED_ELEMENT_NONE;

    // Check for the last table object selected
    ED_HitType iHit =  EDT_GetSelectedTableElement(pMWContext, NULL);
    if( iHit )
    {
        switch( iHit )
        {
            case ED_HIT_SEL_ROW:
                nID = IDS_POPUP_TABLE_PROPS;
                break;
            case ED_HIT_SEL_COL:
                nID = IDS_POPUP_TABLE_PROPS;
                break;
            case ED_HIT_SEL_CELL:
                nID = IDS_POPUP_TABLE_PROPS;
                break;
        }
    } else {
        ElementType = EDT_GetCurrentElementType(pMWContext);
        switch ( ElementType ){
            case ED_ELEMENT_IMAGE:
                nID = IDS_PROPS_IMAGE;
                break;
            case ED_ELEMENT_HRULE:
                nID = IDS_PROPS_HRULE;
                break;
            case ED_ELEMENT_TARGET:
                nID = IDS_PROPS_TARGET;
                break;
            case ED_ELEMENT_UNKNOWN_TAG:
                nID = IDS_PROPS_TAG;
                break;
            default:
                nID = IDS_PROPS_CHARACTER;
                break;
        }
    }
    if( nID )
    {
        char pMenu[256];
        strcpy(pMenu, szLoadString(nID));
        strcat(pMenu, szLoadString(IDS_ALT_ENTER));
        pCmdUI->m_pMenu->ModifyMenu(ID_PROPS_LOCAL, MF_BYCOMMAND | MF_STRING, ID_PROPS_LOCAL,
                                    pMenu);
    }
    // Use this to suppress char props when uncertain,
    pCmdUI->Enable(CAN_INTERACT);

    // Enable/Disable the "Make image the background" menu item        
    pCmdUI->m_pMenu->EnableMenuItem(ID_MAKE_IMAGE_BACKGROUND,
                                    ElementType == ED_ELEMENT_IMAGE ? MF_ENABLED : MF_GRAYED);

}

// Call this from toolbar or popup menu (right mouse button)
void CNetscapeEditView::OnLocalProperties()
{
    MWContext *pMWContext = GET_MWCONTEXT;
    if( !pMWContext )
        return;

    // Check for last table hit area
    ED_HitType iHit =  EDT_GetTableHitRegion(pMWContext, -1, -1, NULL, FALSE);
    if( iHit != ED_HIT_NONE )
    {
        int iStartPage = -1;
        switch( iHit )
        {
            case ED_HIT_SEL_TABLE:
            case ED_HIT_ADD_ROWS:
            case ED_HIT_ADD_COLS:
            case ED_HIT_SIZE_TABLE_WIDTH:
                // Allow any of 4 corners to select table properties
                iStartPage = 0;
                break;
            default:
                iStartPage = 1;   // The Cell Properties page
                break;
        }
        if( iStartPage >= 0 )
        {
            OnTableProperties(iStartPage);
            return;
        }
    }

    ED_ElementType type = EDT_GetCurrentElementType(pMWContext);
    switch( type ) {
        case ED_ELEMENT_IMAGE:
            OnImageProperties();
            break;
        case ED_ELEMENT_HRULE:
            OnHRuleProperties();
            break;
        case ED_ELEMENT_TARGET:
            OnTargetProperties();
            break;
        case ED_ELEMENT_UNKNOWN_TAG:
            OnTagProperties();
            break;
        default:
            // Here if element = text or unknown (multiple/selected)
            if(EDT_CanSetHREF(GET_MWCONTEXT)){
                // Do link properties if we are in a link
                CString m_csHref = EDT_GetHREF(pMWContext);
                if(!m_csHref.IsEmpty()){
                    DoProperties(PROPS_PAGE_LINK);
                    break;
                }
            }
            // If here, we are in text or unknown selection
            OnTextProperties();
            break;
    }
}

// Central processing for common property page system
//
void CNetscapeEditView::DoProperties(int iStartPage, UINT nIDFirstTab)
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if (pMWContext == NULL || !EDT_IS_EDITOR(pMWContext)) {
        return;
    }
    BOOL   bImage = FALSE;
    BOOL   bInsertLink = FALSE;
    BOOL   bInsertImage = FALSE;
    char **ppLinkImage = NULL;
    ED_ElementType type = EDT_GetCurrentElementType(pMWContext);

    if( nIDFirstTab > 0 )
    {
        bImage = (nIDFirstTab == IDD_PAGE_IMAGE);
    } else {
        bImage = (type == ED_ELEMENT_IMAGE);
    }
    // Figure out context: what is object we are in and/or selected

    // Get or create object data
    EDT_ImageData     *pImageData = NULL;
    EDT_HREFData      *pHrefData = NULL;
    EDT_CharacterData *pCharData = NULL;
    BOOL               bMayHaveOtherLinks = FALSE;
    
    if(bImage){
        if( type == ED_ELEMENT_IMAGE ) {
            pImageData = EDT_GetImageData(pMWContext);
        }
        if(pImageData == NULL){
            // Create a new structure to insert an image
            pImageData = EDT_NewImageData();
            if(pImageData == NULL){
                // Something is really wacked!
                return;
            }
            bInsertImage = TRUE;
        }
        //GetImageData is not filling this in yet,
        //   it will work when it does...
        // Set pointer so Link Dialog uses the data from the Image
        pHrefData = pImageData->pHREFData;
        // This will be used by Link Dialog to show the Image filename
        // If it is changed by Image Dialog, Link will update correctly
        ppLinkImage = &pImageData->pSrc;
    } else {
        // First page will be Character page
        pCharData = EDT_GetCharacterData(pMWContext);
        if( pCharData ){
            // If HREF flag is indeterminate
            // (i.e., the mask bit is not set),
            //  then there may be other links we can remove
            bMayHaveOtherLinks = (pCharData->mask & TF_HREF) == 0;
        
            // By design, we NEVER change HREF data in Character dialog,
            //   so clear HREF mask bit so we don't change it.
            //   bMayHaveOtherLinks will allow Link page to remove 
            //   link attributes from the text selection via Remove Link button.
            pCharData->mask &= ~TF_HREF;
        } else {
            // We should never get here!
            // We should identify the selected object,
            //    or we are left with text or mixed selection
            return;
        }
    }
    
    // This will, of course, be true when we don't have an image
    if(pHrefData == NULL){
        // Get HREF data from current element
        pHrefData = EDT_GetHREFData(pMWContext);
        if(pHrefData && pHrefData->pURL == NULL){
            // No current link -- we will insert one
            bInsertLink = TRUE;
        }
    }
    if(pHrefData == NULL){
        // Should never happen unless memory allocation failed
        return;
    }

    // Use other HREF data if GetImageData didn't supply any
    if(bImage && pImageData->pHREFData == NULL){
        pImageData->pHREFData = pHrefData;
        ppLinkImage = &pImageData->pSrc;
    }

    // Create the sheet with generic "Properties" caption
    // TODO: Should we bother to change this on context?
    CNetscapePropertySheet PropsDlg(szLoadString(CASTUINT(bImage ? IDS_IMAGE_PROPS_CAPTION : IDS_TEXT_PROPS_CAPTION)),
                                    GET_DLG_PARENT(this), 0, pMWContext, TRUE); // Use the Apply Button

    CCharacterPage * CharacterPage = NULL;
    m_pImagePage = NULL;

    // This will change resource hInstance to EditorXX.dll (in its constructor)
    // We pass this into our property page, which resets back to EXE's resources later
    CEditorResourceSwitcher ResourceSwitcher;

    // These pages are always used
    CParagraphPage * ParagraphPage = new CParagraphPage(this, pMWContext, &ResourceSwitcher);
    CLinkPage * LinkPage =  new CLinkPage(this, pMWContext, &ResourceSwitcher, pHrefData,
                                          bInsertLink, bMayHaveOtherLinks, ppLinkImage);


    if(bImage){
        m_pImagePage = new CImagePage(this, pMWContext, &ResourceSwitcher, pImageData, bInsertImage);
        PropsDlg.AddPage(m_pImagePage);
    } else {
        CharacterPage = new CCharacterPage(this, pMWContext, &ResourceSwitcher, pCharData);
        PropsDlg.AddPage(CharacterPage);
    }

    PropsDlg.AddPage(LinkPage);
    PropsDlg.AddPage(ParagraphPage);

    int32 iPrefInt;
    
    if ( iStartPage < 0 ) {
        // Figure out what page to start on
        if(bImage){
            iStartPage = 0;
        } else {
            // Get last page depending on preferences
            // NOTE: Only choice now is for Text properites
			PREF_GetIntPref("editor.last_text_page", &iPrefInt);
            iStartPage = (int)iPrefInt;
        }
    }
    // Must initially set page to 0 so dialog is sized relative to Image page
    PropsDlg.SetCurrentPage(0);
    PropsDlg.SetCurrentPage(iStartPage);
	
	if(PropsDlg.DoModal() == IDOK) {
        int iCurrentPage = PropsDlg.GetCurrentPage();
        if( iCurrentPage <= 2 ){
            // Save current page if NOT Link page
            // Somewhat arbitrary, but seems to 
            //  make more sense with testing
            if(iCurrentPage != PROPS_PAGE_LINK){
				PREF_GetIntPref("editor.last_text_page", &iPrefInt);
                iCurrentPage = (int)iPrefInt;
            }
        }
    }
	// If we automatically caused a selection, then clear it now
	//  so user doesn't accidentally delete our selected object.
	if( m_bAutoSelectObject && EDT_IsSelected(pMWContext) ){
		EDT_ClearSelection(pMWContext);
        m_bAutoSelectObject = FALSE;
    }
    if(pImageData) {
        EDT_FreeImageData(pImageData);
        // pHrefData is a member of pImageData and is freed there
    } else if(pHrefData) {
        // We didn't have an Image page -- free link data
        EDT_FreeHREFData(pHrefData);
    }
    if(pCharData) EDT_FreeCharacterData(pCharData);

    delete ParagraphPage;
    delete LinkPage;
    if(CharacterPage){
        delete CharacterPage;
    } else if(m_pImagePage){
        delete m_pImagePage;
        m_pImagePage = NULL;
    }
}

void CNetscapeEditView::OnImageProperties()
{
    DoProperties(PROPS_PAGE_TEXT_OR_IMAGE, IDD_PAGE_IMAGE);
}

void CNetscapeEditView::OnLinkProperties()
{
    if ( GET_MWCONTEXT ) {
        // Set first page to Character or Image depending
        //   on the current object
        DoProperties(PROPS_PAGE_LINK,
                     EDT_GetCurrentElementType(GET_MWCONTEXT) == ED_ELEMENT_IMAGE ? 
                         IDD_PAGE_IMAGE : IDD_PAGE_CHARACTER);
    }
}

void CNetscapeEditView::OnParagraphProperties()
{
    DoProperties(PROPS_PAGE_PARA);
}

// 10/28/97 Not called currently! (Resource ID = ID_PROPS_TEXT)
void CNetscapeEditView::OnCharacterProperties()
{
    DoProperties(PROPS_PAGE_TEXT_OR_IMAGE, IDD_PROPS_CHARACTER);
}

// Same as Character or Paragraph,
// but start page is same as last use
void CNetscapeEditView::OnTextProperties()
{
    DoProperties(-1, IDD_PROPS_CHARACTER);
}


void CNetscapeEditView::OnUpdateImageProperties(CCmdUI* pCmdUI)
{
    // In menu, properties does not allow insert,
    //  so we check object type
    // Toolbar button does Insert or Change existing properties
    if (pCmdUI->m_pMenu ) {
        pCmdUI->Enable(CAN_INTERACT && 
                       EDT_GetCurrentElementType(GET_MWCONTEXT) == ED_ELEMENT_IMAGE);
    } else {
        pCmdUI->Enable(CAN_INTERACT);
    }
}

void CNetscapeEditView::OnUpdateLinkProperties(CCmdUI* pCmdUI)
{
    // Must be on an existing link:
    pCmdUI->Enable( CAN_INTERACT && EDT_GetHREF(GET_MWCONTEXT) );
}

// *** Inserting the objects used in common properties

void CNetscapeEditView::OnInsertImage()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( pMWContext ){
        // Unselect so we insert after after the object
        if( NonLinkObjectIsSelected(pMWContext) )
            EDT_ClearSelection(pMWContext);
        OnImageProperties();
    }
}

void CNetscapeEditView::OnMakeLink()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    // Force saving a new or remote document
    if( pMWContext ) {
        // Unselect a single object that can't be a link
        if( NonLinkObjectIsSelected(pMWContext) ) {
            EDT_ClearSelection(pMWContext);
        }
        OnLinkProperties();
    }
}

void CNetscapeEditView::OnInsertLink()
{
    OnMakeLink();
}

void CNetscapeEditView::OnUpdateInsertLink(CCmdUI* pCmdUI)
{
    MWContext * pMWContext = GET_MWCONTEXT;
    // To reduce confusion, try to not allow inserting a link
    //  inside of an exisiting link.
    // But if selected, we can't know if selection includes an existing link
    // Action will do a "modify link" rather than insert new link
    pCmdUI->Enable( CAN_INTERACT &&
                    (EDT_IsSelected(pMWContext) ||
                     !EDT_CanSetHREF(pMWContext)) );
}

void CNetscapeEditView::OnRemoveLinks()
{
    // Remove a single link or all links
    //  within selected region
    if( EDT_CanSetHREF(GET_MWCONTEXT) ){
        EDT_SetHREF(GET_MWCONTEXT, NULL);
    }
}

void CNetscapeEditView::OnUpdateRemoveLinks(CCmdUI* pCmdUI)
{
    if( !CAN_INTERACT ) {
        pCmdUI->Enable(FALSE);
        return;
    }
    MWContext * pMWContext = GET_MWCONTEXT;

    ED_ElementType type = EDT_GetCurrentElementType(pMWContext);

    pCmdUI->Enable( (type == ED_ELEMENT_TEXT || type == ED_ELEMENT_SELECTION || 
                     type == ED_ELEMENT_IMAGE || type >= ED_ELEMENT_TABLE) &&
                    EDT_CanSetHREF(GET_MWCONTEXT) );
}

void CNetscapeEditView::OnSetImageAsBackground()
{
    EDT_SetImageAsBackground(GET_MWCONTEXT);
}

// End of common property page system
//
/////////////////////////////////////////////////////
//
void CNetscapeEditView::OnDocColorProperties()
{
    OnDocProperties(1);
}

void CNetscapeEditView::OnDocumentProperties()
{
    OnDocProperties(0);
}

void CNetscapeEditView::OnDocProperties(int iStartPage)
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if (pMWContext == NULL) {
        return;
    }
    EDT_PageData * pPageData = EDT_GetPageData(pMWContext);
    if (pPageData == NULL) {
        return;
    }
    CNetscapePropertySheet Properties( szLoadString(IDS_DOC_PROPS_CAPTION),
                                    GET_DLG_PARENT(this), 0, pMWContext, TRUE); // Use the Apply Button

    // This will change resource hInstance to EditorXX.dll (in its constructor)
    // We pass this into our property page, which resets back to EXE's resources later
    CEditorResourceSwitcher ResourceSwitcher;

    CDocInfoPage * pDocInfoPage = NULL;
    CDocMetaPage * pDocMetaPage = NULL;
    if( !pMWContext->bIsComposeWindow ){
        pDocInfoPage = new CDocInfoPage(this, pMWContext, &ResourceSwitcher, pPageData);
    }
    // 2nd param is nIDCaption: use dialog caption, 3rd is initial focus ID
    // 6th param tells dialog to behave like Doc properties, not preference dialog
    // Note that we keep pointer to this page so XP code can tell us
    //   what the background image is when it has changed
    m_pColorPage = new CDocColorPage(this, 0, 0, pMWContext, &ResourceSwitcher, pPageData);

    if( !pMWContext->bIsComposeWindow ){
        pDocMetaPage = new CDocMetaPage(this, pMWContext, &ResourceSwitcher);
        Properties.AddPage(pDocInfoPage);
    }

    Properties.AddPage(m_pColorPage);
    
    if( !pMWContext->bIsComposeWindow ){
        Properties.AddPage(pDocMetaPage);
    }
    
    // Set to page passed in or last page accessed
	int32 iPage=0;
    if( iStartPage >= 0 && iStartPage < 3 )
        iPage = iStartPage;
    else 
	    PREF_GetIntPref("editor.last_doc_page",&iPage);
    
    Properties.SetCurrentPage(CASTINT(iPage));

    Properties.DoModal();

    // Cleanup - (Must be done in reverse order of AddPage?)
    if( pDocMetaPage ){
        delete pDocMetaPage;
    }
    if( m_pColorPage ){
        delete m_pColorPage;
    }
    m_pColorPage = NULL;
    if( pDocInfoPage ){
        delete pDocInfoPage;
    }
    
    EDT_FreePageData(pPageData);

    // Save last page accessed
	PREF_SetIntPref("editor.last_doc_page",Properties.GetCurrentPage());
}

////////////////////////////////////////////////////
void CNetscapeEditView::OnFormatIndent()
{
    MWContext *pMWContext = GET_MWCONTEXT ;
    if ( pMWContext ){
        EDT_BeginBatchChanges(pMWContext);
        EDT_Indent(pMWContext);
        EDT_EndBatchChanges(pMWContext);
    }
}

void CNetscapeEditView::OnFormatOutdent()
{
    MWContext *pMWContext = GET_MWCONTEXT ;
    if ( pMWContext ){
        EDT_BeginBatchChanges(pMWContext);
        EDT_Outdent( GET_MWCONTEXT );
        EDT_EndBatchChanges(pMWContext);
    }
}

void CNetscapeEditView::OnRemoveList()
{
    // Repeat removing indent until last Unnumbered list is gone
    MWContext * pMWContext = GET_MWCONTEXT;
    if( ! pMWContext ){
        return;
    }
    EDT_RemoveList(pMWContext);
}

void CNetscapeEditView::OnUpdateRemoveList(CCmdUI* pCmdUI)
{
    EDT_ListData * pListData = EDT_GetListData(GET_MWCONTEXT);
    pCmdUI->Enable(CAN_INTERACT);
    if( pCmdUI->m_pMenu) {
        pCmdUI->SetCheck(!pListData);
    }
    if(pListData){
        EDT_FreeListData(pListData);
    }
}

void CNetscapeEditView::UpdateListMenuItem(CCmdUI* pCmdUI, TagType t)
{
    BOOL bIsList = EDT_GetToggleListState(GET_MWCONTEXT, t);

    if( pCmdUI->m_pMenu) {
        pCmdUI->SetCheck(bIsList);
    }
    pCmdUI->Enable(CAN_INTERACT);

    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController && CAN_INTERACT && !pCmdUI->m_pMenu && pController->GetCharacterBar() ) {
			(pController->GetCharacterBar())->SetCheck( pCmdUI->m_nID, bIsList );
    }
}

void CNetscapeEditView::OnAlignPopup()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( ! pMWContext ){
        return;
    }
    ED_ElementType type = EDT_GetCurrentElementType(pMWContext);
    ED_Alignment old_align = ED_ALIGN_DEFAULT;

    switch( type )
    {
        case ED_ELEMENT_HRULE:
        {
            EDT_HorizRuleData* pData = EDT_GetHorizRuleData(pMWContext);
            if ( pData ){
                old_align = pData->align;
            }
            break;
        }
        case ED_ELEMENT_TABLE:
            break;
        case ED_ELEMENT_ROW:
            break;
        case ED_ELEMENT_COL:
        case ED_ELEMENT_CELL:
            break;
        default:
            old_align = EDT_GetParagraphAlign(pMWContext);
            break;
    }
    ED_Alignment align = old_align;
    
    RECT rectCaller = {0,0, 0, 0};
	RECT newRectCaller = {0, 0, 0, 0 };
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
    if (pController) {
        CComboToolBar * pToolbar = pController->GetCharacterBar();
		CCommandToolbar *pCommandToolbar = pController->GetCNSToolbar();
       if( pToolbar ){
            pToolbar->GetButtonRect(ID_ALIGN_POPUP, &rectCaller);
        }
	   if(pCommandToolbar){
			pCommandToolbar->GetButtonRect(ID_ALIGN_POPUP, &rectCaller);
	   }


    }
    UINT nInitialID = 0;
    switch (old_align ){
        case ED_ALIGN_LEFT:
            nInitialID = ID_ALIGN_LEFT;
            break;
        case ED_ALIGN_CENTER:
        case ED_ALIGN_ABSCENTER:
            nInitialID = ID_ALIGN_CENTER;
            break;
        case ED_ALIGN_RIGHT:
            nInitialID = ID_ALIGN_RIGHT;
            break;
    }
        
    pTB = NULL;
    // Build dropdown toolbar for alignment buttons
    //   CommandID is sent to parent frame when pressed
    // Use this to include current state as a push-down button:
    if (GetEmbedded())
        pTB = new CDropdownToolbar(this, pMWContext, &rectCaller, ID_ALIGN_POPUP, nInitialID);
    else
        pTB = new CDropdownToolbar(GetParentFrame(), pMWContext, &rectCaller, ID_ALIGN_POPUP, nInitialID);
    if( pTB ){
        pTB->AddButton(IDB_HALIGN_LEFT, ID_ALIGN_LEFT);
        pTB->AddButton(IDB_HALIGN_CENTER, ID_ALIGN_CENTER);
        pTB->AddButton(IDB_HALIGN_RIGHT, ID_ALIGN_RIGHT);
        pTB->Show();
    }
}

void DoAlign(MWContext* pMWContext, ED_Alignment align)
{
    ED_ElementType type = EDT_GetCurrentElementType(pMWContext);

    switch ( type )
    {
        case ED_ELEMENT_HRULE:
        {
            EDT_HorizRuleData* pData = EDT_GetHorizRuleData(pMWContext);
            if ( pData ){
                pData->align = align;
                EDT_SetHorizRuleData(pMWContext, pData);
            }
            break;
        }
       default: // For Images, Text, or selection, this will do all:
            EDT_SetParagraphAlign( pMWContext, align );
            break;
    }
}

void CNetscapeEditView::OnUpdateAlignLeft(CCmdUI* pCmdUI)
{
    if( pCmdUI->m_pMenu ){
        // Get current alignment to set check next to menu items
        MWContext * pMWContext = GET_MWCONTEXT;
        if( pMWContext ){
            ED_ElementType type = EDT_GetCurrentElementType(pMWContext);
            EDT_HorizRuleData* pData = NULL;
            ED_Alignment align = ED_ALIGN_LEFT;

            if( type == ED_ELEMENT_HRULE ){
                pData = EDT_GetHorizRuleData(pMWContext);
                if ( pData ){
                    align = pData->align;
                }
            } else {
                align = EDT_GetParagraphAlign(pMWContext);
            }
            if( align == ED_ALIGN_ABSCENTER ){
                align = ED_ALIGN_CENTER;
            }
            if( align == ED_ALIGN_DEFAULT ){
                align = ED_ALIGN_LEFT;
            }

            pCmdUI->SetCheck(align == ED_ALIGN_LEFT ? 1 : 0);

            // Set check for center and right while we are here:
            pCmdUI->m_pMenu->CheckMenuItem(ID_ALIGN_CENTER, align == ED_ALIGN_CENTER ? MF_CHECKED : MF_UNCHECKED );
            pCmdUI->m_pMenu->CheckMenuItem(ID_ALIGN_RIGHT, align == ED_ALIGN_RIGHT ? MF_CHECKED : MF_UNCHECKED );
        }
    }
    pCmdUI->Enable(CAN_INTERACT);
}

void CNetscapeEditView::OnAlignLeft()
{
        
    DoAlign(GET_MWCONTEXT, ED_ALIGN_LEFT); 
}

void CNetscapeEditView::OnAlignRight()
{
    DoAlign(GET_MWCONTEXT, ED_ALIGN_RIGHT); 
}
void CNetscapeEditView::OnAlignCenter()
{                                
    DoAlign(GET_MWCONTEXT, ED_ALIGN_CENTER); 
}

void CNetscapeEditView::OnAlignTableLeft()
{
    EDT_SetTableAlign(GET_MWCONTEXT, ED_ALIGN_LEFT);
}

void CNetscapeEditView::OnAlignTableRight()
{
    EDT_SetTableAlign(GET_MWCONTEXT, ED_ALIGN_RIGHT);
}

void CNetscapeEditView::OnAlignTableCenter()
{
    EDT_SetTableAlign(GET_MWCONTEXT, ED_ALIGN_CENTER);
}

void CNetscapeEditView::OnInsertLineBreak()
{
    EDT_InsertBreak(GET_MWCONTEXT, ED_BREAK_NORMAL);
}

void CNetscapeEditView::OnInsertBreakLeft()
{
    EDT_InsertBreak(GET_MWCONTEXT, ED_BREAK_LEFT);
}

void CNetscapeEditView::OnInsertBreakRight()
{
    EDT_InsertBreak(GET_MWCONTEXT, ED_BREAK_RIGHT);
}

void CNetscapeEditView::OnInsertBreakBoth()
{
    EDT_InsertBreak(GET_MWCONTEXT, ED_BREAK_BOTH);
}

void CNetscapeEditView::OnUpdateInsertBreak(CCmdUI* pCmdUI)
{
    ED_ElementType type = EDT_GetCurrentElementType(GET_MWCONTEXT);
    pCmdUI->Enable(CAN_INTERACT &&
                   type == ED_ELEMENT_TEXT || type >= ED_ELEMENT_TABLE);
}

void CNetscapeEditView::OnSelectAll()
{
    EDT_SelectAll(GET_MWCONTEXT);
    // Trigger updating paragraph styles combo
    SetEditChanged();
}

void CNetscapeEditView::OnSelectTable()
{
    EDT_SelectTable(GET_MWCONTEXT);
    // Trigger updating paragraph styles combo
    SetEditChanged();
}
void CNetscapeEditView::OnSelectTableRow()
{
    EDT_SelectTableElement(GET_MWCONTEXT, 0,0, NULL, ED_HIT_SEL_ROW, FALSE, FALSE );
}

void CNetscapeEditView::OnSelectTableColumn()
{
    EDT_SelectTableElement(GET_MWCONTEXT, 0,0, NULL, ED_HIT_SEL_COL, FALSE, FALSE );
}

void CNetscapeEditView::OnSelectTableCell()
{
    EDT_SelectTableElement(GET_MWCONTEXT, 0,0, NULL, ED_HIT_SEL_CELL, FALSE, FALSE );
}

void CNetscapeEditView::OnSelectTableAllCells()
{
    EDT_SelectTableElement(GET_MWCONTEXT, 0,0, NULL, ED_HIT_SEL_ALL_CELLS, FALSE, FALSE );
}

void CNetscapeEditView::OnUpdateInTable(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(EDT_IsInsertPointInTable(GET_MWCONTEXT));
}

void CNetscapeEditView::OnMergeTableCells()
{
    EDT_MergeTableCells(GET_MWCONTEXT);
}

void CNetscapeEditView::OnSplitTableCell()
{
    EDT_SplitTableCell(GET_MWCONTEXT);
}

void CNetscapeEditView::OnUpdateMergeTableCells(CCmdUI* pCmdUI)
{
    ED_MergeType MergeType = EDT_GetMergeTableCellsType(GET_MWCONTEXT);
    if(pCmdUI->m_pMenu)
    {
        pCmdUI->m_pMenu->ModifyMenu( ID_MERGE_TABLE_CELLS, MF_BYCOMMAND, 
                             ID_MERGE_TABLE_CELLS, 
                             szLoadString(MergeType == ED_MERGE_NEXT_CELL ? IDS_MERGE_NEXT_CELL : IDS_MERGE_SELECTED_CELLS ) );
    }
    // Experimental: Allow merging wacky selection sets to see what happens!
    pCmdUI->Enable(CAN_INTERACT && MergeType != ED_MERGE_NONE);
}

void CNetscapeEditView::OnUpdateSplitTableCell(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CAN_INTERACT && EDT_CanSplitTableCell);
}

void CNetscapeEditView::OnTableTextConvert()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( EDT_IsTableSelected(pMWContext) || EDT_IsInsertPointInTable(pMWContext) )
    {
        EDT_ConvertTableToText(pMWContext);
    } else {
        // Get number of columns from user
        CGetColumnsDlg dlg(this);
        if( dlg.DoModal() == IDOK )
        {
            // Convert selected text into a table
            EDT_ConvertTextToTable(pMWContext, dlg.GetColumns());
        }
    }
}

void CNetscapeEditView::OnUpdateTableTextConvert(CCmdUI* pCmdUI)
{
    // Insert point should ALWAYS be inside table if any cells are selected
    MWContext * pMWContext = GET_MWCONTEXT;
    if( EDT_IsTableSelected(pMWContext) ||
        (EDT_IsInsertPointInTable(pMWContext) &&
         !EDT_IsSelected(pMWContext) && 
         EDT_GetSelectedCellCount(pMWContext) == 0) )
    {
        // Note that we allow converting of entire table to text
        //  if caret is in a cell, but nothing is selected
        pCmdUI->Enable(CAN_INTERACT);
    } else {
        pCmdUI->Enable(CAN_INTERACT && EDT_CanConvertTextToTable(pMWContext));
    }
}

void CNetscapeEditView::OnUndo()
{
#ifdef _IME_COMPOSITION
    if (m_imebool)
    {    
        if (m_pime)
        {
    #ifdef XP_WIN32
            HIMC hIMC;
            if (hIMC = m_pime->ImmGetContext(this->m_hWnd))
            {
                m_pime->ImmNotifyIME(hIMC,NI_COMPOSITIONSTR,CPS_COMPLETE,NULL);
                m_pime->ImmReleaseContext(this->m_hWnd,hIMC);
            }
    #else //xp_win16
            MWContext * pMWContext = GET_MWCONTEXT;
            if (pMWContext)
            {
                EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+m_imelength,0);
                m_pchardata->mask= -1;
                CLEARBIT(m_pchardata->values,TF_INLINEINPUT);//we are done!
                EDT_SetCharacterDataAtOffset(pMWContext,m_pchardata,m_imeoffset,m_imelength);
                m_imelength=0;
                OnImeEndComposition();
            }
    #endif //XP_WIN32 else xp_win16
        }
    }
#endif //_IME_COMPOSITION
    if( EDT_GetRedoCommandID(GET_MWCONTEXT, 0 ) != CEDITCOMMAND_ID_NULL ){
        EDT_Redo(GET_MWCONTEXT);
    } else if( EDT_GetUndoCommandID(GET_MWCONTEXT, 0 ) != CEDITCOMMAND_ID_NULL ){    
        EDT_Undo(GET_MWCONTEXT);
    }
}

void CNetscapeEditView::OnUpdateUndo(CCmdUI* pCmdUI)
{
    BOOL bRedo = EDT_GetRedoCommandID(GET_MWCONTEXT, 0 ) != CEDITCOMMAND_ID_NULL;
    // Change strings between "Undo" and "Redo" appropriately
	if( pCmdUI->m_pMenu ){
        pCmdUI->m_pMenu->ModifyMenu(ID_EDIT_UNDO, MF_BYCOMMAND | MF_STRING, ID_EDIT_UNDO,
                                    szLoadString(bRedo ? IDS_REDO : IDS_UNDO) );
    }    
    // Disabled only on initial page load.
    pCmdUI->Enable(CAN_INTERACT && (bRedo || EDT_GetUndoCommandID(GET_MWCONTEXT, 0 ) != CEDITCOMMAND_ID_NULL) );
}

void CNetscapeEditView::OnDisplayParagraphMarks()
{
    EDT_SetDisplayParagraphMarks(GET_MWCONTEXT, ! EDT_GetDisplayParagraphMarks(GET_MWCONTEXT));
}

void CNetscapeEditView::OnUpdateDisplayParagraphMarks(CCmdUI* pCmdUI)
{
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if( pController && pCmdUI->m_pMenu ){
        pCmdUI->m_pMenu->ModifyMenu(ID_EDIT_DISPLAY_PARAGRAPH_MARKS, MF_BYCOMMAND | MF_STRING, 
                                    CASTUINT(ID_EDIT_DISPLAY_PARAGRAPH_MARKS),
                                    szLoadString(EDT_GetDisplayParagraphMarks(GET_MWCONTEXT) ?
                                                 IDS_HIDE_PARA_MARKS : IDS_SHOW_PARA_MARKS) );
    }
    pCmdUI->Enable(CAN_INTERACT);
}

void CNetscapeEditView::OnInsertObjectPopup()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( ! pMWContext ){
        return;
    }
    RECT rectCaller = {0,0, 0, 0};
    CEditToolBarController *pController;
    CWnd *t_parent;
    if (!GetEmbedded())
        t_parent = (CWnd *)GetParentFrame();
    else
        t_parent = GetParent();
    if (t_parent)
        pController = (CEditToolBarController *)t_parent->SendMessage(WM_TOOLCONTROLLER);
	if (pController)
    {
		// This ALWAYS gets the CharacterToolbar 
#ifdef ENDER //NOT TRUE IN THE EMBEDDED CASE UNFORTUNATELY
        if (GetEmbedded())
        {
            CComboToolBar *t_bar = pController->GetCharacterBar();
            if (t_bar)
                t_bar->GetButtonRect(ID_INSERT_POPUP, &rectCaller);
        }
        else
        {
#endif //ENDER
        CNSToolbar2 *pCNSToolbar = pController->GetCNSToolbar();
        if( !pCNSToolbar || !pCNSToolbar->GetButtonRect(ID_INSERT_POPUP, &rectCaller) )
        {
		    // If we didn't find the button on the Charater toolbar
            //    (used in just the Message Composer), then we are in
            //    Composer window and button is on the Composition toolbar
        	CGenericFrame *pParent = (CGenericFrame*)GetParentFrame();
    		LPNSTOOLBAR pIToolBar;
            pParent->GetChrome()->QueryInterface( IID_INSToolBar, (LPVOID *) &pIToolBar );
            if (pIToolBar)
            {
                pIToolBar->GetButtonRect(ID_INSERT_POPUP, &rectCaller);
       			pIToolBar->Release();
            }
        }
#ifdef ENDER
        } //this changed to one block if not embedded 
#endif //ENDER

    }

    // Build dropdown toolbar for insert object buttons
    //   CommandID is sent to parent frame when pressed
    pTB = NULL;
    if (GetEmbedded())
        pTB = new CDropdownToolbar(this, pMWContext, &rectCaller, ID_INSERT_POPUP, 0);
    else
        pTB = new CDropdownToolbar(GetParentFrame(), pMWContext, &rectCaller, ID_INSERT_POPUP, 0);
    if( pTB ){
        pTB->AddButton(IDB_INSERT_LINK, ID_MAKE_LINK);
        pTB->AddButton(IDB_INSERT_TARGET, ID_INSERT_TARGET);
        pTB->AddButton(IDB_INSERT_IMAGE, ID_INSERT_IMAGE);
        pTB->AddButton(IDB_INSERT_HRULE, ID_INSERT_HRULE);
	    pTB->AddButton(IDB_INSERT_TABLE, ID_INSERT_TABLE);
        pTB->Show();
    }
}


// InsertTable

void CNetscapeEditView::OnInsertTable()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( pMWContext )
    {
        EDT_BeginBatchChanges(pMWContext);

        // Unselect so we insert after after the object
        if( NonLinkObjectIsSelected(pMWContext) )
            EDT_ClearSelection(pMWContext);
    
        // Insert a default table
        EDT_TableData *pTableData = EDT_NewTableData();
        if( pTableData )
        {
            // Insert a 1x1 default table
            EDT_InsertTable(pMWContext, pTableData);
            EDT_FreeTableData(pTableData);

            // Let user set number of rows, columns, etc,
            //  and remove table if they cancel immediately
            if( OnTableProperties(0) == IDCANCEL )
            {
                EDT_DeleteTable(pMWContext);
            }
        }
        EDT_EndBatchChanges(pMWContext);
    }
}

void CNetscapeEditView::OnInsertTableOrTableProps()
{
    MWContext * pMWContext = GET_MWCONTEXT;
    if( pMWContext )
    {
        // If inside a table, toolbar button should
        //  show properties of that table
        if( EDT_IsInsertPointInTable(pMWContext) )
        {
            // Get last-selected table region type so 
            //  we can show region-specific properties item,
            //  even though we will also show character and other property menu items above it
            ED_HitType iTableHit = EDT_GetSelectedTableElement(pMWContext, NULL);
            // Default is Table properties - first page
            int iStart = 0;
            if( iTableHit == ED_HIT_SEL_ROW ||
                iTableHit == ED_HIT_SEL_COL ||
                iTableHit == ED_HIT_SEL_CELL )
            {
                iStart = 1;
            }
            OnTableProperties(iStart);
        } else {
            // Not in table -- insert a new one
            OnInsertTable();
        }
    }
}

void CNetscapeEditView::OnUpdateInsertTable(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(CAN_INTERACT && !EDT_IsJavaScript(GET_MWCONTEXT));
}

// DeleteTable

void CNetscapeEditView::OnDeleteTable()
{
    EDT_DeleteTable(GET_MWCONTEXT);
}

// InsertTableRow

void CNetscapeEditView::OnInsertTableRow()
{
    EDT_TableRowData* pData = EDT_GetTableRowData( GET_MWCONTEXT );
    if( pData){
        SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
        // The 0 will cause us to use number of selected rows,
        //  or just 1 if there's none or 1 selected cell
        EDT_InsertTableRows(GET_MWCONTEXT, pData, TRUE, 0);
        EDT_FreeTableRowData(pData);
        SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
    }
}

void CNetscapeEditView::OnInsertTableRowAbove()
{
    EDT_TableRowData* pData = EDT_GetTableRowData( GET_MWCONTEXT );
    if( pData){
        SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
        EDT_InsertTableRows(GET_MWCONTEXT, pData, FALSE, 0);
        EDT_FreeTableRowData(pData);
        SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
    }
}

void CNetscapeEditView::OnUpdateInsertTableRow(CCmdUI* pCmdUI)
{
    // ALERT! BOGUS MFC BEHAVIOR
    // We should only test for menu popup,
    //  but pCmdUI->m_pMenu is NULL when called from WINCORE.CPP:
    //    BOOL CWnd::OnCommand(WPARAM wParam, LPARAM lParam)...
    //              OnCmdMsg(nID, CN_UPDATE_COMMAND_UI, &state, NULL);
    // Why, I don't know! But result is command doesn't happen and you get
    //  message: Warning: not executing disabled command <ID>
 
    if( /*pCmdUI->m_pMenu && */ EDT_IsInsertPointInTableRow(GET_MWCONTEXT) ){
        EDT_TableData* pData = EDT_GetTableData( GET_MWCONTEXT );
        if(pData){
            pCmdUI->Enable(pData->iRows < MAX_TABLE_ROWS);
            EDT_FreeTableData(pData);
            return;
        }
    }
    pCmdUI->Enable(FALSE);
}

// DeleteTableRow

void CNetscapeEditView::OnDeleteTableRow()
{
    EDT_DeleteTableRows(GET_MWCONTEXT, 0);
}

void CNetscapeEditView::OnUpdateInTableRow(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(EDT_IsInsertPointInTableRow(GET_MWCONTEXT));
}

// InsertTableCaption

void CNetscapeEditView::OnInsertTableCaption()
{
    EDT_TableCaptionData* pData = EDT_NewTableCaptionData();
    if( pData ){
        EDT_InsertTableCaption(GET_MWCONTEXT, pData);
        EDT_FreeTableCaptionData(pData);
    }
}

void CNetscapeEditView::OnUpdateInsertTableCaption(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(EDT_IsInsertPointInTable(GET_MWCONTEXT));
}

// DeleteTableCaption

void CNetscapeEditView::OnDeleteTableCaption()
{
    EDT_DeleteTableCaption(GET_MWCONTEXT);
}

void CNetscapeEditView::OnUpdateInTableCaption(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(EDT_IsInsertPointInTableCaption(GET_MWCONTEXT));
}

// InsertTableColumn

void CNetscapeEditView::OnInsertTableColumn()
{
    EDT_TableCellData* pData = EDT_NewTableCellData();
    if( pData ){
        SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
        // The 0 will cause us to use number of selected columns,
        //  or just 1 if there's none or 1 selected cell
        EDT_InsertTableColumns(GET_MWCONTEXT, pData, TRUE, 0);
        EDT_FreeTableCellData(pData);
        SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
    }
}

void CNetscapeEditView::OnInsertTableColumnBefore()
{
    EDT_TableCellData* pData = EDT_NewTableCellData();
    if( pData ){
        SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
        EDT_InsertTableColumns(GET_MWCONTEXT, pData, FALSE, 0);
        EDT_FreeTableCellData(pData);
        SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
    }
}

void CNetscapeEditView::OnUpdateInsertTableColumn(CCmdUI* pCmdUI)
{
    if( EDT_IsInsertPointInTableRow(GET_MWCONTEXT) )
    {
        EDT_TableData* pData = EDT_GetTableData( GET_MWCONTEXT );
        if(pData){
            pCmdUI->Enable(pData->iColumns < MAX_TABLE_COLUMNS);
            EDT_FreeTableData(pData);
            return;
        }
    }
    pCmdUI->Enable(FALSE);
}

// DeleteTableColumn

void CNetscapeEditView::OnDeleteTableColumn()
{
    EDT_DeleteTableColumns(GET_MWCONTEXT, 0);
}

void CNetscapeEditView::OnUpdateInTableColumn(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(EDT_IsInsertPointInTableRow(GET_MWCONTEXT));
}

// InsertTableCell

void CNetscapeEditView::OnInsertTableCell()
{
    EDT_TableCellData* pData = EDT_NewTableCellData();
    if( pData ){
        SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
        EDT_InsertTableCells(GET_MWCONTEXT, pData, TRUE, 1);
        EDT_FreeTableCellData(pData);
        SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
    }
}

void CNetscapeEditView::OnInsertTableCellBefore()
{
    EDT_TableCellData* pData = EDT_NewTableCellData();
    if( pData ){
        SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
        EDT_InsertTableCells(GET_MWCONTEXT, pData, FALSE, 1);
        EDT_FreeTableCellData(pData);
        SetCursor(theApp.LoadStandardCursor(IDC_ARROW));
    }
}

// DeleteTableCell

void CNetscapeEditView::OnDeleteTableCell()
{
    EDT_DeleteTableCells(GET_MWCONTEXT, 1);
}

void CNetscapeEditView::OnUpdateInTableCell(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(EDT_IsInsertPointInTableRow(GET_MWCONTEXT));
}

// Table Properties
void CNetscapeEditView::OnPropsTable()
{
    OnTableProperties(0);
}

void CNetscapeEditView::OnPropsTableRow()
{
    OnTableProperties(1);
}

void CNetscapeEditView::OnPropsTableColumn()
{
    OnTableProperties(0);
}

void CNetscapeEditView::OnPropsTableCell()
{
    OnTableProperties(1);
}

int CNetscapeEditView::OnTableProperties(int iStartPage)
{
    int iResult = IDOK;

    MWContext * pMWContext = GET_MWCONTEXT;
    if (pMWContext == NULL || !EDT_IS_EDITOR(pMWContext)) {
        return IDOK;
    }
    int iPageMax = 0;
    EDT_TableData * pTableData = EDT_GetTableData(pMWContext);
    if( !pTableData )
        return IDOK;

    // Get the cell data. This will be NULL if caret is 
    //  in a table caption. If it is, then do not add the Cell page
    EDT_TableCellData* pCellData = EDT_GetTableCellData(pMWContext);

    CNetscapePropertySheet PropsDlg( szLoadString(IDS_PROPS_TABLE_CAPTION),
                                    GET_DLG_PARENT(this), 0, pMWContext, TRUE); // Use the Apply Button

    // This will change resource hInstance to EditorXX.dll (in its constructor)
    // We pass this into our property page, which resets back to EXE's resources later
    CEditorResourceSwitcher ResourceSwitcher;
    
    CTablePage * pTablePage = new CTablePage(this, pMWContext, &ResourceSwitcher, pTableData);
    CTableCellPage * pCellPage = NULL;

    PropsDlg.AddPage(pTablePage);

    if( pCellData )
    {
        // Change the tab text corresponding to selected cells type
        // NOTE: These string must be in EDITORxx.DLL resources
        //        because they are loaded during dialog construction
        UINT nIDCaption = 0; // Default is single "Cell" label
        if( pCellData->iSelectedCount > 1 )
        {
            switch( pCellData->iSelectionType )
            {
                case ED_HIT_SEL_COL:
                    nIDCaption = IDS_SELECTED_COLUMN_CAPTION;
                    break;
                case ED_HIT_SEL_ROW:
                    nIDCaption = IDS_SELECTED_ROW_CAPTION;
                    break;
                default:
                   nIDCaption = IDS_SELECTED_CELLS_CAPTION;
                   break;
            }
        }
        
        pCellPage = new CTableCellPage(this, pMWContext, &ResourceSwitcher, pCellData, nIDCaption);
        PropsDlg.AddPage(pCellPage);
        iPageMax++;
    }

	int32 iPage = max( iPageMax, iStartPage );
    PropsDlg.SetCurrentPage(min(CASTINT(iStartPage), iPageMax));
    
    if( pCellData )
    {
        // Note: We used to do  EDT_BeginBatchChanges/EDT_EndBatchChanges around this
        // But that causes us to loose undo buffer even if user cancels from properties

        // This selects the current cell and changes other
        //  cells selected to the "special" selection style
        //  so user can tell the focus cell (= current) from
        //  other selected cells
        EDT_StartSpecialCellSelection(pMWContext, pCellData);
        iResult = PropsDlg.DoModal();
        EDT_ClearSpecialCellSelection(pMWContext);
    }

    // NOTE: DO NOT CALL EDT_FreeTableData OR EDT_FreeTableCellData here
    //   because prop pages may be freeing/replacing their data when table
    //   and/or cell sizes change. Freeing is done in destructors of the 
    //   property pages
    
    if( pCellPage ) delete pCellPage;
    delete pTablePage;

    return iResult;
}

void CNetscapeEditView::OnUpdateEditFindincurrent(CCmdUI* pCmdUI)
{
    if (!CAN_INTERACT)
        pCmdUI->Enable(FALSE);
    else
        CGenericView::OnUpdateEditFindincurrent(pCmdUI);
}

void CNetscapeEditView::OnUpdateEditFindAgain(CCmdUI* pCmdUI)
{
    if (!CAN_INTERACT)
        pCmdUI->Enable(FALSE);
    else
        CGenericView::OnUpdateEditFindAgain(pCmdUI);
}

void CNetscapeEditView::OnUpdateFileDocinfo(CCmdUI* pCmdUI) 
{
    if (!CAN_INTERACT)
        pCmdUI->Enable(FALSE);
    else
        CGenericView::OnUpdateFileDocinfo(pCmdUI);
}


/////////////////////////ADDED here from edview.cpp due to compiler running out of keys///////////
//MFJ10-8-97 mjudge

#ifdef _IME_COMPOSITION

#ifdef XP_WIN32

BOOL
CNetscapeEditView::ImeSetFont(HWND p_hwnd,LOGFONT *p_plogfont)
{
    if (!initializeIME())
        return FALSE;
    HIMC hIMC;
    if ((hIMC = m_pime->ImmGetContext(this->m_hWnd)) == NULL)
        return FALSE;
    return m_pime->ImmSetCompositionFont(hIMC,p_plogfont);
}


BOOL
CNetscapeEditView::initializeIME()
{
    if (!m_pime)
        m_pime=m_resourcedll.CreateImeDll();
    if (!m_pime)
    {
        assert(FALSE);
        return FALSE;
    }
    HIMC hIMC;
    if ((hIMC = m_pime->ImmGetContext(this->m_hWnd)) == NULL)
        return FALSE;
   	CPoint point= GetCaretPos();
    CANDIDATEFORM form;
    form.dwIndex=0;
    form.dwStyle=CFS_CANDIDATEPOS;
    form.ptCurrentPos.x=point.x;
    form.ptCurrentPos.y=point.y;
    VERIFY(m_pime->ImmSetCandidateWindow(hIMC,&form));

    m_pime->ImmReleaseContext(this->m_hWnd,hIMC);

    CWinCX    * pContext = GetContext();
    MWContext * pMWContext = pContext->GetContext();

    //one more thing, if previous selection,  we must delete it.
    if (EDT_IsSelected(pMWContext))
    {
        EDT_DeleteChar(pMWContext);//message boxes here would screw up IME
        //do nothing with error, errors here do not concern us.  probably just crossed a table boundary.            
    }
    m_imeoffset=EDT_GetInsertPointOffset(pMWContext);//used for calls to backend
    m_imelength=0;
    m_imebool=TRUE;
    XP_FREEIF(m_pchardata);
    m_pchardata=EDT_GetCharacterData(pMWContext);
    m_oldstring="";
    return TRUE;
}

LRESULT CNetscapeEditView::OnWmeImeStartComposition(WPARAM wparam,LPARAM lparam)
{
    if (!initializeIME())
    {
        assert(FALSE);
        return DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);
    }
    return TRUE; //DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);
}



LRESULT CNetscapeEditView::OnWmeImeKeyDown(WPARAM wparam,LPARAM lparam)
{
    if (m_pime&& (0x19==wparam))
    {
        //open ime, remember state in imestate
        //set open status to Chinese characters
        HIMC hIMC;
        HKL hKL=GetKeyboardLayout(0);	
        //DWORD fdwConversion;
        //DWORD fdwSentence;
        MWContext * pMWContext = GET_MWCONTEXT;
        if (!pMWContext)
            return DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);


        if (hIMC = m_pime->ImmGetContext(this->m_hWnd)){

            UINT t_imeoffset=EDT_GetInsertPointOffset(pMWContext);//used for calls to backend
            EDT_NextChar(pMWContext,TRUE);//TRUE to select character
            char *text;
            int32 textLen, hLen;
            XP_HUGE_CHAR_PTR htmlData = 0;
            SetCursor(theApp.LoadStandardCursor(IDC_WAIT));
            if(( EDT_COP_OK != (CASTINT(EDT_CopySelection(pMWContext, &text, &textLen, &htmlData, &hLen))) ))
                return DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);
            if (!text||!strlen(text))
                return DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);
            if (m_pime->ImeEscape(hKL,hIMC,IME_ESC_HANJA_MODE,text))
            {
                if (!initializeIME())
                    return DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);
                m_imelength=strlen(text);
                EDT_SetInsertPointToOffset(pMWContext,m_imeoffset,0);
            }
            XP_FREEIF(text);
            m_pime->ImmReleaseContext(this->m_hWnd,hIMC);
        }
        return DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);
    }
    else
        return DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);
}



LRESULT CNetscapeEditView::OnWmeImeEndComposition(WPARAM wparam,LPARAM lparam)
{
    if (!m_pime)
    {
        assert(FALSE);
        return DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);
    }
    return TRUE;//DefWindowProc(WM_IME_ENDCOMPOSITION,wparam,lparam);
}


LRESULT CNetscapeEditView::OnWmeImeComposition(WPARAM wparam,LPARAM lparam)
{
    if (!m_imebool)
        if (!initializeIME())
    {
        assert(FALSE);
        return DefWindowProc(WM_IME_STARTCOMPOSITION,wparam,lparam);
    }
    HIMC hIMC;
    char *szComp=NULL;
    char *szResultStr=NULL;

    BYTE *attributearray=NULL;
    DWORD dwAttrSize=0;
    DWORD dwResultSize=0;
    DWORD dwRead=0;
    DWORD dwSize=0;
    WORD cursorPos;
    WORD deltaPos;
    BOOL cursorPosMoved=FALSE;

    if ((hIMC = m_pime->ImmGetContext(this->m_hWnd)) == NULL)
        return FALSE;
    if ( lparam & GCS_RESULTSTR )
    {
		dwResultSize=m_pime->ImmGetCompositionString( hIMC, GCS_RESULTSTR, NULL, 0 );
        szResultStr=new char[dwResultSize+1];
		dwResultSize=m_pime->ImmGetCompositionString( hIMC, GCS_RESULTSTR, szResultStr, dwResultSize );
    }
    if ( lparam & GCS_COMPSTR )
	{
         dwSize=m_pime->ImmGetCompositionString(hIMC,GCS_COMPSTR, NULL, 0);
         szComp=new char[dwSize+1];
         dwSize=m_pime->ImmGetCompositionString(hIMC,GCS_COMPSTR, szComp, dwSize);
	}
    if ( lparam & GCS_DELTASTART)
    {
         deltaPos=LOWORD((DWORD) m_pime->ImmGetCompositionString(hIMC,GCS_DELTASTART, NULL, 0));
    }
    if (lparam & GCS_CURSORPOS)
    {
         cursorPos=LOWORD((DWORD) m_pime->ImmGetCompositionString(hIMC,GCS_CURSORPOS, NULL, 0));
         cursorPosMoved= cursorPos!=m_imeoldcursorpos;
         m_imeoldcursorpos=cursorPos;
    }
    if (lparam & GCS_COMPATTR)
    {
        dwAttrSize = LOWORD((DWORD) m_pime->ImmGetCompositionString(hIMC, GCS_COMPATTR,NULL, 0));
        attributearray = new BYTE[dwAttrSize+1];
        dwAttrSize = LOWORD((DWORD) m_pime->ImmGetCompositionString(hIMC, GCS_COMPATTR,attributearray, dwAttrSize));
    }
    
	if (lparam==0)
		dwSize=0;
    m_pime->ImmReleaseContext(this->m_hWnd,hIMC);//done with context

	if (szComp)
        szComp[dwSize]=(char)NULL;
	if (szResultStr)
        szResultStr[dwResultSize]=(char)NULL;

    CWinCX    * pContext = GetContext();
    MWContext * pMWContext = pContext->GetContext();
    INTL_CharSetInfo csi = LO_GetDocumentCharacterSetInfo(pMWContext);
    int16 win_csid = INTL_GetCSIWinCSID(csi);
    unsigned char *t_unicodestring=NULL;

    if (dwResultSize)
    {
        EDT_SetInsertPointToOffset(pMWContext,m_imeoffset,m_imelength);
        m_pchardata->mask= -1;//TF_INLINEINPUT;
        CLEARBIT(m_pchardata->values,TF_INLINEINPUT);

        if ((win_csid==CS_UTF8)||(win_csid==CS_UTF7))
        {
            t_unicodestring= INTL_ConvertLineWithoutAutoDetect(m_csid,win_csid,(unsigned char *)szResultStr,strlen(szResultStr));
            EDT_InsertText( pMWContext, (char *)t_unicodestring ); 
            m_imelength= strlen((char *)t_unicodestring);
            XP_FREEIF(t_unicodestring);
        }
        else
        {
            EDT_InsertText( pMWContext, szResultStr ); 
            m_pchardata->mask= -1;//TF_INLINEINPUT;
            m_imelength= strlen(szResultStr);
        }
        EDT_SetCharacterDataAtOffset(pMWContext,m_pchardata,m_imeoffset,m_imelength);
        m_imeoffset+=m_imelength;
        m_imelength=0;//no selections. everything is commited
        m_oldstring="";
        m_imebool=FALSE;
        m_imeoldcursorpos= (DWORD)-1;
    }
    if (dwSize)
    {
        int t_szcompstrlen;
        int t_difference=-1;
        char *t_pchar;
        if ((win_csid==CS_UTF8)||(win_csid==CS_UTF7))
        {
            t_unicodestring= INTL_ConvertLineWithoutAutoDetect(m_csid,win_csid,(unsigned char *)szComp,strlen(szComp));
            t_szcompstrlen=strlen((char *)t_unicodestring);
            t_pchar=(char *)t_unicodestring;

            if (lparam & GCS_DELTASTART)
            {
                int32 charCount = INTL_TextByteCountToCharLen(m_csid, (unsigned char *)szComp, deltaPos);
                t_difference = (WORD)INTL_TextCharLenToByteCount(win_csid, t_unicodestring, charCount);
            }
            else
            {        
                t_difference=findDifference(m_oldstring,(char *)t_unicodestring,pMWContext); //different strings?
                if (t_difference!= -1)
                {
                    int32 charCount = INTL_TextByteCountToCharLen(m_csid, (unsigned char *)szComp, t_difference);
                    t_difference = (WORD)INTL_TextCharLenToByteCount(win_csid, t_unicodestring, charCount);
                }
            }

            if (lparam & GCS_CURSORPOS)
            {
                if(cursorPos == strlen(szComp))
                {
                    cursorPos = t_szcompstrlen;
                }
                else
                {
                    int32 charCount = INTL_TextByteCountToCharLen(m_csid, (unsigned char *)szComp, cursorPos);
                    cursorPos = (WORD)INTL_TextCharLenToByteCount(win_csid, t_unicodestring, charCount);
                }
            }
        }
        else
        {
            t_szcompstrlen=strlen(szComp);
            t_pchar=szComp;
            if (lparam & GCS_DELTASTART)
                t_difference = deltaPos;
            else
                t_difference=findDifference(m_oldstring,t_pchar,pMWContext); //different strings?
        }
        if ((t_szcompstrlen==t_difference)&&(t_difference==m_oldstring.GetLength()))
            t_difference= -1; //hack to prevent invalid DELTAPOS from causing me to delete characters.
        if ((t_difference!= -1)||!m_oldstring.GetLength())
        {
            if (t_difference== -1)
                t_difference=0;
            if (t_difference==t_szcompstrlen) //someone hit backspace!
            {
                EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+t_difference,m_imelength-t_difference);
                EDT_DeleteChar(pMWContext); //do nothing with error, errors here do not concern us.  probably just crossed a table boundary.            
                m_imelength-=m_imelength-t_difference;
                m_oldstring=t_pchar;//autocopy from overloaded =
            }
            else //if (t_difference>=m_imelength)
            {
                if ((t_difference>=m_imelength)&&(m_imelength != (ED_BufferOffset)strlen(t_pchar)))
                    EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+m_imelength,0);
                else
                    EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+t_difference,m_imelength-t_difference);
                m_imelength= strlen(t_pchar);
                m_oldstring=t_pchar;
                t_pchar+=t_difference;
                m_pchardata->mask= -1;//TF_INLINEINPUT;
                SETBIT(m_pchardata->values,TF_INLINEINPUT);

                EDT_InsertText( pMWContext, t_pchar ); 
                EDT_SetCharacterDataAtOffset(pMWContext,m_pchardata,m_imeoffset+t_difference,m_imelength-t_difference);
            }
        }
        m_imebool=TRUE;
    }
    if (!dwSize&&!dwResultSize)// someone deleted the whole thing
    {
        EDT_SetInsertPointToOffset(pMWContext,m_imeoffset,m_imelength);
        m_pchardata->mask= -1;
        CLEARBIT(m_pchardata->values,TF_INLINEINPUT);//we are done!
        EDT_DeleteChar(pMWContext); //do nothing with error, errors here do not concern us.  probably just crossed a table boundary.            
        EDT_SetCharacterData(pMWContext,m_pchardata);
        m_imelength=0;
        m_imebool=FALSE;
    }

    int startloc= -1;
    int endloc= -1;
    if (( lparam & GCS_COMPATTR ) && (attributearray))
    {
        //find start location
        for (DWORD /*int*/ i=0;(startloc== -1)&&(i< dwAttrSize);i++)
        {
            if ((attributearray[i]==ATTR_TARGET_CONVERTED)||
                ((attributearray[i]==ATTR_TARGET_NOTCONVERTED)&&!cursorPosMoved) //if the cursor position moved, domeone hit esc if there is target notconverted. gotcha MS
                )
                startloc=i;
        }
        if (startloc!= -1)
        {
            for (DWORD /*int*/ i=startloc;(endloc== -1)&&(i<dwAttrSize);i++)
                if ((attributearray[i]!=ATTR_TARGET_CONVERTED)&&(attributearray[i]!=ATTR_TARGET_NOTCONVERTED))
                    endloc=i;
            if (endloc== -1)
                endloc=dwAttrSize;
            if ((win_csid==CS_UTF8)||(win_csid==CS_UTF7))
            {
                int32 charCount = INTL_TextByteCountToCharLen(m_csid, (unsigned char *)szComp, endloc);
                endloc = (WORD)INTL_TextCharLenToByteCount(win_csid, t_unicodestring, charCount);
                charCount = INTL_TextByteCountToCharLen(m_csid, (unsigned char *)szComp, startloc);
                startloc = (WORD)INTL_TextCharLenToByteCount(win_csid, t_unicodestring, charCount);
            }
            EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+startloc,endloc-startloc);
        }
    }
    if (startloc == -1)
    {
        if (lparam & GCS_CURSORPOS)
            EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+cursorPos,0);
        else
            EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+m_imelength,0);
    }
    if (szComp)
        delete [] szComp;
    if (szResultStr)
        delete [] szResultStr;
    if (attributearray)
        delete [] attributearray;
    XP_FREEIF(t_unicodestring);
    return TRUE;
}

void CNetscapeEditView::OnLButtonDown(UINT uFlags, CPoint cpPoint)
{
    if (m_imebool&&m_pime)
    {
        HIMC hIMC;
        if (hIMC = m_pime->ImmGetContext(this->m_hWnd))
        {
            m_pime->ImmNotifyIME(hIMC,NI_COMPOSITIONSTR,CPS_COMPLETE,NULL);
            m_pime->ImmReleaseContext(this->m_hWnd,hIMC);
        }
    }
    CNetscapeView::OnLButtonDown(uFlags,cpPoint);
    return;
}

LRESULT
CNetscapeEditView::OnInputLanguageChange(WPARAM wParam,LPARAM lParam)
{
    MWContext * pMWContext=GET_MWCONTEXT;
    if (!pMWContext)
        return FALSE;
    CString t_string;
    int t_numchar;
    int t_ansicp;

    INTL_CharSetInfo csi = LO_GetDocumentCharacterSetInfo(pMWContext);
    int16 win_csid = INTL_GetCSIWinCSID(csi);
    if ((win_csid==CS_UTF8)||(win_csid==CS_UTF7))    
    {
      int t_langid=(int)(lParam&0xFFFF);//only want low order word
      int t_localeid=MAKELCID(t_langid,SORT_DEFAULT);
      t_numchar=GetLocaleInfo(t_localeid,LOCALE_IDEFAULTANSICODEPAGE,0,NULL);
      t_numchar=GetLocaleInfo(t_localeid,LOCALE_IDEFAULTANSICODEPAGE,t_string.GetBuffer(t_numchar),t_numchar);
      t_string.ReleaseBuffer();
      if (!t_string.GetLength())
         return FALSE;
      t_ansicp=atoi(t_string);
      m_csid=CIntlWin::CodePageToCsid(t_ansicp);
        return CNetscapeView::DefWindowProc(WM_INPUTLANGCHANGE,wParam,lParam);
    }
    return FALSE;
}

LRESULT
CNetscapeEditView::OnInputLanguageChangeRequest(WPARAM wParam,LPARAM lParam)
{
    return CNetscapeView::DefWindowProc(WM_INPUTLANGCHANGEREQUEST,wParam,lParam);
}


#else //it is win16

void CNetscapeEditView::OnLButtonDown(UINT uFlags, CPoint cpPoint)
{
    if (m_imebool&&m_pime)
        return;
    else
        CNetscapeView::OnLButtonDown(uFlags,cpPoint);
}



LRESULT
CNetscapeEditView::OnReportIme(WPARAM wParam,LPARAM lParam)
{
    switch (wParam)
        {
        case IR_UNDETERMINE:
            return OnImeChangeComposition((HGLOBAL)lParam);
            break;
        case IR_CLOSECONVERT://should not be getting these!
            OnImeEndComposition();
            break;
        case IR_OPENCONVERT:
            initializeIME();//should not be getting these!
            break;
        case IR_STRINGSTART://should not be getting these!
            break;
        case IR_STRINGEND:    //should not be getting these!
            break;
        case IR_CHANGECONVERT:
            initializeIME();//should not be getting these!
            break;
        case IR_STRINGEX:
            return insertStringEx((HGLOBAL)lParam);
            break;
        default :
            break;
        }
    return TRUE;//doesnt matter on default returns
}

BOOL
CNetscapeEditView::initializeIME()
{
    if (!m_pime)
    {    
        m_pime=m_resourcedll.CreateImeDll();
    }
    if (!m_pime)
    {
        return FALSE; //may be english version.
    }
    if (!m_hIME)
        ImeCreate(this->GetSafeHwnd());
    LPIMESTRUCT lpIme;
    if (!(lpIme = (LPIMESTRUCT)GlobalLock(m_hIME)))
        return FALSE;

   	CPoint point=GetCaretPos();
    ClientToScreen(&point);
    lpIme->fnc=IME_SETCONVERSIONWINDOW;
    lpIme->wParam=MCW_HIDDEN;
    CRect t_rect;
    GetClientRect(t_rect);
    if (t_rect.IsRectEmpty())//this is necessary for 1st run through when window isnt sized properly.
    {
        t_rect.left=5;
        t_rect.top=5;
        t_rect.right=50;
        t_rect.bottom=20;
    }
    if ((point.x<0)||(point.x>GetSystemMetrics(SM_CXSCREEN))||(point.y<0)||(point.y>GetSystemMetrics(SM_CYSCREEN))) //something went wrong. bad client to screen conversion
    {
        point.x=0;
        point.y=0;
    }

    lpIme->lParam1=MAKELPARAM(point.x, point.y);
    lpIme->lParam2=MAKELPARAM(point.x, point.y);//BAD BAD BAD CHANGE!!!
    lpIme->lParam3=MAKELPARAM(point.x+t_rect.right, point.y+20);//BAD BAD BAD CHANGE!!!
    GlobalUnlock(m_hIME);
    m_pime->SendIMEMessageEx(this->GetSafeHwnd(),m_lIMEParam);

    CWinCX    * pContext = GetContext();
    if ((!m_imebool)&&(pContext))
    {
        MWContext * pMWContext = pContext->GetContext();
        m_imeoffset=EDT_GetInsertPointOffset(pMWContext);//used for calls to backend
        m_imelength=0;
        XP_FREEIF(m_pchardata);
        m_pchardata=EDT_GetCharacterData(pMWContext);
    }
    ImeSetFont(this->GetSafeHwnd(),(HFONT)GetStockObject(SYSTEM_FONT));
    m_oldstring="";
    return TRUE;
}


void    CNetscapeEditView::ImeCreate(HWND hWnd)
{
    HDC         hDC;
    TEXTMETRIC  tm;
    LPIMEPRO    lpImepro;
    CString     t_desc,t_name;
    m_hIME = GlobalAlloc(GHND, (LONG)sizeof(IMESTRUCT));
    m_lIMEParam = MAKELPARAM(m_hIME, 0);

    return;
}

/*-----------------------------------------------------------------------------

ImeDestroy

WM_DESTROY

-----------------------------------------------------------------------------*/
void    CNetscapeEditView::ImeDestroy(void)
{
    GlobalFree(m_hIME);
    return;
}


/*-----------------------------------------------------------------------------

    ImeMoveConvertWin



-----------------------------------------------------------------------------*/
void    CNetscapeEditView::ImeMoveConvertWin(HWND hWnd,int x,int y)
{
    if (!initializeIME())
    {
        return;
    }
    LPIMESTRUCT lpIme;

    if (lpIme = (LPIMESTRUCT)GlobalLock(m_hIME)) {
        lpIme->fnc = IME_SETCONVERSIONWINDOW;
        /* x == -1 && y == -1 */
        if (x == -1 && y == -1)
            lpIme->wParam = MCW_DEFAULT;
        else
            if (0) {//this decides if it needs to wrap. if so, throw it to bottom left

                lpIme->wParam = MCW_WINDOW;
                lpIme->lParam1 = MAKELPARAM(x, y);
            } else {

                lpIme->wParam = MCW_DEFAULT;
            }
        GlobalUnlock(m_hIME);

        LRESULT t_result=m_pime->SendIMEMessageEx(hWnd, m_lIMEParam);
    }
    return;
}



/*-----------------------------------------------------------------------------

    ImeSetFont

-----------------------------------------------------------------------------*/
HFONT   CNetscapeEditView::ImeSetFont(HWND hWnd,HFONT hFont)
{
    LPIMESTRUCT lpIme;
    HFONT       hResult = 0;

    if (lpIme = (LPIMESTRUCT)GlobalLock(m_hIME)) {
        /* IME_SETCONVERSIONFONT*/
        lpIme->fnc = IME_SETCONVERSIONFONT;
        /*IME*/
        lpIme->wParam = (WPARAM)hFont;
        LRESULT t_result=m_pime->SendIMEMessageEx(hWnd, m_lIMEParam);
        if (lpIme = (LPIMESTRUCT)GlobalLock(m_hIME)) 
        {
            hResult = (HFONT)lpIme->wParam;
            GlobalUnlock(m_hIME);
        }
        GlobalUnlock(m_hIME);
    }
    return hResult;
}


void CNetscapeEditView::OnImeStartComposition()
{
    if (!initializeIME())
    {
        assert(FALSE);
        return;
    }
    return;
}

void
CNetscapeEditView::OnImeEndComposition()
{
    if (!m_pime)
    {
        assert(FALSE);
        return;
    }
    CWinCX    * pContext = GetContext();
    MWContext * pMWContext = pContext->GetContext();
    m_pchardata->mask= -1;
    CLEARBIT(m_pchardata->values,TF_INLINEINPUT);//we are done!

    EDT_SetCharacterDataAtOffset(pMWContext,m_pchardata,m_imeoffset,m_imelength);

    EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+m_imelength,0);

    m_imebool=FALSE;
    m_imeoffset=0;
    return;
}



LRESULT CNetscapeEditView::insertStringEx(HGLOBAL p_global)
{
    if (!m_imebool)
    {
        if (!initializeIME())
        {
            assert(FALSE);
            return FALSE;
        }
        MWContext * pMWContext = GET_MWCONTEXT;
        //get rid of previous text! here
        if (pMWContext)
        {
            if (EDT_IsSelected(pMWContext))
            {
                EDT_DeleteChar(pMWContext);//message boxes here would screw up IME
                //do nothing with error, errors here do not concern us.  probably just crossed a table boundary.            
            }
        }
    }
    if (!p_global)
        return FALSE;
    char *t_pimestring=NULL;
    LPSTRINGEXSTRUCT t_struct=(LPSTRINGEXSTRUCT)GlobalLock(p_global);
    if (!t_struct)
        return FALSE;
    if (!t_struct->uDeterminePos) //no string??
    {
        GlobalUnlock(p_global);
        return FALSE;
    }
    t_pimestring=XP_STRDUP(((char *)t_struct)+t_struct->uDeterminePos);//use strdup because we want to release this global quickly
    GlobalUnlock(p_global);

    CWinCX    * pContext = GetContext();
    MWContext * pMWContext = pContext->GetContext();

    EDT_SetInsertPointToOffset(pMWContext,m_imeoffset,0);
    EDT_InsertText( pMWContext, t_pimestring ); 
    XP_FREEIF(t_pimestring);
    return TRUE;
}



LRESULT CNetscapeEditView::OnImeChangeComposition(HGLOBAL p_global)
{
    int t_justturnedon=FALSE;
    if (!p_global)
        return TRUE;
    if (!m_imebool)
    {
        if (!initializeIME())
        {
            assert(FALSE);
            return FALSE;
        }
        MWContext * pMWContext = GET_MWCONTEXT;
        //get rid of previous text! here
        if (pMWContext)
        {
            if (EDT_IsSelected(pMWContext))
            {
                EDT_DeleteChar(pMWContext);//message boxes here would screw up IME
                //do nothing with error, errors here do not concern us.  probably just crossed a table boundary.            
            }
        }
        m_imebool=TRUE;
        t_justturnedon=TRUE;
    }

    char *t_pimestring=NULL;
    char *t_pdeterminestring=NULL;
    char *t_attribarray=NULL;
    DWORD dwSize=0;
    DWORD dwAttrSize=0;
    WORD cursorPos=0;
    LPUNDETERMINESTRUCT t_struct=(LPUNDETERMINESTRUCT)GlobalLock(p_global);
    char *t_undetchar=((char *)t_struct)+t_struct->uUndetTextPos;
    char *t_detchar=((char *)t_struct)+t_struct->uDetermineTextPos;
    cursorPos=t_struct->uCursorPos;
    BOOL cursorPosMoved=cursorPos!=m_imeoldcursorpos;
    m_imeoldcursorpos=cursorPos;
    CWinCX    * pContext = GetContext();
    MWContext * pMWContext = pContext->GetContext();
    
    if ((t_struct->uUndetTextPos)&&(strlen(t_undetchar))) //undetermined string
    {
        if ((t_struct->uDetermineTextPos)&&(strlen(t_detchar))) //also determined string??  that means user hit space and is continuing
        {
            t_pdeterminestring=XP_STRDUP(t_detchar);//+1 for null
        }
        t_pimestring=XP_STRDUP(t_undetchar);
        if (t_struct->uUndetAttrPos)
        {
            dwAttrSize=strlen(t_undetchar);
            char *t_ptr=((char *)t_struct)+t_struct->uUndetAttrPos;
            t_attribarray=new char[dwAttrSize];
            memcpy(t_attribarray,t_ptr,dwAttrSize);
        }
    }
    else
        if ((t_struct->uDetermineTextPos)&&(strlen(t_detchar)))
        {
            t_pdeterminestring=XP_STRDUP(t_detchar);
        }
        else
        {
            GlobalUnlock(p_global);
            if (!t_justturnedon)
            {
                EDT_SetInsertPointToOffset(pMWContext,m_imeoffset,m_imelength);
                EDT_DeleteChar(pMWContext); //do nothingwith error, errors here do not concern us.  probably just crossed a table boundary.            
                m_imelength=0;
            }
            OnImeEndComposition();
            return TRUE;
        }
    GlobalUnlock(p_global);
    if ((!t_pimestring)&&(!t_pdeterminestring))
        return FALSE;


    int t_offset=m_imeoffset;//used in the case of optipization
    if (t_pdeterminestring)//we may have both determined and undetermined strings if this is true
    {
        m_pchardata->mask= -1;
        CLEARBIT(m_pchardata->values,TF_INLINEINPUT);//we are done!
        if (findDifference(m_oldstring,t_pdeterminestring,pMWContext)!= -1) //different strings
        {
            EDT_SetInsertPointToOffset(pMWContext,m_imeoffset,m_imelength);
            EDT_InsertText( pMWContext, t_pdeterminestring ); 
        }
        EDT_SetCharacterDataAtOffset(pMWContext,m_pchardata,m_imeoffset,strlen(t_pdeterminestring));
        m_imeoffset+=strlen(t_pdeterminestring);
        m_imelength=0;//just replaced previous string. no need to do so again
        EDT_SetInsertPointToOffset(pMWContext,m_imeoffset,m_imelength);  //or m_imeoffset,0
        m_oldstring="";
        m_imebool=FALSE;
        m_imeoldcursorpos= -1;
    }
    if (t_pimestring)
    {
        int t_difference=findDifference(m_oldstring,t_pimestring,pMWContext);
        if (!m_imelength||(-1 != t_difference)) //they are the same if -1 is returned
        {
            t_difference=max(t_difference,0);  //-1 becomes 0 if no old imelength
            if (t_difference>=strlen(t_pimestring)) //someone hit backspace!
            {
                EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+t_difference,m_imelength-t_difference);
                EDT_DeleteChar(pMWContext); //do nothing with error, errors here do not concern us.  probably just crossed a table boundary.            
                m_imelength-=m_imelength-t_difference;
                m_oldstring=m_oldstring.Mid(0,m_imelength);
            }
            else
            {
                if (t_difference>=m_imelength)
                    EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+m_imelength,0);
                else
                    EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+t_difference,m_imelength-t_difference);
                EDT_InsertText(pMWContext,t_pimestring+t_difference);
                m_imelength= strlen(t_pimestring);
                m_oldstring=t_pimestring;
                m_pchardata->mask= -1;
                SETBIT(m_pchardata->values,TF_INLINEINPUT);//we are done!
                EDT_SetCharacterDataAtOffset(pMWContext,m_pchardata,m_imeoffset+t_difference,m_imelength-t_difference);
            }
        }
        m_imebool=TRUE;
    }
    int startloc=-1;
    int endloc=-1;
    if (dwAttrSize)
    {
        //find start location
        for (int i=0;(startloc== -1)&&(i<dwAttrSize);i++)
        {
            if ((t_attribarray[i]==ATTR_TARGET_CONVERTED)||
                ((t_attribarray[i]==ATTR_TARGET_NOTCONVERTED)&&!cursorPosMoved) //if the cursor position moved, domeone hit esc if there is target notconverted. gotcha MS
                )
                startloc=i;
        }
        if (startloc!= -1)
        {
            for (int i=startloc;(endloc== -1)&&(i<dwAttrSize);i++)
                if ((t_attribarray[i]!=ATTR_TARGET_CONVERTED)&&(t_attribarray[i]!=ATTR_TARGET_NOTCONVERTED))
                    endloc=i;
            if (endloc== -1)
                endloc=dwAttrSize;
            EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+startloc,endloc-startloc);
        }
    }
    if (startloc== -1) //check cursor position
    {
        EDT_SetInsertPointToOffset(pMWContext,m_imeoffset+cursorPos,0);
    }
    XP_FREEIF(t_pimestring);
    XP_FREEIF(t_pdeterminestring);
    if (t_attribarray)
        delete []t_attribarray;
    return TRUE;
}


#endif//XP_WIN32 else win16


#endif //_IME_COMPOSITION


int CNetscapeEditView::BuildEditHistoryMenu(HMENU hMenu, int iStartItem)
{
    if( !hMenu )
        return 0;
    int iCount = GetMenuItemCount(hMenu);
    int i;
    
    // Delete existing menu
    for( i = iCount - 1; i >= iStartItem; i-- )
        DeleteMenu(hMenu, i, MF_BYPOSITION);

    iCount = 0;
    char * pUrl = NULL;
    char * pMenuItem = NULL;
	for( i = 0; i < MAX_EDIT_HISTORY_LOCATIONS; i++ )
	{
		if(EDT_GetEditHistory(GET_MWCONTEXT, i, &pUrl, NULL))
        {
            // Condense in case URL is too long for the menu
            // Note: pUrl is static string - don't free it
            CString csMenuString = pUrl;
            WFE_CondenseURL(csMenuString, MAX_MENU_ITEM_LENGTH, FALSE);

	        if (i < 9)
                // Add 1 - 9 as first character menu accelerator
		        pMenuItem = PR_smprintf(" &%d %s", i+1, csMenuString);
	        else if (i == 9)
                // Use "0" as accelerator for tenth file            
		        pMenuItem = PR_smprintf("%d&%d %s", 1, 0, csMenuString);
            else
                // This isn't used with our current maximum of 10 files,
                //   but keep in case we increase MAX_EDIT_HISTORY_LOCATIONS
		        pMenuItem = PR_smprintf("   %s", csMenuString);
            
            if( pMenuItem )
            {
                AppendMenu(hMenu, MF_STRING, ID_EDIT_HISTORY_BASE+i, pMenuItem);
                XP_FREE(pMenuItem);
                iCount++;
            }
        }
    }
    return iCount;
}

void CNetscapeEditView::OnCheckSpelling()
{
    MWContext *pMWContext = GET_MWCONTEXT;

    CHtmlSpellChecker SpellChecker(GET_MWCONTEXT, this);

    if (SpellChecker.ProcessDocument() != 0){
       // ASSERT(FALSE);
        TRACE0("SpellChecker.ProcessDocument() != 0\n");
    }
}

void CNetscapeEditView::OnSpellingLanguage()
{
}

// Button drop down menu commands and menu strings
const UINT idArrayNewDoc[] = { ID_EDT_NEW_DOC_BLANK,
                               ID_EDT_NEW_DOC_FROM_TEMPLATE,
                               ID_COMMAND_PAGE_FROM_WIZARD };

const UINT strArrayNewDoc[] = { IDS_NEW_DOC_BLANK,
                                IDS_NEW_DOC_FROM_TEMPLATE,
                                IDS_NEW_DOC_FROM_WIZARD };

const UINT idArrayPrint[] = { ID_FILE_PRINT,
                              ID_FILE_PAGE_SETUP,
                              ID_FILE_PRINT_PREVIEW };

const UINT strArrayPrint[] = { IDS_FILE_PRINT,
                               IDS_FILE_PAGE_SETUP,
                               IDS_FILE_PRINT_PREVIEW };

LRESULT CNetscapeEditView::OnButtonMenuOpen(WPARAM wParam, LPARAM lParam)
{

	HMENU hMenu = (HMENU) lParam;
	UINT nCommand = (UINT) LOWORD(wParam);

	const UINT *idArray = NULL;
	const UINT *strArray = NULL;
    int nSize = 3;
	
    if( nCommand == ID_EDT_NEW_DOC_BLANK )
    {
        idArray = idArrayNewDoc;
		strArray = strArrayNewDoc;
	} 
    else if( nCommand == ID_FILE_OPENURL )
    {
        // Menu length is dynamic - don't use fixed arrays
        // First 2 items are always the same
        AppendMenu(hMenu, MF_STRING, ID_FILE_OPENURL, szLoadString(IDS_FILE_OPENURL));
        AppendMenu(hMenu, MF_STRING, ID_FILE_OPEN, szLoadString(IDS_FILE_OPEN));
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);

        // The rest of the menu comes from the history list of recently-edited URLs
        BuildEditHistoryMenu(hMenu, 3);
    } 
    else if( nCommand == ID_FILE_PRINT )
    {
        idArray = idArrayPrint;
		strArray = strArrayPrint;
    }

	if ( idArray )
    {
		CString str;
		for(int i = 0; i < nSize; i++)
		{
			str.LoadString(strArray[i]);
			AppendMenu(hMenu, MF_STRING, idArray[i], (const char*) str);
		}
	}
	return 1;
}

////////////////////////////////////////
// Drag/drop functions

// Special non-blinking drop indicater to avoid stealing 
//  the one-and-only system caret
void DisplayDropCaret(HDC hDC, CRect cRect)
{
    int iRop = ::GetROP2(hDC);
    HPEN hPenOld = (HPEN)SelectObject(hDC, GetStockObject(WHITE_PEN));
    SetROP2(hDC, R2_XORPEN);

    // Solid, non-blinking "caret"
    InvertRect(hDC, LPRECT(cRect));

    // Triangle at top of caret
    MoveToEx(hDC, cRect.left - 1, cRect.top, 0);
    LineTo(hDC, cRect.left - 1, cRect.top + 2);
    MoveToEx(hDC, cRect.right, cRect.top, 0);
    LineTo(hDC, cRect.right, cRect.top + 2);

    MoveToEx(hDC, cRect.left - 2, cRect.top - 1, 0);
    LineTo(hDC, cRect.right + 2, cRect.top - 1);
    MoveToEx(hDC, cRect.left - 3, cRect.top - 2, 0);
    LineTo(hDC, cRect.right + 3, cRect.top - 2);

    SetROP2(hDC, iRop);
    SelectObject(hDC, hPenOld);
}

// Checks file extension for .gif, *.jpg etc.
BOOL CNetscapeEditView::CanSupportImageFile(const char * pFilename)
{
    if( pFilename ){
        const char * pExt = strrchr(pFilename, '.');
        if( pExt &&
            0 == _strcmpi( pExt, ".gif")  ||
            0 == _strcmpi( pExt, ".jpg")  ||
            0 == _strcmpi( pExt, ".jpeg") ||  
            0 == _strcmpi( pExt, ".png")  ||
            0 == _strcmpi( pExt, ".bmp") ) {
            return TRUE;
        }
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
CEditViewDropTarget::CEditViewDropTarget() :
    m_nDragType(0),
    m_dwLastKeyState(0),
    m_cLastPoint(-1,-1),
    m_LastDropEffect(DROPEFFECT_NONE)
{
}


DROPEFFECT CEditViewDropTarget::OnDragEnter(CWnd* pWnd, 
                COleDataObject* pDataObject, DWORD dwKeyState, CPoint point )
{
	if(pDataObject->IsDataAvailable(
		::RegisterClipboardFormat(vCardClipboardFormat)) )
			return DROPEFFECT_NONE;	

    m_nDragType = FE_DRAG_UNKNOWN;
    m_nIsOKDrop = FALSE;
    CNetscapeEditView * pView = (CNetscapeEditView*)pWnd;
    MWContext *pMWContext;
    if( pView->GetContext() == NULL || (pMWContext = pView->GetContext()->GetContext()) == NULL )
        return DROPEFFECT_NONE;
    
    pView->m_bDragOver = TRUE;

    // This gets called no matter who starts a drag,
    //  so look for types we are interested in
    CLIPFORMAT nImageFormat = RegisterClipboardFormat(NETSCAPE_IMAGE_FORMAT);
    if( pDataObject->IsDataAvailable(nImageFormat) ) {
        m_nDragType = FE_DRAG_IMAGE;
    } else if( pDataObject->IsDataAvailable(RegisterClipboardFormat(NETSCAPE_BOOKMARK_FORMAT)) ) {
        m_nDragType = FE_DRAG_LINK;
    } else if( pDataObject->IsDataAvailable(RegisterClipboardFormat(NETSCAPE_EDIT_FORMAT)) ) {
        m_nDragType = EDT_IsDraggingTable(pMWContext) ? FE_DRAG_TABLE : FE_DRAG_HTML;
    } else if( pDataObject->IsDataAvailable(CF_TEXT) 
#ifdef XP_WIN32
				|| pDataObject->IsDataAvailable(CF_UNICODETEXT) 
#endif
				) {
        m_nDragType = FE_DRAG_TEXT;
#ifdef XP_WIN32
    } else if( pDataObject->IsDataAvailable(CF_HDROP) ) {
        // Check what kind of file it is
        // We accept HTML for link creation,
        // GIF and JPG for image insertion
        
        HDROP handle = (HDROP)pDataObject->GetGlobalData(CF_HDROP);
        char pFilename[1024];
        if ( DragQueryFile(handle, 0, 
                           pFilename, 1024) ) {
            CString csFilename(pFilename);
            int iLastDot = csFilename.ReverseFind('.');
            if(iLastDot>0) {
                CString csExt = csFilename.Mid(iLastDot);
                if ( 0 == csExt.CompareNoCase(".htm") ||
                     0 == csExt.CompareNoCase(".html") ||
                     0 == csExt.CompareNoCase(".shtml") ) {
                    m_nDragType = FE_DRAG_LINK;
                } 
                else if ( pView->CanSupportImageFile(LPCSTR(csExt)) ) {
                    m_nDragType = FE_DRAG_IMAGE;
                }
            }
        }
        // Didn't find any files of interest                
        if ( m_nDragType == FE_DRAG_UNKNOWN ){
            return(DROPEFFECT_NONE);
        }
#endif
    } else {
        return(DROPEFFECT_NONE);
    }
    m_nIsOKDrop = TRUE;

    if ( pView->GetContext()->IsDragging() ) {
        return(DROPEFFECT_MOVE);
    } else {
        return(DROPEFFECT_COPY);
    }
}

DROPEFFECT CEditViewDropTarget::OnDragOver(CWnd* pWnd, 
                COleDataObject* pDataObject, DWORD dwKeyState, CPoint cPoint )
{
    if ( !m_nIsOKDrop )
    {
        return(DROPEFFECT_NONE);
    }
    DROPEFFECT DropEffect = DROPEFFECT_NONE;

    // Simply return last state if position and keystate is the same
    if( dwKeyState == m_dwLastKeyState && cPoint == m_cLastPoint )
    {
        return(m_LastDropEffect);
    }
    m_dwLastKeyState = dwKeyState;
    m_cLastPoint = cPoint;

    CNetscapeEditView * pView = (CNetscapeEditView *) pWnd;

    // Do Caret-moving to show where we will drop
    CWinCX *     pContext = pView->GetContext();
    MWContext  * pMWContext = pContext->GetContext();
    if ( pContext && pMWContext )
    {
        // Don't allow dropping on selection if drag source is our view
        if( pContext->IsDragging() && pContext->PtInSelectedRegion(cPoint, TRUE) )
        {
            return(DROPEFFECT_NONE);
        }
        
        switch ( m_nDragType )
        {
            case FE_DRAG_LINK:
            // Drop HTML or image anywhere we can drop text:
            case FE_DRAG_HTML:
            case FE_DRAG_TABLE:
            case FE_DRAG_IMAGE:
            case FE_DRAG_TEXT:
                int32 xVal, yVal;        
                pView->ClientToDocXY( cPoint, &xVal, &yVal );
                // Check if near a border (within 10 pixels) and we should scroll the window
                while( pContext->CheckAndScrollWindow(xVal, yVal, 0, 10) )
                {
                    // Get current mouse location and convert to doc coordinates
                    POINT point;
                    GetCursorPos(&point);
                    pView->ScreenToClient(&point);
                    cPoint.x = point.x;
                    cPoint.y = point.y;
                    pView->ClientToDocXY( cPoint, &xVal, &yVal );

                    EDT_PositionDropCaret(pMWContext, xVal, yVal);
                    // Delay 20 millisecs between each cycle so it doesn't scroll too fast
                    DWORD startTime = timeGetTime();
                    do { FEU_StayingAlive(); }
                    while( timeGetTime() - startTime < 20 );
                }

                // Note: This will also handle feedback for where to drop table/cells
                if( EDT_PositionDropCaret(pMWContext, xVal, yVal) )
                {
                    // Figure out what drag cursor to use
                    // TODO: This gets messier when doing drag between 2 frames
                    //       Need to know if source is editor vs. browser
                    //       to know when to allow move or copy 
                    if ( pContext->IsDragging() && 
                         !(dwKeyState & MK_CONTROL) ) {
                        // We are draging within the frame
                        DropEffect = DROPEFFECT_MOVE;
                    } else {
                        DropEffect = DROPEFFECT_COPY;
                    }
                }
                break;
        }
    }

    m_LastDropEffect = DropEffect;

    return(DropEffect);
}

BOOL CEditViewDropTarget::OnDrop(CWnd* pWnd, 
                COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint cPoint)
{
    if(!pDataObject || !pWnd)
        return(FALSE);

    CNetscapeEditView *pView = (CNetscapeEditView *)pWnd;
    pView->m_bDragOver = FALSE;

    // Use paste routine shared with clipboard pasting
    BOOL bResult = (pView->DoPasteItem(pDataObject, &cPoint, 
                                       dropEffect == DROPEFFECT_MOVE, // bDeleteSource
                                       ED_PASTE_NORMAL ));


    // TRACE2("OnDrop m_hWnd=%X, SafeHwnd=%X\n", pWnd->m_hWnd, pWnd->GetSafeHwnd() );
    MWContext * pMWContext = pView->GetContext()->GetContext();
    if ( EDT_IS_EDITOR(pMWContext)  &&
         ::GetFocus() != pWnd->m_hWnd && pView->m_caret.bEnabled ) {
        // Kill caret if we don't have focus
        FE_DestroyCaret(pMWContext);
        pView->m_caret.cShown = 0;
        pView->m_caret.bEnabled = FALSE;
        DestroyCaret();
    }    
    return bResult;
}

void CEditViewDropTarget::OnDragLeave(CWnd* pWnd)
{
    // Note: We don't come here if we drop into our view
    CNetscapeEditView * pView = (CNetscapeEditView *) pWnd;
    
    pView->m_bDragOver = FALSE;

    // Restore caret coordinates
    if ( pView->m_caret.bEnabled ) {
        pView->m_caret.cShown = 0;
        TRACE0("OnDragLeave: caret disabled\n");
        pView->m_caret.bEnabled = FALSE;
        DestroyCaret();
    }
}
// end of CEditViewDropTarget

#endif // EDITOR
