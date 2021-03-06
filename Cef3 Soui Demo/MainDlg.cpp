// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
	
#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif
#include "CefWebView/CefClientApp.h"
#include "CefWebView/Cef3WebView.h"
#include "CefWebView/ExtendEvents.h"

#define CEF_BROWSER_CLOSE_EVENT 100
	
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	#ifdef DWMBLUR	//win7毛玻璃开关
	MARGINS mar = {5,5,30,5};
	DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	#endif

	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;

	SOUI::SButton *pBtnGo = FindChildByName2<SOUI::SButton>(L"btn_go");
	if (pBtnGo) {
		pBtnGo->GetEventSet()->subscribeEvent(SOUI::EVT_CMD, Subscriber(&CMainDlg::OnLoadURL, this));
	}

	SOUI::SButton *pBtnRunJs = FindChildByName2<SOUI::SButton>(L"btn_run_js");
	if (pBtnRunJs) {
		pBtnRunJs->GetEventSet()->subscribeEvent(SOUI::EVT_CMD, Subscriber(&CMainDlg::OnRunJavaScript, this));
	}

	SOUI::SCefWebView *pCefBrowser = FindChildByName2<SOUI::SCefWebView>(L"cef_browser");
	if (pCefBrowser) {
		pCefBrowser->GetEventSet()->subscribeEvent(EVT_WEBVIEW_NOTIFY, Subscriber(&CMainDlg::OnWebViewNotify, this));
	}

	return 0;
}


void CMainDlg::OnClose()
{
	// 关闭窗口之前，必须要先确保所有的浏览器实例已经全部关闭了
	if (CefClientApp::GetOpenedBrowserCount() != 0) {
		SetTimer(CEF_BROWSER_CLOSE_EVENT, 1000);
		SOUI::SCefWebView *pCefBrowser = FindChildByName2<SOUI::SCefWebView>(L"cef_browser");
		if (pCefBrowser != NULL) {
			pCefBrowser->Close();
		}
	} else {
		CSimpleWnd::DestroyWindow();
	}
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	if (!m_bLayoutInited) return;
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

void CMainDlg::OnTimer(UINT_PTR idEvent)
{
	if (idEvent == CEF_BROWSER_CLOSE_EVENT) {
		if (CefClientApp::GetOpenedBrowserCount() != 0) {
			SetTimer(CEF_BROWSER_CLOSE_EVENT, 1000);
		} else {
			CSimpleWnd::DestroyWindow();
		}
	}
}

// 载入网址
bool CMainDlg::OnLoadURL(SOUI::EventArgs *pEvt)
{
	SOUI::SEdit *pEditURL = FindChildByName2<SOUI::SEdit>(L"edit_input_url");
	SOUI::SCefWebView *pCefBrowser = FindChildByName2<SOUI::SCefWebView>(L"cef_browser");
	if (pEditURL && pCefBrowser) {
		if (!pCefBrowser->Open()) {
			STRACE(_T("Eror: Can not create cef browser"));
			return true;
		}
		SStringW strURL = pEditURL->GetWindowText();
		pCefBrowser->LoadURL(strURL);
		return true;
	}
	return true;
}

// 执行 JavaScript 代码
bool CMainDlg::OnRunJavaScript(SOUI::EventArgs *pEvt)
{
	SOUI::SEdit *pEditURL = FindChildByName2<SOUI::SEdit>(L"edit_input_url");
	SOUI::SCefWebView *pCefBrowser = FindChildByName2<SOUI::SCefWebView>(L"cef_browser");
	if (pEditURL && pCefBrowser) {
		SStringW strJs = pEditURL->GetWindowText();
		pCefBrowser->ExecJavaScript(strJs);
	}
	return true;
}

// 获得 Js 的通知信息，显示结果
bool CMainDlg::OnWebViewNotify(SOUI::EventArgs *pEvt)
{
	SOUI::EventWebViewNotify *pev = (SOUI::EventWebViewNotify*)pEvt;
	SStringW args;
	for (size_t i = 0; i < pev->Arguments.GetCount(); ++i) {
		if (i != 0) {
			args += _T(", ");
		}
		args += pev->Arguments[i];
	}

	SOUI::SMessageBox(m_hWnd, args, pev->MessageName, MB_OK);
	return true;
}