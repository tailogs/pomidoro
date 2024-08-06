#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>
#include <stdio.h>
#include <shellapi.h> // Для системного трея
#include "resource.h"

#define VERSION "1.1.1"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void StartTimer(HWND, int);
void StopTimer(HWND);
void UpdateTimeDisplay(HWND, int);
void ShowAboutDialog(HWND);

HFONT hFont16, hFont32, hFont64;
HBRUSH hBrush;
COLORREF bgColor = RGB(30, 30, 30);
COLORREF textColor = RGB(200, 200, 200);
HWND hToggleButton, hStatusLabel, hTimeLabel, hPomodoroInput, hBreakInput;
bool isRunning = false;
bool isPomodoro = true;
int remainingTime = 0;
int pomodoroInterval = POMODORO_DEFAULT * 60 * 1000; // по умолчанию 25 минут
int breakInterval = BREAK_DEFAULT * 60 * 1000;       // по умолчанию 5 минут

char textToogleButton[6] = "Start";

NOTIFYICONDATA nid; // Для системного трея

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    MSG msg;
    WNDCLASS wc = {0};
    HWND hwnd;

    HWND myConsole = GetConsoleWindow(); //window handle
    ShowWindow(myConsole, 0); //handle window

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "PomodoroClass";
    wc.hbrBackground = CreateSolidBrush(bgColor);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

    if (!RegisterClass(&wc)) {
        return -1;
    }

    hwnd = CreateWindow(
        "PomodoroClass",
        "Pomodoro Timer",
        ((WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_VISIBLE),
        CW_USEDEFAULT, CW_USEDEFAULT, 280, 280,
        NULL, NULL, hInstance, NULL
    );

    HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MYMENU));
    SetMenu(hwnd, hMenu);

    hFont16 = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    hFont32 = CreateFont(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    // Создание шрифта
    hFont64 = CreateFont(
        64,                        // Высота шрифта
        0,                         // Ширина шрифта (0 означает автоматическая ширина)
        0,                         // Угол наклона шрифта
        0,                         // Угол наклона шрифта
        FW_BOLD,                   // Толщина шрифта
        FALSE,                     // Курсив
        FALSE,                     // Подчеркивание
        FALSE,                     // Перечеркивание
        ANSI_CHARSET,              // Набор символов
        OUT_DEFAULT_PRECIS,        // Точность вывода
        CLIP_DEFAULT_PRECIS,       // Точность обрезки
        DEFAULT_QUALITY,           // Качество вывода
        DEFAULT_PITCH | FF_SWISS,  // Стиль шрифта и семья шрифтов
        "Segoe UI"                 // Название шрифта
    );
    hBrush = CreateSolidBrush(bgColor);

    // Инициализация иконки трея
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    nid.uCallbackMessage = WM_USER + 1;
    strcpy(nid.szTip, "Pomodoro Timer");
    Shell_NotifyIcon(NIM_ADD, &nid);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hFont16);
    DeleteObject(hFont32);
    DeleteObject(hFont64);
    DeleteObject(hBrush);

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int pomodoroCount = 0;

    switch (msg) {
        case WM_CREATE:
            hToggleButton = CreateWindow("BUTTON", "Start", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | SS_CENTER | BS_OWNERDRAW, 180, 180, 60, 30, hwnd, (HMENU)IDM_TOGGLE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hTimeLabel = CreateWindow("STATIC", "", WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE, 10, 10, 255, 60, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hStatusLabel = CreateWindow("STATIC", "Status: Waiting", WS_VISIBLE | WS_CHILD | SS_CENTER, 30, 180, 140, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hTimeLabel = CreateWindow("STATIC", "00:00", WS_VISIBLE | WS_CHILD | SS_CENTER, 10, 10, 255, 60, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            CreateWindow("STATIC", "Pomodoro (min):", WS_VISIBLE | WS_CHILD, 30, 90, 140, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hPomodoroInput = CreateWindow("EDIT", "25", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_CENTER, 180, 90, 60, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            CreateWindow("STATIC", "Break (min):", WS_VISIBLE | WS_CHILD, 30, 130, 140, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBreakInput = CreateWindow("EDIT", "5", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_CENTER, 180, 130, 60, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            SendMessage(hToggleButton, WM_SETFONT, (WPARAM)hFont16, TRUE);
            SendMessage(hStatusLabel, WM_SETFONT, (WPARAM)hFont16, TRUE);
            SendMessage(hTimeLabel, WM_SETFONT, (WPARAM)hFont64, TRUE);
            SendMessage(hPomodoroInput, WM_SETFONT, (WPARAM)hFont16, TRUE);
            SendMessage(hBreakInput, WM_SETFONT, (WPARAM)hFont16, TRUE);
            break;

        case WM_TIMER:
            if (wParam == ID_TIMER) {
                remainingTime -= TIMER_UPDATE_INTERVAL;
                UpdateTimeDisplay(hwnd, remainingTime);

                if (remainingTime <= 0) {
                    KillTimer(hwnd, ID_TIMER);
                    if (isPomodoro) {
                        if (pomodoroCount < 4) {
                            MessageBox(hwnd, "Time for a short break!", "Pomodoro Timer", MB_OK | MB_ICONINFORMATION);
                            SetWindowText(hStatusLabel, "Status: Break");
                            StartTimer(hwnd, breakInterval);
                            pomodoroCount++;
                        } else {
                            MessageBox(hwnd, "Time for a long break!", "Pomodoro Timer", MB_OK | MB_ICONINFORMATION);
                            SetWindowText(hStatusLabel, "Status: Long Break");
                            StartTimer(hwnd, pomodoroInterval * 2);
                            pomodoroCount = 0;
                        }
                        isPomodoro = !isPomodoro;
                    } else {
                        MessageBox(hwnd, "Back to work!", "Pomodoro Timer", MB_OK | MB_ICONINFORMATION);
                        SetWindowText(hStatusLabel, "Status: Working");
                        StartTimer(hwnd, pomodoroInterval);
                        isPomodoro = !isPomodoro;
                    }
                }
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_TOGGLE:
                    if (isRunning) {
                        StopTimer(hwnd);
                        strcpy(textToogleButton, "Start");
                        SetWindowText(hStatusLabel, "Status: Stopped");
                    } else {
                        char pomodoroText[10], breakText[10];
                        GetWindowText(hPomodoroInput, pomodoroText, sizeof(pomodoroText));
                        GetWindowText(hBreakInput, breakText, sizeof(breakText));
                        pomodoroInterval = atoi(pomodoroText) * 60 * 1000;
                        breakInterval = atoi(breakText) * 60 * 1000;

                        if (isPomodoro) {
                            SetWindowText(hStatusLabel, "Status: Working");
                            remainingTime = pomodoroInterval;
                        } else {
                            SetWindowText(hStatusLabel, "Status: Break");
                            remainingTime = breakInterval;
                        }
                        StartTimer(hwnd, TIMER_UPDATE_INTERVAL);
                        strcpy(textToogleButton, "Stop");
                    }
                    UpdateTimeDisplay(hwnd, remainingTime);
                    isRunning = !isRunning;

                    // Принудительная перерисовка кнопки
                    InvalidateRect(hToggleButton, NULL, TRUE);
                    UpdateWindow(hToggleButton);
                    break;
                case ID_FILE_EXIT:
                    Shell_NotifyIcon(NIM_DELETE, &nid); // Удаление иконки трея
                    PostQuitMessage(0); // Завершение приложения
                    break;
                case ID_TRAY_SHOW:
                    ShowWindow(hwnd, SW_RESTORE); // Восстановление окна
                    SetForegroundWindow(hwnd); // Активировать окно
                    break;
                case ID_HELP_ABOUT:
                    ShowAboutDialog(hwnd);
                    break;
            }
            break;
        case WM_USER + 1: // Сообщение из трея
            if (lParam == WM_RBUTTONUP) {
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, ID_TRAY_SHOW, "Show");
                AppendMenu(hMenu, MF_STRING, ID_FILE_EXIT, "Exit");

                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            } else if (lParam == WM_LBUTTONDBLCLK) {
                // Двойной клик ЛКМ по иконке
                ShowWindow(hwnd, SW_RESTORE); // Восстановление окна
                SetForegroundWindow(hwnd); // Активировать окно
            }
            break;

        case WM_SIZE:
            // Изменение размера шрифта для hTimeLabel
            {
                RECT rect;
                GetClientRect(hTimeLabel, &rect);
                int newFontSize = min(rect.right - rect.left, rect.bottom - rect.top) / 1.1;
                if (newFontSize < 10) newFontSize = 10; // Минимальный размер шрифта
                
                DeleteObject(hFont64); // Удаление старого шрифта
                hFont64 = CreateFont(
                    newFontSize,               // Высота шрифта
                    0,                         // Ширина шрифта (0 означает автоматическая ширина)
                    0,                         // Угол наклона шрифта
                    0,                         // Угол наклона шрифта
                    FW_BOLD,                   // Толщина шрифта
                    FALSE,                     // Курсив
                    FALSE,                     // Подчеркивание
                    FALSE,                     // Перечеркивание
                    ANSI_CHARSET,              // Набор символов
                    OUT_DEFAULT_PRECIS,        // Точность вывода
                    CLIP_DEFAULT_PRECIS,       // Точность обрезки
                    DEFAULT_QUALITY,           // Качество вывода
                    DEFAULT_PITCH | FF_SWISS,  // Стиль шрифта и семья шрифтов
                    "Segoe UI"                 // Название шрифта
                );
                SendMessage(hTimeLabel, WM_SETFONT, (WPARAM)hFont64, TRUE);
            }
            break;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                SetBkMode(hdc, TRANSPARENT); // Прозрачный фон
                SetTextColor(hdc, textColor);

                // Пример для статического элемента
                RECT rect;
                GetClientRect(hTimeLabel, &rect);
                DrawText(hdc, "00:00", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                EndPaint(hwnd, &ps);
            }
            break;

        case WM_DRAWITEM: {
                LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
                if (dis->CtlType == ODT_BUTTON) {
                    HBRUSH hBrush = CreateSolidBrush(RGB(50, 50, 50));
                    FillRect(dis->hDC, &dis->rcItem, hBrush);
                    DeleteObject(hBrush);
                    
                    SetTextColor(dis->hDC, RGB(255, 255, 255));
                    SetBkMode(dis->hDC, TRANSPARENT);
                    
                    if (dis->hwndItem == hToggleButton) {
                        DrawText(dis->hDC, textToogleButton, -1, &dis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                }
            }
            break;

        case WM_CTLCOLORSTATIC: {
                HDC hdc = (HDC)wParam;
                SetBkColor(hdc, RGB(30, 30, 30));
                SetTextColor(hdc, RGB(255, 255, 255));
                return (INT_PTR)GetStockObject(NULL_BRUSH);
            }
            break;

        case WM_CTLCOLORLISTBOX: {
                HDC hdc = (HDC)wParam;
                SetTextColor(hdc, RGB(255, 255, 255));
                return (INT_PTR)GetStockObject(NULL_BRUSH);
	        }
	        break;

	    case WM_CTLCOLOREDIT: {
                HDC hdc = (HDC)wParam;
                SetBkColor(hdc, RGB(30, 30, 30)); //  
                SetTextColor(hdc, RGB(255, 255, 255)); //  
                return (INT_PTR)GetStockObject(NULL_BRUSH);
            }
            break;

        case WM_CLOSE:
            // Скрытие окна вместо его закрытия
            ShowWindow(hwnd, SW_HIDE);
            return 0;

        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid); // Удаление иконки трея
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void StartTimer(HWND hwnd, int interval) {
    SetTimer(hwnd, ID_TIMER, interval, NULL);
}

void StopTimer(HWND hwnd) {
    KillTimer(hwnd, ID_TIMER);
}

void UpdateTimeDisplay(HWND hwnd, int timeMs) {
    (void)hwnd; // Явное игнорирование неиспользуемого параметра
    int minutes = timeMs / 60000;
    int seconds = (timeMs % 60000) / 1000;
    char timeText[10];
    sprintf(timeText, "%02d:%02d", minutes, seconds);
    SetWindowText(hTimeLabel, timeText);
}

void ShowAboutDialog(HWND hwnd) {
    char message[256];
    sprintf(message, "Pomodoro Timer %s\nA simple pomodoro technique timer.\nDeveloper - Tailogs.", VERSION);
    MessageBox(hwnd, message, "About pomodoro timer", MB_OK | MB_ICONINFORMATION);
}