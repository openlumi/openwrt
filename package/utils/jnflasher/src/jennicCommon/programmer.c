
/****************************************************************************
 *
 * MODULE:             JN51xx Programmer
 *
 * COMPONENT:          Main programmer functions
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2009/03/02 13:33:44 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Matt Redfearn
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
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#if defined WIN32
#include <windows.h>
#else
//#include <arpa/inet.h>
#endif /* WIN32 */

#include <programmer.h>

#include "ChipID.h"
#include "JN51xx_BootLoader.h"
#include "uart.h"
#include "dbg.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_PROGRAMMER
#define TRACE_PROGRAMMER    TRUE
#else
#define TRACE_PROGRAMMER    FALSE
#endif

#ifndef O_BINARY
# ifdef _O_BINARY
# define O_BINARY _O_BINARY
# else
# define O_BINARY 0
# endif
#endif

#define BL_MAX_CHUNK_SIZE   248

#define RSTCTRL_REGISTER_ADDRESS                0x0200104C
#define RSTCTRL_CPU_REBOOT_MASK                 (1 << 1)

/* JN513x / JN514x definitions */
#define JN514X_ROM_ID_ADDR                      0x00000004
#define JN514X_MAC_ADDRESS_LOCATION             0x00000030
#define JN514X_MIB_MAC_ADDRESS_LOCATION         0x00000010


/* JN516x definitions */

/* Location of bootloader information in memory map */
#define JN516X_BOOTLOADER_VERSION_ADDRESS               0x00000062
#define JN516X_BOOTLOADER_ENTRY                         0x00000066

/* JN517x definitions */

/* Location of bootloader information in memory map */
#define JN517X_BOOTLOADER_VERSION_ADDRESS               0x00000044
// this address not needed for this implementation
#define JN517X_BOOTLOADER_ENTRY                         0xFFFFFFFF

/** Inline function to convert index sector page / word into memory mapped address */
static inline uint32_t u32PRG_JN516x_index_sector_address(uint8_t u8Page, uint8_t u8Word)
{
    uint32_t u32Address = 0x01001000 + ((uint32_t)u8Page << 8) + ((uint32_t)u8Word << 4);

    DBG_vPrintf(TRACE_PROGRAMMER, "u32PRG_JN516x_index_sector_address = 0x%x\n", u32Address);
    return 0x01001000 + ((uint32_t)u8Page << 8) + ((uint32_t)u8Word << 4);
}

/** Index sector word length in bytes */
#define IP2111_INDEX_SECTOR_WORD_LENGTH                 16

/* IP2111 configuration index sector word */
#define JN516X_INDEX_SECTOR_IP2111_CONFIG_PAGE          4
#define JN516X_INDEX_SECTOR_IP2111_CONFIG_WORD          0
#define JN516X_INDEX_SECTOR_IP2111_CONFIG_ADDRESS       u32PRG_JN516x_index_sector_address(JN516X_INDEX_SECTOR_IP2111_CONFIG_PAGE, JN516X_INDEX_SECTOR_IP2111_CONFIG_WORD)


/* ATE device settings flash index sector word */
#define JN516X_INDEX_SECTOR_ATE_SETTINGS_PAGE           5
#define JN516X_INDEX_SECTOR_ATE_SETTINGS_WORD           0
#define JN516X_INDEX_SECTOR_ATE_SETTINGS_ADDRESS        u32PRG_JN516x_index_sector_address(JN516X_INDEX_SECTOR_ATE_SETTINGS_PAGE, JN516X_INDEX_SECTOR_ATE_SETTINGS_WORD)

/* Customer configuration flash index sector word */
#define JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_PAGE        5
#define JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_WORD        1
#define JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_ADDRESS     u32PRG_JN516x_index_sector_address(JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_PAGE, JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_WORD)

/* Customer user data 0 flash index sector word */
#define JN516X_INDEX_SECTOR_USER0_PAGE                  5
#define JN516X_INDEX_SECTOR_USER0_WORD                  4
#define JN516X_INDEX_SECTOR_USER0_ADDRESS               u32PRG_JN516x_index_sector_address(JN516X_INDEX_SECTOR_USER0_PAGE, JN516X_INDEX_SECTOR_USER0_WORD)

/* Customer user data 1 flash index sector word */
#define JN516X_INDEX_SECTOR_USER1_PAGE                  5
#define JN516X_INDEX_SECTOR_USER1_WORD                  5
#define JN516X_INDEX_SECTOR_USER1_ADDRESS               u32PRG_JN516x_index_sector_address(JN516X_INDEX_SECTOR_USER1_PAGE, JN516X_INDEX_SECTOR_USER1_WORD)

/* Customer user data 2 flash index sector word */
#define JN516X_INDEX_SECTOR_USER2_PAGE                  5
#define JN516X_INDEX_SECTOR_USER2_WORD                  6
#define JN516X_INDEX_SECTOR_USER2_ADDRESS               u32PRG_JN516x_index_sector_address(JN516X_INDEX_SECTOR_USER2_PAGE, JN516X_INDEX_SECTOR_USER2_WORD)

/* Cutomer MAC address flash index sector word */
#define JN516X_INDEX_SECTOR_MAC_CUSTOMER_PAGE           5
#define JN516X_INDEX_SECTOR_MAC_CUSTOMER_WORD           7
#define JN516X_INDEX_SECTOR_MAC_CUSTOMER_ADDRESS        u32PRG_JN516x_index_sector_address(JN516X_INDEX_SECTOR_MAC_CUSTOMER_PAGE, JN516X_INDEX_SECTOR_MAC_CUSTOMER_WORD)

/* Factory MAC address flash index sector word */
#define JN516X_INDEX_SECTOR_MAC_FACTORY_PAGE            5
#define JN516X_INDEX_SECTOR_MAC_FACTORY_WORD            8
#define JN516X_INDEX_SECTOR_MAC_FACTORY_ADDRESS         u32PRG_JN516x_index_sector_address(JN516X_INDEX_SECTOR_MAC_FACTORY_PAGE, JN516X_INDEX_SECTOR_MAC_FACTORY_WORD)

/* AES Key flash index sector word */
#define JN516X_INDEX_SECTOR_AES_KEY_PAGE                5
#define JN516X_INDEX_SECTOR_AES_KEY_WORD                12
#define JN516X_INDEX_SECTOR_AES_KEY_ADDRESS             u32PRG_JN516x_index_sector_address(JN516X_INDEX_SECTOR_AES_KEY_PAGE, JN516X_INDEX_SECTOR_AES_KEY_WORD)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

static teStatus ePRG_DeviceConfigGet(tsPRG_Context *psContext);
static teStatus ePRG_ChipGetChipId(tsPRG_Context *psContext);
static teStatus ePRG_ChipGetFlashId(tsPRG_Context *psContext);
static teStatus ePRG_ChipGetMacAddress(tsPRG_Context *psContext);
static teStatus ePRG_ChipGetEEPROMenable(tsPRG_Context *psContext);
static teStatus ePRG_ChipGetFlashProgrammerExtensionDetails(tsPRG_Context *psContext);
static teStatus ePRG_FlashProgrammerExtensionLoad(tsPRG_Context *psContext, const char *pcOperation);
static teStatus ePRG_FlashProgrammerExtensionReturn(tsPRG_Context *psContext);
static teStatus ePRG_SetUpImage(tsPRG_Context *psContext, tsFW_Info *psFWImage, tsChipDetails *psChipDetails);
static teStatus ePRG_ConfirmAlways(void *pvUser, const char *pcTitle, const char *pcText);
#if 0
static teStatus ePRG_ResetDevice(tsPRG_Context *psContext);
#endif

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/** Version string for libprogrammer */
#ifndef VERSION_MAJOR
#error Major Version is not defined!
#else
#ifndef VERSION_MINOR
#error Minor Version is not defined!
#else
#ifndef VERSION_SVN
#error SVN Version is not defined!
#else
const char *pcPRG_Version = VERSION_MAJOR "." VERSION_MINOR " (r" VERSION_SVN ")";
#endif
#endif
#endif

/** Import binary data from FlashProgrammerExtension_JN5168.bin */
#if defined POSIX
extern int _binary_FlashProgrammerExtension_JN5168_bin_start;
extern int _binary_FlashProgrammerExtension_JN5168_bin_size;
#define FLASHPROGRAMMEREXTENSION_JN5168_BIN     ((uint8_t *)    &_binary_FlashProgrammerExtension_JN5168_bin_start)
#ifdef __amd64__
#define FLASHPROGRAMMEREXTENSION_JN5168_BIN_LEN ((uint64_t)     &_binary_FlashProgrammerExtension_JN5168_bin_size)
#else
#define FLASHPROGRAMMEREXTENSION_JN5168_BIN_LEN ((uint32_t)     &_binary_FlashProgrammerExtension_JN5168_bin_size)
#endif
#elif defined WIN32
extern int binary_FlashProgrammerExtension_JN5168_bin_start;
extern int binary_FlashProgrammerExtension_JN5168_bin_size;
#define FLASHPROGRAMMEREXTENSION_JN5168_BIN     ((uint8_t *)    &binary_FlashProgrammerExtension_JN5168_bin_start)
#define FLASHPROGRAMMEREXTENSION_JN5168_BIN_LEN ((uint32_t)     &binary_FlashProgrammerExtension_JN5168_bin_size)
#endif

/** Import binary data from FlashProgrammerExtension_JN5169.bin */
#if defined POSIX
extern int _binary_FlashProgrammerExtension_JN5169_bin_start;
extern int _binary_FlashProgrammerExtension_JN5169_bin_size;
#define FLASHPROGRAMMEREXTENSION_JN5169_BIN     ((uint8_t *)    &_binary_FlashProgrammerExtension_JN5169_bin_start)
#ifdef __amd64__
#define FLASHPROGRAMMEREXTENSION_JN5169_BIN_LEN ((uint64_t)     &_binary_FlashProgrammerExtension_JN5169_bin_size)
#else
#define FLASHPROGRAMMEREXTENSION_JN5169_BIN_LEN ((uint32_t)     &_binary_FlashProgrammerExtension_JN5169_bin_size)
#endif
#elif defined WIN32
extern int binary_FlashProgrammerExtension_JN5169_bin_start;
extern int binary_FlashProgrammerExtension_JN5169_bin_size;
#define FLASHPROGRAMMEREXTENSION_JN5169_BIN     ((uint8_t *)    &binary_FlashProgrammerExtension_JN5169_bin_start)
#define FLASHPROGRAMMEREXTENSION_JN5169_BIN_LEN ((uint32_t)     &binary_FlashProgrammerExtension_JN5169_bin_size)
#endif

/** Import binary data from FlashProgrammerExtension_JN5179.bin */
#if defined POSIX
extern int _binary_FlashProgrammerExtension_JN5179_bin_start;
extern int _binary_FlashProgrammerExtension_JN5179_bin_size;
#define FLASHPROGRAMMEREXTENSION_JN5179_BIN     ((uint8_t *)    &_binary_FlashProgrammerExtension_JN5179_bin_start)
#ifdef __amd64__
#define FLASHPROGRAMMEREXTENSION_JN5179_BIN_LEN ((uint64_t)     &_binary_FlashProgrammerExtension_JN5179_bin_size)
#else
#define FLASHPROGRAMMEREXTENSION_JN5179_BIN_LEN ((uint32_t)     &_binary_FlashProgrammerExtension_JN5179_bin_size)
#endif
#elif defined WIN32
extern int binary_FlashProgrammerExtension_JN5179_bin_start;
extern int binary_FlashProgrammerExtension_JN5179_bin_size;
#define FLASHPROGRAMMEREXTENSION_JN5179_BIN     ((uint8_t *)    &binary_FlashProgrammerExtension_JN5179_bin_start)
#define FLASHPROGRAMMEREXTENSION_JN5179_BIN_LEN ((uint32_t)     &binary_FlashProgrammerExtension_JN5179_bin_size)
#endif

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/




