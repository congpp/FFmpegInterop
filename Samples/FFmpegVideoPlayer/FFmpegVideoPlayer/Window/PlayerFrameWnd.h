#pragma once
#include <memory>

#include "UIlib.h"
using namespace DuiLib;

#define UI_NAME_ABOUTBOX_BUTTONOK     L"aboutboxbuttonok"
#define UI_NAME_ABOUTBOX_BUTTONEXIT   L"aboutboxbuttonexit"
#define UI_NAME_ABOUTBOX_TEXTVERSION  L"aboutboxtextversion"
#define UI_NAME_ABOUTBOX_TEXTWEBSITE  L"aboutboxtextwebsite"
#define UI_NAME_ABOUTBOX_TEXTSOFTNAME L"aboutboxtextsoftname"

class CPlayerFrameWnd : public WindowImplBase
{
public:
    CPlayerFrameWnd();
    ~CPlayerFrameWnd();

public:

    LPCTSTR GetWindowClassName() const;

    virtual void OnFinalMessage(HWND hWnd);

    virtual void InitWindow();

    virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);

    virtual CDuiString GetSkinFile();

    virtual CDuiString GetSkinFolder();

    virtual CControlUI* CreateControl(LPCTSTR pstrClass);

    virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    virtual LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    virtual LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    void ShowModal(HWND hWndParent);

protected:

    void Notify(TNotifyUI& msg);
    void OnPrepare(TNotifyUI& msg);
    void OnTimer(TNotifyUI& msg);


    CButtonUI* m_pBtnOK;
    CButtonUI* m_pBtnExit;
    CTextUI*   m_pTextVersion;
    CTextUI*   m_pTextWebSize;

    DuiLib::CShadowUI* m_pWndShadow;
    int m_nExitCode;
};
