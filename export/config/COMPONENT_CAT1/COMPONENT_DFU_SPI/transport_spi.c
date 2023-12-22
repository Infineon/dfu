/***************************************************************************//**
* \file transport_spi.c
* \version 5.1
*
* This file provides the source code of the DFU communication APIs
* for the SPI driver from HAL.
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
#include "cyhal_spi.h"
#include "cycfg_pins.h"
#include "transport_spi.h"

/* SPI bus speed, 1 Mbps */
#ifndef DFU_SPI_BUS_SPEED
    #define DFU_SPI_BUS_SPEED           (1000000UL)
#endif
#ifndef DFU_SPI_BITS_NUM
    #define DFU_SPI_BITS_NUM            (8U)
#endif
#ifndef DFU_SPI_MOSI
    #define DFU_SPI_MOSI                CYBSP_SPI_MOSI
#endif
#ifndef DFU_SPI_MISO
    #define DFU_SPI_MISO                CYBSP_SPI_MISO
#endif
#ifndef DFU_SPI_CLK
    #define DFU_SPI_CLK                 CYBSP_SPI_CLK
#endif
#ifndef DFU_SPI_CS
    #define DFU_SPI_CS                  CYBSP_SPI_CS
#endif
#ifndef DFU_SPI_MODE
    #define DFU_SPI_MODE                CYHAL_SPI_MODE_00_MSB
#endif
#ifndef DFU_SPI_CS_POLARITY
    #define DFU_SPI_CS_POLARITY         CYHAL_SPI_SSEL_ACTIVE_LOW
#endif


/**
* SPI_initVar indicates whether the SPI driver has been initialized. The
* variable is initialized to false and set to true the first time
* \ref SPI_SpiCyBtldrCommStart is called. This allows  the driver to restart
* without re-initialization after the first call to the
* \ref SPI_SpiCyBtldrCommStart routine.
* For re-initialization set \ref SPI_initVar to false and call
* \ref SPI_SpiCyBtldrCommStart.
*/
bool SPI_initVar = false;

/* Global SPI object */
static cyhal_spi_t spi_slave_obj;

/*******************************************************************************
* Internal function declarations
*******************************************************************************/


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommStart
****************************************************************************//**
*
*  Starts the SPI transport.
*
* \note
*  This function does not configure an infrastructure required for the SCB SPI
*  operation: clocks and pins. For the PSoC Creator and ModusToolbox flows, the
*  generated files configure clocks and pins. This configuration must be
*  performed by the application when the project uses only PDL.
*
*******************************************************************************/
void SPI_SpiCyBtldrCommStart(void)
{
    if (!SPI_initVar)
    {
        cy_rslt_t rslt;

        /* Initialize transport */
        rslt = cyhal_spi_init(&spi_slave_obj, DFU_SPI_MOSI, DFU_SPI_MISO,
                                DFU_SPI_CLK, DFU_SPI_CS, NULL, DFU_SPI_BITS_NUM, DFU_SPI_MODE, true);
        /* A SPI initialization error - stops the execution */
        CY_ASSERT(CY_RSLT_SUCCESS == rslt);

        /* Set the SPI data rate */
         rslt = cyhal_spi_set_frequency(&spi_slave_obj, DFU_SPI_BUS_SPEED);
        /* A SPI configuration error - stops the execution */
        CY_ASSERT(CY_RSLT_SUCCESS == rslt);

        /* Set polarity for active SSEL pin */
         rslt = cyhal_spi_slave_select_config(&spi_slave_obj, DFU_SPI_CS, DFU_SPI_CS_POLARITY);
        /* A SPI configuration error - stops the execution */
        CY_ASSERT(CY_RSLT_SUCCESS == rslt);

        (void) rslt; /* Avoid warning for release mode */

        /* The transport is configured */
        SPI_initVar = true;
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
    cyhal_spi_free(&spi_slave_obj);
    SPI_initVar=false;
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommReset
****************************************************************************//**
*
*  Resets the receive and transmit communication buffers and the slave status.
*
*******************************************************************************/
void SPI_SpiCyBtldrCommReset(void)
{
    (void)cyhal_spi_abort_async(&spi_slave_obj);
    (void)cyhal_spi_clear(&spi_slave_obj);
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

        if (CY_RSLT_SUCCESS == cyhal_spi_slave_read(&spi_slave_obj, pData, &dataSize, timeout))
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

    if ((NULL != pData) && (size > 0U))
    {
        statusLoc = CY_DFU_ERROR_TIMEOUT;
        dataSize = (uint16_t) size;

        cyhal_spi_clear(&spi_slave_obj);
        if (CY_RSLT_SUCCESS == cyhal_spi_slave_write(&spi_slave_obj, pData, &dataSize, timeout))
        {
            *count = dataSize;
            statusLoc = CY_DFU_SUCCESS;
        }
    }

    return (statusLoc);
}

/* [] END OF FILE */
