/*  __      __ _   _  _  _____  ____   ____  ____  ____   ___   ___  ___
    \ \_/\_/ /| |_| || ||_   _|| ___| | __ \| __ \| ___| / _ \ |   \/   |
     \      / |  _  || |  | |  | __|  | __ <|    /| __| |  _  || |\  /| |
      \_/\_/  |_| |_||_|  |_|  |____| |____/|_|\_\|____||_| |_||_| \/ |_|
*/
/*! \copyright Copyright (c) 2014-2024, White Bream, https://whitebream.nl
*************************************************************************//*!
 Implementation of Picture Transfer Protocol (PTP) and Media Transfer
 Protocol (MTP) for the STM32 USB implementation by ST.
****************************************************************************/

#ifndef _USBD_MTP_HID_H
#define _USBD_MTP_HID_H

#ifdef __cplusplus
 extern "C" {
#endif


#include "usbd_ioreq.h"
#include "usbd_mtp.h"


//#define HID_EPIN_ADDR               0x81
//#define HID_EPOUT_ADDR              0x01
#define HID_EPIN_ADDR               0x83
#define HID_EPOUT_ADDR              0x03

#define HID_EPIN_SIZE               64
#define HID_EPOUT_SIZE              64

//#define MTP_EPOUT_ADDR            0x03
//#define MTP_EPIN_ADDR             0x82
//#define MTP_EP2IN_ADDR            0x83
//#define MTP_EP_SIZE               64
//#define MTP_EP2_SIZE              8

#define USB_HID_DESC_SIZ            9

#define HID_DESCRIPTOR_TYPE         0x21
#define HID_REPORT_DESC             0x22

#define HID_REQ_SET_PROTOCOL        0x0B
#define HID_REQ_GET_PROTOCOL        0x03

#define HID_REQ_SET_IDLE            0x0A
#define HID_REQ_GET_IDLE            0x02

#define HID_REQ_SET_REPORT          0x09
#define HID_REQ_GET_REPORT          0x01


typedef enum
{
    MTP_HID_IDLE = 0,
    MTP_HID_BUSY,
}
MTP_HID_StateTypeDef;

typedef struct _USBD_MTP_HID_Itf
{
    uint8_t *pReport;
    int8_t (* Init)(void);
    int8_t (* DeInit)(void);
    int8_t (* OutEvent)(uint8_t, uint8_t );
}
USBD_MTP_HID_ItfTypeDef;

typedef struct
{
	uint8_t     MtpCmdBuf[10];
	uint8_t     MtpDataBuf[MTP_EP_SIZE];

    uint8_t     Report_buf[USB_MAX_EP0_SIZE];
    uint32_t    Protocol;
    uint32_t    IdleState;
    uint32_t    AltSetting;
    uint32_t    IsReportAvailable;
    MTP_HID_StateTypeDef     state;
}
USBD_MTP_HID_HandleTypeDef;


extern USBD_ClassTypeDef  USBD_MTP_HID;
#define USBD_MTP_HID_CLASS    &USBD_MTP_HID


uint8_t USBD_MTP_HID_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);
uint8_t USBD_MTP_HID_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MTP_HID_ItfTypeDef *fops);
uint8_t USBD_MTP_SendInterruptData(uint8_t* buf, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif  /* _USBD_MTP_HID_H */
