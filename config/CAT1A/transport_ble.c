/***************************************************************************//**
* \file transport_ble.c
* \version 4.10
*
*  This file provides the source code of the DFU communication APIs
*  for the BLE Component.
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "transport_ble.h"
#include "cy_dfu.h"

#if defined(CY_PSOC_CREATOR_USED)
#include "BLE.h"
#else
#include "cy_flash.h"
#include "cy_ble_gap.h"
#include "cy_ble_stack.h"
#endif /* defined(CY_PSOC_CREATOR_USED) */

#include "cy_ble_stack_host_error.h"
#include "cy_ble_event_handler.h"
#include "cy_ble_bts.h"

#if CY_BLE_HOST_CORE

static uint16_t cyBle_btsDataPacketIndex = 0U;
static uint8_t  cyBle_cmdReceivedFlag = 0U;
static uint16_t cyBle_cmdLength = 0U;
static uint8_t  *cyBle_btsBuffPtr;

/* Connection Handle */
cy_stc_ble_conn_handle_t appConnHandle;

static void BlessInterrupt(void);

/*******************************************************************************
* Function Name: BlessInterrupt
****************************************************************************/
static void BlessInterrupt(void)
{
    /* Call interrupt processing */
    Cy_BLE_BlessIsrHandler();
}


/*******************************************************************************
* Function Name: CyBLE_CyBtldrCommStart
****************************************************************************//**
*
* Initializes DFU state for BLE communication.
*
*******************************************************************************/
void CyBLE_CyBtldrCommStart(void)
{
#if defined(CY_PSOC_CREATOR_USED)
    /* Start BLE and register the callback function */
    (void)Cy_BLE_Start(&AppCallBack);
#else
   /*
    * Initialize and enable BLE Middleware for single CPU mode.
    * For dual CPU mode, please refer to the Configuration Considerations
    * section of BLE Middleware API reference guide
    */

    /* BLESS interrupt configure structure (for single CPU mode) */
    static cy_stc_sysint_t blessIsrCfg =
    {
        /* The BLESS interrupt */
        .intrSrc = (IRQn_Type)bless_interrupt_IRQn,

        /* The interrupt priority number */
        .intrPriority = 1U
    };

    /* Hook interrupt service routines for BLESS */
    (void) Cy_SysInt_Init(&blessIsrCfg, &BlessInterrupt);

    /* Store pointer to blessIsrCfg in BLE configuration structure */
    cy_ble_config.hw->blessIsrConfig = &blessIsrCfg;

    /* Start BLE component and register generic event handler */
    /* Register the generic event handler */
    Cy_BLE_RegisterEventCallback(&AppCallBack);

    /* Initialize the BLE host */
    (void)Cy_BLE_Init(&cy_ble_config);

    /* Enable BLE Low Power Mode (LPM)*/
    Cy_BLE_EnableLowPowerMode();

    /* Enable BLE */
    (void)Cy_BLE_Enable();
#endif /* defined(CY_PSOC_CREATOR_USED) */

    /* Registers a callback function for DFU */
    (void)Cy_BLE_BTS_RegisterAttrCallback(&DFUCallBack);
    cyBle_btsDataPacketIndex  = 0U;
}


/*******************************************************************************
* Function Name: CyBLE_CyBtldrCommStop
****************************************************************************//**
*
* Disconnects from the peer device and stops BLE component.
*
******************************************************************************/
void CyBLE_CyBtldrCommStop(void)
{
    cy_stc_ble_gap_disconnect_info_t disconnectInfoParam =
    {
        .bdHandle = appConnHandle.bdHandle,
        .reason = CY_BLE_HCI_ERROR_OTHER_END_TERMINATED_USER
    };

    /* Initiate disconnection from the peer device*/
    if(Cy_BLE_GAP_Disconnect(&disconnectInfoParam) == CY_BLE_SUCCESS)
    {
        /* Wait for disconnection event */
        while(Cy_BLE_GetConnectionState(appConnHandle) == CY_BLE_CONN_STATE_CONNECTED)
        {
            /* Process BLE events */
            Cy_BLE_ProcessEvents();
        }
    }
    /* Stop BLE component. Ignores an error code because current function returns nothing */
    (void) Cy_BLE_Disable();
}


/*******************************************************************************
* Function Name: CyBtldrCommReset
****************************************************************************//**
*
* Resets DFU state for BLE communication.
*
*******************************************************************************/
void CyBLE_CyBtldrCommReset(void)
{
    cyBle_btsDataPacketIndex  = 0U;
}

