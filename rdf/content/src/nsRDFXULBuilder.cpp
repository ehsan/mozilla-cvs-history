/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

/*

  An nsIRDFDocument implementation that builds a XUL content model.
  

 */

#include "nsCOMPtr.h"
#include "nsDebug.h"
#include "nsIAtom.h"
#include "nsIDocument.h"
#include "nsINameSpaceManager.h"
#include "nsIRDFContent.h"
#include "nsIRDFContentModelBuilder.h"
#include "nsIRDFCursor.h"
#include "nsIRDFCompositeDataSource.h"
#include "nsIRDFDocument.h"
#include "nsIRDFNode.h"
#include "nsIRDFService.h"
#include "nsIServiceManager.h"
#include "nsINameSpaceManager.h"
#include "nsIServiceManager.h"
#include "nsISupportsArray.h"
#include "nsLayoutCID.h"
#include "nsRDFCID.h"
#include "nsRDFContentUtils.h"
#include "nsString.h"
#include "rdf.h"
#include "rdfutil.h"

#if 0 // need to link against layout.dll for this...
#include "nsIHTMLContent.h"
#include "nsHTMLParts.h"
#endif

////////////////////////////////////////////////////////////////////////

static NS_DEFINE_IID(kIContentIID,                NS_ICONTENT_IID);
static NS_DEFINE_IID(kIDocumentIID,               NS_IDOCUMENT_IID);
static NS_DEFINE_IID(kINameSpaceManagerIID,       NS_INAMESPACEMANAGER_IID);
static NS_DEFINE_IID(kIRDFResourceIID,            NS_IRDFRESOURCE_IID);
static NS_DEFINE_IID(kIRDFLiteralIID,             NS_IRDFLITERAL_IID);
static NS_DEFINE_IID(kIRDFContentIID,             NS_IRDFCONTENT_IID);
static NS_DEFINE_IID(kIRDFContentModelBuilderIID, NS_IRDFCONTENTMODELBUILDER_IID);
static NS_DEFINE_IID(kIRDFServiceIID,             NS_IRDFSERVICE_IID);

static NS_DEFINE_CID(kNameSpaceManagerCID,        NS_NAMESPACEMANAGER_CID);
static NS_DEFINE_CID(kRDFServiceCID,              NS_RDFSERVICE_CID);

////////////////////////////////////////////////////////////////////////
// standard vocabulary items

DEFINE_RDF_VOCAB(RDF_NAMESPACE_URI, RDF, type);
DEFINE_RDF_VOCAB(RDF_NAMESPACE_URI, RDF, child); // XXX bogus: needs to be NC:child

////////////////////////////////////////////////////////////////////////

class RDFXULBuilderImpl : public nsIRDFContentModelBuilder
{
private:
    nsIRDFCompositeDataSource* mDB;
    nsIRDFDocument*            mDocument;

    // pseudo-constants
    static PRInt32 gRefCnt;
    static nsIRDFService*  gRDFService;

    static PRInt32  kNameSpaceID_RDF;
    static PRInt32  kNameSpaceID_XUL;

    static nsIAtom* kContentsGeneratedAtom;

    static nsIRDFResource* kRDF_type;
    static nsIRDFResource* kRDF_child; // XXX needs to become kNC_child

public:
    RDFXULBuilderImpl();
    virtual ~RDFXULBuilderImpl();

    // nsISupports interface
    NS_DECL_ISUPPORTS

    // nsIRDFContentModelBuilder interface
    NS_IMETHOD SetDocument(nsIRDFDocument* aDocument);
    NS_IMETHOD CreateRoot(nsIRDFResource* aResource);
    NS_IMETHOD CreateContents(nsIRDFContent* aElement);
    NS_IMETHOD OnAssert(nsIRDFContent* aElement, nsIRDFResource* aProperty, nsIRDFNode* aValue);
    NS_IMETHOD OnUnassert(nsIRDFContent* aElement, nsIRDFResource* aProperty, nsIRDFNode* aValue);

    // Implementation methods
    nsresult AppendChild(nsIContent* aElement,
                         nsIRDFNode* aValue);

