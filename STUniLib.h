#ifndef _SCANCTRL_H
#define _SCANCTRL_H

#ifdef _USRDLL
#define DLL_EX __declspec(dllexport)
#else
#define DLL_EX __declspec(dllimport)
#endif

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>

//---------------------------------------------------------------------------
// Размер области памяти для работы с устройством
#define LEN_MEM 4096

#define CAMW 1280
#define CAMH 1024
#define MAX_DEV_COUNT 8
#define FRAME_SIZE CAMW*CAMH

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
//  Определения для работы с устройством и его периферией
#define MAX_DEVICE_COUNT 8


#define STEP_QUANTITY 0		//  команда на установку делителя шага двигателя
#define SPEED_SELECTION 1	//	команда на установку скорости вращения вала двигателся
#define STEP_N 2			//  команда на установку шагов двигателя

#define FULL_STEP 0			// кратность 1:1
#define HALF_STEP 1			// кратность 1:2
#define QUARTER_STEP 2		// кратность 1:4
#define EIGHTH_STEP 3		// кратность 1:8
#define SIXTEENTH_STEP 7	// кратность 1:16

#define CW 0				// направление вращения двигателся - по часовой стр.
#define CCW 1				// против часовой стр.

#define SCAN_MODE 1			// режим сканирования линейным сенсором для KCL001
#define CAM_MODE 0			// режим сканирования матричным сенсором для KCL001


//---------------------------------------------------------------------------
// Определения IOCTL индексов для обращения к драйверу
#define USBCAM_IOCTL_INDEX  0x0800

// Для команд HOLD, RUN
#define IOCTL_UsbCam_VENDOR_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+5,\
                                     METHOD_BUFFERED,\
                                     FILE_ANY_ACCESS)

// Для получения строки с версией
#define IOCTL_UsbCam_GET_STRING_DESCRIPTOR  CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+17,\
                                     METHOD_BUFFERED,\
                                     FILE_ANY_ACCESS)

// Для загрузки во внутреннюю память FX2
#define IOCTL_USBCAM_ANCHOR_DOWNLOAD CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+27,\
                                     METHOD_IN_DIRECT,\
                                     FILE_ANY_ACCESS)


//Для работы с "нулевой" конечной точкой FX2
#define IOCTL_USBCAM_VENDOR_OR_CLASS_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+22,\
                                     METHOD_IN_DIRECT,\
                                     FILE_ANY_ACCESS)

#define IOCTL_UsbCam_RESETPIPE CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+13,\
                                     METHOD_IN_DIRECT,\
                                     FILE_ANY_ACCESS)

// для чтения указанного потока
#define IOCTL_USBCAM_BULK_READ CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+19,\
                                     METHOD_OUT_DIRECT,\
                                     FILE_ANY_ACCESS)

// Для чтения дескриптора устройства
#define IOCTL_UsbCam_GET_DEVICE_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+1,\
                                     METHOD_BUFFERED,\
                                     FILE_ANY_ACCESS)

// Для получения лога от драйвера
#define IOCTL_USBCAM_GET_LOG CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+0x7f,\
                                     METHOD_BUFFERED,\
                                     FILE_ANY_ACCESS)

// Сброс порта
#define IOCTL_UsbCam_RESET   CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+12,\
                                     METHOD_IN_DIRECT,\
                                     FILE_ANY_ACCESS)

// Для загрузки HEX файла в FX2
#define TGT_IMG_SIZE 0x10000	// 64KB (65,536 Byte) target image
#define TGT_SEG_SIZE 16			// 16 byte segments


//---------------------------------------------------------------------------
// Структура размера кадра
typedef struct _FRAME_PROPERTY {
  int left;   // x координата исходной точки относительно полного изображения
  int top;    // y координата исходной точки относительно полного изображения
  int width;  // ширина возвращаемого изображения
  int height; // высота возвращаемого изображения
} FRAME_PROPERTY, *PFRAME_PROPERTY;

// Структура серийного номера
typedef struct _SERIAL_NO {
  WORD SerNo;
} SERIAL_NO, *PSERIAL_NO;

// Структура дескриптора устройства
typedef struct _USB_DEVICE_DESCRIPTOR {
  UCHAR  bLength;
  UCHAR  bDescriptorType;
  USHORT bcdUSB;
  UCHAR  bDeviceClass;
  UCHAR  bDeviceSubClass;
  UCHAR  bDeviceProtocol;
  UCHAR  bMaxPacketSize0;
  USHORT idVendor;
  USHORT idProduct;
  USHORT bcdDevice;
  UCHAR  iManufacturer;
  UCHAR  iProduct;
  UCHAR  iSerialNumber;
  UCHAR  bNumConfigurations;
} USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;

