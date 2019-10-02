/***************************************************************************//**
* \file cy_dfu.h
* \version 4.0
*
* Provides API declarations for the BWC with Bootloader SDK.
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_DFU_BWC_MACRO_H)
#define CY_DFU_BWC_MACRO_H    


/** \cond INTERNAL */

/*******************************************************************************
* The following code is DEPRECATED and must not be used. Kept for backward 
* compatibility.
*******************************************************************************/

/* cy_dfu.h*/

#define CY_BOOTLOADER_SDK_VERSION_MAJOR                     CY_DFU_SDK_VERSION_MAJOR
#define CY_BOOTLOADER_SDK_VERSION_MINOR                     CY_DFU_SDK_VERSION_MINOR

#define CY_BOOTLOAD_STATE_NONE                              CY_DFU_STATE_NONE         
#define CY_BOOTLOAD_STATE_BOOTLOADING                       CY_DFU_STATE_UPDATING 
#define CY_BOOTLOAD_STATE_FINISHED                          CY_DFU_STATE_FINISHED    
#define CY_BOOTLOAD_STATE_FAILED                            CY_DFU_STATE_FAILED      

#define CY_BOOTLOAD_PACKET_MIN_SIZE                         CY_DFU_PACKET_MIN_SIZE

#define CY_BOOTLOAD_CMD_ENTER                               CY_DFU_CMD_ENTER
#define CY_BOOTLOAD_CMD_EXIT                                CY_DFU_CMD_EXIT         
#define CY_BOOTLOAD_CMD_PROGRAM_DATA                        CY_DFU_CMD_PROGRAM_DATA 
#define CY_BOOTLOAD_CMD_VERIFY_DATA                         CY_DFU_CMD_VERIFY_DATA  
#define CY_BOOTLOAD_CMD_ERASE_DATA                          CY_DFU_CMD_ERASE_DATA   
#define CY_BOOTLOAD_CMD_VERIFY_APP                          CY_DFU_CMD_VERIFY_APP   
#define CY_BOOTLOAD_CMD_SEND_DATA                           CY_DFU_CMD_SEND_DATA    
#define CY_BOOTLOAD_CMD_SEND_DATA_WR                        CY_DFU_CMD_SEND_DATA_WR 
#define CY_BOOTLOAD_CMD_SYNC                                CY_DFU_CMD_SYNC         
#define CY_BOOTLOAD_CMD_SET_APP_META                        CY_DFU_CMD_SET_APP_META 
#define CY_BOOTLOAD_CMD_GET_METADATA                        CY_DFU_CMD_GET_METADATA 
#define CY_BOOTLOAD_CMD_SET_EIVECTOR                        CY_DFU_CMD_SET_EIVECTOR 

#define CY_BOOTLOAD_IOCTL_READ                              CY_DFU_IOCTL_READ    
#define CY_BOOTLOAD_IOCTL_COMPARE                           CY_DFU_IOCTL_COMPARE 
                                                                                 
#define CY_BOOTLOAD_IOCTL_WRITE                             CY_DFU_IOCTL_WRITE 
#define CY_BOOTLOAD_IOCTL_ERASE                             CY_DFU_IOCTL_ERASE   
                                                                                 
#define CY_BOOTLOAD_IOCTL_BHP                               CY_DFU_IOCTL_BHP   

#define CY_BOOTLOAD_RSP_SIZE_0                              CY_DFU_RSP_SIZE_0         
#define CY_BOOTLOAD_RSP_SIZE_VERIFY_APP                     CY_DFU_RSP_SIZE_VERIFY_APP

#define CY_BOOTLOAD_ID                                      CY_DFU_ID

#define CY_BOOTLOAD_BASIC_APP                               CY_DFU_BASIC_APP      
#define CY_BOOTLOAD_CYPRESS_APP                             CY_DFU_CYPRESS_APP    
#define CY_BOOTLOAD_SIMPLIFIED_APP                          CY_DFU_SIMPLIFIED_APP 

#define CY_BOOTLOAD_VERIFY_FAST                             CY_DFU_VERIFY_FAST 
#define CY_BOOTLOAD_VERIFY_FULL                             CY_DFU_VERIFY_FULL 

/* Types */
#define cy_stc_bootload_params_t                            cy_stc_dfu_params_t
#define cy_stc_bootload_enter_t                             cy_stc_dfu_enter_t
#define cy_en_bootload_status_t                             cy_en_dfu_status_t

/* Enums */
#define CY_BOOTLOAD_SUCCESS                                 CY_DFU_SUCCESS        
#define CY_BOOTLOAD_ERROR_VERIFY                            CY_DFU_ERROR_VERIFY   
#define CY_BOOTLOAD_ERROR_LENGTH                            CY_DFU_ERROR_LENGTH   
#define CY_BOOTLOAD_ERROR_DATA                              CY_DFU_ERROR_DATA     
#define CY_BOOTLOAD_ERROR_CMD                               CY_DFU_ERROR_CMD      
#define CY_BOOTLOAD_ERROR_CHECKSUM                          CY_DFU_ERROR_CHECKSUM 
#define CY_BOOTLOAD_ERROR_ADDRESS                           CY_DFU_ERROR_ADDRESS  
#define CY_BOOTLOAD_ERROR_TIMEOUT                           CY_DFU_ERROR_TIMEOUT  /*  */
#define CY_BOOTLOAD_ERROR_UNKNOWN                           CY_DFU_ERROR_UNKNOWN 

