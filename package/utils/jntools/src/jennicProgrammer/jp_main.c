/****************************************************************************
*
* MODULE:             Jennic Module Programmer
*
* COMPONENT:          Main file
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include "JN51xx_BootLoader.h"
#include "Firmware.h"
#include "uart.h"
#include "ChipID.h"
#include "dbg.h"
#include "newLog.h"

// #define LL_LOG( t )
#define LL_LOG( t ) filelog( "/tmp/flasher.log", t )

#define vDelay(a) usleep(a * 1000)

int iVerbosity = 1;

/** Import binary data from FlashProgrammerExtension_JN5168.bin */
int _binary_FlashProgrammerExtension_JN5168_bin_start;
int _binary_FlashProgrammerExtension_JN5168_bin_size;
int _binary_FlashProgrammerExtension_JN5169_bin_start;
int _binary_FlashProgrammerExtension_JN5169_bin_size;
int _binary_FlashProgrammerExtension_JN5179_bin_start;
int _binary_FlashProgrammerExtension_JN5179_bin_size;

char * flashExtension = NULL;

const char *cpSerialDevice = "/dev/ttyUSB0";
const char *pcFirmwareFile = NULL;

char *pcMAC_Address = NULL;
uint64_t u64MAC_Address;
uint64_t *pu64MAC_Address = NULL;


int iInitialSpeed=38400;
int iProgramSpeed=1000000;


void print_usage_exit(char *argv[])
{
    fprintf(stderr, "Usage: %s\n", argv[0]);
    fprintf(stderr, "  Arguments:\n");
    fprintf(stderr, "    -s --serial        <serial device> Serial device for 15.4 module, e.g. /dev/tts/1\n");
    fprintf(stderr, "  Options:\n");
    fprintf(stderr, "    -V --verbosity     <verbosity>     Verbosity level. Increses amount of debug information. Default 0.\n");
    fprintf(stderr, "    -I --initialbaud   <rate>          Set initial baud rate\n");
    fprintf(stderr, "    -P --programbaud   <rate>          Set programming baud rate\n");
    fprintf(stderr, "    -f --firmware      <firmware>      Load module flash with the given firmware file.\n");
    fprintf(stderr, "    -v --verify                        Verify image. If specified, verify the image programmedwas loaded correctly.\n");
    fprintf(stderr, "    -m --mac           <MAC Address>   Set MAC address of device. If this is not specified, the address is read from flash.\n");
    exit(EXIT_FAILURE);
}


teStatus cbProgress(void *pvUser, const char *pcTitle, const char *pcText, int iNumSteps, int iProgress)
{
    int progress;
    if (iNumSteps > 0)
    {
        progress = ((iProgress * 100) / iNumSteps);
    }
    else
    {
        // Begin marker
        progress = 0;
        printf( "\n" );
    }
    printf( "%c[A%s = %d%%\n", 0x1B, pcText, progress );
        
    return E_PRG_OK;
}

static int importExtension( char * file, int * start, int * size ) {
    struct stat sb;
    if (stat(file, &sb) == -1) {
        perror("stat");
        return 0;
    }
    
    printf("File size: %lld bytes\n", (long long) sb.st_size);
    size_t bytestoread = sb.st_size;
    
    if ( ( flashExtension = malloc( sb.st_size + 100 ) ) == NULL ) {
        perror("malloc");
        return 0;
    }
    
    int fp, bytesread;
    if ( ( fp = open(file,O_RDONLY) ) < 0 ) {
        perror("open");
        return 0;
    }
    
    char * pbuf = flashExtension;
    while ( bytestoread > 0 ) {
        if ( ( bytesread = read( fp, pbuf, bytestoread ) ) < 0 ) {
            break;
        }
        bytestoread -= bytesread;
        pbuf += bytesread;
        }
        
    if ( bytestoread == 0 ) {
        *start = (int)flashExtension;
        *size  = sb.st_size;
        printf( "Loaded binary of %d bytes\n", *size );
        return 1;
    }
    
    return 0;
}

