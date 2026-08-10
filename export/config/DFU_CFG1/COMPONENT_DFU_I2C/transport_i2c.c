/***************************************************************************//**
* \file transport_i2c.c
*
* This file provides the source code of the DFU communication APIs
* for the I2C driver from HAL.
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

#include <string.h>
#include "mtb_hal_system.h"
#include "mtb_hal_i2c.h"
#include "transport_i2c.h"


/*******************************************************************************
* User configuration of I2C device
*******************************************************************************/
/* Size of Read/Write buffers for I2C DFU  */
#ifndef DFU_I2C_TX_BUFFER_SIZE
    #define DFU_I2C_TX_BUFFER_SIZE      (128U)
#endif
#ifndef DFU_I2C_RX_BUFFER_SIZE
    #define DFU_I2C_RX_BUFFER_SIZE      (128U)
#endif


/*******************************************************************************
* Internal variables
*******************************************************************************/
/* The pointer to the I2C HAL object */
static mtb_hal_i2c_t *i2c_target_obj;

/* The pointer to the initialization/de-initialization callback function
 * I2C hardware
 */
static Cy_DFU_TransportI2cCallback i2c_callback;

/* Writes to this buffer */
static uint8_t I2C_targetTxBuf[DFU_I2C_TX_BUFFER_SIZE];

/* Reads from this buffer */
static uint8_t I2C_targetRxBuf[DFU_I2C_RX_BUFFER_SIZE];

/* Flag to release buffer to be read */
static uint32_t I2C_applyBuffer;

/*******************************************************************************
* Internal function declarations
*******************************************************************************/
static void mtb_hal_i2c_event_callback(void* callback_arg, mtb_hal_i2c_event_t event);


