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

#ifndef _USBD_MTP_HID_IF_H_
#define _USBD_MTP_HID_IF_H_

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "usbd_ioreq.h"
#include "usbd_mtp_hid.h"


/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_MTP_HID
  * @brief header
  * @{
  */

/** @defgroup USBD_MTP_HID_Exported_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MTP_HID_Exported_Types
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MTP_HID_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MTP_HID_Exported_Variables
  * @{
  */
  extern USBD_MTP_HID_ItfTypeDef  USBD_MTP_HID_fops_FS;

/**
  * @}
  */

/** @defgroup USBD_MTP_HID_Exported_FunctionsPrototype
  * @{
  */

/**
  * @}
  */

/**
  * @}
  */

/**
* @}
*/

#ifdef __cplusplus
}
#endif

#endif /* _USBD_MTP_HID_IF_H_ */