static teStatus ePRG_ImportExtension(tsPRG_Context *psContext)
{
    int ret = 0;
    switch (CHIP_ID_PART(psContext->sChipDetails.u32ChipId))
    {
        case (CHIP_ID_PART(CHIP_ID_JN5168)):
            ret = importExtension( "/usr/share/iot/FlashProgrammerExtension_JN5168.bin",
                &_binary_FlashProgrammerExtension_JN5168_bin_start,
                &_binary_FlashProgrammerExtension_JN5168_bin_size );
            psContext->pu8FlashProgrammerExtensionStart    = (uint8_t *)_binary_FlashProgrammerExtension_JN5168_bin_start;
            psContext->u32FlashProgrammerExtensionLength   = (uint32_t)_binary_FlashProgrammerExtension_JN5168_bin_size;
            break;
        case (CHIP_ID_PART(CHIP_ID_JN5169)):
            ret = importExtension( "/usr/share/iot/FlashProgrammerExtension_JN5169.bin",
                &_binary_FlashProgrammerExtension_JN5169_bin_start,
                &_binary_FlashProgrammerExtension_JN5169_bin_size );
            psContext->pu8FlashProgrammerExtensionStart    = (uint8_t *)_binary_FlashProgrammerExtension_JN5169_bin_start;
            psContext->u32FlashProgrammerExtensionLength   = (uint32_t)_binary_FlashProgrammerExtension_JN5169_bin_size;
            break;
        case (CHIP_ID_PART(CHIP_ID_JN5179)):
            ret = importExtension( "/usr/share/iot/FlashProgrammerExtension_JN5179.bin",
                &_binary_FlashProgrammerExtension_JN5179_bin_start,
                &_binary_FlashProgrammerExtension_JN5179_bin_size );
            psContext->pu8FlashProgrammerExtensionStart    = (uint8_t *)_binary_FlashProgrammerExtension_JN5179_bin_start;
            psContext->u32FlashProgrammerExtensionLength   = (uint32_t)_binary_FlashProgrammerExtension_JN5179_bin_size;
            break;
    }
    if ( ret ) {
        return E_PRG_OK;
    }
        
    return E_PRG_ERROR;
}

