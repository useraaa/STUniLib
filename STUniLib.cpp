//---------------------------------------------------------------------------
// STUniLib.cpp
//---------------------------------------------------------------------------
#include "stdafx.h"
#include "STUniLib.h"
#include "Vend_Ax.h"

#include <cstdlib>
#include <iostream>
#include <wchar.h>


void __fastcall AllocateMemory(void);
void __fastcall FreeMemory(void);

void *pMem = NULL,
	 *pCam = NULL,
	 *pScan = NULL;

device_tree dev_tr;
 
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
  switch(ul_reason_for_call) {

    case DLL_PROCESS_ATTACH:
	  AllocateMemory();
    break;

    case DLL_THREAD_ATTACH:
    break;

    case DLL_THREAD_DETACH:
    break;

    case DLL_PROCESS_DETACH:
      FreeMemory();
    break;
  }
  return TRUE;
}

//---------------------------------------------------------------------------
// Выделение блока физической памяти для работы с устройством
// Память зафиксирована т.к. часть вызовов драйвера по методу "direct"
void __fastcall AllocateMemory(void)
{
	if(pMem != NULL)
		return;

	pMem = VirtualAlloc(NULL, LEN_MEM, MEM_RESERVE |MEM_COMMIT |
					  MEM_TOP_DOWN, PAGE_READWRITE);

	if(pMem == NULL)
		return;

	if(!VirtualLock(pMem, LEN_MEM)) {
		VirtualFree(pMem,0,MEM_RELEASE);
		pMem = NULL;
	}

	if(pScan != NULL)
		return;

	pScan = VirtualAlloc(NULL, LEN_MEM, MEM_RESERVE |MEM_COMMIT |
					  MEM_TOP_DOWN, PAGE_READWRITE);

	if(pScan == NULL)
		return;

	if(!VirtualLock(pScan, LEN_MEM)) {
		VirtualFree(pScan,0,MEM_RELEASE);
		pScan = NULL;
	}

	if(pCam != NULL)
		return;

	pCam = VirtualAlloc(NULL, LEN_MEM, MEM_RESERVE |MEM_COMMIT |
                      MEM_TOP_DOWN, PAGE_READWRITE);

	if(pCam == NULL)
		return;

	if(!VirtualLock(pCam, LEN_MEM)) {
	    VirtualFree(pCam,0,MEM_RELEASE);
		pCam = NULL;
	}

}

