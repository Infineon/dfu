/***************************************************************************//**
* \file transport_uart.c
*
* This file provides the source code of the DFU communication APIs
* for the UART driver from HAL.
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

#include "transport_uart.h"
#include "mtb_hal_system.h"
#include "mtb_hal_uart.h"
#include "cy_dfu_logging.h"

#ifndef UART_BYTE_TO_BYTE_TIMEOUT_US
    #define UART_BYTE_TO_BYTE_TIMEOUT_US            (868U)
#endif

/* Returns a number of bytes to copy into a DFU buffer */
#define UART_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32_t)(actBufSize) < (uint32_t)(bufSize)) ? \
                                ((uint32_t) (actBufSize)) : ((uint32_t) (bufSize)) )

/*******************************************************************************
* Internal variable
*******************************************************************************/
/* The pointer to the UART HAL object */
static mtb_hal_uart_t *uart_obj;
/* The pointer to initialize/de-initialize the callback function for
 * UART hardware.
 */
static Cy_DFU_TransportUartCallback uart_callback;


/*******************************************************************************
* Function Name: Cy_DFU_TransportUartConfig
****************************************************************************//**
*
* Configure DFU UART Transport
*
* Call this function in the user application to provide the HAL object and callback
* function to DFU transport.
*
* \param config Configuration structure
*
*******************************************************************************/
void Cy_DFU_TransportUartConfig(cy_stc_dfu_transport_uart_cfg_t * config)
{
    CY_ASSERT(NULL != config);
    CY_ASSERT(NULL != config->uart);
    CY_ASSERT(NULL != config->callback);

    uart_obj = config->uart;
    uart_callback = config->callback;
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommStart
****************************************************************************//**
*
* Starts the UART transport.
*
*******************************************************************************/
void UART_UartCyBtldrCommStart(void)
{
    CY_ASSERT(NULL != uart_callback);
    if (NULL != uart_callback)
    {
        uart_callback(CY_DFU_TRANSPORT_UART_INIT);
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
    CY_ASSERT(NULL != uart_callback);
    if (NULL != uart_callback)
    {
        uart_callback(CY_DFU_TRANSPORT_UART_DEINIT);
    }
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
    (void)mtb_hal_uart_clear(uart_obj);
}

#define UART_SCB_HW_BUF_SIZE        (96U)

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
    cy_rslt_t statusHal;

    status = CY_DFU_ERROR_UNKNOWN;

    if ((pData != NULL) && (size > 0U))
    {
        status = CY_DFU_ERROR_TIMEOUT;

        /* Wait with timeout 1 ms for packet end */
        byteCount = 0U;
        do
        {
            /* Check packet start */
            if (mtb_hal_uart_readable(uart_obj) != 0U)
            {
                uint32_t readBytes = 0;
                /* Wait for end of packet */
                do
                {
                    byteCount = mtb_hal_uart_readable(uart_obj);
                    if(byteCount >= UART_SCB_HW_BUF_SIZE)
                    {
                        /* prevent write out of buffer */
                        if((readBytes + byteCount) >= size)
                        {
                            status = CY_DFU_ERROR_LENGTH;
                            CY_DFU_LOG_ERR("UART: read more data %d than size of buffer %d",
                                                (unsigned int)(readBytes + byteCount), (unsigned int)size);
                        }
                        else
                        {
                            cy_rslt_t rslt = mtb_hal_uart_read(uart_obj, (void*)pData, &byteCount);
                            status = (rslt == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_UNKNOWN;
                        }
                        if((status == CY_DFU_ERROR_UNKNOWN) || (status == CY_DFU_ERROR_LENGTH))
                        {
                            break;
                        }

                        readBytes = readBytes + byteCount;
                        pData=&pData[byteCount];
                        byteCount=0U;
                    }
                    mtb_hal_system_delay_us(UART_BYTE_TO_BYTE_TIMEOUT_US);
                }
                while (byteCount != mtb_hal_uart_readable(uart_obj));

                byteCount = UART_BYTES_TO_COPY(byteCount, size);
                *count = byteCount + readBytes;
                if (status != CY_DFU_ERROR_LENGTH)
                {
                    status = CY_DFU_SUCCESS;
                }

                break;
            }

            statusHal = mtb_hal_system_delay_ms(1U);
            CY_ASSERT(CY_RSLT_SUCCESS == statusHal);
            /* To avoid the compiler warning in Release mode */
            (void) statusHal;
            --timeout;
        }
        while (timeout != 0U);

        if ((status == CY_DFU_SUCCESS) && (byteCount > 0U))
        {
            /* Get data from RX buffer into DFU buffer */
            cy_rslt_t rslt = mtb_hal_uart_read(uart_obj, (void*)pData, &byteCount);
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
* \param timeout   The time out is not used by this function.
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
        cy_rslt_t rslt = mtb_hal_uart_write(uart_obj, (void*)pData, &byteCount);
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
