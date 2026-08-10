/***************************************************************************//**
* \file transport_pmbus.c
*
* This file provides the source code of the DFU communication APIs
* for the PMBUS driver from HAL.
*
********************************************************************************
* \copyright
* (c) (2016-2026), Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
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

#include <string.h>
#include "transport_pmbus.h"


/*******************************************************************************
* Internal variables
*******************************************************************************/
/* The pointer to the PMBUS object */
static mtb_pmbus_stc_t *pmbusObj;

/* The PMBUS command used for DFU */
static uint32_t pmbusCmdCode;
/* The pointer to the PMBUS command data storage */
static uint8_t *pmbusCmdData;
/* The PMBUS command data size */
static uint8_t pmbusCmdDataSize;

/* Data ready from Master */
static volatile uint8_t pmbusMasterDataReady;
/* Data ready from Slave */
static volatile uint8_t pmbusSlaveDataReady;


/*******************************************************************************
* Function Name: Cy_DFU_TransportPMBusConfig
****************************************************************************//**
*
* Configure DFU PMBUS Transport
*
* Call this function in the user application to provide the HAL object and callback
* function to DFU transport.
*
* \param config Configuration structure
*
*******************************************************************************/
void Cy_DFU_TransportPMBusConfig(cy_stc_dfu_transport_pmbus_cfg_t * config)
{
    CY_ASSERT(NULL != config);
    CY_ASSERT(NULL != config->pmbus);
    CY_ASSERT(NULL != config->cmdData);

    pmbusObj = config->pmbus;
    pmbusCmdCode = config->cmdCode;
    pmbusCmdData = config->cmdData;
}


/*******************************************************************************
* Function Name: PMBUS_CyBtldrCommStart
****************************************************************************//**
*
* Starts the PMBUS transport.
*
*******************************************************************************/
void PMBUS_CyBtldrCommStart(void)
{
    if (NULL != pmbusObj)
    {
        (void)mtb_pmbus_enable(pmbusObj);
    }
}


/*******************************************************************************
* Function Name: PMBUS_CyBtldrCommStop
****************************************************************************//**
*
*  Stops the PMBUS transport.
*
*******************************************************************************/
void PMBUS_CyBtldrCommStop(void)
{
    if (NULL != pmbusObj)
    {
        (void)mtb_pmbus_disable(pmbusObj);
    }
}


/*******************************************************************************
* Function Name: PMBUS_CyBtldrCommReset
****************************************************************************//**
*
*  Resets the receive and transmit communication flags.
*
*******************************************************************************/
void PMBUS_CyBtldrCommReset(void)
{
    if (NULL != pmbusObj)
    {
        (void)mtb_pmbus_disable(pmbusObj);
        (void)mtb_pmbus_enable(pmbusObj);
    }
    
    pmbusCmdDataSize = 0U;
    pmbusMasterDataReady = 0U;
    pmbusSlaveDataReady = 0U;
}


/*******************************************************************************
* Function Name: dfu_pmbus_cmd_callback
****************************************************************************//**
*
*  The callback function called in the PMBus ISR to notify the user
*  about occurrences of command specific events.
*
*  \param event: The PMBus command event.
*  \param page: The PMBus page number to update.
*  \param phase: The PMBus phase number to update.
*  \param byte The current byte in the exchange.
*
*  \return
*   Value is not used in the DFU.
*
*******************************************************************************/
bool dfu_pmbus_cmd_callback(mtb_pmbus_cmd_events_t event, int32_t page, int32_t phase, uint8_t byte)
{
    /* Avoid compiler warnings */
    (void) byte;

    if ((NULL != pmbusObj) && (NULL != pmbusCmdData))
    {
        if (event == MTB_PMBUS_CMD_WRITE_DONE)
        {
            mtb_pmbus_cmd_get_transfer_size_isr(pmbusObj, pmbusCmdCode, &pmbusCmdDataSize);
            if (pmbusCmdDataSize > 0U)
            {
                mtb_pmbus_cmd_read_data_ext_isr(pmbusObj, pmbusCmdCode, page, phase, pmbusCmdData, pmbusCmdDataSize);
                pmbusMasterDataReady = 1U;

                /* Block the Master from reading data while the Slave response is on the go */
                mtb_pmbus_cmd_enable_disable_isr(pmbusObj, pmbusCmdCode, false);
            }
        }

        if (event == MTB_PMBUS_CMD_READ_REQ)
        {
            if (pmbusSlaveDataReady && (pmbusCmdDataSize > 0U))
            {
                mtb_pmbus_cmd_update_data_ext_isr(pmbusObj, pmbusCmdCode, page, phase, pmbusCmdData, pmbusCmdDataSize);
                pmbusSlaveDataReady = 0U;
             }
        }
    }

    /* Ignored */
    return true;
}