/*******************************************************************************
* Function Name: CyBLE_CyBtldrCommWrite
****************************************************************************//**
*
* Requests that the provided size (number of bytes) should be written from the
* input data buffer to the host device. This function in turn invokes the
* CyBle_GattsNotification() API to sent the data. If a notification is
* accepted, the function returns CYRET_SUCCESS. The timeOut parameter is ignored
* in this case.
*
* \param data  The pointer to the buffer containing data to be written.
* \param size  The number of bytes from the data buffer to write.
* \param count The pointer to where the BLE component will write the number
*        of written bytes, generally the same as the size.
* \param timeOut Ignored. Used for consistency.
*
* \return
* The return value is of type \ref cy_en_dfu_status_t:
* - CY_DFU_SUCCESS       - Indicates if a notification is successful.
* - CY_DFU_ERROR_UNKNOWN - Failed to send notification to the host.
*
*******************************************************************************/
cy_en_dfu_status_t CyBLE_CyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t retStatus = CY_DFU_ERROR_UNKNOWN;

    if (timeout == 0U)
    {
        /* empty */
    }

    if(Cy_BLE_BTSS_SendNotification(appConnHandle, CY_BLE_BTS_BT_SERVICE, size, (const uint8 *)pData) == CY_BLE_SUCCESS)
    {
        *count = size;
        retStatus = CY_DFU_SUCCESS;
    }
    else
    {
        *count = 0U;
    }
    return (retStatus);
}

/*******************************************************************************
* Function Name: CyBLE_CyBtldrCommRead
****************************************************************************//**
*
* Requests that the provided size (number of bytes) is read from the host device
* and stored in the provided data buffer. Once the read is done, the "count" is
* endorsed with the number of bytes written. The timeOut parameter is used to
* provide an upper bound on the time that the function is allowed to operate. If
* the read completes early, it should return success code as soon as possible.
* If the read was not successful before the allocated time has expired, it
* should return an error.
*
* \param data  The pointer to the buffer to store data from the host controller.
* \param size  The number of bytes to read into the data buffer.
* \param count The pointer to where the BLE component will write the number of
*              read bytes.
* \param timeout The amount of time (in milliseconds) for which the
*                BLE component should wait before indicating communication
*                time out.
*
* \return
* The return value is of type \ref cy_en_dfu_status_t:
* - CY_DFU_SUCCESS       - A command was successfully read.
* - CY_DFU_ERROR_DATA    - The size of the command exceeds the buffer.
* - CY_DFU_ERROR_TIMEOUT - The host controller did not respond during
    specified time out.
* \sideeffect
* \ref CyBle_ProcessEvents() is called as a part of this function.
*
*******************************************************************************/
cy_en_dfu_status_t CyBLE_CyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t retStatus = CY_DFU_ERROR_UNKNOWN;

    if ((pData != NULL) && (size > 0U))
    {
        retStatus = CY_DFU_ERROR_TIMEOUT;

        while(timeout != 0U)
        {
            /* Process BLE events */
            Cy_BLE_ProcessEvents();

            if(cyBle_cmdReceivedFlag == 1U)
            {
                /* Clear command receive flag */
                cyBle_cmdReceivedFlag = 0U;

                if(cyBle_cmdLength < size)
                {
                    (void) memcpy((void *) pData, (const void *) cyBle_btsBuffPtr, (uint32_t)cyBle_cmdLength);

                    /* Return actual received command length */
                    *count = cyBle_cmdLength;

                    retStatus = CY_DFU_SUCCESS;
                }
                else
                {
                    pData = NULL;
                    *count = 0U;
                    retStatus = CY_DFU_ERROR_DATA;
                }
                break;
            }
            /* Wait 1 ms and update timeout counter */
            Cy_SysLib_Delay(1U);
            --timeout;
        }

        /* Process BLE events */
        Cy_BLE_ProcessEvents();
    }
    return (retStatus);
}

