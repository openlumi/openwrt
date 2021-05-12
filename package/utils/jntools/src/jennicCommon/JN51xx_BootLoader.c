/****************************************************************************
 *
 * MODULE:             JN51xx_BootLoader
 *
 * COMPONENT:          $RCSfile: JN51xx_BootLoader.c,v $
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2009/03/02 13:33:44 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell
 *
 * DESCRIPTION:
 *
 *
 * LAST MODIFIED BY:   $Author: lmitch $
 *                     $Modtime: $
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 * Copyright NXP B.V. 2012. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include <programmer.h>

#include "uart.h"
#include "dbg.h"
#include "JN51xx_BootLoader.h"
#include "portable_endian.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_BOOTLOADER
#define TRACE_BOOTLOADER    TRUE
#else
#define TRACE_BOOTLOADER    FALSE
#endif

#define BL_TIMEOUT_250MS    250000
#define BL_TIMEOUT_500MS    500000
#define BL_TIMEOUT_1S       1000000
#define BL_TIMEOUT_10S      10000000

#define BOOTLOADER_MAX_MESSAGE_LENGTH 255


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef enum
{
    E_BL_MSG_TYPE_SET_CS_REQUEST                        = 0x05,
    E_BL_MSG_TYPE_SET_CS_RESPONSE                       = 0x06,
    E_BL_MSG_TYPE_FLASH_ERASE_REQUEST                   = 0x07,
    E_BL_MSG_TYPE_FLASH_ERASE_RESPONSE                  = 0x08,
    E_BL_MSG_TYPE_FLASH_PROGRAM_REQUEST                 = 0x09,
    E_BL_MSG_TYPE_FLASH_PROGRAM_RESPONSE                = 0x0a,
    E_BL_MSG_TYPE_FLASH_READ_REQUEST                    = 0x0b,
    E_BL_MSG_TYPE_FLASH_READ_RESPONSE                   = 0x0c,
    E_BL_MSG_TYPE_FLASH_SECTOR_ERASE_REQUEST            = 0x0d,
    E_BL_MSG_TYPE_FLASH_SECTOR_ERASE_RESPONSE           = 0x0e,
    E_BL_MSG_TYPE_FLASH_WRITE_PRG_REGISTER_REQUEST      = 0x0f,
    E_BL_MSG_TYPE_FLASH_WRITE_PRG_REGISTER_RESPONSE     = 0x10,
    E_BL_MSG_TYPE_RESET_REQUEST                         = 0x14,
    E_BL_MSG_TYPE_RESET_RESPONSE                        = 0x15,
    E_BL_MSG_TYPE_RAM_WRITE_REQUEST                     = 0x1d,
    E_BL_MSG_TYPE_RAM_WRITE_RESPONSE                    = 0x1e,
    E_BL_MSG_TYPE_RAM_READ_REQUEST                      = 0x1f,
    E_BL_MSG_TYPE_RAM_READ_RESPONSE                     = 0x20,
    E_BL_MSG_TYPE_RAM_RUN_REQUEST                       = 0x21,
    E_BL_MSG_TYPE_RAM_RUN_RESPONSE                      = 0x22,
    E_BL_MSG_TYPE_FLASH_READ_ID_REQUEST                 = 0x25,
    E_BL_MSG_TYPE_FLASH_READ_ID_RESPONSE                = 0x26,
    E_BL_MSG_TYPE_SET_BAUD_REQUEST                      = 0x27,
    E_BL_MSG_TYPE_SET_BAUD_RESPONSE                     = 0x28,
    E_BL_MSG_TYPE_FLASH_SELECT_TYPE_REQUEST             = 0x2c,
    E_BL_MSG_TYPE_FLASH_SELECT_TYPE_RESPONSE            = 0x2d,

    E_BL_MSG_TYPE_GET_CHIPID_REQUEST                    = 0x32,
    E_BL_MSG_TYPE_GET_CHIPID_RESPONSE                   = 0x33,

    /* Flash programmer extension commands */
    E_BL_MSG_TYPE_PDM_ERASE_REQUEST                     = 0x36,
    E_BL_MSG_TYPE_PDM_ERASE_RESPONSE                    = 0x37,
    E_BL_MSG_TYPE_PROGRAM_INDEX_SECTOR_REQUEST          = 0x38,
    E_BL_MSG_TYPE_PROGRAM_INDEX_SECTOR_RESPONSE         = 0x39,
    E_BL_MSG_TYPE_EEPROM_READ_REQUEST                   = 0x3A,
    E_BL_MSG_TYPE_EEPROM_READ_RESPONSE                  = 0x3B,
    E_BL_MSG_TYPE_EEPROM_WRITE_REQUEST                  = 0x3C,
    E_BL_MSG_TYPE_EEPROM_WRITE_RESPONSE                 = 0x3D,

} __attribute ((packed)) teBL_MessageType;


