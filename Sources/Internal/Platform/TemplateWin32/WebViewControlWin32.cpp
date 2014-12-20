/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#pragma warning(push)
#pragma warning(disable: 4717) //'ATL::CCRTAllocator::free' : recursive on all control paths, function will cause runtime stack overflow
#include <atlbase.h>
#pragma warning(pop)
#include <atlcom.h>
#include <atlstr.h> // see http://msdn.microsoft.com/en-us/library/bwea7by5.aspx
// see http://www.suodenjoki.dk/us/archive/2010/min-max.htm
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#include <atlimage.h>
#undef min
#undef max
#include <ExDisp.h>
#include <ExDispid.h>
#include <ObjIdl.h>
#include <Shlwapi.h>

#include "WebViewControlWin32.h"
#include "CorePlatformWin32.h"
using namespace DAVA;

extern _ATL_FUNC_INFO BeforeNavigate2Info;
_ATL_FUNC_INFO BeforeNavigate2Info = {CC_STDCALL, VT_EMPTY, 7, {VT_DISPATCH,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_VARIANT,VT_BYREF|VT_BOOL}};

extern _ATL_FUNC_INFO DocumentCompleteInfo;
_ATL_FUNC_INFO DocumentCompleteInfo =  {CC_STDCALL,VT_EMPTY,2,{VT_DISPATCH,VT_BYREF | VT_VARIANT}};
namespace DAVA 
{

template <class T>
class ScopedComPtr {
protected:
    T *ptr;
public:
    ScopedComPtr() : ptr(NULL) { }
    explicit ScopedComPtr(T *ptr) : ptr(ptr) { }
    ~ScopedComPtr() {
        if (ptr)
            ptr->Release();
    }
    bool Create(const CLSID clsid) {
        CrashIf(ptr);
        if (ptr) return false;
        HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_ALL, IID_PPV_ARGS(&ptr));
        return SUCCEEDED(hr);
    }
    operator T*() const { return ptr; }
    T** operator&() { return &ptr; }
    T* operator->() const { return ptr; }
    T* operator=(T* newPtr) {
        if (ptr)
            ptr->Release();
        return (ptr = newPtr);
    }
};

template <class T>
class ScopedComQIPtr : public ScopedComPtr<T> {
public:
    ScopedComQIPtr() : ScopedComPtr() { }
    explicit ScopedComQIPtr(IUnknown *unk) {
        HRESULT hr = unk->QueryInterface(&ptr);
        if (FAILED(hr))
            ptr = NULL;
    }
    T* operator=(IUnknown *newUnk) {
        if (ptr)
            ptr->Release();
        HRESULT hr = newUnk->QueryInterface(&ptr);
        if (FAILED(hr))
            ptr = NULL;
        return ptr;
    }
};

class HtmlMoniker : public IMoniker
{
public:
    HtmlMoniker() :
        refCount(1),
        htmlStream(NULL)
    {

    }

    virtual ~HtmlMoniker()
    {
        if (htmlStream)
            htmlStream->Release();
    }

    HRESULT HtmlMoniker::SetHtml(const String& _htmlData)
    {
        htmlData = _htmlData;
        if (htmlStream)
            htmlStream->Release();
        htmlStream = CreateStreamFromData(htmlData.c_str(), htmlData.size());
        return S_OK;
    }