#if 0
teStatus LIBPROGRAMMER ePRG_Init(tsPRG_Context *psContext)
{
    memset(psContext, 0, sizeof(tsPRG_Context));

    psContext->pvPrivate = malloc(sizeof(tsPRG_PrivateContext));

    if (!psContext->pvPrivate)
    {
        return E_PRG_OUT_OF_MEMORY;
    }

    memset(psContext->pvPrivate, 0, sizeof(tsPRG_PrivateContext));

    /* Default is to automatically stobe program / reset lines is possible */
    psContext->sFlags.bAutoProgramReset = 1;

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}

teStatus LIBPROGRAMMER ePRG_Destroy(tsPRG_Context *psContext)
{
    ePRG_FwClose(psContext);

    ePRG_ConnectionClose(psContext);

    free(psContext->sChipDetails.asFlashes);
    free(psContext->pvPrivate);
    return E_PRG_OK;
}

char *LIBPROGRAMMER pcPRG_GetLastStatusMessage(tsPRG_Context *psContext)
{
    tsPRG_PrivateContext *psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;

    if (psPriv)
    {
        return psPriv->acStatusMessage;
    }
    return NULL;
}

teStatus LIBPROGRAMMER ePRG_ConnectionOpen(tsPRG_Context *psContext, tsConnection *psConnection)
{
    if ((!psContext) || (!psConnection))
    {
        return E_PRG_NULL_PARAMETER;
    }

    /* Close any existing connection */
    ePRG_ConnectionClose(psContext);

    switch(psConnection->eType)
    {
        case (E_CONNECT_SERIAL):
            return ePRG_ConnectionUartOpen(psContext, psConnection);

        default:
            return E_PRG_INVALID_TRANSPORT;
    }
}

teStatus LIBPROGRAMMER ePRG_ConnectionClose(tsPRG_Context *psContext)
{
    teStatus eStatus;
    tsPRG_PrivateContext *psPriv;
    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;

    switch(psPriv->sConnection.eType)
    {
        case (E_CONNECT_NONE):
            eStatus = E_PRG_OK;
            break;

        case (E_CONNECT_SERIAL):
            if (psContext->sFlags.bAutoProgramReset)
            {
                ePRG_ResetDevice(psContext);
            }
            eStatus = ePRG_ConnectionUartClose(psContext);
            break;

        default:
            eStatus = E_PRG_INVALID_TRANSPORT;
            break;
    }

    psPriv->sConnection.eType = E_CONNECT_NONE;
    return eStatus;
}