typedef enum
{
    E_BL_RESPONSE_OK                                    = 0x00,
    E_BL_RESPONSE_NOT_SUPPORTED                         = 0xff,
    E_BL_RESPONSE_WRITE_FAIL                            = 0xfe,
    E_BL_RESPONSE_INVALID_RESPONSE                      = 0xfd,
    E_BL_RESPONSE_CRC_ERROR                             = 0xfc,
    E_BL_RESPONSE_ASSERT_FAIL                           = 0xfb,
    E_BL_RESPONSE_USER_INTERRUPT                        = 0xfa,
    E_BL_RESPONSE_READ_FAIL                             = 0xf9,
    E_BL_RESPONSE_TST_ERROR                             = 0xf8,
    E_BL_RESPONSE_AUTH_ERROR                            = 0xf7,
    E_BL_RESPONSE_NO_RESPONSE                           = 0xf6,
    E_BL_RESPONSE_ERROR                                 = 0xf0,
} __attribute__ ((packed)) teBL_Response;


typedef struct
{
    uint8_t                 u8ManufacturerID;
    uint8_t                 u8DeviceID;
    uint8_t                 u8FlashType;
    const char             *pcFlashName;
    uint32_t                u32FlashSize;
} tsBL_FlashDevice;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

static teBL_Response eBL_Request(tsPRG_Context *psContext, int iTimeoutMicroseconds, teBL_MessageType eTxType, uint8_t u8HeaderLen, uint8_t *pu8Header, uint8_t u8TxLength, uint8_t *pu8TxData, teBL_MessageType *peRxType, uint8_t *pu8RxLength, uint8_t *pu8RxData);

static teStatus eBL_WriteMessage(tsPRG_Context *psContext, teBL_MessageType eType, uint8_t u8HeaderLength, uint8_t *pu8Header, uint8_t u8Length, uint8_t *pu8Data);
static teBL_Response eBL_ReadMessage(tsPRG_Context *psContext, int iTimeoutMicroseconds, teBL_MessageType *peType, uint8_t *pu8Length, uint8_t *pu8Data);

static teStatus eBL_CheckResponse(const char *pcFunction, teBL_Response eResponse, teBL_MessageType eRxType, teBL_MessageType eExpectedRxType);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/** Array of flash devices that the bootloader knows about */
static tsBL_FlashDevice asFlashDevices[] =
{
    {
        .u8ManufacturerID   = 0x05,
        .u8DeviceID         = 0x05,
        .u8FlashType        = 4,
        .pcFlashName        = "SPI flash ST M25P05-A",
        .u32FlashSize       = 64 * 1024,
    },
    {
        .u8ManufacturerID   = 0x10,
        .u8DeviceID         = 0x10,
        .u8FlashType        = 0,
        .pcFlashName        = "SPI flash ST M25P10-A",
        .u32FlashSize       = 128 * 1024,
    },
    {
        .u8ManufacturerID   = 0x11,
        .u8DeviceID         = 0x11,
        .u8FlashType        = 5,
        .pcFlashName        = "SPI flash ST M25P20-A",
        .u32FlashSize       = 256 * 1024,
    },

    {
        .u8ManufacturerID   = 0x12,
        .u8DeviceID         = 0x12,
        .u8FlashType        = 3,
        .pcFlashName        = "SPI flash ST M25P40",
        .u32FlashSize       = 512 * 1024,
    },

    {
        .u8ManufacturerID   = 0xBF,
        .u8DeviceID         = 0x49,
        .u8FlashType        = 1,
        .pcFlashName        = "SPI flash SST 25VF010A",
        .u32FlashSize       = 128 * 1024,
    },

    {
        .u8ManufacturerID   = 0x1F,
        .u8DeviceID         = 0x60,
        .u8FlashType        = 2,
        .pcFlashName        = "SPI flash Atmel 25F512",
        .u32FlashSize       = 512 * 1024,
    },

    {
        /* JN516x Internal flash - don't need the name / size as it is determined by ePRG_ChipGetChipId */
        .u8ManufacturerID   = 0xCC,
        .u8DeviceID         = 0xEE,
        .u8FlashType        = 8,
    },
};


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