    nsresult CreateElement(nsIRDFResource* aResource,
                           nsIContent** aResult);

    nsresult CreateHTMLElement(nsIRDFResource* aResource,
                               nsIAtom* aTag,
                               nsIContent** aResult);

    nsresult CreateHTMLContents(nsIContent* aElement,
                                nsIRDFResource* aResource);

    nsresult CreateXULElement(nsIRDFResource* aResource,
                              PRInt32 aNameSpaceID,
                              nsIAtom* aTag,
                              nsIContent** aResult);

    nsresult AddAttribute(nsIContent* aElement,
                          nsIRDFResource* aProperty,
                          nsIRDFNode* aValue);
};

////////////////////////////////////////////////////////////////////////

PRInt32         RDFXULBuilderImpl::gRefCnt = 0;
nsIRDFService*  RDFXULBuilderImpl::gRDFService = nsnull;

PRInt32         RDFXULBuilderImpl::kNameSpaceID_RDF = kNameSpaceID_Unknown;
PRInt32         RDFXULBuilderImpl::kNameSpaceID_XUL = kNameSpaceID_Unknown;

nsIAtom*        RDFXULBuilderImpl::kContentsGeneratedAtom = nsnull;

nsIRDFResource* RDFXULBuilderImpl::kRDF_type   = nsnull;
nsIRDFResource* RDFXULBuilderImpl::kRDF_child  = nsnull;

////////////////////////////////////////////////////////////////////////

nsresult
NS_NewRDFXULBuilder(nsIRDFContentModelBuilder** result)
{
    NS_PRECONDITION(result != nsnull, "null ptr");
    if (! result)
        return NS_ERROR_NULL_POINTER;

    RDFXULBuilderImpl* builder = new RDFXULBuilderImpl();
    if (! builder)
        return NS_ERROR_OUT_OF_MEMORY;

    NS_ADDREF(builder);
    *result = builder;
    return NS_OK;
}



RDFXULBuilderImpl::RDFXULBuilderImpl(void)
    : mDB(nsnull)
{
    NS_INIT_REFCNT();

    if (gRefCnt++ == 0) {
        nsresult rv;
        nsINameSpaceManager* mgr;
        if (NS_SUCCEEDED(rv = nsRepository::CreateInstance(kNameSpaceManagerCID,
                                                           nsnull,
                                                           kINameSpaceManagerIID,
                                                           (void**) &mgr))) {

// XXX This is sure to change. Copied from mozilla/layout/xul/content/src/nsXULAtoms.cpp
static const char kXULNameSpaceURI[]
    = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";

static const char kRDFNameSpaceURI[]
    = RDF_NAMESPACE_URI;

            rv = mgr->RegisterNameSpace(kXULNameSpaceURI, kNameSpaceID_XUL);
            NS_ASSERTION(NS_SUCCEEDED(rv), "unable to register XUL namespace");

            rv = mgr->RegisterNameSpace(kRDFNameSpaceURI, kNameSpaceID_RDF);
            NS_ASSERTION(NS_SUCCEEDED(rv), "unable to register RDF namespace");

            NS_RELEASE(mgr);
        }
        else {
            NS_ERROR("couldn't create namepsace manager");
        }


        if (NS_SUCCEEDED(rv = nsServiceManager::GetService(kRDFServiceCID,
                                                           kIRDFServiceIID,
                                                           (nsISupports**) &gRDFService))) {

            NS_VERIFY(NS_SUCCEEDED(gRDFService->GetResource(kURIRDF_type, &kRDF_type)),
                      "unable to get resource");

            NS_VERIFY(NS_SUCCEEDED(gRDFService->GetResource(kURIRDF_child, &kRDF_child)),
                      "unable to get resource");
        }
        else {
            NS_ERROR("couldnt' get RDF service");
        }

        kContentsGeneratedAtom = NS_NewAtom("contentsGenerated");
    }
}

RDFXULBuilderImpl::~RDFXULBuilderImpl(void)
{
    NS_IF_RELEASE(mDB);

    if (--gRefCnt == 0) {
        if (gRDFService)
            nsServiceManager::ReleaseService(kRDFServiceCID, gRDFService);

        NS_IF_RELEASE(kRDF_type);
        NS_IF_RELEASE(kRDF_child);
        NS_IF_RELEASE(kContentsGeneratedAtom);
    }
}