// Структура типов устройства
typedef enum _device_type {
	NONE,
	LS7,		// анаморфотный прокатный, дроздов
	LS2FP,		// двупалый, дроздов
	LS1FP,		// однопалый, дроздов
	LSPP,		// роликовый
	LSPPFP,		// роликовый с прокаткой
	LSPFP,		// плоская призма
	LS1FP10,
	LS2FP10
}device_type, *pdevice_type;

char *device_names [] = {"NONE", "LS7", "LS2FP", "LS1FP", "LSPP", "LSPPFP", "LSPFP"};


// Структура информации об устройстве в списке _device_tree
typedef struct _device_info {
	device_type dev_type;
	SERIAL_NO SN;
	HANDLE dev;
	USB_DEVICE_DESCRIPTOR Descriptor;
} device_info, *pdevice_info;

// Cпискок подключенных устройств
typedef struct _device_tree {
	device_info device[MAX_DEV_COUNT];
} device_tree, *pdevice_tree;

// Служебные структуры
typedef struct {
  WORD Addr;
  WORD Size;
  PBYTE pData;
} MemSeg;

typedef struct {
  BYTE data[TGT_IMG_SIZE];
} TMemImg;

typedef struct {
  TMemImg *pImg;
  int nSeg;
  MemSeg pSeg[TGT_IMG_SIZE/TGT_SEG_SIZE];
} TMemCache;

typedef struct _GET_STRING_DESCRIPTOR_IN {
  UCHAR    Index;
  USHORT   LanguageId;
} GET_STRING_DESCRIPTOR_IN, *PGET_STRING_DESCRIPTOR_IN;

typedef struct _USB_STRING_DESCRIPTOR {
  UCHAR  bLength;
  UCHAR  bDescriptorType;
  WCHAR  bString[1];
} USB_STRING_DESCRIPTOR, *PUSB_STRING_DESCRIPTOR;

typedef struct _VENDOR_OR_CLASS_REQUEST_CONTROL {
  UCHAR direction;
  UCHAR requestType;
  UCHAR recepient;
  UCHAR requestTypeReservedBits;
  UCHAR request;
  USHORT value;
  USHORT index;
} VENDOR_OR_CLASS_REQUEST_CONTROL, *PVENDOR_OR_CLASS_REQUEST_CONTROL;

typedef struct _VENDOR_REQUEST_IN {
  BYTE bRequest;
  WORD wValue;
  WORD wIndex;
  WORD wLength;
  BYTE direction;
  BYTE bData;
} VENDOR_REQUEST_IN, *PVENDOR_REQUEST_IN;

typedef struct _BULK_TRANSFER_CONTROL {
  ULONG pipeNum;
} BULK_TRANSFER_CONTROL, *PBULK_TRANSFER_CONTROL;



//---------------------------------------------------------------------------
// КЛАССЫ УСТРОЙСТВ

