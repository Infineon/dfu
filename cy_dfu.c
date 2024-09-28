/***************************************************************************//**
* \file cy_dfu.c
* \version 5.2
*
*  This file provides the implementation of DFU Middleware.
*
********************************************************************************
* \copyright
* (c) (2016-2024), Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation. All rights reserved.
********************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation or one of its
* affiliates ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include <string.h>
#include "cy_dfu.h"
#include "cy_dfu_logging.h"


#define CySoftwareReset() NVIC_SystemReset()

/** \cond INTERNAL */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
CY_SECTION(".cy_boot_noinit.appId") __USED static uint8_t cy_dfu_appId;
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */


/* The timeout for Cy_DFU_Continue(), in milliseconds */
#define UPDATE_TIMEOUT                      (20U)
/* The timeout for the default Cy_DFU__TransportWrite() call in milliseconds */
#define TRANSPORT_WRITE_TIMEOUT             (150U)
/* The number of bytes per app in the metadata section */
#define METADATA_BYTES_PER_APP              (8U)

#define UINT16_SIZE                         (2U)
#define UINT32_SIZE                         (4U)

#define NIBBLE_POS                          (4U)
#define NIBBLE_MSK                          (0xFU)

#define SHA1_CHECKSUM_LENGTH                (20U)   /* The SHA1 length is 20 bytes */
/* A number of uint32_t elements in the SHA1 buffer */
#define SHA1_BUF_SIZE_UINT32                (SHA1_CHECKSUM_LENGTH/UINT32_SIZE)

#define RSA_CHECKSUM_LENGTH                 (256U)  /* The RSA public key modulo length is 2048 bits = 256 bytes */

#define CRC_CHECKSUM_LENGTH                 (4U)    /* The size of metadata flash row CRC in bytes */
#define CRC_POLYNOMIAL                      (0x1EDC6F41U) /* The CRC 32 polynomial */
#define CRC_LFSR_SEED                       (0xFFFFFFFFU)
#define CRC_DATA_REVERSE                    (1U)
#define CRC_DATA_XOR                        (0U)
#define CRC_REM_REVERSE                     (1U)
#define CRC_REM_XOR                         (0xFFFFFFFFU)
#define CRC_TABLE_SIZE                      (16U)           /* A number of uint32_t elements in the CRC32 table */
#define CRC_INIT                            (0xFFFFFFFFU)

#define CRC_CCITT_INIT                      (0xFFFFU)
#define CRC_CCITT_POLYNOMIAL                (0x8408U)

#define STATUS_BYTE_MSK                     (0xFFU)

/* The size in bytes of the DFU command parameters */
#define PARAMS_SIZE                         (8U)
/* The length in bytes of data in the "Set App Metadata" DFU command */
#define DATA_LENGTH                         (9U)
/* Possible sizes of the data field in the DFU packets */
#define DATA_PACKET_SIZE_4BYTES             (4U)
#define DATA_PACKET_SIZE_6BYTES             (6U)
#define DATA_PACKET_SIZE_8BYTES             (8U)
#define DATA_PACKET_SIZE_16BYTES            (16U)

#define PACKET_DATA_NO_OFFSET               (0U)
#define PROGRAM_DATA_CRC_OFFSET             (4U) /* The offset in bytes to the CRC field in the Program Data command */
#define VERIFY_DATA_CRC_OFFSET              (4U) /* The offset in bytes to the CRC field in the Verify Data command */

/* The size in bytes of the data field in the Verify Application command */
#define VERIFY_APP_DATA_SIZE                (1U)
/* The offset in bytes to the Application Length in the application metadata */
#define METADATA_APP_LENGTH_OFFSET          (4U)
/* The offset in bytes to the application length in the data field of the Set Application Metadata command packet*/
#define SET_APP_METADATA_OFFSET             (1U)
/* The offset to the application start address in the data field of the Set Application Metadata command packet */
#define SET_APP_METADATA_LENGTH_OFFSET      (5U)
/* The offset to  the "to" part of the data field in the Get Metadata packet */
#define GET_METADATA_TO_OFFSET              (2U)

/* The size in bytes of the App Size field in Cypress Simplified User Application Object */
#define SIMPLIFIED_APP_APPSIZE_SIZE         (4U)

/* The offset in bytes to the VT offset in the Cypress Standard User Application Object */
#define CYPRESS_APP_VTOFFSET_OFFSET_BYTES   (0x10U)
/* The offset in uint32_t to the VT offset in the Cypress Standard User Application Object */
#define CYPRESS_APP_VTOFFSET_OFFSET_UINT32  (CYPRESS_APP_VTOFFSET_OFFSET_BYTES/UINT32_SIZE)
#define TOC_EMPTY                           (0UL) /* Both TOC2 and RTOC2 are empty */
#define TOC_INVALID                         (1UL) /* Either TOC2 or RTOC2 is invalid */
#define PUBLIC_KEY_IDX                      (9UL) /* The TOC item at index 9 is a public Key object  */
#define PUBLIC_KEY_OFFSET                   (8UL) /* The Public Key offset in the Public key Object */

/* The address of the verify application function entry in the Flash Boot shared functions table */
#define VERIFY_APP_TABLE_ADDR               (0x16002040UL)
/* The address of the verify key function entry in the Flash Boot shared functions table */
#define IS_VALID_KEY_TABLE_ADDR             (0x16002044UL)
/* The address of the verify TOC function entry in the Flash Boot shared functions table */
#define VALIDATE_TOC_TABLE_ADDR             (0x1600204CUL)

/* For the DFU packet */
#define PACKET_SOP_VALUE                    (0x01U)
#define PACKET_EOP_VALUE                    (0x17U)
#define PACKET_SOP_IDX                      (0x00U)
#define PACKET_CMD_IDX                      (0x01U)
#define PACKET_SIZE_IDX                     (0x02U)
#define PACKET_DATA_IDX                     (0x04U)
#define PACKET_CHECKSUM_LENGTH              (2U)    /* The length in bytes of a packet checksum field */


/* The Flash Boot verification functions*/
#if(CY_DFU_APP_FORMAT != CY_DFU_BASIC_APP)
typedef bool (*Cy_FB_VerifyApp_t)(uint32_t address, uint32_t length, uint32_t signature, uint32_t publicKeyAddr);
typedef bool (*Cy_FB_IsValidKey_t)(uint32_t tocAddr, uint32_t publicKeyAddr);
typedef uint32_t (*Cy_FB_ValidateToc_t)(uint32_t tocAddress);
#endif /* (CY_DFU_APP_FORMAT != CY_DFU_BASIC_APP) */

/* Pointer to function that is used to jump into address */
typedef void (*cy_fn_dfu_jump_ptr_t)(void);

/** \endcond */

/* The static functions declaration */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
    static uint32_t ElfSymbolToAddr(void volatile const *symbol);
    static __NO_RETURN void SwitchToApp(uint32_t stackPointer, uint32_t address);
    #if ((CY_DFU_OPT_CRYPTO_HW != 0) && (CY_DFU_APP_FORMAT == CY_DFU_BASIC_APP))
        static bool ComputeSha1(uint32_t address, uint32_t length, uint8_t *result);
    #endif /*((CY_DFU_OPT_CRYPTO_HW != 0) && (CY_DFU_APP_FORMAT == CY_DFU_BASIC_APP))*/
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */

static uint16_t GetU16(uint8_t const array[]);
static uint32_t GetU32(uint8_t const array[]);
static void     PutU16(uint8_t array[], uint32_t offset, uint32_t value);

/* Because PutU32() is used only when updating the metadata */
#if (CY_DFU_METADATA_WRITABLE != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW)
    static void PutU32(uint8_t array[], uint32_t offset, uint32_t value);
#endif /* (CY_DFU_METADATA_WRITABLE != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW) */
static uint32_t PacketChecksumIndex(uint32_t size);
static uint32_t PacketEopIndex(uint32_t size);
static uint32_t GetPacketCommand(const uint8_t packet[]);
static uint32_t GetPacketDSize(const uint8_t packet[]);
static uint8_t* GetPacketData(uint8_t packet[], uint32_t offset);
static uint32_t GetPacketChecksum(const uint8_t packet[], uint32_t packetSize);
static uint32_t ValidatePacketFooter(const uint8_t packet[], uint32_t packetSize);
static void SetPacketHeader(uint8_t packet[]);
static void SetPacketCmd(uint8_t packet[], uint32_t cmd);
static void SetPacketDSize(uint8_t packet[], uint32_t size);
static void SetPacketChecksum(uint8_t packet[], uint32_t size, uint32_t checksum);
static void SetPacketFooter(uint8_t packet[], uint32_t size);

static uint32_t PacketChecksum(const uint8_t buffer[], uint32_t size);
static cy_en_dfu_status_t VerifyPacket(uint32_t numberRead, const uint8_t packet[]);
static cy_en_dfu_status_t ReadVerifyPacket(uint8_t packet[], bool *noResponse, uint32_t timeout);
static cy_en_dfu_status_t WritePacket(cy_en_dfu_status_t status, uint8_t *packet, uint32_t rspSize);
static void EnterResponse(uint8_t *packet, uint32_t *rspSize, uint32_t *state);

static cy_en_dfu_status_t CopyToDataBuffer(uint8_t dataBuffer[], uint32_t *dataOffset, uint8_t const packet[],
                                                uint32_t packetSize);

static cy_en_dfu_status_t CommandEnter(uint8_t *packet, uint32_t *rspSize, uint32_t *state,
                                            cy_stc_dfu_params_t *params);
static cy_en_dfu_status_t CommandProgramData(uint8_t *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params);

#if CY_DFU_OPT_ERASE_DATA != 0
static cy_en_dfu_status_t CommandEraseData(uint8_t *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params);
#endif

#if CY_DFU_OPT_VERIFY_DATA != 0
static cy_en_dfu_status_t CommandVerifyData(uint8_t *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params);
#endif /* CY_DFU_OPT_VERIFY_DATA != 0*/

#if CY_DFU_OPT_SEND_DATA != 0
static cy_en_dfu_status_t CommandSendData(uint8_t *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params);
#endif /* CY_DFU_OPT_SEND_DATA != 0 */

#if CY_DFU_OPT_VERIFY_APP != 0
static cy_en_dfu_status_t CommandVerifyApp(uint8_t *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params);
#endif /* CY_DFU_OPT_VERIFY_APP != 0 */

static cy_en_dfu_status_t CommandSetAppMetadata(uint8_t *packet, uint32_t *rspSize,
                                                     cy_stc_dfu_params_t *params);
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW

