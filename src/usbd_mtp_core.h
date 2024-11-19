/*  __      __ _   _  _  _____  ____   ____  ____  ____   ___   ___  ___
    \ \_/\_/ /| |_| || ||_   _|| ___| | __ \| __ \| ___| / _ \ |   \/   |
     \      / |  _  || |  | |  | __|  | __ <|    /| __| |  _  || |\  /| |
      \_/\_/  |_| |_||_|  |_|  |____| |____/|_|\_\|____||_| |_||_| \/ |_|
*/
/*! \copyright Copyright (c) 2014-2020, White Bream, https://whitebream.nl
*************************************************************************//*!
 \file      usbd_mtp_core.h
 \brief     Media Transfer Protocol
 \version   1.0.0
 \since     June 27, 2014
 \date      May 11, 2020

 Project    S110, STM32 MTP Class

 Implementation of Picture Transfer Protocol (PTP) and Media Transfer
 Protocol (MTP) for the STM32 USB implementation by ST.
****************************************************************************/

#ifndef _USBD_MTP_CORE_H
#define _USBD_MTP_CORE_H

#ifdef __cplusplus
extern "C" {
#else
#define nullptr (void*)0
#endif

#define _GNU_SOURCE     /* Want strcasestr and similar extensions */


#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "vfs.h"


#ifdef _WHITEBREAM_H

	//#define MTP_EVENTS	// Not yet supported, requires various callbacks from file system

    #define MTP_DBG_LVL0(x, ...)    syslog("MTP", x "\n", __VA_ARGS__)
    //#define MTP_DBG_LVL1(x, ...)    syslog("MTP1", x "\n", __VA_ARGS__)
    //#define MTP_DBG_LVL2(x, ...)    syslog("MTP2", x "\n", __VA_ARGS__)
    //#define MTP_DBG_LVL3(x, ...)    syslog("MTP3", x "\n", __VA_ARGS__)

    #include "..\version.h"

    extern void FirmwareCheckRequest(void);

    /* Perform actions based on the (already closed) VfsFile_t* handle of the received file */
    #define MTP_SEND_OBJECT_HOOK(handle, path)				        	\
    {	                                                            	\
        MTP_DBG_LVL1("MTP_SEND_OBJECT_HOOK(%p,%s)", handle, path); 	\
        if (strcasestr(path, ".FWX") != nullptr) 						\
		{   															\
        	FirmwareCheckRequest();										\
		}																\
    }

    /* Definitions for some strings which are shown in the devices' property page */
    #define MTP_MANUFACTURER        (char*)pManufacturer
    #define MTP_FRIENDLYNAME        (char*)pDeviceName
    #define MTP_SERIAL              (char*)pBootVersion->serial

#else

    #define MTP_DBG_LVL0(x, ...)    iprintf("MTP " x "\n", __VA_ARGS__)
    //#define MTP_DBG_LVL1(x, ...)    iprintf("MTP1 " x "\n", __VA_ARGS__)
    //#define MTP_DBG_LVL2(x, ...)    iprintf("MTP2 " x "\n", __VA_ARGS__)
    //#define MTP_DBG_LVL3(x, ...)    iprintf("MTP3 " x "\n", __VA_ARGS__)

#endif


/* Define MTP_READONLY here to implement read-only MTP */
//#define MTP_READONLY            1


#ifdef MTP_EVENTS
typedef enum MtpEvent_e
{
    PTP_EVENT_UNDEFINED = 0x4000,
    PTP_EVENT_CANCEL_TRANSACTION,
    PTP_EVENT_OBJECT_ADDED,
    PTP_EVENT_OBJECT_REMOVED,
    PTP_EVENT_STORE_ADDED,
    PTP_EVENT_STORE_REMOVED,
    PTP_EVENT_DEVICE_PROP_CHANGED,
    PTP_EVENT_OBJECT_INFO_CHANGED,
    PTP_EVENT_DEVICE_INFO_CHANGED,
    PTP_EVENT_REQUEST_OBJECT_TRANSFER,
    PTP_EVENT_STORE_FULL,
    PTP_EVENT_DEVICE_RESET,
    PTP_EVENT_STORE_INFO_CHANGED,
    PTP_EVENT_CAPTURE_COMPLETE,
    PTP_EVENT_UNREPORTED_STATUS,
    MTP_EVENT_OBJECT_PROP_CHANGED = 0xC801,
    MTP_EVENT_OBJECT_PROP_DESC_CHANGED,
    MTP_EVENT_OBJECT_REF_CHANGED
}
MtpEvent_t;
#endif

extern uint32_t MtpFileId(const uint8_t* pDrive, const VfsInfo_t* pData);

extern bool PtpPayloadIn(uint8_t* buf, uint32_t vLength);
extern uint8_t* PtpPayloadOut(uint32_t vRequestLength, uint32_t* pLength);

extern void PtpReset(void);
extern void PtpCancelRequest(uint8_t* buf);
extern uint8_t* MtpGetDeviceStatus(uint16_t* len);

#ifdef MTP_EVENTS
bool PtpEvent(MtpEvent_t vEvent, uint32_t vParam);
#endif


#ifdef __cplusplus
}
#endif

#endif /* _USBD_MTP_CORE_H */
