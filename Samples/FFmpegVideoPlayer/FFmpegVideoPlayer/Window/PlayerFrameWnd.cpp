#include "stdafx.h"
#include "PlayerFrameWnd.h"

CPlayerFrameWnd::CPlayerFrameWnd()
    : m_pBtnOK(NULL)
    , m_pBtnExit(NULL)
    , m_nExitCode(0)
{}

CPlayerFrameWnd::~CPlayerFrameWnd()
{
}

LPCTSTR CPlayerFrameWnd::GetWindowClassName() const
{
    return _T("CAboutBox");
}

CControlUI* CPlayerFrameWnd::CreateControl(LPCTSTR pstrClass)
{
    return NULL;
}

void CPlayerFrameWnd::OnFinalMessage(HWND hWnd)
{
    WindowImplBase::OnFinalMessage(hWnd);
}

CDuiString CPlayerFrameWnd::GetSkinFile()
{
    return _T("aboutbox.xml");
}

CDuiString CPlayerFrameWnd::GetSkinFolder()
{
    return CPaintManagerUI::GetResourcePath();
}

LRESULT CPlayerFrameWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
    return 0;
}

LRESULT CPlayerFrameWnd::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
    return 0;
}

LRESULT CPlayerFrameWnd::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (wParam == SC_CLOSE)
    {
        bHandled = TRUE;
        return 0;
    }
    return __super::OnSysCommand(uMsg, wParam, lParam, bHandled);
}

LRESULT CPlayerFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CPlayerFrameWnd::ResponseDefaultKeyEvent(WPARAM wParam)
{
    if (wParam == VK_RETURN)
    {
        return FALSE;
    }
    else if (wParam == VK_ESCAPE)
    {
        return TRUE;
    }
    return FALSE;
}

void CPlayerFrameWnd::OnTimer(TNotifyUI& msg)
{
}


void CPlayerFrameWnd::InitWindow()
{
    m_pBtnOK  = static_cast<CButtonUI*>(m_pm.FindControl(UI_NAME_ABOUTBOX_BUTTONOK));
    m_pBtnExit = static_cast<CButtonUI*>(m_pm.FindControl(UI_NAME_ABOUTBOX_BUTTONEXIT));
    m_pTextVersion = static_cast<CTextUI*>(m_pm.FindControl(UI_NAME_ABOUTBOX_TEXTVERSION));
    m_pTextWebSize = static_cast<CTextUI*>(m_pm.FindControl(UI_NAME_ABOUTBOX_TEXTWEBSITE));

    m_pWndShadow = new DuiLib::CShadowUI;
    m_pWndShadow->Create(&m_pm);
    m_pWndShadow->SetImage(L"img\\shadow.png");
    RECT rcShadow = {8,8,10,10};
    m_pWndShadow->SetShadowCorner(rcShadow);
}

void CPlayerFrameWnd::OnPrepare(TNotifyUI& msg)
{
}

void CPlayerFrameWnd::Notify(TNotifyUI& msg)
{
    if (_tcsicmp(msg.sType, _T("windowinit")) == 0)
    {
        OnPrepare(msg);
    }
    else if (_tcsicmp(msg.sType, _T("killfocus")) == 0)
    {
    }
    else if (_tcsicmp(msg.sType, _T("click")) == 0)
    {
        if (msg.pSender == m_pBtnExit || msg.pSender == m_pBtnOK)
        {
            Close();
        }
    }
    else if (_tcsicmp(msg.sType, DUI_MSGTYPE_LINK) == 0)
    {
        if (msg.pSender == m_pTextWebSize)
        {
            CDuiString strURL = msg.pSender->GetUserData();
            if (strURL.IsEmpty())
                strURL = L"about:blank";
            ShellExecute(NULL, L"open", strURL.GetData(), NULL, NULL, SW_SHOWNORMAL);
        }
    }
    else if (_tcsicmp(msg.sType, _T("return")) == 0)
    {
    }
    else if (_tcsicmp(msg.sType, _T("timer")) == 0)
    {
        return OnTimer(msg);
    }
}

LRESULT CPlayerFrameWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
    return 0;
}

void CPlayerFrameWnd::ShowModal(HWND hWndParent)
{
    //
    Create(hWndParent, L"MSGBOX", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, WS_EX_TOOLWINDOW);
    CenterWindow(); 
    ShowWindow();

    WindowImplBase::ShowModal();

}
