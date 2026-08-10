/***************************************************************************//**
* \file cy_dfu.h
*
* Provides API declarations for the DFU Middleware.
*
********************************************************************************
* \copyright
* (c) (2016-2026), Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
********************************************************************************
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

#if !defined(CY_DFU_H)
#define CY_DFU_H

/**
* \mainpage
*
*
* For an overview, feature list, and prerequisites refer to
* [README.md](https://github.com/Infineon/dfu/blob/master/README.md).
*
********************************************************************************
* \section group_dfu_more_info More Information
********************************************************************************
*
* For more information, refer to the links in the
* [README.md](https://github.com/Infineon/dfu/blob/master/README.md#more-information)
*
********************************************************************************
*
* \defgroup group_dfu_functions     Functions
* \defgroup group_dfu_macro         Macros
* \{
* \defgroup group_dfu_macro_config  User Config Macros
* \}
* \defgroup group_dfu_globals       Global Variables
* \defgroup group_dfu_data_structs  Data Structures
* \defgroup group_dfu_enums         Enumerated Types
*/

#if !defined(__GNUC__) && !defined(__ARMCC_VERSION) && !defined(__ICCARM__)
    #error "Unsupported compiler, use either GNU, ARM or IAR C compilers"
#endif

#include <stdint.h>
#include "mtb_hal_system.h"
#include "dfu_user.h"
#include "cy_dfu_bwc_macro.h"

#if defined(CY_DFU_OPT_CRYPTO_HW) && (CY_DFU_OPT_CRYPTO_HW != 0)
    #include "cy_crypto.h"
#endif

#include "cy_syslib.h"