#if CY_DFU_OPT_GET_METADATA != 0
static cy_en_dfu_status_t CommandGetMetadata(uint8_t *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params);
#endif /* CY_DFU_OPT_GET_METADATA != 0 */

#if CY_DFU_OPT_SET_EIVECTOR != 0
    static cy_en_dfu_status_t CommandSetEIVector(uint8_t *packet, uint32_t *rspSize,
                                                      cy_stc_dfu_params_t *params);
#endif /* CY_DFU_OPT_SET_EIVECTOR != 0 */
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */

static cy_en_dfu_status_t CommandUnsupported(uint8_t packet[], uint32_t *rspSize,
                                                              cy_stc_dfu_params_t *params );
static cy_en_dfu_status_t ContinueHelper(uint32_t command, uint8_t *packet, uint32_t *rspSize,
                                              cy_stc_dfu_params_t *params, bool *noResponse);
#if(CY_DFU_APP_FORMAT != CY_DFU_BASIC_APP)
    #if(CY_DFU_SEC_APP_VERIFY_TYPE == CY_DFU_VERIFY_FAST)
        static bool VerifySecureAppShort(uint32_t verifyStartAddr, uint32_t verifyLength, uint32_t signatureAddr);
    #else
        static bool VerifySecureAppFull(uint32_t verifyStartAddr, uint32_t verifyLength, uint32_t signatureAddr);
    #endif /* CY_DFU_SEC_APP_VERIFY_TYPE == CY_DFU_VERIFY_FAST */
    static bool VerifySecureApp(uint32_t verifyStartAddr, uint32_t verifyLength, uint32_t signatureAddr);
#endif/*(CY_DFU_APP_FORMAT != CY_DFU_BASIC_APP)*/


/*******************************************************************************
* Function Name: Cy_DFU_Init
****************************************************************************//**
*
* This function starts the application download and install operations.
* Make subsequent calls to Cy_DFU_Continue() to continue the
* process. \n
* Returns immediately, reporting success or failure. \n
* Only one updating operation can be done at a time - the user's code must
* ensure this.
*
* \param state      The pointer to a state variable, that is updated by
*                   the function. See \ref group_dfu_macro_state
* \param params     The pointer to a DFU parameters structure
*                   See \ref cy_stc_dfu_params_t
*
* \return See \ref cy_en_dfu_status_t.
* - \ref CY_DFU_SUCCESS if successful.
* - \ref CY_DFU_ERROR_UNKNOWN either parameter is a NULL pointer.
*
* \snippet snippet/main.c snipped_cy_dfu_init
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_Init(uint32_t *state, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    if ( (state == NULL) || (params == NULL) )
    {
        status = CY_DFU_ERROR_UNKNOWN;
    }

    if (status == CY_DFU_SUCCESS)
    {
        *state = CY_DFU_STATE_NONE;
        params->dataOffset = 0U;
    }
    return (status);
}


#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
/*******************************************************************************
* Function Name: Cy_DFU_ExecuteApp
****************************************************************************//**
*
* This function transfers control from the current application to another
* application. The function performs switching via software reset. In case if
* application need to switch without reset, for example if it needs to enable
* some peripheral during and after the application switching,
* use \ref Cy_DFU_SwitchToApp().
* The function does not return.
* \note It is assumed appId is a valid application number.
*
* \param appId  An application number of the application to switch to.
*
*******************************************************************************/
void Cy_DFU_ExecuteApp(uint32_t appId)
{
    CY_ASSERT(appId < CY_DFU_MAX_APPS);
    cy_dfu_appId = (uint8_t)appId;
    CySoftwareReset();
}


/*******************************************************************************
* Function Name: SwitchToApp
****************************************************************************//**
*
* Set main stack pointer and then jumps into the address.
*
* \param stackPointer Stack pointer
* \param address      Address to jump into
*
* \note This function does not return.
*
*******************************************************************************/
static void SwitchToApp(uint32_t stackPointer, uint32_t address)
{
    __set_MSP(stackPointer);
CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 11.1','Casting int to a function pointer is safe as it is guaranteed to have a valid address.');
    ((cy_fn_dfu_jump_ptr_t) address) ();

    /* This function does not return */
    for(;;)
    {
    }
}


/*******************************************************************************
* Function Name: Cy_DFU_SwitchToApp
****************************************************************************//**
*
* This function switches to the application through the jump instruction.
* The function should be used when an application switching must be done without
* a software reset. Possible reason is a need to enable some peripheral during
* and after the application switching. In other case use \ref Cy_DFU_ExecuteApp().
*
* Before calling this function, ensure all the peripherals and bus masters are
* in a known state. User is responsible to disable peripherals and to set MCU
* internal state before or after an application switching.
*
* \note It is assumed appId is a valid application number.
*
* \param appId      An application number of the application to switch to.
*
* \return It doesn't return if succeeds. If failed, returns the status code.
*         See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_SwitchToApp(uint32_t appId)
{
    uint32_t startAddress;
    cy_en_dfu_status_t status;

    CY_ASSERT(appId < CY_DFU_MAX_APPS);

    status = Cy_DFU_GetAppMetadata(appId, &startAddress, NULL);

    if (status == CY_DFU_SUCCESS)
    {
    #if(CY_DFU_APP_FORMAT == CY_DFU_SIMPLIFIED_APP)
        uint32_t offsetVt = ((uint32_t *)(startAddress + SIMPLIFIED_APP_APPSIZE_SIZE))[0];
        startAddress += SIMPLIFIED_APP_APPSIZE_SIZE + offsetVt;
    #elif(CY_DFU_APP_FORMAT == CY_DFU_CYPRESS_APP)
        uint32_t offsetVt = ((uint32_t *)startAddress)[CYPRESS_APP_VTOFFSET_OFFSET_UINT32];
        startAddress += CYPRESS_APP_VTOFFSET_OFFSET_BYTES + offsetVt;
    #else
        /* Cypress Basic Application Format (CyBAF) */
    #endif
        uint32_t stackPointer = ((uint32_t *)startAddress)[0]; /* The Stack Pointer of the app to switch to */
        uint32_t resetHandler = ((uint32_t *)startAddress)[1]; /* Reset_Handler() address */
        SwitchToApp(stackPointer, resetHandler);
    }
    return (status);
}


/*******************************************************************************
* Function Name: ElfSymbolToAddr
****************************************************************************//**
*
* This function is used to convert an ELF file symbol address to uint32_t. \n
* This is safer than casting a symbol address to an integer because the
* function does not produce a MISRA warning at the call side.
* Also, a function call is more readable and easier to search with the text
* editor.
*
* \param symbol The address of the ELF file symbol to get the uint32_t value for.
*
* \return The address of the ELF file symbol as uint32_t.
*
*******************************************************************************/
static uint32_t ElfSymbolToAddr(void volatile const *symbol)
{
CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 11.6','Casting pointer to a int is safe as symbol must be valid 4-byte value.');
    return (uint32_t) symbol;
}


/*******************************************************************************
* Function Name: Cy_DFU_GetAppMetadata
****************************************************************************//**
*
* Reads application metadata to \c verifyAddress and \c verifySize.
* The metadata is supposed to be located in internal flash.
*
* This is a weak function and the user may override it in the user's code by
* providing a function with the same name.
* This allows the user to place metadata in any NVM.
*
* \note It is assumed appId is a valid application number.
*
* \param appId          The application number.
* \param verifyAddress  The pointer to a variable where an application
*                       verified area start address is stored.
* \param verifySize     The pointer to a variable where a size of verified
*                       application area is stored.
*
* \return See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
__WEAK cy_en_dfu_status_t Cy_DFU_GetAppMetadata(uint32_t appId, uint32_t *verifyAddress, uint32_t *verifySize)
{
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

   CY_ASSERT(appId < CY_DFU_MAX_APPS);

   uint32_t *ptr = (uint32_t*) ( ElfSymbolToAddr(&__cy_boot_metadata_addr) + (appId * METADATA_BYTES_PER_APP) );

   if (verifyAddress != NULL)
   {
       *verifyAddress = ptr[0];
   }
   if (verifySize != NULL)
   {
       *verifySize      = ptr[1];
   }

    return (status);
}


/* Use PDL Hardware Crypto API */
#if ((CY_DFU_OPT_CRYPTO_HW != 0) && (CY_DFU_APP_FORMAT == CY_DFU_BASIC_APP))
/*******************************************************************************
* Function Name: ComputeSha1
****************************************************************************//**
*
* This function computes SHA1 for the message.
*
* \note Ensure the Crypto block is properly initialized
* and \ref CY_DFU_OPT_CRYPTO_HW is set.
*
* \param address    The pointer to a buffer containing data to compute
*                   the checksum for. \n
*                   It must be 4-byte aligned.
* \param length     The number of bytes in the buffer to compute SHA1 for.
* \param result     The pointer to a buffer to store the SHA1 output.
*                   It must be 4-byte aligned.
*
* \return
* - true  - If calculation is successful.
* - false - If calculation is unsuccessful.
*
*******************************************************************************/
static bool ComputeSha1(uint32_t address, uint32_t length, uint8_t *result)
{

    cy_stc_crypto_context_sha_t cryptoShaContext;
    cy_en_crypto_status_t cryptoStatus;
    bool statusOk = true;

    cryptoStatus = Cy_Crypto_Enable();
    if (cryptoStatus == CY_CRYPTO_SUCCESS)
    {
CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 11.3','Casting result operand to uint32_t is safe as calling function use uint32_t pointer.');
        cryptoStatus = Cy_Crypto_Sha_Run((uint32_t *)address, length, (uint32_t *)result, CY_CRYPTO_MODE_SHA1,
                                          &cryptoShaContext);
        if (cryptoStatus == CY_CRYPTO_SUCCESS)
        {
            /* Waiting for SHA1 calculation is finished. */
            cryptoStatus = Cy_Crypto_Sync(CY_CRYPTO_SYNC_BLOCKING);
        }
        (void) Cy_Crypto_Disable();
    }
    if (cryptoStatus != CY_CRYPTO_SUCCESS)
    {
        statusOk = false;
    }
    return (statusOk);
}
#endif /* ((CY_DFU_OPT_CRYPTO_HW != 0) && (CY_DFU_APP_FORMAT == CY_DFU_BASIC_APP)) */


