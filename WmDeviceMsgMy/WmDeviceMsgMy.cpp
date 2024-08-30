#include <iostream>

#include <Windows.h>
#include <Dbt.h>


LRESULT WindowProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	switch (_uMsg)
	{
	case WM_DEVICECHANGE:
		std::cout << "uMsg: WM_DEVICECHANGE" << " wParam: " << _wParam << std::endl;
		if (_wParam == DBT_DEVICEARRIVAL)
		{
			DEV_BROADCAST_HDR* devStruct = (DEV_BROADCAST_HDR*)_lParam;
			std::cout << "devStruct->dbch_devicetype: " << devStruct->dbch_devicetype << std::endl;
			if (devStruct->dbch_devicetype == DBT_DEVTYP_VOLUME)
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)devStruct;
				std::cout << 
					"lpdbv->dbcv_devicetype: "	<<	lpdbv->dbcv_devicetype	<<	" " <<
					"lpdbv->dbcv_flags: "		<<	lpdbv->dbcv_flags		<<	" " <<
					"lpdbv->dbcv_reserved: "	<<	lpdbv->dbcv_reserved	<<	" " <<
					"lpdbv->dbcv_size: "		<<	lpdbv->dbcv_size		<<	" " <<
					"lpdbv->dbcv_unitmask: "	<<	lpdbv->dbcv_unitmask	<<	" " <<
					std::endl;
			}

			/// Послать сообщение
			//SendMessageA(_hWnd, DBT_DEVICEREMOVECOMPLETE, _wParam, _lParam);
			PostMessageA(_hWnd, DBT_DEVICEREMOVECOMPLETE, _wParam, _lParam);

		}
		return 0;
	}

	return 0;
}

int heart()
{
	WNDCLASS wc;

	//HINSTANCE hinstance = _hinstance;
	HINSTANCE hinstance = NULL;

	// Register the main window class.
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = GetStockObject(WHITE_BRUSH);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = "MainMenu";
	wc.lpszClassName = "MainWindowClass";

	if (!RegisterClassA(&wc))
	{
		DWORD res = GetLastError();
		std::cout << "RegisterClassA() code: " << res << std::endl;
		return FALSE;
	}

	// Создание окна
	int nCmdShow = SW_SHOWDEFAULT;
	HWND hwnd = CreateWindowA("MainWindowClass", "Окно пользователя",
		WS_OVERLAPPEDWINDOW, 500, 300, 500, 380, NULL, NULL, hinstance, NULL);
	BOOL res = ShowWindow(hwnd, nCmdShow);		// отображение
	res = UpdateWindow(hwnd);				// перерисовка

	// Цикл обработки сообщений
	MSG msg;							// структура сообщения
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int WinMain(
	HINSTANCE _hinstance,	// handle to current instance 
	HINSTANCE _hinstPrev,	// handle to previous instance 
	LPSTR _lpCmdLine,		// address of command-line string 
	int _nCmdShow			// show-window type 
)
{
	return heart();
}

int main(int argc, char** argv)
{
	return heart();
}