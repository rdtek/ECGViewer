#include "wndmain.h"

bool windowCreated = false;
WindowSize windowSize;

VOID CALLBACK PaintTimerProc(HWND hwnd, UINT uMessage, UINT_PTR uEventId, DWORD dwTime) {
	BOOL result = KillTimer(hwnd, uEventId);
	wantDrawSignal = 1;
	SaveWindowSize(hwnd, &windowSize);
	DoRedraw(hwnd);
}

//Window Procedure - handles window messages
LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC         hDeviceContext;
	RECT        windowRect;

	switch (msg)
	{
	    case WM_CREATE:
        {
            AddMenus(hWindow);
            int windowWidth = GetWindowWidth(hWindow);
            
            hBtnPageLeft = CreateButtonW(hWindow, IDC_PAGELEFT_BUTTON, 
                L"<", windowWidth - 100, 220, 100, 24);
            hBtnPageRight = CreateButtonW(hWindow, IDC_PAGELEFT_BUTTON, 
                L">", 50, 220, 100, 24);

            windowCreated = true;
            break;
        }

        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 300;
            lpMMI->ptMinTrackSize.y = 300;
            break;
        }

	    case WM_SIZE:
        {
            if (GetClientRect(hWindow, &windowRect)) {
                if (windowCreated == true) {
                    int windowWidth = GetWindowWidth(hWindow);
                    SetWindowPos(hBtnPageLeft, NULL, windowWidth - 100, 10, 30, 30, NULL);
                    SetWindowPos(hBtnPageRight, NULL, windowWidth - 60, 10, 30, 30, NULL);
                }
                SaveWindowSize(hWindow, &windowSize);
                wantDrawSignal = 0;
                SetTimer(hWindow, 0, 150, PaintTimerProc);
            }
            break;
        }

	    case WM_EXITSIZEMOVE:
        {
            wantDrawSignal = 1;
            if (WindowSizeChanged(hWindow, &windowSize) >= 1) {
                DoRedraw(hWindow);
            }
            break;
        }

	    case WM_PAINT:
        {
            //OutputDebugString(TEXT("\nPAINT"));
            hDeviceContext = BeginPaint(hWindow, &ps);
            DrawGrid(hDeviceContext, hWindow);
            if (signalLoaded == 1 && wantDrawSignal >= 1) {
                DrawSignal(hDeviceContext, hWindow);
                //Save window size to avoid unnecessary redraw
                SaveWindowSize(hWindow, &windowSize);
            }
            EndPaint(hWindow, &ps);
            break;
        }

	    case WM_COMMAND:
        {
            switch (LOWORD(wParam)) {
            case IDM_FILE_OPEN:
                DoOpenFile(signalBuffer, maxSamples);
                wantDrawSignal = 1;
                signalLoaded = 1;
                DoRedraw(hWindow);
                break;
            case IDM_FILE_QUIT:
                SendMessage(hWindow, WM_CLOSE, 0, 0);
                break;
            case IDM_TOOLS_REFRESH:
                InvalidateRect(hWindow, 0, 1);
                DoRedraw(hWindow);
                break;
            }
            break;
        }

	    case WM_CLOSE: { DestroyWindow(hWindow); break; }
	    case WM_DESTROY: { PostQuitMessage(0); break; }
	    default: break;
	}
	return DefWindowProc(hWindow, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	LPCWSTR szAppName = L"ECG Viewer";

	//Registering the Window Class
	wc.cbSize           = sizeof(WNDCLASSEX);
	wc.style            = 0;
	wc.lpfnWndProc      = WndProc;
	wc.cbClsExtra       = 0;
	wc.cbWndExtra       = 0;
	wc.hInstance        = hInstance;
	wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName     = NULL;
	wc.lpszClassName    = MYCLASSNAME;;
	wc.hIconSm          = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, L"Window Registration Failed!", L"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	//Creating the Window
	hwnd = CreateWindow(MYCLASSNAME, MYWINDOWNAME, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, L"Window Creation Failed!", L"Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	//Setup the Message Loop
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

void DoRedraw(HWND hwnd) {
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void AddMenus(HWND hwnd) {

	HMENU hMenubar;
	HMENU hMenuFile;
	HMENU hMenuTools;

	hMenubar = CreateMenu();

	hMenuFile = CreateMenu();
	AppendMenuW(hMenuFile, MF_STRING, IDM_FILE_OPEN, L"&Open");
	AppendMenuW(hMenuFile, MF_SEPARATOR, 0, NULL);
	AppendMenuW(hMenuFile, MF_STRING, IDM_FILE_QUIT, L"&Quit");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenuFile, L"&File");

	hMenuTools = CreateMenu();
	AppendMenuW(hMenuTools, MF_STRING, IDM_TOOLS_REFRESH, L"&Refresh");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenuTools, L"&Tools");

	SetMenu(hwnd, hMenubar);
}

void DoOpenFile(long* signalArray, int maxNum) {
	WCHAR szFile[100];

	ZeroMemory(&openFileName, sizeof(openFileName));
	openFileName.lStructSize = sizeof(openFileName);
	openFileName.hwndOwner = NULL;
	openFileName.lpstrFile = szFile;
	openFileName.lpstrFile[0] = '\0';
	openFileName.nMaxFile = sizeof(szFile);
	openFileName.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFileTitle = NULL;
	openFileName.nMaxFileTitle = 0;
	openFileName.lpstrInitialDir = NULL;
	openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	GetOpenFileName(&openFileName);

	FILE * ptr_file = _wfopen(openFileName.lpstrFile, L"r");
	//int numLines = CountFileLines(openFileName.lpstrFile);

	if (ptr_file != NULL) {

		int idxSignalArray = 0;
		char strLine[128];

		while (fgets(strLine, sizeof strLine, ptr_file) != NULL) {
			signalArray[idxSignalArray] = strtol(strLine, NULL, 10);
			idxSignalArray++;
			//log_int("\nidxSignalArray: ", idxSignalArray);
			if (idxSignalArray >= maxNum) break;
		}
		if (idxSignalArray >= 1) signalLoaded = 1;

		fclose(ptr_file);

	}

	return;
}

int CountFileLines(const wchar_t* fileName) {
	int numLines = 0;
	int chr = 0;

	log_wstr(L"ECG Filename: ", fileName);

	FILE * ptr_file = _wfopen(fileName, L"r");
	if (ptr_file != NULL) {
		while (!feof(ptr_file)) {
			chr = fgetc(ptr_file);
			if (chr == '\n') {
				numLines++;
			}
		}
		fclose(ptr_file);
	}

	log_int("\nNumber of lines: ", numLines);
	return numLines;
}