////////////////////////////////////////////////////////////////////////

NS_IMPL_ISUPPORTS(RDFXULBuilderImpl, kIRDFContentModelBuilderIID);

////////////////////////////////////////////////////////////////////////
// nsIRDFContentModelBuilder methods

NS_IMETHODIMP
RDFXULBuilderImpl::SetDocument(nsIRDFDocument* aDocument)
{
    NS_PRECONDITION(aDocument != nsnull, "null ptr");
    if (! aDocument)
        return NS_ERROR_NULL_POINTER;

    mDocument = aDocument; // not refcounted

    nsresult rv;
    if (NS_FAILED(rv = mDocument->GetDataBase(mDB)))
        return rv;

    return NS_OK;
}

NS_IMETHODIMP
RDFXULBuilderImpl::CreateRoot(nsIRDFResource* aResource)
{
    NS_PRECONDITION(mDocument != nsnull, "not initialized");
    if (! mDocument)
        return NS_ERROR_NOT_INITIALIZED;

    NS_PRECONDITION(aResource != nsnull, "null ptr");
    if (! aResource)
        return NS_ERROR_NULL_POINTER;

    nsresult rv;

    nsCOMPtr<nsIContent> root;
    if (NS_FAILED(rv = CreateElement(aResource, getter_AddRefs(root)))) {
        NS_ERROR("unable to create root element");
        return rv;
    }

    // Now set it as the document's root content
    nsCOMPtr<nsIDocument> doc;
    if (NS_FAILED(rv = mDocument->QueryInterface(kIDocumentIID,
                                                 (void**) getter_AddRefs(doc)))) {
        NS_ERROR("couldn't get nsIDocument interface");
        return rv;
    }

    doc->SetRootContent(root);

    return NS_OK;
}


NS_IMETHODIMP
RDFXULBuilderImpl::CreateContents(nsIRDFContent* aElement)
{
    nsresult rv;

    if (NS_FAILED(rv = aElement->SetAttribute(kNameSpaceID_XUL,
                                              kContentsGeneratedAtom,
                                              "true",
                                              PR_FALSE))) {
        NS_ERROR("unable to set contents-generated attribute");
        return rv;
    }

    nsCOMPtr<nsIRDFResource> resource;
    if (NS_FAILED(rv = aElement->GetResource(*getter_AddRefs(resource)))) {
        NS_ERROR("unable to get resource from element");
        return rv;
    }

    nsCOMPtr<nsIRDFAssertionCursor> children;
    if (NS_FAILED(rv = mDB->GetTargets(resource, kRDF_child, PR_TRUE, getter_AddRefs(children)))) {
        NS_ERROR("unable to create cursor for children");
        return rv;
    }

    while (NS_SUCCEEDED(rv = children->Advance())) {
        nsCOMPtr<nsIRDFNode> child;
        if (NS_FAILED(rv = children->GetObject(getter_AddRefs(child)))) {
            NS_ERROR("error reading cursor");
            return rv;
        }

        if (NS_FAILED(AppendChild(aElement, child))) {
            NS_ERROR("problem appending child to content model");
            return rv;
        }
    }

    if (rv == NS_ERROR_RDF_CURSOR_EMPTY)
        rv = NS_OK;

    return rv;
}