teStatus eBL_SetBaudrate(tsPRG_Context *psContext, uint32_t u32Baudrate)
{
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t au8Buffer[6];
    uint32_t u32Divisor;

    DBG_vPrintf(TRACE_BOOTLOADER, "Set BL Baud rate to %d\n", u32Baudrate);

    // Divide 1MHz clock by baudrate to get the divisor
    // u32Divisor = (uint32_t)roundf(1000000.0 / (float)u32Baudrate);
    u32Divisor = 1000000 / u32Baudrate;

    DBG_vPrintf(TRACE_BOOTLOADER, "Set divisor %d\n", u32Divisor);

    au8Buffer[0] = (uint8_t)u32Divisor;
    au8Buffer[1] = 0;
    au8Buffer[2] = 0;
    au8Buffer[3] = 0;
    au8Buffer[4] = 0;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_SET_BAUD_REQUEST, 1, au8Buffer, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_SET_BAUD_RESPONSE);
}


/****************************************************************************
 *
 * NAME: iBL_ReadChipId
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_ChipIdRead(tsPRG_Context *psContext, uint32_t *pu32ChipId, uint32_t *pu32BootloaderVersion)
{

    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t u8RxDataLen = 0;
    uint8_t au8Buffer[BOOTLOADER_MAX_MESSAGE_LENGTH];

    if(pu32ChipId == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }

    if(pu32BootloaderVersion == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }
    
    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_GET_CHIPID_REQUEST, 0, NULL, 0, NULL, &eRxType, &u8RxDataLen, au8Buffer);

    if ((u8RxDataLen != 4) && (u8RxDataLen != 8))
    {
        return E_PRG_COMMS_FAILED;
    }

    *pu32ChipId  = au8Buffer[0] << 24;
    *pu32ChipId |= au8Buffer[1] << 16;
    *pu32ChipId |= au8Buffer[2] << 8;
    *pu32ChipId |= au8Buffer[3] << 0;

    if(u8RxDataLen == 8)
    {
        // Bootloader version is included
        *pu32BootloaderVersion  = au8Buffer[4] << 24;
        *pu32BootloaderVersion |= au8Buffer[5] << 16;
        *pu32BootloaderVersion |= au8Buffer[6] << 8;
        *pu32BootloaderVersion |= au8Buffer[7] << 0;

        DBG_vPrintf(TRACE_BOOTLOADER, "Bootloader Id Detected : u32BootloaderVersion = %x \n", *pu32BootloaderVersion);
    } else {
        *pu32BootloaderVersion = 0;
    }

    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_GET_CHIPID_RESPONSE);
}


/****************************************************************************
 *
 * NAME: eBL_DiscoverFlash
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_DiscoverFlash(tsPRG_Context *psContext, uint8_t u8ChipSelect, uint8_t *pu8ManufacturerID, uint8_t *pu8DeviceID, const char **ppcName, uint32_t *pu32FlashSize)
{
    teStatus eStatus;
    int n;

    /* Search for flash type given flash device id code */
    for(n = 0; n < sizeof(asFlashDevices) / sizeof(tsBL_FlashDevice); n++)
    {
        if ((asFlashDevices[n].u8ManufacturerID == FLASH_MANUFACTURER_JN516X) && (asFlashDevices[n].u8DeviceID == FLASH_DEVICE_JN516X))
        {
            /* Don't attempt to detect the internal flash - that has already been detected if present */
            continue;
        }
        /* Attempt to automatically discover SPI connected flash */
        DBG_vPrintf(TRACE_BOOTLOADER, "Trying flash type %d (0x%02X:0x%02X)\n", asFlashDevices[n].u8FlashType, asFlashDevices[n].u8ManufacturerID, asFlashDevices[n].u8DeviceID);
        eStatus = eBL_FlashSelectDevice(psContext, asFlashDevices[n].u8ManufacturerID, asFlashDevices[n].u8DeviceID, u8ChipSelect);
        if (eStatus != E_PRG_OK)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "Flash select device failed(%d)\n", eStatus);
        }
        else
        {
            /* Get the flash Id */
            eStatus = eBL_FlashIdRead(psContext, pu8ManufacturerID, pu8DeviceID, ppcName, pu32FlashSize);
            if (eStatus == E_PRG_OK)
            {
                DBG_vPrintf(TRACE_BOOTLOADER, "Found flash %s\n", asFlashDevices[n].pcFlashName);
                return E_PRG_OK;
            }
        }
    }

    DBG_vPrintf(TRACE_BOOTLOADER, "No SPI flash device\n");
    return E_PRG_FLASH_DEVICE_UNAVAILABLE;
}