    HRESULT HtmlMoniker::SetBaseUrl(const WideString& _baseUrl)
    {
        baseUrl = _baseUrl;
        return S_OK;
    }
public:
    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
    {
        static const QITAB qit[] = {
            QITABENT(HtmlMoniker, IMoniker),
            QITABENT(HtmlMoniker, IPersistStream),
            QITABENT(HtmlMoniker, IPersist),
            { 0 }
        };
        return QISearch(this, qit, riid, ppvObject);
    }

    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        LONG res = InterlockedDecrement(&refCount);
        DVASSERT(res >= 0);
        if (0 == res)
            delete this;
        return res;
    }

    // IMoniker
    STDMETHODIMP BindToStorage(IBindCtx *pbc, IMoniker *pmkToLeft, REFIID riid, void **ppvObj)
    {
        LARGE_INTEGER seek = {0};
        htmlStream->Seek(seek, STREAM_SEEK_SET, NULL);
        return htmlStream->QueryInterface(riid, ppvObj);
    }

    STDMETHODIMP GetDisplayName(IBindCtx *pbc, IMoniker *pmkToLeft, LPOLESTR *ppszDisplayName)
    {
        if (!ppszDisplayName)
            return E_POINTER;
        DVASSERT(!baseUrl.empty())

        *ppszDisplayName = OleStrDup(baseUrl.c_str(), baseUrl.length());

        return S_OK;
    }

    STDMETHODIMP BindToObject(IBindCtx *pbc, IMoniker *pmkToLeft, REFIID riidResult, void **ppvResult) { return E_NOTIMPL; }
    STDMETHODIMP Reduce(IBindCtx *pbc, DWORD dwReduceHowFar, IMoniker **ppmkToLeft, IMoniker **ppmkReduced) { return E_NOTIMPL; }
    STDMETHODIMP ComposeWith(IMoniker *pmkRight, BOOL fOnlyIfNotGeneric, IMoniker **ppmkComposite) { return E_NOTIMPL; }
    STDMETHODIMP Enum(BOOL fForward, IEnumMoniker **ppenumMoniker) { return E_NOTIMPL; }
    STDMETHODIMP IsEqual(IMoniker *pmkOtherMoniker) { return E_NOTIMPL; }
    STDMETHODIMP Hash(DWORD *pdwHash) { return E_NOTIMPL; }
    STDMETHODIMP IsRunning(IBindCtx *pbc, IMoniker *pmkToLeft, IMoniker *pmkNewlyRunning) { return E_NOTIMPL; }
    STDMETHODIMP GetTimeOfLastChange(IBindCtx *pbc, IMoniker *pmkToLeft, FILETIME *pFileTime) { return E_NOTIMPL; }
    STDMETHODIMP Inverse(IMoniker **ppmk) { return E_NOTIMPL; }
    STDMETHODIMP CommonPrefixWith(IMoniker *pmkOther, IMoniker **ppmkPrefix) { return E_NOTIMPL; }
    STDMETHODIMP RelativePathTo(IMoniker *pmkOther, IMoniker **ppmkRelPath) { return E_NOTIMPL; }
    STDMETHODIMP ParseDisplayName(IBindCtx *pbc, IMoniker *pmkToLeft,LPOLESTR pszDisplayName,
        ULONG *pchEaten, IMoniker **ppmkOut) { return E_NOTIMPL; }

    STDMETHODIMP IsSystemMoniker(DWORD *pdwMksys) {
        if (!pdwMksys)
            return E_POINTER;
        *pdwMksys = MKSYS_NONE;
        return S_OK;
    }

    // IPersistStream methods
    STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty)  { return E_NOTIMPL; }
    STDMETHODIMP IsDirty() { return E_NOTIMPL; }
    STDMETHODIMP Load(IStream *pStm) { return E_NOTIMPL; }
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize) { return E_NOTIMPL; }

    // IPersist
    STDMETHODIMP GetClassID(CLSID *pClassID) { return E_NOTIMPL; }

private:
    LPOLESTR OleStrDup(const WCHAR *s, int len)
    {
        size_t cb = sizeof(WCHAR) * (len + 1);
        LPOLESTR ret = (LPOLESTR)CoTaskMemAlloc(cb);
        memcpy(ret, s, cb);
        return ret;
    }

    IStream *CreateStreamFromData(const void *data, size_t len)
    {
        if (!data)
            return NULL;

        ScopedComPtr< IStream > stream;
        if (FAILED(CreateStreamOnHGlobal(NULL, TRUE, &stream)))
            return NULL;

        ULONG written;
        if (FAILED(stream->Write(data, (ULONG)len, &written)) || written != len)
            return NULL;

        LARGE_INTEGER zero = { 0 };
        stream->Seek(zero, STREAM_SEEK_SET, NULL);

        stream->AddRef();

        return stream;
    }

    LONG                refCount;
    String        htmlData;
    IStream *           htmlStream;
    WideString    baseUrl;
};