NS_IMETHODIMP
RDFXULBuilderImpl::OnAssert(nsIRDFContent* aElement,
                            nsIRDFResource* aProperty,
                            nsIRDFNode* aValue)
{
    nsresult rv;

    if (aProperty == kRDF_child) {
        // It's a child node. If the contents of aElement _haven't_
        // yet been generated, then just ignore the assertion. We do
        // this because we know that _eventually_ the contents will be
        // generated (via CreateContents()) when somebody asks for
        // them later.
        nsAutoString contentsGenerated;
        if (NS_FAILED(rv = aElement->GetAttribute(kNameSpaceID_XUL,
                                                  kContentsGeneratedAtom,
                                                  contentsGenerated))) {
            if (rv == NS_CONTENT_ATTR_NOT_THERE) {
                return NS_OK;
            }
            else {
                NS_ERROR("severe problem trying to get attribute");
                return rv;
            }
        }

        if (! contentsGenerated.EqualsIgnoreCase("true"))
            return NS_OK;

        // Okay, it's a "live" element, so go ahead and append the new
        // child to this node.
        if (NS_FAILED(AppendChild(aElement, aValue))) {
            NS_ERROR("problem appending child to content model");
            return rv;
        }
    }
    else if (aProperty == kRDF_type) {
        // We shouldn't ever see this: if we do, there ain't much we
        // can do.
        NS_ERROR("attempt to change tag type after-the-fact");
        return NS_ERROR_UNEXPECTED;
    }
    else {
        // Add the thing as a vanilla attribute to the element.
        if (NS_FAILED(rv = AddAttribute(aElement, aProperty, aValue))) {
            NS_ERROR("unable to add attribute to the element");
            return rv;
        }
    }
    return NS_OK;
}


NS_IMETHODIMP
RDFXULBuilderImpl::OnUnassert(nsIRDFContent* aElement,
                              nsIRDFResource* aProperty,
                              nsIRDFNode* aValue)
{
    NS_NOTYETIMPLEMENTED("write me!");
    return NS_ERROR_NOT_IMPLEMENTED;
}


////////////////////////////////////////////////////////////////////////

nsresult
RDFXULBuilderImpl::AppendChild(nsIContent* aElement,
                               nsIRDFNode* aValue)
{
    nsresult rv;

    // Add the specified node as a child container of this
    // element. What we do will vary slightly depending on whether
    // aValue is a resource or a literal.
    nsCOMPtr<nsIRDFResource> resource;
    nsCOMPtr<nsIRDFLiteral> literal;

    if (NS_SUCCEEDED(rv = aValue->QueryInterface(kIRDFResourceIID,
                                                 (void**) getter_AddRefs(resource)))) {

        // If it's a resource, then add it as a child container.
        nsCOMPtr<nsIContent> child;
        if (NS_FAILED(rv = CreateElement(resource, getter_AddRefs(child)))) {
            NS_ERROR("unable to create new XUL element");
            return rv;
        }

        if (NS_FAILED(rv = aElement->AppendChildTo(child, PR_TRUE))) {
            NS_ERROR("unable to add element to content model");
            return rv;
        }
    }
    else if (NS_SUCCEEDED(rv = aValue->QueryInterface(kIRDFLiteralIID,
                                                      (void**) getter_AddRefs(literal)))) {
        // If it's a literal, then add it as a simple text node.

        if (NS_FAILED(rv = rdf_AttachTextNode(aElement, literal))) {
            NS_ERROR("unable to add text to content model");
            return rv;
        }
    }
    else {
        // This should _never_ happen
        NS_ERROR("node is not a value or a resource");
        return NS_ERROR_UNEXPECTED;
    }

    return NS_OK;
}


nsresult
RDFXULBuilderImpl::CreateElement(nsIRDFResource* aResource,
                                 nsIContent** aResult)
{
    nsresult rv;

    // Split the resource into a namespace ID and a tag, and create
    // a content element for it.
    nsCOMPtr<nsIRDFNode> typeNode;
    if (NS_FAILED(rv = mDB->GetTarget(aResource, kRDF_type, PR_TRUE, getter_AddRefs(typeNode)))) {
        NS_ERROR("unable to get node's type");
        return rv;
    }

    nsCOMPtr<nsIRDFResource> type;
    if (NS_FAILED(rv = typeNode->QueryInterface(kIRDFResourceIID, getter_AddRefs(type)))) {
        NS_ERROR("type wasn't a resource");
        return rv;
    }

    PRInt32 nameSpaceID;
    nsCOMPtr<nsIAtom> tag;
    if (NS_FAILED(rv = mDocument->SplitProperty(type, &nameSpaceID, getter_AddRefs(tag)))) {
        NS_ERROR("unable to split resource into namespace/tag pair");
        return rv;
    }

    if (nameSpaceID == kNameSpaceID_HTML)
        return CreateHTMLElement(aResource, tag, aResult);
    else
        return CreateXULElement(aResource, nameSpaceID, tag, aResult);
}

