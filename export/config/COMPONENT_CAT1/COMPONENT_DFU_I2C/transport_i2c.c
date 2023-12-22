/***************************************************************************//**
* \file transport_i2c.c
* \version 5.1
*
* This file provides the source code of the DFU communication APIs
* for the I2C driver from HAL.
*
********************************************************************************
* \copyright
* (c) (2016-2023), Cypress Semiconductor Corporation (an Infineon company) or
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
#include "cyhal_system.h"
#include "cyhal_i2c.h"
#include "cycfg_pins.h"
#include "transport_i2c.h"


/*******************************************************************************
* User configuration of I2C device
*******************************************************************************/
/* Bus speed, 100kHz */
#ifndef DFU_I2C_SPEED
    #define DFU_I2C_SPEED               (100000U)
#endif
/* Bus address for slave */
#ifndef DFU_I2C_ADDR
    #define DFU_I2C_ADDR                (0x0CU)
#endif
/* Pins aliases */
#ifndef DFU_I2C_SDA
    #define DFU_I2C_SDA                 CYBSP_I2C_SDA
#endif
#ifndef DFU_I2C_SCL
    #define DFU_I2C_SCL                 CYBSP_I2C_SCL
#endif

/* Size of Read/Write buffers for I2C DFU  */
#ifndef DFU_I2C_TX_BUFFER_SIZE
    #define DFU_I2C_TX_BUFFER_SIZE      (64U)
#endif
#ifndef DFU_I2C_RX_BUFFER_SIZE
    #define DFU_I2C_RX_BUFFER_SIZE      (64U)
#endif

/** Interrupt priority. Check device TRM for valid range e.g. Cortex-M4 0 to 7. */
#ifndef DFU_I2C_IRQ_PRIORITY
    #define DFU_I2C_IRQ_PRIORITY        (7U)
#endif

/**
* I2C_initVar indicates whether the I2C driver has been initialized. The
* variable is initialized to false and set to true the first time
* \ref I2C_I2cCyBtldrCommStart is called. This allows  the driver to restart
* without re-initialization after the first call to the
* \ref I2C_I2cCyBtldrCommStart routine.
* For re-initialization set \ref I2C_initVar to false and call
* \ref I2C_I2cCyBtldrCommStart.
*/
bool I2C_initVar = false;

/* Global I2C object */
static cyhal_i2c_t i2c_slave_obj;

/* Writes to this buffer */
static uint8_t I2C_slaveTxBuf[DFU_I2C_TX_BUFFER_SIZE];

/* Reads from this buffer */
static uint8_t I2C_slaveRxBuf[DFU_I2C_RX_BUFFER_SIZE];

/* Flag to release buffer to be read */
static uint32_t I2C_applyBuffer;


/*******************************************************************************
* Internal function declarations
*******************************************************************************/
static void I2C_Start(void);
static void cyhal_i2c_event_callback(void* callback_arg, cyhal_i2c_event_t event);


/*******************************************************************************
* Function Name: I2C_Start
****************************************************************************//**
*
* Starts SCB I2C operation. Setup interrupt.
*
* \globalvars
* \ref I2C_initVar - used to check initial configuration, modified on first
*                    function call.
*
*******************************************************************************/
static void I2C_Start(void)
{
    if (!I2C_initVar)
    {
        cy_rslt_t rslt;

        /* I2C configuration structure (set of values from defines) */
        cyhal_i2c_cfg_t i2c_slave_config =
        {
            CYHAL_I2C_MODE_SLAVE,   /* .is_slave = true */
            DFU_I2C_ADDR,           /* .address = 0x0CU */
            DFU_I2C_SPEED           /* .frequencyhal_hz = 100000U */
        };

        /* Initialize the I2C block */
        rslt = cyhal_i2c_init(&i2c_slave_obj, DFU_I2C_SDA, DFU_I2C_SCL, NULL);
        /* A I2C initialization error - stops the execution */
        CY_ASSERT(CY_RSLT_SUCCESS == rslt);

        /* Configure the I2C interface in slave mode */
        rslt = cyhal_i2c_configure(&i2c_slave_obj, &i2c_slave_config);
        /* A I2C configuration error - stops the execution */
        CY_ASSERT(CY_RSLT_SUCCESS == rslt);

        (void) rslt; /* Avoid warning for release mode */

        /* The transport is configured */
        I2C_initVar = true;
    }
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
    I2C_Start();

    /* Register I2C slave event callback */
    cyhal_i2c_register_callback(&i2c_slave_obj, (cyhal_i2c_event_callback_t)cyhal_i2c_event_callback, NULL);

    /* Enable I2C events for slave */
    cyhal_i2c_enable_event(&i2c_slave_obj, (cyhal_i2c_event_t)
                           (CYHAL_I2C_SLAVE_WRITE_EVENT | CYHAL_I2C_SLAVE_READ_EVENT | CYHAL_I2C_SLAVE_WR_CMPLT_EVENT),
                            DFU_I2C_IRQ_PRIORITY,
                            true);

    /* Manage I2C Rx/Tx buffers */
    (void)cyhal_i2c_slave_config_read_buffer(&i2c_slave_obj, I2C_slaveTxBuf, DFU_I2C_TX_BUFFER_SIZE);
    (void)cyhal_i2c_slave_config_write_buffer(&i2c_slave_obj, I2C_slaveRxBuf, DFU_I2C_RX_BUFFER_SIZE);

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
    cyhal_i2c_free(&i2c_slave_obj);
    I2C_initVar=false;
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
    (void)cyhal_i2c_slave_config_read_buffer(&i2c_slave_obj, I2C_slaveTxBuf, DFU_I2C_TX_BUFFER_SIZE);
    (void)cyhal_i2c_slave_config_write_buffer(&i2c_slave_obj, I2C_slaveRxBuf, DFU_I2C_RX_BUFFER_SIZE);

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

        if (CY_RSLT_SUCCESS == cyhal_i2c_slave_read(&i2c_slave_obj, pData, &dataSize, timeout))
        {
            status = CY_DFU_ERROR_UNKNOWN;
            *count = dataSize;

            /* Prepare the slave buffer for next reception */
            if (CY_RSLT_SUCCESS == cyhal_i2c_slave_config_write_buffer(&i2c_slave_obj, I2C_slaveRxBuf, DFU_I2C_RX_BUFFER_SIZE))
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
        if (CY_RSLT_SUCCESS == cyhal_i2c_slave_write(&i2c_slave_obj, pData, &dataSize, 0U))
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
* Function Name: cyhal_i2c_event_callback
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
static void cyhal_i2c_event_callback(void* callback_arg, cyhal_i2c_event_t event)
{
    /* To remove unused variable warning */
    (void)callback_arg;

    if ((CYHAL_I2C_SLAVE_READ_EVENT == event) && (0U != I2C_applyBuffer))
    {
        /* Address phase, host reads: release read buffer */
        (void)cyhal_i2c_slave_config_read_buffer(&i2c_slave_obj, I2C_slaveTxBuf, I2C_applyBuffer);
        I2C_applyBuffer = 0U;
    }
    else if (CYHAL_I2C_SLAVE_WRITE_EVENT == event)
    {
        /* Address phase, host writes: make read buffer empty so that host will
         * receive only 0xFF (CY_SCB_I2C_DEFAULT_TX) until the DFU
         * application has a valid response packet.
         */
        (void)cyhal_i2c_slave_abort_read(&i2c_slave_obj);
    }
    else
    {
        /* No action */
    }
}

/* [] END OF FILE */