teStatus ePRG_ConnectionUpdate(tsPRG_Context *psContext, tsConnection *psConnection)
{
    tsPRG_PrivateContext *psPriv;
    teStatus eStatus;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psPriv = (tsPRG_PrivateContext *)psContext->pvPrivate;

    if (psPriv->sConnection.eType != E_CONNECT_SERIAL)
    {
        // It is invalid to attempt to change baudrate if we're not using the serial transport.
        return ePRG_SetStatus(psContext, E_PRG_INVALID_TRANSPORT, "cannot change baud rate on non-serial transport");
    }

    /* Change bootloader to new speed */
    if ((eStatus = eBL_SetBaudrate(psContext, psConnection->uDetails.sSerial.u32BaudRate)) != E_PRG_OK)
    {
        switch (CHIP_ID_PART(psContext->sChipDetails.u32ChipId))
        {
            /* These 4x bootloaders corrupt the CRC byte on the change baud response so are expected to fail */
            case (CHIP_ID_PART(CHIP_ID_JN5148_REV2A)):
            case (CHIP_ID_PART(CHIP_ID_JN5142_REV1A)):
            case (CHIP_ID_PART(CHIP_ID_JN5142_REV1B)):
            case (CHIP_ID_PART(CHIP_ID_JN5142_REV1C)):
                DBG_vPrintf(TRACE_PROGRAMMER, "Expected CRC fail\n");
                break;
            default:
                DBG_vPrintf(TRACE_PROGRAMMER, "Error selecting baud rate\n");
                return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
    }

    /* change local port to new speed */
    if ((eStatus = ePRG_ConnectionUartUpdate(psContext, psConnection)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
    }

    psPriv->sConnection.uDetails = psConnection->uDetails;

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}
#endif


teStatus ePRG_ChipGetDetails(tsPRG_Context *psContext)
{
    teStatus eStatus;

    if ((eStatus = eUART_Flush(psContext->iUartFD)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "flushing UART");
    }
    DBG_vPrintf(TRACE_PROGRAMMER, "Flushing UART OK\n");

    if ((eStatus = ePRG_ChipGetChipId(psContext)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "reading chip ID");
    }
    DBG_vPrintf(TRACE_PROGRAMMER, "Read CHIP_ID OK\n");

    if ((eStatus = ePRG_ChipGetFlashId(psContext)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "reading flash ID");
    }
    DBG_vPrintf(TRACE_PROGRAMMER, "Read FLASH_ID OK\n");

    if ((eStatus = ePRG_ChipGetMacAddress(psContext)) != E_PRG_OK)
    {
        ePRG_SetStatus(psContext, eStatus, "reading MAC address");
        DBG_vPrintf(TRACE_PROGRAMMER, "Failed to read MAC %s\n", pcPRG_GetLastErrorMessage(psContext));
    }
    else
    {
        DBG_vPrintf(TRACE_PROGRAMMER, "Read MAC OK\n");
    }

    if ((eStatus = ePRG_DeviceConfigGet(psContext)) != E_PRG_OK)
    {
        ePRG_SetStatus(psContext, eStatus, "reading device configuration");
        DBG_vPrintf(TRACE_PROGRAMMER, "Failed to read DCON %s\n", pcPRG_GetLastErrorMessage(psContext));
    }
    else
    {
        DBG_vPrintf(TRACE_PROGRAMMER, "Read DCON OK\n");
    }

    if ((eStatus = ePRG_ChipGetEEPROMenable(psContext)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "getting EEPROM availability");
    }
    DBG_vPrintf(TRACE_PROGRAMMER, "Read EEPROM availability OK\n");

    if ((eStatus = ePRG_ChipGetFlashProgrammerExtensionDetails(psContext)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "reading Flash Programmer Extension details");
    }
    DBG_vPrintf(TRACE_PROGRAMMER, "Read FPE OK\n");

    // Always select flash 0 to start off with.
    return ePRG_FlashSelectDevice(psContext, 0);
}


teStatus LIBPROGRAMMER ePRG_FlashAddDevice(tsPRG_Context *psContext, const char *pcName, const uint32_t u32FlashSize,
                                           uint8_t u8ManufacturerID, const uint8_t u8DeviceID, const uint8_t u8ChipSelect)
{
    tsFlashDetails *psFlashDetails;

    if ((!psContext) || (!pcName))
    {
        return E_PRG_NULL_PARAMETER;
    }

    /** Reallocate storage */

    psFlashDetails = realloc(psContext->sChipDetails.asFlashes, sizeof(tsFlashDetails) * (psContext->sChipDetails.u32NumFlashes + 1));
    if (!psFlashDetails)
    {
        return ePRG_SetStatus(psContext, E_PRG_OUT_OF_MEMORY, "");
    }

    psContext->sChipDetails.asFlashes = psFlashDetails;

    psFlashDetails[psContext->sChipDetails.u32NumFlashes].pcFlashName       = pcName;
    psFlashDetails[psContext->sChipDetails.u32NumFlashes].u32FlashSize      = u32FlashSize;
    psFlashDetails[psContext->sChipDetails.u32NumFlashes].u8ManufacturerID  = u8ManufacturerID;
    psFlashDetails[psContext->sChipDetails.u32NumFlashes].u8DeviceID        = u8DeviceID;
    psFlashDetails[psContext->sChipDetails.u32NumFlashes].u8ChipSelect      = u8ChipSelect;

    DBG_vPrintf(TRACE_PROGRAMMER, "Added flash %d: \"%s\" size %dk (0x%02X:0x%02X on CS %d)\n",
                psContext->sChipDetails.u32NumFlashes, pcName, u32FlashSize / 1024,
                u8ManufacturerID, u8DeviceID, u8ChipSelect);

    psContext->sChipDetails.u32NumFlashes++;

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


teStatus LIBPROGRAMMER ePRG_FlashSelectDevice(tsPRG_Context *psContext, uint32_t u32FlashDevice)
{
    tsFlashDetails *psFlashDevice;
    teStatus eStatus;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    /* Select flash & enable write access */
    if (u32FlashDevice >= psContext->sChipDetails.u32NumFlashes)
    {
        /* Out of bounds flash device */
        DBG_vPrintf(TRACE_PROGRAMMER, "Flash %d out of bounds\n", u32FlashDevice);
        return ePRG_SetStatus(psContext, E_PRG_FLASH_DEVICE_UNAVAILABLE, "");
    }

    psFlashDevice = &psContext->sChipDetails.asFlashes[u32FlashDevice];
#if 0
    /* Set the flash type */
    DBG_vPrintf(TRACE_PROGRAMMER, "psFlashDevice->u8ManufacturerID = 0x%x\n", psFlashDevice->u8ManufacturerID);
    DBG_vPrintf(TRACE_PROGRAMMER, "psFlashDevice->pcFlashName = %s\n", psFlashDevice->pcFlashName);
    DBG_vPrintf(TRACE_PROGRAMMER, "psFlashDevice->u32FlashSize = 0x%x\n", psFlashDevice->u32FlashSize);
    DBG_vPrintf(TRACE_PROGRAMMER, "psFlashDevice->u8ChipSelect = %d\n", psFlashDevice->u8ChipSelect);
    DBG_vPrintf(TRACE_PROGRAMMER, "psFlashDevice->u8DeviceID = 0x%x\n", psFlashDevice->u8DeviceID);
#endif
    if((eStatus = eBL_FlashSelectDevice(psContext, psFlashDevice->u8ManufacturerID, psFlashDevice->u8DeviceID, psFlashDevice->u8ChipSelect)) != E_PRG_OK)
    {
        DBG_vPrintf(TRACE_PROGRAMMER, "Error selecting flash device\n");
        return ePRG_SetStatus(psContext, eStatus, "selecting flash type");
    }

    /* If its not internal flash, we need to enable write access */
    if(!((psFlashDevice->u8ManufacturerID == FLASH_MANUFACTURER_JN516X) && (psFlashDevice->u8DeviceID == FLASH_DEVICE_JN516X)))
    {
        if((eStatus = eBL_FlashStatusRegisterWrite(psContext, 0x00)) != E_PRG_OK)
        {
            DBG_vPrintf(TRACE_PROGRAMMER, "Error enabling write access\n");
            return ePRG_SetStatus(psContext, eStatus, "writing to flash status register");
        }
    }

    psContext->u32SelectedFlash = u32FlashDevice;
    DBG_vPrintf(TRACE_PROGRAMMER, "Flash device %d selected\n", u32FlashDevice);
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


teStatus ePRG_FlashErase(tsPRG_Context *psContext, tcbFW_Progress cbProgress, void *pvUser)
{
    teStatus eStatus;
#if !(defined JN5172_FPGA)
    uint8_t au8Buffer1[BL_MAX_CHUNK_SIZE + 1];
    uint8_t au8Buffer2[BL_MAX_CHUNK_SIZE + 1];
#endif
    char acOperationText[256];

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    snprintf(acOperationText, sizeof(acOperationText), "Erasing %s", psContext->sChipDetails.asFlashes[psContext->u32SelectedFlash].pcFlashName);

    /* Erase the flash memory */
    if (cbProgress)
    {
        if (cbProgress(pvUser, acOperationText, "Erasing", 0, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }
    if((eStatus = eBL_FlashErase(psContext)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "erasing flash");
    }

#if !(defined JN5172_FPGA)
    if ((psContext->sDeviceConfig.eCRP == E_DC_CRP_DEFAULT) ||
        (psContext->sDeviceConfig.eCRP == E_DC_CRP_LEVEL0))
    {
        /* Ensure that flash is erased */
        DBG_vPrintf(TRACE_PROGRAMMER, "Checking flash is blank...\n");
        memset(au8Buffer2, 0xFF, 64);

        if ((eStatus = eBL_FlashRead(psContext, 0, 64, au8Buffer1)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "reading Flash at address 0x%08X", 0);
        }
        else
        {
            if (memcmp(au8Buffer1, au8Buffer2, 64))
            {
                return ePRG_SetStatus(psContext, E_PRG_ERROR, "flash erase unsuccessful");
            }
            else
            {
                if (cbProgress)
                {
                    if (cbProgress(pvUser, acOperationText, "Erasing", 1, 1) != E_PRG_OK)
                    {
                        return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
                    }
                }
            }
        }
    }
    else
    {
        DBG_vPrintf(TRACE_PROGRAMMER, "CRP set: Cannot check flash is erased on this device\n");
    }
#else
    DBG_vPrintf(TRACE_PROGRAMMER, "FPGA: No erase flash functionality on this device - this is ok \n");
#endif

    return ePRG_SetStatus(psContext, E_PRG_OK, "flash erased succesfully");
}


teStatus ePRG_FlashProgram(tsPRG_Context *psContext, tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    teStatus eStatus;
    int n;
    uint8_t u8ChunkSize;
    tsChipDetails *psChipDetails;
    tsFW_Info *psFWImage;
    char acOperationText[256];

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;
    psFWImage = &psContext->sFirmwareInfo;
    
    DBG_vPrintf(TRACE_PROGRAMMER, "ROM version 0x%08x, Supported 0x%08x\n", psFWImage->u32ROMVersion, psChipDetails->u32SupportedFirmware );

    if (memcmp(&psFWImage->u32ROMVersion, &psChipDetails->u32SupportedFirmware, 4) != 0)
    {
        eStatus = E_PRG_INCOMPATIBLE;
        if (psFWImage->sFlags.bRawImage)
        {
            if (cbConfirm)
            {
                eStatus = cbConfirm(pvUser, "Confirm Operation", "The loaded file does not appear to be a valid image. Program raw?");
            }
            if (eStatus != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_INCOMPATIBLE, "not a valid image");
            }
        }
        else
        {
            if (cbConfirm)
            {
                eStatus = cbConfirm(pvUser, "Confirm Operation", "The loaded image does not appear to be compatible with the connected device. Continue?");
            }
            if (eStatus != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_INCOMPATIBLE, "image built for 0x%08X, connected device is 0x%08x", psFWImage->u32ROMVersion, psChipDetails->u32SupportedFirmware);
            }
        }
        // User has given the go ahead to continue anyway. Here be dragons.
    }

    if ((eStatus = ePRG_SetUpImage(psContext, psFWImage, psChipDetails)) != E_PRG_OK)
    {
        return eStatus;
    }

    DBG_vPrintf(TRACE_PROGRAMMER, "Programming image length %d into flash length %d at offset 0x%08x\n", psFWImage->u32ImageLength, psContext->sChipDetails.asFlashes[psContext->u32SelectedFlash].u32FlashSize, psContext->u32FlashOffset);
    if ((psContext->u32FlashOffset + psFWImage->u32ImageLength) > psContext->sChipDetails.asFlashes[psContext->u32SelectedFlash].u32FlashSize)
    {
        return ePRG_SetStatus(psContext, E_PRG_INCOMPATIBLE, "image is larger than selected flash device.");
    }

    snprintf(acOperationText, sizeof(acOperationText), "Programming %s", psContext->sChipDetails.asFlashes[psContext->u32SelectedFlash].pcFlashName);

    if ((eStatus = ePRG_FlashErase(psContext, cbProgress, pvUser)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "erasing flash");
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, acOperationText, "Writing", 0, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    for(n = 0; n < psFWImage->u32ImageLength; n += u8ChunkSize)
    {
        if((psFWImage->u32ImageLength - n) > 128)
        {
            u8ChunkSize = 128;
        }
        else
        {
            u8ChunkSize = psFWImage->u32ImageLength - n;
        }

        if((eStatus = eBL_FlashWrite(psContext, psContext->u32FlashOffset + n, u8ChunkSize, psFWImage->pu8ImageData + n)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "writing flash at address 0x%08X", psContext->u32FlashOffset + n);
        }

        if (cbProgress)
        {
            if (cbProgress(pvUser, acOperationText, "Writing", psFWImage->u32ImageLength, n) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }
    
    if (cbProgress)
    {
        if (cbProgress(pvUser, acOperationText, "Writing", psFWImage->u32ImageLength, psFWImage->u32ImageLength) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "flash written succesfully");
}


teStatus ePRG_FlashVerify(tsPRG_Context *psContext, tcbFW_Progress cbProgress, void *pvUser)
{
    teStatus eStatus;
    int n;
    uint8_t u8ChunkSize;
    uint8_t au8Buffer1[BL_MAX_CHUNK_SIZE + 1];
    tsChipDetails *psChipDetails;
    tsFW_Info *psFWImage;
    char acOperationText[256];

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    if (psContext->sDeviceConfig.eCRP == E_DC_CRP_LEVEL1)
    {
        return ePRG_SetStatus(psContext, E_PRG_CRP_SET, "verifying device");
    }

    snprintf(acOperationText, sizeof(acOperationText), "Verifying %s", psContext->sChipDetails.asFlashes[psContext->u32SelectedFlash].pcFlashName);

    psChipDetails = &psContext->sChipDetails;
    psFWImage = &psContext->sFirmwareInfo;

    DBG_vPrintf(TRACE_PROGRAMMER, "Verifying image length %d into flash length %d at offset 0x%08x\n", psFWImage->u32ImageLength, psContext->sChipDetails.asFlashes[psContext->u32SelectedFlash].u32FlashSize, psContext->u32FlashOffset);
    if ((psContext->u32FlashOffset + psFWImage->u32ImageLength) > psContext->sChipDetails.asFlashes[psContext->u32SelectedFlash].u32FlashSize)
    {
        return ePRG_SetStatus(psContext, E_PRG_INCOMPATIBLE, "image is larger than selected flash device.");
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, acOperationText, "Verifying", 0, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    if ((eStatus = ePRG_SetUpImage(psContext, psFWImage, psChipDetails)) != E_PRG_OK)
    {
        return eStatus;
    }

    for(n = 0; n < psFWImage->u32ImageLength; n += u8ChunkSize)
    {
        if((psFWImage->u32ImageLength - n) > 128)
        {
            u8ChunkSize = 128;
        }
        else
        {
            u8ChunkSize = psFWImage->u32ImageLength - n;
        }

        if ((eStatus = eBL_FlashRead(psContext, psContext->u32FlashOffset + n, u8ChunkSize, au8Buffer1)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "reading Flash at address 0x%08X", psContext->u32FlashOffset + n);
        }
        else
        {
            if (memcmp(psFWImage->pu8ImageData + n, au8Buffer1, u8ChunkSize))
            {
                return ePRG_SetStatus(psContext, E_PRG_VERIFICATION_FAILED, "at address 0x%08X", psContext->u32FlashOffset + n);
            }
        }

        if (cbProgress)
        {
            if (cbProgress(pvUser, acOperationText, "Verifying", psFWImage->u32ImageLength, n) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, acOperationText, "Verifying", psFWImage->u32ImageLength, psFWImage->u32ImageLength) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "flash verified succesfully");
}


teStatus LIBPROGRAMMER ePRG_FlashDump(tsPRG_Context *psContext, char *pcDumpFile, tcbFW_Progress cbProgress, void *pvUser)
{
    int iFd;
    int n;
    uint8_t u8ChunkSize = 128;
    uint32_t u32FlashSize;
    char acOperationText[256];

    if (!psContext || !pcDumpFile)
    {
        return E_PRG_NULL_PARAMETER;
    }

    if (psContext->sDeviceConfig.eCRP == E_DC_CRP_LEVEL1)
    {
        return ePRG_SetStatus(psContext, E_PRG_CRP_SET, "dumping flash");
    }

    u32FlashSize = psContext->sChipDetails.asFlashes[psContext->u32SelectedFlash].u32FlashSize;

    snprintf(acOperationText, sizeof(acOperationText), "Dumping %s", psContext->sChipDetails.asFlashes[psContext->u32SelectedFlash].pcFlashName);

    iFd = open(pcDumpFile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR);

    if (iFd < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_FAILED_TO_OPEN_FILE, "\"%s\" (%s)", pcDumpFile, pcPRG_GetLastErrorMessage(psContext));
    }

    for(n = psContext->u32FlashOffset; n < u32FlashSize ; n += u8ChunkSize)
    {
        uint8_t au8Buffer[u8ChunkSize];
        teStatus eStatus;

        if ((n + u8ChunkSize) > u32FlashSize)
        {
            // Don't run off the end of flash
            u8ChunkSize = u32FlashSize - n;
        }

        if ((eStatus = eBL_FlashRead(psContext, n, u8ChunkSize, au8Buffer)) != E_PRG_OK)
        {
            close(iFd);
            return ePRG_SetStatus(psContext, eStatus, "reading Flash at address 0x%08X", n);
        }

        if (write(iFd, au8Buffer, u8ChunkSize) < 0)
        {
            close(iFd);
            return ePRG_SetStatus(psContext, E_PRG_ERROR_WRITING, "file at address 0x%08X", n);
        }

        if (cbProgress)
        {
            if (cbProgress(pvUser, acOperationText, "Dumping Flash", u32FlashSize, n) != E_PRG_OK)
            {
                close(iFd);
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }
    if (cbProgress)
    {
        if (cbProgress(pvUser, acOperationText, "Dumping Flash", u32FlashSize, u32FlashSize) != E_PRG_OK)
        {
            close(iFd);
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    close(iFd);
    return ePRG_SetStatus(psContext, E_PRG_OK, "Flash dumped succesfully");
}


teStatus ePRG_MemoryLoadExecute(tsPRG_Context *psContext, tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    teStatus eStatus;
    int n;
    uint8_t u8ChunkSize;
    tsChipDetails *psChipDetails;
    tsFW_Info *psFWImage;

    if (!psContext || !cbConfirm)
    {
        return E_PRG_NULL_PARAMETER;
    }
    if (psContext->sDeviceConfig.eCRP == E_DC_CRP_LEVEL1)
    {
        return ePRG_SetStatus(psContext, E_PRG_CRP_SET, "loading RAM");
    }

    psChipDetails = &psContext->sChipDetails;
    psFWImage = &psContext->sFirmwareInfo;

    if (psFWImage->sFlags.bRawImage)
    {
        return ePRG_SetStatus(psContext, E_PRG_INCOMPATIBLE, "cannot execute invalid image");
    }

    if (memcmp(&psFWImage->u32ROMVersion, &psChipDetails->u32SupportedFirmware, 4) != 0)
    {
        eStatus = E_PRG_INCOMPATIBLE;
        DBG_vPrintf(TRACE_PROGRAMMER, "Confirm operation: image built for 0x%08X, connected device is 0x%08x", psFWImage->u32ROMVersion, psChipDetails->u32SupportedFirmware);
        if (cbConfirm)
        {
            eStatus = cbConfirm(pvUser, "Confirm Operation", "The loaded image does not appear to be compatible with the connected device. Continue?");
        }
        if (eStatus != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_INCOMPATIBLE, "image built for 0x%08X, connected device is 0x%08x", psFWImage->u32ROMVersion, psChipDetails->u32SupportedFirmware);
        }
        // User has given the go ahead to continue anyway. Here be dragons.
    }

    if ((eStatus = ePRG_SetUpImage(psContext, psFWImage, psChipDetails)) != E_PRG_OK)
    {
        return eStatus;
    }

    DBG_vPrintf(TRACE_PROGRAMMER, "Programming image length %d into RAM length %d\n", psFWImage->u32ImageLength, psContext->sChipDetails.u32RamSize);
    if ((psFWImage->u32ImageLength + psFWImage->u32BssSectionLength) > psContext->sChipDetails.u32RamSize)
    {
        return ePRG_SetStatus(psContext, E_PRG_INCOMPATIBLE, "image is larger than RAM.");
    }

    DBG_vPrintf(TRACE_PROGRAMMER, "Text Start 0x%08x - Length %d bytes\n", psFWImage->u32TextSectionLoadAddress, psFWImage->u32TextSectionLength);
    DBG_vPrintf(TRACE_PROGRAMMER, "BSS  Start 0x%08x - Length %d bytes\n", psFWImage->u32BssSectionLoadAddress, psFWImage->u32BssSectionLength);
    DBG_vPrintf(TRACE_PROGRAMMER, "Reset entry point 0x%08x\n", psFWImage->u32ResetEntryPoint);
    DBG_vPrintf(TRACE_PROGRAMMER, "Wake  entry point 0x%08x\n", psFWImage->u32WakeUpEntryPoint);

    DBG_vPrintf(TRACE_PROGRAMMER, "Loading .text\n");

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Loading program in RAM", "Loading", 0, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    /* Download text segment */
    for(n = 0; n < psFWImage->u32TextSectionLength; n += u8ChunkSize)
    {
        uint8_t au8Buffer[BL_MAX_CHUNK_SIZE + 1];

        if((psFWImage->u32TextSectionLength - n) > BL_MAX_CHUNK_SIZE)
        {
            u8ChunkSize = BL_MAX_CHUNK_SIZE;
        }
        else
        {
            u8ChunkSize = psFWImage->u32TextSectionLength - n;
        }

        if((eStatus = eBL_MemoryWrite(psContext, psFWImage->u32TextSectionLoadAddress + n, u8ChunkSize, sizeof(uint8_t), psFWImage->pu8TextData + n)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "writing chunk at address 0x%08X", psFWImage->u32TextSectionLoadAddress + n);
        }

        /* Verify the memory contents */
        if ((eStatus = eBL_MemoryRead(psContext, psFWImage->u32TextSectionLoadAddress + n, u8ChunkSize, sizeof(uint8_t), au8Buffer)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "reading at address 0x%08X", psFWImage->u32TextSectionLoadAddress + n);
        }
        else
        {
            if (memcmp(psFWImage->pu8TextData + n, au8Buffer, u8ChunkSize))
            {
                return ePRG_SetStatus(psContext, E_PRG_VERIFICATION_FAILED, "at address 0x%08X", psFWImage->u32TextSectionLoadAddress + n);
            }
        }

        if (cbProgress)
        {
            if (cbProgress(pvUser, "Loading Image in RAM", "Loading", psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength, n) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }

    DBG_vPrintf(TRACE_PROGRAMMER, "Clearing .bss     \n");

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Loading program in RAM", "Clearing .bss", psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength, psFWImage->u32TextSectionLength) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    /* Clear BSS segment */
    {
        uint8_t au8Buffer[127];
        memset(au8Buffer, 0, sizeof(au8Buffer));
        for(n = 0; n < psFWImage->u32BssSectionLength; n += u8ChunkSize)
        {
            if((psFWImage->u32BssSectionLength - n) > sizeof(au8Buffer))
            {
                u8ChunkSize = sizeof(au8Buffer);
            }
            else
            {
                u8ChunkSize = psFWImage->u32BssSectionLength - n;
            }
            if((eStatus = eBL_MemoryWrite(psContext, psFWImage->u32BssSectionLoadAddress + n, u8ChunkSize, sizeof(uint8_t), au8Buffer)) != E_PRG_OK)
            {
                return ePRG_SetStatus(psContext, eStatus, "writing bss at address 0x%08X", psFWImage->u32BssSectionLoadAddress + n);
            }

            if (cbProgress)
            {
                if (cbProgress(pvUser, "Loading program in RAM", "Clearing .bss", psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength, psFWImage->u32TextSectionLength + n) != E_PRG_OK)
                {
                    return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
                }
            }
        }
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Loading program in RAM", "Clearing .bss", psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength, psFWImage->u32TextSectionLength + psFWImage->u32BssSectionLength) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    DBG_vPrintf(TRACE_PROGRAMMER, "Starting module application\n");

    return ePRG_SetStatus(psContext, eBL_MemoryExecute(psContext, psFWImage->u32ResetEntryPoint), "executing program in RAM");
}


teStatus ePRG_EepromErase(tsPRG_Context *psContext, teEepromErase eErase, tcbFW_Progress cbProgress, void *pvUser)
{
    int iEraseAll;
    teStatus eStatus;
    tsChipDetails *psChipDetails;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psChipDetails = &psContext->sChipDetails;

    /* check chip has EEPROM */
    if(psChipDetails->boEEPpresent == FALSE)
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
    }

    switch (eErase)
    {
        case E_ERASE_EEPROM_PDM:
            iEraseAll = 0;
            break;

        case E_ERASE_EEPROM_ALL:
            iEraseAll = 1;
            break;

        case E_ERASE_EEPROM_NONE:
        default:
            return ePRG_SetStatus(psContext, E_PRG_BAD_PARAMETER, "");
    }
    
    printf( "iEraseAll = %d\n", iEraseAll );

    if(!BOOTLOADER_CAPABILITY_EEPROM_ACCESS(psChipDetails->u32BootloaderVersion))
    {
        /* For this bootloader we have to load an extension binary in to RAM first */
        if ((eStatus = ePRG_FlashProgrammerExtensionLoad(psContext, "EEPROM access")) != E_PRG_OK)
        {
            return eStatus;
        }
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Erasing EEPROM", "Erasing EEPROM", 0, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    if ((eStatus = eBL_EEPROMErase(psContext, iEraseAll)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "erasing EEPROM");
    }

    if(!BOOTLOADER_CAPABILITY_EEPROM_ACCESS(psChipDetails->u32BootloaderVersion))
    {
        /* Return to bootloader */
        if ((eStatus = ePRG_FlashProgrammerExtensionReturn(psContext)) != E_PRG_OK)
        {
            return eStatus;
        }
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Erasing EEPROM", "Erasing EEPROM", 1, 1) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


teStatus ePRG_EepromDump(tsPRG_Context *psContext, char *pcDumpFile, tcbFW_Progress cbProgress, void *pvUser)
{
    uint8_t u8ChunkSize = 64;
    int iFd;
    uint32_t n;
    teStatus eStatus;
    tsChipDetails *psChipDetails;

    if (!psContext || !pcDumpFile)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psChipDetails = &psContext->sChipDetails;

    /* check chip has EEPROM */
    if(psChipDetails->boEEPpresent == FALSE)
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
    }

    iFd = open(pcDumpFile, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR);
    if (iFd < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_FAILED_TO_OPEN_FILE, "\"%s\" (%s)", pcDumpFile, pcPRG_GetLastErrorMessage(psContext));
    }

    if(!BOOTLOADER_CAPABILITY_EEPROM_ACCESS(psChipDetails->u32BootloaderVersion))
    {
        /* For this bootloader we have to load an extension binary in to RAM first */
        if ((eStatus = ePRG_FlashProgrammerExtensionLoad(psContext, "EEPROM access")) != E_PRG_OK)
        {
            return eStatus;
        }
    }

    // using extension or native bootloader support
    for(n = psContext->u32EepromOffset; n < psContext->sChipDetails.u32EepromSize ; n += u8ChunkSize)
    {
        uint8_t au8Buffer[u8ChunkSize];
        teStatus eStatus;

        if ((n + u8ChunkSize) > psContext->sChipDetails.u32EepromSize)
        {
            // Don't run off the end of eeprom
            u8ChunkSize = psContext->sChipDetails.u32EepromSize - n;
        }

        if ((eStatus = eBL_EEPROMRead(psContext, n, u8ChunkSize, au8Buffer)) != E_PRG_OK)
        {
            close(iFd);
            return ePRG_SetStatus(psContext, eStatus, "reading EEPROM at address 0x%08X", n);
        }

        if (write(iFd, au8Buffer, u8ChunkSize) < 0)
        {
            close(iFd);
            return ePRG_SetStatus(psContext, E_PRG_ERROR_WRITING, "file at address 0x%08X", n);
        }

        if (cbProgress)
        {
            if (cbProgress(pvUser, "Dumping EEPROM", "Dumping EEPROM", psContext->sChipDetails.u32EepromSize, n) != E_PRG_OK)
            {
                close(iFd);
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Dumping EEPROM", "Dumping EEPROM", psContext->sChipDetails.u32EepromSize, psContext->sChipDetails.u32EepromSize) != E_PRG_OK)
        {
            close(iFd);
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    close(iFd);

    if(!BOOTLOADER_CAPABILITY_EEPROM_ACCESS(psChipDetails->u32BootloaderVersion))
    {
        /* Return to bootloader */
        if ((eStatus = ePRG_FlashProgrammerExtensionReturn(psContext)) != E_PRG_OK)
        {
            return eStatus;
        }
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "");

}


teStatus ePRG_EepromProgram(tsPRG_Context *psContext, char *pcLoadFile, tcbFW_Progress cbProgress, void *pvUser)
{
    uint8_t u8ChunkSize;
    int iFd;
    uint32_t n;
    teStatus eStatus;
    tsChipDetails *psChipDetails;

    if (!psContext || !pcLoadFile)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psChipDetails = &psContext->sChipDetails;

    /* check chip has EEPROM */
    if(psChipDetails->boEEPpresent == FALSE)
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
    }

    iFd = open(pcLoadFile, O_RDONLY | O_BINARY);
    if (iFd < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_FAILED_TO_OPEN_FILE, "\"%s\" (%s)", pcLoadFile, pcPRG_GetLastErrorMessage(psContext));
    }

    {
        struct stat sb;
        uint32_t u32FileSize;

        if (fstat(iFd, &sb) == -1)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "(%s)", pcPRG_GetLastErrorMessage(psContext));
        }

        u32FileSize = (uint32_t)sb.st_size;

        DBG_vPrintf(TRACE_PROGRAMMER, "Programming EEPROM image length %d into EEPROM length %d at offset 0x%x\n", u32FileSize, psContext->sChipDetails.u32EepromSize, psContext->u32EepromOffset);
        if (u32FileSize == 0)
        {
            return ePRG_SetStatus(psContext, E_PRG_INVALID_FILE, ", file is empty");
        }
        else if ((psContext->u32EepromOffset + u32FileSize) > psContext->sChipDetails.u32EepromSize)
        {
            return ePRG_SetStatus(psContext, E_PRG_INVALID_FILE, ", image is larger than EEPROM");
        }
    }

    if(!BOOTLOADER_CAPABILITY_EEPROM_ACCESS(psChipDetails->u32BootloaderVersion))
    {
        /* For this bootloader we have to load an extension binary in to RAM first */
        if ((eStatus = ePRG_FlashProgrammerExtensionLoad(psContext, "EEPROM access")) != E_PRG_OK)
        {
            return eStatus;
        }
    }

    for(n = 0; ; n += u8ChunkSize)
    {
        uint8_t au8Buffer[128];
        teStatus eStatus;

        u8ChunkSize = read(iFd, au8Buffer, 128);

        if (u8ChunkSize <= 0)
        {
            DBG_vPrintf(TRACE_PROGRAMMER, "End of EEPROM load file at address 0x%08X\n", n);
            break;
        }

        if ((n + u8ChunkSize) > psChipDetails->u32EepromSize)
        {
            DBG_vPrintf(TRACE_PROGRAMMER, "File is larger than the device EEPROM\n", n);
            break;
        }

        DBG_vPrintf(TRACE_PROGRAMMER, "Write %d bytes to EEPROM address 0x%08X\n", u8ChunkSize, psContext->u32EepromOffset + n);

        if ((eStatus = eBL_EEPROMWrite(psContext, psContext->u32EepromOffset + n, u8ChunkSize, au8Buffer)) != E_PRG_OK)
        {
            close(iFd);
            return ePRG_SetStatus(psContext, eStatus, "loading EEPROM at address 0x%08X", psContext->u32EepromOffset + n);
        }

        if (cbProgress)
        {
            if (cbProgress(pvUser, "Loading EEPROM", "Loading EEPROM", psContext->sChipDetails.u32EepromSize, n) != E_PRG_OK)
            {
                close(iFd);
                return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
            }
        }
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Loading EEPROM", "Loading EEPROM", psContext->sChipDetails.u32EepromSize, psContext->sChipDetails.u32EepromSize) != E_PRG_OK)
        {
            close(iFd);
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    close(iFd);

    if(!BOOTLOADER_CAPABILITY_EEPROM_ACCESS(psChipDetails->u32BootloaderVersion))
    {
        /* Return to bootloader */
        if ((eStatus = ePRG_FlashProgrammerExtensionReturn(psContext)) != E_PRG_OK)
        {
            return eStatus;
        }
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


teStatus ePRG_IndexSectorWrite(tsPRG_Context *psContext, uint8_t u8Page, uint8_t u8WordNumber, uint32_t au32Data[4], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    teStatus eStatus;
    tsChipDetails *psChipDetails;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    psChipDetails = &psContext->sChipDetails;

    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) != CHIP_ID_PART(CHIP_ID_JN5168)) &&
        (CHIP_ID_PART(psChipDetails->u32ChipId) != CHIP_ID_PART(CHIP_ID_JN5169)) &&
        (CHIP_ID_PART(psChipDetails->u32ChipId) != CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
    }

    if (cbConfirm == NULL)
    {
        /* We must be able to confirm that the user is OK with this. */
        return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
    }

    {
        uint8_t au8Buffer[IP2111_INDEX_SECTOR_WORD_LENGTH];
        uint8_t au8TempBuffer[IP2111_INDEX_SECTOR_WORD_LENGTH];
        memset(au8TempBuffer, 0xFF, IP2111_INDEX_SECTOR_WORD_LENGTH);

        /* First we read the existing word, to make sure it is not already written */
        if ((eStatus = eBL_MemoryRead(psContext, u32PRG_JN516x_index_sector_address(u8Page, u8WordNumber), IP2111_INDEX_SECTOR_WORD_LENGTH, sizeof(uint32_t), au8Buffer)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "reading from flash index sector");
        }

        if (memcmp(au8TempBuffer, au8Buffer, IP2111_INDEX_SECTOR_WORD_LENGTH))
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "one time programmable data has already been set in this device");
        }
    }

    if (cbConfirm(pvUser, "Confirm Operation",
        "WARNING: This operation will program one-time programmable fields in the index sector of the device.\n"
        "The index sector contains many fields, which if set incorrectly could render the device inoperable, voiding any warranties.\n"
        "Please consult the user documentation for more details on the setting of these fields.\n"
        "Are you sure you wish to continue with this operation?"
        ) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
    }

    if (cbProgress)
    {
        if (cbProgress(pvUser, "Programming Index Sector", "Programming Index Sector", 0, 0) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ABORTED, "");
        }
    }

    if(!BOOTLOADER_CAPABILITY_INDEX_SECTOR(psChipDetails->u32BootloaderVersion))
    {
        /* For this bootloader we have to load an extension binary in to RAM first */
        if ((eStatus = ePRG_FlashProgrammerExtensionLoad(psContext, "programming OTP data")) != E_PRG_OK)
        {
            return eStatus;
        }
    }

    if ((eStatus = eBL_IndexSectorWrite(psContext, u8Page, u8WordNumber, au32Data)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "writing index sector");
    }

    if(!BOOTLOADER_CAPABILITY_INDEX_SECTOR(psChipDetails->u32BootloaderVersion))
    {
        /* Return to bootloader */
        if ((eStatus = ePRG_FlashProgrammerExtensionReturn(psContext)) != E_PRG_OK)
        {
            return eStatus;
        }
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


teStatus ePRG_MACAddressSet(tsPRG_Context *psContext, uint8_t au8MacAddress[8], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    tsChipDetails *psChipDetails;
    uint32_t au32WriteData[IP2111_INDEX_SECTOR_WORD_LENGTH / sizeof(uint32_t)];

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    DBG_vPrintf(TRACE_PROGRAMMER, "Setting MAC address %02X:%02X:%02X:%02X:%02X:%02X:%02X\n",
                au8MacAddress[0], au8MacAddress[1], au8MacAddress[2], au8MacAddress[3],
                au8MacAddress[4], au8MacAddress[5], au8MacAddress[6], au8MacAddress[7]);

    psChipDetails = &psContext->sChipDetails;

    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        au32WriteData[0] = (au8MacAddress[0] << 24) | (au8MacAddress[1] << 16) | (au8MacAddress[2] << 8) | (au8MacAddress[3]);
        au32WriteData[1] = (au8MacAddress[4] << 24) | (au8MacAddress[5] << 16) | (au8MacAddress[6] << 8) | (au8MacAddress[7]);
        memset(&au32WriteData[2], 0xFF, 8);

        // return  E_PRG_OK;
        return ePRG_IndexSectorWrite(psContext, JN516X_INDEX_SECTOR_MAC_CUSTOMER_PAGE, JN516X_INDEX_SECTOR_MAC_CUSTOMER_WORD, au32WriteData, cbProgress, cbConfirm, pvUser);
    }
    else
    {
        /* On older devices, just update the MAC address in the Chip information for the next time we write to flash. */
        memcpy(psChipDetails->au8MacAddress, au8MacAddress, 8);
        return ePRG_SetStatus(psContext, E_PRG_OK, "");
    }
}

teStatus LIBPROGRAMMER ePRG_AesKeyGet(tsPRG_Context *psContext, uint32_t au32Key[4], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    tsChipDetails *psChipDetails;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;

    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        if(eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_AES_KEY_ADDRESS, 16, sizeof(uint32_t), (uint8_t*)au32Key) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "reading AES key from flash index sector");
        }
        return ePRG_SetStatus(psContext, E_PRG_OK, "");
    }
    return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
}

teStatus ePRG_AesKeySet(tsPRG_Context *psContext, uint32_t au32Key[4], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    tsChipDetails *psChipDetails;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;

    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        return ePRG_IndexSectorWrite(psContext, JN516X_INDEX_SECTOR_AES_KEY_PAGE, JN516X_INDEX_SECTOR_AES_KEY_WORD, au32Key, cbProgress, cbConfirm, pvUser);
    }

    return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
}

teStatus LIBPROGRAMMER ePRG_UserDataGet(tsPRG_Context *psContext, uint32_t u32Index, uint32_t au32Data[4], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    tsChipDetails *psChipDetails;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;

    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        switch (u32Index)
        {
            case (0):
                if(eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_USER0_ADDRESS, 16, sizeof(uint32_t), (uint8_t*)au32Data) != E_PRG_OK)
                {
                    return ePRG_SetStatus(psContext, E_PRG_ERROR, "reading userdata from flash index sector");
                }
                return ePRG_SetStatus(psContext, E_PRG_OK, "");

            case (1):
                if(eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_USER1_ADDRESS, 16, sizeof(uint32_t), (uint8_t*)au32Data) != E_PRG_OK)
                {
                    return ePRG_SetStatus(psContext, E_PRG_ERROR, "reading userdata from flash index sector");
                }
                return ePRG_SetStatus(psContext, E_PRG_OK, "");

            case (2):
                if(eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_USER2_ADDRESS, 16, sizeof(uint32_t), (uint8_t*)au32Data) != E_PRG_OK)
                {
                    return ePRG_SetStatus(psContext, E_PRG_ERROR, "reading userdata from flash index sector");
                }
                return ePRG_SetStatus(psContext, E_PRG_OK, "");

            default:
                return ePRG_SetStatus(psContext, E_PRG_BAD_PARAMETER, "out of range userdata word");
        }
    }
    else
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
    }
}



teStatus LIBPROGRAMMER ePRG_UserDataSet(tsPRG_Context *psContext, uint32_t u32Index, uint32_t au32Data[4], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    tsChipDetails *psChipDetails;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;

    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        switch (u32Index)
        {
            case(0):
                return ePRG_IndexSectorWrite(psContext, JN516X_INDEX_SECTOR_USER0_PAGE, JN516X_INDEX_SECTOR_USER0_WORD, au32Data, cbProgress, cbConfirm, pvUser);
            case(1):
                return ePRG_IndexSectorWrite(psContext, JN516X_INDEX_SECTOR_USER1_PAGE, JN516X_INDEX_SECTOR_USER1_WORD, au32Data, cbProgress, cbConfirm, pvUser);
            case(2):
                return ePRG_IndexSectorWrite(psContext, JN516X_INDEX_SECTOR_USER2_PAGE, JN516X_INDEX_SECTOR_USER2_WORD, au32Data, cbProgress, cbConfirm, pvUser);
            default:
                return ePRG_SetStatus(psContext, E_PRG_ERROR, "JN516x device only has 3 words of user data");
        }
    }

    return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
}


static teStatus ePRG_DeviceConfigGet(tsPRG_Context *psContext)
{
    tsChipDetails *psChipDetails;
    tsDeviceConfig *psDeviceConfig;
    uint32_t u32DeviceConfig;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;
    psDeviceConfig = &psContext->sDeviceConfig;

    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        uint32_t au32Buffer[IP2111_INDEX_SECTOR_WORD_LENGTH / sizeof(uint32_t)];

        /* Read IP2111 configuration first */
        if (eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_IP2111_CONFIG_ADDRESS, sizeof(au32Buffer), sizeof(uint32_t), (uint8_t*)au32Buffer) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "reading configuration from flash index sector");
        }

        DBG_vPrintf(TRACE_PROGRAMMER, "IP2111 config (read from 0x%08X): 0x%08X 0x%08X 0x%08X 0x%08X\n",
                    JN516X_INDEX_SECTOR_IP2111_CONFIG_ADDRESS, au32Buffer[0], au32Buffer[1], au32Buffer[2], au32Buffer[3]);

        // Majority voting
        u32DeviceConfig = (au32Buffer[0] & au32Buffer[1]) | (au32Buffer[0] & au32Buffer[2]) | (au32Buffer[1] & au32Buffer[2]);
        if (__builtin_popcount(u32DeviceConfig & 0x0000000F) <= 2)
        {
            /* JTAG Flash access is disabled */
            psDeviceConfig->eJtag = E_DC_JTAG_DISABLE_FLASH;
        }
        else
        {
            psDeviceConfig->eJtag = E_DC_JTAG_ENABLE;
        }

        /* And now the customer settings word */

        if (eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_ADDRESS, sizeof(au32Buffer), sizeof(uint32_t), (uint8_t*)au32Buffer) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "reading configuration from flash index sector");
        }
        DBG_vPrintf(TRACE_PROGRAMMER, "Customer config (read from 0x%08X): 0x%08X 0x%08X 0x%08X 0x%08X\n",
                    JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_ADDRESS, au32Buffer[0], au32Buffer[1], au32Buffer[2], au32Buffer[3]);

        // Majority voting
        u32DeviceConfig = (au32Buffer[0] & au32Buffer[1]) | (au32Buffer[0] & au32Buffer[2]) | (au32Buffer[1] & au32Buffer[2]);

        DBG_vPrintf(TRACE_PROGRAMMER, "Configuration (read from 0x%08X): 0x%08X (0x", JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_ADDRESS, u32DeviceConfig);
        {
            int i;
            for (i = 0; i < sizeof(au32Buffer); i++)
            {
                DBG_vPrintf(TRACE_PROGRAMMER, "%02X", ((uint8_t*)au32Buffer)[i]);
            }
            DBG_vPrintf(TRACE_PROGRAMMER, ")\n");
        }

        if (psDeviceConfig->eJtag == E_DC_JTAG_DISABLE_FLASH)
        {
            /* JTAG Flash access is disabled */
            if ((u32DeviceConfig & 0x01) == 0)
            {
                /* CPU JTAG Tap is disabled */
                psDeviceConfig->eJtag = E_DC_JTAG_DISABLE;
            }
        }
        else
        {
            /* JTAG Flash access is enabled */
            if ((u32DeviceConfig & 0x01) == 0)
            {
                /* CPU JTAG Tap is disabled */
                psDeviceConfig->eJtag = E_DC_JTAG_DISABLE_CPU;
            }
        }

        switch (((u32DeviceConfig & (7 << 1)) >> 1) ^ 6)
        {
            case (0):   psDeviceConfig->eVbo = E_DC_VBO_195;    break;
            case (1):   psDeviceConfig->eVbo = E_DC_VBO_200;    break;
            case (2):   psDeviceConfig->eVbo = E_DC_VBO_210;    break;
            case (3):   psDeviceConfig->eVbo = E_DC_VBO_220;    break;
            case (4):   psDeviceConfig->eVbo = E_DC_VBO_230;    break;
            case (5):   psDeviceConfig->eVbo = E_DC_VBO_240;    break;
            case (6):   psDeviceConfig->eVbo = E_DC_VBO_270;    break;
            case (7):   psDeviceConfig->eVbo = E_DC_VBO_300;    break;
            default: break;
        }

        switch((u32DeviceConfig & (3 << 4)) >> 4)
        {
            case (0):   psDeviceConfig->eCRP = E_DC_CRP_LEVEL2; break;
            case (1):   psDeviceConfig->eCRP = E_DC_CRP_LEVEL1; break;
            case (2):   psDeviceConfig->eCRP = E_DC_CRP_LEVEL2; break;
            case (3):   psDeviceConfig->eCRP = E_DC_CRP_LEVEL0; break;
            default: break;
        }

        if (u32DeviceConfig & (1 << 6))
            psDeviceConfig->eExternalFlashEncrypted = E_DC_EXTERNAL_FLASH_ENCRYPTED_FALSE;
        else
            psDeviceConfig->eExternalFlashEncrypted = E_DC_EXTERNAL_FLASH_ENCRYPTED_TRUE;

        if (u32DeviceConfig & (1 << 7))
            psDeviceConfig->eExternalFlashLoad = E_DC_EXTERNAL_FLASH_LOAD_ENABLE;
        else
            psDeviceConfig->eExternalFlashLoad = E_DC_EXTERNAL_FLASH_LOAD_DISABLE;

        return ePRG_SetStatus(psContext, E_PRG_OK, "");
    }
    return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
}


