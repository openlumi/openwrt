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

#ifndef  PROGRAMMER_H_INCLUDED
#define  PROGRAMMER_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <stdint.h>
#include <termios.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define FALSE   0
#define TRUE    1

#ifdef WIN32
#ifdef LIBPROGRAMMER_BUILD
#define LIBPROGRAMMER __declspec(dllexport)
#else
#define LIBPROGRAMMER __declspec(dllimport)
#endif /* LIBPROGRAMMER_BUILD */

#else /* WIN32 */

#define LIBPROGRAMMER

#endif /* WIN32 */

#define PRG_MAX_STATUS_LENGTH 1024

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef enum
{
    E_PRG_OK,
    E_PRG_ERROR,
    E_PRG_OUT_OF_MEMORY,
    E_PRG_ERROR_WRITING,
    E_PRG_ERROR_READING,
    E_PRG_FAILED_TO_OPEN_FILE,
    E_PRG_BAD_PARAMETER,
    E_PRG_NULL_PARAMETER,
    E_PRG_INCOMPATIBLE,
    E_PRG_INVALID_FILE,
    E_PRG_UNSUPPORTED_CHIP,
    E_PRG_ABORTED,
    E_PRG_VERIFICATION_FAILED,
    E_PRG_INVALID_TRANSPORT,
    E_PRG_COMMS_FAILED,                 // 14
    E_PRG_UNSUPPORTED_OPERATION,
    E_PRG_FLASH_DEVICE_UNAVAILABLE,
    E_PRG_CRP_SET,
} teStatus;

typedef enum
{
    E_CONNECT_NONE,
    E_CONNECT_SERIAL,
} teConnectionType;


/** Structure describing an available connection */
typedef struct
{
    teConnectionType    eType;
    char *              pcName;

    union
    {
        struct
        {
        } sNone;

        struct
        {
            uint32_t    u32BaudRate;
        } sSerial;
    } uDetails;
} tsConnection;


typedef enum
{
    E_ERASE_EEPROM_NONE,
    E_ERASE_EEPROM_PDM,
    E_ERASE_EEPROM_ALL,
} teEepromErase;

typedef enum
{
    E_DC_JTAG_DEFAULT,                      /**< Use default option */
    E_DC_JTAG_ENABLE,                       /**< JTAG access is enabled */
    E_DC_JTAG_DISABLE_FLASH,                /**< JTAG access to Flash is disabled */
    E_DC_JTAG_DISABLE_CPU,                  /**< JTAG access to CPU is disabled */
    E_DC_JTAG_DISABLE,                      /**< JTAG access is disabled */
} teJtagAccess;

typedef enum
{
    E_DC_VBO_DEFAULT,                       /**< Use default option */
    E_DC_VBO_195,                           /**< Brown out = 1.95V */
    E_DC_VBO_200,                           /**< Brown out = 2.00V */
    E_DC_VBO_210,                           /**< Brown out = 2.10V */
    E_DC_VBO_220,                           /**< Brown out = 2.20V */
    E_DC_VBO_230,                           /**< Brown out = 2.30V */
    E_DC_VBO_240,                           /**< Brown out = 2.40V */
    E_DC_VBO_270,                           /**< Brown out = 2.70V */
    E_DC_VBO_300,                           /**< Brown out = 3.00V */
} teBrownoutVoltage;

typedef enum
{
    E_DC_CRP_DEFAULT,                       /**< Use default option */
    E_DC_CRP_LEVEL0,                        /**< Code protect level 0 (none) */
    E_DC_CRP_LEVEL1,                        /**< Code protect level 1 (UART bootloader flash read protection) */
    E_DC_CRP_LEVEL2,                        /**< Code protect level 2 (UART bootloader access disabled) */
} teCRPLevel;

typedef enum
{
    E_DC_EXTERNAL_FLASH_ENCRYPTED_DEFAULT,  /**< Use default option */
    E_DC_EXTERNAL_FLASH_ENCRYPTED_TRUE,     /**< External flash is encrypted */
    E_DC_EXTERNAL_FLASH_ENCRYPTED_FALSE,    /**< External flash is not encrypted */
} teExternalFlashEncrypted;

