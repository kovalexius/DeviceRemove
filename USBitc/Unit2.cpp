///////////////////////////////////////////////////////////////////////////////
///
/// Взято отсюда: https://radiokot.ru/articles/75/
/// 
///////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit2.h"
#include <Windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <devguid.h>
#include <regstr.h>
#include <tchar.h>
#include <string.h>
#include <devpropdef.h>
#include <devpkey.h>
#include <initguid.h>
#include <hidsdi.h>
#include <iostream>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <stdlib.h>
#include <fstream>
#include <Classes.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TForm2 *Form2;
DWORD WINAPI ReadThread(LPVOID);
DWORD WINAPI WriteThread(LPVOID);
HANDLE reader;
HANDLE writer;
OVERLAPPED overlapped;
OVERLAPPED overlappedwr;
HANDLE USBport;
HANDLE filed = 0;
int wo=0;
unsigned char reportwrite[17];
unsigned char reportread[17];
void senddata();

//*****************************************************************************
//*****************************************************************************
//--Поток чтения --------------------------------------------------------------
DWORD WINAPI ReadThread(LPVOID)
{
  overlapped.hEvent = CreateEvent(NULL, false, true, NULL);
  DWORD Bytes =0;
  while(1)
  {
	bool pr = false;
	pr = ReadFile (filed,&reportread[0],17,&Bytes,(LPOVERLAPPED) &overlapped);
	DWORD prw = WaitForSingleObject(overlapped.hEvent,INFINITE);
	if(prw == WAIT_OBJECT_0)
	{
			AnsiString ans = "READ: ";
			char m[17]="";
			for (int g = 0; g < 17; g++)
			{
			 sprintf(m, "%02X ", reportread[g]);
			 ans=ans+m+" ";
			}
			Form2->ListBox1->Items->Add(ans);
			prw=NULL;

	}
  }
}
//*****************************************************************************
// Поток записи ---------------------------------------------------------------
DWORD WINAPI WriteThread(LPVOID)
{
 DWORD bcount=0;;
 char Report[17];
 Report[0]=2;
 Report[1]=0x02;
 overlappedwr.hEvent = CreateEvent(NULL, false, true, NULL);
 bool pr=false;
 while(1)
 {
   pr = WriteFile(filed,&Report,17,&bcount,&overlappedwr);
   SuspendThread(writer);
 }
}
//****************************************************************************
// ПЕРЕМЕННЫЕ ДЛЯ БАЗОВЫХ ФУНКЦИЙ ---------------------------------------------
bool connecthid(AnsiString path);
bool getinfo();
bool enumd();
void disconhid();

HDEVINFO hDev; //информация о списке устройств
SP_DEVINFO_DATA dInf; //массив данных об устройстве
SP_DEVICE_INTERFACE_DATA dIntDat; //массив данных об интерфейсе
PSP_DEVICE_INTERFACE_DETAIL_DATA    dIntDet = NULL;
ULONG                               pLength = 0;
ULONG                               rLength = 0;
DWORD i;
TCHAR pathdev[512];
AnsiString dsval;
AnsiString stabp;
AnsiString productdata[3] = {"1155","22350","I2Cdial"};
AnsiString getdata[3];
int finded=0;
int g=0;
int pressed=0;
int sizin=0;

//*****************************************************************************
// ФУНКЦИИ HIDD ---------------------------------------------------------------
typedef void (WINAPI* pHidD_GetHidGuid)( OUT LPGUID );
typedef BOOLEAN (WINAPI* pHidD_GetManufacturerString)(IN HANDLE, OUT PVOID, IN ULONG);
typedef BOOLEAN (WINAPI* pHidD_GetProductString)(IN HANDLE, OUT PVOID, IN ULONG);
typedef BOOLEAN (WINAPI* pHidD_GetFeature)(IN HANDLE, OUT PVOID, IN ULONG);
typedef BOOLEAN (WINAPI* pHidD_SetFeature)(IN HANDLE, IN PVOID, IN ULONG);
typedef BOOLEAN (WINAPI* pHidD_SetOutputReport)(IN HANDLE, IN PVOID, IN ULONG);
typedef BOOLEAN (WINAPI* pHidD_GetAttributes)(HANDLE, PHIDD_ATTRIBUTES);
typedef BOOLEAN (WINAPI* pHidD_GetInputReport)(IN HANDLE, IN PVOID, IN ULONG);