#if(CY_DFU_APP_FORMAT != CY_DFU_BASIC_APP)
#if(CY_DFU_SEC_APP_VERIFY_TYPE == CY_DFU_VERIFY_FAST)
/*******************************************************************************
* Function Name: VerifySecureAppShort
****************************************************************************//**
*
* This function reports whether or not the specified application is valid.
*
* \param verifyStartAddr The start address of the application to verify.
* \param verifyLength The length of the application to verify.
* \param signatureAddr The address of the application signature.
*
* \return
*       - true - If the application is valid.
*       - false - If the application is invalid
*
*******************************************************************************/
static bool VerifySecureAppShort(uint32_t verifyStartAddr, uint32_t verifyLength, uint32_t signatureAddr)
{
    uint32_t publicKeyAddr = ( (uint32_t)& SFLASH->PUBLIC_KEY[0] ) + PUBLIC_KEY_OFFSET;
    Cy_FB_VerifyApp_t Cy_FB_VerifyApp = (Cy_FB_VerifyApp_t) (*(uint32_t *) VERIFY_APP_TABLE_ADDR);
    return Cy_FB_VerifyApp(verifyStartAddr, verifyLength, signatureAddr, publicKeyAddr );
}


#else
/*******************************************************************************
* Function Name: VerifySecureAppFull
****************************************************************************//**
*
* This function reports whether or not the specified application, TOC, and key
* are valid.
*
* \param verifyStartAddr The start address of the application to verify.
* \param verifyLength The length of the application to verify.
* \param signatureAddr The address of the application signature.
*
* \return
*       - true - If the application is valid.
*       - false - If the application is invalid.
*
*******************************************************************************/
static bool VerifySecureAppFull(uint32_t verifyStartAddr, uint32_t verifyLength, uint32_t signatureAddr)
{
    Cy_FB_ValidateToc_t Cy_FB_ValidateToc = (Cy_FB_ValidateToc_t) (*(uint32_t *) VALIDATE_TOC_TABLE_ADDR);
    Cy_FB_VerifyApp_t   Cy_FB_VerifyApp   = (Cy_FB_VerifyApp_t)   (*(uint32_t *) VERIFY_APP_TABLE_ADDR  );
    Cy_FB_IsValidKey_t  Cy_FB_IsValidKey  = (Cy_FB_IsValidKey_t)  (*(uint32_t *) IS_VALID_KEY_TABLE_ADDR);
    bool status = true;

    uint32_t tocAddr = Cy_FB_ValidateToc((uint32_t)& SFLASH->TOC2_OBJECT_SIZE);

    if ((tocAddr == TOC_EMPTY) || (tocAddr == TOC_INVALID))
    {
        status = false;
    }
    else
    {
        uint32_t publicKeyAddr = *(uint32_t *)(tocAddr + (sizeof(uint32_t) * PUBLIC_KEY_IDX))
                                + PUBLIC_KEY_OFFSET;
        status = Cy_FB_IsValidKey(tocAddr, publicKeyAddr);
        if (status)
        {
            status = Cy_FB_VerifyApp(verifyStartAddr, verifyLength, signatureAddr, publicKeyAddr );
        }
    }
    return (status);
}
#endif /* CY_DFU_SEC_APP_VERIFY_TYPE == CY_DFU_VERIFY_FAST */


/*******************************************************************************
* Function Name: VerifySecureApp
****************************************************************************//**
*
* This function reports whether or not the specified application is valid.
*
* \param verifyStartAddr The start address of the application to verify.
* \param verifyLength The length of the application to verify.
* \param signatureAddr The address of the application signature.
*
* \return
*       - true - If the application is valid.
*       - false - If the application is invalid.
*
*******************************************************************************/
static bool VerifySecureApp(uint32_t verifyStartAddr, uint32_t verifyLength, uint32_t signatureAddr)
{
#if(CY_DFU_SEC_APP_VERIFY_TYPE == CY_DFU_VERIFY_FAST)
    return VerifySecureAppShort(verifyStartAddr, verifyLength, signatureAddr);
#else
    return VerifySecureAppFull (verifyStartAddr, verifyLength, signatureAddr);
#endif /* CY_DFU_SEC_APP_VERIFY_TYPE == CY_DFU_VERIFY_FAST */
}
#endif/*(CY_DFU_APP_FORMAT != CY_DFU_BASIC_APP)*/
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */


/*******************************************************************************
* Function Name: Cy_DFU_ValidateApp
****************************************************************************//**
*
* This function reports whether or not metadata and the specified application is
* valid. It checks:
* - checksum for applications without format;
* - application signature for Cypress Standard User Application format.
*
* This is a weak function and the user may override it in the user's code by
* providing a function with the same name.
*
* \note It is assumed appId is a valid application number.
*
* \param appId      The application number of the application to be validated.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t.
* - \ref CY_DFU_SUCCESS If the application is valid.
* - \ref CY_DFU_ERROR_VERIFY If the application is invalid.
*
*******************************************************************************/
__WEAK cy_en_dfu_status_t Cy_DFU_ValidateApp(uint32_t appId, cy_stc_dfu_params_t *params)
{
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
    uint32_t appVerifyStartAddress;
    uint32_t appVerifySize;

    (void)params;

    CY_ASSERT(appId < CY_DFU_MAX_APPS);

    cy_en_dfu_status_t status = Cy_DFU_GetAppMetadata(appId, &appVerifyStartAddress, &appVerifySize);


    if (status == CY_DFU_SUCCESS)
    {
    #if(CY_DFU_APP_FORMAT == CY_DFU_CYPRESS_APP)
        status = (VerifySecureApp(appVerifyStartAddress, appVerifySize, appVerifyStartAddress + appVerifySize))?
                                  CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
    #elif (CY_DFU_APP_FORMAT == CY_DFU_SIMPLIFIED_APP)
        status = (VerifySecureApp(appVerifyStartAddress, appVerifySize, appVerifyStartAddress - RSA_CHECKSUM_LENGTH))?
                                  CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
    #else
        #if(CY_DFU_OPT_CRYPTO_HW != 0)
            uint32_t sha1buf[SHA1_BUF_SIZE_UINT32];
            uint32_t appFooterAddress = appVerifyStartAddress + appVerifySize;
            if (ComputeSha1(appVerifyStartAddress, appVerifySize, (uint8_t*)&sha1buf))
            {
CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 11.6','Casting int to pointer is safe as it has valid address defined in linker script.');
                status = (memcmp((const void *)sha1buf, (const void *)appFooterAddress, SHA1_CHECKSUM_LENGTH) == 0)?
                                                                        CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
            }
            else
            {
                status = CY_DFU_ERROR_VERIFY;
            }
        #else

            uint32_t appCrc = Cy_DFU_DataChecksum((uint8_t *)appVerifyStartAddress, appVerifySize, params);
            uint32_t appFooterAddress = (appVerifyStartAddress + appVerifySize);
            status = (*(uint32_t*)appFooterAddress == appCrc) ? CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
        #endif/* (CY_DFU_OPT_CRYPTO_HW != 0) */
    #endif /* (CY_DFU_APP_FORMAT == CY_DFU_CYPRESS_APP) */
    }
    return (status);
#else
    CY_UNUSED_PARAMETER(appId);
    CY_UNUSED_PARAMETER(params);
    return CY_DFU_SUCCESS;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
}


#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
/*******************************************************************************
* Function Name: Cy_DFU_GetRunningApp
****************************************************************************//**
*
* This function reports the application number of the currently running
* application.
*
* \return application number
*
*******************************************************************************/
uint32_t Cy_DFU_GetRunningApp(void)
{
    return ElfSymbolToAddr(&__cy_app_id);
}


/*******************************************************************************
* Function Name: Cy_DFU_CopyApp
****************************************************************************//**
*
* This function copies an application from a temporary location in flash to its
* destination location in flash. This function is typically called when updating
* an application used as part of an update process, for example updating
* a BLE stack.
* \note This API is only for demonstration purpose, use it only when copying
* from internal flash to internal flash. For other user cases, implement a
* custom, more general function.
*
* \param destAddress  The start address of the application to copy to.
* \param srcAddress   The start address of the copy of the application to be
*                     copied.
* \param length       The number of bytes to copy.
* \param rowSize      The size of a flash row in bytes.
* \param params       The pointer to a DFU parameters structure.
*                     See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_CopyApp(uint32_t destAddress, uint32_t srcAddress, uint32_t length,
                                            uint32_t rowSize, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;
    uint32_t writeAddr  = destAddress;
    uint32_t readAddr   = srcAddress;
    uint32_t endAddress = destAddress + length;

    while (writeAddr < endAddress)
    {
        status = Cy_DFU_ReadData(readAddr, rowSize, CY_DFU_IOCTL_READ, params);
        if (status == CY_DFU_SUCCESS)
        {
            status = Cy_DFU_WriteData(writeAddr, rowSize, CY_DFU_IOCTL_WRITE, params);
        }
        if (status != CY_DFU_SUCCESS)
        {
            break;
        }
        writeAddr += rowSize;
        readAddr  += rowSize;
    }

    return (status);
}


/*******************************************************************************
* Function Name: Cy_DFU_OnResetApp0
****************************************************************************//**
*
* This function is used in an App0 firmware image in Reset_Handler() only.
* Checks if switching to the other application is scheduled with
* \ref Cy_DFU_ExecuteApp(). \n
* If the switch is scheduled, then it validates the application and transfers
* control to it.
*
*******************************************************************************/
void Cy_DFU_OnResetApp0(void)
{
    /* Set cy_dfu_appId to ZERO under a non-software reset. This means
     * that the DFU application is scheduled - the initial clean state.
     * The value of cy_dfu_appId is valid only under a software reset.
     */
    if (Cy_SysLib_GetResetReason() != CY_SYSLIB_RESET_SOFT)
    {
        cy_dfu_appId = 0U;
    }
    else
    {
        if ((cy_dfu_appId != 0U) && (cy_dfu_appId < CY_DFU_MAX_APPS))
        {
            (void) Cy_DFU_SwitchToApp((uint32_t) cy_dfu_appId);
        }
    }
}
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */


/*******************************************************************************
* Function Name: Cy_DFU_ReadData
****************************************************************************//**
*
* This function must be implemented in the user's code.
*
* Reads \c buffer from flash, QSPI flash, or any other external memory type with
* custom pre and post read commands.
*
* \param address    The address from where to read data, must be aligned to
*                   a flash row, QSPI flash page, etc.
* \param length     The length in bytes of data to read, must be multiple of
*                   a flash row, QSPI flash page, etc.
* \param ctl        Additional features of the read function:
* - CY_DFU_IOCTL_READ    - Only read.
* - CY_DFU_IOCTL_COMPARE - Compare the data in the buffer with the data in
*                               memory.
* - CY_DFU_IOCTL_BHP     - Decrypt data before comparing the buffer with
*                               memory,
*   if the DFU Host provided encrypted data.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
* \return See \ref cy_en_dfu_status_t
* - CY_DFU_SUCCESS - If successful.
* - CY_DFU_ERROR_LENGTH if \c The length value is invalid.
* - CY_DFU_ERROR_ADDRESS if \c The address is invalid.
*
*******************************************************************************/
__WEAK cy_en_dfu_status_t Cy_DFU_ReadData (uint32_t address, uint32_t length, uint32_t ctl,
                                                     cy_stc_dfu_params_t *params)
{
    /*
    * This function does nothing, weak implementation.
    * The purpose of this code is to disable compiler warnings for Non-optimized
    * builds which do not remove unused functions and require them for the
    * completeness of the linking step.
    */

    (void) address;
    (void) length;
    (void) ctl;
    (void) params;

    return (CY_DFU_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_DFU_WriteData
****************************************************************************//**
*
* This function must be implemented in the user's code.
*
* Writes the \c buffer to flash, QSPI flash, or any other external memory type
* with custom pre and post write commands.
*
* \param address    The address to write data to, must be aligned to a flash
*                   row, QSPI flash page, etc.
* \param length     The length in bytes of data to be written, must be multiple
*                   of a flash row, QSPI flash page, etc.
* \param ctl        Additional features of the write function:
* - CY_DFU_IOCTL_WRITE - Only write.
* - CY_DFU_IOCTL_ERASE - Erase the sector, the sector size can be bigger
*   than the size of the page to write.
* - CY_DFU_IOCTL_BHP   - Decrypt data before writing to memory, if
*   the DFU Host provided encrypted data.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
* \return See \ref cy_en_dfu_status_t.
* - CY_DFU_SUCCESS - If successful.
* - CY_DFU_ERROR_LENGTH if \c The length value is invalid.
* - CY_DFU_ERROR_ADDRESS if \c The address is invalid.
*
*******************************************************************************/
__WEAK cy_en_dfu_status_t Cy_DFU_WriteData(uint32_t address, uint32_t length, uint32_t ctl,
                                                     cy_stc_dfu_params_t *params)
{
    /*
    * This function does nothing, weak implementation.
    * The purpose of this code is to disable compiler warnings for Non-optimized
    * builds which do not remove unused functions and require them for the
    * completeness of the linking step.
    */

    (void) address;
    (void) length;
    (void) ctl;
    (void) params;

    return (CY_DFU_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportRead
****************************************************************************//**
* This function must be implemented in the user's code.
*
* This function receives a command packet from the DFU Host via the
* communication channel. The function waits for a timeout until all bytes are
* received.
*
* \param buffer The pointer to a buffer to store a received command.
* \param size   The number of bytes to read.
* \param count  The pointer to the variable that contains the number of received
*               bytes.
* \param timeout The time to wait before the function returns because of a
*                timeout, in milliseconds.
*
* \return The status of the transmit operation:
* - CY_DFU_SUCCESS - If successful.
* - CY_DFU_ERROR_TIMEOUT - If no data is received.
* - See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
__WEAK cy_en_dfu_status_t Cy_DFU_TransportRead(uint8_t buffer[], uint32_t size, uint32_t *count,
                                                         uint32_t timeout)
{
    /*
    * This function does nothing, weak implementation.
    * The purpose of this code is to disable compiler warnings for Non-optimized
    * builds which do not remove unused functions and require them for the
    * completeness of the linking step.
    */

    (void) buffer;
    (void) size;
    (void) count;
    (void) timeout;

    return (CY_DFU_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportWrite
****************************************************************************//**
* This function must be implemented in the user's code.
*
* This function transmits a response packet to the DFU host via the
* communication channel. The function waits for a timeout until all bytes are
* sent.
*
* \param buffer The pointer response packet buffer.
* \param size   The number of bytes to transmit.
* \param count  The pointer to the actual number of transmitted bytes.
* \param timeout The time to wait before the function returns because of a
*        timeout, in milliseconds
*
* \return See \ref cy_en_dfu_status_t.
* The status of the transmit operation:
* - CY_DFU_SUCCESS - If successful.
* - CY_DFU_ERROR_TIMEOUT - If no data is transmitted.
*
*******************************************************************************/
__WEAK cy_en_dfu_status_t Cy_DFU_TransportWrite(uint8_t buffer[], uint32_t size, uint32_t *count,
                                                          uint32_t timeout)
{
    /*
    * This function does nothing, weak implementation.
    * The purpose of this code is to disable compiler warnings for Non-optimized
    * builds which do not remove unused functions and require them for the
    * completeness of the linking step.
    */

    (void) buffer;
    (void) size;
    (void) count;
    (void) timeout;

    return (CY_DFU_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportReset
****************************************************************************//**
*
* This function must be implemented in the user's code. \n
* Resets the communication interface with clearing buffers, offsets, length,
* etc.
*
*******************************************************************************/
__WEAK void Cy_DFU_TransportReset(void)
{
    /*
    * This function does nothing, weak implementation.
    * The purpose of this code is to disable compiler warnings for Non-optimized
    * builds which do not remove unused functions and require them for the
    * completeness of the linking step.
    */
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportStart
****************************************************************************//**
*
* This function must be implemented in the user's code. \n
* Starts the communication interface through which updating will be working.
*
* \param transport defines transport interface to use. See
*           \ref cy_en_dfu_transport_t for available options
*
*******************************************************************************/
__WEAK void Cy_DFU_TransportStart(cy_en_dfu_transport_t transport)
{
    /*
    * This function does nothing, weak implementation.
    * The purpose of this code is to disable compiler warnings for Non-optimized
    * builds which do not remove unused functions and require them for the
    * completeness of the linking step.
    */
   CY_UNUSED_PARAMETER(transport);
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportStop
****************************************************************************//**
*
* This function must be implemented in the user's code. \n
* Stops the communication interface through which updating will be working.
*
*******************************************************************************/
__WEAK void Cy_DFU_TransportStop(void)
{
    /*
    * This function does nothing, weak implementation.
    * The purpose of this code is to disable compiler warnings for Non-optimized
    * builds which do not remove unused functions and require them for the
    * completeness of the linking step.
    */
}


/*******************************************************************************
*        Cy_DFU_Continue related code, till the EOF
*******************************************************************************/


/*******************************************************************************
* Function Name: GetU16
****************************************************************************//**
*
* This function is used to read a uint16_t value from a byte array. \n
* MISRA is informed with pragma that the function has no side effects.
*
* \param array      The pointer to the byte array to get data from.
*
* \return The uint16_t value from the \c array parameter.
*
*******************************************************************************/
static uint16_t GetU16(uint8_t const array[])
{
    uint16_t temp;
    (void) memcpy( (void*)&temp, (const void*)&array[0], UINT16_SIZE);
    return (temp);
}


/*******************************************************************************
* Function Name: GetU32
****************************************************************************//**
*
* This function is used to read an uint32_t value from a byte array. \n
* MISRA is informed with pragma that the function has no side effects.
*
* \param array      The pointer to the byte array to get data from.
*
* \return The uint32_t value from the \c array parameter.
*
*******************************************************************************/
static uint32_t GetU32(uint8_t const array[])
{
    uint32_t temp;
    (void) memcpy( (void*)&temp, (const void*)&array[0], UINT32_SIZE);
    return (temp);
}


/*******************************************************************************
* Function Name: PutU16
****************************************************************************//**
*
* This function is used to write an uint16_t value to a byte array.
*
* \param array      The pointer to the byte array.
* \param offset     The offset within the byte array to write data to.
* \param value      The value to be stored to the byte array.
*
*******************************************************************************/
static void PutU16(uint8_t array[], uint32_t offset, uint32_t value)
{
    (void) memcpy( (void*)&array[offset], (const void*)&value, UINT16_SIZE);
}


#if (CY_DFU_METADATA_WRITABLE != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW)
    /*******************************************************************************
    * Function Name: PutU32
    ****************************************************************************//**
    *
    * This function is used to write an uint32_t value to a byte array.
    *
    * \param array      The pointer to the byte array.
    * \param offset     The offset within the byte array to write data to.
    * \param value      The value to be stored to the byte array.
    *
    *******************************************************************************/
    static void PutU32(uint8_t array[], uint32_t offset, uint32_t value)
    {
        (void) memcpy( (void*)&array[offset], (const void*)&value, UINT32_SIZE);
    }
#endif /* (CY_DFU_METADATA_WRITABLE != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW)*/


/*******************************************************************************
* Function Name: PacketChecksumIndex
****************************************************************************//**
*
* This function returns an index to the checksum field in a received packet
*
* \param size The DFU packet size value.
*
* \return The index to the checksum.
*
*******************************************************************************/
static uint32_t PacketChecksumIndex(uint32_t size)
{
    return (PACKET_DATA_IDX + size);
}


/*******************************************************************************
* Function Name: PacketEopIndex
****************************************************************************//**
*
* This function returns an index to the end of the packet field in a received
* packet.
*
* \param size  The DFU packet size value.
*
* \return Returns an index to the end of the packet.
*
*******************************************************************************/
static uint32_t PacketEopIndex(uint32_t size)
{
    return (PACKET_DATA_IDX + size + PACKET_CHECKSUM_LENGTH);
}


/*******************************************************************************
* Function Name: GetPacketCommand
****************************************************************************//**
*
* This function returns a DFU packet command value for a given \c packet.
*
* \param packet  The pointer to the byte array containing DFU packet
*                data.
*
* \return A DFU packet command value.
*
*******************************************************************************/
static uint32_t GetPacketCommand(const uint8_t packet[])
{
    return ( (uint32_t) packet[PACKET_CMD_IDX] );
}


/*******************************************************************************
* Function Name: GetPacketDSize
****************************************************************************//**
*
* This function returns a data size in the DFU packet. \n
* MISRA requires this function have no side effects.
*
* \param packet  The pointer to the byte array containing DFU packet
*                data.
*
* \return A size of the data in bytes in the packet.
*
*******************************************************************************/
static uint32_t GetPacketDSize(const uint8_t packet[])
{
    return ( (uint32_t) GetU16( &packet[PACKET_SIZE_IDX] ) );
}


/*******************************************************************************
* Function Name: GetPacketData
****************************************************************************//**
*
* This function is used to get packet data.
*
* \param packet  The pointer to the byte array containing DFU packet
*                data.
* \param offset  The offset within the data bytes in the packet.
*                E.g. 0 means the first byte of the data.
*
* \return A pointer to the data bytes with the offset in the packet.
*
*******************************************************************************/
static uint8_t* GetPacketData(uint8_t packet[], uint32_t offset)
{
    return ( &packet[PACKET_DATA_IDX + offset] );
}


/*******************************************************************************
* Function Name: GetPacketChecksum
****************************************************************************//**
*
* This function is used to get a DFU packet checksum.
*
* \param packet      The pointer to a byte array containing the
*                    DFU packet data.
* \param packetSize  The offset within the data bytes in the packet. \n
*                    E.g. 0 means the first byte of the data.
*
* \return The DFU packet checksum.
*
*******************************************************************************/
static uint32_t GetPacketChecksum(const uint8_t packet[], uint32_t packetSize)
{
    return ( (uint32_t) GetU16( &packet[ PacketChecksumIndex(packetSize) ] ) );
}


/*******************************************************************************
* Function Name: ValidatePacketFooter
****************************************************************************//**
*
* This function is used to validate a DFU packet footer.
*
* \param packet      The pointer to a byte array containing the
*                    DFU packet data.
* \param packetSize  The offset within the data bytes in the packet. \n
*                    E.g. 0 means the first byte of the data.
* \return
* - 0, if the DFU packet footer is invalid.
* - 1, if the DFU packet footer is valid.
*
*******************************************************************************/
static uint32_t ValidatePacketFooter(const uint8_t packet[], uint32_t packetSize)
{
    return ( (packet[PacketEopIndex(packetSize)] == PACKET_EOP_VALUE)? 1UL : 0UL );
}


/*******************************************************************************
* Function Name: SetPacketHeader
****************************************************************************//**
*
* This function is used to set a DFU packet header value.
*
* \param packet     The pointer to a byte array containing the DFU packet
*                   data.
*
*******************************************************************************/
static void SetPacketHeader(uint8_t packet[])
{
    packet[PACKET_SOP_IDX] = PACKET_SOP_VALUE;
}


/*******************************************************************************
* Function Name: SetPacketCmd
****************************************************************************//**
*
* This function is used to set a DFU packet command value.
*
* \param packet     The pointer to a byte array containing the
*                   DFU packet data.
* \param cmd        The command value to be set in the DFU packet.
*
*******************************************************************************/
static void SetPacketCmd(uint8_t packet[], uint32_t cmd)
{
    packet[PACKET_CMD_IDX] = (uint8_t)cmd;
}


/*******************************************************************************
* Function Name: SetPacketDSize
****************************************************************************//**
*
* This function is used to set a DFU packet size value.
*
* \param packet     The pointer to a byte array containing the
*                   DFU packet data.
* \param size       The value for the DFU packet size.
*
*******************************************************************************/
static void SetPacketDSize(uint8_t packet[], uint32_t size)
{
    PutU16(packet, PACKET_SIZE_IDX, size);
}


/*******************************************************************************
* Function Name: SetPacketChecksum
****************************************************************************//**
*
* This function is used to set a DFU packet checksum value.
*
* \param packet     The pointer to a byte array containing the
*                   DFU packet data.
* \param size       The DFU packet size value.
* \param checksum   The value for a DFU packet checksum.
*
*******************************************************************************/
static void SetPacketChecksum(uint8_t packet[], uint32_t size, uint32_t checksum)
{
    PutU16(packet, PacketChecksumIndex(size), checksum);
}


/*******************************************************************************
* Function Name: SetPacketFooter
****************************************************************************//**
*
* This function is used to set a DFU packet footer value.
*
* \param packet     The pointer to a byte array containing the
*                   DFU packet data.
* \param size       The DFU packet size value.
*
*******************************************************************************/
static void SetPacketFooter(uint8_t packet[], uint32_t size)
{
    packet[PacketEopIndex(size)] = PACKET_EOP_VALUE;
}


#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
/*******************************************************************************
* Function Name: Cy_DFU_ValidateMetadata
****************************************************************************//**
*
* The function checks if the DFU metadata is valid. It calculates CRC-32C and
* compare with stored value at the end of metadata.
*
* \param metadataAddress    Start address of the DFU metadata location.
* \param params             The pointer to a DFU parameters structure.
*                           See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t
* - \ref CY_DFU_SUCCESS - metadata is valid.
* - \ref CY_DFU_ERROR_VERIFY - metadata is not valid.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_ValidateMetadata(uint32_t metadataAddress, cy_stc_dfu_params_t *params)
{
   const uint32_t metadataLength = ElfSymbolToAddr(&__cy_boot_metadata_length);

   uint32_t crc = Cy_DFU_DataChecksum( (uint8_t *)metadataAddress, metadataLength - CRC_CHECKSUM_LENGTH, params);
   uint32_t crcMeta = *(uint32_t *)(metadataAddress + (metadataLength - CRC_CHECKSUM_LENGTH) );
   cy_en_dfu_status_t status = (crc == crcMeta) ? CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
   return (status);
}
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */


/*******************************************************************************
* Function Name: PacketChecksum
****************************************************************************//**
*
* This function computes a 16-bit checksum for the provided number of bytes
* contained
* in the provided buffer. \n
* This function is used to calculate the checksum of DFU packets.
*
* MISRA requires this function have no side effects.
*
* \param buffer     The buffer containing the data to compute the checksum for.
* \param size       The number of bytes in the buffer to compute the checksum
* for.
*
* \return A 16-bit checksum for the provided data
*
*******************************************************************************/
static uint32_t PacketChecksum(const uint8_t buffer[], uint32_t size)
{
#if (CY_DFU_OPT_PACKET_CRC != 0U)
    uint16_t crc = CRC_CCITT_INIT;
    uint16_t tmp;
    uint32_t i;
    uint16_t tmpIndex;

    size += PACKET_DATA_IDX; /* 4 bytes before data in Cypress DFU packet */

    tmpIndex = (uint16_t)size;

    if(0U == size)
    {
        crc = ~crc;
    }
    else
    {
        do
        {
            tmp = buffer[tmpIndex - size];

            for (i = 0U; i < 8U; i++)
            {
                if (0U != ((crc & 0x0001U) ^ (tmp & 0x0001U)))
                {
                    crc = (crc >> 1U) ^ CRC_CCITT_POLYNOMIAL;
                }
                else
                {
                    crc >>= 1U;
                }

                tmp >>= 1U;
            }

            size--;
        }
        while(0U != size);

        crc = ~crc;
        tmp = crc;
        crc = ((uint16_t)(crc << 8U) | (tmp >> 8U) ) & 0xFFFFU;
    }

    return ((uint32_t)crc);
#else
    uint16_t sum = 0U;
    size += PACKET_DATA_IDX; /* 4 bytes before data in Cypress DFU packet */

    while (size > 0U)
    {
        size--;
        sum += buffer[size];
    }

    return ( (uint32_t) (  (1U + ~(uint32_t)sum) & 0xFFFFU  ) );
#endif /* CY_DFU_OPT_PACKET_CRC != 0U */
}


/*******************************************************************************
* Function Name: Cy_DFU_DataChecksum
****************************************************************************//**
*
* This function computes a CRC-32C for the provided number of bytes contained
* in the provided buffer. \n
* This function is used to validate the Program Data and Verify Data DFU
* commands and a metadata row.
* \note Ensure the Crypto block is properly initialized
* if \ref CY_DFU_OPT_CRYPTO_HW is set.
*
* \param address    The pointer to a buffer containing the data to compute
*                   the checksum for.
* \param length     The number of bytes in the buffer to compute the checksum
*                   for.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return CRC-32C for the provided data.
*
*******************************************************************************/
uint32_t Cy_DFU_DataChecksum(const uint8_t *address, uint32_t length, cy_stc_dfu_params_t *params)
{
    /* Note: param is unused by the current implementation */
    /* but it may be used in the future, if the Crypto API changes */
    (void)params;

#if CY_DFU_OPT_CRYPTO_HW != 0 /* Use PDL Hardware Crypto API */
    /* Note that the length will be < 64KB due to the hardware limitation in the Crypto Block. */
    /* If the block size is bigger, use software implementation instead. */

    uint32_t crcOut = 0U;

    cy_stc_crypto_context_crc_t cryptoCrcContext;
    cy_en_crypto_status_t cryptoStatus;

    cryptoStatus = Cy_Crypto_Enable();
    if (cryptoStatus == CY_CRYPTO_SUCCESS)
    {
        cryptoStatus = Cy_Crypto_Crc_Init( CRC_POLYNOMIAL,     CRC_DATA_REVERSE,
                                           CRC_DATA_XOR  ,     CRC_REM_REVERSE ,
                                           CRC_REM_XOR,        &cryptoCrcContext );
        if (cryptoStatus == CY_CRYPTO_SUCCESS)
        {
            cryptoStatus = Cy_Crypto_Sync(CY_CRYPTO_SYNC_BLOCKING);
        }
        if (cryptoStatus == CY_CRYPTO_SUCCESS)
        {
CY_MISRA_DEVIATE_BLOCK_START('MISRA C-2012 Rule 11.8',1,'Removing const does not have negative impact as function does not modify data.');
            cryptoStatus = Cy_Crypto_Crc_Run (
                /* dataPtr */ (void *)address,          /* length        */ (uint16_t) length,
                /* crcPtr  */ &crcOut,                  /* lfsrInitState */ CRC_LFSR_SEED,
                /* cfContext     */ &cryptoCrcContext  );
        }
CY_MISRA_BLOCK_END('MISRA C-2012 Rule 11.8');
        if (cryptoStatus == CY_CRYPTO_SUCCESS)
        {
            cryptoStatus = Cy_Crypto_Sync(CY_CRYPTO_SYNC_BLOCKING);
        }

        (void) Cy_Crypto_Disable();
    }
    if (cryptoStatus != CY_CRYPTO_SUCCESS)
    {
        CY_HALT();
    }

    return (crcOut);
#else /* Use software implementation */
    /* Contains generated values to calculate CRC-32C by 4 bits per iteration*/
    static const uint32_t crcTable[CRC_TABLE_SIZE] =
    {
        0x00000000U, 0x105ec76fU, 0x20bd8edeU, 0x30e349b1U,
        0x417b1dbcU, 0x5125dad3U, 0x61c69362U, 0x7198540dU,
        0x82f63b78U, 0x92a8fc17U, 0xa24bb5a6U, 0xb21572c9U,
        0xc38d26c4U, 0xd3d3e1abU, 0xe330a81aU, 0xf36e6f75U,
    };

    uint32_t crc = CRC_INIT;
    if (length != 0U)
    {
        do
        {
            crc = crc ^ *address;
            crc = (crc >> NIBBLE_POS) ^ crcTable[crc & NIBBLE_MSK];
            crc = (crc >> NIBBLE_POS) ^ crcTable[crc & NIBBLE_MSK];
            --length;
            ++address;
        } while (length != 0U);
    }
    return (~crc);
#endif /* CY_DFU_OPT_CRYPTO_HW != 0 */
}


/*******************************************************************************
* Function Name: VerifyPacket
****************************************************************************//**
*
* This function is used inside DFU_ReadVerifyPacket() to verify if a
* received packet is correct.
*
* \param numberRead     The number of bytes read from the communication
*                       interface.
* \param packet         The pointer to the DFU packet buffer.
*
* \return  See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
static cy_en_dfu_status_t VerifyPacket(uint32_t numberRead, const uint8_t packet[])
{
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    if ((numberRead < CY_DFU_PACKET_MIN_SIZE) || (packet[PACKET_SOP_IDX] != PACKET_SOP_VALUE))
    {
        status = CY_DFU_ERROR_DATA;
    }
    else
    {
        uint32_t packetSize = GetPacketDSize(packet);

        /*
         * If the whole packet length exceeds the number of bytes that have
         * been read by the communication component or the size of
         * the buffer that is reserved for the packet, then give an error.
         */
        if (   ((packetSize + CY_DFU_PACKET_MIN_SIZE) > numberRead)
            || ((packetSize + CY_DFU_PACKET_MIN_SIZE) > CY_DFU_SIZEOF_CMD_BUFFER)  )
        {
            status = CY_DFU_ERROR_LENGTH;
        }
        else /* The packet length is OK */
        {
            if ( ValidatePacketFooter(packet, packetSize) == 0U )
            {
                status = CY_DFU_ERROR_DATA;
            }
            else
            {
                uint32_t pktChecksum = GetPacketChecksum(packet, packetSize);
                if (pktChecksum != PacketChecksum(packet, packetSize) )
                {
                    status = CY_DFU_ERROR_CHECKSUM;
                }
            }
        }
    }
    return (status);
}


/*******************************************************************************
* Function Name: ReadVerifyPacket
****************************************************************************//**
*
* This function is used inside Cy_DFU_Continue to read and verify
* a received DFU packet.
*
* \param packet       The pointer to the DFU packet buffer.
* \param noResponse   The pointer to a variable that states whether to send
*                      a response back to a DFU Host or not.
* \param timeout      The timeout in milliseconds to wait.
*
* \return See \ref cy_en_dfu_status_t
* - \ref CY_DFU_SUCCESS - If a packet is successfully received.
* - \ref CY_DFU_ERROR_TIMEOUT - If no packet is received during
*   the timeout period.
*
*******************************************************************************/
static cy_en_dfu_status_t ReadVerifyPacket(uint8_t packet[], bool *noResponse, uint32_t timeout)
{
    cy_en_dfu_status_t status;
    uint32_t numberRead = 0U;

    status = Cy_DFU_TransportRead( packet, CY_DFU_SIZEOF_CMD_BUFFER, &numberRead, timeout );

    if (status == CY_DFU_ERROR_TIMEOUT)
    {
        *noResponse = true;
    }
    if (status == CY_DFU_SUCCESS)
    {
        status = VerifyPacket(numberRead, packet);
    }
    return (status);
}


/*******************************************************************************
* Function Name: WritePacket
****************************************************************************//**
*
* This function creates a DFU response packet and transmits it back to
* the DFU host application over the already established communications
* protocol.
*
* \param status     The error response code of the DFU response packet.
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The number of bytes contained within the \c packet
*                   to pass back to a DFU Host.
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
static cy_en_dfu_status_t WritePacket(cy_en_dfu_status_t status, uint8_t *packet, uint32_t rspSize)
{
    uint32_t checksum;
    /*
    * The DFU Host expects only one byte of the status,
    * its value must be compatible with the PSoC 3/4/5LP Bootloader Component
    */
    uint32_t statusCode = (uint32_t)status & STATUS_BYTE_MSK;

    /* Build a packet */
    SetPacketHeader(packet);
    SetPacketCmd   (packet, statusCode);
    SetPacketDSize (packet, rspSize);

    checksum = PacketChecksum(packet, rspSize);
    SetPacketChecksum(packet, rspSize, checksum);
    SetPacketFooter  (packet, rspSize);

    return ( Cy_DFU_TransportWrite(packet, rspSize + CY_DFU_PACKET_MIN_SIZE, &rspSize,
                                        TRANSPORT_WRITE_TIMEOUT));
}


/*******************************************************************************0
* Function Name: EnterResponse
****************************************************************************//**
*
* This function is used inside Cy_DFU_ContinueEntr() to add version and
* size fields.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param state      The pointer to a state variable, that is updated by
*                   the function. See \ref group_dfu_macro_state.
*
*******************************************************************************/
static void EnterResponse(uint8_t *packet, uint32_t *rspSize, uint32_t *state)
{
    static const cy_stc_dfu_enter_t dfuVersion =
    {
        CY_DFU_SILICON_ID,
        (uint8_t) CY_DFU_SILICON_REV,
        {
            (uint8_t) CY_DFU_SDK_VERSION_MINOR,
            (uint8_t) CY_DFU_SDK_VERSION_MAJOR,
            0x01U /* Used for BWC with the Bootloader component */
        }
    };
    *state = CY_DFU_STATE_UPDATING;
    *rspSize = sizeof(dfuVersion);
    (void) memcpy( (void*)GetPacketData(packet, PACKET_DATA_NO_OFFSET),
                    (const void*)&dfuVersion, *rspSize);
}


/*******************************************************************************
* Function Name: CommandEnter
****************************************************************************//**
*
* This is a helper function for Cy_DFU_Continue().
*
* This function is called to handle Command Enter, see the DFU Commands
* section in the DFU SDK User's Guide.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param state      The pointer to a state variable, that is updated by
*                   the function. See \ref group_dfu_macro_state.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
static cy_en_dfu_status_t CommandEnter(uint8_t *packet, uint32_t *rspSize, uint32_t *state,
                                            cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_LENGTH;
     volatile uint32_t productId;
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
    productId = ElfSymbolToAddr(&__cy_product_id);
#else
    productId = CY_DFU_PRODUCT;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
    uint32_t packetSize = GetPacketDSize(packet);
    *rspSize = CY_DFU_RSP_SIZE_0;

    if (packetSize == 0U)  /* 'Product ID' not demanded */
    {
        status = (productId == 0U)? CY_DFU_SUCCESS : CY_DFU_ERROR_LENGTH;
        if (status == CY_DFU_SUCCESS)
        {
            EnterResponse(packet, rspSize, state);
        }
    }
    else if ( (packetSize == DATA_PACKET_SIZE_4BYTES) || (packetSize == DATA_PACKET_SIZE_6BYTES) )
    {
        status = CY_DFU_ERROR_DATA;
        if (productId == GetU32(GetPacketData(packet, PACKET_DATA_NO_OFFSET) ) )
        {
            EnterResponse(packet, rspSize, state);
            status = CY_DFU_SUCCESS;
        }
    }
    else
    {
        /* Empty */
    }

    CY_UNUSED_PARAMETER(params); /* Remove the unused warning */

    return (status);
}


/*******************************************************************************
* Function Name: CopyToDataBuffer
****************************************************************************//**
*
* This is a helper function, called in the following functions:
* - DFU_CommandProgramData
* - CommandVerifyData
* - CommandSendData
*
* This is used to copy packet data to dataBuffer and increase dataOffset.
*
* \param dataBuffer     The pointer to a buffer containing the data to be
*                       written to an NVM.
* \param dataOffset     The offset within dataBuffer, indicates the current
*                       dataBuffer length.
* \param packet         The pointer to the DFU packet buffer.
* \param packetSize     The length in bytes of the data in the DFU
*                       packet buffer.
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
static cy_en_dfu_status_t CopyToDataBuffer(uint8_t dataBuffer[], uint32_t *dataOffset, uint8_t const packet[],
                                                uint32_t packetSize)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;
    if ( (dataBuffer != NULL) && (dataOffset != NULL) && (packet != NULL) )
    {
        status = CY_DFU_ERROR_LENGTH;
        if ( (*dataOffset + packetSize) <= CY_DFU_SIZEOF_DATA_BUFFER )
        {
            status = CY_DFU_SUCCESS;
            (void) memcpy( &dataBuffer[*dataOffset], packet, packetSize);
            *dataOffset += packetSize;
        }
    }
    return (status);
}


/*******************************************************************************
* Function Name: CommandProgramData
****************************************************************************//**
*
* This a helper function for Cy_DFU_Continue().
* This function is used to program data to an NVM.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
static cy_en_dfu_status_t CommandProgramData(uint8_t  *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_LENGTH;
    uint8_t  *dataBufferLocal =  params->dataBuffer;
    uint32_t *dataOffsetLocal = &params->dataOffset;
    uint32_t  packetSize =  GetPacketDSize(packet);

    *rspSize = CY_DFU_RSP_SIZE_0;
    if (packetSize >= PARAMS_SIZE)
    {
        uint32_t address =  GetU32( GetPacketData(packet, PACKET_DATA_NO_OFFSET) );
        uint32_t crc = GetU32( GetPacketData(packet, PROGRAM_DATA_CRC_OFFSET) );

        /* Data may be sent with the Program Data DFU command, so copy it to dataBuffer */
        status = CopyToDataBuffer(dataBufferLocal, dataOffsetLocal, GetPacketData(packet, PARAMS_SIZE),
                                  packetSize - PARAMS_SIZE );

        if (status == CY_DFU_SUCCESS)
        {
            if (crc != Cy_DFU_DataChecksum(dataBufferLocal, *dataOffsetLocal, params) )
            {
                status = CY_DFU_ERROR_CHECKSUM;
            }
        }
        if (status == CY_DFU_SUCCESS)
        {
            status = Cy_DFU_WriteData(address, *dataOffsetLocal, CY_DFU_IOCTL_BHP, params);
        }
        if (status == CY_DFU_SUCCESS)
        {
            status = Cy_DFU_ReadData (address, *dataOffsetLocal, CY_DFU_IOCTL_COMPARE, params);
        }
    } /* if (packetSize >= PARAMS_SIZE) */
    *dataOffsetLocal = 0U;
    return (status);
}


#if CY_DFU_OPT_ERASE_DATA != 0
/*******************************************************************************
* Function Name: CommandEraseData
****************************************************************************//**
*
* This is a helper function for Cy_DFU_Continue().
* This function erases an NVM row or page.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
static cy_en_dfu_status_t CommandEraseData(uint8_t  *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_LENGTH;
    *rspSize = CY_DFU_RSP_SIZE_0;
    if (GetPacketDSize(packet) == DATA_PACKET_SIZE_4BYTES)
    {
        uint32_t address = GetU32( GetPacketData(packet, PACKET_DATA_NO_OFFSET) );
        status = Cy_DFU_WriteData(address, 0U, CY_DFU_IOCTL_ERASE, params);
    }
    params->dataOffset = 0U;
    return (status);
}
#endif /* CY_DFU_OPT_ERASE_DATA != 0 */


#if CY_DFU_OPT_VERIFY_DATA != 0
/*******************************************************************************
* Function Name: CommandVerifyData
****************************************************************************//**
*
* This is a helper function for Cy_DFU_Continue().
* This function verifies an NVM row or page.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
static cy_en_dfu_status_t CommandVerifyData(uint8_t  *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status    = CY_DFU_ERROR_LENGTH;
    uint8_t  *dataBufferLocal =  params->dataBuffer;
    uint32_t *dataOffsetLocal = &params->dataOffset;

    uint32_t  packetSize = GetPacketDSize(packet);
    *rspSize = CY_DFU_RSP_SIZE_0;
    if (packetSize >= PARAMS_SIZE)
    {
        uint32_t address = GetU32( GetPacketData(packet, PACKET_DATA_NO_OFFSET) );
        uint32_t crc     = GetU32( GetPacketData(packet, VERIFY_DATA_CRC_OFFSET) );

        /* Data may be sent with the Program Data DFU command, so copy it to dataBuffer */
        status = CopyToDataBuffer(dataBufferLocal, dataOffsetLocal, GetPacketData(packet, PARAMS_SIZE),
                                  packetSize - PARAMS_SIZE );

        if (status == CY_DFU_SUCCESS)
        {
            if (crc != Cy_DFU_DataChecksum(dataBufferLocal, *dataOffsetLocal, params) )
            {
                status = CY_DFU_ERROR_CHECKSUM;
            }
        }

        if (status == CY_DFU_SUCCESS)
        {
            status = Cy_DFU_ReadData(address, *dataOffsetLocal, CY_DFU_IOCTL_COMPARE, params);
            status = (status == CY_DFU_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_VERIFY;
        }
    }
    *dataOffsetLocal = 0U;
    return (status);
}
#endif /* CY_DFU_OPT_VERIFY_DATA != 0 */


#if CY_DFU_OPT_SEND_DATA != 0
/*******************************************************************************
* Function Name: CommandSendData
****************************************************************************//**
*
* This is a helper function for Cy_DFU_Continue().
* This function handles the Send Data DFU command.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
static cy_en_dfu_status_t CommandSendData(uint8_t  *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status;
    uint32_t packetSize = GetPacketDSize(packet);
    uint8_t  *dataBufferLocal =  params->dataBuffer;
    uint32_t *dataOffsetLocal = &params->dataOffset;
    *rspSize = CY_DFU_RSP_SIZE_0;

    /* Data may be sent with the Program Data DFU command, so copy it to dataBuffer */
    status = CopyToDataBuffer(dataBufferLocal, dataOffsetLocal,
                                GetPacketData(packet, PACKET_DATA_NO_OFFSET),
                                packetSize);

    return (status);
}
#endif /* CY_DFU_OPT_SEND_DATA != 0 */


#if CY_DFU_OPT_VERIFY_APP != 0
/*******************************************************************************
* Function Name: CommandVerifyApp
****************************************************************************//**
*
* This is a helper function for Cy_DFU_Continue().
* This function handles the Verify Application DFU command.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
static cy_en_dfu_status_t CommandVerifyApp(uint8_t *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status  = CY_DFU_ERROR_LENGTH;
    uint32_t packetSize = GetPacketDSize(packet);
    *rspSize = CY_DFU_RSP_SIZE_0;


    if ( (packetSize == sizeof(uint32_t) ) || (packetSize == VERIFY_APP_DATA_SIZE) )
    {
        uint32_t app = (uint32_t) *GetPacketData(packet,PACKET_DATA_NO_OFFSET);
        if (app < CY_DFU_MAX_APPS)
        {
            status = Cy_DFU_ValidateApp(app, params);
        }
        else
        {
            status = CY_DFU_ERROR_VERIFY;
        }
    }

    if ( (status == CY_DFU_SUCCESS) || (status == CY_DFU_ERROR_VERIFY) )
    {
        uint8_t *valid = GetPacketData(packet, PACKET_DATA_NO_OFFSET);
        *valid = (status == CY_DFU_SUCCESS) ? 1U : 0U;
        status = CY_DFU_SUCCESS;
        *rspSize = CY_DFU_RSP_SIZE_VERIFY_APP;
    }
    return (status);
}
#endif /* CY_DFU_OPT_VERIFY_APP != 0 */


#if ((CY_DFU_METADATA_WRITABLE != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW)) || defined(CY_DOXYGEN)
/*******************************************************************************
* Function Name: Cy_DFU_SetAppMetadata
****************************************************************************//**
*
* This is a helper function for Cy_DFU_Continue().
* This function sets application metadata and updates a metadata checksum.
* \note If the application metadata is the same as already
* present in the NVM, then the NVM is not rewritten and this function only exits.
* \note This function uses params->dataBuffer for the read and write NVM.
*
* \param appId           The application number to update metadata for.
* \param verifyAddress   An application verified area start address
* \param verifySize      The size of verified application area
* \param params          The pointer to a DFU parameters structure.
*                        See \ref cy_stc_dfu_params_t.
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_SetAppMetadata(uint32_t appId, uint32_t verifyAddress, uint32_t verifySize,
                                                   cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;
    uint32_t metadataAddress = 0U;
    uint32_t metadataLength  = 0U;
    if ((params == NULL) || (appId >= CY_DFU_MAX_APPS))
    {
        status = CY_DFU_ERROR_UNKNOWN;
    }
    if (status == CY_DFU_SUCCESS)
    {
        metadataAddress = ElfSymbolToAddr(&__cy_boot_metadata_addr  );
        metadataLength  = ElfSymbolToAddr(&__cy_boot_metadata_length);
        status = Cy_DFU_ReadData(metadataAddress, metadataLength, CY_DFU_IOCTL_READ, params);
    }
    if (status == CY_DFU_SUCCESS)
    {
        uint32_t getVerifyAddress = GetU32(params->dataBuffer + (appId * METADATA_BYTES_PER_APP));
        uint32_t getVerifySize      = GetU32(params->dataBuffer + (appId * METADATA_BYTES_PER_APP) +
                                            METADATA_APP_LENGTH_OFFSET);
        if ( (getVerifyAddress != verifyAddress) || ( getVerifySize != verifySize) )
        {
            uint32_t crc;
            PutU32(params->dataBuffer, (appId * METADATA_BYTES_PER_APP)     , verifyAddress);
            PutU32(params->dataBuffer, (appId * METADATA_BYTES_PER_APP) + METADATA_APP_LENGTH_OFFSET, verifySize);
            crc = Cy_DFU_DataChecksum(params->dataBuffer, metadataLength - CRC_CHECKSUM_LENGTH, params);
            PutU32(params->dataBuffer, metadataLength - CRC_CHECKSUM_LENGTH, crc);
            status = Cy_DFU_WriteData(metadataAddress, metadataLength, CY_DFU_IOCTL_WRITE, params);
        }
    }
    return (status);
}
#endif /* (CY_DFU_METADATA_WRITABLE != 0 CY_DFU_FLOW == CY_DFU_BASIC_FLOW) || defined(CY_DOXYGEN) */


/*******************************************************************************
* Function Name: CommandSetAppMetadata
****************************************************************************//**
* This is a helper function for Cy_DFU_Continue().
* This function handles the Set Application Metadata DFU command.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
static cy_en_dfu_status_t CommandSetAppMetadata(uint8_t *packet, uint32_t *rspSize,
                                                     cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;

    *rspSize = CY_DFU_RSP_SIZE_0;

    if (GetPacketDSize(packet) != DATA_LENGTH)
    {
        status = CY_DFU_ERROR_LENGTH;
    }

    if (status == CY_DFU_SUCCESS)
    {
        /* Data offsets 0, 1, 5 are defined in the DFU Packet Structure */
        uint32_t app       =      *( GetPacketData(packet, PACKET_DATA_NO_OFFSET) );

    #if (CY_DFU_METADATA_WRITABLE != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW)
        uint32_t verifyAddress = GetU32( GetPacketData(packet, SET_APP_METADATA_OFFSET) );
        uint32_t verifySize   = GetU32( GetPacketData(packet, SET_APP_METADATA_LENGTH_OFFSET) );

        status = Cy_DFU_SetAppMetadata(app, verifyAddress, verifySize, params);
    #endif /*(CY_DFU_METADATA_WRITABLE != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW)*/

        params->appId = app;
    }
    return (status);
}