/*******************************************************************************
* Function Name: DFUCallBack
****************************************************************************//**
*
* Handles the events from the BLE stack for the DFU Service.
*
* \param eventCode  Event code
* \param eventParam Event parameters
*
*******************************************************************************/
void DFUCallBack(uint32 event, void* eventParam)
{
    /* To remove incorrect compiler warning */
    (void)eventParam;

    static uint16_t cyBle_btsDataPacketSize = 0U;
    static uint8_t  cyBle_btsDataBuffer[CY_FLASH_SIZEOF_ROW + CYBLE_BTS_COMMAND_CONTROL_BYTES_NUM];

    switch ((cy_en_ble_evt_t)event)
    {
    case CY_BLE_EVT_BTSS_NOTIFICATION_ENABLED:
        break;

    case CY_BLE_EVT_BTSS_NOTIFICATION_DISABLED:
        break;

    case CY_BLE_EVT_BTSS_EXEC_WRITE_REQ:
        /* Check the execWriteFlag before execute or cancel write long operation */
        if(((cy_stc_ble_gatts_exec_write_req_t *)eventParam)->execWriteFlag == CY_BLE_GATT_EXECUTE_WRITE_EXEC_FLAG)
        {
            cyBle_btsBuffPtr = ((cy_stc_ble_gatts_exec_write_req_t *)eventParam)->baseAddr[0U].handleValuePair.value.val;

            /* Extract length of command data and add control bytes to data
            * length to get command length.
            */
            cyBle_cmdLength = (((uint16)(((uint16) cyBle_btsBuffPtr[CYBLE_BTS_COMMAND_DATA_LEN_OFFSET + 1U]) << 8U)) |
                                (uint16) cyBle_btsBuffPtr[CYBLE_BTS_COMMAND_DATA_LEN_OFFSET]) +
                                CYBLE_BTS_COMMAND_CONTROL_BYTES_NUM;

            if(cyBle_cmdLength > CYBLE_BTS_COMMAND_MAX_LENGTH)
            {
                cyBle_cmdLength = CYBLE_BTS_COMMAND_MAX_LENGTH;
            }

            /* Set flag for DFU to know that command is received from host */
            cyBle_cmdReceivedFlag = 1U;
        }
        break;

    case CY_BLE_EVT_BTSS_PREP_WRITE_REQ:
        if(((cy_stc_ble_gatts_prep_write_req_param_t *)eventParam)->currentPrepWriteReqCount == 1U)
        {
            /* Send Prepare Write Response which identifies acknowledgment for
            *  long characteristic value write.
            */
            cyBle_cmdLength = 0U;
        }
        break;

    case CY_BLE_EVT_BTSS_WRITE_CMD_REQ:
    {
        uint8 *localDataBuffer = ((cy_stc_ble_bts_char_value_t *)eventParam)->value->val;

        /* This is the beginning of the packet, let's read the size now */
        if(cyBle_btsDataPacketIndex == 0U)
        {
            cyBle_btsDataPacketSize = (((uint16)(((uint16) localDataBuffer[CYBLE_BTS_COMMAND_DATA_LEN_OFFSET + 1U]) << 8U)) |
                               (uint16) localDataBuffer[CYBLE_BTS_COMMAND_DATA_LEN_OFFSET]) +
                                CYBLE_BTS_COMMAND_CONTROL_BYTES_NUM;

        }

        (void) memcpy(&cyBle_btsDataBuffer[cyBle_btsDataPacketIndex], localDataBuffer, (uint32_t) ((cy_stc_ble_bts_char_value_t *)eventParam)->value->len);

        cyBle_btsDataPacketIndex += ((cy_stc_ble_bts_char_value_t *)eventParam)->value->len;

        if(cyBle_btsDataPacketIndex == cyBle_btsDataPacketSize)
        {
            cyBle_btsBuffPtr         = &cyBle_btsDataBuffer[0];
            cyBle_cmdLength          = cyBle_btsDataPacketSize;
            cyBle_cmdReceivedFlag    = 1U;
            cyBle_btsDataPacketIndex = 0U;
        }
        break;
    }

    case CY_BLE_EVT_BTSS_WRITE_REQ:
        cyBle_btsBuffPtr =
               CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_PTR(cy_ble_btssConfigPtr->attrInfo->btServiceInfo[0U].btServiceCharHandle);

        /* Extract length of command data and add control bytes to data
        * length to get command length.
        */
        cyBle_cmdLength = (((uint16)(((uint16) cyBle_btsBuffPtr[CYBLE_BTS_COMMAND_DATA_LEN_OFFSET + 1U]) << 8U)) |
                            (uint16) cyBle_btsBuffPtr[CYBLE_BTS_COMMAND_DATA_LEN_OFFSET]) +
                            CYBLE_BTS_COMMAND_CONTROL_BYTES_NUM;

        /* Set flag for DFU to know that command is received from host */
        cyBle_cmdReceivedFlag = 1U;
        break;

    default:
        /* Unsupported event */
        break;
    }
}


#ifndef CY_DFU_BLE_TRANSPORT_DISABLE

/*******************************************************************************
* Function Name: Cy_DFU_TransportRead
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_TransportRead(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    return (CyBLE_CyBtldrCommRead(buffer, size, count, timeout));
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportWrite
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
cy_en_dfu_status_t Cy_DFU_TransportWrite(uint8_t buffer[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    return (CyBLE_CyBtldrCommWrite(buffer, size, count, timeout));
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportReset
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportReset(void)
{
    CyBLE_CyBtldrCommReset();
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportStart
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportStart(void)
{
    CyBLE_CyBtldrCommStart();
}


/*******************************************************************************
* Function Name: Cy_DFU_TransportStop
****************************************************************************//**
*
* This function documentation is part of the DFU SDK API, see the
* cy_dfu.h file or DFU SDK API Reference Manual for details.
*
*******************************************************************************/
void Cy_DFU_TransportStop(void)
{
    CyBLE_CyBtldrCommStop();
}

#endif /* CY_DFU_BLE_TRANSPORT_DISABLE */

#endif /* CY_BLE_HOST_CORE */


/* [] END OF FILE */