teStatus LIBPROGRAMMER ePRG_DeviceConfigSet(tsPRG_Context *psContext, tsDeviceConfig *psDeviceConfig, tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser)
{
    tsChipDetails *psChipDetails;
    teStatus eStatus;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;
    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        uint32_t au32Buffer[IP2111_INDEX_SECTOR_WORD_LENGTH / sizeof(uint32_t)];
        uint32_t u32DeviceConfig = 0xFFFFFFFF;

        /* Write IP2111 configuration first */

        if ((psDeviceConfig->eJtag == E_DC_JTAG_DISABLE) || (psDeviceConfig->eJtag == E_DC_JTAG_DISABLE_FLASH))
        {
            u32DeviceConfig &= 0xFFFFFFF0;

            /* Device has 3 copies of 32 bit word and uses majority vote to determine setting */
            au32Buffer[0] = u32DeviceConfig;
            au32Buffer[1] = u32DeviceConfig;
            au32Buffer[2] = u32DeviceConfig;
            au32Buffer[3] = u32DeviceConfig;

            DBG_vPrintf(TRACE_PROGRAMMER, "Write to Index sector: 0x%08X%08X%08X%08X\n", au32Buffer[0], au32Buffer[1], au32Buffer[2], au32Buffer[3]);

            eStatus = ePRG_IndexSectorWrite(psContext, JN516X_INDEX_SECTOR_IP2111_CONFIG_PAGE, JN516X_INDEX_SECTOR_IP2111_CONFIG_WORD, au32Buffer, cbProgress, cbConfirm, pvUser);

            /* Stop further user prompts */
            cbConfirm = ePRG_ConfirmAlways;

            if (eStatus != E_PRG_OK)
            {
                return eStatus;
            }
        }

        /* And now the customer settings */
        u32DeviceConfig = 0xFFFFFFFF;

        if ((psDeviceConfig->eJtag == E_DC_JTAG_DISABLE) || (psDeviceConfig->eJtag == E_DC_JTAG_DISABLE_CPU))
            u32DeviceConfig &= 0xFFFFFFFE;

        switch (psDeviceConfig->eVbo)
        {
#define JN5168_VBO(a) (((a ^ 6) << 1) | ~(7 << 1))
            case (E_DC_VBO_195):    u32DeviceConfig &= JN5168_VBO(0);   break;
            case (E_DC_VBO_200):    u32DeviceConfig &= JN5168_VBO(1);   break;
            case (E_DC_VBO_210):    u32DeviceConfig &= JN5168_VBO(2);   break;
            case (E_DC_VBO_220):    u32DeviceConfig &= JN5168_VBO(3);   break;
            case (E_DC_VBO_230):    u32DeviceConfig &= JN5168_VBO(4);   break;
            case (E_DC_VBO_240):    u32DeviceConfig &= JN5168_VBO(5);   break;
            case (E_DC_VBO_270):    u32DeviceConfig &= JN5168_VBO(6);   break;
            case (E_DC_VBO_300):    u32DeviceConfig &= JN5168_VBO(7);   break;
#undef JN5168_VBO
            default: break;
        }

        switch(psDeviceConfig->eCRP)
        {
            case (E_DC_CRP_LEVEL1): u32DeviceConfig &= (1 << 4) | ~(3 << 4);    break;
            case (E_DC_CRP_LEVEL2): u32DeviceConfig &= (2 << 4) | ~(3 << 4);    break;
            default: break;
        }

        if (psDeviceConfig->eExternalFlashEncrypted == E_DC_EXTERNAL_FLASH_ENCRYPTED_TRUE)
            u32DeviceConfig &= ~(1 << 6);

        if (psDeviceConfig->eExternalFlashLoad == E_DC_EXTERNAL_FLASH_LOAD_DISABLE)
            u32DeviceConfig &= ~(1 << 7);

        /* Device has 3 copies of 32 bit word and uses majority vote to determine setting */
        au32Buffer[0] = u32DeviceConfig;
        au32Buffer[1] = u32DeviceConfig;
        au32Buffer[2] = u32DeviceConfig;
        au32Buffer[3] = u32DeviceConfig;

        DBG_vPrintf(TRACE_PROGRAMMER, "Write to Index sector: 0x%08X%08X%08X%08X\n", au32Buffer[0], au32Buffer[1], au32Buffer[2], au32Buffer[3]);

        eStatus = ePRG_IndexSectorWrite(psContext, JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_PAGE, JN516X_INDEX_SECTOR_CUSTOMER_CONFIG_WORD, au32Buffer, cbProgress, cbConfirm, pvUser);

        if (eStatus == E_PRG_OK)
        {
            memcpy(&psContext->sDeviceConfig, psDeviceConfig, sizeof(tsDeviceConfig));
        }
        return eStatus;
    }

    return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
}

