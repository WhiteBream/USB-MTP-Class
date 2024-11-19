/*  __      __ _   _  _  _____  ____   ____  ____  ____   ___   ___  ___
    \ \_/\_/ /| |_| || ||_   _|| ___| | __ \| __ \| ___| / _ \ |   \/   |
     \      / |  _  || |  | |  | __|  | __ <|    /| __| |  _  || |\  /| |
      \_/\_/  |_| |_||_|  |_|  |____| |____/|_|\_\|____||_| |_||_| \/ |_|
*/
/*! \copyright Copyright (c) 2014-2018, White Bream, https://whitebream.nl
*************************************************************************//*!
 \file      usbd_mtp_hid_if.c
 \brief     Media Transfer Protocol
 \version   1.0.0
 \since     June 27, 2014
 \date      April 25, 2018

 Project    S110, STM32 MTP Class

 Implementation of Picture Transfer Protocol (PTP) and Media Transfer
 Protocol (MTP) for the STM32 USB implementation by ST.
****************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "usbd_mtp_hid_if.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_MTP_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_MTP_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */

/** @defgroup USBD_MTP_HID_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MTP_HID_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MTP_HID_IF_Private_Variables
 * @{
 */

/**
  * @}
  */

/** @defgroup USBD_MTP_HID_IF_Exported_Variables
  * @{
  */
  extern USBD_HandleTypeDef hUsbDeviceFS;
  extern const uint8_t HID_ReportDesc_FS[];

/**
  * @}
  */

/** @defgroup USBD_MTP_HID_Private_FunctionPrototypes
  * @{
  */
static int8_t MTP_HID_Init_FS     (void);
static int8_t MTP_HID_DeInit_FS   (void);
static int8_t MTP_HID_OutEvent_FS (uint8_t event_idx, uint8_t state);


USBD_MTP_HID_ItfTypeDef USBD_MTP_HID_fops_FS =
{
    (uint8_t*)HID_ReportDesc_FS,
    MTP_HID_Init_FS,
    MTP_HID_DeInit_FS,
    MTP_HID_OutEvent_FS,
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  MTP_HID_Init_FS
  *         Initializes the CUSTOM HID media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t MTP_HID_Init_FS(void)
{
  return (0);
}

/**
  * @brief  CUSTOM_HID_DeInit_FS
  *         DeInitializes the CUSTOM HID media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t MTP_HID_DeInit_FS(void)
{
  return (0);
}

/**
  * @brief  MTP_HID_OutEvent_FS
  *         Manage the MTP HID class events
  * @param  event_idx: event index
  * @param  state: event state
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t MTP_HID_OutEvent_FS  (uint8_t event_idx, uint8_t state)
{
  return (0);
}

/**
  * @brief  USBD_MTP_HID_SendReport_FS
  *         Send the report to the Host
  * @param  report: the report to be sent
  * @param  len: the report length
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
/*
static int8_t USBD_MTP_HID_SendReport_FS ( uint8_t *report,uint16_t len)
{
  return USBD_MTP_HID_SendReport(&hUsbDeviceFS, report, len);
}
*/

/**
  * @}
  */

/**
  * @}
  */