class DLL_EX ST_USBDevice	// общий класс устройств на базе CY68013 (USB2.0 контроллер)
{
public:
	// constructor
	ST_USBDevice(void);
	// destructor
	~ST_USBDevice(void);
	// Получить количество, список доступных устройств
	int GetDeviceCount(pdevice_tree dev_tree);
	// Открыть устройство с номером из списка
	HANDLE OpenDevice(int Number);
	// Закрыть устройство с номером из списка
	void CloseDevice(int Number);
	// Устройство подключено? TRUE - да
	BOOL IsConnectDev(int Number);
	// Прочитать строку с версией "firmware"
	BOOL GetStringDescr(HANDLE hDev, PBYTE pBuff);
	// Прочитать дескриптор устройства
	BOOL GetDevDescr(HANDLE hDev, PUSB_DEVICE_DESCRIPTOR pDD);
	// Прочитать серийный номер устройства
	BOOL GetSerialNo(HANDLE hDev, PWORD pBuff);
	BOOL SetSerialNo(HANDLE hDev, WORD serial);
	// Получить тип устройства
	BOOL _fastcall GetType(HANDLE hDev, pdevice_type pValue, PBYTE bitcount);
	// Reset device;
	BOOL __fastcall ResetDevice(HANDLE hDev, BYTE RestBit);
	// Vendor request с устанавливаемыми параметрами
	BOOL VREQ(HANDLE hDev, BYTE req, WORD value, WORD index, BYTE len, PBYTE pData);
	// Сбросить поток
	BOOL ResetPipe(HANDLE hDev);
	// Получить лог от драйвера
	BOOL GetLog(HANDLE hDev, char *Buf, ULONG Size);
	// Остановить работу CPU FX2
	BOOL HoldDev(HANDLE hDev);
	// Запустить CPU FX2
	BOOL RunDev(HANDLE hDev);
	// Загрузить блок во внутреннюю память
	BOOL AnchorDownload(HANDLE hDev, WORD Addr, WORD Len, PBYTE Data);
	// Загрузить программу "Загрузчик"
	BOOL DownLoader(HANDLE hDev);
	// Для загрузки HEX файла в FX2
	BOOL DownloadFirmware(HANDLE hDev, PCHAR pHexFileName);
	// Прочитать по I2C EEPROM
	BOOL UploadI2C(HANDLE hDev, WORD Addr, WORD Len, PWORD pData, WORD Params);
	// Записать по I2C EEPROM
	BOOL DownloadI2C(HANDLE hDev, WORD Addr, WORD Len, PWORD pData, WORD Params);
	// Прочитаь WAKEUPCS
	BOOL GetWAKEUPCS(HANDLE hDev, PBYTE pData);
	// Читать из EP2
	int ReadPipeMem(HANDLE hDev, PBYTE pMemCam, DWORD Len);
	// Обработка HEX файла и заполнение MemCache данными
	BOOL __fastcall FileToCache(TMemCache* pMemCache, CHAR *pHexFileName);
	BOOL __fastcall VendorRequest(HANDLE hDev, PVENDOR_OR_CLASS_REQUEST_CONTROL pReq,WORD Len);
	// Загрузить во внешнию, по отношению к FX2, память
	BOOL DownLoadHigh(HANDLE hDev, WORD Addr, WORD Len, PBYTE pData);
	// Прочитать внутреннюю память FX2
	BOOL UploadLow(HANDLE hDev, WORD Addr, WORD Len, PBYTE pData);
	// Прочитать внешнюю, по отношению к FX2, память
	BOOL UploadHigh(HANDLE hDev, WORD Addr, WORD Len, PBYTE pData);
	// Прочитать настройки
	// pData - указатель на массив длинною len(в байтах). Не более 1К!;
	BOOL GetSettings(HANDLE hDev, PBYTE pData, WORD len);
	// Записать настройки
	// pData - указатель на массив длинною len(в байтах). Не более 1К!;
	BOOL SetSettings(HANDLE hDev, PBYTE pData, WORD len);
	 

};


class DLL_EX KCP2001 : public ST_USBDevice		// класс для 2хсенсорного контроллера KCP-2001
{

public:
	KCP2001(void);
	~KCP2001(void);
	// Прочитать размеры окна камеры, описание PFRAME_PROPERTY см. выше
	BOOL GetFrameProperty(HANDLE hDev, PFRAME_PROPERTY pFrame);
	// Установить размеры окна камеры, описание PFRAME_PROPERTY см. выше
	BOOL SetFrameProperty(HANDLE hDev, PFRAME_PROPERTY pFrame);
	// Установить усиление камеры (0-63)
	BOOL SetGain(HANDLE hDev, BYTE Value);
	// Прочитать усиление камеры (0-63)
	BOOL GetGain(HANDLE hDev, PBYTE pValue);
	// Прочитать регистр управления подсветкой (0 - правая, 1 - левая) (0 - 255)
	BOOL GetLight(HANDLE hDev, PBYTE pValue, BYTE num);
	// Записать регистр управления подсветкой (0 - правая, 1 - левая) (0 - 255)
	BOOL SetLight(HANDLE hDev, BYTE Value, BYTE num);
	// Получить изображение с устройства
	BOOL GetImage(HANDLE hDev, PBYTE image);
	// Получить 10 битное изображение с устройства LSxFP10
	BOOL GetImage16(HANDLE hDev, PSHORT image);
	// Переключить камеру (0 - правая, 1 - левая)
	BYTE SetCamera(HANDLE hDev, BYTE num);
	// Прочитать значение смещения сигнала в подключенной камере(-255:+255)(*2мВ)
	BOOL GetOffset(HANDLE hDev, PSHORT pValue);
	// Записать значение смещения сигнала в подключенной камере (-255:+255)(*2мВ)
	BOOL SetOffset(HANDLE hDev, PSHORT offset);
protected:
	// Читать из EP2
	int ReadPipeMem(HANDLE hDev, PBYTE KCP2001_pMemCam, DWORD Len);


};

 

void __fastcall AllocateMemory(void);
void __fastcall FreeMemory(void);


#ifdef __cplusplus
}
#endif

#endif // _SCANCTRL_H