struct EventSink : public IDispEventImpl<1, EventSink, &DIID_DWebBrowserEvents2>
{
private:
	IUIWebViewDelegate* delegate;
	UIWebView* webView;
    WebBrowserContainer* container;
public:
	EventSink()
	{
		delegate = NULL;
		webView = NULL;
		container = NULL;
	};

	void SetDelegate(IUIWebViewDelegate *delegate, UIWebView* webView, WebBrowserContainer* container)
	{
		if (delegate && webView && container)
		{
			this->delegate = delegate;
			this->webView = webView;
            this->container = container;
		}
	}
    void SetWebView(UIWebView* webView)
    {
        this->webView = webView;
    }
    // TODO test only
    void SetContainer( WebBrowserContainer* container)
    {
        this->container = container;
    }

	void  __stdcall DocumentComplete(IDispatch* pDisp, VARIANT* URL)
	{
        if (container)
        {
            int32 width = 0;
            int32 height = 0;
            container->GetContainerSize(&width, &height);
            DVASSERT(webView);
            container->SaveSnapshot(width, height, webView);
        }
		if (delegate && webView)
		{
            if (!container->DoOpenBuffer())
                delegate->PageLoaded(webView);
		}
	}

	void __stdcall BeforeNavigate2(IDispatch* pDisp, VARIANT* URL, VARIANT* Flags,
								   VARIANT* TargetFrameName, VARIANT* PostData,
								   VARIANT* Headers, VARIANT_BOOL* Cancel)
	{
		bool process = true;

		if (delegate && webView)
		{
			BSTR bstr = V_BSTR(URL);
			int32 len = SysStringLen(bstr) + 1;
			char* str = new char[len];
			WideCharToMultiByte(CP_ACP, 0, bstr, -1, str, len, NULL, NULL);
			String s = str;
			delete[] str;
			bool isRedirectedByMouseClick  = Flags->intVal == navHyperlink ;
			IUIWebViewDelegate::eAction action = delegate->URLChanged(webView, s, isRedirectedByMouseClick);

			switch (action)
			{
				case IUIWebViewDelegate::PROCESS_IN_WEBVIEW:
					Logger::FrameworkDebug("PROCESS_IN_WEBVIEW");
					break;

				case IUIWebViewDelegate::PROCESS_IN_SYSTEM_BROWSER:
					Logger::FrameworkDebug("PROCESS_IN_SYSTEM_BROWSER");
					process = false;
					ShellExecute(NULL, L"open", bstr, NULL, NULL, SW_SHOWNORMAL);
					break;

				case IUIWebViewDelegate::NO_PROCESS:
					Logger::FrameworkDebug("NO_PROCESS");

				default:
					process = false;
					break;
			}
		}

		*Cancel = process ? VARIANT_FALSE : VARIANT_TRUE;
	}

	BEGIN_SINK_MAP(EventSink)
		SINK_ENTRY_INFO(1, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, BeforeNavigate2, &BeforeNavigate2Info)
		SINK_ENTRY_INFO(1, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, DocumentComplete, &DocumentCompleteInfo)
	END_SINK_MAP()
};


WebBrowserContainer::WebBrowserContainer() :
	hwnd(0),
	webBrowser(NULL),
    openFromBufferQueued(false)
{
}

WebBrowserContainer::~WebBrowserContainer()
{
	sink->DispEventUnadvise(webBrowser, &DIID_DWebBrowserEvents2);
	delete sink;

	if (webBrowser)
	{
		webBrowser->Release();
		webBrowser = NULL;
	}
}

void WebBrowserContainer::SetDelegate(IUIWebViewDelegate *delegate, UIWebView* webView)
{
	sink->SetDelegate(delegate, webView, this);
}

