/***************************************************************************//**
* \file transport_spi.c
* \version 6.0
*
* This file provides the source code of the DFU communication APIs
* for the SPI driver from HAL.
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
#include "mtb_hal_spi.h"
#include "transport_spi.h"


/*******************************************************************************
* Internal variable
*******************************************************************************/
/* The pointer to the SPI HAL object */
static mtb_hal_spi_t *spi_target_obj;

/* The pointer to callback function for initialization/de-initialization of
 * SPI hardware.
 */
static Cy_DFU_TransportSpiCallback spi_callback;


/*******************************************************************************
* Function Name: Cy_DFU_TransportSpiConfig
****************************************************************************//**
*
* Configure DFU SPI Transport
*
* Call this function in the user application to provide the HAL object and callback
* function to DFU transport.
*
* \param config Configuration structure
*
*******************************************************************************/
void Cy_DFU_TransportSpiConfig(cy_stc_dfu_transport_spi_cfg_t * config)
{
    spi_target_obj = config->spi;
    spi_callback = config->callback;
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommStart
****************************************************************************//**
*
*  Starts the SPI transport.
*
* \note
*  This function calls user callback to perform HW configuration of transport.
*
*******************************************************************************/
void SPI_SpiCyBtldrCommStart(void)
{
    spi_callback(CY_DFU_TRANSPORT_SPI_INIT);
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommStop
****************************************************************************//**
*
*  Stops the SPI transport.
*
*******************************************************************************/
void SPI_SpiCyBtldrCommStop(void)
{
    spi_callback(CY_DFU_TRANSPORT_SPI_DEINIT);
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommReset
****************************************************************************//**
*
*  Resets the receive and transmit communication buffers and the target status.
*
*******************************************************************************/
void SPI_SpiCyBtldrCommReset(void)
{
    (void) mtb_hal_spi_clear(spi_target_obj);
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommRead
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
cy_en_dfu_status_t SPI_SpiCyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t statusLoc = CY_DFU_ERROR_BAD_PARAM;
    uint16_t dataSize;

    if ((pData != NULL) && (size > 0U))
    {
        statusLoc = CY_DFU_ERROR_TIMEOUT;
        dataSize = (uint16_t) size;

        if (CY_RSLT_SUCCESS == mtb_hal_spi_target_read(spi_target_obj, pData, &dataSize, timeout))
        {
            *count = dataSize;
            statusLoc = CY_DFU_SUCCESS;
        }
    }

    return (statusLoc);
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommWrite
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
cy_en_dfu_status_t SPI_SpiCyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t statusLoc = CY_DFU_ERROR_BAD_PARAM;
    uint16_t dataSize;
    cy_rslt_t statusHal;

    if ((NULL != pData) && (size > 0U))
    {
        statusLoc = CY_DFU_ERROR_TIMEOUT;
        dataSize = (uint16_t) size;

        /* mtb_hal_spi_clear always returns success */
        (void) mtb_hal_spi_clear(spi_target_obj);
        /* Check if the DFU Host tool has already started the SPI transfer.
         * The DFU Host tool starts transfer through specific periods of time.
         * If mtb_hal_spi_target_write() is called during active data transmission,
         * the extra byte will be read and recognized as a new DFU packet.
         */
        while(mtb_hal_spi_is_busy(spi_target_obj) && (timeout > 0U))
        {
            statusHal = mtb_hal_system_delay_ms(1U);
            CY_ASSERT(CY_RSLT_SUCCESS == statusHal);
            /* To avoid the compiler warning in Release mode */
            (void) statusHal;
            timeout--;
        }

        if (timeout > 0U)
        {
            if (CY_RSLT_SUCCESS == mtb_hal_spi_target_write(spi_target_obj, pData, &dataSize, timeout))
            {
                *count = dataSize;
                statusLoc = CY_DFU_SUCCESS;
            }
        }

        /* Clear RX buffer to delete the extra byte if the DFU Host tool
         * starts transmission during the SPI transfer setup from the DFU middleware
         * side in the cyhal_spi_target_write() function.
         * mtb_hal_spi_clear always returns success.
         */
        (void) mtb_hal_spi_clear(spi_target_obj);
    }

    return (statusLoc);
}

/* [] END OF FILE */