/****************************************************************************
 *
 * NAME: iBL_ReadFlashId
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_FlashIdRead(tsPRG_Context *psContext, uint8_t *pu8ManufacturerID, uint8_t *pu8DeviceID, const char **ppcName, uint32_t *pu32FlashSize)
{
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t u8RxDataLen = 0;
    uint8_t au8Buffer[BOOTLOADER_MAX_MESSAGE_LENGTH];
    int n;

    if(!psContext || !pu8ManufacturerID || !pu8DeviceID || !pu32FlashSize)
    {
        return E_PRG_NULL_PARAMETER;
    }

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_READ_ID_REQUEST, 0, NULL, 0, NULL, &eRxType, &u8RxDataLen, au8Buffer);

    if (u8RxDataLen != 2)
    {
        return E_PRG_COMMS_FAILED;
    }

    /* Search for flash type given flash device id code */
    for(n = 0; n < sizeof(asFlashDevices) / sizeof(tsBL_FlashDevice); n++)
    {
        /* If we found a match, send command to select this flash device type */
        if((asFlashDevices[n].u8ManufacturerID == au8Buffer[0]) && (asFlashDevices[n].u8DeviceID == au8Buffer[1]))
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "Found Flash \"%s\"\n", asFlashDevices[n].pcFlashName);
            *pu8ManufacturerID  = au8Buffer[0];
            *pu8DeviceID        = au8Buffer[1];
            *ppcName            = asFlashDevices[n].pcFlashName;
            *pu32FlashSize      = asFlashDevices[n].u32FlashSize;
            break;
        }
    }

    if (n == sizeof(asFlashDevices) / sizeof(tsBL_FlashDevice))
    {
        /* Got to end of list without finding the device */
        return E_PRG_FLASH_DEVICE_UNAVAILABLE;
    }

    DBG_vPrintf(TRACE_BOOTLOADER, "Flash device manufacturer 0x%02X, device 0x%02X\n", au8Buffer[0], au8Buffer[1]);

    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_FLASH_READ_ID_RESPONSE);
}


/****************************************************************************
 *
 * NAME: iBL_ReadFlashId
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_FlashSelectDevice(tsPRG_Context *psContext, uint8_t u8ManufacturerID, uint8_t u8DeviceID, uint8_t u8ChipSelect)
{
    int n;
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    uint8_t au8Buffer[6];
    uint8_t u8FlashType = 0;
    teStatus eStatus;

    if ((u8ManufacturerID != 0) && (u8DeviceID != 0))
    {
        /* Search for flash type given flash device id code */
        for(n = 0; n < sizeof(asFlashDevices) / sizeof(tsBL_FlashDevice); n++)
        {
            /* If we found a match, send command to select this flash device type */
            if((asFlashDevices[n].u8ManufacturerID == u8ManufacturerID) && (asFlashDevices[n].u8DeviceID == u8DeviceID))
            {
                DBG_vPrintf(TRACE_BOOTLOADER, "Selecting \"%s\"\n", asFlashDevices[n].pcFlashName);
                u8FlashType = asFlashDevices[n].u8FlashType;
                break;
            }
        }

        if (n == sizeof(asFlashDevices) / sizeof(tsBL_FlashDevice))
        {
            /* Got to end of list without finding the device */
            return E_PRG_FLASH_DEVICE_UNAVAILABLE;
        }

        /* Assert the selected chip select */

        if (u8ChipSelect != 0)
        {
            au8Buffer[0] = u8ChipSelect;

            DBG_vPrintf(TRACE_BOOTLOADER, "Selecting chip select %d\n", u8ChipSelect);
            eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_SET_CS_REQUEST, 1, au8Buffer, 0, NULL, &eRxType, NULL, NULL);
            if ((eStatus = eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_SET_CS_RESPONSE)) != E_PRG_OK)
            {
                return eStatus;
            }
        }
    }

    DBG_vPrintf(TRACE_BOOTLOADER, "Select flash type %d\n", u8FlashType);
    au8Buffer[0] = u8FlashType;
    au8Buffer[1] = 0;
    au8Buffer[2] = 0;
    au8Buffer[3] = 0;
    au8Buffer[4] = 0;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_SELECT_TYPE_REQUEST, 5, au8Buffer, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_FLASH_SELECT_TYPE_RESPONSE);
}


