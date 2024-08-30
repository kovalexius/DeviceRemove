/////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Взято отсюда: https://learn.microsoft.com/en-us/windows/win32/devio/processing-a-request-to-remove-a-device
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <windows.h>
#include <dbt.h>
#include <strsafe.h>
#include <iostream>

INT_PTR WINAPI WinProcCallback(HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	LPCTSTR FileName = NULL;              // path to the file or device of interest
	HANDLE  hFile = INVALID_HANDLE_VALUE; // handle to the file or device

	PDEV_BROADCAST_HDR    pDBHdr;
	PDEV_BROADCAST_HANDLE pDBHandle;
	TCHAR szMsg[80];

	switch (message)
	{
	case WM_DEVICECHANGE:
		switch (wParam)
		{
		case DBT_DEVICEQUERYREMOVE:
			pDBHdr = (PDEV_BROADCAST_HDR)lParam;
			switch (pDBHdr->dbch_devicetype)
			{
			case DBT_DEVTYP_HANDLE:
				// A request has been made to remove the device;
				// close any open handles to the file or device

				pDBHandle = (PDEV_BROADCAST_HANDLE)pDBHdr;
				if (hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
				}
			}
			return TRUE;

		case DBT_DEVICEQUERYREMOVEFAILED:
			pDBHdr = (PDEV_BROADCAST_HDR)lParam;
			switch (pDBHdr->dbch_devicetype)
			{
			case DBT_DEVTYP_HANDLE:
				// Removal of the device has failed;
				// reopen a handle to the file or device

				pDBHandle = (PDEV_BROADCAST_HANDLE)pDBHdr;
				hFile = CreateFile(FileName,
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				if (hFile == INVALID_HANDLE_VALUE)
				{
					StringCchPrintf(szMsg, sizeof(szMsg) / sizeof(szMsg[0]),
						TEXT("CreateFile failed: %lx.\n"),
						GetLastError());
					MessageBox(hWnd, szMsg, TEXT("CreateFile"), MB_OK);
				}
			}
			return TRUE;

		case DBT_DEVICEREMOVEPENDING:
			pDBHdr = (PDEV_BROADCAST_HDR)lParam;
			switch (pDBHdr->dbch_devicetype)
			{
			case DBT_DEVTYP_HANDLE:

				// The device is being removed;
				// close any open handles to the file or device

				if (hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
				}

			}
			return TRUE;

		case DBT_DEVICEREMOVECOMPLETE:
			pDBHdr = (PDEV_BROADCAST_HDR)lParam;
			switch (pDBHdr->dbch_devicetype)
			{
			case DBT_DEVTYP_HANDLE:
				pDBHandle = (PDEV_BROADCAST_HANDLE)pDBHdr;
				// The device has been removed from the system;
				// unregister its notification handle

				UnregisterDeviceNotification(
					pDBHandle->dbch_hdevnotify);

				// The device has been removed;
				// close any remaining open handles to the file or device

				if (hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
				}

			}
			return TRUE;

		default:
			return TRUE;
		}
	}

	return TRUE;
}


int heart()
{
	WNDCLASS wc;

	HINSTANCE hinstance = NULL;

	// Register the main window class.
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WinProcCallback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
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