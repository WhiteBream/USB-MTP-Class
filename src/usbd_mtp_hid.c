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

#include "usbd_mtp_hid.h"
#include "usbd_mtp_core.h"
#include "usbd_hid.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usb_device.h"


static uint8_t  USBD_MTP_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MTP_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_MTP_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t* USBD_MTP_HID_GetCfgDesc(uint16_t *length);
static uint8_t  USBD_MTP_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MTP_HID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_MTP_HID_EP0_RxReady(USBD_HandleTypeDef  *pdev);


USBD_ClassTypeDef  USBD_MTP_HID =
{
    USBD_MTP_HID_Init,
    USBD_MTP_HID_DeInit,
    USBD_MTP_HID_Setup,
    NULL, /*EP0_TxSent*/
    USBD_MTP_HID_EP0_RxReady, /*EP0_RxReady*/ /* STATUS STAGE IN */
    USBD_MTP_HID_DataIn, /*DataIn*/
    USBD_MTP_HID_DataOut,
    NULL, /*SOF */
    NULL,
    NULL,
	USBD_MTP_HID_GetCfgDesc,
    USBD_MTP_HID_GetCfgDesc,
    USBD_MTP_HID_GetCfgDesc,
    USBD_GetDeviceQualifierDesc,
};


/* USB CUSTOM_HID device Configuration Descriptor */
static const uint8_t USBD_MTP_HID_CfgDesc[71] =
{
    0x09, /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    WBVAL(sizeof(USBD_MTP_HID_CfgDesc)), /* wTotalLength: Bytes returned */
    2,         /*bNumInterfaces: 2 */
    0x01,         /*bConfigurationValue: Configuration value*/
    0x00,         /*iConfiguration: Index of string descriptor describing the configuration*/
    0xC0,		// Bus powered
    0x32,         /*MaxPower 0 mA */

    /************** Descriptor of HID interface ****************/
    /* 09 */
    0x09,         /*bLength: Interface Descriptor size*/
    USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
    0x00,         /*bInterfaceNumber: Number of Interface*/
    0x00,         /*bAlternateSetting: Alternate setting*/
    0x02,         /*bNumEndpoints*/
    USB_DEVICE_CLASS_HUMAN_INTERFACE,         /*bInterfaceClass: CUSTOM_HID*/
    HID_SUBCLASS_NONE,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    HID_PROTOCOL_NONE,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,            /*iInterface: Index of string descriptor*/
    /******************** Descriptor of HID *************************/
    /* 18 */
    0x09,         /*bLength: CUSTOM_HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: CUSTOM_HID*/
    WBVAL(0x0111),         /*bcdHID: release number*/
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of CUSTOM_HID class descriptors to follow*/
    HID_REPORT_DESC,         /*bDescriptorType*/
    WBVAL(sizeof(HID_ReportDesc)),/*wItemLength: Total length of Report descriptor*/
    /******************** Descriptors of HID endpoints ********************/
    /* 27 */
    0x07,          /*bLength: Endpoint Descriptor size*/
    USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
    HID_EPIN_ADDR,     /*bEndpointAddress: Endpoint Address (IN)*/
    USB_ENDPOINT_TYPE_INTERRUPT,          /*bmAttributes: Interrupt endpoint*/
    WBVAL(HID_EPIN_SIZE), /*wMaxPacketSize: 2 Byte max */
    0x20,          /*bInterval: Polling Interval (20 ms)*/
    /* 34 */
    0x07,	         /* bLength: Endpoint Descriptor size */
    USB_DESC_TYPE_ENDPOINT,	/* bDescriptorType: */
    HID_EPOUT_ADDR,  /*bEndpointAddress: Endpoint Address (OUT)*/
    USB_ENDPOINT_TYPE_INTERRUPT,	/* bmAttributes: Interrupt endpoint */
    WBVAL(HID_EPOUT_SIZE),	/* wMaxPacketSize: 2 Bytes max  */
    0x20,	/* bInterval: Polling Interval (20 ms) */
    /* 41 */

    /************** Descriptor of MTP interface ****************/
	/* Interface 1, Alternate Setting 0, MTP Class */
	0x09,           /* bLength */
	USB_DESC_TYPE_INTERFACE,     /* bDescriptorType */
	0x01,                              /* bInterfaceNumber */
	0x00,                              /* bAlternateSetting */
	3,                                 /* bNumEndpoints */
	USB_DEVICE_CLASS_IMAGE,            /* bInterfaceClass */
	1,                                 /* bInterfaceSubClass: SCI Transparent */
	1,                                 /* bInterfaceProtocol */
	USBD_IDX_CONFIG_STR,               /* iInterface */
    /******************** Descriptor of MTP endpoints ********************/
	/* Endpoint, MTP Data Out */
	0x07,            /* bLength */
	USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType */
	MTP_EPOUT_ADDR,               /* bEndpointAddress */
	USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
	WBVAL(MTP_EP_SIZE),        /* wMaxPacketSize */
	0, /* ms */                        /* bInterval */
	/* Endpoint, MTP Data In */
	0x07,            /* bLength */
	USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType */
	MTP_EPIN_ADDR,                /* bEndpointAddress */
	USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
	WBVAL(MTP_EP_SIZE),         /* wMaxPacketSize */
	0, /* ms */                        /* bInterval */
	/* Endpoint, MTP Interrupt Out */
	0x07,            /* bLength */
	USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType */
	MTP_EP2IN_ADDR,                /* bEndpointAddress */
	USB_ENDPOINT_TYPE_INTERRUPT,       /* bmAttributes */
	WBVAL(MTP_EP2_SIZE),         /* wMaxPacketSize */
	100, /* ms */                      /* bInterval */
} ;


