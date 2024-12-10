/***************************************************************************//**
* \file transport_i2c.c
* \version 6.0
*
* This file provides the source code of the DFU communication APIs
* for the I2C driver from HAL.
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


/*******************************************************************************
* Function Name: I2C_I2cCyBtldrCommStop
****************************************************************************//**
*
*  Stops the I2C transport.
*
*******************************************************************************/
void I2C_I2cCyBtldrCommStop(void)
{
    i2c_callback(CY_DFU_TRANSPORT_I2C_DEINIT);
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
