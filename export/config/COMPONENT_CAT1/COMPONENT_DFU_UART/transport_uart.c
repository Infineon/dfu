/***************************************************************************//**
* \file transport_uart.c
* \version 5.1
*
* This file provides the source code of the DFU communication APIs
* for the UART driver from HAL.
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

#include "transport_uart.h"
#include "cyhal_system.h"
#include "cyhal_uart.h"
#include "cycfg_pins.h"

/*
* USER CONFIGURABLE: Byte to byte time interval: calculated basing on current
* baud rate configuration.
* Set it to approximately to (50e6 / baud_rate) value in microseconds.
* E.g. baud_rate = 115200, UART_BYTE_TO_BYTE_TIMEOUT_US ~ 434
*/
#ifndef DFU_UART_BAUD
    #define DFU_UART_BAUD                       CYHAL_UART_DEFAULT_BAUD
#endif
#ifndef UART_BYTE_TO_BYTE_TIMEOUT_US
    #define UART_BYTE_TO_BYTE_TIMEOUT_US        (868U)
#endif
#ifndef DFU_UART_TX
    #define DFU_UART_TX                         CYBSP_DEBUG_UART_TX
#endif
#ifndef DFU_UART_RX
    #define DFU_UART_RX                         CYBSP_DEBUG_UART_RX
#endif
#ifndef DFU_UART_PARITY
    #define DFU_UART_PARITY                     CYHAL_UART_PARITY_NONE
#endif
#ifndef DFU_UART_DATA_BITS
    #define DFU_UART_DATA_BITS                  (8U)
#endif
#ifndef DFU_UART_STOP_BITS
    #define DFU_UART_STOP_BITS                  (1U)
#endif


/**
* UART_initVar indicates whether the UART driver has been initialized. The
* variable is initialized to false and set to true the first time
* \ref UART_UartCyBtldrCommStart is called. This allows  the driver to restart
* without re-initialization after the first call to the
* \ref UART_UartCyBtldrCommStart routine.
* For re-initialization set \ref UART_initVar to false and call
* \ref UART_UartCyBtldrCommStart.
*/
bool UART_initVar = false;

/* Global uart object */
static cyhal_uart_t uart_obj;


/* Returns a number of bytes to copy into a DFU buffer */
#define UART_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32_t)(actBufSize) < (uint32_t)(bufSize)) ? \
                                ((uint32_t) (actBufSize)) : ((uint32_t) (bufSize)) )


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommStart
****************************************************************************//**
*
* Starts the UART transport.
*
*******************************************************************************/
void UART_UartCyBtldrCommStart(void)
{
    if (!UART_initVar)
    {
        cy_rslt_t rslt;

        const cyhal_uart_cfg_t uart_config =
        {
            .data_bits      = DFU_UART_DATA_BITS,
            .stop_bits      = DFU_UART_STOP_BITS,
            .parity         = DFU_UART_PARITY,
            .rx_buffer      = NULL,
            .rx_buffer_size = 0U
        };

        /* Initialize UART */
        rslt = cyhal_uart_init(&uart_obj, DFU_UART_TX, DFU_UART_RX, NC, NC,
                             NULL, &uart_config);
        /* A UART initialization error - stops the execution */
        CY_ASSERT(CY_RSLT_SUCCESS == rslt);

    #if (DFU_UART_BAUD != CYHAL_UART_DEFAULT_BAUD)
        rslt = cyhal_uart_set_baud(&uart_obj, DFU_UART_BAUD, NULL);
        CY_ASSERT(CY_RSLT_SUCCESS == rslt);
    #endif

        (void) rslt; /* Avoid warning for release mode */

        /* The transport is configured */
        UART_initVar = true;
    }
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommStop
****************************************************************************//**
*
* Stops the UART transport.
*
*******************************************************************************/
void UART_UartCyBtldrCommStop(void)
{
    cyhal_uart_free(&uart_obj);
    UART_initVar=false;
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommReset
****************************************************************************//**
*
* Abort the ongoing TX/RX transactions.
*
*******************************************************************************/
void UART_UartCyBtldrCommReset(void)
{
    (void)cyhal_uart_write_abort(&uart_obj);
    (void)cyhal_uart_read_abort(&uart_obj);
    (void)cyhal_uart_clear(&uart_obj);
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommRead
****************************************************************************//**
*
* Allows the caller to read data from the DFU host (the host writes the
* data). The function handles polling to allow a block of data to be completely
* received from the host device.
*
* \param pData   Pointer to a buffer to store received command.
* \param size    Number of bytes to be read.
* \param count   Pointer to the variable that contains number of bytes that were
*                received.
* \param timeout Time to wait before the function returns because of timeout,
*                in milliseconds.
*
* \return
* The status of the operation:
* - \ref CY_DFU_SUCCESS if successful.
* - \ref CY_DFU_ERROR_TIMEOUT if no data has been received.
* - See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
cy_en_dfu_status_t UART_UartCyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status;
    size_t byteCount;

    status = CY_DFU_ERROR_UNKNOWN;

    if ((pData != NULL) && (size > 0U))
    {
        status = CY_DFU_ERROR_TIMEOUT;

        /* Wait with timeout 1 ms for packet end */
        byteCount = 0U;
        do
        {
            /* Check packet start */
            if (cyhal_uart_readable(&uart_obj) != 0U)
            {
                /* Wait for end of packet */
                do
                {
                    byteCount = cyhal_uart_readable(&uart_obj);
                    cyhal_system_delay_us(UART_BYTE_TO_BYTE_TIMEOUT_US);
                }
                while (byteCount != cyhal_uart_readable(&uart_obj));

                byteCount = UART_BYTES_TO_COPY(byteCount, size);
                *count = byteCount;
                status = CY_DFU_SUCCESS;

                break;
            }

            Cy_SysLib_Delay(1U);
            --timeout;
        }
        while (timeout != 0U);

        if (status == CY_DFU_SUCCESS)
        {
            /* Get data from RX buffer into DFU buffer */
            cy_rslt_t rslt = cyhal_uart_read(&uart_obj, (void*)pData, &byteCount);
            status = (rslt == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_UNKNOWN;
        }
    }

    return (status);
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommWrite
****************************************************************************//**
*
* Allows the caller to write data to the DFU host (the host reads the
* data). The function does not use timeout and returns after data has been
* copied into the transmit buffer. The data transmission starts immediately
* after the first data element is written into the buffer and lasts until all
* data elements from the buffer are sent.
*
* \param pData     Pointer to the block of data to be written to the DFU
*                  host.
* \param size      Number of bytes to be written.
* \param count     Pointer to the variable to write the number of bytes
*                  actually written.
* \param timeOut   The time out is not used by this function.
*                  The function returns as soon as data is copied into the
*                  transmit buffer.
* \return
* The status of the operation:
* - \ref CY_DFU_SUCCESS if successful.
* - See \ref cy_en_dfu_status_t.
*
*******************************************************************************/
cy_en_dfu_status_t UART_UartCyBtldrCommWrite(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t status;

    status = CY_DFU_ERROR_UNKNOWN;

    if ((pData != NULL) && (size > 0U))
    {
        /* Transmit data. This function does not wait until data is sent. */
        size_t byteCount = size;
        cy_rslt_t rslt = cyhal_uart_write(&uart_obj, (void*)pData, &byteCount);
        status = (rslt == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_UNKNOWN;

        *count = size;

        if (timeout != 0U)
        {
            /* empty */
        }
    }

    return (status);
}


/* [] END OF FILE */
