/*  __      __ _   _  _  _____  ____   ____  ____  ____   ___   ___  ___
    \ \_/\_/ /| |_| || ||_   _|| ___| | __ \| __ \| ___| / _ \ |   \/   |
     \      / |  _  || |  | |  | __|  | __ <|    /| __| |  _  || |\  /| |
      \_/\_/  |_| |_||_|  |_|  |____| |____/|_|\_\|____||_| |_||_| \/ |_|
*/
/*! \copyright Copyright (c) 2014-2021, White Bream, https://whitebream.nl
*************************************************************************//*!
 \file      usbd_mtp.c
 \brief     Media Transfer Protocol
 \version   1.0.0
 \since     June 27, 2014
 \date      June 29, 2021

 Project    S110, STM32 MTP Class

 Implementation of Picture Transfer Protocol (PTP) / Media Transfer
 Protocol (MTP) for the STM32 USB implementation by ST.
****************************************************************************/

/*! \copyright License information
 *
 This software and accompanying documentation are protected by copyright law
 and also by International Treaty provisions. Any use of this software in
 violation of copyright law or the terms of this agreement will be
 prosecuted to the best of our ability.

 White Bream authorizes you to make archival copies of this software for the
 sole purpose of back-up and protecting your investment from loss. Under no
 circumstances may you copy this software or documentation for the purposes
 of distribution to others. Under no conditions may you remove the copyright
 notices made part of the software or documentation.

 You may distribute your own compiled programs based on any of the source
 code of the S110 MTP Class. You may not distribute any of the S110 MTP Class
 source code, compiled units, or compiled example programs without the express
 written approval of an Executive Officer of White Bream. You may not use S110
 MTP Class to create modules to be used by other developers without the
 express written approval of an Executive Officer of White Bream.

 Note that the previous restrictions do not prohibit you from distributing
 your own source code or modules that depend upon S110 MTP Class. However,
 others who receive your source code or modules need to purchase their own
 copies of S110 MTP Class in order to compile the source code or to write
 programs that use your modules.

 One single license may be used for one type of device only, to be specified
 upon purchase of the product. In case of royalty based licensing, Buyer shall
 update the license when the licensed quantity is passed.

 White Bream warrants that the software will function as described in the
 documentation for a period of 60 days from receipt. If you encounter a bug
 or deficiency, we will require a problem report detailed enough to allow
 us to find and fix the problem. If you properly notify us of such a software
 problem within the warranty period, White Bream will update the defective
 software at no cost to you.

 WHITE BREAM DOES NOT ASSUME ANY LIABILITY FOR THE USE OF S110 MTP CLASS
 BEYOND THE ORIGINAL PURCHASE PRICE OF THE SOFTWARE. IN NO EVENT WILL WHITE
 BREAM BE LIABLE TO YOU FOR ADDITIONAL DAMAGES, INCLUDING ANY LOST PROFITS,
 LOST SAVINGS, OR OTHER INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF
 THE USE OF OR INABILITY TO USE THESE PROGRAMS, EVEN IF WHITE BREAM HAS BEEN
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

 White Bream products are not authorized for use in, or in connection with
 surgical implants, or as critical components in any medical, or aircraft,
 or other transportation devices or systems where failure to perform can
 reasonably be expected to cause significant injury to the user, without
 the express written approval of an Executive Officer of White Bream. Such
 use is at Buyer's sole risk, and Buyer is responsible for verification and
 validation of the suitability of products incorporated in any such devices
 or systems. Buyer agrees that White Bream is not liable, in whole or in
 part, for any claim or damage arising from such use and shall have no
 obligation to warranty such products. Buyer agrees to indemnify, defend
 and hold White Bream harmless from and against any and all claims, damages
 losses, costs, expenses and liabilities arising out of or in connection with
 Buyer's use of White Bream products in such applications to the extent Buyer
 has not obtained the express written approval of an Executive Officer of
 White Bream.

 All brand and product names are trademarks or registered trademarks of
 their respective holders. All sales are subject to the general conditions
 by FENIT.
****************************************************************************/

#include "usbd_mtp_core.h"
#include "usbd_mtp.h"

#include "usb_device.h"
#include "usbd_conf.h"
#include "usbd_core.h"


/* Not all MTP functions need to be in use. Prevent warnings for the unused ones */
#pragma GCC diagnostic ignored "-Wunused-function"
/* Some variables remain unused, depending on the debuf message levels */
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"


/* STANDARD_RESPONSECODE */
#define PtpErr_Undefined                                0x2000
#define OK                                              0x2001
#define PtpErr_GeneralError                             0x2002
#define PtpErr_SessionNotOpen                           0x2003
#define PtpErr_InvalidTransactionId                     0x2004
#define PtpErr_OperationNotSupported                    0x2005
#define PtpErr_ParameterNotSupported                    0x2006
#define PtpErr_IncompleteTransfer                       0x2007
#define PtpErr_InvalidStorageId                         0x2008
#define PtpErr_InvalidObjectHandle                      0x2009
#define PtpErr_DevicepropNotSupported                   0x200A
#define PtpErr_InvalidObjectFormatCode                  0x200B
#define PtpErr_StoreFull                                0x200C
#define PtpErr_ObjectWriteProtected                     0x200D
#define PtpErr_StoreReadOnly                            0x200E
#define PtpErr_AccessDenied                             0x200F
#define PtpErr_NoThumbnailPresent                       0x2010
#define PtpErr_SelftestFailed                           0x2011
#define PtpErr_PartialDeletion                          0x2012
#define PtpErr_StoreNotAvailable                        0x2013
#define PtpErr_SpecificationByFormatUnsupported         0x2014
#define PtpErr_NoValidObjectInfo                        0x2015
#define PtpErr_InvalidCodeFormat                        0x2016
#define PtpErr_UnknownVendorCode                        0x2017
#define PtpErr_CaptureAlreadyTerminated                 0x2018
#define PtpErr_DeviceBusy                               0x2019
#define PtpErr_InvalidParentObject                      0x201A
#define PtpErr_InvalidDevicePropFormat                  0x201B
#define PtpErr_InvalidDevicePropValue                   0x201C
#define PtpErr_InvalidParameter                         0x201D
#define PtpErr_SessionAlreadyOpen                       0x201E
#define PtpErr_TransactionCancelled                     0x201F
#define PtpErr_SpecificationOfDestinationUnsupported    0x2020
#define PtpErr_InvalidObjectPropCode                    0xA801
#define PtpErr_InvalidObjectPropFormat                  0xA802
#define PtpErr_InvalidObjectPropValue                   0xA803
#define PtpErr_InvalidObjectReference                   0xA804
#define PtpErr_GroupNotSupported                        0xA805
#define PtpErr_InvalidDataset                           0xA806
#define PtpErr_SpecificationByGroupUnsupported          0xA807
#define PtpErr_SpecificationByDepthUnsupported          0xA808
#define PtpErr_ObjectTooLarge                           0xA809
#define PtpErr_ObjectPropNotSupported                   0xA80A

#define PTPVERSION		        100
#define MTPVERSION		        100
#define FUNCTIONALMODE	        0x0000

#define MAX_ROOT_LENGTH         10      // Max length of any FF_VOLUME_STRS string + 3 chars

#define STORAGE_ID(x)           ((((x) + 1) << 16) + 1)
#define DRIVE_NUM(x)            (((x) >> 16) - 1)

#if VFS_NODIRS != 1
#define MTP_FOLDER_CACHE_FILE	"/_.MTP"
#endif

#define PTP_BUF_SIZE    MTP_EP_SIZE
//#define PTP_BUF_SIZE    512

#ifndef MTP_SEND_OBJECT_HOOK
    #define MTP_SEND_OBJECT_HOOK(handle, path)
#endif

#ifndef MTP_SESSION_OPEN_HOOK
    #define MTP_SESSION_OPEN_HOOK(session)
#endif

#ifndef MTP_SESSION_CLOSE_HOOK
    #define MTP_SESSION_CLOSE_HOOK()
#endif

#ifndef MTP_DBG_LVL0
    #define MTP_DBG_LVL0(x, ...)
#endif
#ifndef MTP_DBG_LVL1
    #define MTP_DBG_LVL1(x, ...)
#endif
#ifndef MTP_DBG_LVL2
    #define MTP_DBG_LVL2(x, ...)
#endif
#ifndef MTP_DBG_LVL3
    #define MTP_DBG_LVL3(x, ...)
#endif

#if (MTP_READONLY == 0)
    #undef MTP_READONLY
#endif
#ifdef MTP_FATFS
    #if (FF_FS_READONLY != 0) && (MTP_READONLY != 1)
        #define MTP_READONLY FF_FS_READONLY
    #endif
#endif

#ifndef MTP_MANUFACTURER
    #define MTP_MANUFACTURER        "White Bream"
#endif
#ifndef MTP_FRIENDLYNAME
    #define MTP_FRIENDLYNAME        "STM32 MTP Class"
#endif
#ifndef MTP_SERIAL
    #define MTP_SERIAL              "0123456789"
#endif


static VfsFile_t vSendObjectHandle = {0};
static uint32_t vCurrentParent = 0;
static uint32_t vFolderCacheDirty = 0;

#if (FF_USE_LFN <= 2)
#define STATIC_WORKPATH
#endif
#ifdef STATIC_WORKPATH
static char vWorkPath[MAX_PATH + 1];    // Static to keep track of current directory
static VfsInfo_t vFilInfo;  // Cached to accommodate multiple property request on a file
#endif