#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW

#if CY_DFU_OPT_GET_METADATA != 0
/*******************************************************************************
* Function Name: CommandGetMetadata
****************************************************************************//**
* This is a helper function for Cy_DFU_Continue().
* This function handles the Get Metadata DFU command.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
static cy_en_dfu_status_t CommandGetMetadata(uint8_t *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;
    uint32_t locRspSize = CY_DFU_RSP_SIZE_0;

    if (packet == NULL)
    {
        status = CY_DFU_ERROR_UNKNOWN;
    }
    if (status == CY_DFU_SUCCESS)
    {
        if (GetPacketDSize(packet) != DATA_PACKET_SIZE_4BYTES)
        {
            status = CY_DFU_ERROR_LENGTH;
        }
    }
    if (status == CY_DFU_SUCCESS)
    {
        uint32_t fromAddr = GetU16( GetPacketData(packet, PACKET_DATA_NO_OFFSET) );
        uint32_t toAddr   = GetU16( GetPacketData(packet, GET_METADATA_TO_OFFSET) );
        if ( (toAddr < fromAddr)
            || ( ( (toAddr - fromAddr) + CY_DFU_PACKET_MIN_SIZE) > CY_DFU_SIZEOF_CMD_BUFFER) )
        {
            status  = CY_DFU_ERROR_DATA;
        }
        else
        {
            uint32_t metadataAddr    = ElfSymbolToAddr(&__cy_boot_metadata_addr  );
            uint32_t metadataLength  = ElfSymbolToAddr(&__cy_boot_metadata_length);

            status = Cy_DFU_ValidateMetadata(metadataAddr, params);
            if (status == CY_DFU_SUCCESS)
            {
                status = Cy_DFU_ReadData(metadataAddr, metadataLength, CY_DFU_IOCTL_READ, params);
            }
            if (status == CY_DFU_SUCCESS)
            {
                locRspSize = (toAddr - fromAddr);
                (void) memmove( GetPacketData(packet, PACKET_DATA_NO_OFFSET),
                                &(params->dataBuffer[fromAddr]),
                                locRspSize);
                status = CY_DFU_SUCCESS;
            }
        }
    }
    *rspSize = locRspSize;
    return (status);
}
#endif /* CY_DFU_OPT_GET_METADATA != 0 */


