#include "CEFWebViewControl.h"
#include "CEFWebViewControlProxy.h"

namespace DAVA
{
std::unique_ptr<IWebViewControl> IWebViewControl::Create(UIWebView& uiWebView)
{
    return std::make_unique<WebViewControl>(uiWebView);
}

WebViewControl::WebViewControl(UIWebView& uiWebView)
    : impl(new CEFWebViewControl(uiWebView))
    , cefController(impl)
{
    if (!cefController.IsCEFInitialized())
    {
        impl = nullptr;
    }
}

WebViewControl::~WebViewControl()
{
    if (impl)
    {
        impl->Deinitialize();
        impl = nullptr;
    }
}

void WebViewControl::Initialize(const Rect& rect)
{
    if (impl != nullptr)
    {
        impl->Initialize(rect);
    }
}

void WebViewControl::OpenURL(const String& url)
{
    if (impl != nullptr)
    {
        impl->OpenURL(url);
    }
}

void WebViewControl::LoadHtmlString(const WideString& htmlString)
{
    if (impl != nullptr)
    {
        impl->LoadHtmlString(htmlString);
    }
}

void WebViewControl::OpenFromBuffer(const String& htmlString, const FilePath& basePath)
{
    if (impl != nullptr)
    {
        impl->OpenFromBuffer(htmlString, basePath);
    }
}

void WebViewControl::ExecuteJScript(const String& scriptString)
{
    if (impl != nullptr)
    {
        impl->ExecuteJScript(scriptString);
    }
}

void WebViewControl::DeleteCookies(const String& url)
{
    if (impl != nullptr)
    {
        impl->DeleteCookies(url);
    }
}

String WebViewControl::GetCookie(const String& url, const String& name) const
{
    return impl ? impl->GetCookie(url, name) : "";
}

Map<String, String> WebViewControl::GetCookies(const String& url) const
{
    return impl ? impl->GetCookies(url) : Map<String, String>();
}

void WebViewControl::SetRect(const Rect& rect)
{
    if (impl != nullptr)
    {
        impl->SetRect(rect);
    }
}

void WebViewControl::SetVisible(bool isVisible, bool hierarchic)
{
    if (impl != nullptr)
    {
        impl->SetVisible(isVisible, hierarchic);
    }
}

void WebViewControl::SetBackgroundTransparency(bool enabled)
{
    if (impl != nullptr)
    {
        impl->SetBackgroundTransparency(enabled);
    }
}

void WebViewControl::SetDelegate(IUIWebViewDelegate* webViewDelegate, UIWebView* webView)
{
    if (impl != nullptr)
    {
        impl->SetDelegate(webViewDelegate, webView);
    }
}

void WebViewControl::SetRenderToTexture(bool value)
{
    if (impl != nullptr)
    {
        impl->SetRenderToTexture(value);
    }
}

bool WebViewControl::IsRenderToTexture() const
{
    return impl ? impl->IsRenderToTexture() : false;
}

void WebViewControl::Draw(const UIGeometricData& geometricData)
{
    if (impl != nullptr)
    {
        impl->Draw(geometricData);
    }
}

void WebViewControl::Input(UIEvent* currentInput)
{
    if (impl != nullptr)
    {
        impl->Input(currentInput);
    }
}

void WebViewControl::Update()
{
    if (impl != nullptr)
    {
        cefController.Update();
        impl->Update();
    }
}

} // namespace DAVA
