/****************************************************************************
 *
 * MODULE:             Firmware
 *
 * COMPONENT:          $RCSfile: Firmware.c,v $
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
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "portable_endian.h"

#ifdef POSIX
#include <arpa/inet.h>
#include <sys/mman.h>
#elif defined WIN32
#include <winsock2.h>
#endif

#include <programmer.h>

#include "dbg.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_FIRMWARE
#define TRACE_FIRMWARE      TRUE
#else
#define TRACE_FIRMWARE      FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#if defined POSIX
typedef struct
{
    int         iFirmwareFD;
    uint32_t    u32FileSize;
    uint8_t*    pu8Firmware;
} tsFwPrivate;

#elif defined WIN32
typedef struct
{
    HANDLE      hFile;
    HANDLE      hMapping;

    DWORD       dwFileSize;
    
    uint8_t*    pu8Firmware;
} tsFwPrivate;
#endif

typedef struct
{
    uint32_t    u32ROMVersion;
    uint32_t    au32BootImageRecord[4];
    uint32_t    au32MacAddress[2];
    uint32_t    au32EncryptionInitialisationVector[4];
    uint32_t    u32DataSectionInfo;
    uint32_t    u32BssSectionInfo;
    uint32_t    u32WakeUpEntryPoint;
    uint32_t    u32ResetEntryPoint;
    uint8_t     u8TextDataStart;
} __attribute__ ((packed)) tsBL_BinHeaderV2;


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


#if defined POSIX
teStatus ePRG_FwOpen(tsPRG_Context *psContext, char *pcFirmwareFile)
{
    struct stat sb;
    
    if ((!psContext) || (!pcFirmwareFile))
    {
        return E_PRG_NULL_PARAMETER;
    }

    if (psContext->iFirmwareFD > 0)
    {
        if (ePRG_FwClose(psContext) != E_PRG_OK)
        {
            return ePRG_SetStatus(psContext, E_PRG_ERROR, "closing existing file");
        }
    }
    
    psContext->iFirmwareFD = open(pcFirmwareFile, O_RDONLY);
    if (psContext->iFirmwareFD < 0)
    {
        return ePRG_SetStatus(psContext, E_PRG_FAILED_TO_OPEN_FILE, "\"%s\" (%s)", pcFirmwareFile, pcPRG_GetLastErrorMessage(psContext));
    }
    
    DBG_vPrintf(TRACE_FIRMWARE, "opened FD %d\n", psContext->iFirmwareFD);
    
    if (fstat(psContext->iFirmwareFD, &sb) == -1)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "(%s)", pcPRG_GetLastErrorMessage(psContext));
    }
    
    psContext->u32FirmWareFileSize = (uint32_t)sb.st_size;
    DBG_vPrintf(TRACE_FIRMWARE, "Opened file size %d\n", psContext->u32FirmWareFileSize);
    
    psContext->sFirmwareInfo.u32ImageLength = psContext->u32FirmWareFileSize;
    
    /* Copy-on-write, changes are not written to the underlying file. */
    psContext->pu8Firmware = mmap(NULL, psContext->u32FirmWareFileSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, psContext->iFirmwareFD, 0);
    DBG_vPrintf(TRACE_FIRMWARE, "mapped file at %p\n", psContext->pu8Firmware);
    
    if (psContext->pu8Firmware == MAP_FAILED)
    {
        return ePRG_SetStatus(psContext, E_PRG_ERROR, "(%s)", pcPRG_GetLastErrorMessage(psContext));
    }
    
    return ePRG_FwGetInfo(psContext, psContext->pu8Firmware, psContext->sFirmwareInfo.u32ImageLength);
}


teStatus ePRG_FwClose(tsPRG_Context *psContext)
{
    if (!psContext)
    {
        return E_PRG_NULL_PARAMETER;
    }

    if (psContext->pu8Firmware)
    {
	DBG_vPrintf(TRACE_FIRMWARE, "unmapping file at %p\n", psContext->pu8Firmware);
	munmap(psContext->pu8Firmware, psContext->u32FirmWareFileSize);
    }

    if (psContext->iFirmwareFD > 0)
    {
	DBG_vPrintf(TRACE_FIRMWARE, "closing FD %d\n", psContext->iFirmwareFD);
	close(psContext->iFirmwareFD);
	psContext->iFirmwareFD = 0;
    }
        
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}