typedef enum
{
    E_DC_EXTERNAL_FLASH_LOAD_DEFAULT,       /**< Use default option */
    E_DC_EXTERNAL_FLASH_LOAD_DISABLE,       /**< Load from external flash is disabled */
    E_DC_EXTERNAL_FLASH_LOAD_ENABLE,        /**< Load from external flash is enabled */
} teExternalFlashLoad;

/** Structure containing all available device settings.
 *  Memsetting this to 0 will cause all settings to be default.
 *  Each required setting should be set in the structure before passing it to \ref ePRG_DeviceConfigSet
 */
typedef struct
{
    /** Enable / disable JTAG access to device */
    teJtagAccess eJtag;

    /** Set device brown out voltage */
    teBrownoutVoltage eVbo;

    /** Set device code protection level in UART bootloader */
    teCRPLevel eCRP;

    /** Set whether external flash is encrypted */
    teExternalFlashEncrypted eExternalFlashEncrypted;

    /** Set whether bootloader will search external flash for a bootable image */
    teExternalFlashLoad eExternalFlashLoad;
} tsDeviceConfig;


/** Abstracted firmware information common across bootloaders. */
typedef struct
{
    uint32_t    u32ROMVersion;                  /**< ROM Version that the FW was built for */
    uint32_t    u32TextSectionLoadAddress;      /**< Address to load .text section */
    uint32_t    u32TextSectionLength;           /**< Length (bytes) of .text section */
    uint32_t    u32BssSectionLoadAddress;       /**< Address of start of .bss section */
    uint32_t    u32BssSectionLength;            /**< Length (bytes) of .bss section */
    uint32_t    u32WakeUpEntryPoint;            /**< Address of wake up (warm start) entry point */
    uint32_t    u32ResetEntryPoint;             /**< Address of rest (cold start) entry point */

    uint8_t*    pu8ImageData;                   /**< Pointer to loaded image data for Flash */
    uint32_t    u32ImageLength;                 /**< Length (bytes) of image for Flash */

    uint32_t    u32MacAddressLocation;          /**< Offset in file of MAC address */
    uint8_t*    pu8TextData;                    /**< Pointer to loaded .text section for RAM */
    
    struct
    {
        unsigned        bRawImage : 1;          /**< Flag that the image is considered to be a raw binary file
                                                  *  i.e. not a valid firmware image */
    } sFlags;
} tsFW_Info;


/** Flash device information */
typedef struct
{
    const char* pcFlashName;                    /**< Human readable name of the flash */
    uint32_t    u32FlashSize;                   /**< Flash size (bytes) */
    uint8_t     u8ManufacturerID;               /**< Flash manufacturer ID */
    uint8_t     u8DeviceID;                     /**< Flash device ID */
    uint8_t     u8ChipSelect;                   /**< Which SPI cheip select for this flash */
} tsFlashDetails;


/** Structure Contatining Imformation about connected device */
typedef struct
{
    uint32_t            u32ChipId;                      /**< 32 bit Chip ID */
    uint32_t            u32SupportedFirmware;           /**< Required ROM version */

    const char*         pcChipName;                     /**< String representing chip name */

    uint32_t            u32NumFlashes;                  /**< Number of flashes available */
    tsFlashDetails*     asFlashes;                      /**< Array of available flashes */

    uint32_t            u32RamSize;                     /**< Ram Size (bytes) */
    uint32_t            u32EepromSize;                  /**< EEPROM size (bytes) */

    uint32_t            u32BootloaderVersion;           /**< Bootloader version (JN516x series) */

    uint8_t             au8MacAddress[8];               /**< MAC Address */

    enum
    {
        E_CHIP_BIG_ENDIAN,                              /**< Chip has a big endian CPU */
        E_CHIP_LITTLE_ENDIAN,                           /**< Chip has a little endian CPU */
    } eEndianness;
    
    uint8_t             boEEPpresent;                   /** EEPROM present flag */
} tsChipDetails;