/****************************************************************************
 *
 * NAME: iBL_WriteStatusRegister
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_FlashStatusRegisterWrite(tsPRG_Context *psContext, uint8_t u8StatusReg)
{
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    DBG_vPrintf(TRACE_BOOTLOADER, "Writing %02x to flash status register\n", u8StatusReg);

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_WRITE_PRG_REGISTER_REQUEST, 1, &u8StatusReg, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_FLASH_WRITE_PRG_REGISTER_RESPONSE);
}


/****************************************************************************
 *
 * NAME: iBL_RunRAM
 *
 * DESCRIPTION:
 *  Starts the module executing code from a given address
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_MemoryExecute(tsPRG_Context *psContext, uint32_t u32Address)
{
    uint8_t au8CmdBuffer[4];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    DBG_vPrintf(TRACE_BOOTLOADER, "Execute code at 0x%08X\n", u32Address);

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_RAM_RUN_REQUEST, 4, au8CmdBuffer, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_RAM_RUN_RESPONSE);
}


/****************************************************************************
 *
 * NAME: iBL_ReadRAM
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_MemoryRead(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t u8BlockSize, uint8_t *pu8Buffer)
{
    uint8_t u8RxDataLen = 0;
    uint8_t au8CmdBuffer[6];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    int i;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        return E_PRG_BAD_PARAMETER;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
    au8CmdBuffer[4] = u8Length;
    au8CmdBuffer[5] = 0;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_RAM_READ_REQUEST, 6, au8CmdBuffer, 0, NULL, &eRxType, &u8RxDataLen, pu8Buffer);

    if (u8RxDataLen != u8Length)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Requested %d bytes, got %d\n", u8Length, u8RxDataLen);
        return E_PRG_ERROR_READING;
    }

    /* Convert the read byte buffer into native format */
    if (u8BlockSize == sizeof(uint16_t))
    {
        for (i = 0; i < u8Length; i += sizeof(uint16_t))
        {
            uint16_t u16Short;
            memcpy(&u16Short, &pu8Buffer[i], sizeof(uint16_t));

            if (psContext->sChipDetails.eEndianness == E_CHIP_LITTLE_ENDIAN)
            {
                u16Short = le16toh(u16Short);
            }
            else if (psContext->sChipDetails.eEndianness == E_CHIP_BIG_ENDIAN)
            {
                u16Short = be16toh(u16Short);
            }
            else
            {
                return E_PRG_BAD_PARAMETER;
            }
            memcpy(&pu8Buffer[i], &u16Short, sizeof(uint16_t));
        }
    }
    else if (u8BlockSize == sizeof(uint32_t))
    {
        for (i = 0; i < u8Length; i += sizeof(uint32_t))
        {
            uint32_t u32Word;
            memcpy(&u32Word, &pu8Buffer[i], sizeof(uint32_t));

            if (psContext->sChipDetails.eEndianness == E_CHIP_LITTLE_ENDIAN)
            {
                u32Word = le32toh(u32Word);
            }
            else if (psContext->sChipDetails.eEndianness == E_CHIP_BIG_ENDIAN)
            {
                u32Word = be32toh(u32Word);
            }
            else
            {
                return E_PRG_BAD_PARAMETER;
            }
            memcpy(&pu8Buffer[i], &u32Word, sizeof(uint32_t));
        }
    }

    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_RAM_READ_RESPONSE);
}

/****************************************************************************
 *
 * NAME: iBL_WriteRAM
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_MemoryWrite(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t u8BlockSize, uint8_t *pu8Buffer)
{
    uint8_t au8CmdBuffer[6];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    int i;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        return E_PRG_BAD_PARAMETER;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
    au8CmdBuffer[4] = u8Length;
    au8CmdBuffer[5] = 0;

    /* Convert the native byte buffer into chip format */
    if (u8BlockSize == sizeof(uint16_t))
    {
        for (i = 0; i < u8Length; i += sizeof(uint16_t))
        {
            uint16_t u16Short;
            memcpy(&u16Short, &pu8Buffer[i], sizeof(uint16_t));

            if (psContext->sChipDetails.eEndianness == E_CHIP_LITTLE_ENDIAN)
            {
                u16Short = htole16(u16Short);
            }
            else if (psContext->sChipDetails.eEndianness == E_CHIP_BIG_ENDIAN)
            {
                u16Short = htobe16(u16Short);
            }
            else
            {
                return E_PRG_BAD_PARAMETER;
            }
            memcpy(&pu8Buffer[i], &u16Short, sizeof(uint16_t));
        }
    }
    else if (u8BlockSize == sizeof(uint32_t))
    {
        for (i = 0; i < u8Length; i += sizeof(uint32_t))
        {
            uint32_t u32Word;
            memcpy(&u32Word, &pu8Buffer[i], sizeof(uint32_t));

            if (psContext->sChipDetails.eEndianness == E_CHIP_LITTLE_ENDIAN)
            {
                u32Word = htole32(u32Word);
            }
            else if (psContext->sChipDetails.eEndianness == E_CHIP_BIG_ENDIAN)
            {
                u32Word = htobe32(u32Word);
            }
            else
            {
                return E_PRG_BAD_PARAMETER;
            }
            memcpy(&pu8Buffer[i], &u32Word, sizeof(uint32_t));
        }
    }

    eResponse = eBL_Request(psContext, BL_TIMEOUT_250MS, E_BL_MSG_TYPE_RAM_WRITE_REQUEST, 4, au8CmdBuffer, u8Length, pu8Buffer, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_RAM_WRITE_RESPONSE);
}


