// MozillaBrowser.h : Declaration of the CMozillaBrowser

#ifndef __MOZILLABROWSER_H_
#define __MOZILLABROWSER_H_

#include "resource.h"       // main symbols

#include "WebShellContainer.h"

// This file is autogenerated using the ATL proxy wizard
// so don't edit it!
#include "CPMozillaControl.h"

/////////////////////////////////////////////////////////////////////////////
// CMozillaBrowser
class ATL_NO_VTABLE CMozillaBrowser : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMozillaBrowser, &CLSID_MozillaBrowser>,
	public CComControl<CMozillaBrowser>,
	public CProxyDWebBrowserEvents<CMozillaBrowser>,
	public CStockPropImpl<CMozillaBrowser, IWebBrowser, &IID_IWebBrowser, &LIBID_MOZILLACONTROLLib>,
	public IProvideClassInfo2Impl<&CLSID_MozillaBrowser, &DIID_DWebBrowserEvents, &LIBID_MOZILLACONTROLLib>,
	public IPersistStreamInitImpl<CMozillaBrowser>,
	public IPersistStorageImpl<CMozillaBrowser>,
	public IQuickActivateImpl<CMozillaBrowser>,
	public IOleControlImpl<CMozillaBrowser>,
	public IOleObjectImpl<CMozillaBrowser>,
	public IOleInPlaceActiveObjectImpl<CMozillaBrowser>,
	public IViewObjectExImpl<CMozillaBrowser>,
	public IOleInPlaceObjectWindowlessImpl<CMozillaBrowser>,
	public IDataObjectImpl<CMozillaBrowser>,
	public ISupportErrorInfo,
	public IConnectionPointContainerImpl<CMozillaBrowser>,
	public ISpecifyPropertyPagesImpl<CMozillaBrowser>
{
	friend CWebShellContainer;
public:
	CMozillaBrowser();
	virtual ~CMozillaBrowser();

DECLARE_REGISTRY_RESOURCEID(IDR_MOZILLABROWSER)

BEGIN_COM_MAP(CMozillaBrowser)
	// IE web browser interface
	COM_INTERFACE_ENTRY(IWebBrowser)
//	COM_INTERFACE_ENTRY(IMozillaBrowser)
	COM_INTERFACE_ENTRY_IID(IID_IDispatch, IWebBrowser)
	COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY_IMPL(IOleControl)
	COM_INTERFACE_ENTRY_IMPL(IOleObject)
//	COM_INTERFACE_ENTRY_IMPL(IQuickActivate)
	COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
	COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
	COM_INTERFACE_ENTRY_IMPL(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY_IMPL(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CMozillaBrowser)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	PROP_PAGE(CLSID_StockColorPage)
END_PROPERTY_MAP()


BEGIN_CONNECTION_POINT_MAP(CMozillaBrowser)
	// Fires IE events
	CONNECTION_POINT_ENTRY(DIID_DWebBrowserEvents)
END_CONNECTION_POINT_MAP()


BEGIN_MSG_MAP(CMozillaBrowser)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
END_MSG_MAP()

// Windows message handlers
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IViewObjectEx
	STDMETHOD(GetViewStatus)(DWORD* pdwStatus)
	{
		ATLTRACE(_T("IViewObjectExImpl::GetViewStatus\n"));
		*pdwStatus = VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE;
		return S_OK;
	}

// Protected members
protected:
	CWebShellContainer	*	m_pWebShellContainer;

	// Mozilla interfaces
    nsIWebShell			*	m_pIWebShell;
	nsIPref             *   m_pIPref;
	
	// Indicates the browser is busy doing something
	BOOL					m_bBusy;

	virtual HRESULT CreateWebShell();
	virtual BOOL IsValid();

// IWebBrowser implementation
public:
    virtual HRESULT STDMETHODCALLTYPE GoBack(void);
    virtual HRESULT STDMETHODCALLTYPE GoForward(void);
    virtual HRESULT STDMETHODCALLTYPE GoHome(void);
    virtual HRESULT STDMETHODCALLTYPE GoSearch(void);
    virtual HRESULT STDMETHODCALLTYPE Navigate(BSTR URL, VARIANT __RPC_FAR *Flags, VARIANT __RPC_FAR *TargetFrameName, VARIANT __RPC_FAR *PostData, VARIANT __RPC_FAR *Headers);
    virtual HRESULT STDMETHODCALLTYPE Refresh(void);
    virtual HRESULT STDMETHODCALLTYPE Refresh2(VARIANT __RPC_FAR *Level);
    virtual HRESULT STDMETHODCALLTYPE Stop( void);
    virtual HRESULT STDMETHODCALLTYPE get_Application(IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
    virtual HRESULT STDMETHODCALLTYPE get_Parent(IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
    virtual HRESULT STDMETHODCALLTYPE get_Container(IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
    virtual HRESULT STDMETHODCALLTYPE get_Document(IDispatch __RPC_FAR *__RPC_FAR *ppDisp);
    virtual HRESULT STDMETHODCALLTYPE get_TopLevelContainer(VARIANT_BOOL __RPC_FAR *pBool);
    virtual HRESULT STDMETHODCALLTYPE get_Type(BSTR __RPC_FAR *Type);
    virtual HRESULT STDMETHODCALLTYPE get_Left(long __RPC_FAR *pl);
    virtual HRESULT STDMETHODCALLTYPE put_Left(long Left);
    virtual HRESULT STDMETHODCALLTYPE get_Top(long __RPC_FAR *pl);
    virtual HRESULT STDMETHODCALLTYPE put_Top(long Top);
    virtual HRESULT STDMETHODCALLTYPE get_Width(long __RPC_FAR *pl);
    virtual HRESULT STDMETHODCALLTYPE put_Width(long Width);
    virtual HRESULT STDMETHODCALLTYPE get_Height(long __RPC_FAR *pl);
    virtual HRESULT STDMETHODCALLTYPE put_Height(long Height);
    virtual HRESULT STDMETHODCALLTYPE get_LocationName(BSTR __RPC_FAR *LocationName);
    virtual HRESULT STDMETHODCALLTYPE get_LocationURL(BSTR __RPC_FAR *LocationURL);
    virtual HRESULT STDMETHODCALLTYPE get_Busy(VARIANT_BOOL __RPC_FAR *pBool);

public:
	HRESULT OnDraw(ATL_DRAWINFO& di);

};

#endif //__MOZILLABROWSER_H_