int main(int argc, char *argv[])
{
    tsPRG_Context   sPRG_Context;
    int ret = 0;
    int iVerify = 1;

    printf("JennicModuleProgrammer Version: %s\n", pcPRG_Version);
    
    memset(&sPRG_Context, 0, sizeof(tsPRG_Context));

    {
        static struct option long_options[] =
        {
            {"help",                    no_argument,        NULL,       'h'},
            {"verbosity",               required_argument,  NULL,       'V'},
            {"initialbaud",             required_argument,  NULL,       'I'},
            {"programbaud",             required_argument,  NULL,       'P'},
            {"serial",                  required_argument,  NULL,       's'},
            {"firmware",                required_argument,  NULL,       'f'},
            {"verify",                  no_argument,        NULL,       'v'},
            {"mac",                     required_argument,  NULL,       'm'},
            { NULL, 0, NULL, 0}
        };
        signed char opt;
        int option_index;
        
        while ((opt = getopt_long(argc, argv, "hs:V:f:vI:P:m:", long_options, &option_index)) != -1) 
        {
            switch (opt) 
            {
                case 0:
                    
                case 'h':
                    print_usage_exit(argv);
                    break;
                case 'V':
                    iVerbosity = atoi(optarg);
                    break;
                case 'v':
                    iVerify = 1;
                    break;
                case 'I':
                {
                    char *pcEnd;
                    errno = 0;
                    iInitialSpeed = strtoul(optarg, &pcEnd, 0);
                    if (errno)
                    {
                        printf("Initial baud rate '%s' cannot be converted to 32 bit integer (%s)\n", optarg, strerror(errno));
                        print_usage_exit(argv);
                    }
                    if (*pcEnd != '\0')
                    {
                        printf("Initial baud rate '%s' contains invalid characters\n", optarg);
                        print_usage_exit(argv);
                    }
                    break;
                }
                case 'P':
                {
                    char *pcEnd;
                    errno = 0;
                    iProgramSpeed = strtoul(optarg, &pcEnd, 0);
                    if (errno)
                    {
                        printf("Program baud rate '%s' cannot be converted to 32 bit integer (%s)\n", optarg, strerror(errno));
                        print_usage_exit(argv);
                    }
                    if (*pcEnd != '\0')
                    {
                        printf("Program baud rate '%s' contains invalid characters\n", optarg);
                        print_usage_exit(argv);
                    }
                    break;
                }
                case 's':
                    cpSerialDevice = optarg;
                    break;
                case 'f':
                    pcFirmwareFile = optarg;
                    break;
                case 'm':
                    pcMAC_Address = optarg;
                    u64MAC_Address = strtoll(pcMAC_Address, (char **) NULL, 16);
                    pu64MAC_Address = &u64MAC_Address;
                    break;

                default: /* '?' */
                    print_usage_exit(argv);
            }
        }
    }
    
#if 0
    // TEST TODO
    int ret = importExtension( "/usr/share/iot/FlashProgrammerExtension_JN5169.bin",
                &_binary_FlashProgrammerExtension_JN5169_bin_start,
                &_binary_FlashProgrammerExtension_JN5169_bin_size );
    printf( "ret = %d\n", ret );
    dump( (char *)_binary_FlashProgrammerExtension_JN5169_bin_start, 64 );
    return 0;
    // TEST
#endif
    
    if (eUART_Initialise((char *)cpSerialDevice, iInitialSpeed, &sPRG_Context.iUartFD, &sPRG_Context.sUartOptions) != E_PRG_OK)
    {
        fprintf(stderr, "Error opening serial port\n");
        LL_LOG( "Error opening serial port" );
        return 1;
    }
    LL_LOG( "Serial port opened at iInitialSpeed (= 38400)" );

    if (iInitialSpeed != iProgramSpeed)
    {
        if (iVerbosity > 1)
        {
            printf("Setting baudrate for port %d to %d\n", sPRG_Context.iUartFD, iProgramSpeed);
        }

        /* Talking at initial speed - change bootloader to programming speed */
        int retry = 3;
        while ( retry-- > 0 && eBL_SetBaudrate(&sPRG_Context, iProgramSpeed) != E_PRG_OK) {
            printf("Error setting (bootloader) baudrate to iProgramSpeed (%d) (%d)\n", iProgramSpeed, retry);
            LL_LOG( "Error setting (bootloader) baudrate to iProgramSpeed (= 1000000)" );
            // ret = 2;
        }
        if ( retry <= 0 ) ret = 2;
        /* change local port to programming speed */
        if (eUART_SetBaudRate(sPRG_Context.iUartFD, &sPRG_Context.sUartOptions, iProgramSpeed) != E_PRG_OK)
        {
            printf("Error setting (local port) baudrate to iProgramSpeed (%d)\n", iProgramSpeed);
            LL_LOG( "Error setting (local port) baudrate to iProgramSpeed" );
            // eBL_SetBaudrate(&sPRG_Context, iInitialSpeed );
            ret = 3;
        }
    }

    if ( ret != 0 ) return ret;

    /* Read module details at initial baud rate */
    if (ePRG_ChipGetDetails(&sPRG_Context) != E_PRG_OK)
    {
        fprintf(stderr, "Error reading module information - check cabling and power\n");
        LL_LOG( "Error reading module information - check cabling and power" );
        ret = 4;
    }

    if ( ret == 0 && iVerbosity > 0)
    {
        const char *pcPartName;
        
        switch (sPRG_Context.sChipDetails.u32ChipId)
        {
            case (CHIP_ID_JN5148_REV2A):    pcPartName = "JN5148";      break;
            case (CHIP_ID_JN5148_REV2B):    pcPartName = "JN5148";      break;
            case (CHIP_ID_JN5148_REV2C):    pcPartName = "JN5148";      break;
            case (CHIP_ID_JN5148_REV2D):    pcPartName = "JN5148J01";   break;
            case (CHIP_ID_JN5148_REV2E):    pcPartName = "JN5148Z01";   break;

            case (CHIP_ID_JN5142_REV1A):    pcPartName = "JN5142";      break;
            case (CHIP_ID_JN5142_REV1B):    pcPartName = "JN5142";      break;
            case (CHIP_ID_JN5142_REV1C):    pcPartName = "JN5142J01";   break;

            case (CHIP_ID_JN5168):          pcPartName = "JN5168";      break;
            case (CHIP_ID_JN5168_COG07A):   pcPartName = "JN5168";      break;
            case (CHIP_ID_JN5168_COG07B):   pcPartName = "JN5168";      break;
            
            case (CHIP_ID_JN5169):          pcPartName = "JN5169";      break;
            case (CHIP_ID_JN5169_DONGLE):   pcPartName = "JN5169";      break;

            case (CHIP_ID_JN5172):          pcPartName = "JN5172";      break;

            case (CHIP_ID_JN5179):          pcPartName = "JN5179";      break;

            default:                        pcPartName = "Unknown";     break;
        }

        printf("Detected Chip: %s\n", pcPartName);
        
        printf("MAC Address:   %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n", 
                sPRG_Context.sChipDetails.au8MacAddress[0] & 0xFF, 
                sPRG_Context.sChipDetails.au8MacAddress[1] & 0xFF, 
                sPRG_Context.sChipDetails.au8MacAddress[2] & 0xFF, 
                sPRG_Context.sChipDetails.au8MacAddress[3] & 0xFF, 
                sPRG_Context.sChipDetails.au8MacAddress[4] & 0xFF, 
                sPRG_Context.sChipDetails.au8MacAddress[5] & 0xFF, 
                sPRG_Context.sChipDetails.au8MacAddress[6] & 0xFF, 
                sPRG_Context.sChipDetails.au8MacAddress[7] & 0xFF);
    }


    if (ret == 0 && pcFirmwareFile)
    {
        /* Have file to program */
    
        if (ePRG_FwOpen(&sPRG_Context, (char *)pcFirmwareFile)) {
            /* Error with file. FW module has displayed error so just exit. */
            LL_LOG( "Error with firmware file" );
            ret = 5;
        } else if ( ePRG_FlashProgram(&sPRG_Context, cbProgress, NULL, NULL) != E_PRG_OK ) {
            LL_LOG( "Error with flashing" );
            ret = 6;
        } else if ( iVerify && ePRG_FlashVerify(&sPRG_Context, cbProgress, NULL) != E_PRG_OK ) {
            LL_LOG( "Error in verification" );
            ret = 7;
        }
        
        if ( ret == 0 ) {
            if ( ePRG_ImportExtension(&sPRG_Context) != E_PRG_OK ) {
                LL_LOG( "Error importing extension" );
                ret = 8;
            } else if ( ePRG_EepromErase(&sPRG_Context, E_ERASE_EEPROM_PDM, cbProgress, NULL) != E_PRG_OK) {
                LL_LOG( "Error erasing EEPROM" );
                ret = 9;
            }
        }
    }
    
    // Set BL and local port back to initial speed (in reverse order)
    eBL_SetBaudrate(&sPRG_Context, iInitialSpeed);
    eUART_SetBaudRate(sPRG_Context.iUartFD, &sPRG_Context.sUartOptions, iInitialSpeed);
    
    if ( ret == 0 && iVerbosity > 0) {
        printf("Success\n");
    }
    return 0;
}


void dbg_vPrintfImpl(const char *pcFile, uint32_t u32Line, const char *pcFormat, ...)
{
    va_list argp;
    va_start(argp, pcFormat);
#ifdef DBG_VERBOSE
    printf("%s:%d ", pcFile, u32Line);
#endif /* DBG_VERBOSE */
    vprintf(pcFormat, argp);
    return;
}

