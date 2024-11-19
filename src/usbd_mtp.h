/*  __      __ _   _  _  _____  ____   ____  ____  ____   ___   ___  ___
    \ \_/\_/ /| |_| || ||_   _|| ___| | __ \| __ \| ___| / _ \ |   \/   |
     \      / |  _  || |  | |  | __|  | __ <|    /| __| |  _  || |\  /| |
      \_/\_/  |_| |_||_|  |_|  |____| |____/|_|\_\|____||_| |_||_| \/ |_|
*/
/*! \copyright Copyright (c) 2014-2020 White Bream, https://whitebream.nl
*************************************************************************//*!
 \file      usbd_mtp.h
 \brief     Media Transfer Protocol
 \version   1.0.0.0
 \since     June 27, 2014
 \date      May 11, 2020

 Project    S110, STM32 MTP Class

 Implementation of Picture Transfer Protocol (PTP) and Media Transfer
 Protocol (MTP) for the STM32 USB implementation by ST.
****************************************************************************/

#ifndef _USB_MTP_H
#define _USB_MTP_H

#ifdef __cplusplus
 extern "C" {
#endif


#include "usbd_ioreq.h"
#include "usbd_mtp_core.h"


//#define MTP_EPIN_ADDR                   0x82
//#define MTP_EPOUT_ADDR                  0x03
//#define MTP_EP2IN_ADDR                  0x83
#define MTP_EPIN_ADDR                   0x81
#define MTP_EPOUT_ADDR                  0x01
#define MTP_EP2IN_ADDR                  0x82

#define MTP_EP_SIZE                     64
#define MTP_EP2_SIZE                    8

#define USB_ENDPOINT_TYPE_BULK          0x02
#define USB_ENDPOINT_TYPE_INTERRUPT     0x03

#define MTP_DESCRIPTOR_TYPE             0x21
#define MTP_REPORT_DESC                 0x22

#ifndef WBVAL
#define WBVAL(x) ((x) & 0xFF),(((x) >> 8) & 0xFF)
#endif


typedef struct _USBD_MTP_Itf
{
    uint8_t                  *pReport;
    int8_t (* Init)          (void);
    int8_t (* DeInit)        (void);
    int8_t (* OutEvent)      (uint8_t, uint8_t );
}
USBD_MTP_ItfTypeDef;

typedef struct
{
	uint8_t  MtpCmdBuf[10];
	uint8_t  MtpDataBuf[MTP_EP_SIZE];

    uint32_t AltSetting;
}
USBD_MTP_HandleTypeDef;


extern USBD_ClassTypeDef  USBD_MTP;
#define USBD_MTP_CLASS    &USBD_MTP


uint8_t USBD_MTP_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MTP_ItfTypeDef *fops);
uint8_t USBD_MTP_SendInterruptData(uint8_t* buf, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif  /* _USB_MTP_H */