uint8_t* USBD_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    USBD_GetString((uint8_t*)"MTP", USBD_StrDesc, length);
    return(USBD_StrDesc);
}


static uint8_t
USBD_MTP_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    uint8_t ret = 0;
    USBD_MTP_HID_HandleTypeDef     *hMtpHid;

    /* Open EP IN & out */
    USBD_LL_OpenEP(pdev, HID_EPIN_ADDR, USBD_EP_TYPE_INTR, HID_EPIN_SIZE);
    USBD_LL_OpenEP(pdev, HID_EPOUT_ADDR, USBD_EP_TYPE_INTR, HID_EPOUT_SIZE);

    /* Open MTP Endpoints */
    USBD_LL_OpenEP(pdev, MTP_EPIN_ADDR, USBD_EP_TYPE_BULK, MTP_EP_SIZE);
    USBD_LL_OpenEP(pdev, MTP_EP2IN_ADDR, USBD_EP_TYPE_INTR, MTP_EP2_SIZE);
    USBD_LL_OpenEP(pdev, MTP_EPOUT_ADDR, USBD_EP_TYPE_BULK, MTP_EP_SIZE);

    pdev->pClassData = USBD_malloc(sizeof (USBD_MTP_HID_HandleTypeDef));

    if(pdev->pClassData == NULL)
    {
        ret = 1;
    }
    else
    {
        hMtpHid = (USBD_MTP_HID_HandleTypeDef*)pdev->pClassData;

        hMtpHid->state = MTP_HID_IDLE;
        //((USBD_MTP_HID_ItfTypeDef *)pdev->pUserData[0])->Init();

        /* Prepare Out endpoints to receive 1st packet */
        USBD_LL_PrepareReceive(pdev, HID_EPOUT_ADDR, hMtpHid->Report_buf, HID_EPOUT_SIZE);
        USBD_LL_PrepareReceive(pdev, MTP_EPOUT_ADDR, hMtpHid->MtpDataBuf, MTP_EP_SIZE);
    }
    return ret;
}