nsresult
RDFXULBuilderImpl::CreateHTMLElement(nsIRDFResource* aResource,
                                     nsIAtom* aTag,
                                     nsIContent** aResult)
{
    NS_NOTYETIMPLEMENTED("need to link against layout.dll for this to work");

#if 0
    nsresult rv;

    nsCOMPtr<nsIHTMLContent> element;
    if (NS_FAILED(rv = NS_CreateHTMLElement(getter_AddRefs(element), aTag->GetUnicode()))) {
        NS_ERROR("unable to create HTML element");
        return rv;
    }

    // Now iterate through all the properties and add them as
    // attributes on the element.  First, create a cursor that'll
    // iterate through all the properties that lead out of this
    // resource.
    nsCOMPtr<nsIRDFArcsOutCursor> properties;
    if (NS_FAILED(rv = mDB->ArcLabelsOut(aResource, getter_AddRefs(properties)))) {
        NS_ERROR("unable to create arcs-out cursor");
        return rv;
    }

    // Advance that cursor 'til it runs outta steam
    while (NS_SUCCEEDED(rv = properties->Advance())) {
        nsCOMPtr<nsIRDFResource> property;

        if (NS_FAILED(rv = properties->GetPredicate(getter_AddRefs(property)))) {
            NS_ERROR("unable to get property from cursor");
            return rv;
        }

        // These are special beacuse they're used to specify the tree
        // structure of the XUL: ignore them b/c they're not attributes
        if (property == kRDF_type)
            continue;

        // Recursively generate child nodes NOW: we can't "dummy" up
        // nsIHTMLContent.
        if (property == kRDF_child) {
            CreateHTMLContents(element, aResource);
            continue;
        }

        // For each property, get its value: this will be the value of
        // the new attribute.
        nsCOMPtr<nsIRDFNode> value;
        if (NS_FAILED(rv = mDB->GetTarget(aResource, property, PR_TRUE, getter_AddRefs(value)))) {
            NS_ERROR("unable to get value for property");
            return rv;
        }

        // Add the attribute to the newly constructed element
        if (NS_FAILED(rv = AddAttribute(element, property, value))) {
            NS_ERROR("unable to add attribute to element");
            return rv;
        }
    }

    if (rv == NS_ERROR_RDF_CURSOR_EMPTY) {
        rv = NS_OK;
    }
    else if (NS_FAILED(rv)) {
        // uh oh...
        NS_ERROR("problem iterating properties");
        return rv;
    }

    if (NS_FAILED(rv = element->QueryInterface(kIContentIID, (void**) aResult))) {
        NS_ERROR("unable to get nsIContent interface");
        return rv;
    }
#endif

    return NS_OK;
}


nsresult
RDFXULBuilderImpl::CreateHTMLContents(nsIContent* aElement,
                                      nsIRDFResource* aResource)
{
    nsresult rv;

    nsCOMPtr<nsIRDFAssertionCursor> children;
    if (NS_FAILED(rv = mDB->GetTargets(aResource, kRDF_child, PR_TRUE, getter_AddRefs(children)))) {
        NS_ERROR("unable to create cursor for children");
        return rv;
    }

    while (NS_SUCCEEDED(rv = children->Advance())) {
        nsCOMPtr<nsIRDFNode> child;
        if (NS_FAILED(rv = children->GetObject(getter_AddRefs(child)))) {
            NS_ERROR("error reading cursor");
            return rv;
        }

        if (NS_FAILED(AppendChild(aElement, child))) {
            NS_ERROR("problem appending child to content model");
            return rv;
        }
    }

    if (rv == NS_ERROR_RDF_CURSOR_EMPTY)
        rv = NS_OK;

    return rv;
}



