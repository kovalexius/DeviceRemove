//////////////////////////////////////////////////////////////////////////////////////////////////
///
/// https://learn.microsoft.com/ru-ru/windows/win32/devio/detecting-media-insertion-or-removal
/// 
/// 
//////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <dbt.h>
#include <strsafe.h>
#include <iostream>
#pragma comment(lib, "user32.lib" )

void Main_OnDeviceChange(HWND hwnd, WPARAM wParam, LPARAM lParam);
char FirstDriveFromMask(ULONG unitmask);  //prototype

/*------------------------------------------------------------------
   Main_OnDeviceChange( hwnd, wParam, lParam )

   Description
	  Handles WM_DEVICECHANGE messages sent to the application's
	  top-level window.
--------------------------------------------------------------------*/

void Main_OnDeviceChange(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
	TCHAR szMsg[80];

	switch (wParam)
	{
	case DBT_DEVICEARRIVAL:
		// Check whether a CD or DVD was inserted into a drive.
		if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
		{
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;

			if (lpdbv->dbcv_flags & DBTF_MEDIA)
			{
				StringCchPrintf(szMsg, sizeof(szMsg) / sizeof(szMsg[0]),
					TEXT("Drive %c: Media has arrived.\n"),
					FirstDriveFromMask(lpdbv->dbcv_unitmask));

				MessageBox(hwnd, szMsg, TEXT("WM_DEVICECHANGE"), MB_OK);
			}
		}
		break;

	case DBT_DEVICEREMOVECOMPLETE:
		// Check whether a CD or DVD was removed from a drive.
		if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
		{
			PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;

			if (lpdbv->dbcv_flags & DBTF_MEDIA)
			{
				StringCchPrintf(szMsg, sizeof(szMsg) / sizeof(szMsg[0]),
					TEXT("Drive %c: Media was removed.\n"),
					FirstDriveFromMask(lpdbv->dbcv_unitmask));

				MessageBox(hwnd, szMsg, TEXT("WM_DEVICECHANGE"), MB_OK);
			}
		}
		break;

	default:
		/*
		  Process other WM_DEVICECHANGE notifications for other
		  devices or reasons.
		*/
		;
	}
}

/*------------------------------------------------------------------
   FirstDriveFromMask( unitmask )

   Description
	 Finds the first valid drive letter from a mask of drive letters.
	 The mask must be in the format bit 0 = A, bit 1 = B, bit 2 = C,
	 and so on. A valid drive letter is defined when the
	 corresponding bit is set to 1.

   Returns the first drive letter that was found.
--------------------------------------------------------------------*/

char FirstDriveFromMask(ULONG unitmask)
{
	char i;

	for (i = 0; i < 26; ++i)
	{
		if (unitmask & 0x1)
			break;
		unitmask = unitmask >> 1;
	}

	return(i + 'A');
}



int heart()
{
	WNDCLASS wc;
	HINSTANCE hinstance = NULL;

	// Register the main window class.
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)Main_OnDeviceChange;
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