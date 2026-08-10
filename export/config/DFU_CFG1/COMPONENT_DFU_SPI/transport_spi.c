/***************************************************************************//**
* \file transport_spi.c
*
* This file provides the source code of the DFU communication APIs
* for the SPI driver from HAL.
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
    CY_ASSERT(NULL != config);
    CY_ASSERT(NULL != config->spi);
    CY_ASSERT(NULL != config->callback);

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
    CY_ASSERT(NULL != spi_callback);
    if (NULL != spi_callback)
    {
        spi_callback(CY_DFU_TRANSPORT_SPI_INIT);
    }
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
    CY_ASSERT(NULL != spi_callback);
    if (NULL != spi_callback)
    {
        spi_callback(CY_DFU_TRANSPORT_SPI_DEINIT);
    }
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
cy_en_dfu_status_t SPI_SpiCyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t *count, uint32_t timeout)
{
    cy_en_dfu_status_t statusLoc = CY_DFU_ERROR_BAD_PARAM;
    uint16_t dataSize;

    if ((pData != NULL) && (size > 0U))
    {
        statusLoc = CY_DFU_ERROR_TIMEOUT;
        dataSize = (uint16_t) size;

        #ifdef CY_IP_MXS22SCB
        if (CY_RSLT_SUCCESS == mtb_hal_spi_target_read_transaction(spi_target_obj, pData, &dataSize, timeout))
        #else
        if (CY_RSLT_SUCCESS == mtb_hal_spi_target_read(spi_target_obj, pData, &dataSize, timeout))
        #endif
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
*  \param timeout: The timeout is not used by this function.
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