/*******************************************************************************
* Function Name: Cy_DFU_TransportI2cConfig
****************************************************************************//**
*
* Configure DFU I2C Transport
*
* Call this function in the user application to provide the HAL object and callback
* function to DFU transport.
*
* \param config Configuration structure
*
*******************************************************************************/
void Cy_DFU_TransportI2cConfig(cy_stc_dfu_transport_i2c_cfg_t * config)
{
    CY_ASSERT(NULL != config);
    CY_ASSERT(NULL != config->i2c);
    CY_ASSERT(NULL != config->callback);

    i2c_target_obj = config->i2c;
    i2c_callback = config->callback;
}


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommStart
****************************************************************************//**
*
* Starts the I2C transport.
*
*******************************************************************************/
void I2C_I2cCyBtldrCommStart(void)
{
    CY_ASSERT(NULL != i2c_callback);
    if (NULL != i2c_callback)
    {
        i2c_callback(CY_DFU_TRANSPORT_I2C_INIT);

        /* Register I2C target event callback */
        mtb_hal_i2c_register_callback(i2c_target_obj, (mtb_hal_i2c_event_callback_t)mtb_hal_i2c_event_callback, NULL);

        /* Enable I2C events for target */
        mtb_hal_i2c_enable_event(i2c_target_obj, MTB_HAL_I2C_TARGET_WRITE_EVENT, true);
        mtb_hal_i2c_enable_event(i2c_target_obj, MTB_HAL_I2C_TARGET_READ_EVENT, true);
        mtb_hal_i2c_enable_event(i2c_target_obj, MTB_HAL_I2C_TARGET_WR_CMPLT_EVENT, true);

        /* Manage I2C Rx/Tx buffers */
        (void)mtb_hal_i2c_target_config_write_buffer(i2c_target_obj, I2C_targetTxBuf, DFU_I2C_TX_BUFFER_SIZE);
        (void)mtb_hal_i2c_target_config_read_buffer(i2c_target_obj, I2C_targetRxBuf, DFU_I2C_RX_BUFFER_SIZE);

        I2C_applyBuffer = 0U;
    }
}


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommStop
****************************************************************************//**
*
*  Stops the I2C transport.
*
*******************************************************************************/
void I2C_I2cCyBtldrCommStop(void)
{
    CY_ASSERT(NULL != i2c_callback);
    if (NULL != i2c_callback)
    {
        i2c_callback(CY_DFU_TRANSPORT_I2C_DEINIT);
    }
}


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommReset
****************************************************************************//**
*
*  Resets the receive and transmit communication buffers.
*
*******************************************************************************/
void I2C_I2cCyBtldrCommReset(void)
{
    /* Manage I2C Rx/Tx buffers */
    (void)mtb_hal_i2c_target_config_write_buffer(i2c_target_obj, I2C_targetTxBuf, DFU_I2C_TX_BUFFER_SIZE);
    (void)mtb_hal_i2c_target_config_read_buffer(i2c_target_obj, I2C_targetRxBuf, DFU_I2C_RX_BUFFER_SIZE);

    I2C_applyBuffer = 0U;
}


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommRead
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
*  \param timeout The amount of time (in milliseconds) for which the
*                function should wait before indicating communication
*                time out.
*
*  \return
*   Returns CYRET_SUCCESS if no problem was encountered or returns the value
*   that best describes the problem. For more information refer to the
*   "Return Codes" section of the System Reference Guide.
*
*******************************************************************************/
cy_en_dfu_status_t I2C_I2cCyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_BAD_PARAM;
    uint16_t dataSize;

    if ((pData != NULL) && (size > 0U))
    {
        status = CY_DFU_ERROR_TIMEOUT;
        dataSize = (uint16_t) size;

        if (CY_RSLT_SUCCESS == mtb_hal_i2c_target_read(i2c_target_obj, pData, &dataSize, timeout))
        {
            status = CY_DFU_ERROR_UNKNOWN;
            *count = dataSize;

            /* Prepare the target buffer for next reception */
            if (CY_RSLT_SUCCESS == mtb_hal_i2c_target_config_read_buffer(i2c_target_obj, I2C_targetRxBuf, DFU_I2C_RX_BUFFER_SIZE))
            {
                status = CY_DFU_SUCCESS;
            }
        }
    }

    return (status);
}


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommWrite
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
cy_en_dfu_status_t I2C_I2cCyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeOut)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_BAD_PARAM;
    (void)timeOut;

    uint16_t dataSize;

    if ((NULL != pData) && (size > 0U))
    {
        dataSize = (uint16_t) size;

        /* Copy response into read buffer */
        if (CY_RSLT_SUCCESS == mtb_hal_i2c_target_write(i2c_target_obj, pData, &dataSize, 0U))
        {
            /* Read buffer is ready to be released to host */
            *count = dataSize;
            I2C_applyBuffer = (uint32_t) count;

            status = CY_DFU_SUCCESS;
        }
    }

    return (status);
}


/*******************************************************************************
* Function Name: mtb_hal_i2c_event_callback
****************************************************************************//**
*
*  Releases the read buffer to be read when a response is copied to the buffer
*  and a new read transaction starts.
*  Closes the read buffer when write transaction is started.
*
* \globalvars
*  I2C_applyBuffer - the flag to release the buffer with a response
*  to be read by the host.
*
*******************************************************************************/
static void mtb_hal_i2c_event_callback(void* callback_arg, mtb_hal_i2c_event_t event)
{
    /* To remove unused variable warning */
    (void)callback_arg;

    if ((MTB_HAL_I2C_TARGET_READ_EVENT == event) && (0U != I2C_applyBuffer))
    {
        /* Address phase, host reads: release write buffer */
        (void)mtb_hal_i2c_target_config_write_buffer(i2c_target_obj, I2C_targetTxBuf, (uint16_t) I2C_applyBuffer);
        I2C_applyBuffer = 0U;
    }
    else if (MTB_HAL_I2C_TARGET_WRITE_EVENT == event)
    {
        /* Address phase, host writes: make read buffer empty so that host will
         * receive only 0xFF (CY_SCB_I2C_DEFAULT_TX) until the DFU
         * application has a valid response packet.
         */
        (void)mtb_hal_i2c_target_abort_read(i2c_target_obj);
    }
    else
    {
        /* No action */
    }
}

/* [] END OF FILE */
