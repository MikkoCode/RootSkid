#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <winsvc.h>
#include <string>

namespace {
constexpr wchar_t kWindowClass[] = L"RootSkidWindowClass";
constexpr int kActionButtonId = 1001;
constexpr COLORREF kBackground = RGB(246, 248, 252);
HFONT g_titleFont = nullptr;
HFONT g_bodyFont = nullptr;
HWND g_statusLabel = nullptr;
HWND g_results = nullptr, g_log = nullptr;

void addResult(const wchar_t* level,const wchar_t* check,const wchar_t* result,const wchar_t* fix){ LVITEMW x{}; x.mask=LVIF_TEXT; x.iItem=ListView_GetItemCount(g_results); x.pszText=const_cast<wchar_t*>(level); int row=ListView_InsertItem(g_results,&x); ListView_SetItemText(g_results,row,1,const_cast<wchar_t*>(check)); ListView_SetItemText(g_results,row,2,const_cast<wchar_t*>(result)); ListView_SetItemText(g_results,row,3,const_cast<wchar_t*>(fix)); }
void logLine(const std::wstring& s){ int n=GetWindowTextLengthW(g_log); SendMessageW(g_log,EM_SETSEL,n,n); std::wstring line=s+L"\r\n"; SendMessageW(g_log,EM_REPLACESEL,0,reinterpret_cast<LPARAM>(line.c_str())); }
bool running(const wchar_t* name){ SC_HANDLE m=OpenSCManagerW(nullptr,nullptr,SC_MANAGER_CONNECT),v=m?OpenServiceW(m,name,SERVICE_QUERY_STATUS):nullptr; SERVICE_STATUS st{}; bool ok=v&&QueryServiceStatus(v,&st)&&st.dwCurrentState==SERVICE_RUNNING; if(v)CloseServiceHandle(v);if(m)CloseServiceHandle(m);return ok; }
void scan(){ ListView_DeleteAllItems(g_results);SetWindowTextW(g_log,L"");logLine(L"[scan] Scan started"); struct C{const wchar_t*k,*n;}cs[]={{L"WinDefend",L"Microsoft Defender"},{L"mpssvc",L"Windows Firewall"},{L"wuauserv",L"Windows Update"}};int issues=0;for(auto&c:cs){logLine(std::wstring(L"[check] ")+c.n);bool ok=running(c.k);addResult(ok?L"OK":L"Action",c.n,ok?L"Running":L"Stopped or unavailable",ok?L"None":L"Start in Windows Services");issues+=!ok;}DWORD u=0,z=sizeof u,t=0;bool ok=RegGetValueW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",L"EnableLUA",RRF_RT_REG_DWORD,&t,&u,&z)==ERROR_SUCCESS&&u;addResult(ok?L"OK":L"Critical",L"User Account Control",ok?L"Enabled":L"Disabled",ok?L"None":L"Enable UAC");issues+=!ok;z=sizeof u;bool proxy=RegGetValueW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",L"ProxyEnable",RRF_RT_REG_DWORD,&t,&u,&z)==ERROR_SUCCESS&&u;addResult(proxy?L"Review":L"OK",L"Internet proxy",proxy?L"Manual proxy enabled":L"No manual proxy",proxy?L"Confirm it is expected":L"None");issues+=proxy;std::wstring done=L"Scan complete: 5 checks, "+std::to_wstring(issues)+L" need attention.";SetWindowTextW(g_statusLabel,done.c_str());logLine(done); }

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
    MoveWindow(GetDlgItem(window, kActionButtonId),34,140,140,38,TRUE);
    if(g_statusLabel)MoveWindow(g_statusLabel,34,188,width-68,24,TRUE); if(g_results)MoveWindow(g_results,34,220,width-68,210,TRUE); if(g_log)MoveWindow(g_log,34,440,width-68,120,TRUE);
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
        const HWND button = CreateWindowW(L"BUTTON", L"Scan now",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, window,
            reinterpret_cast<HMENU>(kActionButtonId), nullptr, nullptr);
        g_statusLabel = CreateWindowW(L"STATIC", L"Status: Ready", WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, window, reinterpret_cast<HMENU>(102), nullptr, nullptr);
        SendMessageW(title, WM_SETFONT, reinterpret_cast<WPARAM>(g_titleFont), TRUE);
        SendMessageW(description, WM_SETFONT, reinterpret_cast<WPARAM>(g_bodyFont), TRUE);
        SendMessageW(button, WM_SETFONT, reinterpret_cast<WPARAM>(g_bodyFont), TRUE);
        SendMessageW(g_statusLabel, WM_SETFONT, reinterpret_cast<WPARAM>(g_bodyFont), TRUE);
        g_results=CreateWindowW(WC_LISTVIEWW,L"",WS_CHILD|WS_VISIBLE|WS_BORDER|LVS_REPORT|LVS_SINGLESEL,0,0,0,0,window,nullptr,nullptr,nullptr); g_log=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"Activity log appears here after a scan.\r\n",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL,0,0,0,0,window,nullptr,nullptr,nullptr); const wchar_t*h[]={L"Level",L"Check",L"Result",L"Fix list"};int widths[]={70,170,220,280};for(int i=0;i<4;i++){LVCOLUMNW c{};c.mask=LVCF_TEXT|LVCF_WIDTH;c.pszText=const_cast<wchar_t*>(h[i]);c.cx=widths[i];ListView_InsertColumn(g_results,i,&c);}ListView_SetExtendedListViewStyle(g_results,LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES); SendMessageW(g_results,WM_SETFONT,reinterpret_cast<WPARAM>(g_bodyFont),TRUE);SendMessageW(g_log,WM_SETFONT,reinterpret_cast<WPARAM>(g_bodyFont),TRUE);
        positionControls(window);
        return 0;
    }
    case WM_SIZE: positionControls(window); return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == kActionButtonId && HIWORD(wParam) == BN_CLICKED) {
            scan();
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
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 640, nullptr, nullptr, instance, nullptr);
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