bool WebBrowserContainer::SaveSnapshot(int32 imageWidth, int32 imageHeight, UIControl* control)
{
    CComPtr<IDispatch> pDispatch;

    // TODO: "If the document object type is not safe for scripting,
    // this method returns successfully but sets ppDisp to NULL. For
    // Internet Explorer 7 and later, the return code is S_FALSE..."

    HRESULT hr = webBrowser->get_Document(&pDispatch);

    if (FAILED(hr))
        return true;

    CComPtr<IHTMLDocument3> spDocument3;
    hr = pDispatch->QueryInterface(IID_IHTMLDocument3, (void**)&spDocument3);

    if (FAILED(hr))
        return true;

    CComPtr<IViewObject2> spViewObject;

    // This used to get the interface from the m_pWebBrowser but that seems
    // to be an undocumented feature, so we get it from the Document instead.
    hr = spDocument3->QueryInterface(IID_IViewObject2, (void**)&spViewObject);

    if (FAILED(hr))
        return true;

    RECTL rcBounds = { 0, 0, imageWidth, imageHeight };
    CImage image;

    image.Create(imageWidth, imageHeight, 24);

    HDC imgDc = image.GetDC();
    hr = spViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, imgDc,
        imgDc, &rcBounds, NULL, NULL, 0);
    image.ReleaseDC();

    if (SUCCEEDED(hr))
    {
        LPCTSTR str = L"test_output.png";
        hr = image.Save(str);
    }

    Vector<uint8> imageData;
    imageData.resize(imageWidth * imageHeight * 3);

    int lineSizeByte = imageWidth * 3;

    for (int i = 0; i < imageHeight; ++i)
    {
        // copy line of pixels
        int startRowIndex = i * lineSizeByte;
        Memcpy(&imageData[startRowIndex], image.GetPixelAddress(0, i), lineSizeByte);
        // need change RbG to GbR
        for (int j = 0; j < lineSizeByte; j += 3)
        {
            uint8 red = imageData[startRowIndex + j];
            imageData[startRowIndex + j] = imageData[startRowIndex + j + 2];
            imageData[startRowIndex + j + 2] = red;
        }
    }

    uint8* rawData = imageData.data();

    Texture* tex = Texture::CreateFromData(FORMAT_RGB888, rawData, static_cast<uint32>(imageWidth), static_cast<uint32>(imageHeight), false);

    Sprite* spr = Sprite::CreateFromTexture(Vector2(100.f, 100.f), tex, Vector2(0.f, 0.f), Vector2(float(imageWidth), float(imageHeight)));

    control->SetSprite(spr, 0);



    return false;
}

bool WebBrowserContainer::Initialize(HWND parentWindow,  UIControl* control)
{
	this->hwnd = parentWindow;

	IOleObject* oleObject = NULL;
	HRESULT hRes = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_INPROC, IID_IOleObject, (void**)&oleObject);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::Inititalize(), CoCreateInstance(CLSID_WebBrowser) failed!, error code %i", hRes);
		return false;
	}

	hRes = oleObject->SetClientSite(this);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::Inititalize(), IOleObject::SetClientSite() failed!, error code %i", hRes);
		oleObject->Release();
		return false;
	}

	// Activating the container.
	RECT rect = {0};
	GetClientRect(hwnd, &rect);
	hRes = oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, this, 0, this->hwnd, &rect);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::InititalizeBrowserContainer(), IOleObject::DoVerb() failed!, error code %i", hRes);
		oleObject->Release();
		return false;
	}

	// Prepare the browser itself.
	hRes = oleObject->QueryInterface(IID_IWebBrowser2, (void**)&this->webBrowser);
	if (FAILED(hRes))
	{
		Logger::Error("WebViewControl::InititalizeBrowserContainer(), IOleObject::QueryInterface(IID_IWebBrowser2) failed!, error code %i", hRes);
		oleObject->Release();
		return false;
	}

	sink = new EventSink();
	EventSink* s = sink;
    sink->SetContainer(this); // TODO test only
    sink->SetWebView(dynamic_cast<UIWebView*>(control));
	hRes = s->DispEventAdvise(webBrowser, &DIID_DWebBrowserEvents2);
	if (FAILED(hRes))
	{
		Logger::Error("WebViewControl::InititalizeBrowserContainer(), EventSink::DispEventAdvise(&DIID_DWebBrowserEvents2) failed!, error code %i", hRes);
		return false;
	}

	// Initialization is OK.
	oleObject->Release();
	return true;
}

HRESULT __stdcall WebBrowserContainer::QueryInterface(REFIID riid, void** ppvObject)
{
	if( !ppvObject )
	{
		return E_POINTER;
	}

	if( riid==IID_IUnknown || riid==IID_IOleWindow || riid==IID_IOleInPlaceSite )
	{
		return *ppvObject = (void*)static_cast<IOleInPlaceSite*>(this), S_OK;
	}

	if( riid==IID_IOleClientSite )
	{
		return *ppvObject = (void*)static_cast<IOleClientSite*>(this), S_OK;
	}

	*ppvObject = NULL;
	return E_NOINTERFACE;
}

