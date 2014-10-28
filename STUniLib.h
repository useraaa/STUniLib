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
// ��������� ������� �����
typedef struct _FRAME_PROPERTY {
  int left;   // x ���������� �������� ����� ������������ ������� �����������
  int top;    // y ���������� �������� ����� ������������ ������� �����������
  int width;  // ������ ������������� �����������
  int height; // ������ ������������� �����������
} FRAME_PROPERTY, *PFRAME_PROPERTY;

// ��������� ��������� ������
typedef struct _SERIAL_NO {
  WORD SerNo;
} SERIAL_NO, *PSERIAL_NO;

// ��������� ����������� ����������
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

// ��������� ����� ����������
typedef enum _device_type {
	NONE,
	LS7,		// ������������ ���������, �������
	LS2FP,		// ��������, �������
	LS1FP,		// ���������, �������
	LSPP,		// ���������
	LSPPFP,		// ��������� � ���������
	LSPFP		// ������� ������
}device_type, *pdevice_type;
 


// ��������� ���������� �� ���������� � ������ _device_tree
typedef struct _device_info {
	device_type dev_type;
	SERIAL_NO SN;
	HANDLE dev;
	USB_DEVICE_DESCRIPTOR Descriptor;
} device_info, *pdevice_info;

// C������ ������������ ���������
typedef struct _device_tree {
	device_info device[MAX_DEVICE_COUNT];
} device_tree, *pdevice_tree;



//---------------------------------------------------------------------------
// ������ ���������

class DLL_EX ST_USBDevice	// ����� ����� ��������� �� ���� CY68013 (USB2.0 ����������)
{
public:
	// constructor
	ST_USBDevice(void);
	// destructor
	~ST_USBDevice(void);
	// �������� ����������, ������ ��������� ���������
	int GetDeviceCount(pdevice_tree dev_tree);
	// ������� ���������� � ������� �� ������
	HANDLE OpenDevice(int Number);
	// ������� ���������� � ������� �� ������
	void CloseDevice(int Number);
	// ���������� ����������? TRUE - ��
	BOOL IsConnectDev(int Number);
	// ��������� ������ � ������� "firmware"
	BOOL GetStringDescr(HANDLE hDev, PBYTE pBuff);
	// ��������� ���������� ����������
	BOOL GetDevDescr(HANDLE hDev, PUSB_DEVICE_DESCRIPTOR pDD);
	// ��������� �������� ����� ����������
	BOOL GetSerialNo(HANDLE hDev, PWORD pBuff);
	BOOL SetSerialNo(HANDLE hDev, WORD serial);
	// �������� ��� ����������
	BOOL _fastcall GetType(HANDLE hDev, pdevice_type pValue);
	// Reset device;
	BOOL __fastcall ResetDevice(HANDLE hDev, BYTE RestBit);
	// Vendor request � ���������������� �����������
	BOOL VREQ(HANDLE hDev, BYTE req, WORD value, WORD index, BYTE len, PBYTE pData);
	// �������� �����
	BOOL ResetPipe(HANDLE hDev);
	// �������� ��� �� ��������
	BOOL GetLog(HANDLE hDev, char *Buf, ULONG Size);
	// ���������� ������ CPU FX2
	BOOL HoldDev(HANDLE hDev);
	// ��������� CPU FX2
	BOOL RunDev(HANDLE hDev);
	// ��������� ���� �� ���������� ������
	BOOL AnchorDownload(HANDLE hDev, WORD Addr, WORD Len, PBYTE Data);
	// ��������� ��������� "���������"
	BOOL DownLoader(HANDLE hDev);
	// ��� �������� HEX ����� � FX2
	BOOL DownloadFirmware(HANDLE hDev, PCHAR pHexFileName);
	// ��������� �� I2C EEPROM
	BOOL UploadI2C(HANDLE hDev, WORD Addr, WORD Len, PWORD pData, WORD Params);
	// �������� �� I2C EEPROM
	BOOL DownloadI2C(HANDLE hDev, WORD Addr, WORD Len, PWORD pData, WORD Params);
	// �������� WAKEUPCS
	BOOL GetWAKEUPCS(HANDLE hDev, PBYTE pData);
	// ������ �� EP2
	int ReadPipeMem(HANDLE hDev, PBYTE pMemCam, DWORD Len);
	// ��������� �� �������, �� ��������� � FX2, ������
	BOOL DownLoadHigh(HANDLE hDev, WORD Addr, WORD Len, PBYTE pData);
	// ��������� ���������� ������ FX2
	BOOL UploadLow(HANDLE hDev, WORD Addr, WORD Len, PBYTE pData);
	// ��������� �������, �� ��������� � FX2, ������
	BOOL UploadHigh(HANDLE hDev, WORD Addr, WORD Len, PBYTE pData);
	// ��������� ���������
	// pData - ��������� �� ������ ������� len(� ������). �� ����� 1�!;
	BOOL GetSettings(HANDLE hDev, PBYTE pData, WORD len);
	// �������� ���������
	// pData - ��������� �� ������ ������� len(� ������). �� ����� 1�!;
	BOOL SetSettings(HANDLE hDev, PBYTE pData, WORD len);
	 

};


class DLL_EX KCP2001 : public ST_USBDevice		// ����� ��� 2����������� ����������� KCP-2001
{

public:
	KCP2001(void);
	~KCP2001(void);
	// ��������� ������� ���� ������, �������� PFRAME_PROPERTY ��. ����
	BOOL GetFrameProperty(HANDLE hDev, PFRAME_PROPERTY pFrame);
	// ���������� ������� ���� ������, �������� PFRAME_PROPERTY ��. ����
	BOOL SetFrameProperty(HANDLE hDev, PFRAME_PROPERTY pFrame);
	// ���������� �������� ������ (0-63)
	BOOL SetGain(HANDLE hDev, BYTE Value);
	// ��������� �������� ������ (0-63)
	BOOL GetGain(HANDLE hDev, PBYTE pValue);
	// ��������� ������� ���������� ���������� (0 - ������, 1 - �����) (0 - 255)
	BOOL GetLight(HANDLE hDev, PBYTE pValue, BYTE num);
	// �������� ������� ���������� ���������� (0 - ������, 1 - �����) (0 - 255)
	BOOL SetLight(HANDLE hDev, BYTE Value, BYTE num);
	// �������� ����������� � ����������
	BOOL GetImage(HANDLE hDev, PBYTE image);
	// �������� 10 ������ ����������� � ���������� LSxFP10
	BOOL GetImage16(HANDLE hDev, PSHORT image);
	// ����������� ������ (0 - ������, 1 - �����)
	BYTE SetCamera(HANDLE hDev, BYTE num);
	// ��������� �������� �������� ������� � ������������ ������(-255:+255)(*2��)
	BOOL GetOffset(HANDLE hDev, PSHORT pValue);
	// �������� �������� �������� ������� � ������������ ������ (-255:+255)(*2��)
	BOOL SetOffset(HANDLE hDev, PSHORT offset);
protected:
	// ������ �� EP2
	int ReadPipeMem(HANDLE hDev, PBYTE KCP2001_pMemCam, DWORD Len);


};



#ifdef __cplusplus
}
#endif

#endif // _SCANCTRL_H


