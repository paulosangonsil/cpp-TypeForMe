#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <string>
#include <vector>
#include <utility>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")

#ifndef MSFTEDIT_CLASS
#define MSFTEDIT_CLASS L"RICHEDIT50W"
#endif

#define IDC_EDIT_TEXT 1001
#define IDC_BTN_SEND 1002
#define IDC_SLIDER_DELAY 1003
#define IDC_LABEL_DELAY 1004
#define IDC_LABEL_INSTRUCT 1005

HWND hEdit = nullptr, hSlider = nullptr;
HINSTANCE hInst = nullptr;
bool g_isTyping = false;

struct TypingRequest {
    std::wstring text;

    int delayMs;

    HWND targetWindow;
};

static std::wstring GetEditText() {
    auto len = GetWindowTextLengthW(hEdit);

    if (len <= 0) return {};

    std::vector<wchar_t> buffer(len + 1);

    GetWindowTextW(hEdit, buffer.data(), len + 1);

    return std::wstring(buffer.data());
}

static void ExecuteTyping(const TypingRequest& req) {
    if (req.text.empty() || !IsWindow(req.targetWindow)) return;

    SetForegroundWindow(req.targetWindow);
    Sleep(50);

    std::vector<INPUT> inputs;
    inputs.reserve(req.text.size() * 2);

    for (size_t i = 0; i < req.text.size(); ++i) {
        auto ch = req.text[i];
        
        if (ch == L'\r') {
            if (i + 1 < req.text.size() && req.text[i + 1] == L'\n') {
                continue;
            }

            ch = L'\n';
        }

        INPUT down = {}, up = {};
        down.type = INPUT_KEYBOARD;
        up.type = INPUT_KEYBOARD;

        if (ch == L'\n') {
            down.ki.wVk = VK_RETURN;
            down.ki.dwFlags = 0;
            up.ki.wVk = VK_RETURN;
            up.ki.dwFlags = KEYEVENTF_KEYUP;
        }
        else {
            down.ki.wVk = 0;
            down.ki.wScan = static_cast<WORD>(ch);
            down.ki.dwFlags = KEYEVENTF_UNICODE;
            
            up.ki.wVk = 0;
            up.ki.wScan = static_cast<WORD>(ch);
            up.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        }

        inputs.push_back(down);
        inputs.push_back(up);
    }

    for (size_t i = 0; i < inputs.size(); ++i) {
        if (SendInput(1, &inputs[i], sizeof(INPUT)) != 1) break;
        else if ((inputs[i].ki.dwFlags & KEYEVENTF_KEYUP) != 0 && req.delayMs > 0) {
            Sleep(req.delayMs);
        }
    }
}

static void UpdateDelayLabel(const HWND& hWnd) {
    auto pos = static_cast<int>(SendMessageW(hSlider, TBM_GETPOS, 0, 0));

    wchar_t label[64] = {};

    wsprintfW(label, L"Delay: %d ms/key", pos);

    SetWindowTextW(GetDlgItem(hWnd, IDC_LABEL_DELAY), label);
}

static void HandleSendClick(const HWND& hWnd) {
    if (g_isTyping) return;

    auto text = GetEditText();
    if (text.empty()) {
        MessageBoxW(hWnd, L"Enter text first.", L"Info", MB_ICONINFORMATION);
        return;
    }

    auto delay = static_cast<int>(SendMessageW(hSlider, TBM_GETPOS, 0, 0));
    g_isTyping = true;

    ShowWindow(hWnd, SW_HIDE);
    Sleep(5000);

    auto hTarget = GetForegroundWindow();
    if (hTarget == nullptr || hTarget == hWnd) {
        ShowWindow(hWnd, SW_SHOW);
        SetForegroundWindow(hWnd);
        MessageBoxW(hWnd, L"Focus a target window after clicking Send, then wait.", L"Error", MB_ICONERROR);

        g_isTyping = false;

        return;
    }

    TypingRequest req{std::move(text), delay, hTarget};
    ExecuteTyping(req);

    ShowWindow(hWnd, SW_SHOW);
    SetForegroundWindow(hWnd);
    g_isTyping = false;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            RECT rc = {};
            GetClientRect(hWnd, &rc);
            int width = rc.right - rc.left, height = rc.bottom - rc.top;

            CreateWindowExW(0, L"STATIC", L"Text to inject:", WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 10, width - 20, 20, hWnd, nullptr, hInst, nullptr);

            // Upgraded to RICHEDIT50W for robust multiline and clipboard paste support
            hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, MSFTEDIT_CLASS, L"",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL,
                10, 35, width - 20, height - 145, hWnd, reinterpret_cast<HMENU>(IDC_EDIT_TEXT), hInst, nullptr);

            CreateWindowExW(0, L"STATIC", L"Click Send, then focus target within 5 seconds.",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, height - 105, width - 20, 20, hWnd, reinterpret_cast<HMENU>(IDC_LABEL_INSTRUCT), hInst, nullptr);

            hSlider = CreateWindowExW(0, L"msctls_trackbar32", L"",
                WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS,
                10, height - 80, width - 110, 30, hWnd, reinterpret_cast<HMENU>(IDC_SLIDER_DELAY), hInst, nullptr);

            SendMessageW(hSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 500));
            SendMessageW(hSlider, TBM_SETPOS, TRUE, 50);

            CreateWindowExW(0, L"STATIC", L"Delay: 50 ms/key", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                width - 100, height - 80, 90, 20, hWnd, reinterpret_cast<HMENU>(IDC_LABEL_DELAY), hInst, nullptr);

            CreateWindowExW(0, L"BUTTON", L"Send", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                width - 100, height - 45, 90, 30, hWnd, reinterpret_cast<HMENU>(IDC_BTN_SEND), hInst, nullptr);

            break;
        }

        case WM_SIZE: {
            if (wParam == SIZE_MINIMIZED) break;

            auto width = LOWORD(lParam);
            auto height = HIWORD(lParam);

            MoveWindow(GetDlgItem(hWnd, IDC_EDIT_TEXT), 10, 35, width - 20, height - 145, TRUE);
            MoveWindow(GetDlgItem(hWnd, IDC_LABEL_INSTRUCT), 10, height - 105, width - 20, 20, TRUE);
            MoveWindow(hSlider, 10, height - 80, width - 110, 30, TRUE);
            MoveWindow(GetDlgItem(hWnd, IDC_LABEL_DELAY), width - 100, height - 80, 90, 20, TRUE);
            MoveWindow(GetDlgItem(hWnd, IDC_BTN_SEND), width - 100, height - 45, 90, 30, TRUE);

            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_BTN_SEND && HIWORD(wParam) == BN_CLICKED) {
                HandleSendClick(hWnd);
            }

            break;
        }

        case WM_HSCROLL: {
            if (reinterpret_cast<HWND>(lParam) == hSlider) {
                UpdateDelayLabel(hWnd);
            }

            break;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    INITCOMMONCONTROLSEX icc = { static_cast<DWORD>(sizeof(icc)), ICC_BAR_CLASSES };
    InitCommonControlsEx(&icc);

    // Load Rich Edit 5.0 (Unicode, robust multiline and clipboard support)
    LoadLibraryW(L"Msftedit.dll");

    hInst = hInstance;
    const wchar_t className[] = L"TextInjectorClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    RegisterClassW(&wc);

    HWND hWnd = CreateWindowExW(0, className, L"Unicode Text Injector",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 560, 420,
        nullptr, nullptr, hInstance, nullptr);

    if (hWnd == nullptr) return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