/** Programmer context structure */
typedef struct
{
    int                 iUartFD;
    struct termios      sUartOptions;
    uint32_t            u32BaudRate;
    
    int                 iFirmwareFD;
    uint32_t            u32FirmWareFileSize;
    uint8_t *           pu8Firmware;
    tsFW_Info           sFirmwareInfo;          /**< Loaded firmware for the context */
    
    tsChipDetails       sChipDetails;           /**< Information about the connected device */
    tsDeviceConfig      sDeviceConfig;          /**< Device configuration */
// SHAIT    void *              pvPrivate;              /**< Private context information */
    
    uint32_t            u32SelectedFlash;
    uint8_t *           pu8FlashProgrammerExtensionStart;
    uint32_t            u32FlashProgrammerExtensionLength;
    uint32_t            u32BootloaderEntry;
    uint32_t            u32FlashOffset;         /**< Offset within flash at which to load / dump an image. Default 0 (start) */
    
    uint32_t            u32EepromOffset;        /**< Offset within eeprom at which to load / dump an image. Default 0 (start) */
    
    struct
    {
        unsigned        bAutoProgramReset : 1;  /**< If possible, automatically assert the program / reset lines. Default true. */
    } sFlags;
    
    char            acStatusMessage[PRG_MAX_STATUS_LENGTH];
    char            acErrorMsgBuffer[PRG_MAX_STATUS_LENGTH];

} tsPRG_Context;


/** Callback function for updating UI with operation progress
 *  \param pvUser       Pointer to user data
 *  \param pcTitle      Title string
 *  \param pcText       Operation string
 *  \param iNumSteps    Number of steps to complete the operation
 *  \param iProgress    Number of steps completed so far.
 *  \return E_PRG_OK if ok. Any other status will abort the operation.
 */
typedef teStatus(*tcbFW_Progress)(void *pvUser, const char *pcTitle, const char *pcText, int iNumSteps, int iProgress);


/** Callback function for requesting user to confirm an action.
 *  \param pvUser       Pointer to user data
 *  \param pcTitle      Title string
 *  \param pcText       Operation string
 *  \return E_PRG_OK if ok. Any other status will abort the operation.
 */
typedef teStatus(*tcbFW_Confirm)(void *pvUser, const char *pcTitle, const char *pcText);

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/** Version string for libprogrammer */
extern const char * LIBPROGRAMMER pcPRG_Version;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/** Initialise the programmer library
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_Init(tsPRG_Context *psContext);


/** Destroy the programmer library
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_Destroy(tsPRG_Context *psContext);


/** Get the status of the last operation with a given chip.
 *  If an operation fails, this will return a string describing the
 *  error state. On success it will return the string "Success".
 *  \return String containing description of last operation status.
 */
char *LIBPROGRAMMER pcPRG_GetLastStatusMessage(tsPRG_Context *psContext);


