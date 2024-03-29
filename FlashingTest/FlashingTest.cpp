#include "framework.h"
#include "FlashingTest.h"
#include <sstream>
#include <vector>

const int NUMBER_OF_WINDOWS = 3;

static std::vector<HWND> windows;

void FlashTaskbarIcon(const int count, HWND hwdn) {
	FLASHWINFO fwi{ sizeof(fwi) };
	fwi.hwnd = hwdn;
	fwi.uCount = count;
	fwi.dwFlags = FLASHW_TRAY;
	const bool flash_window_result = ::FlashWindowEx(&fwi);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
		windows.erase(std::remove(windows.begin(), windows.end(), hwnd), windows.end());
		if (windows.empty()) {
			PostQuitMessage(0);
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == 1001) {
			FlashTaskbarIcon(2, windows[1]); // let the 2nd window flash
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	// reg the win class
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"MyWindowClass";

	if (!RegisterClass(&wc)) {
		MessageBox(nullptr, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// create windows
	for (auto i = 0; i < NUMBER_OF_WINDOWS; i++) {
		std::wstringstream name;
		name << "Window " << i;
		windows.push_back(CreateWindowEx(0, L"MyWindowClass", name.str().c_str(), WS_OVERLAPPEDWINDOW, 100 + i * 400, 100, 400, 300,
			nullptr, nullptr, GetModuleHandle(nullptr), nullptr));
	}

	// Set window title to guide user for repro steps
	SetWindowText(windows[0], L"(1) Info");
	SetWindowText(windows[1], L"(2) Minimize");
	SetWindowText(windows[2], L"(3) Min+Close");

	// win flash button
	HWND btn = CreateWindow(
		L"BUTTON",        // class;
		L"Flash Window",  // txt
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		10,               // x
		10,               // y
		100,              // w
		30,               // h
		windows[0],       // 1st window is parent
		(HMENU)1001,      // Button identifier (used in WM_COMMAND)
		GetModuleHandle(nullptr),
		nullptr);

	std::wstring text_box_text = L"1. Minimize other windows (2) (3)";
	text_box_text += L"\r\n2. Close window (3) from Windows taskbar";
	text_box_text += L"\r\n3. Click on 'Flash window' button in (1)";
	text_box_text += L"\r\n";
	text_box_text += L"\r\nTaskbar icon should only flash.";
	text_box_text += L"\r\nThe bug is window (2) activates which it should not.";

	// repro steps description
	HWND text_box = CreateWindow(
		L"EDIT",          // class;
		text_box_text.c_str(),  // txt
		WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
		10,               // x
		50,               // y
		370,              // w
		230,              // h
		windows[0],       // 1st window is parent
		(HMENU)1002,      // Button identifier (used in WM_COMMAND)
		GetModuleHandle(nullptr),
		nullptr);

	for (HWND hwnd : windows) {
		ShowWindow(hwnd, SW_SHOW);
	}

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}