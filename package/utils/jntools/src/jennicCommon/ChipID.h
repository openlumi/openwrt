/*****************************************************************************
 *
 * MODULE:              Chip ID's
 *
 * COMPONENT:           $RCSfile: ChipID.h,v $
 *
 * VERSION:             $Name:  $
 *
 * REVISION:            $Revision: 1.25 $
 *
 * DATED:               $Date: 2009/07/15 08:16:39 $
 *
 * STATUS:              $State: Exp $
 *
 * AUTHOR:              Lee Mitchell
 *
 * DESCRIPTION:
 *
 *
 * LAST MODIFIED BY:    $Author: lmitch $
 *                      $Modtime: $
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

 * Copyright NXP B.V. 2014. All rights reserved
 *
 ***************************************************************************/

#ifndef  CHIP_ID_H_INCLUDED
#define  CHIP_ID_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Jennic/NXP Chip ID's */
#define CHIP_ID_MANUFACTURER_ID_MASK    0x00000fff
#define CHIP_ID_PART_MASK               0x003ff000
#define CHIP_ID_MASK_VERSION_MASK       0x0fc00000
#define CHIP_ID_REV_MASK                0xf0000000

#define CHIP_ID_JN5121_REV1A            0x00000000  /* ZED001               JN5121 Development      */
#define CHIP_ID_JN5121_REV2A            0x10000000  /* ZED002               JN5121 Development      */
#define CHIP_ID_JN5121_REV3A            0x20000000  /* ZED003               JN5121 Production       */

#define CHIP_ID_JN5131_REV1A            0x00001000  /* Alberich             Never Produced          */

#define CHIP_ID_JN5139_REV1A            0x00002000  /* BAL01                JN5139R                 */
#define CHIP_ID_JN5139_REV2A            0x10002000  /* BAL02A               JN5139R1                */
#define CHIP_ID_JN5139_REV2B            0x10002000  /* BAL02B               Test Chip Only          */
#define CHIP_ID_JN5139_REV2C            0x10802000  /* BAL02C               JN5139T01 & JN5139J01   */

#define CHIP_ID_JN5147_REV1A            0x00004686  /* JAG01                JN5147                  */

#define CHIP_ID_JN5148_REV2A            0x10004686  /* JAG02A               JN5148                  */
#define CHIP_ID_JN5148_REV2B            0x10404686  /* JAG02B               JN5148                  */
#define CHIP_ID_JN5148_REV2C            0x10804686  /* JAG02C               JN5148                  */
#define CHIP_ID_JN5148_REV2D            0x10C04686  /* JAG02D               JN5148T01 & JN5148J01   */
#define CHIP_ID_JN5148_REV2E            0x11004686  /* JAG02E               JN5148Z01               */

#define CHIP_ID_JN5142_REV1A            0x00005686  /* PUM01A               JN5142                  */
#define CHIP_ID_JN5142_REV1B            0x00425686  /* PUM01B               JN5142                  */
#define CHIP_ID_JN5142_REV1C            0x00845686  /* PUM01C               JN5142J01               */

#define CHIP_ID_COG03                   0x00006686  /* Cougar COG03                                 */
#define CHIP_ID_COG04                   0x00007686  /* Cougar COG04                                 */
#define CHIP_ID_JN5168                  0x00008686  /* Cougar COG05                                 */
#define CHIP_ID_JN5168_COG07A           0x10008686  /* Cougar COG07A                                */
#define CHIP_ID_JN5168_COG07B           0x10408686  /* Cougar COG07B                                */

#define CHIP_ID_JN5169                  0x0000b686
// variants
#define CHIP_ID_JN5169_LR               0x0000b686
#define CHIP_ID_JN5169_D               	0x0040b686
#define CHIP_ID_JN5169_HR               0x0080b686
#define CHIP_ID_JN5169_VHR              0x00c0b686
#define CHIP_ID_JN5169_DONGLE           0x6000b686  /* RHM */
#define CHIP_ID_JN5169_LUMI             0x0100b686  /* Lumi Gateway */

#define CHIP_ID_JN5169_COG09A           0x00009686  /* Cougar COG09A                                */
#define CHIP_ID_JN5169_COG10A           0x10009686  /* Cougar COG10A                                */
#if (defined JN5172_FPGA)
#define CHIP_ID_JN5172                  0x0800a686  /* Firefly FPGA                                 */
#else
#define CHIP_ID_JN5172                  0x0000a686  /* Firefly  */
// variants
#define CHIP_ID_JN5172_D                0x0000a686
#define CHIP_ID_JN5172_LR               0x0040a686
#define CHIP_ID_JN5172_HR               0x0080a686

#define CHIP_ID_JN5179                  0x1000a686  /* MRA2 */

#endif

/* Macro to get components from the chip ID */
#define CHIP_ID_MANUFACTURER(a)         (a & CHIP_ID_MANUFACTURER_ID_MASK)
#define CHIP_ID_PART(a)                 (a & CHIP_ID_PART_MASK)
#define CHIP_ID_VERSION(a)              (a & CHIP_ID_MASK_VERSION_MASK)
#define CHIP_ID_REVISION(a)             (a & CHIP_ID_REV_MASK)
// updated this to cope with the mask revisions in 69/72
// this macro added to cope with 4x
#define CHIP_ID_WITH_METAL_MASK(a)      (a & ~CHIP_ID_REV_MASK)
#define CHIP_ID(a)                      (a & ~CHIP_ID_REV_MASK & ~CHIP_ID_MASK_VERSION_MASK)


/* SPI Flash device ID's  */
#define	FLASH_ST_M25P10			0x1010	/* ST microelectronics M25P10		*/
#define	FLASH_ST_M25P40			0x1212	/* ST microelectronics M25P40		*/
#define	FLASH_SST_SST25VF010A	0xbf49	/* SST SST25VF010A					*/
#define FLASH_INTERNAL			0xccee	/* Internal on-chip flash 			*/



/* These bits within the bootloader version are used to flag capabilities of the bootloader. 
 * The flag being present means the running bootloader can perform this task */
#define BOOTLOADER_CAPABILITY_INDEX_SECTOR(a)   (a & (1 << 24))
#define BOOTLOADER_CAPABILITY_EEPROM_ACCESS(a)  (a & (1 << 25))

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* CHIP_ID_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