/** Get a list of available connections for use by the programmer. Once the list has been used it should be destroyed using
 *  \ref ePRG_ConnectionListDestroy.
 *  \param          psContext Pointer to programmer context
 *  \param[out]     pu32NumConnections Pointer to location to store the number of available connections
 *  \param[out]     pasConnections Pointer to location to store an array of structures representing available connections.
 *                  The pointer will be allocated by the library and should be destroyed by \ref ePRG_ConnectionListDestroy
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_ConnectionListInit(tsPRG_Context *psContext, uint32_t *pu32NumConnections, tsConnection **pasConnections);


/** Free a list of available connections that was created by \ref ePRG_DeviceListInit.
 *  \param          psContext Pointer to programmer context
 *  \param          u32NumConnections Number of connections in the list
 *  \param          papcDeviceNames Pointer to location of a previously retrieved device list.
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_ConnectionListDestroy(tsPRG_Context *psContext, uint32_t u32NumConnections, tsConnection **pasConnections);


/** Open a connection to chip. psConnection should contain all applicable parameters including type, name and settings.
 *  \param[out]     psContext Pointer to programmer context
 *  \param          psConnection Pointer to connection to open.
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_ConnectionOpen(tsPRG_Context *psContext, tsConnection *psConnection);


/** Close connection to chip
 *  \param          psContext Pointer to programmer context
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_ConnectionClose(tsPRG_Context *psContext);


/** Update connection details.
 *  This can be used to change the baudrate of an open connection.
 *  \param          psContext Pointer to programmer context
 *  \param          psConnection New connection details
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_ConnectionUpdate(tsPRG_Context *psContext, tsConnection *psConnection);


/** Open a firmware file and get information about it using \ref ePRG_FwGetInfo
 *  This populates psContext->sFirmwareInfo with the necessary data for programming this firmware.
 *  \param          psContext Pointer to programmer context
 *  \param          pcFirmwareFile Filename to open
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_FwOpen(tsPRG_Context *psContext, char *pcFirmwareFile);


/** Close a firmware file that has been opened using \ref ePRG_FwOpen.
 *  The file is automatically closed if a new one is opened using \ref ePRG_FwOpen
 *  or if the pcContext is destroyed using \ref ePRG_Destroy.
 *  \param          psContext Pointer to programmer context
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_FwClose(tsPRG_Context *psContext);


/** Get information from a firmware image. This takes a buffer of data
 *  representing the complete firmware image and extracts the required information
 *  from it.
 *  This populates psContext->sFirmwareInfo with the necessary data for programming this firmware.
 *  \param          psContext Pointer to programmer context
 *  \param          pu8Firmware Firmware file data buffer.
 *  \param          u32Length Length of firmware data buffer
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_FwGetInfo(tsPRG_Context *psContext, uint8_t *pu8Firmware, uint32_t u32Length);


/** Get information from the connected device and populate psChipDetails with it.
 *  This populates psContext->sChipDetails and psContext->sDeviceConfig with the necessary data for programming this firmware.
 *  \param          psContext Pointer to programmer context
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_ChipGetDetails(tsPRG_Context *psContext);


/** Add a flash device to the context. This is used internally to add discovered flash devices to the context.
 *  It may be used by an application, for example to add a flash device that is connected to a non-standard chip select.
 *  \param          psContext Pointer to programmer context
 *  \param          pcName  Human readable name of the flash device
 *  \param          u32FlashSize Size (in bytes) of the flash device
 *  \param          u8ManufacturerID Manufacturer ID of the flash device.
 *  \param          u8DeviceID Device ID of the flash device.
 *  \param          u8ChipSelect The chip select that the flash is connected to
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_FlashAddDevice(tsPRG_Context *psContext, const char *pcName, const uint32_t u32FlashSize,
                                           uint8_t u8ManufacturerID, const uint8_t u8DeviceID, const uint8_t u8ChipSelect);


/** Select the target flash device for further flash operations.
 *  \param          psContext Pointer to programmer context
 *  \param          u32FlashDevice Index of the flash to select within the \ref asFlashes array of the \ref tsChipDetails structure.
 *  \return E_PRG_OK on success, or E_PRG_FLASH_DEVICE_UNAVAILABLE if the device could not be selected.
 */
teStatus LIBPROGRAMMER ePRG_FlashSelectDevice(tsPRG_Context *psContext, uint32_t u32FlashDevice);