#if CY_DFU_OPT_SET_EIVECTOR != 0
/*******************************************************************************
* Function Name: CommandSetEIVector
****************************************************************************//**
*
* This is a helper function for Cy_DFU_Continue().
* This function copies the Encryption Initial Vector value from the DFU
* command buffer to the buffer pointed by
* \code params->encryptionVector \endcode.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
static cy_en_dfu_status_t CommandSetEIVector(uint8_t *packet, uint32_t *rspSize, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_SUCCESS;
    *rspSize = CY_DFU_RSP_SIZE_0;

    uint32_t size = GetPacketDSize(packet);

    if (( (size == 0U) ||  (size == DATA_PACKET_SIZE_8BYTES)
        || (size == DATA_PACKET_SIZE_16BYTES) ) && (params->encryptionVector != NULL))
    {
CY_MISRA_FP_LINE('MISRA C-2012 Rule 21.18','Per C99 standard (7.21.1/2) 0 value is allowed with no undefined behaviour. ');
        (void) memcpy((void*)params->encryptionVector, (const void*)GetPacketData(packet, PACKET_DATA_NO_OFFSET), size);
    }
    else
    {
        status = CY_DFU_ERROR_DATA;
    }

    return (status);
}
#endif /* CY_DFU_OPT_SET_EIVECTOR != 0 */
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */


