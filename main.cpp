#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>

namespace {
constexpr wchar_t kWindowClass[] = L"RootSkidWindowClass";
constexpr int kActionButtonId = 1001;
constexpr COLORREF kBackground = RGB(246, 248, 252);
HFONT g_titleFont = nullptr;
HFONT g_bodyFont = nullptr;
HWND g_statusLabel = nullptr;

HFONT createFont(int points, int weight) {
    const HDC screen = GetDC(nullptr);
    const int height = -MulDiv(points, GetDeviceCaps(screen, LOGPIXELSY), 72);
    ReleaseDC(nullptr, screen);
    return CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

void positionControls(HWND window) {
    RECT area{};
    GetClientRect(window, &area);
    const int width = area.right;
    MoveWindow(GetDlgItem(window, 100), 32, 28, width - 64, 42, TRUE);
    MoveWindow(GetDlgItem(window, 101), 34, 82, width - 68, 48, TRUE);
    MoveWindow(GetDlgItem(window, kActionButtonId), 34, 148, 150, 38, TRUE);
    if (g_statusLabel) MoveWindow(g_statusLabel, 34, 207, width - 68, 28, TRUE);
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        g_titleFont = createFont(22, FW_SEMIBOLD);
        g_bodyFont = createFont(10, FW_NORMAL);
        const HWND title = CreateWindowW(L"STATIC", L"RootSkid", WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, window, reinterpret_cast<HMENU>(100), nullptr, nullptr);
        const HWND description = CreateWindowW(L"STATIC", L"The application is running correctly.",
            WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, window, reinterpret_cast<HMENU>(101), nullptr, nullptr);
        const HWND button = CreateWindowW(L"BUTTON", L"Run check",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, window,
            reinterpret_cast<HMENU>(kActionButtonId), nullptr, nullptr);
        g_statusLabel = CreateWindowW(L"STATIC", L"Status: Ready", WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, window, reinterpret_cast<HMENU>(102), nullptr, nullptr);
        SendMessageW(title, WM_SETFONT, reinterpret_cast<WPARAM>(g_titleFont), TRUE);
        SendMessageW(description, WM_SETFONT, reinterpret_cast<WPARAM>(g_bodyFont), TRUE);
        SendMessageW(button, WM_SETFONT, reinterpret_cast<WPARAM>(g_bodyFont), TRUE);
        SendMessageW(g_statusLabel, WM_SETFONT, reinterpret_cast<WPARAM>(g_bodyFont), TRUE);
        positionControls(window);
        return 0;
    }
    case WM_SIZE: positionControls(window); return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == kActionButtonId && HIWORD(wParam) == BN_CLICKED) {
            SetWindowTextW(g_statusLabel, L"Status: Check completed successfully");
            return 0;
        }
        break;
    case WM_CTLCOLORSTATIC: {
        const HDC dc = reinterpret_cast<HDC>(wParam);
        SetBkColor(dc, kBackground);
        SetTextColor(dc, RGB(28, 36, 50));
        return reinterpret_cast<LRESULT>(GetStockObject(NULL_BRUSH));
    }
    case WM_ERASEBKGND: {
        RECT area{};
        GetClientRect(window, &area);
        const HBRUSH brush = CreateSolidBrush(kBackground);
        FillRect(reinterpret_cast<HDC>(wParam), &area, brush);
        DeleteObject(brush);
        return 1;
    }
    case WM_DESTROY:
        DeleteObject(g_titleFont);
        DeleteObject(g_bodyFont);
        PostQuitMessage(0);
        return 0;
    default: break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&controls);
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.lpszClassName = kWindowClass;
    if (!RegisterClassExW(&windowClass)) return 1;
    const HWND window = CreateWindowExW(0, kWindowClass, L"RootSkid", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 340, nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    const BOOL darkCaption = TRUE;
    DwmSetWindowAttribute(window, 20, &darkCaption, sizeof(darkCaption));
    ShowWindow(window, showCommand);
    UpdateWindow(window);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}