#if 0
void dbg_vPrintfImpl(const char *pcFile, uint32_t u32Line, const char *pcFormat, ...)
{
    va_list argp;
    va_start(argp, pcFormat);
#ifdef DBG_VERBOSE
    printf("%s:%d ", pcFile, u32Line);
#endif /* DBG_VERBOSE */
    vprintf(pcFormat, argp);
    fflush(stdout);
    return;
}
#endif

teStatus ePRG_SetStatus(tsPRG_Context *psContext, teStatus eStatus, const char *pcAdditionalInfoFmt, ...)
{
    const char *pcStatus;
    uint32_t u32Length = 0;
    va_list ap;
    va_start(ap, pcAdditionalInfoFmt);

    switch (eStatus)
    {
        case(E_PRG_OK):                         pcStatus = "Success"; break;
        case(E_PRG_ERROR):                      pcStatus = ""; break;
        case(E_PRG_OUT_OF_MEMORY):              pcStatus = "Low on memory"; break;
        case(E_PRG_ERROR_WRITING):              pcStatus = "Write error"; break;
        case(E_PRG_ERROR_READING):              pcStatus = "Read error"; break;
        case(E_PRG_FAILED_TO_OPEN_FILE):        pcStatus = "Failed to open file"; break;
        case(E_PRG_BAD_PARAMETER):              pcStatus = "Bad parameter"; break;
        case(E_PRG_NULL_PARAMETER):             pcStatus = "NULL parameter"; break;
        case(E_PRG_INCOMPATIBLE):               pcStatus = "Image is not compatible with chip,"; break;
        case(E_PRG_INVALID_FILE):               pcStatus = "Invalid image file"; break;
        case(E_PRG_UNSUPPORTED_CHIP):           pcStatus = "Unsupported chip"; break;
        case(E_PRG_ABORTED):                    pcStatus = "Aborted"; break;
        case(E_PRG_VERIFICATION_FAILED):        pcStatus = "Verification failed"; break;
        case(E_PRG_INVALID_TRANSPORT):          pcStatus = "Invalid transport"; break;
        case(E_PRG_COMMS_FAILED):               pcStatus = "Communication failure"; break;
        case(E_PRG_UNSUPPORTED_OPERATION):      pcStatus = "Bootloader doesn't support"; break;
        case(E_PRG_FLASH_DEVICE_UNAVAILABLE):   pcStatus = "Could not select flash device"; break;
        case(E_PRG_CRP_SET):                    pcStatus = "Code protection prevents"; break;
        default:                                pcStatus = "Unknown"; break;
    }

    u32Length = snprintf(psContext->acStatusMessage, PRG_MAX_STATUS_LENGTH, "%s ", pcStatus);
    vsnprintf(&psContext->acStatusMessage[u32Length], PRG_MAX_STATUS_LENGTH - u32Length, pcAdditionalInfoFmt, ap);
    va_end(ap);

    return eStatus;
}