HRESULT __stdcall WebBrowserContainer::GetWindow(HWND *phwnd)
{
	if (!phwnd)
	{
		return E_INVALIDARG;
	}

	*phwnd = this->hwnd;
	return S_OK;
}

HRESULT __stdcall WebBrowserContainer::GetWindowContext( IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
		LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	if( !(ppFrame && ppDoc && lprcPosRect && lprcClipRect && lpFrameInfo) )
	{
		return E_INVALIDARG;
	}

	*ppFrame = NULL;
	*ppDoc = NULL;

	GetClientRect( this->hwnd, lprcPosRect );
	GetClientRect( this->hwnd, lprcClipRect );

	lpFrameInfo->fMDIApp = false;
	lpFrameInfo->hwndFrame = this->hwnd;
	lpFrameInfo->haccel = 0;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

bool WebBrowserContainer::OpenUrl(const WCHAR* urlToOpen)
{
	if (!this->webBrowser)
	{
		return false;
	}

    openFromBufferQueued = false;
    bufferToOpen = "";

	BSTR url = SysAllocString(urlToOpen);
	VARIANT empty = {0};
	VariantInit(&empty);

	this->webBrowser->Navigate(url, &empty, &empty, &empty, &empty);
	SysFreeString(url);

	return true;
}

bool WebBrowserContainer::DoOpenBuffer()
{
    if (bufferToOpenPath.IsEmpty() || !openFromBufferQueued)
        return false;

    ScopedComPtr<HtmlMoniker> moniker(new HtmlMoniker());
    moniker->SetHtml(bufferToOpen);
    moniker->SetBaseUrl(StringToWString(bufferToOpenPath.AsURL()));

    ScopedComPtr<IDispatch> docDispatch;
    HRESULT hr = webBrowser->get_Document(&docDispatch);
    if (FAILED(hr) || !docDispatch)
        return true;

    ScopedComQIPtr<IHTMLDocument2> doc(docDispatch);
    if (!doc)
        return true;

    ScopedComQIPtr<IPersistMoniker> perstMon(doc);
    if (!perstMon)
        return true;
    ScopedComQIPtr<IMoniker> htmlMon(moniker);

    hr = perstMon->Load(TRUE, htmlMon, NULL, STGM_READ);

    bufferToOpen = "";
    openFromBufferQueued = false;

    return true;
}

bool WebBrowserContainer::OpenFromBuffer(const String& buffer, const FilePath& basePath)
{
    if (!this->webBrowser)
    {
        return false;
    }

    OpenUrl(L"about:blank"); // webbrowser2 needs about:blank being open before loading from stream

    openFromBufferQueued = true;
    bufferToOpen = buffer;
    bufferToOpenPath = basePath;

    return true;
}

void WebBrowserContainer::UpdateRect()
{
	IOleInPlaceObject* oleInPlaceObject = NULL;
	HRESULT hRes = this->webBrowser->QueryInterface(IID_IOleInPlaceObject, (void**)&oleInPlaceObject);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::SetSize(), IOleObject::QueryInterface(IID_IOleInPlaceObject) failed!, error code %i", hRes);
		return;
	}

	// Update the browser window according to the holder window.
	RECT rect = {0};
	GetClientRect(this->hwnd, &rect);

	hRes = oleInPlaceObject->SetObjectRects(&rect, &rect);
	if (FAILED(hRes))
	{
		Logger::Error("WebBrowserContainer::SetSize(), IOleObject::SetObjectRects() failed!, error code %i", hRes);
		return;
	}

	oleInPlaceObject->Release();
}

void WebBrowserContainer::GetContainerSize(int32* width, int32* height)
{
    // Update the browser window according to the holder window.
    RECT rect = {0};
    GetClientRect(this->hwnd, &rect);
    
    if (width)
    {
        *width = rect.right;
    }

    if (height)
    {
        *height = rect.bottom;
    }
}