/****************************************************************************
 *
 * NAME: iBL_EraseFlash
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_FlashErase(tsPRG_Context *psContext)
{
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_10S, E_BL_MSG_TYPE_FLASH_ERASE_REQUEST, 0, NULL, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_FLASH_ERASE_RESPONSE);
}


/****************************************************************************
 *
 * NAME: iBL_ReadFlash
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_FlashRead(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
    uint8_t u8RxDataLen = 0;
    uint8_t au8CmdBuffer[6];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Parameter error\n");
        return E_PRG_BAD_PARAMETER;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
    au8CmdBuffer[4] = u8Length;
    au8CmdBuffer[5] = 0;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_READ_REQUEST, 6, au8CmdBuffer, 0, NULL, &eRxType, &u8RxDataLen, pu8Buffer);

    if (u8RxDataLen != u8Length)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Requested %d bytes, got %d\n", u8Length, u8RxDataLen);
        return E_PRG_ERROR_READING;
    }

    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_FLASH_READ_RESPONSE);
}


/****************************************************************************
 *
 * NAME: iBL_WriteFlash
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus eBL_FlashWrite(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
    uint8_t au8CmdBuffer[4];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        return E_PRG_BAD_PARAMETER;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_FLASH_PROGRAM_REQUEST, 4, au8CmdBuffer, u8Length, pu8Buffer, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_FLASH_PROGRAM_RESPONSE);
}


teStatus eBL_EEPROMErase(tsPRG_Context *psContext, int iEraseAll)
{
    uint8_t au8CmdBuffer[1];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    au8CmdBuffer[0] = (uint8_t)(iEraseAll) & 0xff;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_PDM_ERASE_REQUEST, 1, au8CmdBuffer, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_PDM_ERASE_RESPONSE);
}


teStatus eBL_EEPROMRead(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
    uint8_t u8RxDataLen = 0;
    uint8_t au8CmdBuffer[6];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Parameter error\n");
        return E_PRG_BAD_PARAMETER;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
    au8CmdBuffer[4] = u8Length;
    au8CmdBuffer[5] = 0;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_EEPROM_READ_REQUEST, 6, au8CmdBuffer, 0, NULL, &eRxType, &u8RxDataLen, pu8Buffer);

    if (u8RxDataLen != u8Length)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Requested %d bytes, got %d\n", u8Length, u8RxDataLen);
        return E_PRG_ERROR_READING;
    }

    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_EEPROM_READ_RESPONSE);
}


teStatus eBL_EEPROMWrite(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
    uint8_t au8CmdBuffer[4];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Parameter error\n");
        return E_PRG_BAD_PARAMETER;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0)  & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8)  & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_EEPROM_WRITE_REQUEST, 4, au8CmdBuffer, u8Length, pu8Buffer, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_EEPROM_WRITE_RESPONSE);
}


teStatus eBL_IndexSectorWrite(tsPRG_Context *psContext, uint8_t u8Page, uint8_t u8WordNumber, uint32_t au32Data[4])
{
    uint8_t au8CmdBuffer[18];
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;
    int i;

    DBG_vPrintf(TRACE_BOOTLOADER, "Program index sector page %d, word %d, data: 0x%08X%08X%08X%08X\n",
                u8Page, u8WordNumber, au32Data[0], au32Data[1], au32Data[2], au32Data[3]);

    au8CmdBuffer[0] = u8Page;
    au8CmdBuffer[1] = u8WordNumber;

    DBG_vPrintf(TRACE_BOOTLOADER, "Device buffer:");
    for (i = 0; i < 4; i++)
    {
        uint32_t u32Word = au32Data[i];

        if (psContext->sChipDetails.eEndianness == E_CHIP_LITTLE_ENDIAN)
        {
            u32Word = htole32(u32Word);
        }
        else if (psContext->sChipDetails.eEndianness == E_CHIP_BIG_ENDIAN)
        {
            u32Word = htobe32(u32Word);
        }
        else
        {
            return E_PRG_BAD_PARAMETER;
        }
        memcpy(&au8CmdBuffer[2 + (i * sizeof(uint32_t))], &u32Word, sizeof(uint32_t));
        DBG_vPrintf(TRACE_BOOTLOADER, "%02X%02X%02X%02X",
                    au8CmdBuffer[2 + (i * sizeof(uint32_t))], au8CmdBuffer[3 + (i * sizeof(uint32_t))],
                    au8CmdBuffer[4 + (i * sizeof(uint32_t))], au8CmdBuffer[5 + (i * sizeof(uint32_t))]);
    }
    DBG_vPrintf(TRACE_BOOTLOADER, "\n");

    //return E_BL_RESPONSE_OK;
    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_PROGRAM_INDEX_SECTOR_REQUEST, 18, au8CmdBuffer, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_PROGRAM_INDEX_SECTOR_RESPONSE);
}

teStatus eBL_Reset(tsPRG_Context *psContext)
{
    teBL_Response eResponse = 0;
    teBL_MessageType eRxType = 0;

    eResponse = eBL_Request(psContext, BL_TIMEOUT_1S, E_BL_MSG_TYPE_RESET_REQUEST, 0, NULL, 0, NULL, &eRxType, NULL, NULL);
    return eBL_CheckResponse(__FUNCTION__, eResponse, eRxType, E_BL_MSG_TYPE_RESET_RESPONSE);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: eBL_Request
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
static teBL_Response eBL_Request(tsPRG_Context *psContext, int iTimeoutMicroseconds, teBL_MessageType eTxType, uint8_t u8HeaderLen, uint8_t *pu8Header, uint8_t u8TxLength, uint8_t *pu8TxData,
                          teBL_MessageType *peRxType, uint8_t *pu8RxLength, uint8_t *pu8RxData)
{
    /* Check data is not too long */
    if(u8TxLength > 0xfd)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Data too long\n");
        return E_BL_RESPONSE_ERROR;
    }

    /* Send message */
    if(eBL_WriteMessage(psContext, eTxType, u8HeaderLen, pu8Header, u8TxLength, pu8TxData) != E_PRG_OK)
    {
        return E_BL_RESPONSE_ERROR;
    }

    /* Get the response to the request */
    return eBL_ReadMessage(psContext, iTimeoutMicroseconds, peRxType, pu8RxLength, pu8RxData);
}