#if defined POSIX
char *pcPRG_GetLastErrorMessage(tsPRG_Context *psContext)
{
    strerror_r(errno, psContext->acErrorMsgBuffer, PRG_MAX_STATUS_LENGTH);
    return psContext->acErrorMsgBuffer;
}
#elif defined WIN32
char *pcPRG_GetLastErrorMessage(tsPRG_Context *psContext)
{
    DWORD dwLastError = GetLastError();
    DWORD dwNumChars;

    dwNumChars = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dwLastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&psContext->acErrorMsgBuffer,
        PRG_MAX_STATUS_LENGTH,
        NULL);

    // Remove \n\r from end of message.
    psContext->acErrorMsgBuffer[dwNumChars-2] = '\0';

    return psContext->acErrorMsgBuffer;
}
#endif /* WIN32 */


void vPRG_WaitMs(uint32_t u32TimeoutMs)
{
    DBG_vPrintf(TRACE_PROGRAMMER, "Delay %dms...", u32TimeoutMs);
#if defined POSIX
    usleep(u32TimeoutMs * 1000);
#elif defined WIN32
    Sleep(u32TimeoutMs);
#endif
    DBG_vPrintf(TRACE_PROGRAMMER, "waited\n");
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

static teStatus ePRG_ChipGetChipId(tsPRG_Context *psContext)
{
    uint8_t au8Buffer[16];
    uint32_t au32Buffer[4];
    uint32_t u32DeviceConfig;
    tsChipDetails *psChipDetails = &psContext->sChipDetails;

    DBG_vPrintf(TRACE_PROGRAMMER, "Get Chip ID\n");

    if(psChipDetails == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }

    memset(psChipDetails, 0, sizeof(tsChipDetails));

    /* Default to big endian chip */
    psChipDetails->eEndianness = E_CHIP_BIG_ENDIAN;

    /* Send chip id request */
    if(eBL_ChipIdRead(psContext, &psChipDetails->u32ChipId, &psChipDetails->u32BootloaderVersion) != E_PRG_OK)
    {
        DBG_vPrintf(TRACE_PROGRAMMER, "Error reading chip id\n");

        /* That failed so it might be an old device that doesn't support the command, try reading it directly */
        if (eBL_MemoryRead(psContext, 0x100000FC, sizeof(uint32_t), sizeof(uint32_t), au8Buffer) != E_PRG_OK)
        {
            DBG_vPrintf(TRACE_PROGRAMMER, "Error Reading processor ID register\n");
            return E_PRG_ERROR;
        }
        else
        {
            memcpy(&psChipDetails->u32ChipId, au8Buffer, sizeof(uint32_t));
        }
    }

    DBG_vPrintf(TRACE_PROGRAMMER, "Chip ID: 0x%08X\n", psContext->sChipDetails.u32ChipId);

    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        if (psChipDetails->u32BootloaderVersion == 0)
        {
            if(CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
            {
                psChipDetails->eEndianness = E_CHIP_LITTLE_ENDIAN;

                if (eBL_MemoryRead(psContext, JN517X_BOOTLOADER_VERSION_ADDRESS, sizeof(uint32_t), sizeof(uint32_t), au8Buffer) != E_PRG_OK)
                {
                    DBG_vPrintf(TRACE_PROGRAMMER, "Error Reading bootloader version\n");
                    return E_PRG_ERROR;
                }
            }
            else // 6x
            {
                if (eBL_MemoryRead(psContext, JN516X_BOOTLOADER_VERSION_ADDRESS, sizeof(uint32_t), sizeof(uint32_t), au8Buffer) != E_PRG_OK)
                {
                    DBG_vPrintf(TRACE_PROGRAMMER, "Error Reading bootloader version\n");
                    return E_PRG_ERROR;
                }
            }

            // MemoryRead converts the word buffer into native endianness
            memcpy(&psChipDetails->u32BootloaderVersion, au8Buffer, sizeof(uint32_t));
        }

        DBG_vPrintf(TRACE_PROGRAMMER, "Bootloader version 0x%08x\n", psChipDetails->u32BootloaderVersion);

        teStatus eStatus;
        uint32_t u32FlashSize;

        if(eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_ATE_SETTINGS_ADDRESS, sizeof(au32Buffer), sizeof(uint32_t), (uint8_t*)au32Buffer) == E_PRG_OK)
        {

            // Majority voting
            u32DeviceConfig = (au32Buffer[0] & au32Buffer[1]) | (au32Buffer[0] & au32Buffer[2]) | (au32Buffer[1] & au32Buffer[2]);

            DBG_vPrintf(TRACE_PROGRAMMER, "ATE Settings (read from 0x%08X): 0x%08X (0x", JN516X_INDEX_SECTOR_ATE_SETTINGS_ADDRESS, u32DeviceConfig);
            {
                int i;
                for (i = 0; i < sizeof(au32Buffer); i++)
                {
                    DBG_vPrintf(TRACE_PROGRAMMER, "%02X", ((uint8_t*)au32Buffer)[i]);
                }
                DBG_vPrintf(TRACE_PROGRAMMER, ")\n");
            }

            if (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168))
            {
                psChipDetails->u32EepromSize    = (4 * 1024) - 64; /* Final segment not usable */
                u32FlashSize                    = (u32DeviceConfig & 0x07) >> 0;
                psChipDetails->u32RamSize       = (u32DeviceConfig & 0x30) >> 4;
            }
            else if (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169))
            {
                psChipDetails->u32EepromSize    = (16 * 1024) - 64; /* Final segment not usable */
                u32FlashSize                    = (u32DeviceConfig & 0x0F) >> 0;
                psChipDetails->u32RamSize       = (u32DeviceConfig & 0x30) >> 4;
            }
            else // JN5172 will enter here
            {
                psChipDetails->u32EepromSize    = (4 * 1024) - 64; /* Final segment not usable */
                u32FlashSize                    = (u32DeviceConfig & 0x0F) >> 0;
                psChipDetails->u32RamSize       = (u32DeviceConfig & 0x30) >> 4;
            }
        } else {
            psContext->sDeviceConfig.eCRP = E_DC_CRP_LEVEL1;

            if (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168))
            {
                psChipDetails->u32EepromSize    = (4 * 1024) - 64; /* Final segment not usable */
                u32FlashSize                    = 7;
                psChipDetails->u32RamSize       = 3;
            }
            else if (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169))
            {
                psChipDetails->u32EepromSize    = (16 * 1024) - 64; /* Final segment not usable */
                u32FlashSize                    = 0xF;
                psChipDetails->u32RamSize       = 3;
            }
            else // JN5172 will enter here
            {
                psChipDetails->u32EepromSize    = (4 * 1024) - 64; /* Final segment not usable */
                u32FlashSize                    = 0xF;
                psChipDetails->u32RamSize       = 3;
            }
        }

        psChipDetails->u32SupportedFirmware = (
            (psChipDetails->u32RamSize              << 16) |
            (u32FlashSize                           << 24) |
            (CHIP_ID_PART(psChipDetails->u32ChipId) >> 12));

        psChipDetails->u32RamSize       = ((psChipDetails->u32RamSize + 1) * 8) * 1024;
        u32FlashSize                    = ((u32FlashSize + 1) * 32) * 1024;

        DBG_vPrintf(TRACE_PROGRAMMER, "Internal Flash\n");
        if ((eStatus = ePRG_FlashAddDevice(psContext, "Internal Flash", u32FlashSize, FLASH_MANUFACTURER_JN516X, FLASH_DEVICE_JN516X, 0)) != E_PRG_OK)
        {
            return eStatus;
        }

        DBG_vPrintf(TRACE_PROGRAMMER, " RAM size:     %dk\n", psChipDetails->u32RamSize / 1024);
        DBG_vPrintf(TRACE_PROGRAMMER, " Flash size:   %dk\n", psChipDetails->asFlashes[0].u32FlashSize / 1024);
        DBG_vPrintf(TRACE_PROGRAMMER, " EEPROM size:  %d Segments (64 Bytes per Segment)\n", psChipDetails->u32EepromSize/64);
        DBG_vPrintf(TRACE_PROGRAMMER, " Bootloader version 0x%08x\n", psChipDetails->u32BootloaderVersion);
        DBG_vPrintf(TRACE_PROGRAMMER, " Supported firmware 0x%08x\n", psChipDetails->u32SupportedFirmware);

    }
    else
    {
        if (eBL_MemoryRead(psContext, JN514X_ROM_ID_ADDR, sizeof(uint32_t), sizeof(uint32_t), au8Buffer) != E_PRG_OK)
        {
            DBG_vPrintf(TRACE_PROGRAMMER, "Error Reading ROM ID\n");
            return E_PRG_ERROR;
        }
        else
        {
            memcpy(&psChipDetails->u32SupportedFirmware, au8Buffer, sizeof(uint32_t));
        }
    }

    switch (CHIP_ID(psChipDetails->u32ChipId))
    {
        /* Handle JN5148 variants */
        case (CHIP_ID(CHIP_ID_JN5148_REV2A)):
            switch(CHIP_ID_WITH_METAL_MASK(psChipDetails->u32ChipId))
            {
                case (CHIP_ID_WITH_METAL_MASK(CHIP_ID_JN5148_REV2A)):
                case (CHIP_ID_WITH_METAL_MASK(CHIP_ID_JN5148_REV2B)):
                case (CHIP_ID_WITH_METAL_MASK(CHIP_ID_JN5148_REV2C)):
                    psChipDetails->pcChipName = "JN5148-001";
                    break;
                case (CHIP_ID_WITH_METAL_MASK(CHIP_ID_JN5148_REV2D)):
                    psChipDetails->pcChipName = "JN5148-J01";
                    break;
                case (CHIP_ID_WITH_METAL_MASK(CHIP_ID_JN5148_REV2E)):
                    psChipDetails->pcChipName = "JN5148-Z01";
                    break;
            }
            break;

        /* JN5142 is special and has different chip part numbers for each revision */
        case (CHIP_ID(CHIP_ID_JN5142_REV1A)):
        case (CHIP_ID(CHIP_ID_JN5142_REV1B)):
            psChipDetails->pcChipName = "JN5142";
            break;
        case (CHIP_ID(CHIP_ID_JN5142_REV1C)):
            psChipDetails->pcChipName = "JN5142-J01";
            break;

        /* Handle JN5168 variants */
        case (CHIP_ID(CHIP_ID_JN5168)):
            if (psChipDetails->asFlashes[0].u32FlashSize == (64 * 1024))
            {
                psChipDetails->pcChipName = "JN5161";
            }
            else if (psChipDetails->asFlashes[0].u32FlashSize == (160 * 1024))
            {
                psChipDetails->pcChipName = "JN5164";
            }
            else if (psChipDetails->asFlashes[0].u32FlashSize == (256 * 1024))
            {
                psChipDetails->pcChipName = "JN5168";
            }
            else
            {
               psChipDetails->pcChipName = "JN516x";
            }
            break;
            
        /* Handle JN5169 variants */
        case (CHIP_ID(CHIP_ID_JN5169)):
            psChipDetails->pcChipName = "JN5169";
            break;

        /* Handle JN5179 variants */
        case (CHIP_ID(CHIP_ID_JN5172)):
            switch(psChipDetails->u32ChipId)
            {
                case(CHIP_ID_JN5172_D):
                    psChipDetails->pcChipName = "JN5172_D";
                    break;
                case(CHIP_ID_JN5172_LR):
                    psChipDetails->pcChipName = "JN5172_LR";
                    break;
                case(CHIP_ID_JN5172_HR):
                    psChipDetails->pcChipName = "JN5172_HR";
                    break;
                case(CHIP_ID_JN5179):
                    if (psChipDetails->asFlashes[0].u32FlashSize == (160 * 1024))
                    {
                        psChipDetails->pcChipName = "JN5174";      break;
                    }
                    else if (psChipDetails->asFlashes[0].u32FlashSize == (256 * 1024))
                    {
                        psChipDetails->pcChipName = "JN5178";      break;
                    }
                    else if (psChipDetails->asFlashes[0].u32FlashSize == (512 * 1024))
                    {
                        psChipDetails->pcChipName = "JN5179";      break;
                    }
                    else
                    {
                        psChipDetails->pcChipName = "JN517x";      
                    }
                    break;

                default:
                    psChipDetails->pcChipName = "Unknown";
                    break;
            }
            break;

        default:   psChipDetails->pcChipName = "Unknown";     break;
    }

    DBG_vPrintf(TRACE_PROGRAMMER, "Chip ID: 0x%08X\n", psContext->sChipDetails.u32ChipId);
    DBG_vPrintf(TRACE_PROGRAMMER, "Chip Name: %s\n", psContext->sChipDetails.pcChipName);

    return E_PRG_OK;
}