/*******************************************************************************
* Function Name: CommandUnsupported
****************************************************************************//**
*
* This is a helper function for Cy_DFU_Continue().
* This function responses to an unsupported command.
*
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
static cy_en_dfu_status_t CommandUnsupported(uint8_t packet[], uint32_t *rspSize, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_CMD;

    /* To remove the compiler warnings */
    (void)packet;
    params->dataOffset = 0U;

    *rspSize = CY_DFU_RSP_SIZE_0;
    return (status);
}


/*******************************************************************************
* Function Name: ContinueHelper
****************************************************************************//**
* This is a helper function for Cy_DFU_Continue(). This function parses
*  most of the DFU commands.
*
* \param command    The DFU packet command value.
* \param packet     The pointer to the DFU packet buffer.
* \param rspSize    The pointer to a response packet size.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t .
* \param noResponse The pointer to a variable that states whether to send
*                    a response back to a DFU Host or not.
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
static cy_en_dfu_status_t ContinueHelper(uint32_t command, uint8_t *packet, uint32_t *rspSize,
                                              cy_stc_dfu_params_t *params, bool *noResponse)
{
    /* Set to a close warning (status may be used uninitialized)*/
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN;
    switch (command)
    {
    case CY_DFU_CMD_PROGRAM_DATA:
        CY_DFU_LOG_INF("Receive Program command");
        status = CommandProgramData(packet, rspSize, params);
        break;

#if CY_DFU_OPT_VERIFY_DATA != 0
    case CY_DFU_CMD_VERIFY_DATA:
        CY_DFU_LOG_INF("Receive Verify Data command");
        status = CommandVerifyData(packet, rspSize, params);
        break;
#endif /* CY_DFU_OPT_VERIFY_DATA != 0 */

#if CY_DFU_OPT_ERASE_DATA != 0
    case CY_DFU_CMD_ERASE_DATA:
        CY_DFU_LOG_INF("Receive Erase Data command");
        status = CommandEraseData(packet, rspSize, params);
        break;
#endif /* CY_DFU_OPT_ERASE_DATA != 0 */

#if CY_DFU_OPT_VERIFY_APP != 0
    case CY_DFU_CMD_VERIFY_APP:
        CY_DFU_LOG_INF("Receive Verify App command");
        status = CommandVerifyApp(packet, rspSize, params);
        break;
#endif /* CY_DFU_OPT_VERIFY_APP != 0 */

#if CY_DFU_OPT_SEND_DATA != 0
    case CY_DFU_CMD_SEND_DATA_WR:
        CY_DFU_LOG_INF("Receive Data Write command");
        *noResponse = true;
        status = CommandSendData(packet, rspSize, params);
        break;

    case CY_DFU_CMD_SEND_DATA:
        CY_DFU_LOG_INF("Receive Send Data command");
        status = CommandSendData(packet, rspSize, params);
        break;
#endif /* CY_DFU_NO_CMD_SEND_DATA == 0 */

    case CY_DFU_CMD_SYNC: /* If something fails, then the Host sends this command to reset the DFU */
        CY_DFU_LOG_INF("Receive Sync command");
        params->dataOffset = 0U;
        *noResponse = true;
        status = CY_DFU_SUCCESS;
        break;

    case CY_DFU_CMD_SET_APP_META:
        CY_DFU_LOG_INF("Receive Set App Metadata command");
        status = CommandSetAppMetadata(packet, rspSize, params);
        break;

#if (CY_DFU_OPT_GET_METADATA != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW)
    case CY_DFU_CMD_GET_METADATA:
        CY_DFU_LOG_INF("Receive Get App Metadata command");
        status = CommandGetMetadata(packet, rspSize, params);
        break;
#endif /* (CY_DFU_OPT_GET_METADATA != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW) */

#if (CY_DFU_OPT_SET_EIVECTOR != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW)
    case CY_DFU_CMD_SET_EIVECTOR:
        CY_DFU_LOG_INF("Receive Set EI Vector command");
        status = CommandSetEIVector(packet, rspSize, params);
        break;
#endif /* (CY_DFU_OPT_SET_EIVECTOR != 0) && (CY_DFU_FLOW == CY_DFU_BASIC_FLOW) */

    default:
    #if CY_DFU_OPT_CUSTOM_CMD != 0
        if((NULL != params->handlerCmd) && (command >= CY_DFU_USER_CMD_START))
        {
            status = params->handlerCmd(command, GetPacketData(packet, PACKET_DATA_NO_OFFSET), GetPacketDSize(packet),
                                        rspSize, params, noResponse);
        }
        else
    #endif /* CY_DFU_OPT_CUSTOM_CMD != 0 */
        {
            CY_DFU_LOG_ERR("Received command unsupported");
            status = CommandUnsupported(packet, rspSize, params);
        }
        break;
    } /* switch (command) */
    return (status);
}