//*****************************************************************************
// Загрузка внешней библиотеки -------------------------------------------------
HINSTANCE hDLL = LoadLibrary(L"HID.DLL");
GUID hguid;
//*****************************************************************************
// Указатели на функции HIDD ---------------------------------------------------
pHidD_GetProductString   GetProductString = NULL;
pHidD_GetHidGuid         GetHidGuid = NULL;
pHidD_GetAttributes      GetAttributes = NULL;
pHidD_SetFeature         SetFeature = NULL;
pHidD_GetFeature         GetFeature = NULL;
pHidD_SetOutputReport    SetOutReport = NULL;
pHidD_GetInputReport     GetInputReport = NULL;

//*****************************************************************************
//*****************************************************************************
// Функция загрузки библиотеки ------------------------------------------------
void loadlib()
{
  if(hDLL != 0)
  {
  GetHidGuid = (pHidD_GetHidGuid)GetProcAddress(hDLL, "HidD_GetHidGuid");
  GetProductString = (pHidD_GetProductString) GetProcAddress(hDLL, "HidD_GetProductString");
  GetFeature = (pHidD_GetFeature) GetProcAddress(hDLL, "HidD_GetFeature");
  SetFeature = (pHidD_SetFeature) GetProcAddress(hDLL, "HidD_SetFeature");
  GetAttributes = (pHidD_GetAttributes)GetProcAddress(hDLL, "HidD_GetAttributes");
  SetOutReport = (pHidD_SetOutputReport)GetProcAddress(hDLL, "HidD_SetOutputReport");
  if(GetHidGuid)
	  {
		GetHidGuid(&hguid);
		Form2->StatusBar1->Panels->Items[0]->Text = "LIB: OK";
	  }
  }
}

//*****************************************************************************
//*****************************************************************************
// Функция энкмерации устройств -----------------------------------------------
bool enumd()
{
	loadlib();
	i=0;
	finded=0;
	hDev = SetupDiGetClassDevs(&hguid,NULL,0,DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);
	if (hDev == INVALID_HANDLE_VALUE)
	   {
		   ShowMessage("Error1");
	   }
		dInf.cbSize = sizeof(SP_DEVINFO_DATA);
		dIntDat.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
		while (SetupDiEnumDeviceInterfaces(hDev,0,&hguid,i,&dIntDat))
		{
				if(dIntDet)
				{
				free (dIntDet);
				dIntDet = NULL;
				}
				SetupDiGetDeviceInterfaceDetail(hDev, &dIntDat, 0, 0, &rLength, 0);
				pLength = rLength;
				dIntDet = (SP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(pLength);

				if(dIntDet)
				dIntDet->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);
				if (!SetupDiGetDeviceInterfaceDetail(hDev, &dIntDat, dIntDet, rLength, &rLength, 0))
				   {
				   free(dIntDet);
				   }
				dsval=dIntDet->DevicePath;
				if(finded==0)
				{
				  if (connecthid(dsval)) return true;
				}
		i=i+1;
		}
		if(finded==0)
		{
		  Form2->StatusBar1->Panels->Items[1]->Text = "DEVICE: NO";
		}
	   SetupDiDestroyDeviceInfoList(hDev);
}


//***************************************************************************
// Функция соединения с найденным устройством
bool connecthid(AnsiString path)
{
	  wchar_t wstr[100];
	  dsval.WideChar(wstr,100);
	  filed = CreateFile       ( wstr,
								GENERIC_READ|GENERIC_WRITE,
								 0,
								NULL, // no SECURITY_ATTRIBUTES structure
								OPEN_EXISTING, // No special create flags
								FILE_FLAG_OVERLAPPED, // No special attributes
								0);

			 if (filed == INVALID_HANDLE_VALUE) { }
			 else
			 {
			   if (getinfo())
				{
				reader = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
				writer = CreateThread(NULL, 0, WriteThread, NULL, CREATE_SUSPENDED, NULL);
				Form2->StatusBar1->Panels->Items[1]->Text = "DEVICE: OK";
				Form2->StatusBar1->Panels->Items[2]->Text = "VID: 1155";
				Form2->StatusBar1->Panels->Items[3]->Text = "PID: 22350";
				Form2->Panel1->Color = clLime;
				Form2->Panel1->Caption = "ON";
				Form2->Button1->Enabled = false;
				Form2->Button2->Enabled = true;
				finded = 1;
				return true;
				}
				else
				{
				disconhid();
				return false;
			   }
			 }
    return false;
}