nsresult
RDFXULBuilderImpl::CreateXULElement(nsIRDFResource* aResource,
                                    PRInt32 aNameSpaceID,
                                    nsIAtom* aTag,
                                    nsIContent** aResult)
{
    nsresult rv;

    nsCOMPtr<nsIRDFContent> element;
    if (NS_FAILED(rv = NS_NewRDFResourceElement(getter_AddRefs(element),
                                                aResource,
                                                aNameSpaceID,
                                                aTag))) {
        NS_ERROR("unable to create new content element");
        return rv;
    }

    // Now iterate through all the properties and add them as
    // attributes on the element.  First, create a cursor that'll
    // iterate through all the properties that lead out of this
    // resource.
    nsCOMPtr<nsIRDFArcsOutCursor> properties;
    if (NS_FAILED(rv = mDB->ArcLabelsOut(aResource, getter_AddRefs(properties)))) {
        NS_ERROR("unable to create arcs-out cursor");
        return rv;
    }

    // Advance that cursor 'til it runs outta steam
    while (NS_SUCCEEDED(rv = properties->Advance())) {
        nsCOMPtr<nsIRDFResource> property;

        if (NS_FAILED(rv = properties->GetPredicate(getter_AddRefs(property)))) {
            NS_ERROR("unable to get property from cursor");
            return rv;
        }

        // These are special beacuse they're used to specify the tree
        // structure of the XUL: ignore them b/c they're not attributes
        if (property == kRDF_child || property == kRDF_type)
            continue;

        // For each property, set its value.
        nsCOMPtr<nsIRDFNode> value;
        if (NS_FAILED(rv = mDB->GetTarget(aResource, property, PR_TRUE, getter_AddRefs(value)))) {
            NS_ERROR("unable to get value for property");
            return rv;
        }

        // Add the attribute to the newly constructed element
        if (NS_FAILED(rv = AddAttribute(element, property, value))) {
            NS_ERROR("unable to add attribute to element");
            return rv;
        }
    }

    if (rv == NS_ERROR_RDF_CURSOR_EMPTY) {
        rv = NS_OK;
    }
    else if (NS_FAILED(rv)) {
        // uh oh...
        NS_ERROR("problem iterating properties");
        return rv;
    }

    // Make it a container so that its contents get recursively
    // generated on-demand.
    if (NS_FAILED(rv = element->SetContainer(PR_TRUE))) {
        NS_ERROR("unable to make element a container");
        return rv;
    }

    // Finally, assign the newly constructed element to the result
    // pointer and addref it for the trip home.
    *aResult = element;
    NS_ADDREF(*aResult);

    return NS_OK;
}



nsresult
RDFXULBuilderImpl::AddAttribute(nsIContent* aElement,
                                nsIRDFResource* aProperty,
                                nsIRDFNode* aValue)
{
    nsresult rv;

    // First, split the property into its namespace and tag components
    PRInt32 nameSpaceID;
    nsCOMPtr<nsIAtom> tag;
    if (NS_FAILED(rv = mDocument->SplitProperty(aProperty, &nameSpaceID, getter_AddRefs(tag)))) {
        NS_ERROR("unable to split resource into namespace/tag pair");
        return rv;
    }

    nsCOMPtr<nsIRDFResource> resource;
    nsCOMPtr<nsIRDFLiteral> literal;

    // Now we need to figure out the attributes value and actually set
    // it on the element. What we do differs a bit depending on
    // whether we're aValue is a resource or a literal.
    if (NS_SUCCEEDED(rv = aValue->QueryInterface(kIRDFResourceIID,
                                                 (void**) getter_AddRefs(resource)))) {
        const char* uri;
        resource->GetValue(&uri);
        rv = aElement->SetAttribute(nameSpaceID, tag, uri, PR_TRUE);
    }
    else if (NS_SUCCEEDED(rv = aValue->QueryInterface(kIRDFLiteralIID,
                                                      (void**) getter_AddRefs(literal)))) {
        const PRUnichar* s;
        literal->GetValue(&s);
        rv = aElement->SetAttribute(nameSpaceID, tag, s, PR_TRUE);
    }
    else {
        // This should _never_ happen.
        NS_ERROR("uh, this isn't a resource or a literal!");
        rv = NS_ERROR_UNEXPECTED;
    }

    return rv;
}