/*******************************************************************************
* Function Name: Cy_DFU_Continue
****************************************************************************//**
*
* The function processes Host Commands according to Host Command/Response
* Protocol.
* The function waits for the Host data packet till timeout occurs. If valid
* packet is received, it decodes received command, processes it and transfer
* back a response if needed. See description of Host Command/Response Protocol
* in [AN213924](https://www.infineon.com/an213924) DFU SDK User Guide.
*
* \param state      The pointer to a state variable, that is updated by
*                   the function. See \ref group_dfu_macro_state.
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t.
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_Continue(uint32_t *state, cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_UNKNOWN; /* Give a value to a close warning */
    uint8_t *packet = params->packetBuffer; /* Receive/Transmit buffer */

    uint32_t rspSize = CY_DFU_RSP_SIZE_0;
    bool noResponse = false;        /* Indicates whether to send a response packet back to the Host */

    CY_ASSERT(params->timeout != 0U);
    CY_ASSERT(params->dataBuffer != NULL);
    CY_ASSERT(params->packetBuffer != NULL);


    if ( (*state == CY_DFU_STATE_NONE) || (*state == CY_DFU_STATE_UPDATING) )
    {
        status = ReadVerifyPacket(packet, &noResponse, params->timeout);
        if (status == CY_DFU_SUCCESS)
        {
            uint32_t command = GetPacketCommand(packet);

            if      (command == CY_DFU_CMD_ENTER)
            {
                CY_DFU_LOG_INF("Receive Start command");
                status = CommandEnter(packet, &rspSize, state, params);
            }
            else if (command == CY_DFU_CMD_EXIT)
            {
                CY_DFU_LOG_INF("Receive Exit command");
                *state = CY_DFU_STATE_FINISHED;
                noResponse = true;
            }
            else if (*state != CY_DFU_STATE_UPDATING)
            {
                CY_DFU_LOG_INF("Receive Unexpected command in current state");
                status = CY_DFU_ERROR_CMD;
            }
            else
            {
                status = ContinueHelper(command, packet, &rspSize, params, &noResponse);
            }
        }

        if (!noResponse)
        {
            (void) WritePacket(status, packet, rspSize);
        }
    }
    else
    {
        /* empty */
    }
    return (status);
}


#if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN)
/*******************************************************************************
* Function Name: Cy_DFU_RegisterUserCommand
****************************************************************************//**
*
*   Registering user commands handler.
*
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t.
*
* \param handler  user command handler
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_RegisterUserCommand(cy_stc_dfu_params_t *params, Cy_DFU_CustomCommandHandler handler)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_BAD_PARAM;

    if ((NULL != params) && (NULL != handler))
    {
        params->handlerCmd = handler;
        status = CY_DFU_SUCCESS;
    }
    return status;
}


/*******************************************************************************
* Function Name: Cy_DFU_UnRegisterUserCommand
****************************************************************************//**
*
*   Unregisters user commands handling.
*
* \param params     The pointer to a DFU parameters structure.
*                   See \ref cy_stc_dfu_params_t.
*
* \return See \ref cy_en_dfu_status_t
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_UnRegisterUserCommand(cy_stc_dfu_params_t *params)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_BAD_PARAM;

    if (NULL != params)
    {
        params->handlerCmd = NULL;
        status = CY_DFU_SUCCESS;
    }
    return status;
}

#endif /* #if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN) */

/* [] END OF FILE */
