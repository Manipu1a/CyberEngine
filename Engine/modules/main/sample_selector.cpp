#include "sample_selector.h"
#include <windows.h>

namespace
{
    constexpr int ID_LISTBOX = 1001;
    constexpr int ID_OK      = IDOK;
    constexpr int ID_CANCEL  = IDCANCEL;

    struct SelectorState
    {
        int selected = -1;
        bool done = false;
        HWND list = nullptr;
    };

    LRESULT CALLBACK selector_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        SelectorState* st = reinterpret_cast<SelectorState*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        switch (msg)
        {
        case WM_COMMAND:
        {
            const int id   = LOWORD(wp);
            const int code = HIWORD(wp);
            if (id == ID_OK || (id == ID_LISTBOX && code == LBN_DBLCLK))
            {
                if (st && st->list)
                {
                    int cur = (int)SendMessageW(st->list, LB_GETCURSEL, 0, 0);
                    st->selected = (cur == LB_ERR) ? -1 : cur;
                }
                if (st) st->done = true;
                DestroyWindow(hwnd);
                return 0;
            }
            if (id == ID_CANCEL)
            {
                if (st) { st->selected = -1; st->done = true; }
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            if (st) { st->selected = -1; st->done = true; }
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            if (st) st->done = true;
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
}

int show_sample_selector(HINSTANCE hInst, const Cyber::Samples::SampleEntry* entries, int count)
{
    if (count <= 0 || entries == nullptr)
    {
        MessageBoxW(nullptr,
            L"No samples were compiled into this build.\n"
            L"Enable at least one --build_xxx=y option or reconfigure without any flags to build them all.",
            L"CyberEngine", MB_ICONERROR | MB_OK);
        return -1;
    }
    if (count == 1)
    {
        return 0;
    }

    const wchar_t* kClassName = L"CyberSampleSelector";

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = selector_proc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    const int win_w = 420;
    const int win_h = 300;
    const int screen_w = GetSystemMetrics(SM_CXSCREEN);
    const int screen_h = GetSystemMetrics(SM_CYSCREEN);
    const int x = (screen_w - win_w) / 2;
    const int y = (screen_h - win_h) / 2;

    SelectorState state;

    HWND hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        kClassName,
        L"CyberEngine - Select Sample",
        WS_CAPTION | WS_SYSMENU | WS_POPUP,
        x, y, win_w, win_h,
        nullptr, nullptr, hInst, nullptr);

    if (!hwnd)
    {
        UnregisterClassW(kClassName, hInst);
        return -1;
    }

    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&state));

    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    HWND label = CreateWindowExW(0, L"STATIC",
        L"Choose a sample to launch:",
        WS_CHILD | WS_VISIBLE,
        12, 10, 380, 18,
        hwnd, nullptr, hInst, nullptr);
    SendMessageW(label, WM_SETFONT, (WPARAM)font, TRUE);

    HWND list = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX",
        nullptr,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
            LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
        12, 34, 380, 180,
        hwnd, (HMENU)(INT_PTR)ID_LISTBOX, hInst, nullptr);
    SendMessageW(list, WM_SETFONT, (WPARAM)font, TRUE);
    for (int i = 0; i < count; ++i)
    {
        SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)entries[i].display_name);
    }
    SendMessageW(list, LB_SETCURSEL, 0, 0);
    state.list = list;

    HWND ok = CreateWindowExW(0, L"BUTTON", L"OK",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
        224, 224, 80, 28,
        hwnd, (HMENU)(INT_PTR)ID_OK, hInst, nullptr);
    SendMessageW(ok, WM_SETFONT, (WPARAM)font, TRUE);

    HWND cancel = CreateWindowExW(0, L"BUTTON", L"Cancel",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        312, 224, 80, 28,
        hwnd, (HMENU)(INT_PTR)ID_CANCEL, hInst, nullptr);
    SendMessageW(cancel, WM_SETFONT, (WPARAM)font, TRUE);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SetFocus(list);

    MSG msg;
    while (!state.done && GetMessageW(&msg, nullptr, 0, 0))
    {
        if (!IsDialogMessageW(hwnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    UnregisterClassW(kClassName, hInst);
    return state.selected;
}