static teStatus ePRG_ChipGetFlashId(tsPRG_Context *psContext)
{
    teStatus eStatus;
    uint8_t u8ManufacturerID;
    uint8_t u8DeviceID;
    const char *pcName;
    uint32_t u32FlashSize;

    tsChipDetails *psChipDetails = &psContext->sChipDetails;

    DBG_vPrintf(TRACE_PROGRAMMER, "Get Flash ID\n");

    if(psChipDetails == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }

    if ((eStatus = eBL_DiscoverFlash(psContext, 0, &u8ManufacturerID, &u8DeviceID, &pcName, &u32FlashSize)) != E_PRG_OK)
    {
        DBG_vPrintf(TRACE_PROGRAMMER, "No flash devices discovered\n");
        if( (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
            (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
            (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
        )
        {
            DBG_vPrintf(TRACE_PROGRAMMER, "This is OK ....\n");
            /* It is expected behaviour to not find a SPI flash connected to the '68, '69 and '72 */
            return E_PRG_OK;
        }
        return ePRG_SetStatus(psContext, eStatus, "discovering flash device");
    }

    return ePRG_FlashAddDevice(psContext, pcName, u32FlashSize, u8ManufacturerID, u8DeviceID, 0);
}


static teStatus ePRG_ChipGetMacAddress(tsPRG_Context *psContext)
{
    teStatus eStatus;
    uint32_t au32InvalidMac[2] = {0xffffffff, 0xffffffff};
    tsChipDetails *psChipDetails = &psContext->sChipDetails;

    if(psChipDetails == NULL)
    {
        return E_PRG_NULL_PARAMETER;
    }

    switch(CHIP_ID_PART(psChipDetails->u32ChipId))
    {
        case CHIP_ID_PART(CHIP_ID_JN5148_REV2A):
            DBG_vPrintf(TRACE_PROGRAMMER, "JN5148 ");
            switch (CHIP_ID_VERSION(psChipDetails->u32ChipId))
            {
                case CHIP_ID_VERSION(CHIP_ID_JN5148_REV2D):
                case CHIP_ID_VERSION(CHIP_ID_JN5148_REV2E):
                    DBG_vPrintf(TRACE_PROGRAMMER, "multi-image bootloader\n");
                    eStatus = eBL_FlashRead(psContext, JN514X_MIB_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
                    break;

                default:
                    DBG_vPrintf(TRACE_PROGRAMMER, "single image bootloader\n");
                    eStatus = eBL_FlashRead(psContext, JN514X_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
                    break;
            }
            break;

        case CHIP_ID_PART(CHIP_ID_JN5142_REV1A):
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1B):
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1C):
            eStatus = eBL_FlashRead(psContext, JN514X_MIB_MAC_ADDRESS_LOCATION, 8, psChipDetails->au8MacAddress);
            break;

        case CHIP_ID_PART(CHIP_ID_JN5168):
        case CHIP_ID_PART(CHIP_ID_JN5169):
        case CHIP_ID_PART(CHIP_ID_JN5172):
            /* First we read the customer specific MAC address, and if its not all F's, we use that */
        {
            uint32_t u32Words[2] = {0x0, 0x0};

            eStatus = eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_MAC_CUSTOMER_ADDRESS, sizeof(u32Words), sizeof(uint32_t), (uint8_t*)&u32Words);

            DBG_vPrintf(TRACE_PROGRAMMER, "Customer MAC Address: %08X%08X\n", u32Words[0], u32Words[1]);

            /* If its all F's, read factory assigned MAC */
            if(memcmp(u32Words, au32InvalidMac, sizeof(au32InvalidMac)) == 0)
            {
                DBG_vPrintf(TRACE_PROGRAMMER, "No customer MAC address - using factory\n");
                eStatus = eBL_MemoryRead(psContext, JN516X_INDEX_SECTOR_MAC_FACTORY_ADDRESS, sizeof(u32Words), sizeof(uint32_t), (uint8_t*)&u32Words);

                DBG_vPrintf(TRACE_PROGRAMMER, "Factory MAC Address: %08X%08X\n", u32Words[0], u32Words[1]);
            }
            // Convert into byte array.
            psChipDetails->au8MacAddress[0] = u32Words[0] >> 24;
            psChipDetails->au8MacAddress[1] = u32Words[0] >> 16;
            psChipDetails->au8MacAddress[2] = u32Words[0] >> 8;
            psChipDetails->au8MacAddress[3] = u32Words[0] >> 0;
            psChipDetails->au8MacAddress[4] = u32Words[1] >> 24;
            psChipDetails->au8MacAddress[5] = u32Words[1] >> 16;
            psChipDetails->au8MacAddress[6] = u32Words[1] >> 8;
            psChipDetails->au8MacAddress[7] = u32Words[1] >> 0;
            break;
        }

        default:
            return E_PRG_ERROR;
    }

    return eStatus;
}

static teStatus ePRG_ChipGetEEPROMenable(tsPRG_Context *psContext)
{
    tsChipDetails *psChipDetails;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;

    if(
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5168)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5169)) ||
        (CHIP_ID_PART(psChipDetails->u32ChipId) == CHIP_ID_PART(CHIP_ID_JN5172))
    )
    {
        DBG_vPrintf(TRACE_PROGRAMMER, "EEPROM is available\n");
        psChipDetails->boEEPpresent = TRUE;
    }
    else
    {
        DBG_vPrintf(TRACE_PROGRAMMER, "EEPROM is NOT available\n");
        psChipDetails->boEEPpresent = FALSE;
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


static teStatus ePRG_ChipGetFlashProgrammerExtensionDetails(tsPRG_Context *psContext)
{
    tsChipDetails *psChipDetails;

    psChipDetails = &psContext->sChipDetails;

    switch(CHIP_ID_PART(psChipDetails->u32ChipId))
    {
        case CHIP_ID_PART(CHIP_ID_JN5168):
            DBG_vPrintf(TRACE_PROGRAMMER, "Setting JN5168 flash programmer extension\n");
            psContext->pu8FlashProgrammerExtensionStart    = FLASHPROGRAMMEREXTENSION_JN5168_BIN;
            psContext->u32FlashProgrammerExtensionLength   = FLASHPROGRAMMEREXTENSION_JN5168_BIN_LEN;
            psContext->u32BootloaderEntry                  = JN516X_BOOTLOADER_ENTRY;
            break;

        case CHIP_ID_PART(CHIP_ID_JN5169):
            DBG_vPrintf(TRACE_PROGRAMMER, "Setting JN5169 flash programmer extension\n");
            psContext->pu8FlashProgrammerExtensionStart    = FLASHPROGRAMMEREXTENSION_JN5169_BIN;
            psContext->u32FlashProgrammerExtensionLength   = FLASHPROGRAMMEREXTENSION_JN5169_BIN_LEN;
            psContext->u32BootloaderEntry                  = JN516X_BOOTLOADER_ENTRY;
            break;

        case CHIP_ID_PART(CHIP_ID_JN5172):
            DBG_vPrintf(TRACE_PROGRAMMER, "Setting JN5172 flash programmer extension\n");
            psContext->pu8FlashProgrammerExtensionStart    = FLASHPROGRAMMEREXTENSION_JN5179_BIN;
            psContext->u32FlashProgrammerExtensionLength   = FLASHPROGRAMMEREXTENSION_JN5179_BIN_LEN;
            psContext->u32BootloaderEntry                  = JN517X_BOOTLOADER_ENTRY;
            break;
        default:
            /* Extension not available - this may be ok, depending on what operations are attempted. */
            DBG_vPrintf(TRACE_PROGRAMMER, "No flash programmer extension available\n");
            break;
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


static teStatus ePRG_FlashProgrammerExtensionLoad(tsPRG_Context *psContext, const char *pcOperation)
{
    /* Attempt to load the flash prorgammer extension for this device */
    tsPRG_Context sContext;
    teStatus eStatus;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }
    if ((psContext->pu8FlashProgrammerExtensionStart == NULL) || (psContext->u32FlashProgrammerExtensionLength == 0))
    {
        return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_OPERATION, pcOperation);
    }

    DBG_vPrintf(TRACE_PROGRAMMER, "Loading flash programmer extension\n");

    /* Copy context data for loading the extension programmer */
    sContext = *psContext;

    /* get flash start */
    if ((eStatus = ePRG_FwGetInfo(&sContext, psContext->pu8FlashProgrammerExtensionStart, psContext->u32FlashProgrammerExtensionLength)) != E_PRG_OK)
    {
        /* Error with file. FW module has displayed error so just exit. */
        return ePRG_SetStatus(psContext, eStatus, "loading extension binary");
    }

    if ((eStatus = ePRG_MemoryLoadExecute(&sContext, NULL, ePRG_ConfirmAlways, NULL)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "loading extension binary into RAM");
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


static teStatus ePRG_FlashProgrammerExtensionReturn(tsPRG_Context *psContext)
{
    /* Return to the bootloader */
    uint32_t u32BaudRate;
    teStatus eStatus;
    tsChipDetails *psChipDetails;

    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    psChipDetails = &psContext->sChipDetails;

    DBG_vPrintf(TRACE_PROGRAMMER, "returning to bootloader\n");

    if ((eStatus = eBL_MemoryExecute(psContext, psContext->u32BootloaderEntry)) != E_PRG_OK)
    {
        return ePRG_SetStatus(psContext, eStatus, "executing bootloader");
    }

    if(CHIP_ID_PART(psChipDetails->u32ChipId) != CHIP_ID_PART(CHIP_ID_JN5172))
    {
        // not required - Bootloader doesn't reset itself after FLASH extension exit
        u32BaudRate = psContext->u32BaudRate;

        psContext->u32BaudRate = 38400;

        /* change local port to default speed of built in bootloader */
        if ((eStatus = eUART_SetBaudRate(psContext->iUartFD, &psContext->sUartOptions, 38400)) != E_PRG_OK)
        // if ((eStatus = ePRG_ConnectionUartUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }

        // Wait 500ms for bootloader to switch back to 38400.
        vPRG_WaitMs(500);

        psContext->u32BaudRate = u32BaudRate;

        /* Go back to selected speed */
        if ((eStatus = eUART_SetBaudRate(psContext->iUartFD, &psContext->sUartOptions, u32BaudRate)) != E_PRG_OK)
        // if ((eStatus = ePRG_ConnectionUpdate(psContext, &psPriv->sConnection)) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, eStatus, "selecting baud rate");
        }
    }

    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


static teStatus ePRG_SetUpImage(tsPRG_Context *psContext, tsFW_Info *psFWImage, tsChipDetails *psChipDetails)
{
    /* Depending on chip type, we may need to copy the MAC address into the firmware image
     * Also, some images have a 4 byte header in the binary that needs stripping off, so do that here
     * by adjusting start point and length to suit */
    switch(CHIP_ID_PART(psChipDetails->u32ChipId))
    {
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1A):
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1B):
        case CHIP_ID_PART(CHIP_ID_JN5142_REV1C):
            DBG_vPrintf(TRACE_PROGRAMMER, "multi-image bootloader\n");
            memcpy(&psFWImage->pu8ImageData[JN514X_MIB_MAC_ADDRESS_LOCATION], psChipDetails->au8MacAddress, 8);
            break;

        case CHIP_ID_PART(CHIP_ID_JN5148_REV2A):
            DBG_vPrintf(TRACE_PROGRAMMER, "JN5148 ");
            switch (CHIP_ID_VERSION(psChipDetails->u32ChipId))
            {
                case CHIP_ID_VERSION(CHIP_ID_JN5148_REV2D):
                case CHIP_ID_VERSION(CHIP_ID_JN5148_REV2E):
                    DBG_vPrintf(TRACE_PROGRAMMER, "multi-image bootloader\n");
                    memcpy(&psFWImage->pu8ImageData[JN514X_MIB_MAC_ADDRESS_LOCATION], psChipDetails->au8MacAddress, 8);
                    break;

                default:
                    DBG_vPrintf(TRACE_PROGRAMMER, "single image bootloader\n");
                    memcpy(&psFWImage->pu8ImageData[JN514X_MAC_ADDRESS_LOCATION], psChipDetails->au8MacAddress, 8);
                    break;
            }
            break;

        case CHIP_ID_PART(CHIP_ID_JN5168):
        case CHIP_ID_PART(CHIP_ID_JN5169):
        case CHIP_ID_PART(CHIP_ID_JN5172):
            break;

        default:
            return ePRG_SetStatus(psContext, E_PRG_UNSUPPORTED_CHIP, "");
            break;
    }
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}

static teStatus ePRG_ConfirmAlways(void *pvUser, const char *pcTitle, const char *pcText)
{
    return E_PRG_OK;
}

#if 0
static teStatus ePRG_ResetDevice(tsPRG_Context *psContext)
{

    teStatus eStatus;

    DBG_vPrintf(TRACE_PROGRAMMER, "Reset device\n");

    if ((eStatus = eBL_Reset(psContext)) != E_PRG_OK)
    {
        ePRG_SetStatus(psContext, eStatus, "reseting device");

        uint32_t u32Mask = RSTCTRL_CPU_REBOOT_MASK;

        eStatus = eBL_MemoryWrite(psContext, RSTCTRL_REGISTER_ADDRESS, sizeof(u32Mask), sizeof(uint32_t), (uint8_t *)&u32Mask);

        /* Expect no response as device has reset */
        if (eStatus == E_PRG_COMMS_FAILED) {
            return E_PRG_OK;
        }
    }

    return eStatus;

}
#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
