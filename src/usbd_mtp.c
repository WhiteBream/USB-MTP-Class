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


#include "usbd_mtp.h"
#include "usbd_mtp_core.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usb_device.h"


// Not defined in usb_device.h
extern USBD_HandleTypeDef hUsbDeviceFS;


static uint8_t  USBD_MTP_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MTP_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MTP_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t* USBD_MTP_GetCfgDesc(uint16_t *length);
static uint8_t  USBD_MTP_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MTP_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MTP_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t* USBD_MTP_GetDeviceQualifierDesc(uint16_t *length);


USBD_ClassTypeDef USBD_MTP =
{
    USBD_MTP_Init,
    USBD_MTP_DeInit,
    USBD_MTP_Setup,
    NULL, /*EP0_TxSent*/
    USBD_MTP_EP0_RxReady, /*EP0_RxReady*/ /* STATUS STAGE IN */
    USBD_MTP_DataIn, /*DataIn*/
    USBD_MTP_DataOut,
    NULL, /*SOF */
    NULL,
    NULL,
    USBD_MTP_GetCfgDesc,
    USBD_MTP_GetCfgDesc,
    USBD_MTP_GetCfgDesc,
	USBD_MTP_GetDeviceQualifierDesc,
};


/* USB CUSTOM_HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MTP_CfgDesc[39] __ALIGN_END =
{
    0x09, /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    WBVAL(sizeof(USBD_MTP_CfgDesc)),  /* wTotalLength: Bytes returned */
    0x01,         /*bNumInterfaces: 1 interface*/
    0x01,         /*bConfigurationValue: Configuration value*/
    0x00,         /*iConfiguration: Index of string descriptor describing
    the configuration*/
    0xC0,         /*bmAttributes: bus powered */
    0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Descriptor of MTP interface ****************/
    /* 09 */
    0x09,         /*bLength: Interface Descriptor size*/
    USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
    0x00,         /*bInterfaceNumber: Number of Interface*/
    0x00,         /*bAlternateSetting: Alternate setting*/
    0x03,         /*bNumEndpoints*/
    0x06,         /*bInterfaceClass: MTP*/
    0x01,         /*bInterfaceSubClass */
    0x01,         /*nInterfaceProtocol */
    USBD_IDX_CONFIG_STR,   /*iInterface: Index of string descriptor*/
    /******************** Descriptors of MTP endpoints ********************/
    /* 18 */
    0x07,	         /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,	/* bDescriptorType: */
    MTP_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
    USB_ENDPOINT_TYPE_BULK,	/* bmAttributes: Interrupt endpoint */
	WBVAL(MTP_EP_SIZE),	/* wMaxPacketSize: 64 Bytes max  */
    0,			/* bInterval */
    /* 25 */
    0x07,          /*bLength: Endpoint Descriptor size*/
    USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
    MTP_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
    USB_ENDPOINT_TYPE_BULK,          /*bmAttributes: Interrupt endpoint*/
	WBVAL(MTP_EP_SIZE), /*wMaxPacketSize: 64 Byte max */
    0,          			/*bInterval */
    /* 32 */
    0x07,            /* bLength */
    USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType */
    MTP_EP2IN_ADDR,                /* bEndpointAddress */
    USB_ENDPOINT_TYPE_INTERRUPT,       /* bmAttributes */
	WBVAL(MTP_EP2_SIZE),         /* wMaxPacketSize */
    100, /* ms */                      /* bInterval */
    /* 39 */
} ;

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MTP_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]  __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  MTP_EP_SIZE,
  0x01,
  0x00,
};


/**
  * @brief  USBD_MTP_Init
  *         Initialize the MTP interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MTP_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    uint8_t ret = 0;
    USBD_MTP_HandleTypeDef *hMtp;

    /* Open EP IN */
    USBD_LL_OpenEP(pdev, MTP_EPIN_ADDR, USBD_EP_TYPE_BULK, MTP_EP_SIZE);
    /* Open EP OUT */
    USBD_LL_OpenEP(pdev, MTP_EPOUT_ADDR, USBD_EP_TYPE_BULK, MTP_EP_SIZE);

    USBD_LL_OpenEP(pdev, MTP_EP2IN_ADDR, USBD_EP_TYPE_INTR, MTP_EP2_SIZE);

    if (pdev->pClassData = USBD_malloc(sizeof (USBD_MTP_HandleTypeDef)), pdev->pClassData == nullptr)
    {
        ret = 1;
    }
    else
    {
    	hMtp = (USBD_MTP_HandleTypeDef*)pdev->pClassData;

        /* Prepare Out endpoint to receive 1st packet */
        USBD_LL_PrepareReceive(pdev, MTP_EPOUT_ADDR, hMtp->MtpDataBuf, MTP_EP_SIZE);
    }
    return ret;
}

