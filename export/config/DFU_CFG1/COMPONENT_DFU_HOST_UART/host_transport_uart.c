/***************************************************************************//**
* \file host_transport_uart.c
*
* This file provides the source code of the DFU host communication APIs
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

#include "mtb_hal.h"
#include "cybsp.h"
#include "host_transport_uart.h"
#include "cy_dfu_logging.h"


#define UART_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32_t)(actBufSize) < (uint32_t)(bufSize)) ? \
                                ((uint32_t) (actBufSize)) : ((uint32_t) (bufSize)) )

#ifndef UART_BYTE_TO_BYTE_TIMEOUT_US
    #define UART_BYTE_TO_BYTE_TIMEOUT_US            (868U)
#endif

#define UART_SCB_HW_BUF_SIZE        (96U)

/*******************************************************************************
* Internal variables
*******************************************************************************/
/* The UART HAL object for DFU host */
static mtb_hal_uart_t *host_uartObj;

/* The pointer to the initialization/de-initialization callback function
 * UART host hardware
 */
static host_UartCallback host_uartCallback;


/*******************************************************************************
* Function Name: Host_UartConfig
****************************************************************************//**
*
* Configure the DFU host UART transport.
*
* \param config Configuration structure.
*
*******************************************************************************/
void Host_UartConfig(host_uart_cfg_t * config)
{
    CY_ASSERT(NULL != config);
    CY_ASSERT(NULL != config->uartObj);
    CY_ASSERT(NULL != config->callback);

    host_uartObj       = config->uartObj;
    host_uartCallback  = config->callback;
}


/*******************************************************************************
* Function Name: Host_UartStart
****************************************************************************//**
*
* Starts the UART host transport for a companion device.
*
*******************************************************************************/
void Host_UartStart(void)
{
    CY_ASSERT(NULL != host_uartCallback);

    if (NULL != host_uartCallback)
    {
        host_uartCallback(HOST_UART_INIT);
    }
}


/*******************************************************************************
* Function Name: Host_UartStop
****************************************************************************//**
*
*  Stops the UART host transport for a companion device.
*
*******************************************************************************/
void Host_UartStop(void)
{
    CY_ASSERT(NULL != host_uartCallback);

    if (NULL != host_uartCallback)
    {
        host_uartCallback(HOST_UART_DEINIT);
    }
}


/*******************************************************************************
* Function Name: Host_UartReset
****************************************************************************//**
*
*  De-init a DFU host device.
*
*******************************************************************************/
void Host_UartReset(void)
{

}


/*******************************************************************************
* Function Name: Host_UartRead
****************************************************************************//**
*  This function is intended for use with a companion device.
*
*  Allows the caller to read data from the companion device.
*
*  \param pData: Pointer to storage for the block of data to be read from
*   a companion device.
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
cy_en_dfu_status_t Host_UartRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
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
            if (mtb_hal_uart_readable(host_uartObj) != 0U)
            {
                uint32_t readBytes = 0;
                /* Wait for end of packet */
                do
                {
                    byteCount = mtb_hal_uart_readable(host_uartObj);
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
                            cy_rslt_t rslt = mtb_hal_uart_read(host_uartObj, (void*)pData, &byteCount);
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
                while (byteCount != mtb_hal_uart_readable(host_uartObj));

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
            cy_rslt_t rslt = mtb_hal_uart_read(host_uartObj, (void*)pData, &byteCount);
            status = (rslt == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_UNKNOWN;
        }
    }

    return (status);
}
 


/*******************************************************************************
* Function Name: Host_UartWrite
****************************************************************************//**
*  This function is intended for use with a companion device.
*
*  Allows the caller to write data to the companion device.
*
*  \param pData: Pointer to the block of data to be written to the companion device.
*
*  \param size: Number of bytes to be written.
*  \param count: Pointer to the variable to write the number of bytes actually
*   written.
*  \param timeOut: The timeout is not used by this function.
*
*  \return
*   Returns CYRET_SUCCESS if no problem was encountered or returns the value
*   that best describes the problem. For more information refer to the
*   "Return Codes" section of the System Reference Guide.
*
*******************************************************************************/
cy_en_dfu_status_t Host_UartWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeOut)
{
    cy_en_dfu_status_t status;

    status = CY_DFU_ERROR_UNKNOWN;

    if ((pData != NULL) && (size > 0U))
    {
        /* Transmit data. This function does not wait until data is sent. */
        size_t byteCount = size;
        cy_rslt_t rslt = mtb_hal_uart_write(host_uartObj, (void*)pData, &byteCount);
        status = (rslt == CY_RSLT_SUCCESS) ? CY_DFU_SUCCESS : CY_DFU_ERROR_UNKNOWN;

        *count = size;

        if (timeOut != 0U)
        {
            /* empty */
        }
    }

    return (status);
}

/* [] END OF FILE */