#elif defined WIN32
#endif /* POSIX */


teStatus ePRG_FwGetInfo(tsPRG_Context *psContext, uint8_t *pu8Firmware, uint32_t u32Length)
{
    tsBL_BinHeaderV2 *psHeader;
    // printf( "RHM fgi 1\n" );    
    if ((!psContext) || (!pu8Firmware))
    {
        return E_PRG_NULL_PARAMETER;
    }
    
    DBG_vPrintf(TRACE_FIRMWARE, "Getting info on file at 0x%p\n", pu8Firmware);
    
    // printf( "RHM fgi 2\n" );
    psHeader = (tsBL_BinHeaderV2 *)pu8Firmware;
    
    psContext->sFirmwareInfo.u32ImageLength = u32Length;

    // dump( (char *)psHeader->au32BootImageRecord, 12 );
    
    // JN5148-J01 onwards uses multiimage bootloader - check for it's magic number.   
    if ((ntohl(psHeader->au32BootImageRecord[0]) == 0x12345678) &&
        (ntohl(psHeader->au32BootImageRecord[1]) == 0x11223344) &&
        (ntohl(psHeader->au32BootImageRecord[2]) == 0x55667788))
    {
        // printf( "RHM fgi 3\n" );    
        psContext->sFirmwareInfo.u32ROMVersion                = ntohl(psHeader->u32ROMVersion);
        
        psContext->sFirmwareInfo.u32TextSectionLoadAddress    = 0x04000000 + (((ntohl(psHeader->u32DataSectionInfo)) >> 16) * 4);
        psContext->sFirmwareInfo.u32TextSectionLength         = (((ntohl(psHeader->u32DataSectionInfo)) & 0x0000FFFF) * 4);
        psContext->sFirmwareInfo.u32BssSectionLoadAddress     = 0x04000000 + (((ntohl(psHeader->u32BssSectionInfo)) >> 16) * 4);
        psContext->sFirmwareInfo.u32BssSectionLength          = (((ntohl(psHeader->u32BssSectionInfo)) & 0x0000FFFF) * 4);

        psContext->sFirmwareInfo.u32ResetEntryPoint           = ntohl(psHeader->u32ResetEntryPoint);
        psContext->sFirmwareInfo.u32WakeUpEntryPoint          = ntohl(psHeader->u32WakeUpEntryPoint);
        
        /* Pointer to and length of image for flash */
        psContext->sFirmwareInfo.pu8ImageData                 = (uint8_t*)&(psHeader->au32BootImageRecord[0]);
        psContext->sFirmwareInfo.u32ImageLength              -= sizeof(psHeader->u32ROMVersion);
        DBG_vPrintf(TRACE_FIRMWARE, "Image length: %d\n", psContext->sFirmwareInfo.u32ImageLength);
        
        psContext->sFirmwareInfo.u32MacAddressLocation        = 0x10;
        
        /* Pointer to text section in image for RAM */
        psContext->sFirmwareInfo.pu8TextData                  = &(psHeader->u8TextDataStart);

        /* Sanity check the image data */
        if (psContext->sFirmwareInfo.u32TextSectionLength > u32Length)
        {
            /* The text section appears to be larger than the file */
            DBG_vPrintf(TRACE_FIRMWARE, "Text section is larger (%d bytes) than file (%d bytes)\n", 
                        psContext->sFirmwareInfo.u32TextSectionLength, u32Length);
            return ePRG_SetStatus(psContext, E_PRG_INVALID_FILE, ", text section is larger than file");;
        }
        
        DBG_vPrintf(TRACE_FIRMWARE, "Header size:           %d\n", sizeof(tsBL_BinHeaderV2));
        DBG_vPrintf(TRACE_FIRMWARE, "u32ROMVersion:         0x%08x\n", psContext->sFirmwareInfo.u32ROMVersion);
        DBG_vPrintf(TRACE_FIRMWARE, "u32DataSectionInfo:    0x%08x\n", ntohl(psHeader->u32DataSectionInfo));
        DBG_vPrintf(TRACE_FIRMWARE, "u32TextSectionLength:  0x%08x\n", psContext->sFirmwareInfo.u32TextSectionLength);
        
        psContext->sFirmwareInfo.sFlags.bRawImage             = 0;
    }
    // little endian - check for it's magic number
    else if ((le32toh(psHeader->au32BootImageRecord[0]) == 0x12345678) &&
             (le32toh(psHeader->au32BootImageRecord[1]) == 0x11223344) &&
             (le32toh(psHeader->au32BootImageRecord[2]) == 0x55667788))
    {
        // printf( "RHM fgi 4\n" );    
        psContext->sFirmwareInfo.u32ROMVersion                = le32toh(psHeader->u32ROMVersion);

        // writing u16's as u32 and LE platform means we have to flip the read back order compared to JN516x
        psContext->sFirmwareInfo.u32TextSectionLoadAddress    = 0x20000000 + (((le32toh(psHeader->u32DataSectionInfo)) & 0x0000FFFF) * 4);
        psContext->sFirmwareInfo.u32TextSectionLength         = (((le32toh(psHeader->u32DataSectionInfo)) >> 16) * 4);

        // writing u16's as u32 and LE platform means we have to flip the read back order compared to JN516x
        psContext->sFirmwareInfo.u32BssSectionLoadAddress     = 0x20000000 + (((le32toh(psHeader->u32BssSectionInfo)) & 0x0000FFFF) * 4);
        psContext->sFirmwareInfo.u32BssSectionLength          = (((le32toh(psHeader->u32BssSectionInfo)) >> 16) * 4);

        psContext->sFirmwareInfo.u32ResetEntryPoint           = le32toh(psHeader->u32ResetEntryPoint);
        psContext->sFirmwareInfo.u32WakeUpEntryPoint          = le32toh(psHeader->u32WakeUpEntryPoint);

        /* Pointer to and length of image for flash */
        psContext->sFirmwareInfo.pu8ImageData                 = (uint8_t*)&(psHeader->au32BootImageRecord[0]);
        psContext->sFirmwareInfo.u32ImageLength              -= sizeof(psHeader->u32ROMVersion);
        DBG_vPrintf(TRACE_FIRMWARE, "Image length: %d\n", psContext->sFirmwareInfo.u32ImageLength);

        psContext->sFirmwareInfo.u32MacAddressLocation        = 0x10;

        /* Pointer to text section in image for RAM */
        psContext->sFirmwareInfo.pu8TextData                  = &(psHeader->u8TextDataStart);

        /* Sanity check the image data */
        if (psContext->sFirmwareInfo.u32TextSectionLength > u32Length)
        {
            /* The text section appears to be larger than the file */
            DBG_vPrintf(TRACE_FIRMWARE, "Text section is larger (%d bytes) than file (%d bytes)\n",
                        psContext->sFirmwareInfo.u32TextSectionLength, u32Length);
            return ePRG_SetStatus(psContext, E_PRG_INVALID_FILE, ", text section is larger than file");;
        }

        DBG_vPrintf(TRACE_FIRMWARE, "Header size:               %d\n", sizeof(tsBL_BinHeaderV2));
        DBG_vPrintf(TRACE_FIRMWARE, "u32TextSectionLoadAddress: 0x%08x\n", psContext->sFirmwareInfo.u32TextSectionLoadAddress);
        DBG_vPrintf(TRACE_FIRMWARE, "u32TextSectionLength:      0x%08x\n", psContext->sFirmwareInfo.u32TextSectionLength);
        DBG_vPrintf(TRACE_FIRMWARE, "u32ResetEntryPoint:        0x%08x\n", psContext->sFirmwareInfo.u32ResetEntryPoint);

        DBG_vPrintf(TRACE_FIRMWARE, "u32ROMVersion:             0x%08x\n", psContext->sFirmwareInfo.u32ROMVersion);
        DBG_vPrintf(TRACE_FIRMWARE, "u32DataSectionInfo:        0x%08x\n", le32toh(psHeader->u32DataSectionInfo));
        
        psContext->sFirmwareInfo.sFlags.bRawImage             = 0;
    }
    // little endian - check for it's magic number
    else if ((le32toh(psHeader->au32BootImageRecord[0]) == 0x12345678) &&
             (le32toh(psHeader->au32BootImageRecord[1]) == 0x22334455) &&
             (le32toh(psHeader->au32BootImageRecord[2]) == 0x66778899))
    {
        // printf( "RHM fgi 5\n" );    
        psContext->sFirmwareInfo.u32ROMVersion                = le32toh(psHeader->u32ROMVersion);

        // writing u16's as u32 and LE platform means we have to flip the read back order compared to JN516x
        psContext->sFirmwareInfo.u32TextSectionLoadAddress    = 0x20000000 + (((le32toh(psHeader->u32DataSectionInfo)) & 0x0000FFFF) * 4);
        psContext->sFirmwareInfo.u32TextSectionLength         = (((le32toh(psHeader->u32DataSectionInfo)) >> 16) * 4);

        // writing u16's as u32 and LE platform means we have to flip the read back order compared to JN516x
        psContext->sFirmwareInfo.u32BssSectionLoadAddress     = 0x20000000 + (((le32toh(psHeader->u32BssSectionInfo)) & 0x0000FFFF) * 4);
        psContext->sFirmwareInfo.u32BssSectionLength          = (((le32toh(psHeader->u32BssSectionInfo)) >> 16) * 4);

        psContext->sFirmwareInfo.u32ResetEntryPoint           = le32toh(psHeader->u32ResetEntryPoint);
        psContext->sFirmwareInfo.u32WakeUpEntryPoint          = le32toh(psHeader->u32WakeUpEntryPoint);

        /* Pointer to and length of image for flash */
        psContext->sFirmwareInfo.pu8ImageData                 = pu8Firmware;
        DBG_vPrintf(TRACE_FIRMWARE, "Image length: %d\n", psContext->sFirmwareInfo.u32ImageLength);

        psContext->sFirmwareInfo.u32MacAddressLocation        = 0x14;

        /* Pointer to text section in image for RAM */
        psContext->sFirmwareInfo.pu8TextData                  = &(psHeader->u8TextDataStart);

        /* Sanity check the image data */
        if (psContext->sFirmwareInfo.u32TextSectionLength > u32Length)
        {
            /* The text section appears to be larger than the file */
            DBG_vPrintf(TRACE_FIRMWARE, "Text section is larger (%d bytes) than file (%d bytes)\n",
                        psContext->sFirmwareInfo.u32TextSectionLength, u32Length);
            return ePRG_SetStatus(psContext, E_PRG_INVALID_FILE, ", text section is larger than file");;
        }

        DBG_vPrintf(TRACE_FIRMWARE, "Header size:               %d\n", sizeof(tsBL_BinHeaderV2));
        DBG_vPrintf(TRACE_FIRMWARE, "u32TextSectionLoadAddress: 0x%08x\n", psContext->sFirmwareInfo.u32TextSectionLoadAddress);
        DBG_vPrintf(TRACE_FIRMWARE, "u32TextSectionLength:      0x%08x\n", psContext->sFirmwareInfo.u32TextSectionLength);
        DBG_vPrintf(TRACE_FIRMWARE, "u32ResetEntryPoint:        0x%08x\n", psContext->sFirmwareInfo.u32ResetEntryPoint);

        DBG_vPrintf(TRACE_FIRMWARE, "u32ROMVersion:             0x%08x\n", psContext->sFirmwareInfo.u32ROMVersion);
        DBG_vPrintf(TRACE_FIRMWARE, "u32DataSectionInfo:        0x%08x\n", le32toh(psHeader->u32DataSectionInfo));
        
        psContext->sFirmwareInfo.sFlags.bRawImage             = 0;
    }
    else // Not a valid image, assume we will program it raw.
    {
        // printf( "RHM fgi 6\n" );    
        /* Pointer to and length of image for flash */
        psContext->sFirmwareInfo.pu8ImageData                 = pu8Firmware;
        psContext->sFirmwareInfo.sFlags.bRawImage             = 1;
        DBG_vPrintf(TRACE_FIRMWARE, "Firmware file not recognised as valid image - programming raw\n");
    }
    
    return ePRG_SetStatus(psContext, E_PRG_OK, "");
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

