#include <windows.h>
#include <commctrl.h>
#include <stdbool.h>
#include <stdio.h>
#include <shellapi.h> // Для системного трея
#include "resource.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void StartTimer(HWND, int);
void StopTimer(HWND);
void UpdateTimeDisplay(HWND, int);
void ShowAboutDialog(HWND);

HFONT hFont;
HBRUSH hBrush;
COLORREF bgColor = RGB(30, 30, 30);
COLORREF textColor = RGB(200, 200, 200);
HWND hToggleButton, hStatusLabel, hTimeLabel, hPomodoroInput, hBreakInput;
bool isRunning = false;
bool isPomodoro = true;
int remainingTime = 0;
int pomodoroInterval = POMODORO_DEFAULT * 60 * 1000; // по умолчанию 25 минут
int breakInterval = BREAK_DEFAULT * 60 * 1000;       // по умолчанию 5 минут

NOTIFYICONDATA nid; // Для системного трея

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 350,
        NULL, NULL, hInstance, NULL
    );

    HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MYMENU));
    SetMenu(hwnd, hMenu);

    hFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
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

    DeleteObject(hFont);
    DeleteObject(hBrush);

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int pomodoroCount = 0;

    switch (msg) {
        case WM_CREATE:
            hToggleButton = CreateWindow("BUTTON", "Start", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 150, 220, 100, 30, hwnd, (HMENU)IDM_TOGGLE, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hStatusLabel = CreateWindow("STATIC", "Status: Waiting", WS_VISIBLE | WS_CHILD, 50, 180, 120, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hTimeLabel = CreateWindow("STATIC", "00:00", WS_VISIBLE | WS_CHILD, 50, 50, 60, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            CreateWindow("STATIC", "Pomodoro (min):", WS_VISIBLE | WS_CHILD, 50, 90, 120, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hPomodoroInput = CreateWindow("EDIT", "25", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 170, 90, 50, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            CreateWindow("STATIC", "Break (min):", WS_VISIBLE | WS_CHILD, 50, 130, 120, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            hBreakInput = CreateWindow("EDIT", "5", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 170, 130, 50, 30, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            SendMessage(hToggleButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hStatusLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hTimeLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hPomodoroInput, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hBreakInput, WM_SETFONT, (WPARAM)hFont, TRUE);
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
                        SetWindowText(hToggleButton, "Start");
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
                        SetWindowText(hToggleButton, "Stop");
                    }
                    UpdateTimeDisplay(hwnd, remainingTime);
                    isRunning = !isRunning;
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
    int minutes = timeMs / 60000;
    int seconds = (timeMs % 60000) / 1000;
    char timeText[10];
    sprintf(timeText, "%02d:%02d", minutes, seconds);
    SetWindowText(hTimeLabel, timeText);
}

void ShowAboutDialog(HWND hwnd) {
    MessageBox(hwnd, "Pomodoro Timer v1.0\nA simple Pomodoro technique timer.\nDeveloper - Taillogs.", "About Pomodoro Timer", MB_OK | MB_ICONINFORMATION);
}