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

 

#define CAMW 1280
#define CAMH 1024
#define MAX_DEVICE_COUNT 8
#define FRAME_SIZE CAMW*CAMH

#ifdef __cplusplus
extern "C" {
#endif
 

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
  UCHAR  WordWidth;
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
	LSPFP		// плоская призма
}device_type, *pdevice_type;
 


// Структура информации об устройстве в списке _device_tree
typedef struct _device_info {
	device_type dev_type;
	SERIAL_NO SN;
	HANDLE dev;
	USB_DEVICE_DESCRIPTOR Descriptor;
} device_info, *pdevice_info;

// Cпискок подключенных устройств
typedef struct _device_tree {
	device_info device[MAX_DEVICE_COUNT];
} device_tree, *pdevice_tree;



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
	BOOL _fastcall GetType(HANDLE hDev, pdevice_type pValue);
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



#ifdef __cplusplus
}
#endif

#endif // _SCANCTRL_H