#ifdef __cplusplus
extern "C"{
#endif

/**
* \addtogroup group_dfu_macro
* \{
*/

/** The DFU SDK major version */
#define CY_DFU_SDK_MW_VERSION_MAJOR       (6)

/** The DFU SDK minor version */
#define CY_DFU_SDK_MW_VERSION_MINOR       (2)

/**
* \defgroup group_dfu_macro_state DFU State
* \{
* The state of updating. \n
* This is a set of values that the DFU state variable can hold. \n
* When Cy_DFU_Continue() and Cy_DFU_Complete() return, the state parameter
* indicates whether the update has finished successfully or what is the unsuccessful
* state.
*/
#define CY_DFU_STATE_NONE          (0U) /**< Updating has not yet started, no Enter packet received */
#define CY_DFU_STATE_UPDATING      (1U) /**< Updating is in process                                 */
#define CY_DFU_STATE_FINISHED      (2U) /**< Updating has finished successfully                     */
#define CY_DFU_STATE_FAILED        (3U) /**< Updating has finished with an error                    */
#define CY_DFU_STATE_BRIDGING      (4U) /**< Bridging mode with a companion device                  */
/** \} group_dfu_macro_state */

#define CY_DFU_PACKET_MIN_SIZE     (0x07U) /**< The smallest valid DFU packet size */

/**
* \defgroup group_dfu_macro_commands DFU Commands
* \{
*/
#define CY_DFU_CMD_ENTER           (0x38U) /**< DFU command: Enter DFU                  */
#define CY_DFU_CMD_EXIT            (0x3BU) /**< DFU command: Exit DFU                   */
#define CY_DFU_CMD_PROGRAM_DATA    (0x49U) /**< DFU command: Program Data               */
#define CY_DFU_CMD_VERIFY_DATA     (0x4AU) /**< DFU command: Verify Data                */
#define CY_DFU_CMD_ERASE_DATA      (0x44U) /**< DFU command: Erase Data                 */
#define CY_DFU_CMD_VERIFY_APP      (0x31U) /**< DFU command: Verify Application         */
#define CY_DFU_CMD_SEND_DATA       (0x37U) /**< DFU command: Send Data                  */
#define CY_DFU_CMD_SEND_DATA_WR    (0x47U) /**< DFU command: Send Data without Response */
#define CY_DFU_CMD_SYNC            (0x35U) /**< DFU command: Sync DFU                   */
#define CY_DFU_CMD_SET_APP_META    (0x4CU) /**< DFU command: Set Application Metadata   */
#define CY_DFU_CMD_GET_METADATA    (0x3CU) /**< DFU command: Get Metadata               */
#define CY_DFU_CMD_SET_EIVECTOR    (0x4DU) /**< DFU command: Set EI Vector              */
#define CY_DFU_CMD_ENTER_HOST_MODE (0x20U) /**< DFU command: Enter host mode with a companion device */
#define CY_DFU_CMD_EXIT_HOST_MODE  (0x23U) /**< DFU command: Exit host mode with a companion device  */
#define CY_DFU_USER_CMD_START      (0x50U) /**< DFU user commands: min value */
#define CY_DFU_USER_CMD_END        (0xFFU) /**< DFU user commands: max value */

/** \} group_dfu_macro_commands */

/**
* \defgroup group_dfu_macro_host_mode Host Mode Macros
* \{
*/
/** The definition of Host transport enables DFU Host mode */
#if defined(COMPONENT_DFU_HOST_I2C)     \
 || defined(COMPONENT_DFU_HOST_SPI)     \
 || defined(COMPONENT_DFU_HOST_UART)    \
 || defined(COMPONENT_DFU_HOST_CANFD)   \
 || defined(COMPONENT_DFU_HOST_USBHOST) \
 || defined(CY_DOXYGEN)
    #define CY_DFU_OPT_HOST_MODE               (1U)    /**< DFU host mode is active */
#define HOST_MODE_CMD_MODE_IDX                 (0U)    /**< Host mode index */
#define HOST_MODE_INTERFACE_IDX                (1U)    /**< Host mode interface */
#define HOST_MODE_INTERFACE_DEFAULT            (0U)    /**< Host mode single interface */
#define HOST_MODE_INTERFACE_I2C                (1U)    /**< Host mode I2C interface */
#define HOST_MODE_INTERFACE_UART               (3U)    /**< Host mode UART interface */
#define HOST_MODE_CMD_MODE_BRIDGING            (0U)    /**< Host mode is bridging */
#define PACKET_HEADER_SIZE                     (4U)    /**< Size of the DFU packet header in bytes */
#define PACKET_FOOTER_SIZE                     (3U)    /**< Size of the DFU packet footer in bytes */
#define TRANSPORT_REPLY_TIMEOUT_MS             (1000U) /**< Companion device reply timeout, mS */
#ifndef CY_DFU_TRANSPORT_REPLY_DELAY
    #define CY_DFU_TRANSPORT_REPLY_DELAY_US    (1000U) /**< Companion device reply delay, uS */
#endif /* CY_DFU_TRANSPORT_REPLY_DELAY */
#else
    #define CY_DFU_OPT_HOST_MODE               (0U)
#endif /* CY_DFU_OPT_HOST_MODE */

/** \} group_dfu_macro_host_mode */

/**
* \defgroup group_dfu_macro_ioctl Read/Write Data IO Control Values
* \{
* The values of the ctl parameter to the \ref Cy_DFU_ReadData() and \ref Cy_DFU_WriteData() functions.
* - Bit 0:
*   * 0, Normal read or write operations.
*   * 1, Erase a memory page for write operations.
*        Compare a memory page with the data in the buffer for read operation.
* - Bit 1:
*   * 0, Read or write with raw data.
*   * 1, Data received from/to be sent to the DFU Host.
*        May require encryption/decryption or any other special treatment.
         E.g. read/write data from/to an address with an offset.
* - Bit 2: Reserved.
* - Bit 3: Reserved.
* - Bit 4 - 31: Unused in DFU SDK. Up to the user to specify it.
*/

#define CY_DFU_IOCTL_READ          (0x00U) /**< Read data into the buffer                         */
#define CY_DFU_IOCTL_COMPARE       (0x01U) /**< Compare read data with the data in the buffer */

#define CY_DFU_IOCTL_WRITE         (0x00U) /**< Write the buffer to communication */
#define CY_DFU_IOCTL_ERASE         (0x01U) /**< Erase memory page             */

#define CY_DFU_IOCTL_BHP           (0x02U) /**< Data from/to DFU Host. It may require decryption. */

/** \} group_dfu_macro_ioctl */

/**
* \defgroup group_dfu_macro_response_size Response Size
* \{
*/

#define CY_DFU_RSP_SIZE_0          (0U)    /**< Data size for most DFU commands responses */
#define CY_DFU_RSP_SIZE_VERIFY_APP (1U)    /**< Data size for 'Verify Application' DFU command response */

/** \} group_dfu_macro_response_size */

/** DFU SDK PDL ID */
#define CY_DFU_ID                  CY_PDL_DRV_ID(0x06U)


/** \} group_dfu_macro */


/**
* \addtogroup group_dfu_enums
* \{
*/

/** Used to return the statuses of most DFU SDK APIs */
typedef enum
{
    /** Correct status, No error */
    CY_DFU_SUCCESS         =                                   0x00U,
    /** Verification failed */
    CY_DFU_ERROR_VERIFY       = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x02U,
    /** The length of the received packet is outside of the expected range */
    CY_DFU_ERROR_LENGTH       = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x03U,
    /** The data in the received packet is invalid */
    CY_DFU_ERROR_DATA         = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x04U,
    /** The command is not recognized */
    CY_DFU_ERROR_CMD          = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x05U,
    /** The checksum does not match the expected value */
    CY_DFU_ERROR_CHECKSUM     = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x08U,
    /** The wrong address */
    CY_DFU_ERROR_ADDRESS      = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0AU,
    /** The write to external memory device failed */
    CY_DFU_ERROR_WRITE_EXT    = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0BU,
    /** The read from external memory device failed */
    CY_DFU_ERROR_READ_EXT     = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0CU,
    /** The pointer to an external memory object is not provided.
     * Ensure that \ref Cy_DFU_AddExtMemory is called with the proper
     * input parameters.
     */
    CY_DFU_ERROR_NULL_OBJ_EXT = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0DU,
    /** The command timed out */
    CY_DFU_ERROR_TIMEOUT      = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x40U,
    /** One or more of input parameters are invalid */
    CY_DFU_ERROR_BAD_PARAM    = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x50U,
    /** An unknown DFU error, this shall not happen */
    CY_DFU_ERROR_UNKNOWN      = CY_DFU_ID | CY_PDL_STATUS_ERROR | 0x0FU
} cy_en_dfu_status_t;

/** Used to select one of the transport interfaces for the update session */
typedef enum
{
    CY_DFU_NONE    = 0x00U, /**< No transport interface selected */
    CY_DFU_I2C     = 0x01U, /**< I2C transport interface */
    CY_DFU_UART    = 0x02U, /**< UART transport interface */
    CY_DFU_SPI     = 0x03U, /**< SPI transport interface */
    CY_DFU_USB_CDC = 0x04U, /**< USB CDC transport interface */
    CY_DFU_USB_HID = 0x05U, /**< USB HID transport interface */
    CY_DFU_CANFD   = 0x06U, /**< CAN FD transport interface */
    CY_DFU_PMBUS   = 0x07U, /**< PMBus transport interface */
} cy_en_dfu_transport_t;


/** \} group_dfu_enums */


/**
* \addtogroup group_dfu_data_structs
* \{
*/
struct cy_stc_dfu_params_s;
/** The type for custom command handlers */
typedef cy_en_dfu_status_t (*Cy_DFU_CustomCommandHandler) (uint32_t command, uint8_t  *packetData, uint32_t dataSize,
                                                            uint32_t *rspSize, struct cy_stc_dfu_params_s *params,
                                                            bool *noResponse);


/**
 * Working parameters for some DFU SDK APIs to be initialized before calling DFU API.
 * */
typedef struct cy_stc_dfu_params_s
{
    /**
    * The pointer to a buffer that keeps data to read or write to an NVM.
    * It is required to be 4-byte aligned.
    */
    uint8_t  *dataBuffer;
    /**
     * An offset within \c dataBuffer to put a next chunk of data
     */
    uint32_t  dataOffset;
    /**
    * The pointer to a buffer that keeps packets sent and received with the Transport API.
    * It is required to be 4-byte aligned.
    */
    uint8_t  *packetBuffer;
    /**
     * The time (in milliseconds) for which the
     * communication interface waits to receive a new data packet
     * from Host in \ref Cy_DFU_Continue(). A typical value is 20 ms.
     */
    uint32_t  timeout;
    /**
     * Set with the Set App Metadata DFU command.
     * Used to determine an appId of a DFU image
     */
    uint32_t  appId;
    /**
     * Internal, flags if Verify Application is called before Exit
     */
    uint32_t  appVerified;

    /**
    * The initial value to the ctl parameter for
    * \ref Cy_DFU_ReadData and \ref Cy_DFU_WriteData.
    * The DFU SDK functions call the Read/Write Data functions like this: \n
    * Cy_DFU_ReadData(addr, length, CY_DFU_IOCTL_COMPARE, params).
    */
    uint32_t  initCtl;

#if (defined(CY_DFU_OPT_SET_EIVECTOR) && (CY_DFU_OPT_SET_EIVECTOR != 0)) || defined(CY_DOXYGEN)
    /**
    * The pointer to the Encryption Initialization Vector buffer.
    * Must be 0-, 8-, or 16-byte long and 4-byte aligned.
    * This may be used in \ref Cy_DFU_ReadData and \ref Cy_DFU_WriteData
    * to encrypt or decrypt data when the CY_DFU_IOCTL_BHP flag is set in the
    * ctl parameter.
    */
    uint8_t *encryptionVector;
#endif /* (CY_DFU_OPT_SET_EIVECTOR != 0) || defined(CY_DOXYGEN) */

#if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN)
    Cy_DFU_CustomCommandHandler handlerCmd; /**< User handler for the custom commands.*/
#endif /* #if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN) */

} cy_stc_dfu_params_t;

/**
* Only used inside DFU Command_Enter().
* \note A public definition because the user may want to redefine
* the DFU packet.
*/
typedef struct
{
    uint32_t enterSiliconId;             /**< The silicon ID for a device */
    uint8_t  enterRevision;              /**< Silicon Revision for a device */
    uint8_t  enterDFUVersion[3];         /**< The DFU SDK version */
} cy_stc_dfu_enter_t;
/** \} group_dfu_data_structs */


/**
* \addtogroup group_dfu_globals
* \{
*/

/** \cond INTERNAL */

/**
* \defgroup group_dfu_globals_external_elf_symbols External ELF file symbols
* \{
* CyMCUElfTools adds these symbols to a generated ELF file. \n
* Their values are either defined in the linker script (GCC, IAR)
* or in the assembly code (ARM):
* (see section \ref group_dfu_config_linker_scripts).
* They may be used by CyMCUElfTool as parameters for generating a .cyacd2 file.
* Also, use the DFU SDK APIs to refer link-time known values to the compile time.
*/

/**
 * Metadata address.
 * DFU uses this symbol to access metadata.
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_boot_metadata_addr;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * Metadata row size.
 * The DFU uses this symbol to access metadata.
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_boot_metadata_length;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * Product ID.
 * CyMCUElfTool uses this value to place in the .cyacd2 header.
 * The DFU uses this value to verify if an image is compatible with the device.
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_product_id;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * Checksum Algorithm of the DFU Host Command/Response Protocol packet.
 * Possible values
 * - 0 For the Basic Summation algorithm
 * - 1 For the CRC-16 algorithm
 * \note Must be aligned with \ref CY_DFU_OPT_PACKET_CRC
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_checksum_type;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * Current application number
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_app_id;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/**
 * CPU1 vector table address, if present
 */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
extern uint8_t __cy_app_core1_start_addr;
#endif /*CY_DFU_FLOW == CY_DFU_BASIC_FLOW*/
/** \} group_dfu_globals_external_elf_symbols */

/** \endcond*/

/** \} group_dfu_globals */

/**
* \addtogroup group_dfu_functions
* \{
*/

cy_en_dfu_status_t Cy_DFU_Init(uint32_t *state, cy_stc_dfu_params_t *params);
cy_en_dfu_status_t Cy_DFU_Continue(uint32_t *state, cy_stc_dfu_params_t *params);

uint32_t Cy_DFU_DataChecksum(const uint8_t *address, uint32_t length, cy_stc_dfu_params_t *params);

/** \cond INTERNAL */
#if CY_DFU_FLOW == CY_DFU_BASIC_FLOW
/**
* \defgroup group_dfu_functions_meta Metadata Management
* \{
*   DFU functions for operation over meta data.
*/

cy_en_dfu_status_t Cy_DFU_GetAppMetadata(uint32_t appId, uint32_t *verifyAddress, uint32_t *verifySize);
cy_en_dfu_status_t Cy_DFU_ValidateMetadata(uint32_t metadataAddress, cy_stc_dfu_params_t *params);
#if (CY_DFU_METADATA_WRITABLE != 0) || defined(CY_DOXYGEN)
    cy_en_dfu_status_t Cy_DFU_SetAppMetadata(uint32_t appId, uint32_t verifyAddress,
                                                       uint32_t verifySize, cy_stc_dfu_params_t *params);
#endif /* (CY_DFU_METADATA_WRITABLE != 0) || defined(CY_DOXYGEN) */
/** \} group_dfu_functions_meta */


/**
* \defgroup group_dfu_functions_app Application Management
* \{
*   DFU functions for the application management
*/
void Cy_DFU_ExecuteApp(uint32_t appId);
void Cy_DFU_OnResetApp0(void);
uint32_t Cy_DFU_GetRunningApp(void);
cy_en_dfu_status_t Cy_DFU_SwitchToApp(uint32_t appId);
cy_en_dfu_status_t Cy_DFU_CopyApp(uint32_t destAddress, uint32_t srcAddress, uint32_t length,
                                            uint32_t rowSize, cy_stc_dfu_params_t *params);
#endif /* CY_DFU_FLOW == CY_DFU_BASIC_FLOW */
/** \endcond*/

/**
* \defgroup group_dfu_functions_app Application Management
* \{
*   DFU functions for the application management
*/
cy_en_dfu_status_t Cy_DFU_ValidateApp(uint32_t appId, cy_stc_dfu_params_t *params);
/** \} group_dfu_functions_app */

/**
* \defgroup group_dfu_functions_mem Memory Operations
* \{
*   DFU functions for memory operations
*   These IO functions have to be re-implemented in the user's code.
*/
cy_en_dfu_status_t Cy_DFU_ReadData (uint32_t address, uint32_t length, uint32_t ctl,
                                              cy_stc_dfu_params_t *params);
cy_en_dfu_status_t Cy_DFU_WriteData(uint32_t address, uint32_t length, uint32_t ctl,
                                              cy_stc_dfu_params_t *params);
/** \} group_dfu_functions_mem */


/**
* \defgroup group_dfu_functions_transport Transport Management
* \{
*   DFU functions for the communication interface.
*   These communication functions have to be re-implemented in the user's code.
*/
cy_en_dfu_status_t Cy_DFU_TransportRead (uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout);
cy_en_dfu_status_t Cy_DFU_TransportWrite(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout);
void Cy_DFU_TransportReset(void);
void Cy_DFU_TransportStart(cy_en_dfu_transport_t transport);
void Cy_DFU_TransportStop(void);
/** \} group_dfu_functions_transport */

#if (CY_DFU_OPT_HOST_MODE != 0U)
/**
* \defgroup group_dfu_functions_host_transport Transport Management in Host mode
* \{
*   DFU functions for the communication interface in Host mode
*   These communication functions have to be re-implemented in the user's code.
*/

/*******************************************************************************
* Function Name: Cy_DFU_HostTransportRead
****************************************************************************//**
* This function is intended for use with a companion device.
*
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
cy_en_dfu_status_t Cy_DFU_HostTransportRead(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout);


/*******************************************************************************
* Function Name: Cy_DFU_HostTransportWrite
****************************************************************************//**
* This function is intended for use with a companion device.
*
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
cy_en_dfu_status_t Cy_DFU_HostTransportWrite(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout);


/*******************************************************************************
* Function Name: Cy_DFU_HostTransportReset
****************************************************************************//**
* This function is intended for use with a companion device.
*
* This function must be implemented in the user's code. \n
* Resets the communication interface with clearing buffers, offsets, length,
* etc.
*
*******************************************************************************/
void Cy_DFU_HostTransportReset(void);


/*******************************************************************************
* Function Name: Cy_DFU_HostTransportStart
****************************************************************************//**
* This function is intended for use with a companion device.
*
* This function must be implemented in the user's code. \n
* Starts the communication interface through which updating will be working.
*
* \param transport defines transport interface to use. See
*           \ref cy_en_dfu_transport_t for available options
*
*******************************************************************************/
void Cy_DFU_HostTransportStart(cy_en_dfu_transport_t transport);


/*******************************************************************************
* Function Name: Cy_DFU_HostTransportStop
****************************************************************************//**
* This function is intended for use with a companion device.
*
* This function must be implemented in the user's code. \n
* Stops the communication interface through which updating will be working.
*
*******************************************************************************/
void Cy_DFU_HostTransportStop(void);
/** \} group_dfu_functions_host_transport */
#endif /* CY_DFU_OPT_HOST_MODE */

/**
* \defgroup group_dfu_functions_custom_cmd Custom commands
* \{
*
*   The DFU protocol provides a set of pre-defined commands. The user can also
*   add custom commands and register the single handler for all custom commands
*   at the application level. This allows to adjust use case scenarios
*   per the product needs. The feature is enabled with \ref CY_DFU_OPT_CUSTOM_CMD
*   set to non-zero value in the dfu_user.h or project Makefile.
*
*   \note Custom commands only extend the functionality of the DFU command protocol
*   and must be issued after entering the updating state (\ref CY_DFU_STATE_UPDATING).
*
*   The user commands area preserved in the DFU command protocol:
*   - \ref CY_DFU_USER_CMD_START
*   - \ref CY_DFU_USER_CMD_END
*
*  An example of the custom commands usage:
*
*   1. Add a set of the custom commands to the project.
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMANDS
*
*   2. Define the function to handle the custom commands.
*   \note A single function is used as the handler for all custom commands.
*
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMAND_DECLARATION
*
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMAND_HANDLER
*
*   3. Register the function to handle custom commands as a callback in the DFU core before use.
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMAND_REGISTER
*
*   4. Release the callback function when custom command handling is no longer required.
*      \snippet snippet/source/COMPONENT_TEST_CS_COMMON/snippet_common.c DFU_USER_COMMAND_UNREGISTER
*/

#if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN)
cy_en_dfu_status_t Cy_DFU_RegisterUserCommand(cy_stc_dfu_params_t *params, Cy_DFU_CustomCommandHandler handler);
cy_en_dfu_status_t Cy_DFU_UnRegisterUserCommand(cy_stc_dfu_params_t *params);
#endif /* #if (CY_DFU_OPT_CUSTOM_CMD != 0) || defined(CY_DOXYGEN) */
/** \} group_dfu_functions_custom_cmd */

/** \} group_dfu_functions */


/***************************************
*  Internal declarations
****************************************/
/** \cond INTERNAL */

#define CY_DFU_SILICON_ID      (0U)
#define CY_DFU_SILICON_REV     (0U)

/* Cypress Basic Application Format (CyBAF) */
#define CY_DFU_BASIC_APP           (0U)
/* Cypress Secure Application Format (CySAF) - NOT SUPPORTED */
#define CY_DFU_CYPRESS_APP         (1U)
/* Simplified Secure Application Format (SSAF) - NOT SUPPORTED */
#define CY_DFU_SIMPLIFIED_APP      (2U)

/* Set the application format. Only CyBAF is supported. */
#define CY_DFU_APP_FORMAT          (CY_DFU_BASIC_APP)

#define CY_DFU_VERIFY_FAST         (0U)    /* Verification includes only
                                            * application check */
#define CY_DFU_VERIFY_FULL         (1U)    /* Verification includes application,
                                            * key, and TOC checks */

/* Set the verification type for CySAF and SSAF.
 * NOT SUPPORTED - only CyBAF is supported */
#define CY_DFU_SEC_APP_VERIFY_TYPE  (CY_DFU_VERIFY_FAST)

/*
* These defines are obsolete and kept for backward compatibility only.
* They will be removed in the future versions.
*/
#define CY_DFU_SDK_VERSION_MAJOR  (CY_DFU_SDK_MW_VERSION_MAJOR)
#define CY_DFU_SDK_VERSION_MINOR  (CY_DFU_SDK_MW_VERSION_MINOR)
/** \endcond*/

#ifdef __cplusplus
}
#endif

#endif /* !defined(CY_DFU_H) */


/* [] END OF FILE */