/**
  * @brief  USBD_MTP_HID_Init
  *         DeInitialize the CUSTOM_HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t
USBD_MTP_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    /* Close CUSTOM_HID EP IN & out */
    USBD_LL_CloseEP(pdev, HID_EPIN_ADDR);
    USBD_LL_CloseEP(pdev, HID_EPOUT_ADDR);

    /* Open MTP Endpoints */
    USBD_LL_CloseEP(pdev, MTP_EPIN_ADDR);
    USBD_LL_CloseEP(pdev, MTP_EP2IN_ADDR);
    USBD_LL_CloseEP(pdev, MTP_EPOUT_ADDR);

    /* FRee allocated memory */
    if(pdev->pClassData != NULL)
    {
        PtpReset();

        //((USBD_MTP_HID_ItfTypeDef *)pdev->pUserData[0])->DeInit();
        USBD_free(pdev->pClassData);
        pdev->pClassData = NULL;
    }
    return USBD_OK;
}


/**
  * @brief  USBD_MTP_HID_Setup
  *         Handle the MTP_HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t
USBD_MTP_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint16_t len = 0;
    uint8_t  *pbuf = NULL;
    USBD_MTP_HID_HandleTypeDef *hMtpHid = (USBD_MTP_HID_HandleTypeDef*)pdev->pClassData;

    switch(req->bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS:
            switch(req->bRequest)
            {
                case HID_REQ_SET_PROTOCOL:
                    hMtpHid->Protocol = (uint8_t)(req->wValue);
                    break;

                case HID_REQ_GET_PROTOCOL:
                    USBD_CtlSendData(pdev, (uint8_t *)&hMtpHid->Protocol, 1);
                    break;

                case HID_REQ_SET_IDLE:
                    hMtpHid->IdleState = (uint8_t)(req->wValue >> 8);
                    break;

                case HID_REQ_GET_IDLE:
                    USBD_CtlSendData(pdev, (uint8_t*)&hMtpHid->IdleState, 1);
                    break;

                case HID_REQ_GET_REPORT:
                    len = HidDispatch(TRUE, req->wValue, vHidBuf_TX);
                    USBD_CtlSendData(pdev, vHidBuf_TX, len);
                    break;

                case HID_REQ_SET_REPORT:
                    hMtpHid->IsReportAvailable = req->wValue;
                    USBD_CtlPrepareRx(pdev, hMtpHid->Report_buf, (uint8_t)(req->wLength));
                    break;

                case 0x64:
                    USBD_CtlPrepareRx(pdev, hMtpHid->MtpCmdBuf, (uint8_t)(req->wLength));
                    break;

                case 0x66:
                    printf("Ptp_DeviceReset\n");
                    SystemReset();
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
            switch(req->bRequest)
            {
                case USB_REQ_GET_DESCRIPTOR:
                    if((req->wValue >> 8) == HID_REPORT_DESC)
                    {
                        //pbuf = ((USBD_MTP_HID_ItfTypeDef *)pdev->pUserData)->pReport;
                        pbuf = (uint8_t*)HID_ReportDesc;
                        len = MIN(sizeof(HID_ReportDesc), req->wLength);
                    }
                    else if((req->wValue >> 8) == HID_DESCRIPTOR_TYPE)
                    {
                        pbuf = (uint8_t*)USBD_HID_Desc;
                        len = MIN(sizeof(USBD_HID_Desc), req->wLength);
                    }
                    USBD_CtlSendData(pdev, (uint8_t*)pbuf, len);
                    break;

                case USB_REQ_GET_INTERFACE :
                    USBD_CtlSendData(pdev, (uint8_t*)&hMtpHid->AltSetting, 1);
                    break;

                case USB_REQ_SET_INTERFACE :
                    hMtpHid->AltSetting = (uint8_t)(req->wValue);
                    break;
            }
            break;
    }
    return USBD_OK;
}


/*******************************************************************************
* Function Name  : HID_GetReport
* Description    : get the protocol value
* Input          : Length.
* Output         : None.
* Return         : address of the protcol value.
*******************************************************************************/
uint8_t *HID_GetReport(USBD_HandleTypeDef  *pdev, uint16_t Length)
{
	return (nullptr);
}