/****************************************************************************
 *
 * NAME: iBL_WriteMessage
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occured
 *
 ****************************************************************************/
static teStatus eBL_WriteMessage(tsPRG_Context *psContext, teBL_MessageType eType, uint8_t u8HeaderLength, uint8_t *pu8Header, uint8_t u8Length, uint8_t *pu8Data)
{
    uint8_t u8CheckSum = 0;
    int n;

    uint8_t au8Msg[256];

    /* total message length cannot be > 255 bytes */
    if(u8HeaderLength + u8Length >= 0xfe)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Length too big\n");
        return E_PRG_BAD_PARAMETER;
    }

    /* Message length */
    au8Msg[0] = u8HeaderLength + u8Length + 2;

    /* Message type */
    au8Msg[1] = (uint8_t)eType;

    /* Message header */
    memcpy(&au8Msg[2], pu8Header, u8HeaderLength);

    /* Message payload */
    memcpy(&au8Msg[2 + u8HeaderLength], pu8Data, u8Length);

    DBG_vPrintf(TRACE_BOOTLOADER, "Tx: ");
    for(n = 0; n < u8HeaderLength + u8Length + 2; n++)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "%02x ", au8Msg[n]);
        u8CheckSum ^= au8Msg[n];
    }
    DBG_vPrintf(TRACE_BOOTLOADER, "\n");

    /* Message checksum */
    au8Msg[u8HeaderLength + u8Length + 2] = u8CheckSum;

    /* Write whole message to UART */
    return eUART_Write(psContext->iUartFD, au8Msg, u8HeaderLength + u8Length + 3);
}