/*******************************************************************************
* Function Name: PMBUS_CyBtldrCommRead
****************************************************************************//**
*
*  Allows the caller to read data from the DFU host (the host writes the
*  data). The function handles polling to allow a block of data to be completely
*  received from the host device.
*
*  \param pData: Pointer to storage for the block of data to be read from the
*   DFU host
*  \param size: Number of bytes to be read.
*  \param count: Pointer to the variable to write the number of bytes actually
*   read.
*  \param timeOut The amount of time (in milliseconds) for which the
*                function should wait before indicating communication
*                time out.
*
*  \return
*   Returns CYRET_SUCCESS if no problem was encountered or returns the value
*   that best describes the problem. For more information refer to the
*   "Return Codes" section of the System Reference Guide.
*
*******************************************************************************/
cy_en_dfu_status_t PMBUS_CyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeOut)
{
    /* Avoid compiler warnings */
    (void)timeOut;

    cy_en_dfu_status_t status = CY_DFU_ERROR_BAD_PARAM;

    if ((NULL != pData) && (size > 0U) && (NULL != count) && (NULL != pmbusCmdData))
    {
        status = CY_DFU_ERROR_TIMEOUT;

        if (pmbusMasterDataReady)
        {
            /* Copy the Master data from the PMBus data storage for further Slave handling */
            (void)memcpy(pData, pmbusCmdData, pmbusCmdDataSize);
            *count = pmbusCmdDataSize;
            pmbusMasterDataReady = 0U;

            status = CY_DFU_SUCCESS;
        }
    }

    return (status);
}


/*******************************************************************************
* Function Name: PMBUS_CyBtldrCommWrite
****************************************************************************//**
*
*  Allows the caller to write data to the DFU host (the host reads the
*  data). The function does not use timeout and returns after data has been
*  copied into the transmit buffer. The data transmission starts immediately
*  after the first data element is written into the buffer and lasts until all
*  data elements from the buffer are sent.
*
*  \param pData: Pointer to the block of data to be written to the DFU
*   host.
*  \param size: Number of bytes to be written.
*  \param count: Pointer to the variable to write the number of bytes actually
*   written.
*  \param timeOut: The timeout is not used by this function.
*   The function returns as soon as data is copied into the transmit buffer.
*
*  \return
*   Returns CYRET_SUCCESS if no problem was encountered or returns the value
*   that best describes the problem. For more information refer to the
*   "Return Codes" section of the System Reference Guide.
*
*******************************************************************************/
cy_en_dfu_status_t PMBUS_CyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeOut)
{
    /* Avoid compiler warnings */
    (void)timeOut;

    cy_en_dfu_status_t status = CY_DFU_ERROR_BAD_PARAM;

    if ((NULL != pData) && (size > 0U) && (NULL != count) && (NULL != pmbusObj) && (NULL != pmbusCmdData))
    {
        /* Copy the Slave response to the PMBus data storage for further Master reading */
        (void)memcpy(pmbusCmdData, pData, (uint8_t)size);
        pmbusCmdDataSize = size;
        *count = size;
        pmbusSlaveDataReady = 1U;

        /* Enable the Master to read data prepared by the Slave */
        (void)mtb_pmbus_cmd_enable_disable(pmbusObj, pmbusCmdCode, true);

        status = CY_DFU_SUCCESS;
    }

    return (status);
}

/* [] END OF FILE */
