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
// ������ ������� ������ ��� ������ � �����������
#define LEN_MEM 4096

#define CAMW 1280
#define CAMH 1024
#define MAX_DEV_COUNT 8
#define FRAME_SIZE CAMW*CAMH

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
//  ����������� ��� ������ � ����������� � ��� ����������
#define MAX_DEVICE_COUNT 8


#define STEP_QUANTITY 0		//  ������� �� ��������� �������� ���� ���������
#define SPEED_SELECTION 1	//	������� �� ��������� �������� �������� ���� ����������
#define STEP_N 2			//  ������� �� ��������� ����� ���������

#define FULL_STEP 0			// ��������� 1:1
#define HALF_STEP 1			// ��������� 1:2
#define QUARTER_STEP 2		// ��������� 1:4
#define EIGHTH_STEP 3		// ��������� 1:8
#define SIXTEENTH_STEP 7	// ��������� 1:16

#define CW 0				// ����������� �������� ���������� - �� ������� ���.
#define CCW 1				// ������ ������� ���.

#define SCAN_MODE 1			// ����� ������������ �������� �������� ��� KCL001
#define CAM_MODE 0			// ����� ������������ ��������� �������� ��� KCL001


//---------------------------------------------------------------------------
// ����������� IOCTL �������� ��� ��������� � ��������
#define USBCAM_IOCTL_INDEX  0x0800

// ��� ������ HOLD, RUN
#define IOCTL_UsbCam_VENDOR_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+5,\
                                     METHOD_BUFFERED,\
                                     FILE_ANY_ACCESS)

// ��� ��������� ������ � �������
#define IOCTL_UsbCam_GET_STRING_DESCRIPTOR  CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+17,\
                                     METHOD_BUFFERED,\
                                     FILE_ANY_ACCESS)

// ��� �������� �� ���������� ������ FX2
#define IOCTL_USBCAM_ANCHOR_DOWNLOAD CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+27,\
                                     METHOD_IN_DIRECT,\
                                     FILE_ANY_ACCESS)


//��� ������ � "�������" �������� ������ FX2
#define IOCTL_USBCAM_VENDOR_OR_CLASS_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+22,\
                                     METHOD_IN_DIRECT,\
                                     FILE_ANY_ACCESS)

#define IOCTL_UsbCam_RESETPIPE CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+13,\
                                     METHOD_IN_DIRECT,\
                                     FILE_ANY_ACCESS)

// ��� ������ ���������� ������
#define IOCTL_USBCAM_BULK_READ CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+19,\
                                     METHOD_OUT_DIRECT,\
                                     FILE_ANY_ACCESS)

// ��� ������ ����������� ����������
#define IOCTL_UsbCam_GET_DEVICE_DESCRIPTOR CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+1,\
                                     METHOD_BUFFERED,\
                                     FILE_ANY_ACCESS)

// ��� ��������� ���� �� ��������
#define IOCTL_USBCAM_GET_LOG CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+0x7f,\
                                     METHOD_BUFFERED,\
                                     FILE_ANY_ACCESS)

// ����� �����
#define IOCTL_UsbCam_RESET   CTL_CODE(FILE_DEVICE_UNKNOWN,\
                                     USBCAM_IOCTL_INDEX+12,\
                                     METHOD_IN_DIRECT,\
                                     FILE_ANY_ACCESS)

// ��� �������� HEX ����� � FX2
#define TGT_IMG_SIZE 0x10000	// 64KB (65,536 Byte) target image
#define TGT_SEG_SIZE 16			// 16 byte segments


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
  UCHAR  iSerialNumber;
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
	LSPFP,		// ������� ������
	LS1FP10,
	LS2FP10
}device_type, *pdevice_type;

char *device_names [] = {"NONE", "LS7", "LS2FP", "LS1FP", "LSPP", "LSPPFP", "LSPFP"};


// ��������� ���������� �� ���������� � ������ _device_tree
typedef struct _device_info {
	device_type dev_type;
	SERIAL_NO SN;
	HANDLE dev;
	USB_DEVICE_DESCRIPTOR Descriptor;
} device_info, *pdevice_info;

// C������ ������������ ���������
typedef struct _device_tree {
	device_info device[MAX_DEV_COUNT];
} device_tree, *pdevice_tree;

// ��������� ���������
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
	BOOL _fastcall GetType(HANDLE hDev, pdevice_type pValue, PBYTE bitcount);
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
	// ��������� HEX ����� � ���������� MemCache �������
	BOOL __fastcall FileToCache(TMemCache* pMemCache, CHAR *pHexFileName);
	BOOL __fastcall VendorRequest(HANDLE hDev, PVENDOR_OR_CLASS_REQUEST_CONTROL pReq,WORD Len);
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

 

void __fastcall AllocateMemory(void);
void __fastcall FreeMemory(void);


#ifdef __cplusplus
}
#endif

#endif // _SCANCTRL_H