/****************************************************************************
 *
 * NAME: eBL_ReadMessage
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
static teBL_Response eBL_ReadMessage(tsPRG_Context *psContext, int iTimeoutMicroseconds, teBL_MessageType *peType, uint8_t *pu8Length, uint8_t *pu8Data)
{

    int n;
    teStatus eStatus;
    uint8_t au8Msg[BOOTLOADER_MAX_MESSAGE_LENGTH];
    uint8_t u8CalculatedCheckSum = 0;
    uint8_t u8Length = 0;
    teBL_Response eResponse = E_BL_RESPONSE_OK;
    int iAttempts = 0;
    int iBytesRead = 0;
    int iTotalBytesRead = 0;

    /* Get the length byte */
    if((eUART_Read(psContext->iUartFD, iTimeoutMicroseconds, 1, &u8Length, &iBytesRead) != E_PRG_OK) || (iBytesRead != 1))
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Error getting length\n");
        return E_BL_RESPONSE_NO_RESPONSE;
    }

    //DBG_vPrintf(TRACE_BOOTLOADER, "Incoming message length %d\n", u8Length);

    /* Message must have at least 3 bytes, maximum is implicit */
    if (u8Length < 3)
    {
        return E_BL_RESPONSE_NO_RESPONSE;
    }

    /* Add length to checksum */
    u8CalculatedCheckSum ^= u8Length;

    do
    {
        /* Get the rest of the message */
        eStatus = eUART_Read(psContext->iUartFD, iTimeoutMicroseconds, u8Length - iTotalBytesRead, &au8Msg[iTotalBytesRead], &iBytesRead);
        if(eStatus != E_PRG_OK)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "Error reading message from UART\n");
            return E_BL_RESPONSE_NO_RESPONSE;
        }

        iTotalBytesRead += iBytesRead;
        iAttempts++;

    } while ((iTotalBytesRead < u8Length) && (iBytesRead > 0 || iAttempts < 10));

    if(iTotalBytesRead != u8Length)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Got %d bytes but expected %d after %d attempts: ", iTotalBytesRead, u8Length, iAttempts);
        for(n = 0; n < iTotalBytesRead; n++)
        {
            DBG_vPrintf(TRACE_BOOTLOADER, "%02x ", au8Msg[n]);
        }
        DBG_vPrintf(TRACE_BOOTLOADER, "\n");

        return E_BL_RESPONSE_NO_RESPONSE;
    }

    /* Add rest of message to checksum */
    DBG_vPrintf(TRACE_BOOTLOADER, "Rx: %02x ", u8Length);
    for(n = 0; n < u8Length; n++)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "%02x ", au8Msg[n]);
        u8CalculatedCheckSum ^= au8Msg[n];
    }
    DBG_vPrintf(TRACE_BOOTLOADER, "\n");

    if(u8CalculatedCheckSum != 0x00)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "Checksum bad, got %02x expected %02x\n", u8CalculatedCheckSum, 0);
        return E_BL_RESPONSE_CRC_ERROR;
    }
    
    // dump( (char *)au8Msg, u8Length );

    *peType = au8Msg[0];
    eResponse = au8Msg[1];
    if (pu8Length)
    {
        *pu8Length = u8Length - 3;

        if (pu8Data)
        {
            memcpy(pu8Data, &au8Msg[2], *pu8Length);
        }
    }

    DBG_vPrintf(TRACE_BOOTLOADER, "Got response 0x%02x\n", eResponse);

    return eResponse;
}


static teStatus eBL_CheckResponse(const char *pcFunction, teBL_Response eResponse, teBL_MessageType eRxType, teBL_MessageType eExpectedRxType)
{
    DBG_vPrintf(TRACE_BOOTLOADER, "%s: Response %02x\n", pcFunction, eResponse);
    switch (eResponse)
    {
        case(E_BL_RESPONSE_OK):
            break;
        case (E_BL_RESPONSE_NOT_SUPPORTED):
            return E_PRG_UNSUPPORTED_OPERATION;
        case (E_BL_RESPONSE_WRITE_FAIL):
            return E_PRG_ERROR_WRITING;
        case (E_BL_RESPONSE_READ_FAIL):
            return E_PRG_ERROR_READING;
        default:
            return E_PRG_COMMS_FAILED;
    }

    if (eRxType != eExpectedRxType)
    {
        DBG_vPrintf(TRACE_BOOTLOADER, "%s: Got type 0x%02x, expected 0x%02x\n", pcFunction, eRxType, eExpectedRxType);
        return E_PRG_COMMS_FAILED;
    }
    return E_PRG_OK;
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