uint32_t
HandleFilenameBits(char* input)
{
    uint32_t crc = 0xFFFFFFFF;
    char buf[4] = {0};
    size_t size = strlen(input);

    static const uint32_t CrcTable[16] =
    {   // Nibble lookup table for 0x04C11DB7 polynomial
        0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
        0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD
    };

    while (size > 0)
    {
        if (size < 4)
        {
            memcpy(buf, input, size);
            crc = crc ^ *(uint32_t*)buf; // Apply all 32-bits
            size = 0;
        }
        else
        {
            crc = crc ^ *(uint32_t*)input; // Apply all 32-bits
            input += sizeof(uint32_t);
            size -= sizeof(uint32_t);
        }
        // Process 32-bits, 4 at a time, or 8 rounds
        crc = (crc << 4) ^ CrcTable[crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
        crc = (crc << 4) ^ CrcTable[crc >> 28]; //  0x04C11DB7 Polynomial used in STM32
        crc = (crc << 4) ^ CrcTable[crc >> 28];
        crc = (crc << 4) ^ CrcTable[crc >> 28];
        crc = (crc << 4) ^ CrcTable[crc >> 28];
        crc = (crc << 4) ^ CrcTable[crc >> 28];
        crc = (crc << 4) ^ CrcTable[crc >> 28];
        crc = (crc << 4) ^ CrcTable[crc >> 28];
    }
    crc &= INODE_ITEM_MASK;
    if (crc == 1)	// file ID 0 is reserved by MTP
    {
        crc = 1;
    }
    if (crc == INODE_ITEM_MASK)	// file ID -1 is used for directory entries
    {
        crc = INODE_ITEM_MASK - 1;
    }
    return(crc);
}


static bool
GetFileById(VfsInfo_t** pFil, uint32_t handle, bool parent, char** pPath)
{
    static char* pWorkPath = nullptr;    // Static to keep track of current directory
    static size_t vPathLen = 0;
    static VfsInfo_t* pFilInfo;  // Cached to accommodate multiple property request on a file
    static uint32_t vPreviousHandle = UINT32_MAX;

    VfsDir_t scanhandle;
    char* p;
    char* prev = nullptr;
    bool prevmatched;
    bool root = false;
    bool ret = false;

    if ((pFil == nullptr) && (handle == 0) && (pPath == nullptr))
    {
    #ifndef STATIC_WORKPATH
        if (pWorkPath != nullptr)
        {
            free(pWorkPath);
        }
        pWorkPath = nullptr;

        if (pFilInfo != nullptr)
        {
            free(pFilInfo);
        }
        pFilInfo = nullptr;
    #endif

        return(false);
    }
    else
    {
    #ifdef STATIC_WORKPATH
        if (pWorkPath == nullptr)
        {
            pWorkPath = vWorkPath;
        }

        if (pFilInfo == nullptr)
        {
            pFilInfo = &vFilInfo;
        }
    #else
        if (pWorkPath == nullptr)
        {
            pWorkPath = malloc(MAX_PATH + 1);
        }

        if (pFilInfo == nullptr)
        {
            pFilInfo = malloc(sizeof(VfsInfo_t));
        }
    #endif
    }

    // Check if handle is cached
    if ((handle == vPreviousHandle) && !parent)
    {
        if (pFil != nullptr)
        {
            *pFil = pFilInfo;
        }
        ret = true;
    }
    else
    {
        if (handle == 0) // NOTE catch 0 handles - not sure why we see this
        {
            handle = vPreviousHandle;
        }
        else
        {
			int err;

        	vPreviousHandle ^= handle;

        	if (vPreviousHandle & INODE_STORAGE_MASK)	// Different storage device from previous request
			{
                if (p = vfs_volume(INODE_STORAGE(handle)), p != nullptr)
                {
                    sniprintf(pWorkPath, MAX_PATH + 1, "%s/", p);  // Set drive root
                }
				vCurrentParent = (handle & INODE_STORAGE_MASK);
			}
			if (vPreviousHandle & (INODE_STORAGE_MASK | INODE_FOLDER_MASK))	// Different folder since last request
			{
            	vPreviousHandle = handle & (INODE_FOLDER_MASK | INODE_STORAGE_MASK);

            	if ((handle & INODE_FOLDER_MASK) == INODE_FOLDER_MASK)	// Root?
            	{
					if (p = strchr(pWorkPath, '/'), p != nullptr)
					{
						p[1] = '\0';
					}
					if (parent)
					{
						vCurrentParent = (handle & INODE_STORAGE_MASK) | INODE_FOLDER_MASK;
					}
   	   	   	   	   	vPathLen = strlen(pWorkPath);
            	}
            	else
            	{
				#if VFS_NODIRS != 1
            		uint32_t x = 0;
            		char cachePath[13];

					// Fetch folder entry from cache file
                    sniprintf(cachePath, sizeof(cachePath), "%s%s", vfs_volume(INODE_STORAGE(handle)), MTP_FOLDER_CACHE_FILE);
					if (err = vfs_file_open(&vSendObjectHandle, cachePath, VFS_RDONLY), err == 0)
					{
						while (vfs_gets(pFilInfo->name, sizeof(pFilInfo->name), &vSendObjectHandle) != nullptr)
						{
							if (++x == INODE_FOLDER(handle))
							{
			                    sniprintf(pWorkPath, MAX_PATH + 1, "%s%s", vfs_volume(INODE_STORAGE(handle)), pFilInfo->name);
			                    if (p = strchr(pWorkPath, '\n'), p != nullptr)
			                    {
			                    	*p = '\0';
			                    }
			   	   	   	   	   	if (parent)
			   	   	   	   	   	{
			   	   	   	   	   		vCurrentParent = handle;
			   	   	   	   	 //XXX  		vPathLen = strlen(pWorkPath);
			   	   	   	   	   	}
		   	   	   	   	   		vPathLen = strlen(pWorkPath);
			   	   	   	   	   	break;
							}
						}
						vfs_file_close(&vSendObjectHandle);
					}
					if (err != 0)
					{
						MTP_DBG_LVL0("%s[%u] %s (%s)...", __FUNCTION__, __LINE__, strerror(-err), cachePath);
					}
				#endif
            	}
			}
			else if (vPathLen != 0)
			{
				pWorkPath[vPathLen] = '\0';
			}

			if ((handle & INODE_ITEM_MASK) == 0)	// Looking for the folder entry itself?
			{
				if (vfs_stat(pWorkPath, pFilInfo) == 0)
				{
					ret = true;
				}
			}
			else
			{
				// Search folder for handle
				if (err = vfs_dir_open(&scanhandle, pWorkPath), err == 0)
				{
					prevmatched = !root;

					while (vfs_dir_read(&scanhandle, pFilInfo) == 0)
					{
						if (pFilInfo->name[0] == '.')
						{
							// Skip self and parent directory entries
							if ((pFilInfo->name[1] == '\0') || ((pFilInfo->name[1] == '.') && (pFilInfo->name[2] == '\0')))
							{
								continue;
							}
						}
						if ((HandleFilenameBits(pFilInfo->name) | vCurrentParent) == handle)
						{
							if (p = strrchr(pWorkPath, '/'), (p == nullptr) || (p[1] != '\0'))
							{
								strcat(pWorkPath, "/");
							}
							strcat(pWorkPath, pFilInfo->name);
							MTP_DBG_LVL3("%lX -> %s", handle, pWorkPath);

							ret = true;
							break;
						}
					}
					vfs_dir_close(&scanhandle);
				}
				if (err != 0)
				{
					MTP_DBG_LVL0("%s[%u] %s (%s)...", __FUNCTION__, __LINE__, strerror(-err), pWorkPath);
				}
			}
        	vPreviousHandle = handle;
        }
    }
	if (pFil != nullptr)
	{
		*pFil = pFilInfo;
	}
    if (ret && (pPath != nullptr))
    {
        *pPath = pWorkPath;
    }
    return(ret);
}


void
ByteBuffer(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint8_t var)
{
    if (*index > 0)
    {
        (*index)--;
    }
    else if (*reqlen > 0)
    {
        if (*buf != nullptr)
        {
            *(*buf)++ = var;
        }
        (*reqlen)--;
    }
}


static uint16_t
GetUint16(uint8_t* buf)
{
    uint16_t p = (buf[1] << 8) | buf[0];
    return(p);
}


static uint32_t
GetUint32(uint8_t* buf)
{
    uint32_t p = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
    return(p);
}


static uint8_t
Uint8(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint8_t var)
{
    uint8_t i;

    for (i = 0; i < sizeof(uint8_t); i++)
    {
        ByteBuffer(buf, index, reqlen, (var >> (i * 8)) & 0xFF);
    }
    return((uint8_t)sizeof(uint8_t));
}


static uint8_t
Uint16(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint16_t var)
{
    uint8_t i;

    for (i = 0; i < sizeof(uint16_t); i++)
    {
        ByteBuffer(buf, index, reqlen, (var >> (i * 8)) & 0xFF);
    }
    return((uint8_t)sizeof(uint16_t));
}


static uint8_t
Uint32(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t var)
{
    uint8_t i;

    for (i = 0; i < sizeof(uint32_t); i++)
    {
        ByteBuffer(buf, index, reqlen, (var >> (i * 8)) & 0xFF);
    }
    return((uint8_t)sizeof(uint32_t));
}


static uint8_t
Uint64(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint64_t var)
{
    uint8_t i;

    for (i = 0; i < sizeof(uint64_t); i++)
    {
        ByteBuffer(buf, index, reqlen, (var >> (i * 8)) & 0xFF);
    }
    return((uint8_t)sizeof(uint64_t));
}


static uint8_t
String(uint8_t** buf, uint32_t* index, uint32_t* reqlen, const char* str)
{
    uint8_t i = 0;

    if (str != nullptr)
    {
        for (i = 0; (str[i] != '\0') && (i < UINT8_MAX); i++)
        {}
        ByteBuffer(buf, index, reqlen, i + 1);

        for (i = 0; (str[i] != '\0') && (i < UINT8_MAX); i++)
        {
            ByteBuffer(buf, index, reqlen, str[i]);
            ByteBuffer(buf, index, reqlen, 0);
        }
        ByteBuffer(buf, index, reqlen, 0);
        ByteBuffer(buf, index, reqlen, 0);
        i++;
    }
    else
    {
        ByteBuffer(buf, index, reqlen, 0);
    }
    return((i << 1) + 1);
}


static uint8_t
StringWchar(uint8_t** buf, uint32_t* index, uint32_t* reqlen, const wchar_t* str)
{
    uint8_t i = 0;

    if (str != nullptr)
    {
        for (i = 0; (str[i] != '\0') && (i < UINT8_MAX); i++)
        {}
        ByteBuffer(buf, index, reqlen, i);

        for (i = 0; (str[i] != '\0') && (i < UINT8_MAX); i++)
        {
            ByteBuffer(buf, index, reqlen, HIBYTE(str[i]));
            ByteBuffer(buf, index, reqlen, LOBYTE(str[i]));
        }
        ByteBuffer(buf, index, reqlen, 0);
        ByteBuffer(buf, index, reqlen, 0);
    }
    else
    {
        ByteBuffer(buf, index, reqlen, 0);
    }
    return(i + 1);
}


static uint32_t PtpDeviceInfo(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpOpenSession(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpCloseSession(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpGetStorageIds(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpGetStorageInfo(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpGetObjectHandles(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpGetObjectInfo(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpGetObject(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpDeleteObject(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpSendObjectInfo(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpSendObjectInfoData(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
//static uint32_t PtpSendObjectInfoResponse(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpSendObject(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpSendObjectData(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpFormatStore(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);

static uint32_t MtpGetObjectPropsSupported(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t MtpGetObjectPropDesc(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t MtpGetObjectPropValue(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
#if (MTP_READONLY != 1)
static uint32_t MtpSetObjectPropValue(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t MtpSetObjectPropValueData(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
#endif
static uint32_t MtpGetObjectPropList(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);

static uint32_t PtpGetDevicePropDesc(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpGetDevicePropValue(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);
static uint32_t PtpSetDevicePropValue(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);

static uint32_t PtpResponse(uint32_t id, uint8_t* pBuf, uint16_t resp);


typedef  uint32_t (*PtpProc_t)(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen);

const struct PtpOpcodeTable_s
{
    uint16_t opcode;
    PtpProc_t proc;
    PtpProc_t data;
}
vPtpOpcodeTable[] =
{
    {0x1001, PtpDeviceInfo},
    {0x1002, PtpOpenSession},
    {0x1003, PtpCloseSession},
    {0x1004, PtpGetStorageIds},
    {0x1005, PtpGetStorageInfo},
    {0x1007, PtpGetObjectHandles},
    {0x1008, PtpGetObjectInfo},

    {0x1009, PtpGetObject},
#if (MTP_READONLY != 1)
    {0x100B, PtpDeleteObject},
    {0x100C, PtpSendObjectInfo, PtpSendObjectInfoData},
    {0x100D, PtpSendObject, PtpSendObjectData},
#endif
    {0x100F, PtpFormatStore},

    {0x1014, PtpGetDevicePropDesc},
    {0x1015, PtpGetDevicePropValue},
    {0x1016, PtpSetDevicePropValue},

    {0x9801, MtpGetObjectPropsSupported},
    {0x9802, MtpGetObjectPropDesc},
    {0x9803, MtpGetObjectPropValue},
#if (MTP_READONLY != 1)
    {0x9804, MtpSetObjectPropValue},
#endif
    {0x9805, MtpGetObjectPropList},

    {0, nullptr}
};

#define INT8        0x0001
#define UINT8       0x0002
#define INT16       0x0003
#define UINT16      0x0004
#define INT32       0x0005
#define UINT32      0x0006
#define INT64       0x0007
#define UINT64      0x0008
#define INT128      0x0009
#define UINT128     0x000A
#define AUINT8      0x4002
#define STR         0xFFFF

typedef uint8_t (*DeviceProc_t)(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle);
typedef uint8_t (*ObjectProc_t)(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* fileinfo);

#define FORMAT_UNDEFINED    0x3000
#define FORMAT_ASSOCIATION  0x3001
#define FORMAT_DEVICESCRIPT 0x3002
#define FORMAT_DEVICEBINARY 0x3003
#define FORMAT_TEXTFILE     0x3004
#define FORMAT_HTML         0x3005
#define FORMAT_AUDIO_WAV    0x3008
#define FORMAT_AUDIO_MP3    0x3009
#define FORMAT_UNKNOWNIMAGE 0x3800
#define FORMAT_IMAGE_JPG    0x3801
#define FORMAT_IMAGE_BMP    0x3804
#define FORMAT_IMAGE_GIF    0x3807
#define FORMAT_IMAGE_PNG    0x380B

const struct MtpObjectFormat_s
{
    uint16_t format;
    char extension[5];
}
vMtpObjectFormats[] =
{
    {FORMAT_DEVICEBINARY, "BIN"},
    {FORMAT_DEVICEBINARY, "FWX"},
    {FORMAT_TEXTFILE, "TXT"},
    {FORMAT_TEXTFILE, "DCF"},
    {FORMAT_TEXTFILE, "EDS"},
    {FORMAT_DEVICESCRIPT, "INI"},
    {FORMAT_TEXTFILE, "LOG"},
    {FORMAT_HTML, "HTM"},
    {FORMAT_HTML, "HTML"},
    {FORMAT_IMAGE_BMP, "BMP"},
    {FORMAT_IMAGE_PNG, "PNG"},
    {FORMAT_IMAGE_JPG, "JPG"},
    {FORMAT_IMAGE_GIF, "GIF"},
    {FORMAT_UNKNOWNIMAGE, "ICO"},
    {FORMAT_UNDEFINED, "\n\n\n\n"}, // Fake extension, so nothing will match
    {FORMAT_ASSOCIATION, "\n\n\n\n"}, // Fake extension, so nothing will match
    {0, ""}
};


static uint8_t
MtpObjProp_StorageId(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    uint32_t storage = 0;

    (void)info;

    if ((handle != 0) && (handle != UINT32_MAX))
    {
        storage = STORAGE_ID(INODE_STORAGE(handle));
    }
    MTP_DBG_LVL3("%s(%lX): %lX", __FUNCTION__, handle, storage);
    return(Uint32(buf, index, reqlen, storage));
}


static uint8_t
MtpObjProp_ObjectFormat(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    uint16_t format = FORMAT_UNDEFINED;
    uint32_t i;

    if ((info != nullptr) && (info->name[0] != '\0') && (handle != 0) && (handle != UINT32_MAX))
    {
        char* ext;

        if (info->attrib & ATR_DIR)
        {
            format = FORMAT_ASSOCIATION;
        }
        else if (ext = strrchr(info->name, '.'), ext != nullptr)
        {
            ext++;
            for (i = 0; vMtpObjectFormats[i].format != 0; i++)
            {
                if (strcasecmp(ext, vMtpObjectFormats[i].extension) == 0)
                {
                    format = vMtpObjectFormats[i].format;
                    break;
                }
            }
        }
    }
    MTP_DBG_LVL3("%s(%lX): %X", __FUNCTION__, handle, format);
    return(Uint16(buf, index, reqlen, format));
}


static uint8_t
MtpObjProp_ProtectionStatus(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    uint16_t protect = 0;

    if ((info != nullptr) && (info->name[0] != '\0') && (handle != 0) && (handle != UINT32_MAX))
    {
        protect = !(info->attrib & ATR_IWRITE) ? 1 : 0;
    }
    MTP_DBG_LVL3("%s(%lX): %u", __FUNCTION__, handle, protect);
    return(Uint16(buf, index, reqlen, protect));
}


static uint8_t
MtpObjProp_ObjectSize(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    uint64_t size = 0;

    if ((info != nullptr) && (info->name[0] != '\0') && (handle != 0) && (handle != UINT32_MAX))
    {
        size = info->size;
    }
    MTP_DBG_LVL3("%s(%lX): %llu", __FUNCTION__, handle, size);
    return(Uint64(buf, index, reqlen, size));
}


static uint8_t
MtpObjProp_AssociationType(uint8_t* *buf, uint32_t *index, uint32_t *reqlen, uint32_t handle, VfsInfo_t* info)
{
    uint16_t assoc = 0;

    if ((info != nullptr) && (info->name[0] != '\0') && (handle != 0) && (handle != UINT32_MAX))
    {
        assoc = info->attrib & ATR_DIR ? 1 : 0;
    }
    MTP_DBG_LVL3("%s(%lX): %lX", __FUNCTION__, handle, assoc);
    return(Uint16(buf, index, reqlen, assoc));
}


static uint8_t
MtpObjProp_ParentObject(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    uint32_t parent = 0;

    (void)info;

    // TODO retrieve parent object of directories
    if ((handle & INODE_FOLDER_MASK) == INODE_FOLDER_MASK)
    {
        parent = UINT32_MAX;
    }
    else
    {
        //parent = handle & (INODE_STORAGE_MASK | INODE_FOLDER_MASK);
        parent = vCurrentParent;
    }

    MTP_DBG_LVL3("%s(%lX): %lX", __FUNCTION__, handle, parent);
    return(Uint32(buf, index, reqlen, parent));
}


static uint8_t
MtpObjProp_PersistentUniqueObjectIdentifier(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    (void)info;

    if ((handle != 0) && (handle != UINT32_MAX))
    {
        Uint64(buf, index, reqlen, handle);
        Uint64(buf, index, reqlen, 0);
    }
    return(16);
}


static uint8_t
MtpObjProp_ObjectFileName(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    char* pfile = nullptr;

    if ((info != nullptr) && (info->name[0] != '\0') && (handle != 0) && (handle != UINT32_MAX))
    {
        pfile = info->name;
    }
    MTP_DBG_LVL3("%s(%lX): %s", __FUNCTION__, handle, pfile);
    return(String(buf, index, reqlen, pfile));
}


static uint8_t
MtpObjProp_ObjectTimeCreated(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    char str[16] = "20010101T000000";

    if ((info != nullptr) && (info->name[0] != '\0') && (handle != 0) && (handle != UINT32_MAX))
    {
        struct tm* t = gmtime(&info->created);

        sniprintf(str, sizeof(str), "%04d%02d%02dT%02d%02d%02d",  t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    }
    MTP_DBG_LVL3("%s(%lu): %s", __FUNCTION__, handle, str);
    return(String(buf, index, reqlen, str));
}


static uint8_t
MtpObjProp_ObjectTimeModified(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    char str[16] = "20010101T000000";

    if ((info != nullptr) && (info->name[0] != '\0') && (handle != 0) && (handle != UINT32_MAX))
    {
        struct tm* t = gmtime(&info->modified);

        sniprintf(str, sizeof(str), "%04d%02d%02dT%02d%02d%02d",  t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    }
    MTP_DBG_LVL3("%s(%lu): %s", __FUNCTION__, handle, str);
    return(String(buf, index, reqlen, str));
}


static uint8_t
MtpObjProp_UseCount(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t handle, VfsInfo_t* info)
{
    uint32_t count = 1;

    (void)info;

    return(Uint32(buf, index, reqlen, count));
}


const struct MtpObjectPropsSupported_s
{
    uint16_t prop;
    uint16_t type;
    ObjectProc_t proc;
}
vMtpObjectPropsSupported[] =
{
    {0xDC01, UINT32, MtpObjProp_StorageId}, // StorageID
    {0xDC02, UINT16, MtpObjProp_ObjectFormat}, // Object Format
    {0xDC03, UINT16, MtpObjProp_ProtectionStatus}, // ProtectionStatus
    {0xDC04, UINT64, MtpObjProp_ObjectSize}, // ObjectSize
    {0xDC05, UINT16, MtpObjProp_AssociationType}, // Association Type
    {0xDC07, STR, MtpObjProp_ObjectFileName}, // ObjectFileName
    {0xDC08, STR, MtpObjProp_ObjectTimeCreated}, // Date Created
    {0xDC09, STR, MtpObjProp_ObjectTimeModified}, // Date Modified
    {0xDC0B, UINT32, MtpObjProp_ParentObject}, // Parent Object
    {0xDC41, UINT128, MtpObjProp_PersistentUniqueObjectIdentifier}, // Without this property, Windows is unable to delete files!
    {0xDC44, STR, MtpObjProp_ObjectFileName}, // Name
    //{0xDC91, UINT32, MtpObjProp_UseCount},
    {0, 0, nullptr}
};


#define PROP_VALUE      0
#define PROP_DEFAULT    1
#define PROP_MIN        2
#define PROP_MAX        3
#define PROP_STEP       4

static uint8_t
MtpDeviceProp_BatteryLevel(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t value)
{
    uint16_t level = 0;

    return(Uint16(buf, index, reqlen, level));
}


static uint8_t
MtpDeviceProp_FriendlyName(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t value)
{
    return(String(buf, index, reqlen, MTP_FRIENDLYNAME));
}


static uint8_t
MtpDeviceProp_DeviceIcon(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t value)
{
    uint32_t len = 0;

    if (vfs_file_open(&vSendObjectHandle, "DevIcon.fil", VFS_RDONLY) == 0)
    {
        len = vfs_file_size(&vSendObjectHandle);
        if (index == 0)
        {
            len += Uint32(buf, index, reqlen, len);
        }
        vfs_file_seek(&vSendObjectHandle, *index, SEEK_SET);

        if ((buf != nullptr) && (*buf != nullptr))
        {
            size_t rb;

            if (rb = vfs_file_read(&vSendObjectHandle, *buf, *reqlen), rb >= 0)
            {
                *index += rb;
                *reqlen -= rb;
            }
        }
        vfs_file_close(&vSendObjectHandle);
    }
    return(len);
}


static uint8_t
MtpDeviceProp_PerceivedDeviceType(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t value)
{
    return(Uint32(buf, index, reqlen, 0x00000004));
}


static uint8_t
MtpDeviceProp_PlaybackObject(uint8_t** buf, uint32_t* index, uint32_t* reqlen, uint32_t value)
{
    return(Uint32(buf, index, reqlen, 0x00000000));
}


#define FORM_NONE   0
#define FORM_RANGE  1
#define FORM_ENUM   2   // Not supported

const struct MtpDevicePropsSupported_s
{
    uint16_t prop;
    uint16_t type;
    DeviceProc_t proc;
    uint8_t form;
    uint32_t min, max;
}
vMtpDevicePropsSupported[] =
{
    {0x5001, UINT16, MtpDeviceProp_BatteryLevel, FORM_RANGE, 0, 0}, // Battery Level
    //{0x5011, STR, nullptr, FORM_NONE}, // Date Time
    {0xD402, STR, MtpDeviceProp_FriendlyName, FORM_NONE}, // Device Friendly Name
    //{0xD405, AUINT8, MtpDeviceProp_DeviceIcon, FORM_NONE}, // DeviceIcon
    //{0xD407, UINT32, MtpDeviceProp_PerceivedDeviceType, FORM_NONE},
    //{0xD411, UINT32, MtpDeviceProp_PlaybackObject, FORM_NONE},
    {0,0,nullptr}
};


#ifdef MTP_EVENTS
const struct MtpDeviceEventsSupported_s
{
    MtpEvent_t prop;
    DeviceProc_t proc;
}
vMtpDeviceEventsSupported[] =
{
    //{0x4000, nullptr}, // Undefined
    {0x4001, nullptr}, // Cancel Transaction
    {0x4002, nullptr}, // Object Added
    {0x4003, nullptr}, // Object Removed
    {0x4004, nullptr}, // Store Added
    {0x4005, nullptr}, // Store Removed
    {0x4006, nullptr}, // Device Prop Changed
    {0x4007, nullptr}, // Object Info Changed
    {0x4008, nullptr}, // Device Info Changed
    //{0x4009, nullptr}, // Request Object Transfer
    {0x400A, nullptr}, // Store Full
    //{0x400B, nullptr}, // Device Reset
    //{0x400C, nullptr}, // Store Info Changed
    //{0x400D, nullptr}, // Capture Complete
    //{0x400E, nullptr}, // Unreported Status
    //{0xC801, nullptr}, // Object Prop Changed
    //{0xC802, nullptr}, // Object Prop Desc Changed
    {0xC803, nullptr}, // Object References Changed
    {0, nullptr}
};
#endif


static uint8_t vPtpBuffer[PTP_BUF_SIZE];
static uint32_t vPtpSession = 0;

static uint16_t vResponseCode = 0;
static uint8_t vResponseParamCount = 0;
static uint32_t vParam[5];
static uint32_t vResponseParam[5] = {0};


static void
ParamParse(uint8_t* buf, int num)
{
    int i;

    for (i = 0; i < num; i++)
    {
        vParam[i] = GetUint32(&buf[12 + i * 4]);
    }
}


static uint32_t
PtpDeviceInfo(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

    uint32_t i;

    if (reqlen == 0)
    {
        len = 0;
        MTP_DBG_LVL1("%s[%u]", __FUNCTION__, __LINE__);
    }

    len += Uint32(&buf, &index, &reqlen, len);    // Start with zero length
    len += Uint16(&buf, &index, &reqlen, 2);    // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x1001);    // Code
    len += Uint32(&buf, &index, &reqlen, id);    // TransactionID

    len += Uint16(&buf, &index, &reqlen, PTPVERSION);

    len += Uint32(&buf, &index, &reqlen, 0x00000006);	// VendorExtensionID = MTP
    len += Uint16(&buf, &index, &reqlen, MTPVERSION);	// VendorExtensionVersion

    len += String(&buf, &index, &reqlen, nullptr);	// VendorExtensionDesc

    len += Uint16(&buf, &index, &reqlen, FUNCTIONALMODE);

    len += Uint32(&buf, &index, &reqlen, (sizeof(vPtpOpcodeTable) / sizeof(vPtpOpcodeTable[0])) - 1);	// OperationsSupported
    for (i = 0; vPtpOpcodeTable[i].opcode != 0; i++)
    {
        len += Uint16(&buf, &index, &reqlen, vPtpOpcodeTable[i].opcode);
    }

#ifdef MTP_EVENTS
    len += Uint32(&buf, &index, &reqlen, (sizeof(vMtpDeviceEventsSupported) / sizeof(vMtpDeviceEventsSupported[0])) - 1);	// EventsSupported
    for (i = 0; vMtpDeviceEventsSupported[i].prop != 0; i++)
    {
        len += Uint16(&buf, &index, &reqlen, vMtpDeviceEventsSupported[i].prop);
    }
#else
    len += Uint32(&buf, &index, &reqlen, 0);	// EventsSupported
#endif

    len += Uint32(&buf, &index, &reqlen, (sizeof(vMtpDevicePropsSupported) / sizeof(vMtpDevicePropsSupported[0])) - 1);	// DevicePropertiesSupported
    for (i = 0; vMtpDevicePropsSupported[i].prop != 0; i++)
    {
        len += Uint16(&buf, &index, &reqlen, vMtpDevicePropsSupported[i].prop);
    }

    len += Uint32(&buf, &index, &reqlen, 0);	// CaptureFormats

    len += Uint32(&buf, &index, &reqlen, (sizeof(vMtpObjectFormats) / sizeof(vMtpObjectFormats[0])) - 1);	// ImageFormats
    for (i = 0; vMtpObjectFormats[i].format != 0; i++)
    {
        len += Uint16(&buf, &index, &reqlen, vMtpObjectFormats[i].format);
    }

    len += String(&buf, &index, &reqlen, MTP_MANUFACTURER);	// Manufacturer
    len += String(&buf, &index, &reqlen, MTP_FRIENDLYNAME);	// Model

    char versionbuf[16];
#ifdef _VERSION_H
    sniprintf(versionbuf, sizeof(versionbuf), "%u.%u.%u.%u", VER_H, VER_MH, VER_ML, VER_L);
#else
    extern uint8_t USBD_DeviceDesc[];
    sniprintf(versionbuf, sizeof(versionbuf), "%u.%u", USBD_DeviceDesc[13], USBD_DeviceDesc[12]);
#endif
    len += String(&buf, &index, &reqlen, versionbuf);	// DeviceVersion
    len += String(&buf, &index, &reqlen, MTP_SERIAL);	// SerialNumber

    vResponseCode = OK;
    return(len);
}


static uint32_t
PtpOpenSession(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    if (reqlen == 0)
    {
        ParamParse(buf, 1);
        MTP_DBG_LVL1("%s[%u] %lX", __FUNCTION__, __LINE__, vParam[0]);

        if (vPtpSession == vParam[0])
        {
            return(PtpResponse(id, vPtpBuffer, PtpErr_SessionAlreadyOpen));
        }
        else if (vPtpSession != 0)
        {
            return(PtpResponse(id, vPtpBuffer, PtpErr_DeviceBusy));
        }
        else
        {
		#if VFS_NODIRS != 1
            char tmp[13];
            int i;

            for (i = 0; vfs_volume(i) != nullptr; i++)
            {
            	VfsInfo_t info;

            	if (vfs_stat(vfs_volume(i), &info) == 0)
            	{
            		if (!(info.attrib & ATR_FLAT_FILESYSTEM))
					{
						siprintf(tmp, "%s%s", vfs_volume(i), MTP_FOLDER_CACHE_FILE);
						if (vfs_file_open(&vSendObjectHandle, tmp, VFS_RDWR | VFS_TRUNC) == 0)
						{
							vfs_file_close(&vSendObjectHandle);
						}
					}
            	}
            }
		#endif

            vPtpSession = vParam[0];
            MTP_SESSION_OPEN_HOOK(vParam[0]);
        }
    }
    return(PtpResponse(id, vPtpBuffer, OK));
}


static uint32_t
PtpCloseSession(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    if (reqlen == 0)
    {
        MTP_DBG_LVL1("%s[%u]", __FUNCTION__, __LINE__);
        vPtpSession = 0;

	#if VFS_NODIRS != 1
        if (vFolderCacheDirty != 0)
        {
            int i;
            char path[13];

            // Delete cache files
            for (i = 0; i < (1 << INODE_STORAGE_BITS); i++)
            {
                if (vFolderCacheDirty & (1 << i))
                {
                    siprintf(path, "%s%s", vfs_volume(i), MTP_FOLDER_CACHE_FILE);
                    vfs_remove(path);
                }
            }
        }
	#endif

        // Free a bunch of allocated memory
        GetFileById(nullptr, 0, false, nullptr);
        MTP_SESSION_CLOSE_HOOK();
    }
    return(PtpResponse(id, vPtpBuffer, OK));
}


static uint32_t
PtpGetStorageIds(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

    uint32_t i;
    uint32_t vCnt = 0;

    if (reqlen == 0)
    {
        len = 0;
        MTP_DBG_LVL1("%s[%u]", __FUNCTION__, __LINE__);
    }

    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x1004);  // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

    for (i = 0; vfs_volume(i) != nullptr; i++)
    {
        if (vfs_fs_size(vfs_volume(i)) >= 0)
        {
            vCnt++;
        }
    }

    len += Uint32(&buf, &index, &reqlen, vCnt);  // Number of StorageID elements in StorageID
    for (i = 0; vfs_volume(i) != nullptr; i++)
    {
        if (vfs_fs_size(vfs_volume(i)) >= 0)
        {
            len += Uint32(&buf, &index, &reqlen, STORAGE_ID(i));  // StorageID on device
        }
    }
    return(len);
}


static uint32_t
PtpGetStorageInfo(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

    char* drive;
    VfsInfo_t info;

    if (reqlen == 0)
    {
        ParamParse(buf, 1);    // StorageID
        len = 0;
        MTP_DBG_LVL1("%s[%u] %lX", __FUNCTION__, __LINE__, vParam[0]);
    }

    if (drive = vfs_volume(DRIVE_NUM(vParam[0])), drive == nullptr)
    {
        return(PtpResponse(id, nullptr, PtpErr_InvalidStorageId));
    }
    if (vfs_fs_size(drive) < 0)
    {
        return(PtpResponse(id, nullptr, PtpErr_StoreNotAvailable));
    }
    vfs_stat(drive, &info);
    uint64_t sz = info.blocks * (uint64_t)info.blocksize;

    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x1005);  // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

	len += Uint16(&buf, &index, &reqlen, info.attrib & ATR_REMOVABLE_DISK ? 4 : 3);  // Storage type code = Removable or Fixed RAM
    len += Uint16(&buf, &index, &reqlen, info.attrib & ATR_FLAT_FILESYSTEM ? 1 : 2);  // FileSystemType = Generic flat or Generic hierarchical
    len += Uint16(&buf, &index, &reqlen, 0);  // AccessCapability = Read/write
    len += Uint64(&buf, &index, &reqlen, sz);  // Indicates total storage capacity of the store in bytes
    len += Uint64(&buf, &index, &reqlen, sz - info.size);  // FreeSpaceInBytes
    len += Uint32(&buf, &index, &reqlen, info.blocks);  // FreeSpaceInObjects

    len += String(&buf, &index, &reqlen, info.name);	// StorageDescription
    len += String(&buf, &index, &reqlen, drive);	// Volume Identifier

    vResponseCode = OK;
    return(len);
}


static uint32_t
PtpGetObjectHandles(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

    int err;
    VfsDir_t scanhandle;
    VfsInfo_t info = {0};
#if 0
    char pathbuf[MAX_PATH];
#endif
    char* path;
    uint32_t vFileCount = 0;
    char tmp[MAX_PATH];
    char* p;

    if (reqlen == 0)
    {
        ParamParse(buf, 3);    // StorageID, [ObjectFormatCode], [Association]
        len = 0;
        MTP_DBG_LVL0("%s[%u] %lX,%lX,%lX", __FUNCTION__, __LINE__, vParam[0], vParam[1], vParam[2]);
    }

    if (vParam[1] != 0)
    {
        return(PtpResponse(id, nullptr, PtpErr_SpecificationByFormatUnsupported));
    }
    else if ((vParam[2] != 0) && (vParam[2] != UINT32_MAX))
    {
        // Get the folder name whose listing is requested
        if (!GetFileById(nullptr, vParam[2], true, &path))
		//XXX if (!GetFileById(nullptr, vParam[2], false, &path))
        {
            return(PtpResponse(id, nullptr, PtpErr_InvalidParentObject));
        }
        MTP_DBG_LVL1("%lX %s", vParam[2], path);
    }
    else if (!GetFileById(nullptr, DRIVE_NUM(vParam[0]) << (32 - INODE_STORAGE_BITS) | INODE_FOLDER_MASK, true, &path))
    {
        return(PtpResponse(id, nullptr, PtpErr_InvalidStorageId));
    }

    char* path2 = strchr(path, ':') + 1;

    // Check if index is already past the header fields, then skip the counting step
    if (index >= 16)
    {
        len += 16;
        index -= 16;
    }
    else
    {
	#if VFS_NODIRS != 1
        siprintf(tmp, "%s%s", vfs_volume(DRIVE_NUM(vParam[0])), MTP_FOLDER_CACHE_FILE);
        if (err = vfs_file_open(&vSendObjectHandle, tmp, VFS_RDWR | VFS_CREAT), err != 0)
        {
            MTP_DBG_LVL0("%s[%u] %s (%s)...", __FUNCTION__, __LINE__, strerror(-err), tmp);
        }
        if (vfs_stat(tmp, &info) == 0)
        {
            if (!(info.attrib & ATR_HID))
            {
                info.attrib |= ATR_HID;
                vfs_touch(tmp, &info);
            }
        }
	#endif

        if (err = vfs_dir_open(&scanhandle, (char*)path), err == 0)
        {
            while (vfs_dir_read(&scanhandle, &info) == 0)
            {
                if (info.name[0] == '.')
                {
                    // Skip self and parent directory entries
                    if ((info.name[1] == '\0') || ((info.name[1] == '.') && (info.name[2] == '\0')))
                    {
                        continue;
                    }
                }
                if (info.attrib & ATR_HID)
                {
                    continue;
                }
                // Check if folder entries in the cache file, otherwise add it
                if (info.attrib & ATR_DIR)
                {
				#if VFS_NODIRS != 1
                    while (p = vfs_gets(tmp, sizeof(tmp), &vSendObjectHandle), p != nullptr)
                    {
                        if (strncmp(p, path2, strlen(path2)) == 0)
                        {
                            p += strlen(path2);
                            if (*p == '/')
                            {
                                p++;
                            }
                            if (strncmp(p, info.name, strlen(info.name)) == 0)
                            {
                                p += strlen(info.name);
                                if (strncmp(p, "\n", 2) == 0)
                                {
                                    break;
                                }
                            }
                        }
                    }
                    if (p == nullptr)
                    {
                        vfs_puts(path2, &vSendObjectHandle);
                        if (path[strlen(path) - 1] != '/')
                        {
                            vfs_puts("/", &vSendObjectHandle);
                        }
                        vfs_puts(info.name, &vSendObjectHandle);
                        vfs_puts("\n", &vSendObjectHandle);
                    }
				#endif
                }
                vFileCount++;
            }
            vfs_dir_close(&scanhandle);
		#if VFS_NODIRS != 1
            vfs_file_close(&vSendObjectHandle);
		#endif

            len += Uint32(&buf, &index, &reqlen, len);     // Length
            len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
            len += Uint16(&buf, &index, &reqlen, 0x1007);  // Code
            len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

            len += Uint32(&buf, &index, &reqlen, vFileCount);  // Number of elements
        }
        else if ((err == ENOTDIR) || (err == ENODEV))
        {
            return(PtpResponse(id, nullptr, PtpErr_StoreNotAvailable));
        }
        else
        {
            return(PtpResponse(id, nullptr, PtpErr_GeneralError));
        }
    }

    if (vfs_dir_open(&scanhandle, (char*)path) == 0)
    {
        uint32_t x = 0;

	#if VFS_NODIRS != 1
        siprintf(tmp, "%s%s", vfs_volume(DRIVE_NUM(vParam[0])), MTP_FOLDER_CACHE_FILE);
        vfs_file_open(&vSendObjectHandle, tmp, VFS_RDONLY);
	#endif

        while (vfs_dir_read(&scanhandle, &info) == 0)
        {
            if (info.name[0] == '.')
            {
                // Skip self and parent directory entries
                if ((info.name[1] == '\0') || ((info.name[1] == '.') && (info.name[2] == '\0')))
                {
                    continue;
                }
            }
            if (info.attrib & ATR_HID)
            {
                continue;
            }

//XXX            if ((index < 4) && (info.attrib & ATR_DIR))
            if (info.attrib & ATR_DIR)
            {
			#if VFS_NODIRS != 1
                // Fetch corresponding entry from cache file
                while (p = vfs_gets(tmp, sizeof(tmp), &vSendObjectHandle), p != nullptr)
                {
                    x++;
                    if (strncmp(p, path2, strlen(path2)) == 0)
                    {
                        p += strlen(path2);
                        if (*p == '/')
                        {
                            p++;
                        }
                        if (strncmp(p, info.name, strlen(info.name)) == 0)
                        {
                            p += strlen(info.name);
                            if (strncmp(p, "\n", 2) == 0)
                            {
                                break;
                            }
                        }
                    }
                }
			#endif
                info.inode = (x << INODE_ITEM_BITS) | (DRIVE_NUM(vParam[0]) << (32 - INODE_STORAGE_BITS));
            }
            else
            {
                // Determine the hash-based handle
                info.inode = HandleFilenameBits(info.name) | vCurrentParent;
            }

            if (reqlen == 0)
            {
				MTP_DBG_LVL0("List: %lX - %s", info.inode, info.name);
            }
            len += Uint32(&buf, &index, &reqlen, info.inode);  // Object Handle
        }
	#if VFS_NODIRS != 1
        vfs_file_close(&vSendObjectHandle);
	#endif
        vfs_dir_close(&scanhandle);
    }
    return(len);
}


static uint32_t
PtpGetObjectInfo(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

    VfsInfo_t* info = nullptr;

    if (reqlen == 0)
    {
        ParamParse(buf, 1);   // ObjectHandle
        len = 0;
        MTP_DBG_LVL1("%s[%u] %lX", __FUNCTION__, __LINE__, vParam[0]);
    }

    if (!GetFileById(&info, vParam[0], false, nullptr))
    {
        return(PtpResponse(id, nullptr, PtpErr_AccessDenied));
    }

    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x1008);  // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

    len += MtpObjProp_StorageId(&buf, &index, &reqlen, vParam[0], info);  // Storage ID
    len += MtpObjProp_ObjectFormat(&buf, &index, &reqlen, vParam[0], info);  // Object Format
    len += MtpObjProp_ProtectionStatus(&buf, &index, &reqlen, vParam[0], info);  // ProtectionStatus (0=RW, 1=RO)
    MtpObjProp_ObjectSize(&buf, &index, &reqlen, vParam[0], info);  // Object Compressed Size
    len += 4; //  ObjectSize gives 64 bit number, but we only need 32
    len += Uint16(&buf, &index, &reqlen, 0);  // * Thumb Format
    len += Uint32(&buf, &index, &reqlen, 0);  // * Thumb Compressed Size
    len += Uint32(&buf, &index, &reqlen, 0);  // * Thumb Pix Width
    len += Uint32(&buf, &index, &reqlen, 0);  // * Thumb Pix Height
    len += Uint32(&buf, &index, &reqlen, 0);  // Image Pix Width
    len += Uint32(&buf, &index, &reqlen, 0);  // Image Pix Height
    len += Uint32(&buf, &index, &reqlen, 0);  // Image Pix Depth
    len += MtpObjProp_ParentObject(&buf, &index, &reqlen, vParam[0], info);  // Parent Object
    len += Uint16(&buf, &index, &reqlen, info->attrib & ATR_DIR ? 1 : 0);  // Association Code
    len += Uint32(&buf, &index, &reqlen, 0);  // Association Desc
    len += Uint32(&buf, &index, &reqlen, 0);  // * Sequence Number
    len += MtpObjProp_ObjectFileName(&buf, &index, &reqlen, vParam[0], info);	// FileName
    len += MtpObjProp_ObjectTimeCreated(&buf, &index, &reqlen, vParam[0], info);	// Date Created
    len += MtpObjProp_ObjectTimeModified(&buf, &index, &reqlen, vParam[0], info);	// Date Modified
    len += String(&buf, &index, &reqlen, nullptr);	// Keywords

    return(len);
}


static uint32_t
PtpGetObject(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

    char* path;
    VfsInfo_t* info;

    if (reqlen == 0)
    {
        ParamParse(buf, 1);    // ObjectHandle
        len = 0;
        MTP_DBG_LVL1("%s[%u] %lX", __FUNCTION__, __LINE__, vParam[0]);

        if (GetFileById(&info, vParam[0], false, &path))
        {
            len += info->size;

            if (vfs_file_open(&vSendObjectHandle, path, VFS_RDONLY) != 0)
            {
                return(PtpResponse(id, nullptr, PtpErr_AccessDenied));
            }
            MTP_DBG_LVL0("%s[%u] %s", __FUNCTION__, __LINE__, path);
        }
        else
        {
            return(PtpResponse(id, nullptr, PtpErr_GeneralError));
        }
    }
    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x1009);  // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

    if (reqlen > 0)
    {
        if (vSendObjectHandle.filesys != nullptr)
        {
            uint32_t vBytesRead = 0;

            if (vBytesRead = vfs_file_read(&vSendObjectHandle, buf, reqlen), vBytesRead >= 0)
            {
                buf += vBytesRead;
                reqlen -= vBytesRead;
            }
            if (vfs_file_eof(&vSendObjectHandle))
            {
                vfs_file_close(&vSendObjectHandle);
            }
        }
    }
    return(len);
}


#if (MTP_READONLY != 1)
static uint32_t
PtpDeleteObject(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    char* path;
    int err;
    VfsInfo_t* info;

    if (reqlen == 0)
    {
        ParamParse(buf, 1);    // ObjectHandle
        MTP_DBG_LVL1("%s[%u] %lX", __FUNCTION__, __LINE__, vParam[0]);

        if (GetFileById(&info, vParam[0], false, &path))
        {
            // Delete directory contents
            if (info->attrib & ATR_DIR)
            {
                VfsDir_t dir;
                int x;

                for (x = 0; path[x] != '\0'; x++) // cheap tcslen()
                {}

                if (vfs_dir_open(&dir, path) == 0)
                {
                    while (vfs_dir_read(&dir, info) == 0)
                    {
                        if (info->name[0] == '.')
                        {
                            // Skip self and parent directory entries
                            if ((info->name[1] == '\0') || ((info->name[1] == '.') && (info->name[2] == '\0')))
                            {
                                continue;
                            }
                        }
                        strcat(path, "/");
                        strcat(path, info->name);
                        vfs_remove(path);
                        path[x] = '\0';
                    }
                    vfs_dir_close(&dir);
                }
                // Signal that the folder cache should be erased/rebuild when session closes
                vFolderCacheDirty |= 1 << INODE_STORAGE(vParam[0]);
            }
            err = -vfs_remove(path);
            MTP_DBG_LVL0("%s[%u] %s %s", __FUNCTION__, __LINE__, strerror(err), path);
            switch (err)
            {
                case 0:
                    return(PtpResponse(id, nullptr, OK));

                case EINVAL:
                    return(PtpResponse(id, nullptr, PtpErr_InvalidObjectHandle));

                case EROFS:
                    return(PtpResponse(id, nullptr, PtpErr_ObjectWriteProtected));

                case ENOSPC:
                case ENOTDIR:
                    return(PtpResponse(id, nullptr, PtpErr_AccessDenied));

                default:
                    return(PtpResponse(id, nullptr, PtpErr_GeneralError));
            }
        }
    }
    return(PtpResponse(id, nullptr, PtpErr_GeneralError));
}
#endif


#if (MTP_READONLY != 1)
static uint32_t vSendObjectParent;
static uint32_t vSendObjectId;
#endif

// See MTPforUSB-IFv1.1 page 50
#define OBJECTINFO_DATAOFFSET       12
#define OBJECTINFO_FORMATOFFSET     (OBJECTINFO_DATAOFFSET + 4)
#define OBJECTINFO_FILESIZEOFFSET   (OBJECTINFO_DATAOFFSET + 8)
#define OBJECTINFO_FILENAMEOFFSET   (OBJECTINFO_DATAOFFSET + 52)


#if (MTP_READONLY != 1)
static uint32_t
PtpSendObjectInfo(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    uint32_t len = 0;

    VfsInfo_t info;
    char* path;
    int err;

    if (reqlen == 0)
    {
        ParamParse(buf, 2);   // Storage ID, Parent Handle
        MTP_DBG_LVL1("%s[%u] %lX,%lX", __FUNCTION__, __LINE__, vParam[0], vParam[1]);

        vSendObjectParent = vParam[1];
        if ((vSendObjectParent == 0) || (vSendObjectParent == UINT32_MAX))
        {
            if (vfs_volume(DRIVE_NUM(vParam[0])) == nullptr)
            {
                return(PtpResponse(id, nullptr, PtpErr_InvalidStorageId));
            }
            vSendObjectParent = (DRIVE_NUM(vParam[0]) << (32 - INODE_STORAGE_BITS)) | INODE_FOLDER_MASK;
        }
		//XXX if (!GetFileById(nullptr, vSendObjectParent, true, &path))
		if (!GetFileById(nullptr, vSendObjectParent, false, &path))
        {
            return(PtpResponse(id, nullptr, PtpErr_InvalidParentObject));
        }

        if (err = -vfs_stat(path, &info), err == 0)
        {
            PtpSendObjectInfoData(0, nullptr, 0, 0); // init
            len = 0;
        }
        else if (err == ENOTDIR)
        {
            return(PtpResponse(id, nullptr, PtpErr_StoreNotAvailable));
        }
    }
    return(len);
}
#endif


#if (MTP_READONLY != 1)
static time_t
PtpParseDateString(uint8_t* buf)
{
    struct tm vTm = {0};
    char dup[5];

    // First convert fields to struct tm
    memset(dup, 0, sizeof(dup));
    memcpy(dup, buf + 0, 4);
    vTm.tm_year = atol(dup) - 1900;

    memset(dup, 0, sizeof(dup));
    memcpy(dup, buf + 4, 2);
    vTm.tm_mon = atol(dup) - 1;

    memcpy(dup, buf + 6, 2);
    vTm.tm_mday = atol(dup);

    memcpy(dup, buf + 9, 2);
    vTm.tm_hour = atol(dup);

    memcpy(dup, buf + 11, 2);
    vTm.tm_min = atol(dup);

    memcpy(dup, buf + 13, 2);
    vTm.tm_sec = atol(dup);

    return(mktime(&vTm));
}
#endif


#if (MTP_READONLY != 1)
static uint32_t
PtpSendObjectInfoData(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t vExpectLen = 0;
    static uint32_t vReceivedLen = 0;
    static uint32_t vFileSize = 0;
    static uint16_t vFormat = 0;
    static uint16_t vNameLen = 0, vCreatedLen = 0, vModifiedLen = 0, vVarIdx = 0;
    static uint8_t vTimeStr[24];
    static time_t  vCreated;
    static time_t  vModified;

    VfsInfo_t info;
    uint32_t i;
    char* p;

    if (buf == nullptr)
    {
        vExpectLen = 0;
        return(0);
    }

    if (vExpectLen == 0)
    {
        vExpectLen = GetUint32(&buf[0]);
        vReceivedLen = 0;
        MTP_DBG_LVL1("%s[%u] %lu", __FUNCTION__, __LINE__, vExpectLen);
        memset(vPtpBuffer, 0, sizeof(vPtpBuffer));  // Needed because of or'ing of filename later on

        if (GetFileById(nullptr, vSendObjectParent, false, &p))
        {
            strcat((char*)vPtpBuffer, p);
            if (p = strrchr((char*)vPtpBuffer, '/'), (p != nullptr) && (p[1] != '\0'))
            {
                strcat((char*)vPtpBuffer, "/");
            }
        }
        vCreated = 0;
        vModified = 0;
    }

    for (i = 0; i < reqlen; i++)
    {
        switch (vReceivedLen + i)
        {
            case OBJECTINFO_FORMATOFFSET + 0: vFormat = buf[i]; break;
            case OBJECTINFO_FORMATOFFSET + 1: vFormat |= buf[i] << 8; break;

            case OBJECTINFO_FILESIZEOFFSET + 0: vFileSize = buf[i]; break;
            case OBJECTINFO_FILESIZEOFFSET + 1: vFileSize |= buf[i] << 8; break;
            case OBJECTINFO_FILESIZEOFFSET + 2: vFileSize |= buf[i] << 16; break;
            case OBJECTINFO_FILESIZEOFFSET + 3: vFileSize |= buf[i] << 24; break;

            case OBJECTINFO_FILENAMEOFFSET: vNameLen = buf[i] << 1; vVarIdx = 0; break;
        }
        if ((vReceivedLen + i) > OBJECTINFO_FILENAMEOFFSET)
        {
            if ((vReceivedLen + i) <= (OBJECTINFO_FILENAMEOFFSET + vNameLen))
            {
                if (p = strrchr((char*)vPtpBuffer, '/'), p != nullptr)
                {
                    p[1 + (vVarIdx++ >> 1)] |= buf[i];
                }
                else
                {
                    return(PtpResponse(id, nullptr, PtpErr_GeneralError));
                }
            }
            else if ((vReceivedLen + i) == (OBJECTINFO_FILENAMEOFFSET + vNameLen + 1))
            {
                vCreatedLen = buf[i] << 1;
                vVarIdx = 0;
                memset(vTimeStr, 0, sizeof(vTimeStr));
            }
            else if ((vReceivedLen + i) <= (OBJECTINFO_FILENAMEOFFSET + vNameLen + vCreatedLen + 1))
            {
                vTimeStr[vVarIdx++ >> 1] |= buf[i];
                if (vVarIdx == vCreatedLen)
                {
                    vCreated = PtpParseDateString(vTimeStr);
                }
            }
            else if ((vReceivedLen + i) == (OBJECTINFO_FILENAMEOFFSET + vNameLen + vCreatedLen + 2))
            {
                vModifiedLen = buf[i] << 1;
                vVarIdx = 0;
                memset(vTimeStr, 0, sizeof(vTimeStr));
            }
            else if ((vReceivedLen + i) <= (OBJECTINFO_FILENAMEOFFSET + vNameLen + vCreatedLen + vModifiedLen + 2))
            {
                vTimeStr[vVarIdx++ >> 1] |= buf[i];
                if (vVarIdx == vModifiedLen)
                {
                    vModified = PtpParseDateString(vTimeStr);
                }
            }
        }
    }

    vReceivedLen += reqlen;
    if (vReceivedLen >= vExpectLen)
    {
        uint32_t fr = 0;

        vResponseCode = 0;
        MTP_DBG_LVL2("%s[%u] %s %luB", __FUNCTION__, __LINE__, (char*)vPtpBuffer, vFileSize);

        if (vfs_fs_size((char*)vPtpBuffer) < 0)
        {
            vResponseCode = PtpErr_StoreNotAvailable;
        }
        else if (vfs_stat((char*)vPtpBuffer, &info) == 0)
        {
            if (!(info.attrib & ATR_IWRITE))
            {
                vResponseCode = PtpErr_ObjectWriteProtected;
            }
            //XXX else if (info.attrib & (ATR_HID | ATR_SYS | ATR_DIR))
			else if (info.attrib & (ATR_HID | ATR_SYS))
            {
                vResponseCode = PtpErr_AccessDenied;
            }
            else if (info.size > vFileSize)
            {
                if (vfs_file_open(&vSendObjectHandle, (char*)vPtpBuffer, VFS_WRONLY | VFS_TRUNC) == 0)
                {
                    if (vfs_file_seek(&vSendObjectHandle, vFileSize, SEEK_SET) != 0)
                    {
                        vResponseCode = PtpErr_ObjectTooLarge;
                    }

                    vfs_file_close(&vSendObjectHandle);
                }
                else
                {
                    vResponseCode = PtpErr_GeneralError;
                }
            }
            else if (vFileSize >= vfs_fs_free((char*)vPtpBuffer) - info.size)
            {
                vResponseCode = PtpErr_ObjectTooLarge;
            }
        }
        else if (vFormat == FORMAT_ASSOCIATION)
        {
            int err;

		#if VFS_NODIRS != 1
            vResponseCode = PtpErr_GeneralError;
            if (err = vfs_mkdir((char*)vPtpBuffer), err == 0)
            {
                char tmp[13];

                // Add folder entry to cache file
                siprintf(tmp, "%s%s", vfs_volume(DRIVE_NUM(vParam[0])), MTP_FOLDER_CACHE_FILE);
                if (err = vfs_file_open(&vSendObjectHandle, tmp, VFS_RDWR | VFS_CREAT), err != 0)
                {
                    MTP_DBG_LVL0("%s[%u] %s (%s)...", __FUNCTION__, __LINE__, strerror(-err), tmp);
                }
                else
                {
                    i = 0;
                	// Search for either match of previous folder with same name, or determine the next id
                    while (p = vfs_gets(info.name, sizeof(info.name), &vSendObjectHandle), p != nullptr)
                    {
                        p = strchr((char*)vPtpBuffer, ':') + 1;
                        i++;
                        if (strncmp(info.name, p, strlen(p)) == 0)
                        {
                            if (strncmp(info.name + strlen(p), "\n", 2) == 0)
                            {
                                break;
                            }
                        }
                    }
                    if (p == nullptr)
                    {
                        i++;
                        p = strchr((char*)vPtpBuffer, ':') + 1;
                        vfs_puts(p, &vSendObjectHandle);
                        vfs_puts("\n", &vSendObjectHandle);
                    }
                    vSendObjectId = (i << INODE_ITEM_BITS) | (vCurrentParent & INODE_STORAGE_MASK);
                    vfs_file_close(&vSendObjectHandle);
                    vResponseCode = OK;
                }
            }
            else
            {
                MTP_DBG_LVL0("%s[%u] %s (%s)...", __FUNCTION__, __LINE__, strerror(-err), (char*)vPtpBuffer);
            }
		#else
            vResponseCode = PtpErr_AccessDenied;
		#endif
        }
        else if (vFileSize >= vfs_fs_free((char*)vPtpBuffer))
        {
            vResponseCode = PtpErr_ObjectTooLarge;
        }

        if (vResponseCode == 0) // Not yet assigned
        {
            // Create File
            vResponseCode = PtpErr_GeneralError;
            if (vfs_file_open(&vSendObjectHandle, (char*)vPtpBuffer, VFS_RDWR | VFS_TRUNC) == 0)
            {
                vfs_file_sync(&vSendObjectHandle);
                // Generate handle
                vSendObjectId = HandleFilenameBits(strrchr((char*)vPtpBuffer, '/') + 1) | vCurrentParent;
                vResponseCode = OK;
            }
        }

        if (vResponseCode == OK) // Result from successful vfs_file_open or vfs_mkdir
        {
            if (vfs_stat((char*)vPtpBuffer, &info) == 0)
            {
                // Apply timestamp
                info.created = vCreated;
                info.modified = vModified;
                vfs_touch((char*)vPtpBuffer, &info);
                MTP_DBG_LVL2("%s[%u] assigned handle %lX to %s", __FUNCTION__, __LINE__, vSendObjectId, (char*)vPtpBuffer);
            }

            vResponseParam[2] = vSendObjectId;
            vResponseParam[1] = UINT32_MAX;
            if ((vSendObjectParent & INODE_FOLDER_MASK) != INODE_FOLDER_MASK)
            {
                vResponseParam[1] = vSendObjectParent;
            }
            vResponseParam[0] = STORAGE_ID(INODE_STORAGE(vSendObjectId));
            vResponseParamCount = 3;
        }

        MTP_DBG_LVL0("%s[%u] %lX %s rsp 0x%x", __FUNCTION__, __LINE__, vSendObjectId, (char*)vPtpBuffer, vResponseCode);
        return(PtpResponse(id, nullptr, vResponseCode));
    }
    return(0);
}
#endif


#if (MTP_READONLY != 1)
static uint32_t
PtpSendObject(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    if (reqlen == 0)
    {
        PtpSendObjectData(0, nullptr, 0, 0); // init
        MTP_DBG_LVL1("%s[%u]", __FUNCTION__, __LINE__);
    }
    return(0);
}
#endif


#if (MTP_READONLY != 1)
static uint32_t
PtpSendObjectData(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t vExpectLen = 0;
    static uint32_t vReceivedLen = 0;

    int err;

    if (vSendObjectHandle.filesys == nullptr)
    {
        return(PtpResponse(id, nullptr, PtpErr_NoValidObjectInfo));
    }

    if (buf == nullptr)
    {
        vExpectLen = 0;
        return(0);
    }
    if (vExpectLen == 0)
    {
        vExpectLen = GetUint32(&buf[0]) - 12;
        vReceivedLen = 0;
        buf += 12;
        reqlen -= 12;
        MTP_DBG_LVL1("%s[%u] %lu", __FUNCTION__, __LINE__, vExpectLen + 12);
    }

    if (vSendObjectId != 0)
    {
        if (err = vfs_file_write(&vSendObjectHandle, buf, reqlen), err < 0)
        {
            MTP_DBG_LVL0("%s[%u] vfs_file_write error %s", __FUNCTION__, __LINE__, strerror(-err));
            return(PtpResponse(id, nullptr, PtpErr_GeneralError));
        }
    }
    vReceivedLen += reqlen;
    MTP_DBG_LVL3("%s[%u] got %lu bytes of %lu", __FUNCTION__, __LINE__, vReceivedLen, vExpectLen);
    if (vReceivedLen >= vExpectLen)
    {
        if (vSendObjectId == 0) // Created folder
        {
            vResponseCode = OK;
        }
        else
        {
            if (err = vfs_file_close(&vSendObjectHandle), err < 0)
            {
                MTP_DBG_LVL0("%s[%u] vfs_file_close error %s", __FUNCTION__, __LINE__, strerror(-err));
                vResponseCode = PtpErr_GeneralError;
            }
            else
            {
                MTP_DBG_LVL1("%s[%u] saved %luB", __FUNCTION__, __LINE__, vReceivedLen);
                vResponseCode = OK;

        #ifdef MTP_SEND_OBJECT_HOOK
                // Rebuild the name of the file that we just received
                char* path = nullptr;

                GetFileById(nullptr, vSendObjectId, false, &path);
                MTP_SEND_OBJECT_HOOK(&vSendObjectHandle, path);
        #endif
            }
        }
        return(PtpResponse(id, nullptr, vResponseCode));
    }
    return(0);
}
#endif


static uint32_t
PtpFormatStore(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    char* drive;
    int ret;

    if (reqlen == 0)
    {
        ParamParse(buf, 1);   // Storage ID
        MTP_DBG_LVL1("%s[%u] %lX", __FUNCTION__, __LINE__, vParam[0]);

        if (drive = vfs_volume(DRIVE_NUM(vParam[0])), drive == nullptr)
        {
            return(PtpResponse(id, nullptr, PtpErr_InvalidStorageId));
        }

        // format drive
        ret = -vfs_format(drive);
        MTP_DBG_LVL0("%s[%u] %s, result=%u", __FUNCTION__, __LINE__, drive, ret);
        if (ret != 0)
        {
            return(PtpResponse(id, nullptr, PtpErr_GeneralError));
        }
    }
    return(PtpResponse(id, nullptr, OK));
}


static uint32_t
MtpGetObjectPropsSupported(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

    uint32_t i;

    if (reqlen == 0)
    {
        ParamParse(buf, 1); // ?
        len = 0;
        MTP_DBG_LVL1("%s[%u] %lX", __FUNCTION__, __LINE__, vParam[0]);
    }

    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);    // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x9801);    // Code
    len += Uint32(&buf, &index, &reqlen, id);    // TransactionID

    len += Uint32(&buf, &index, &reqlen, (sizeof(vMtpObjectPropsSupported) / sizeof(vMtpObjectPropsSupported[0])) - 1);
    for (i = 0; vMtpObjectPropsSupported[i].prop != 0; i++)
    {
        len += Uint16(&buf, &index, &reqlen, vMtpObjectPropsSupported[i].prop);
    }
    return(len);
}


static uint32_t
MtpGetObjectPropDesc(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;
    uint32_t i;

    if (reqlen == 0)
    {
        ParamParse(buf, 2);   // Property, ObjectType
        len = 0;
        MTP_DBG_LVL1("%s[%u] %lX,%lX", __FUNCTION__, __LINE__, vParam[0], vParam[1]);
    }

    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x9802);  // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

    for (i = 0; vMtpObjectPropsSupported[i].prop != 0; i++)
    {
        if ((vMtpObjectPropsSupported[i].prop == vParam[0]) && (vMtpObjectPropsSupported[i].proc != nullptr))
        {
            len += Uint16(&buf, &index, &reqlen, vMtpObjectPropsSupported[i].prop);
            len += Uint16(&buf, &index, &reqlen, vMtpObjectPropsSupported[i].type);
            len += Uint8(&buf, &index, &reqlen, 0);   // Get (read-only)
            len += (vMtpObjectPropsSupported[i].proc)(&buf, &index, &reqlen, 0, nullptr);   // Default Value
            len += Uint32(&buf, &index, &reqlen, 0);   // Group Code
            len += Uint8(&buf, &index, &reqlen, FORM_NONE);   // Form Flag
            break;
        }
    }
    return(len);
}


static uint32_t
MtpGetObjectPropValue(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;
    uint32_t i;

    if (reqlen == 0)
    {
        ParamParse(buf, 2);   // ObjectHandle, Property
        len = 0;
        MTP_DBG_LVL2("%s[%u] %lX,%lX", __FUNCTION__, __LINE__, vParam[0], vParam[1]);
    }

    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x9803);  // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

    for (i = 0; vMtpObjectPropsSupported[i].prop != 0; i++)
    {
        if ((vMtpObjectPropsSupported[i].prop == vParam[1]) && (vMtpObjectPropsSupported[i].proc != nullptr))
        {
            len += Uint32(&buf, &index, &reqlen, vParam[0]);  // Handle
            len += Uint16(&buf, &index, &reqlen, vMtpObjectPropsSupported[i].prop);
            len += Uint16(&buf, &index, &reqlen, vMtpObjectPropsSupported[i].type);
            len += (vMtpObjectPropsSupported[i].proc)(&buf, &index, &reqlen, vParam[0], nullptr);
            break;
        }
    }
    return(len);
}


#if (MTP_READONLY != 1)
static uint32_t
MtpSetObjectPropValue(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    if (reqlen == 0)
    {
        MtpSetObjectPropValueData(0, nullptr, 0, 0); // init
        MTP_DBG_LVL1("%s[%u]", __FUNCTION__, __LINE__);
    }
    return(0);

    static uint32_t len = 0;
    uint32_t i;

    if (reqlen == 0)
    {
        ParamParse(buf, 2);   // ObjectHandle, Property
        len = 0;
        MTP_DBG_LVL2("%s[%u] %lX,%lX", __FUNCTION__, __LINE__, vParam[0], vParam[1]);
    }

    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x9804);  // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

    if (vParam[1] == 0xDC07)	// ObjectFileName
    {
        len += Uint32(&buf, &index, &reqlen, vParam[0]);  // Handle
        len += Uint16(&buf, &index, &reqlen, vMtpObjectPropsSupported[i].prop);
        len += Uint16(&buf, &index, &reqlen, vMtpObjectPropsSupported[i].type);
 //       len += (vMtpObjectPropsSupported[i].proc)(&buf, &index, &reqlen, vParam[0], nullptr);
    }
    else
    {
        return(PtpResponse(id, nullptr, PtpErr_AccessDenied));
    }
    return(len);
}
#endif


#if (MTP_READONLY != 1)
static uint32_t
MtpSetObjectPropValueData(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t vExpectLen = 0;
    static uint32_t vReceivedLen = 0;
    static uint32_t vFileSize = 0;
    static uint16_t vFormat = 0;
    static uint16_t vNameLen = 0, vCreatedLen = 0, vModifiedLen = 0, vVarIdx = 0;
    static uint8_t vTimeStr[24];
    static time_t  vCreated;
    static time_t  vModified;

    VfsInfo_t info;
    uint32_t i;
    char* p;

    if (buf == nullptr)
    {
        vExpectLen = 0;
        return(0);
    }

    if (vExpectLen == 0)
    {
        vExpectLen = GetUint32(&buf[0]);
        vReceivedLen = 0;
        MTP_DBG_LVL1("%s[%u] %lu", __FUNCTION__, __LINE__, vExpectLen);
        memset(vPtpBuffer, 0, sizeof(vPtpBuffer));  // Needed because of or'ing of filename later on

        if (GetFileById(nullptr, vSendObjectParent, false, &p))
        {
            strcat((char*)vPtpBuffer, p);
            if (p = strrchr((char*)vPtpBuffer, '/'), (p != nullptr) && (p[1] != '\0'))
            {
                strcat((char*)vPtpBuffer, "/");
            }
        }
        vCreated = 0;
        vModified = 0;
    }

    for (i = 0; i < reqlen; i++)
    {
        switch (vReceivedLen + i)
        {
            case OBJECTINFO_FORMATOFFSET + 0: vFormat = buf[i]; break;
            case OBJECTINFO_FORMATOFFSET + 1: vFormat |= buf[i] << 8; break;

            case OBJECTINFO_FILESIZEOFFSET + 0: vFileSize = buf[i]; break;
            case OBJECTINFO_FILESIZEOFFSET + 1: vFileSize |= buf[i] << 8; break;
            case OBJECTINFO_FILESIZEOFFSET + 2: vFileSize |= buf[i] << 16; break;
            case OBJECTINFO_FILESIZEOFFSET + 3: vFileSize |= buf[i] << 24; break;

            case OBJECTINFO_FILENAMEOFFSET: vNameLen = buf[i] << 1; vVarIdx = 0; break;
        }
        if ((vReceivedLen + i) > OBJECTINFO_FILENAMEOFFSET)
        {
            if ((vReceivedLen + i) <= (OBJECTINFO_FILENAMEOFFSET + vNameLen))
            {
                if (p = strrchr((char*)vPtpBuffer, '/'), p != nullptr)
                {
                    p[1 + (vVarIdx++ >> 1)] |= buf[i];
                }
                else
                {
                    return(PtpResponse(id, nullptr, PtpErr_GeneralError));
                }
            }
            else if ((vReceivedLen + i) == (OBJECTINFO_FILENAMEOFFSET + vNameLen + 1))
            {
                vCreatedLen = buf[i] << 1;
                vVarIdx = 0;
                memset(vTimeStr, 0, sizeof(vTimeStr));
            }
            else if ((vReceivedLen + i) <= (OBJECTINFO_FILENAMEOFFSET + vNameLen + vCreatedLen + 1))
            {
                vTimeStr[vVarIdx++ >> 1] |= buf[i];
                if (vVarIdx == vCreatedLen)
                {
                    vCreated = PtpParseDateString(vTimeStr);
                }
            }
            else if ((vReceivedLen + i) == (OBJECTINFO_FILENAMEOFFSET + vNameLen + vCreatedLen + 2))
            {
                vModifiedLen = buf[i] << 1;
                vVarIdx = 0;
                memset(vTimeStr, 0, sizeof(vTimeStr));
            }
            else if ((vReceivedLen + i) <= (OBJECTINFO_FILENAMEOFFSET + vNameLen + vCreatedLen + vModifiedLen + 2))
            {
                vTimeStr[vVarIdx++ >> 1] |= buf[i];
                if (vVarIdx == vModifiedLen)
                {
                    vModified = PtpParseDateString(vTimeStr);
                }
            }
        }
    }

    vReceivedLen += reqlen;
    if (vReceivedLen >= vExpectLen)
    {
        uint32_t fr = 0;

        vResponseCode = 0;
        MTP_DBG_LVL2("%s[%u] %s %luB", __FUNCTION__, __LINE__, (char*)vPtpBuffer, vFileSize);

        if (vfs_fs_size((char*)vPtpBuffer) < 0)
        {
            vResponseCode = PtpErr_StoreNotAvailable;
        }
        else if (vfs_stat((char*)vPtpBuffer, &info) == 0)
        {
            if (!(info.attrib & ATR_IWRITE))
            {
                vResponseCode = PtpErr_ObjectWriteProtected;
            }
            else if (info.attrib & (ATR_HID | ATR_SYS | ATR_DIR))
            {
                vResponseCode = PtpErr_AccessDenied;
            }
            else if (info.size > vFileSize)
            {
                if (vfs_file_open(&vSendObjectHandle, (char*)vPtpBuffer, VFS_WRONLY | VFS_TRUNC) == 0)
                {
                    if (vfs_file_seek(&vSendObjectHandle, vFileSize, SEEK_SET) != 0)
                    {
                        vResponseCode = PtpErr_ObjectTooLarge;
                    }

                    vfs_file_close(&vSendObjectHandle);
                }
                else
                {
                    vResponseCode = PtpErr_GeneralError;
                }
            }
            else if (vFileSize >= vfs_fs_free((char*)vPtpBuffer) - info.size)
            {
                vResponseCode = PtpErr_ObjectTooLarge;
            }
        }
#if VFS_NODIRS != 1
        else if (vFormat == FORMAT_ASSOCIATION)
        {
            int err;

            vResponseCode = PtpErr_GeneralError;
            if (err = vfs_mkdir((char*)vPtpBuffer), err == 0)
            {
                char tmp[13];

                // Add folder entry to cache file
                siprintf(tmp, "%s%s", vfs_volume(DRIVE_NUM(vParam[0])), MTP_FOLDER_CACHE_FILE);
                if (err = vfs_file_open(&vSendObjectHandle, tmp, VFS_RDWR | VFS_CREAT), err != 0)
                {
                    MTP_DBG_LVL0("%s[%u] %s (%s)...", __FUNCTION__, __LINE__, strerror(-err), tmp);
                }
                else
                {
                    i = 0;
                    while (p = vfs_gets(info.name, sizeof(info.name), &vSendObjectHandle), p != nullptr)
                    {
                        p = strchr((char*)vPtpBuffer, ':') + 1;
                        i++;
                        if (strncmp(info.name, p, strlen(p)) == 0)
                        {
                            if (strncmp(info.name + strlen(p), "\n", 2) == 0)
                            {
                                break;
                            }
                        }
                    }
                    if (p == nullptr)
                    {
                        i++;
                        p = strchr((char*)vPtpBuffer, ':') + 1;
                        vfs_puts(p, &vSendObjectHandle);
                        vfs_puts("\n", &vSendObjectHandle);
                    }
                    vSendObjectId = (i << INODE_ITEM_BITS) | (vCurrentParent & INODE_STORAGE_MASK);
                    vfs_file_close(&vSendObjectHandle);
                    vResponseCode = OK;
                }
            }
            else
            {
                MTP_DBG_LVL0("%s[%u] %s (%s)...", __FUNCTION__, __LINE__, strerror(-err), (char*)vPtpBuffer);
            }
        }
#endif
        else if (vFileSize >= vfs_fs_free((char*)vPtpBuffer))
        {
            vResponseCode = PtpErr_ObjectTooLarge;
        }

        if (vResponseCode == 0) // Not yet assigned
        {
            // Create File
            vResponseCode = PtpErr_GeneralError;
            if (vfs_file_open(&vSendObjectHandle, (char*)vPtpBuffer, VFS_RDWR | VFS_TRUNC) == 0)
            {
                vfs_file_sync(&vSendObjectHandle);
                // Generate handle
                vSendObjectId = HandleFilenameBits(strrchr((char*)vPtpBuffer, '/') + 1) | vCurrentParent;
                vResponseCode = OK;
            }
        }

        if (vResponseCode == OK) // Result from successful vfs_file_open or vfs_mkdir
        {
            if (vfs_stat((char*)vPtpBuffer, &info) == 0)
            {
                // Apply timestamp
                info.created = vCreated;
                info.modified = vModified;
                vfs_touch((char*)vPtpBuffer, &info);
                MTP_DBG_LVL2("%s[%u] assigned handle %lX to %s", __FUNCTION__, __LINE__, vSendObjectId, (char*)vPtpBuffer);
            }

            vResponseParam[2] = vSendObjectId;
            vResponseParam[1] = UINT32_MAX;
            if ((vSendObjectParent & INODE_FOLDER_MASK) != INODE_FOLDER_MASK)
            {
                vResponseParam[1] = vSendObjectParent;
            }
            vResponseParam[0] = STORAGE_ID(INODE_STORAGE(vSendObjectId));
            vResponseParamCount = 3;
        }

        MTP_DBG_LVL0("%s[%u] %lX %s rsp 0x%x", __FUNCTION__, __LINE__, vSendObjectId, (char*)vPtpBuffer, vResponseCode);
        return(PtpResponse(id, nullptr, vResponseCode));
    }
    return(0);
}
#endif


static uint32_t
MtpGetObjectPropList(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

    VfsInfo_t* info = nullptr;
    uint32_t count = 0;
    uint32_t i;

    if (reqlen == 0)
    {
        ParamParse(buf, 5);    // ObjectHandle, [ObjectFormatCode], ObjectPropCode, [ObjectPropGroupCode], [Depth]
        len = 0;
        MTP_DBG_LVL2("%s[%u] %lX,%lX,%lX,%lX,%lu", __FUNCTION__, __LINE__, vParam[0], vParam[1], vParam[2], vParam[3], vParam[4]);

        if (vParam[1] != 0)
        {
            return(PtpResponse(id, nullptr, PtpErr_SpecificationByFormatUnsupported));
        }
        if (vParam[3] != 0)
        {
            return(PtpResponse(id, nullptr, PtpErr_SpecificationByGroupUnsupported));
        }
        if (vParam[4] != 0)
        {
            return(PtpResponse(id, nullptr, PtpErr_SpecificationByDepthUnsupported));
        }
    }

    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x9805);  // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

    if ((vParam[0] != 0) && (vParam[0] != UINT32_MAX))
    {
        if (!GetFileById(&info, vParam[0], false, nullptr))
        {
            return(PtpResponse(id, nullptr, PtpErr_InvalidObjectHandle));
        }
    }

    for (i = 0; vMtpObjectPropsSupported[i].prop != 0; i++)
    {
        if (((vMtpObjectPropsSupported[i].prop == vParam[2]) || (vParam[2] == UINT32_MAX)) && (vMtpObjectPropsSupported[i].proc != nullptr))
        {
            count++;
        }
    }
    len += Uint32(&buf, &index, &reqlen, count);  // Number of quadruples

    for (i = 0; vMtpObjectPropsSupported[i].prop != 0; i++)
    {
        if (((vMtpObjectPropsSupported[i].prop == vParam[2]) || (vParam[2] == UINT32_MAX)) && (vMtpObjectPropsSupported[i].proc != nullptr))
        {
            len += Uint32(&buf, &index, &reqlen, vParam[0]);  // Handle
            len += Uint16(&buf, &index, &reqlen, vMtpObjectPropsSupported[i].prop);
            len += Uint16(&buf, &index, &reqlen, vMtpObjectPropsSupported[i].type);
            len += (vMtpObjectPropsSupported[i].proc)(&buf, &index, &reqlen, vParam[0], info);
        }
    }
    return(len);
}


static uint32_t
PtpGetDevicePropDesc(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;
    uint32_t i;

    if (reqlen == 0)
    {
        ParamParse(buf, 1);   // DevicePropCode
        len = 0;
        MTP_DBG_LVL2("%s[%u] %lX", __FUNCTION__, __LINE__, vParam[0]);
    }
    len += Uint32(&buf, &index, &reqlen, len);  // Length
    len += Uint16(&buf, &index, &reqlen, 2);       // Container Type = Data Block
    len += Uint16(&buf, &index, &reqlen, 0x1014);  // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

    for (i = 0; vMtpDevicePropsSupported[i].prop != 0; i++)
    {
        if ((vMtpDevicePropsSupported[i].prop == vParam[0]) && (vMtpDevicePropsSupported[i].proc != nullptr))
        {
            len += Uint16(&buf, &index, &reqlen, vMtpDevicePropsSupported[i].prop);
            len += Uint16(&buf, &index, &reqlen, vMtpDevicePropsSupported[i].type);
            len += Uint8(&buf, &index, &reqlen, 0);   // Get (read-only)
            len += (vMtpDevicePropsSupported[i].proc)(&buf, &index, &reqlen, PROP_DEFAULT);   // Default Value
            len += (vMtpDevicePropsSupported[i].proc)(&buf, &index, &reqlen, PROP_VALUE);   // Current Value
            len += Uint8(&buf, &index, &reqlen, vMtpDevicePropsSupported[i].form);   // Form Flag
            if (vMtpDevicePropsSupported[i].form == FORM_RANGE)
            {
                len += (vMtpDevicePropsSupported[i].proc)(&buf, &index, &reqlen, PROP_MIN);   // Minimum Value
                len += (vMtpDevicePropsSupported[i].proc)(&buf, &index, &reqlen, PROP_MAX);   // Maximum Value
                len += (vMtpDevicePropsSupported[i].proc)(&buf, &index, &reqlen, PROP_STEP);   // Step Value
            }
            break;
        }
    }
    return(len);
}


static uint32_t
PtpGetDevicePropValue(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

    uint32_t i;

    if (reqlen == 0)
    {
        ParamParse(buf, 3);   // Property, unused, unused
        len = 0;
        MTP_DBG_LVL1("%s[%u] %lX,%lu,%lu", __FUNCTION__, __LINE__, vParam[0], vParam[1], vParam[2]);
    }

    for (i = 0; vMtpDevicePropsSupported[i].prop != 0; i++)
    {
        if ((vMtpDevicePropsSupported[i].prop == vParam[0]) && (vMtpDevicePropsSupported[i].proc != nullptr))
        {
            len += (vMtpDevicePropsSupported[i].proc)(&buf, &index, &reqlen, PROP_VALUE);
            break;
        }
    }
    return(PtpResponse(id, vPtpBuffer, OK));
}


static uint32_t
PtpSetDevicePropValue(uint32_t id, uint8_t* buf, uint32_t index, uint32_t reqlen)
{
    static uint32_t len = 0;

#if 0
    uint32_t i;
#endif

    if (reqlen == 0)
    {
        ParamParse(buf, 3);   // Property, unused
        len = 0;
        MTP_DBG_LVL1("%s[%u] %lX,%lu,%lu", __FUNCTION__, __LINE__, vParam[0], vParam[1], vParam[2]);
    }

#if 1
    return(PtpResponse(id, nullptr, PtpErr_AccessDenied));
#else
    for (i = 0; vMtpDevicePropsSupported[i].prop != 0; i++)
    {
        if ((vMtpDevicePropsSupported[i].prop == vParam[0]) )// && (vMtpDevicePropsSupported[i].proc != nullptr))
        {
            //len += vMtpDevicePropsSupported[i].proc(&buf, &index, &reqlen, PROP_TBD);
            break;
        }
    }
    return(PtpResponse(id, vPtpBuffer, OK));
#endif
}

static volatile uint32_t vResponseLength = 0;
static volatile uint32_t vResponseIndex = 0;


static uint32_t
PtpResponse(uint32_t id, uint8_t* buf, uint16_t resp)
{
    uint32_t index = 0;
    uint32_t reqlen = 12 + vResponseParamCount * sizeof(uint32_t);
    uint32_t len = 0;
    uint8_t i;

    MTP_DBG_LVL3("%s[%u] id=%lu buf=%p resp=%X, code=%X, nparam=%u", __FUNCTION__, __LINE__, id, buf, resp, vResponseCode, vResponseParamCount);
    if (resp == 0)   // Use the stored response code
    {
        resp = vResponseCode;
    }
    if (resp == 0)   // Still nothing, then assume OK
    {
        resp = OK;
    }

    len += Uint32(&buf, &index, &reqlen, reqlen);       // Length
    len += Uint16(&buf, &index, &reqlen, 3);       // Container Type = Response Block
    len += Uint16(&buf, &index, &reqlen, resp);    // Code
    len += Uint32(&buf, &index, &reqlen, id);      // TransactionID

    for (i = 0; i < vResponseParamCount; i++)
    {
        len += Uint32(&buf, &index, &reqlen, vResponseParam[i]);
    }

    vResponseIndex = UINT32_MAX;
    if (buf != nullptr)
    {
        vResponseCode = 0;
        vResponseParamCount = 0;
        memset((uint8_t*)vResponseParam, 0, sizeof(vResponseParam));
    }
    else
    {
        vResponseCode = resp;
    }
    return(len);
}


static volatile uint32_t vResponseId;
static PtpProc_t pResponseProc;

static const struct PtpOpcodeTable_s* pPtpOpcode = nullptr;


bool
PtpPayloadIn(uint8_t* buf, uint32_t vLength)
{
    uint32_t length = GetUint32(&buf[0]);	// Offset 0, length 4 bytes -> Container Length
    uint16_t type = GetUint16(&buf[4]);	// Offset 4, length 2 bytes -> Container Type
    uint16_t code = GetUint16(&buf[6]);	// Offset 6, length 2 bytes -> Code
    uint32_t id = GetUint32(&buf[8]);	// Offset 8, length 4 bytes -> Transaction ID
    uint8_t* d = &buf[12];	// Offset 12, length ?? -> Payload

    static uint32_t vDataIndex = 0;
    static PtpProc_t pDataProc = nullptr;

    uint32_t i;

    if ((vDataIndex > 0) && (pDataProc != nullptr))
    {
        vResponseIndex = UINT32_MAX;
        vResponseLength = (pDataProc)(vResponseId, buf, vDataIndex, vLength);
        if (vResponseLength == 0)
        {
            vDataIndex += vLength;
        }
        else
        {
            vDataIndex = 0;
        }
        return(true);
    }

    MTP_DBG_LVL3("%s[%u] type %u, code %X, length %lu", __FUNCTION__, __LINE__, type, code, length);

    switch (type)
    {
        case 1:	// Command Block
            for (i = 0; vPtpOpcodeTable[i].opcode != 0; i++)
            {
                if (vPtpOpcodeTable[i].opcode == code)
                {
                    pPtpOpcode = &vPtpOpcodeTable[i];
                    pResponseProc = vPtpOpcodeTable[i].proc;
                    pDataProc = vPtpOpcodeTable[i].data;
                    vResponseId = id;

                    vResponseIndex = 0;
                    vResponseLength = (pPtpOpcode->proc)(id, buf, vResponseIndex, 0);
                    vDataIndex = 0;
                    return(true);
                }
            }
            return(false);

        case 2:	// Data Block
            if (vResponseId == id)
            {
                vResponseIndex = UINT32_MAX;
                vResponseLength = (pDataProc)(id, buf, vDataIndex, vLength);
                if (vResponseLength == 0)
                {
                    vDataIndex = vLength;
                }
                else
                {
                    vDataIndex = 0;
                }
                return(true);
            }
            return(false);

        case 3:	// Response Block	(only for IN endpoint)
        case 4:	// Event Block		(only for interrupt IN endpoint)
            return(false);	// callee should stall the endpoint
            break;

        case 0: // undefined
        default:
            return(false);	// callee should stall the endpoint
            break;
    }
}


uint8_t*
PtpPayloadOut(uint32_t vRequestLength, uint32_t *pLength)
{
    if ((vResponseIndex <= vResponseLength) && (vResponseLength != 0))
    {
        uint8_t* p = nullptr;

        if (pResponseProc != nullptr)
        {
            // retrieve next segment of data
            (pResponseProc)(vResponseId, vPtpBuffer, vResponseIndex, vRequestLength);
            *pLength = vResponseLength - vResponseIndex;
            if (*pLength > vRequestLength)
            {
                *pLength = vRequestLength;
            }
            p = vPtpBuffer;
            MTP_DBG_LVL3("%s[%u] id %lu: %p idx=%ld len=%lu - sending %lu", __FUNCTION__, __LINE__, vResponseId, p, vResponseIndex, vResponseLength, *pLength);
            vResponseIndex += vRequestLength;
        }
        return(p);
    }
    else if ((pResponseProc != nullptr) && (vResponseLength != 0))
    {
        *pLength = PtpResponse(vResponseId, vPtpBuffer, 0);
        pResponseProc = nullptr;
        MTP_DBG_LVL3("%s[%u] id %lu: %p. %ld %lu -%u", __FUNCTION__, __LINE__, vResponseId, vPtpBuffer, vResponseIndex, vResponseLength, vPtpBuffer[4]);
        return(vPtpBuffer);
    }
    return(nullptr);	// callee should stall the endpoint
}


void
PtpCancelRequest(uint8_t* buf)
{
    uint16_t code = GetUint16(&buf[0]);
    uint32_t id = GetUint32(&buf[2]);

    if (code != 0x4001)
    {
        return;
    }
    MTP_DBG_LVL0("%s[%u] %lu", __FUNCTION__, __LINE__, id);

#if (MTP_READONLY != 1)
    if (vSendObjectHandle.filesys != nullptr)
    {
        vfs_file_close(&vSendObjectHandle);
        // Set marker so that MtpGetDeviceStatus knows the transaction was canceled
        vSendObjectHandle.filesys = (FileSystem_t*)0xA5;

        char* path;
        GetFileById(nullptr, vSendObjectId, false, &path);

        vfs_remove(path);
    }
#endif
}


uint8_t*
MtpGetDeviceStatus(uint16_t* len)
{
    static uint8_t buf[4];
    uint8_t* p = buf;

    uint32_t index = 0, reqlen = 4;
    uint16_t vResponse = OK;

#if (MTP_READONLY != 1)
    if (vSendObjectHandle.filesys == (FileSystem_t*)0xA5)
    {
        vSendObjectHandle.filesys = nullptr;
        vResponse = PtpErr_TransactionCancelled;
    }
#endif

    *len = 0;
    // Using the Uint16 function is overkill here, but we have it anyway
    *len += Uint16(&p, &index, &reqlen, 4);       // Length of response
    *len += Uint16(&p, &index, &reqlen, vResponse);       // Status code

    return(buf);
}


void
PtpReset(void)
{
    PtpCloseSession(0, nullptr, 0, 0);
}


#ifdef MTP_EVENTS
bool
PtpEvent(MtpEvent_t vEvent, uint32_t vParam)
{
    static uint8_t buf[24];
    uint8_t* p = buf;
    uint16_t len = 0;
    uint32_t index = 0, reqlen = 24;

    // TODO Maybe needed to implement GetExtendedEventDataRequest ?

    if (vPtpSession != 0)
    {
        len += Uint32(&p, &index, &reqlen, sizeof(buf));    // Interrupt Data Length
        len += Uint16(&p, &index, &reqlen, 0x0004);         // Container Type = Event
        len += Uint16(&p, &index, &reqlen, vEvent);         // PIMA 15740 Event Code
        len += Uint32(&p, &index, &reqlen, UINT32_MAX);     // Session ID = all
        len += Uint32(&p, &index, &reqlen, 0);              // Transaction ID
        len += Uint32(&p, &index, &reqlen, vParam);              // Event Parameter 1
        //len += Uint32(&p, &index, &reqlen, 0);              // Event Parameter 2
        //len += Uint32(&p, &index, &reqlen, 0);              // Event Parameter 3

    #if 1
        USBD_MTP_SendInterruptData(buf, len);
    #endif
    }
    return(true);
}
#endif