//---------------------------------------------------------------------------
// Освободить память
void __fastcall FreeMemory(void)
{
  if(pMem) {
    VirtualUnlock(pMem,LEN_MEM);
    VirtualFree(pMem,0,MEM_RELEASE);
    pMem = NULL;
  }
  if(pCam) {
		VirtualUnlock(pCam,LEN_MEM);
		VirtualFree(pCam,0,MEM_RELEASE);
		pCam = NULL;
	}
  if(pScan) {
		VirtualUnlock(pScan,LEN_MEM);
		VirtualFree(pScan,0,MEM_RELEASE);
		pScan = NULL;
	}
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------КЛАСС KCP2-001----------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

/* PROTOTYPES */
int ReadPipeMem(HANDLE hDev, PBYTE KCP2001_pMemCam, DWORD Len);


/*  Global Variables */
int current_width = 1280;
int current_height = 1024;
int current_left = 0;
int current_top = 0;

int get_device_num(HANDLE dev){
	for (int i = 0; i < MAX_DEV_COUNT; i++) {
		 if (dev_tr.device[i].dev == dev)
		 	return i;
	}
	return -1;
}

KCP2001::KCP2001(void){

}

KCP2001::~KCP2001(void){

}
//******************************************************************
// Прочитать размеры окна камеры по I2C
BOOL KCP2001::GetFrameProperty(HANDLE hDev, PFRAME_PROPERTY pFrame)
{
	PWORD pData = (PWORD)pCam;
	BYTE temp = 0;
	if ( (UploadI2C(hDev, 0x1, 1, &pData[0], 0x5D02))&(UploadI2C(hDev, 0x2, 1, &pData[1], 0x5D02))&
		 (UploadI2C(hDev, 0x3, 1, &pData[2], 0x5D02))&(UploadI2C(hDev, 0x4, 1, &pData[3], 0x5D02)))
	{
		for (int i = 0; i < 4; i++ ){
			temp = (BYTE)(pData[i]>>8);
			pData[i] = (pData[i]<<8)|temp;
		}
		pFrame->top		= pData[0] - 0x0C;
		pFrame->left	= pData[1] - 0x14;
		pFrame->height	= pData[2] + 1;
		pFrame->width	= pData[3] + 1;
		current_width	= pFrame->width;
		current_height	= pFrame->height;
		return true;
	}
  return false;
}

//---------------------------------------------------------------------------
// Записать размеры окна камеры по I2C
BOOL KCP2001::SetFrameProperty(HANDLE hDev, PFRAME_PROPERTY pFrame)
{
	PWORD pData = (PWORD)pCam;
	memset(pData, 0, 8);
	pData[0] = pFrame->top + 0x0C;
	pData[1] = pFrame->left + 0x14;
	pData[2] = pFrame->height - 1;
	pData[3] = pFrame->width - 1;
	if (!DownloadI2C(hDev, 0x1, 1, &pData[0], 0x5D02)) return false;
	if (!DownloadI2C(hDev, 0x2, 1, &pData[1], 0x5D02)) return false;
	if (!DownloadI2C(hDev, 0x3, 1, &pData[2], 0x5D02)) return false;
	if (!DownloadI2C(hDev, 0x4, 1, &pData[3], 0x5D02)) return false;
	current_width	= pFrame->width;
	current_height	= pFrame->height;
	return true;

}
//---------------------------------------------------------------------------
// Установить смещение камеры
BOOL KCP2001::SetOffset(HANDLE hDev, short *offset)
{
	WORD temp = 0x499;
	DownloadI2C(hDev, 0x62, 1, &temp, 0x5D02);
	PWORD pValue = (PWORD)pCam;
	memset (pValue, 0, 4*sizeof(WORD));

	if (offset[0] < 0) pValue[0] |= (0x1<<8)|(256+(BYTE)offset[0]);
	else pValue[0] = (BYTE)offset[0];

	if (offset[1] < 0) pValue[1] |= (0x1<<8)|(256+(BYTE)offset[1]);
	else pValue[1] = (BYTE)offset[1];

	if (offset[2] < 0) pValue[2] |= (0x1<<8)|(256+(BYTE)offset[2]);
	else pValue[2] = (BYTE)offset[2];

	if (offset[3] < 0) pValue[3] |= (0x1<<8)|(256+(BYTE)offset[3]);
	else pValue[3] = (BYTE)offset[3];

	if (DownloadI2C(hDev, 0x60, 1, &pValue[0], 0x5D02)&
		DownloadI2C(hDev, 0x61, 1, &pValue[1], 0x5D02)&
		DownloadI2C(hDev, 0x63, 1, &pValue[2], 0x5D02)&
		DownloadI2C(hDev, 0x64, 1, &pValue[3], 0x5D02)) return true;
	return false;
}
//---------------------------------------------------------------------------
// Прочитать смещение камеры
BOOL KCP2001::GetOffset(HANDLE hDev, short *pValue)
{
	WORD temp = 0x499;
	DownloadI2C(hDev, 0x62, 1, &temp, 0x5D02);
	PWORD pOffset = (PWORD)pCam;
	memset (pOffset, 0, 4*sizeof(WORD));

	if (UploadI2C(hDev, 0x60, 0x01, &pOffset[0], 0x5D02)&
		UploadI2C(hDev, 0x61, 0x01, &pOffset[1], 0x5D02)&
		UploadI2C(hDev, 0x63, 0x01, &pOffset[2], 0x5D02)&
		UploadI2C(hDev, 0x64, 0x01, &pOffset[3], 0x5D02)) {
			for (int i = 0; i < 4; i++) {
				if (pOffset[i]&0x1) pValue[i] = ((pOffset[i]>>8)-256);
				else pValue[i] = pOffset[i]>>8;
			}
			return true;
	}
	return false;

}
//---------------------------------------------------------------------------
// Установить усиление камеры
BOOL KCP2001::SetGain(HANDLE hDev, BYTE Value)
{
	PWORD pData = (PWORD)pCam;
	pData[0] = (0x7F)&Value;
  if (DownloadI2C(hDev, 0x35, 1, pData, 0x5D02)) return true;
  return false;
}

//---------------------------------------------------------------------------
// Прочитать усиление камеры
BOOL KCP2001::GetGain(HANDLE hDev, PBYTE pValue)
{
	PWORD pData = (PWORD)pCam;
	if (UploadI2C(hDev, 0x35, 1, pData, 0x5D02))
	{
		*pValue = (*pData)>>8;
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------
// Получить изображение с устройства
BOOL KCP2001::GetImage(HANDLE hDev, PBYTE image)
{
 
	BYTE dev_num = 0;
	int len = 0;
	BYTE twelve_bit_mode = 0;

	if ((dev_num = get_device_num(hDev)) < 0) {
		std::cout << "Device handle is not from device tree!";
		return FALSE;
	}

	len = current_width*current_height;

	if ( dev_tr.device[dev_num].Descriptor.iSerialNumber == 10 )
	{
		twelve_bit_mode = 1;
		len *= 2;
	}

	if (ReadPipeMem(hDev, image, (DWORD)len) == len)
	{
		// If we have 10bit scanner we should prepare the array
		if (twelve_bit_mode)
		{
			for (int i = 0; i < len; i++) {
				image[i] = ((PUSHORT)image)[i] >> 2;
			}
		}

		return TRUE;
	}
	return FALSE;
}
/*

*/
BOOL KCP2001::GetImage16(HANDLE hDev, PSHORT image)
{
  int Size; 
  BYTE dev_num = 0;
  int len = 0;

  if ((dev_num = get_device_num(hDev)) < 0) {
	  std::cout << "Device handle is not from device tree!";
	  return FALSE;
  }

  if (dev_tr.device[dev_num].Descriptor.iSerialNumber != 10)
  {
	  std::cout << "Device not comaptible with 10bit mode!";
	  return false;
  }

  len = current_width*current_height*2;

  Size = ReadPipeMem(hDev, (PBYTE) image, (DWORD)len);
  if(Size == len) return TRUE;
  return FALSE;
} 

int KCP2001::ReadPipeMem(HANDLE hDev, PBYTE KCP2001_pMemCam, DWORD Len)
{
  BOOL bRet = false;
  DWORD dwRet = 0;
  BULK_TRANSFER_CONTROL TransfControl;

  TransfControl.pipeNum = 0;

  bRet = DeviceIoControl (hDev,
						  IOCTL_USBCAM_BULK_READ,
                          &TransfControl,
						  sizeof(BULK_TRANSFER_CONTROL),
                          KCP2001_pMemCam,
                          Len,
                          &dwRet,
                          NULL);

  if (bRet) return dwRet;
  return -1;
}
//---------------------------------------------------------------------------
// Переключить камеры(1,0)
BYTE KCP2001::SetCamera(HANDLE hDev, BYTE num)
{
  VENDOR_OR_CLASS_REQUEST_CONTROL Req;

  Req.request = 0xB2;
  Req.value = (BYTE)num;
  Req.requestTypeReservedBits = 0;
  Req.index = 0;
  Req.direction = 0;   // 0=host to device, 1=device to host
  Req.requestType = 2; // 1=class, 2=vendor
  Req.recepient = 0;   // 0=device,1=interface,2=endpoint,3=other

  if(VendorRequest(hDev, &Req, 0)) return num;
  return 0xff;
}
  
//---------------------------------------------------------------------------
// Прочитать регистр сопротивления по I2C
BOOL KCP2001::GetLight(HANDLE hDev, PBYTE pData, BYTE num)
{
	PWORD pDat = (PWORD)pCam;

	switch (num) {
	case 0:	// Bottom backligth for right camera
		if (!UploadI2C(hDev, 0, 1, pDat, 0x2E00)) return false;
		break;
	case 1: // Bottom backligth for left camera
		if (!UploadI2C(hDev, 0, 1, pDat, 0x2C00)) return false;
		break;
	case 2: // Top backligth for left camera
		if (!UploadI2C(hDev, 0, 1, pDat, 0x2D00)) return false;
		break;
	case 3: // Top backligth for right camera
		if (!UploadI2C(hDev, 0, 1, pDat, 0x2F00)) return false;
		break;
	}
	*pData = 255 - *pDat;
	return false;
}
//---------------------------------------------------------------------------
// Записать регистр сопротивления по I2C
BOOL KCP2001::SetLight(HANDLE hDev, BYTE Data, BYTE num)
{
	PWORD pData = (PWORD)pCam;
	pData[0] = 255 - Data;
	switch (num) {
	case 0:	// Bottom backligth for right camera
		if (DownloadI2C(hDev, 0, 1, pData, 0x2E00)) return true;
		break;
	case 1: // Bottom backligth for left camera
		if (DownloadI2C(hDev, 0, 1, pData, 0x2C00)) return true;
		break;
	case 2: // Top backligth for left camera
		if (DownloadI2C(hDev, 0, 1, pData, 0x2D00)) return true;
		break;
	case 3: // Top backligth for right camera
		if (DownloadI2C(hDev, 0, 1, pData, 0x2F00)) return true;
		break;
	}

	return false;
}






//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------КЛАСС ST_USBDevice------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

ST_USBDevice::ST_USBDevice(void)
{
	if (pMem == NULL) {
		AllocateMemory();
	}
}

ST_USBDevice::~ST_USBDevice(void)
{
	FreeMemory();
}
//---------------------------------------------------------------------------
// Открыть устройство
HANDLE ST_USBDevice::OpenDevice(int Number)
{
	 
	wchar_t *DevName = new wchar_t[32];
	wsprintf(DevName, (LPWSTR)"\\\\.\\UsbCam-%d", Number);

	HANDLE hnd = CreateFile(DevName, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING, 0, NULL);

	dev_tr.device[Number].dev = hnd;
	delete DevName;
	return hnd;
}

void ST_USBDevice::CloseDevice(int Number)
{
	wchar_t *DevName = new wchar_t[32];
	wsprintf(DevName, (LPWSTR)"\\\\.\\UsbCam-%d", Number);
  DeleteFile(DevName);
  delete DevName;
  return;
}
//---------------------------------------------------------------------------
// Получить список доступных устройств
int ST_USBDevice::GetDeviceCount(pdevice_tree dev_tree)
{
  int dev_num = 0;
  BYTE bitmode=8;
  HANDLE hDev = INVALID_HANDLE_VALUE;

  pdevice_tree p = &dev_tr;
  for(int i = 0; i < MAX_DEVICE_COUNT; i++)
  {
	hDev = OpenDevice(i);

	if( hDev != INVALID_HANDLE_VALUE )
	{
	  dev_num++;
	  GetDevDescr(hDev, &p->device[i].Descriptor);

	  if (p->device[i].Descriptor.idProduct != 0x8613)
	  {
		 GetSerialNo(hDev, &p->device[i].SN.SerNo);
		 GetType(hDev, &p->device[i].dev_type, &bitmode);
	  } else {
			p->device[i].dev_type = NONE;
			p->device[i].SN.SerNo = 0x8613;
	  }
	  CloseHandle(hDev);
	  hDev = NULL;
	} else {
		p->device[i].dev_type = NONE;
		p->device[i].SN.SerNo = 0;
	}
  }

  memcpy(dev_tree, &dev_tr, sizeof(struct _device_tree));

  return dev_num;
}
//---------------------------------------------------------------------------
// Тип устройства?
BOOL _fastcall ST_USBDevice::GetType(HANDLE hDev, pdevice_type pValue, BYTE *bitcount)
{
  BYTE Data;
  VENDOR_OR_CLASS_REQUEST_CONTROL Req;

  Req.request = 0xD4;
  Req.value = 0;
  Req.requestTypeReservedBits = 0;
  Req.index = 0;
  Req.direction = 1;   // 0=host to device, 1=device to host
  Req.requestType = 2; // 1=class, 2=vendor
  Req.recepient = 0;   // 0=device,1=interface,2=endpoint,3=other

  if(VendorRequest(hDev, &Req, 2)) {
	Data = *((PBYTE)pMem);
	*pValue = (device_type)Data;
	*bitcount = ((PBYTE)pMem)[1];
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------------------------
// Устройство подключено?
BOOL ST_USBDevice::IsConnectDev(int Number)
{
  HANDLE hDev = OpenDevice(Number);
  CloseHandle(hDev);
  return  hDev != INVALID_HANDLE_VALUE;
}

//---------------------------------------------------------------------------
// Прочитать дескриптор устройства
BOOL ST_USBDevice::GetDevDescr(HANDLE hDev, PUSB_DEVICE_DESCRIPTOR pDD)
{
  BOOL bRet;
  DWORD dwRet;

  bRet = DeviceIoControl (hDev,
                          IOCTL_UsbCam_GET_DEVICE_DESCRIPTOR,
                          NULL,
                          0,
                          pDD,
                          sizeof(USB_DEVICE_DESCRIPTOR),
                          &dwRet,
                          NULL);

  if(bRet && dwRet == sizeof(USB_DEVICE_DESCRIPTOR)) {
    return TRUE;
  }
  return FALSE;
}

 

//---------------------------------------------------------------------------
// Установить состояние бита CPUCS.0 в FX2
BOOL __fastcall ST_USBDevice::ResetDevice(HANDLE hDev, BYTE RestBit)
{
  DWORD dwRet;
  VENDOR_REQUEST_IN VReq;

  VReq.bRequest = 0xA0;
  VReq.wValue = 0xE600; // using CPUCS.0 in FX2
  VReq.wIndex = 0x00;
  VReq.wLength = 0x01;
  VReq.bData = RestBit;
  VReq.direction = 0x00;

  return DeviceIoControl (hDev,
                          IOCTL_UsbCam_VENDOR_REQUEST,
                          &VReq,
                          sizeof(VENDOR_REQUEST_IN),
                          NULL,
                          0,
                          &dwRet,
                          NULL);
}

//---------------------------------------------------------------------------
// Остановить работу CPU FX2
BOOL ST_USBDevice::HoldDev(HANDLE hDev)
{
  return ResetDevice(hDev, 1);
}

//---------------------------------------------------------------------------
// Запустить CPU FX2
BOOL ST_USBDevice::RunDev(HANDLE hDev)
{
  return ResetDevice(hDev, 0);
}

//---------------------------------------------------------------------------
// Загрузить блок во внутреннюю память FX2
BOOL ST_USBDevice::AnchorDownload(HANDLE hDev, WORD Addr, WORD Len, PBYTE Data)
{
  DWORD dwRet;
  WORD Offset = Addr;

  memcpy(pMem, Data, Len);

  return DeviceIoControl(hDev,
                         IOCTL_USBCAM_ANCHOR_DOWNLOAD,
                         &Offset,
                         sizeof(WORD),
                         pMem,
                         Len,
                         &dwRet,
                         NULL);
}

//---------------------------------------------------------------------------
// Загрузить программу "Загрузчик"
BOOL ST_USBDevice::DownLoader(HANDLE hDev)
{
  PINTEL_HEX_RECORD ptr = Vend_Ax_Fx2;

  if(!pMem) return FALSE;
  if(!HoldDev(hDev)) return FALSE;

  while (ptr->Type == 0) {
    AnchorDownload(hDev, ptr->Address, ptr->Length, ptr->Data);
    ptr++;
  }
  return RunDev(hDev);
}


//---------------------------------------------------------------------------
// Загрузить во внешнию, по отношению к FX2, память
BOOL ST_USBDevice::DownLoadHigh(HANDLE hDev, WORD Addr, WORD Len, PBYTE pData)
{
  VENDOR_OR_CLASS_REQUEST_CONTROL Req;

  Req.request = 0xA3;
  Req.value = Addr;
  Req.requestTypeReservedBits = 0;
  Req.index = 0;
  Req.direction = 0;   // 0=host to device, 1=device to host
  Req.requestType = 2; // 1=class, 2=vendor
  Req.recepient = 0;   // 0=device,1=interface,2=endpoint,3=other

  memcpy(pMem, pData, Len);

  return VendorRequest(hDev, &Req, Len);
}
//---------------------------------------------------------------------------
// Прочитать внутреннюю память FX2
BOOL ST_USBDevice::UploadLow(HANDLE hDev, WORD Addr, WORD Len, PBYTE pData)
{
  VENDOR_OR_CLASS_REQUEST_CONTROL Req;

  Req.request = 0xA0;
  Req.value = Addr;
  Req.requestTypeReservedBits = 0;
  Req.index = 0;
  Req.direction = 1;   // 0=host to device, 1=device to host
  Req.requestType = 2; // 1=class, 2=vendor
  Req.recepient = 0;   // 0=device,1=interface,2=endpoint,3=other

  if(!VendorRequest(hDev, &Req, Len)) return FALSE;
  memcpy(pData, pMem, Len);
  return TRUE;
}

//---------------------------------------------------------------------------
// Прочитать внешнюю, по отношению к FX2, память
BOOL ST_USBDevice::UploadHigh(HANDLE hDev, WORD Addr, WORD Len, PBYTE pData)
{
  VENDOR_OR_CLASS_REQUEST_CONTROL Req;

  Req.request = 0xA3;
  Req.value = Addr;
  Req.requestTypeReservedBits = 0;
  Req.index = 0;
  Req.direction = 1;   // 0=host to device, 1=device to host
  Req.requestType = 2; // 1=class, 2=vendor
  Req.recepient = 0;   // 0=device,1=interface,2=endpoint,3=other

  if(!DownLoader(hDev)) return FALSE;
  if(!VendorRequest(hDev, &Req, Len)) return FALSE;
  memcpy(pData, pMem, Len);
  return TRUE;
}

//---------------------------------------------------------------------------
// Прочитать строку с версией
BOOL ST_USBDevice::GetStringDescr(HANDLE hDev, PBYTE pBuff)
{
  int i;
  BOOL bRet;
  DWORD dwRet;
  GET_STRING_DESCRIPTOR_IN StrDesc;
  PUSB_STRING_DESCRIPTOR pD;

  StrDesc.Index = 1;
  StrDesc.LanguageId = 27;

  bRet = DeviceIoControl (hDev,
                          IOCTL_UsbCam_GET_STRING_DESCRIPTOR,
                          &StrDesc,
                          sizeof(GET_STRING_DESCRIPTOR_IN),
                          pMem,
                          256,
                          &dwRet,
                          NULL);

  if(bRet && dwRet > 0) {
    pD = (PUSB_STRING_DESCRIPTOR)pMem;
    for(i = 0; i < (pD->bLength/2)-1; i++) {
      pBuff[i] = (BYTE)pD->bString[i];
    }
	pBuff[i] = '\0';
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------------------------
// Прочитаь WAKEUPCS
BOOL ST_USBDevice::GetWAKEUPCS(HANDLE hDev, PBYTE pData)
{
  VENDOR_OR_CLASS_REQUEST_CONTROL Req;

  Req.request = 0xD2;
  Req.value = 0;
  Req.requestTypeReservedBits = 0;
  Req.index = 0;
  Req.direction = 1;   // 0=host to device, 1=device to host
  Req.requestType = 2; // 1=class, 2=vendor
  Req.recepient = 0;   // 0=device,1=interface,2=endpoint,3=other

  if(VendorRequest(hDev, &Req, 1)) {
    *pData = *((PBYTE)pMem);
    return TRUE;
  }
  return FALSE;
}
//---------------------------------------------------------------------------
// Сбросить поток
BOOL ST_USBDevice::ResetPipe(HANDLE hDev)
{
  DWORD dwRet;
  DWORD PipeNum = 0;

  return DeviceIoControl (hDev,
                          IOCTL_UsbCam_RESETPIPE,
                          &PipeNum,
                          sizeof(LONG),
                          NULL,
                          0,
                          &dwRet,
                          NULL);
}

//---------------------------------------------------------------------------
// Прочитать поток в указанную область памяти
int ST_USBDevice::ReadPipeMem(HANDLE hDev, PBYTE pMemCam, DWORD Len)
{
  BOOL bRet = false;
  DWORD dwRet = 0;
  BULK_TRANSFER_CONTROL TransfControl;

  TransfControl.pipeNum = 0;

  bRet = DeviceIoControl (hDev,
                          IOCTL_USBCAM_BULK_READ,
                          &TransfControl,
                          sizeof(BULK_TRANSFER_CONTROL),
                          pMemCam,
                          Len,
                          &dwRet,
                          NULL);

  if (bRet) return dwRet;
  return -1;
}
//---------------------------------------------------------------------------
// Прочитать/записать по I2C серийный номер
BOOL ST_USBDevice::GetSerialNo(HANDLE hDev, PWORD pBuff)
{
	memset(pBuff, 0, 4);
	if (UploadI2C(hDev, 0x1DFE, sizeof(WORD), (PWORD)pBuff, 0x5101)) return true;
	else return FALSE;

}
BOOL ST_USBDevice::SetSerialNo(HANDLE hDev, WORD serial)
{
	if (serial == 0) return false;
	if (DownloadI2C(hDev, 0x1DFE, sizeof(WORD), &serial, 0x5101)) return true;
	else return FALSE;

}

BOOL ST_USBDevice::GetSettings(HANDLE hDev, PBYTE pData, WORD len)
{
	if (len > 512) return false;
	if (UploadI2C(hDev, 0x1E00, len, (PWORD)pData, 0x5101)) return true;
	return false;
}

BOOL ST_USBDevice::SetSettings(HANDLE hDev, PBYTE pData, WORD len)
{
	if (len > 512) return false;
	if (DownloadI2C(hDev, 0x1E00, len, (PWORD)pData, 0x5101)) return true;
	return false;
}



//---------------------------------------------------------------------------
// Записать по I2C EEPROM
BOOL  ST_USBDevice::DownloadI2C(HANDLE hDev, WORD Addr, WORD Len, PWORD pData, WORD Params)
{
  VENDOR_OR_CLASS_REQUEST_CONTROL Req;

  Req.request = 0xA2;
  Req.value = Addr;
  Req.requestTypeReservedBits = 0;
  Req.index = Params;
  Req.direction = 0;   // 0=host to device, 1=device to host
  Req.requestType = 2; // 1=class, 2=vendor
  Req.recepient = 0;   // 0=device,1=interface,2=endpoint,3=other

	if (Params&0x02) {
		memcpy(pMem, pData, Len<<1);
		return VendorRequest(hDev, &Req, Len<<1);
	} else {
		memcpy(pMem, pData, Len);
		return VendorRequest(hDev, &Req, Len);
	}
}

//---------------------------------------------------------------------------
// Прочитать по I2C EEPROM
BOOL  ST_USBDevice::UploadI2C(HANDLE hDev, WORD Addr, WORD Len, PWORD pData, WORD Params)
{
  VENDOR_OR_CLASS_REQUEST_CONTROL Req;

  Req.request = 0xA2;
  Req.value = Addr;
  Req.requestTypeReservedBits = 0;
  Req.index = Params;
  Req.direction = 1;   // 0=host to device, 1=device to host
  Req.requestType = 2; // 1=class, 2=vendor
  Req.recepient = 0;   // 0=device,1=interface,2=endpoint,3=other

	if (Params&0x02) {
		if(VendorRequest(hDev, &Req, Len<<1)) {
			memcpy(pData, pMem, Len<<1);
			return TRUE;
		}
	} else {
		if(VendorRequest(hDev, &Req, Len)) {
			memcpy(pData, pMem, Len);
			return TRUE;
		}
	}

  return FALSE;
}

//---------------------------------------------------------------------------
// Для загрузки HEX файла в FX2
BOOL ST_USBDevice::DownloadFirmware(HANDLE hDev, PCHAR pHexFileName)
{
  BOOL bRet = FALSE;
  TMemCache *pMemCache;
  int i;

  do {
    pMemCache = (TMemCache *)LocalAlloc(LMEM_FIXED,sizeof(TMemCache));
    if (!pMemCache) {
//      MessageBox(NULL,"Insufficient resources","DownloadFirmware",MB_OK);
      break;
    }

    pMemCache->pImg = (TMemImg *)LocalAlloc(LMEM_FIXED,TGT_IMG_SIZE);
    if (!pMemCache->pImg) {
//      MessageBox(NULL,"Insufficient resources","DownloadFirmware",MB_OK);
      break;
    }

    bRet = FileToCache(pMemCache, pHexFileName);
    if (!bRet) break;

    // check for high mem first, load loader first if necessary
    for (i = 0; i < pMemCache->nSeg; i++) {
      if (pMemCache->pSeg[i].Addr >= 0x2000) {
        bRet = DownLoader(hDev);
        break;
      }
    }
    if(!bRet) {
//      MessageBox(NULL,"Error load Downloader","DownloadFirmware",MB_OK);
      break;
    }

    // Загрузка во внешнюю память
    for (i = 0; i < pMemCache->nSeg; i++) {
      if (pMemCache->pSeg[i].Addr >= 0x2000) {
        bRet = DownLoadHigh(hDev, pMemCache->pSeg[i].Addr,
                 pMemCache->pSeg[i].Size, pMemCache->pSeg[i].pData);
        if(!bRet){
//          MessageBox(NULL,"Error DownLoadHigh","DownloadFirmware",MB_OK);
          break;
        }
      }
    }
    if(!bRet) break;

    bRet = HoldDev(hDev);
    if(!bRet){
//      MessageBox(NULL,"Error HoldDev","DownloadFirmware",MB_OK);
      break;
    }

    // Загрузка во внутреннюю память
    for (i = 0; i < pMemCache->nSeg; i++) {
      if (pMemCache->pSeg[i].Addr < 0x2000) {
        bRet = AnchorDownload(hDev, pMemCache->pSeg[i].Addr,
                 pMemCache->pSeg[i].Size, pMemCache->pSeg[i].pData);
        if(!bRet){
//          MessageBox(NULL,"Error AnchorDownload","DownloadFirmware",MB_OK);
          break;
        }
      }
    }
    if(!bRet) break;

    bRet = RunDev(hDev);
    if(!bRet){
//      MessageBox(NULL,"Error RunDev","DownloadFirmware",MB_OK);
    }
  } while(0);

  if (pMemCache) {
    if (pMemCache->pImg) LocalFree(pMemCache->pImg);
    LocalFree(pMemCache);
  }
  return bRet;
}
 
//---------------------------------------------------------------------------
// 4K Control EP0 transfer limit imposed by OS
#define MAX_EP0_XFER_SIZE (1024*4)
//---------------------------------------------------------------------------
// Обработка HEX файла и заполнение MemCache данными
BOOL __fastcall ST_USBDevice::FileToCache(TMemCache* pMemCache, CHAR *pHexFileName)
{
  BOOL bRet = FALSE;
  HANDLE hFile;
  int i, recType, cnt;
  int CNTFIELD, ADDRFIELD, RECFIELD, DATAFIELD;
  PCHAR pstr, str;
  PCHAR pBuf = NULL;
  DWORD byte, addr, lenFile, totalRead;
  PBYTE ptr;
  size_t newsize = strlen(pHexFileName) + 1;
  wchar_t * wcstring = new wchar_t[newsize];
  size_t convertedChars = 0;
   
  mbstowcs_s(&convertedChars, wcstring, newsize, pHexFileName, _TRUNCATE);
  hFile = CreateFile(wcstring, GENERIC_READ, FILE_SHARE_READ,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  delete wcstring;

  if(hFile == INVALID_HANDLE_VALUE) {
//    MessageBox(NULL,"Error CreateFile","FileToCache",MB_OK);
    return FALSE;
  }

  do {
    lenFile = GetFileSize( hFile, NULL);
    if(lenFile < 10 || lenFile > 0x20000) {
//      MessageBox(NULL,"Bad Length File","FileToCache",MB_OK);
      break;
    }

    pBuf = (PCHAR)LocalAlloc(LMEM_FIXED, lenFile + 1);
    if (!pBuf) {
//      MessageBox(NULL,"Insufficient resources","FileToCache",MB_OK);
      break;
    }

    bRet = ReadFile(hFile, pBuf, lenFile, &totalRead, NULL);
    if(!bRet) {
//      MessageBox(NULL,"Error Read File","FileToCache",MB_OK);
      break;
    }
    pstr = pBuf;
    pstr[lenFile] = '\0';

    // offsets of fields within record -- may change later due to "spaces"
    // setting
    CNTFIELD = 1;
    ADDRFIELD = 3;
    RECFIELD = 7;
    DATAFIELD = 9;

    addr = 0;
    pMemCache->nSeg = 0;
  bRet = FALSE;

    while (*pstr) {
      str = pstr;

      // Поиск следующей строки
      while(*pstr++ != '\r');
      *(pstr - 1) = '\0';
      pstr++; // указывает на следующую строку или на '\0'

      if (str[0] != ':') {
//        MessageBox(NULL,"Bad Format HexFile","FileToCache",MB_OK);
        break;
      }

      if (str[1]==' ') { // get the record type
        CNTFIELD = 1 + 1;
        ADDRFIELD = 3 + 2;
        RECFIELD = 7 + 3;
        DATAFIELD = 9 + 4;
      }

      sscanf_s(str+RECFIELD, "%2x", &recType);

      ptr = (PBYTE)pMemCache->pImg;
      switch(recType) {
        case 0: // data record
          sscanf_s(str+CNTFIELD, "%2x", &cnt);
          sscanf_s(str+ADDRFIELD, "%4x", &addr);
          ptr += addr; // get pointer to location in image

          if (pMemCache->nSeg &&
              (pMemCache->pSeg[pMemCache->nSeg-1].Addr ==
                 addr - pMemCache->pSeg[pMemCache->nSeg-1].Size) &&
              (pMemCache->pSeg[pMemCache->nSeg-1].Size + cnt <= MAX_EP0_XFER_SIZE)) {
            // если сегмент продолжает предыдущий, и их суммарная длина
            // не будет превышать предела MAX_EP0_XFER_SIZE то их объединяем
            pMemCache->pSeg[pMemCache->nSeg-1].Size += cnt;
          }
          else {
            // start a new segment
            pMemCache->pSeg[pMemCache->nSeg].Addr = (WORD)addr;
            pMemCache->pSeg[pMemCache->nSeg].Size = (WORD)cnt;
            pMemCache->pSeg[pMemCache->nSeg].pData = ptr;
            pMemCache->nSeg++;
          }

          for (i = 0; i < cnt; i++) {
            sscanf_s(str+DATAFIELD+i*2, "%2x", &byte);
            *(ptr + i) = (CHAR)byte;
          }
        break;

        case 1: //end record
          bRet = TRUE;
        break;
      }
    }
  } while(0);

  if(hFile != INVALID_HANDLE_VALUE) CloseHandle(hFile);
  if(pBuf) LocalFree(pBuf);

  return bRet;
}

//---------------------------------------------------------------------------
// Используется для работы с "нулевой" конечной точкой
BOOL __fastcall ST_USBDevice::VendorRequest(HANDLE hDev, PVENDOR_OR_CLASS_REQUEST_CONTROL pReq,WORD Len)
{
  DWORD dwRet;
  PVOID pBuf = (Len) ? pMem : NULL;

  return DeviceIoControl (hDev,
                          IOCTL_USBCAM_VENDOR_OR_CLASS_REQUEST,
                          pReq,
                          sizeof(VENDOR_OR_CLASS_REQUEST_CONTROL),
                          pBuf,
                          Len,
                          &dwRet,
                          NULL);
}
//---------------------------------------------------------------------------
// Запрос по EP0
BOOL ST_USBDevice::VREQ(HANDLE hDev, BYTE req, WORD value, WORD index, BYTE len, PBYTE pData)
{
 
  VENDOR_OR_CLASS_REQUEST_CONTROL Req;

  Req.request = req;
  Req.value = value;
  Req.requestTypeReservedBits = 0;
  Req.index = index;
  Req.direction = 1;   // 0=host to device, 1=device to host
  Req.requestType = 2; // 1=class, 2=vendor
  Req.recepient = 0;   // 0=device,1=interface,2=endpoint,3=other


  if(VendorRequest(hDev, &Req, len)) {
		memcpy(pData,pMem, len);
	return TRUE;
  }
  return FALSE;
}



 