/* Functions */
#define Cy_Bootload_DoBootload                              Cy_DFU_Complete
#define Cy_Bootload_Init                                    Cy_DFU_Init
#define Cy_Bootload_Continue                                Cy_DFU_Continue/*  */

#define Cy_Bootload_ExecuteApp          Cy_DFU_ExecuteApp
#define Cy_Bootload_SwitchToApp         Cy_DFU_SwitchToApp
#define Cy_Bootload_DataChecksum        Cy_DFU_DataChecksum
#define Cy_Bootload_ValidateMetadata    Cy_DFU_ValidateMetadata
#define Cy_Bootload_ValidateApp         Cy_DFU_ValidateApp
#define Cy_Bootload_GetRunningApp       Cy_DFU_GetRunningApp
#define Cy_Bootload_GetAppMetadata      Cy_DFU_GetAppMetadata
#define Cy_Bootload_SetAppMetadata      Cy_DFU_SetAppMetadata
#define Cy_Bootload_CopyApp             Cy_DFU_CopyApp
#define Cy_Bootload_OnResetApp0         Cy_DFU_OnResetApp0
#define Cy_Bootload_ReadData            Cy_DFU_ReadData
#define Cy_Bootload_WriteData           Cy_DFU_WriteData
#define Cy_Bootload_TransportRead       Cy_DFU_TransportRead
#define Cy_Bootload_TransportWrite      Cy_DFU_TransportWrite
#define Cy_Bootload_TransportReset      Cy_DFU_TransportReset
#define Cy_Bootload_TransportStart      Cy_DFU_TransportStart
#define Cy_Bootload_TransportStop       Cy_DFU_TransportStop

/* Field of structure */
#define enterBootloaderVersion          enterDFUVersion 


/*USER config*/

#define CY_BOOTLOAD_SILICON_ID          CY_DFU_SILICON_ID
#define CY_BOOTLOAD_SILICON_REV         CY_DFU_SILICON_REV

#define CY_BOOTLOAD_SIZEOF_CMD_BUFFER       CY_DFU_SIZEOF_CMD_BUFFER
#define CY_BOOTLOAD_SIZEOF_DATA_BUFFER      CY_DFU_SIZEOF_DATA_BUFFER 
#define CY_BOOTLOAD_OPT_GOLDEN_IMAGE        CY_DFU_OPT_GOLDEN_IMAGE   
#define CY_BOOTLOAD_GOLDEN_IMAGE_IDS        CY_DFU_GOLDEN_IMAGE_IDS 
#define CY_BOOTLOAD_MAX_APPS                CY_DFU_MAX_APPS         
#define CY_BOOTLOAD_OPT_VERIFY_DATA         CY_DFU_OPT_VERIFY_DATA  
#define CY_BOOTLOAD_OPT_ERASE_DATA          CY_DFU_OPT_ERASE_DATA   
#define CY_BOOTLOAD_OPT_VERIFY_APP          CY_DFU_OPT_VERIFY_APP   

#define CY_BOOTLOAD_OPT_SEND_DATAGet        CY_DFU_OPT_SEND_DATAGet   
#define CY_BOOTLOAD_OPT_GET_METADATA        CY_DFU_OPT_GET_METADATA   
#define CY_BOOTLOAD_OPT_SET_EIVECTOR        CY_DFU_OPT_SET_EIVECTOR   
#define CY_BOOTLOAD_METADATA_WRITABLE       CY_DFU_METADATA_WRITABLE  
#define CY_BOOTLOAD_OPT_CRYPTO_HW           CY_DFU_OPT_CRYPTO_HW      
#define CY_BOOTLOAD_OPT_PACKET_CRC          CY_DFU_OPT_PACKET_CRC     
#define CY_BOOTLOAD_APP_FORMAT              CY_DFU_APP_FORMAT         
#define CY_BOOTLOAD_SEC_APP_VERIFY_TYPE     CY_DFU_SEC_APP_VERIFY_TYPE

#define CY_BOOTLOAD_APP0_VERIFY_START       CY_DFU_APP0_VERIFY_START       
#define CY_BOOTLOAD_APP0_VERIFY_LENGTH      CY_DFU_APP0_VERIFY_LENGTH 
#define CY_BOOTLOAD_APP1_VERIFY_START       CY_DFU_APP1_VERIFY_START  
#define CY_BOOTLOAD_APP1_VERIFY_LENGTH      CY_DFU_APP1_VERIFY_LENGTH 
#define CY_BOOTLOAD_SIGNATURE_SIZE          CY_DFU_SIGNATURE_SIZE 

/* transport_ble.h*/
#define BootloaderCallBack                  DFUCallBack

/** \endcond */

#endif /* !defined(CY_DFU_BWC_MACRO_H) */


/* [] END OF FILE */