WebViewControl::WebViewControl(UIControl* webView):
    browserWindow(0),
    browserContainer(0),
    uiWebView(webView),
    gdiplusToken(0),
    renderToTexture(false)
{
    DVASSERT(webView);
}

WebViewControl::~WebViewControl()
{
	if (browserWindow != 0)
	{
		::DestroyWindow(browserWindow);
        browserWindow = 0;
	}

	SafeDelete(browserContainer);

    Gdiplus::GdiplusShutdown(gdiplusToken);
}

void WebViewControl::SetDelegate(IUIWebViewDelegate *delegate, UIWebView* webView)
{
	browserContainer->SetDelegate(delegate, webView);
}

void WebViewControl::SetRenderToTexture(bool value)
{
    renderToTexture = value;
    if (renderToTexture)
    {
        // TODO
        SetVisible(false, false);
    } 
    else
    {
        // TODO
    }
}

bool WebViewControl::IsRenderToTexture() const
{
    return renderToTexture;
}

void WebViewControl::Initialize(const Rect& rect)
{
	CoreWin32PlatformBase *core = static_cast<CoreWin32PlatformBase *>(Core::Instance());
	DVASSERT(core);

    // Initialize GDI+.
    Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    if (status != Gdiplus::Ok)
    {
        DAVA::Logger::Instance()->Error("Error initialize GDI+ %s(%d)", __FILE__, __LINE__);
    }

	// Create the browser holder window.
	browserWindow = ::CreateWindowEx(0, L"Static", L"", 
        WS_CHILD 
        //| WS_VISIBLE 
        | WS_CLIPCHILDREN, // Excludes the area occupied by child windows when drawing occurs within the parent window. This style is used when creating the parent window.
		0, 0, static_cast<int>(rect.dx), static_cast<int>(rect.dy), core->GetWindow(), NULL, core->GetInstance(), NULL);
	SetRect(rect);

	// Initialize the browser itself.
	InititalizeBrowserContainer();
}

bool WebViewControl::InititalizeBrowserContainer()
{
	HRESULT hRes = ::CoInitialize(NULL);
	if (FAILED(hRes))
	{
		Logger::Error("WebViewControl::InititalizeBrowserContainer(), CoInitialize() failed!");
		return false;
	}

	this->browserContainer= new WebBrowserContainer();
	return browserContainer->Initialize(this->browserWindow, uiWebView);
}

void WebViewControl::OpenURL(const String& urlToOpen)
{
	if (this->browserContainer)
	{
		this->browserContainer->OpenUrl(StringToWString(urlToOpen.c_str()).c_str());
	}
}

void WebViewControl::OpenFromBuffer(const String& string, const FilePath& basePath)
{
    if (this->browserContainer)
    {
        this->browserContainer->OpenFromBuffer(string, basePath);
    }
}

void WebViewControl::SetVisible(bool isVisible, bool /*hierarchic*/)
{
	if (this->browserWindow != 0)
	{
		::ShowWindow(this->browserWindow, false/*isVisible*/);
	}
}

void WebViewControl::SetRect(const Rect& rect)
{
	if (this->browserWindow == 0)
	{
		return;
	}

	RECT browserRect = {0};
	//::GetWindowRect(this->browserWindow, &browserRect);

	browserRect.left = (LONG)(rect.x * Core::GetVirtualToPhysicalFactor());
	browserRect.top  = (LONG)(rect.y * Core::GetVirtualToPhysicalFactor());
	browserRect.left  += (LONG)Core::Instance()->GetPhysicalDrawOffset().x;
	browserRect.top += (LONG)Core::Instance()->GetPhysicalDrawOffset().y;

	browserRect.right = (LONG)(browserRect.left + rect.dx * Core::GetVirtualToPhysicalFactor());
	browserRect.bottom = (LONG)(browserRect.top + rect.dy * Core::GetVirtualToPhysicalFactor());

    if (!IsRenderToTexture())
    {
        // show windows
	    //::SetWindowPos(browserWindow, NULL, browserRect.left, browserRect.top,
		   // browserRect.right - browserRect.left, browserRect.bottom - browserRect.top, SWP_NOZORDER );
    }
	if (this->browserContainer)
	{
		this->browserContainer->UpdateRect();
	}
}

}
