/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 * 
 * Author:
 *   Adam Lock <adamlock@netscape.com>
 *
 * Contributor(s): 
 */
#include "stdafx.h"
#include "IEHtmlNode.h"

CIEHtmlNode::CIEHtmlNode()
{
	m_pIDOMNode = nsnull;
	m_pIDispParent = NULL;
}

CIEHtmlNode::~CIEHtmlNode()
{
	SetDOMNode(nsnull);
}

HRESULT CIEHtmlNode::SetParentNode(IDispatch *pIDispParent)
{
	m_pIDispParent = pIDispParent;
	return S_OK;
}

HRESULT CIEHtmlNode::SetDOMNode(nsIDOMNode *pIDOMNode)
{
	if (m_pIDOMNode)
	{
		m_pIDOMNode->Release();
		m_pIDOMNode = nsnull;
	}
	
	if (pIDOMNode)
	{
		m_pIDOMNode = pIDOMNode;
		m_pIDOMNode->AddRef();
	}

	return S_OK;
}

HRESULT CIEHtmlNode::GetDOMNode(nsIDOMNode **pIDOMNode)
{
	if (pIDOMNode == NULL)
	{
		return E_INVALIDARG;
	}

	*pIDOMNode = nsnull;
	if (m_pIDOMNode)
	{
		m_pIDOMNode->AddRef();
		*pIDOMNode = m_pIDOMNode;
	}

	return S_OK;
}

HRESULT CIEHtmlNode::GetDOMElement(nsIDOMElement **pIDOMElement)
{
	if (pIDOMElement == NULL)
	{
		return E_INVALIDARG;
	}

	if (m_pIDOMNode == nsnull)
	{
		return E_NOINTERFACE;
	}

	*pIDOMElement = nsnull;
	m_pIDOMNode->QueryInterface(NS_GET_IID(nsIDOMElement), (void **) pIDOMElement);
	return (*pIDOMElement) ? S_OK : E_NOINTERFACE;
}

HRESULT CIEHtmlNode::GetIDispatch(IDispatch **pDispatch)
{
	if (pDispatch == NULL)
	{
		return E_INVALIDARG;
	}
	
	*pDispatch = NULL;
	return E_NOINTERFACE;
}
