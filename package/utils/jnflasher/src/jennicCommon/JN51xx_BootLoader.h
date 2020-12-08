/****************************************************************************
 *
 * MODULE:             JN51xx_BootLoader
 *
 * COMPONENT:          $RCSfile: JN51xx_BootLoader.h,v $
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.1 $
 *
 * DATED:              $Date: 2008/10/17 10:22:11 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell
 *
 * DESCRIPTION:
 *
 * LAST MODIFIED BY:   $Author: lmitch $
 *                     $Modtime: $
 *
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

#ifndef  JN51XX_BOOTLOADER_H_INCLUDED
#define  JN51XX_BOOTLOADER_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdint.h>

#include <programmer.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define FLASH_MANUFACTURER_JN516X   0xCC
#define FLASH_DEVICE_JN516X         0xEE


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

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

teStatus eBL_SetBaudrate(tsPRG_Context *psContext, uint32_t u32Baudrate);

teStatus eBL_ChipIdRead(tsPRG_Context *psContext, uint32_t *pu32ChipId, uint32_t *pu32BootloaderVersion);

teStatus eBL_MemoryExecute(tsPRG_Context *psContext, uint32_t u32Address);
teStatus eBL_MemoryRead(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t u8BlockSize, uint8_t *pu8Buffer);
teStatus eBL_MemoryWrite(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t u8BlockSize, uint8_t *pu8Buffer);

teStatus eBL_DiscoverFlash(tsPRG_Context *psContext, uint8_t u8ChipSelect, uint8_t *pu8ManufacturerID, uint8_t *pu8DeviceID, const char **ppcName, uint32_t *pu32FlashSize);
teStatus eBL_FlashSelectDevice(tsPRG_Context *psContext, uint8_t u8ManufacturerID, uint8_t u8DeviceID, uint8_t u8ChipSelect);
teStatus eBL_FlashIdRead(tsPRG_Context *psContext, uint8_t *pu8ManufacturerID, uint8_t *pu8DeviceID, const char **ppcName, uint32_t *pu32FlashSize);

teStatus eBL_FlashStatusRegisterWrite(tsPRG_Context *psContext, uint8_t u8StatusReg);
teStatus eBL_FlashErase(tsPRG_Context *psContext);
teStatus eBL_FlashRead(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer);
teStatus eBL_FlashWrite(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer);

teStatus eBL_EEPROMErase(tsPRG_Context *psContext, int iEraseAll);
teStatus eBL_EEPROMRead(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer);
teStatus eBL_EEPROMWrite(tsPRG_Context *psContext, uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer);

teStatus eBL_IndexSectorWrite(tsPRG_Context *psContext, uint8_t u8Page, uint8_t u8WordNumber, uint32_t au32Data[4]);

teStatus eBL_Reset(tsPRG_Context *psContext);

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* JN51XX_BOOTLOADER_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