/**
  * @brief  USBD_MTP_HID_SendReport
  *         Send MTP_HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t
USBD_HID_SendReport(USBD_HandleTypeDef  *pdev, uint8_t *report, uint16_t len)
{
  USBD_MTP_HID_HandleTypeDef *hMtpHid = (USBD_MTP_HID_HandleTypeDef*)pdev->pClassData;

    if(pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        if(hMtpHid->state == MTP_HID_IDLE)
        {
            hMtpHid->state = MTP_HID_BUSY;
            USBD_LL_Transmit (pdev, HID_EPIN_ADDR, report, len);
        }
    }
    return USBD_OK;
}


/**
  * @brief  USBD_MTP_HID_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t*
USBD_MTP_HID_GetCfgDesc(uint16_t *length)
{
    *length = sizeof (USBD_MTP_HID_CfgDesc);
    return (uint8_t*)USBD_MTP_HID_CfgDesc;
}


/**
  * @brief  USBD_MTP_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t
USBD_MTP_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    uint32_t len = 0;

    epnum |= 0x80;
    if(epnum == HID_EPIN_ADDR)
    {
        /* Ensure that the FIFO is empty before a new transfer, this condition could
        be caused by  a new transfer before the end of the previous transfer */
        ((USBD_MTP_HID_HandleTypeDef *)pdev->pClassData)->state = MTP_HID_IDLE;
    }
    else if(epnum == MTP_EPIN_ADDR)
    {
        uint8_t *pTx;

        pTx = PtpPayloadOut(MTP_EP_SIZE, &len);
        if(pTx != NULL)
        {
            USBD_LL_Transmit(pdev, MTP_EPIN_ADDR, pTx, len);
        }
        else
        {
        //    printf("EPIN STALL\n");
        //    USBD_LL_StallEP(pdev, MTP_EPIN_ADDR);
        }
    }
    else if(epnum == MTP_EP2IN_ADDR)
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
static uint8_t
USBD_MTP_HID_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_MTP_HID_HandleTypeDef *hMtpHid = (USBD_MTP_HID_HandleTypeDef*)pdev->pClassData;

    if(epnum == HID_EPOUT_ADDR)
    {
        ((USBD_MTP_HID_ItfTypeDef *)pdev->pUserData)->OutEvent(hMtpHid->Report_buf[0], hMtpHid->Report_buf[1]);

        USBD_LL_PrepareReceive(pdev, HID_EPOUT_ADDR, hMtpHid->Report_buf, HID_EPOUT_SIZE);
    }
    else if(epnum == MTP_EPOUT_ADDR)
    {
        if(PtpPayloadIn(hMtpHid->MtpDataBuf, USBD_LL_GetRxDataSize(pdev, MTP_EPOUT_ADDR)))
        {
            // Start sending the response
            USBD_MTP_HID_DataIn(pdev, MTP_EPIN_ADDR);
        }
        else
        {
            printf("ENDP2 stall\n");
            USBD_LL_StallEP(pdev, MTP_EPOUT_ADDR);
        }
        USBD_LL_PrepareReceive(pdev, MTP_EPOUT_ADDR, hMtpHid->MtpDataBuf, MTP_EP_SIZE);
    }
    return USBD_OK;
}


/**
  * @brief  USBD_MTP_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
uint8_t
USBD_MTP_HID_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
    USBD_MTP_HID_HandleTypeDef *hMtpHid = (USBD_MTP_HID_HandleTypeDef*)pdev->pClassData;

    switch(pdev->request.bmRequest & USB_REQ_TYPE_MASK)
    {
        case USB_REQ_TYPE_CLASS:
            switch(pdev->request.bRequest)
            {
                case HID_REQ_SET_REPORT:
                    HidDispatch(FALSE, hMtpHid->IsReportAvailable, hMtpHid->Report_buf);
                    hMtpHid->IsReportAvailable = 0;
                    break;

                case 0x64:
                    PtpCancelRequest(hMtpHid->MtpCmdBuf);
                    break;
            }
    }
    return USBD_OK;
}

/**
* @brief  USBD_MTP_HID_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CUSTOMHID Interface callback
  * @retval status
  */
uint8_t
USBD_MTP_HID_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MTP_HID_ItfTypeDef *fops)
{
    uint8_t  ret = USBD_FAIL;

    if(fops != NULL)
    {
        pdev->pUserData[0] = fops;
        ret = USBD_OK;
    }

    return ret;
}