/**
  * @brief  USBD_MTP_Init
  *         DeInitialize the MTP layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MTP_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    /* Close CUSTOM_HID EP IN */
    USBD_LL_CloseEP(pdev, MTP_EPIN_ADDR);

    /* Close CUSTOM_HID EP OUT */
    USBD_LL_CloseEP(pdev, MTP_EPOUT_ADDR);

    USBD_LL_CloseEP(pdev, MTP_EP2IN_ADDR);

    /* FRee allocated memory */
    if (pdev->pClassData != nullptr)
    {
        PtpReset();

        USBD_free(pdev->pClassData);
        pdev->pClassData = nullptr;
    }
    return USBD_OK;
}

/**
  * @brief  USBD_MTP_Setup
  *         Handle the MTP specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_MTP_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint16_t len = 0;
    uint8_t  *pbuf = NULL;
    USBD_MTP_HandleTypeDef *hMtp = (USBD_MTP_HandleTypeDef*)pdev->pClassData;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS :
            switch (req->bRequest)
            {
                case 0x64:
                    USBD_CtlPrepareRx(pdev, hMtp->MtpCmdBuf, (uint8_t)(req->wLength));
                    break;

                case 0x66:
                    printf("Ptp_DeviceReset\n");
                    NVIC_SystemReset();
                    break;

                case 0x67:
                    pbuf = MtpGetDeviceStatus(&len);
                    USBD_CtlSendData(pdev, (uint8_t*)pbuf, len);
                    break;

                default:
                    USBD_CtlError(pdev, req);
                    return USBD_FAIL;
            }
            break;

        case USB_REQ_TYPE_STANDARD:
            switch (req->bRequest)
            {
                case USB_REQ_GET_INTERFACE :
                    USBD_CtlSendData(pdev, (uint8_t *)&hMtp->AltSetting, 1);
                    break;

                case USB_REQ_SET_INTERFACE :
                	hMtp->AltSetting = (uint8_t)(req->wValue);
                    break;
            }
    }
    return USBD_OK;
}

/**
  * @brief  USBD_MTP_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MTP_GetCfgDesc(uint16_t *length)
{
    *length = sizeof (USBD_MTP_CfgDesc);
    return USBD_MTP_CfgDesc;
}

/**
  * @brief  USBD_MTP_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_MTP_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    uint32_t len = 0;
    //USBD_MTP_HandleTypeDef *hMtp = (USBD_MTP_HandleTypeDef*)pdev->pClassData;

    epnum |= 0x80;
    if (epnum == MTP_EPIN_ADDR)
    {
        uint8_t *pTx;

        if (pTx = PtpPayloadOut(MTP_EP_SIZE, &len), pTx != nullptr)
        {
            USBD_LL_Transmit(pdev, MTP_EPIN_ADDR, pTx, len);
        }
        else
        {
        //    printf("EPIN STALL\n");
        //    USBD_LL_StallEP(pdev, MTP_EPIN_ADDR);
        }
    }
    else if (epnum == MTP_EP2IN_ADDR)
    {
    }
    return USBD_OK;
}

/**
  * @brief  USBD_MTP_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_MTP_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_MTP_HandleTypeDef *hMtp = (USBD_MTP_HandleTypeDef*)pdev->pClassData;

    if (epnum == MTP_EPOUT_ADDR)
    {
        if (PtpPayloadIn(hMtp->MtpDataBuf, USBD_LL_GetRxDataSize(pdev, MTP_EPOUT_ADDR)))
        {
            // Start sending the response
        	USBD_MTP_DataIn(pdev, MTP_EPIN_ADDR);
        }
        else
        {
            printf("ENDP2 stall\n");
            USBD_LL_StallEP(pdev, MTP_EPOUT_ADDR);
        }
        USBD_LL_PrepareReceive(pdev, MTP_EPOUT_ADDR, hMtp->MtpDataBuf, MTP_EP_SIZE);
    }
    return USBD_OK;
}

/**
  * @brief  USBD_MTP_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_MTP_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
    USBD_MTP_HandleTypeDef *hMtp = (USBD_MTP_HandleTypeDef*)pdev->pClassData;

    switch (pdev->request.bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS:
            switch (pdev->request.bRequest)
            {
                case 0x64:
                    PtpCancelRequest(hMtp->MtpCmdBuf);
                    break;
            }
    }
    return USBD_OK;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t*
USBD_MTP_GetDeviceQualifierDesc(uint16_t *length)
{
    *length = sizeof(USBD_MTP_DeviceQualifierDesc);
    return((uint8_t*)USBD_MTP_DeviceQualifierDesc);
}

/**
* @brief  USBD_MTP_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: MTP Interface callback
  * @retval status
  */
uint8_t USBD_MTP_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MTP_ItfTypeDef *fops)
{
    uint8_t ret = USBD_FAIL;

    if (fops != NULL)
    {
        pdev->pUserData[pdev->classId] = fops;
        ret = USBD_OK;
    }
    return ret;
}

uint8_t USBD_MTP_SendInterruptData(uint8_t* buf, uint32_t len)
{
    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
    {
        USBD_LL_Transmit(&hUsbDeviceFS, MTP_EP2IN_ADDR, buf, len);
    }
    return(len);
}