/** Erase the flash of the selected device. This call will block until the request has completed.
 *  This function is called internally by \ref ePRG_FlashProgram so there is no need to call it as part of the programming process.
 *  During erasing the library will call cbProgress periodically to update the caller with progress.
 *  \param          psContext Pointer to programmer context
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress is called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_FlashErase(tsPRG_Context *psContext, tcbFW_Progress cbProgress, void *pvUser);


/** Reprogram the connected device flash with the given image. This call will block until the request has completed.
 *  During programming the library will call cbProgress periodically to update the caller with progress.
 *  \param          psContext Pointer to programmer context
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          cbConfirm Callback routine to confirm operation. May be NULL, in which case an operation requiring confirmation will fail.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress is called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_FlashProgram(tsPRG_Context *psContext, tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser);


/** Verify the connected device flash with the given image.
 *  During verification the library will call cbProgress periodically to update the caller with progress.
 *  \param          psContext Pointer to programmer context
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress is called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_FlashVerify(tsPRG_Context *psContext, tcbFW_Progress cbProgress, void *pvUser);


/** Dump the contents of the device's flash memory into the file specified.
 *  \param          psContext Pointer to programmer context
 *  \param          pcDumpFile Filename to dump to.
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress is called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_FlashDump(tsPRG_Context *psContext, char *pcDumpFile, tcbFW_Progress cbProgress, void *pvUser);


/** Load the connected device RAM with the given image and execute it.
 *  During loading the library will call cbProgress periodically to update the caller with progress.
 *  \param          psContext Pointer to programmer context
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          cbConfirm Callback routine to confirm operation. May be NULL, in which case an operation requiring confirmation will fail.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress is called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_MemoryLoadExecute(tsPRG_Context *psContext, tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser);


/** Erase the EEPROM of the connected device.
 *  During erasure the library will call cbProgress periodically to update the caller with progress.
 *  \param          psContext Pointer to programmer context
 *  \param          eErase  What type of erase to perfom on the EEPROM
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_EepromErase(tsPRG_Context *psContext, teEepromErase eErase, tcbFW_Progress cbProgress, void *pvUser);


/** Dump the contents of the device's EEPROM memory into the file specified.
 *  \param          psContext Pointer to programmer context
 *  \param          pcDumpFile Filename to dump to.
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress is called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_EepromDump(tsPRG_Context *psContext, char *pcDumpFile, tcbFW_Progress cbProgress, void *pvUser);


/** Load the contents of the device's EEPROM memory from the file specified.
 *  \param          psContext Pointer to programmer context
 *  \param          pcLoadFile Filename to load from.
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress is called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_EepromProgram(tsPRG_Context *psContext, char *pcLoadFile, tcbFW_Progress cbProgress, void *pvUser);


/** Write a new MAC address into the index sector of a connected JN516x device.
 *  During programming the library will call cbProgress periodically to update the caller with progress.
 *  \param          psContext Pointer to programmer context
 *  \param          au8MacAddress New MAC address to write into the device.
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          cbConfirm Callback routine to confirm operation. May not be NULL.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress and cbConfirm are called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_MACAddressSet(tsPRG_Context *psContext, uint8_t au8MacAddress[8], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser);


/** Read AES key from the index sector of a connected JN516x device.
 *  \param          psContext Pointer to programmer context
 *  \param          au32Key Location to store AES key read from the device.
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          cbConfirm Callback routine to confirm operation. May be NULL.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress and cbConfirm are called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_AesKeyGet(tsPRG_Context *psContext, uint32_t au32Key[4], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser);


/** Write a new AES key into the index sector of a connected JN516x device.
 *  During programming the library will call cbProgress periodically to update the caller with progress.
 *  \param          psContext Pointer to programmer context
 *  \param          au32Key New AES key to write into the device.
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          cbConfirm Callback routine to confirm operation. May not be NULL.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress and cbConfirm are called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_AesKeySet(tsPRG_Context *psContext, uint32_t au32Key[4], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser);


/** Read userdata from the index sector of a connected JN516x device.
 *  \param          psContext Pointer to programmer context
 *  \param          u32Index Which user word of data to read
 *  \param          au32Data Location to store data read from the device.
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          cbConfirm Callback routine to confirm operation. May be NULL.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress and cbConfirm are called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_UserDataGet(tsPRG_Context *psContext, uint32_t u32Index, uint32_t au32Data[4], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser);


/** Write new userdata into the index sector of a connected JN516x device.
 *  During programming the library will call cbProgress periodically to update the caller with progress.
 *  \param          psContext Pointer to programmer context
 *  \param          u32Index Which user word of data to write
 *  \param          au32Data Data to write into the device.
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          cbConfirm Callback routine to confirm operation. May not be NULL.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress and cbConfirm are called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_UserDataSet(tsPRG_Context *psContext, uint32_t u32Index, uint32_t au32Data[4], tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser);

/** Set device configuration
 *  \param          psContext Pointer to programmer context
 *  \param          psDeviceConfig Pointer to new device configuration to be written to device
 *  \param          cbProgress Callback routine to update operation progress. May be NULL for no feedback.
 *  \param          cbConfirm Callback routine to confirm operation. May not be NULL.
 *  \param          pvUser  Pointer to user data that is passed back to the application when cbProgress and cbConfirm are called. May be NULL for no user data
 *  \return E_PRG_OK on success
 */
teStatus LIBPROGRAMMER ePRG_DeviceConfigSet(tsPRG_Context *psContext, tsDeviceConfig *psDeviceConfig, tcbFW_Progress cbProgress, tcbFW_Confirm cbConfirm, void *pvUser);


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

teStatus ePRG_SetStatus(tsPRG_Context *psContext, teStatus eStatus, const char *pcAdditionalInfoFmt, ...);
void vPRG_WaitMs(uint32_t u32TimeoutMs);


char *pcPRG_GetLastErrorMessage(tsPRG_Context *psContext);

#if defined __cplusplus
}
#endif

#endif  /* PROGRAMMER_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/