//****************************************************************************
//****************************************************************************
// Получение атрибутов VID PID и ProductName для сравнения с базовыми данными
bool getinfo()
{
  char bufp[10]; //указываем буфер и размер, куда прочитаем атрибуты
  HIDD_ATTRIBUTES Attributes;
  Attributes.Size = sizeof(Attributes);
  bool result = GetAttributes(filed,&Attributes); //читаем атрибуты
	//Если атрибуты прочли успешно
	if (result == true)
	{
	 // Form2->ListBox1->Items->Add("Attributes loaded successfull");

	  if ( (AnsiString) Attributes.VendorID != "1155")
	   {
		 return false;
	   }
	   if ( (AnsiString) Attributes.ProductID != "22350")
	   {
		 return false;
	   }
		//ЧИтаем поле NAME, если успешно, то..
		if (GetProductString(filed,&bufp[0],10))
		{
		   AnsiString prods;
		   sprintf(bufp,"I2Cdial");
		   prods = AnsiString(bufp);
		   if ( (AnsiString) prods != "I2Cdial")
		   {
			return false;
		   }
		}
	 return true;
	}
   return false;
}
//***************************************************************************
//****************************************************************************
//---------------------------------------------------------------------------
void disconhid()
{
	if(reader)
  {
   TerminateThread(reader,0);
   CloseHandle(overlapped.hEvent);	//нужно закрыть объект-событие
   CloseHandle(reader);
  }
	if(writer)
  {
   //ResumeThread(writer);
   TerminateThread(writer,0);
   if (wo==1) {
	  CloseHandle(overlappedwr.hEvent);	//нужно закрыть объект-событие
   }
   CloseHandle(writer);
  }
  CloseHandle(filed);
  Form2->Panel1->Color = clRed;
  Form2->Panel1->Caption = "Off";
  Form2->Button2->Enabled = false;
  Form2->Button1->Enabled = true;
  Form2->StatusBar1->Panels->Items[0]->Text = "LIB: NO";
  Form2->StatusBar1->Panels->Items[1]->Text = "DEVICE: NO";
  Form2->StatusBar1->Panels->Items[2]->Text = "VID: ----";
  Form2->StatusBar1->Panels->Items[3]->Text = "PID: ----";
  //Form2->ListBox1->Clear();
  //Form2->ListBox1->Items->Add("Disconnected mode enable");
  //finded=0;
}

//***************************************************************************
//ОТПРАВИТЬ ДАННЫЕ ----------------------------------------------------------
void senddata()
{
 ResumeThread(writer);
}

//***************************************************************************
//---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner)
	: TForm(Owner)
{
  Form2->Button2->Enabled = false;
  Form2->Panel1->Color = clRed;
  Form2->Panel1->Caption = "Off";
  Form2->StatusBar1->Panels->Items[0]->Width = 48;
  Form2->StatusBar1->Panels->Items[1]->Width = 68;
  Form2->StatusBar1->Panels->Items[2]->Width = 68;
  Form2->StatusBar1->Panels->Items[3]->Width = 68;
  Form2->StatusBar1->Panels->Items[0]->Text = "LIB: NO";
  Form2->StatusBar1->Panels->Items[1]->Text = "DEVICE: NO";
  Form2->StatusBar1->Panels->Items[2]->Text = "VID: ----";
  Form2->StatusBar1->Panels->Items[3]->Text = "PID: ----";
}
//****************************************************************************
//---------------------------------------------------------------------------
void __fastcall TForm2::Button1Click(TObject *Sender)
{
 enumd();
}
//---------------------------------------------------------------------------
void __fastcall TForm2::Button2Click(TObject *Sender)
{
disconhid();
}
//---------------------------------------------------------------------------

void __fastcall TForm2::Button3Click(TObject *Sender)
{
 //senddata();
}
//---------------------------------------------------------------------------

