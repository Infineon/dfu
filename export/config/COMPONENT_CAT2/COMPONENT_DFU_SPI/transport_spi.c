/***************************************************************************//**
* \file transport_spi.c
* \version 5.1
*
* This file provides the source code of the DFU communication APIs
* for the SCB Component SPI mode.
*
* Note
* This file supports the PSoC Creator and ModusToolbox flows.
* For the PSoC Creator flow, the default SPI component instance name is
* "SPI". For the ModusToolbox flow, the SPI personality alias must be
* "DFU_SPI".
* This file serves as a template and can be modified defining any component
* name or personality alias.
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

#include "transport_spi.h"
#include "cy_scb_spi.h"
#include "cy_sysint.h"
#include <string.h>

/* Includes driver configuration */
#include "cycfg_peripherals.h"

#if !defined DFU_SPI_HW
    #error The SPI personality alias must be DFU_SPI to support DFU communication API.

    /* Dummy configuration to generate only error above during a build */
    #define CY_DFU_SPI_HW           NULL
    #define CY_DFU_SPI_CFG_PTR      NULL
    #define SPI_INTR_SOURCE        (0U)

#else

    /* USER CONFIGURABLE: the pointer to the base address of the hardware */
    #define CY_DFU_SPI_HW           DFU_SPI_HW

    /* USER CONFIGURABLE: the pointer to the driver configuration */
    #define  CY_DFU_SPI_CFG_PTR     (&DFU_SPI_config)

    /* USER CONFIGURABLE: Interrupt configuration for the SPI block. */
    #define SPI_INTR_SOURCE         DFU_SPI_IRQ

#endif /* !defined DFU_SPI_HW */

/*
* USER CONFIGURABLE: The slave select line constant. Update it based on the
* pin selected for slave select.
*/
#ifndef CY_SPI_SLAVE_SELECT
    #define CY_SPI_SLAVE_SELECT     CY_SCB_SPI_SLAVE_SELECT1
#endif /* CY_SPI_SLAVE_SELECT */

/** The instance-specific context structure.
* It is used while the driver operation for internal configuration and
* data keeping for the UART. The user should not modify anything in this
* structure.
*/
static cy_stc_scb_spi_context_t      SPI_context;
#define CY_DFU_SPI_CONTEXT           SPI_context

/*******************************************************************************
* Interrupt configuration
*******************************************************************************/

/** Interrupt priority for Cortex-M0. Valid range: 0 to 3. */
#ifndef SPI_INTR_PRIORITY
    #define SPI_INTR_PRIORITY               (3U)
#endif /* SPI_INTR_PRIORITY */

/*******************************************************************************
* UART transport buffers
*******************************************************************************/

/* Size of Read buffer for UART DFU  */
#define SPI_BTLDR_SIZEOF_RX_BUFFER   (64U)

/* UART reads to this buffer */
static uint8_t SPI_RxBuf[SPI_BTLDR_SIZEOF_RX_BUFFER];
/* Number items in the buffer */
static uint32_t SPI_RxBufIdx;

/* Byte to byte time interval in microseconds. Slave waits for this amount of
 * time between checking whether more data is being received. Slave starts
 * processing the packet if the number of received bytes remain same between
 * two successive checks. Change this depending on the expected inter-byte delay
 * for a given SPI master.
 */
#ifndef SPI_BYTE_TO_BYTE
    #define SPI_BYTE_TO_BYTE   (32U)
#endif /* SPI_BYTE_TO_BYTE */

/* Timeout unit in microseconds */
#define SPI_WAIT_1_MS           (1000U)

/* Return number of bytes to copy into DFU buffer */
#define SPI_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32_t)(actBufSize) < (uint32_t)(bufSize)) ? \
                                ((uint32_t) (actBufSize)) : ((uint32_t) (bufSize)) )

/**
* SPI_initVar indicates whether the SPI driver has been initialized. The
* variable is initialized to false and set to true the first time
* \ref SPI_SpiCyBtldrCommStart is called. This allows  the driver to restart
* without re-initialization after the first call to the
* \ref SPI_SpiCyBtldrCommStart routine.
* For re-initialization set \ref SPI_initVar to false and call
* \ref SPI_SpiCyBtldrCommStart.
* Note that PSoC Creator SPI component uses its own initVar variable.
*/
bool SPI_initVar = false;


/*******************************************************************************
* Function Name: UART_Interrupt
****************************************************************************//**
*
* The SCB SPI driver interrupt handler.
*
*******************************************************************************/
void SPI_Interrupt(void);
void SPI_Interrupt(void)
{
    if (0UL != (CY_SCB_RX_INTR_NOT_EMPTY & Cy_SCB_GetRxInterruptStatusMasked(CY_DFU_SPI_HW)))
    {
        if(SPI_RxBufIdx < sizeof(SPI_RxBuf))
        {
            SPI_RxBuf[SPI_RxBufIdx] = (uint8_t)Cy_SCB_SPI_Read(CY_DFU_SPI_HW);
        }
        SPI_RxBufIdx++;

        Cy_SCB_ClearRxInterrupt(CY_DFU_SPI_HW, CY_SCB_RX_INTR_NOT_EMPTY);
    }
}


/*******************************************************************************
* Function Name: SPI_Start
****************************************************************************//**
*
* Starts SCB SPI operation.
*
* \globalvars
* \ref SPI_initVar - used to check initial configuration, modified on first
*                    function call.
*
*******************************************************************************/
static void SPI_Start(void);
static void SPI_Start(void)
{
    if (false == SPI_initVar)
    {
        cy_en_scb_spi_status_t status;

        /* Interrupt configuration structure */
        static const cy_stc_sysint_t SPI_SCB_IRQ_cfg =
        {
            .intrSrc      = (IRQn_Type) SPI_INTR_SOURCE,
            .intrPriority = SPI_INTR_PRIORITY
        };

        /* Configure component */
        status = Cy_SCB_SPI_Init(CY_DFU_SPI_HW, CY_DFU_SPI_CFG_PTR, &CY_DFU_SPI_CONTEXT);

        /* A SPI initialization error - stops the execution */
        CY_ASSERT(CY_SCB_SPI_SUCCESS == status);
        (void) status;

        /* Set active slave select to line 0 */
        Cy_SCB_SPI_SetActiveSlaveSelect(CY_DFU_SPI_HW, CY_SPI_SLAVE_SELECT);

        (void) Cy_SysInt_Init(&SPI_SCB_IRQ_cfg, &SPI_Interrupt);
        NVIC_EnableIRQ((IRQn_Type) SPI_SCB_IRQ_cfg.intrSrc);

        /* Component is configured */
        SPI_initVar = true;
    }

    Cy_SCB_SPI_Enable(CY_DFU_SPI_HW);
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommStart
****************************************************************************//**
*
*  Starts the SPI component.
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
    #if defined(CY_PSOC_CREATOR_USED)
        SPI_API(_Start)();
    #else
        SPI_Start();
    #endif /* #if defined(CY_PSOC_CREATOR_USED) */

    SPI_RxBufIdx = 0U;
    Cy_SCB_SetRxInterruptMask(CY_DFU_SPI_HW, CY_SCB_RX_INTR_NOT_EMPTY);
}


/*******************************************************************************
* Function Name: SPI_SpiCyBtldrCommStop
****************************************************************************//**
*
*  Disables the SPI component.
*
*******************************************************************************/
void SPI_SpiCyBtldrCommStop(void)
{
    Cy_SCB_SPI_Disable(CY_DFU_SPI_HW, NULL);
    Cy_SCB_SPI_DeInit(CY_DFU_SPI_HW);
    SPI_initVar = false;
    Cy_SCB_SetRxInterruptMask(CY_DFU_SPI_HW, 0U);
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
    Cy_SCB_SPI_ClearTxFifo(CY_DFU_SPI_HW);
    Cy_SCB_SPI_ClearRxFifo(CY_DFU_SPI_HW);
    SPI_RxBufIdx = 0U;
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
    cy_en_dfu_status_t statusLoc = CY_DFU_ERROR_UNKNOWN;
    uint32_t byteCount;

    if ((pData != NULL) && (size > 0U))
    {
        statusLoc = CY_DFU_ERROR_TIMEOUT;

        /* Wait with timeout 1mS for packet start */
        byteCount = 0U;

        do
        {
            /* Check packet start */
            if (0U != SPI_RxBufIdx)
            {
                /* Wait for packet end */
                do
                {
                    byteCount = SPI_RxBufIdx;
                    Cy_SysLib_DelayUs(SPI_BYTE_TO_BYTE);
                }
                while (byteCount != SPI_RxBufIdx);

                /* Disable data reception into RX FIFO */
                CY_DFU_SPI_HW->RX_FIFO_CTRL |= SCB_RX_FIFO_CTRL_FREEZE_Msk;

                byteCount = SPI_BYTES_TO_COPY(byteCount, size);
                *count = byteCount;

                /* Get data from RX buffer into DFU buffer */
                (void) memcpy((void *) pData, (void *) SPI_RxBuf, (uint32_t) byteCount);
                SPI_RxBufIdx = 0U;

                statusLoc = CY_DFU_SUCCESS;
                break;
            }

            Cy_SysLib_DelayUs(SPI_WAIT_1_MS);
            --timeout;
        }
        while (0U != timeout);
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
    cy_en_dfu_status_t statusLoc = CY_DFU_ERROR_UNKNOWN;

    if ((NULL != pData) && (size > 0U))
    {
        statusLoc = CY_DFU_ERROR_TIMEOUT;

        Cy_SCB_SPI_ClearTxFifo(CY_DFU_SPI_HW);

        /* Put data into TX buffer */
        CY_MISRA_DEVIATE_LINE('MISRA C-2012 Rule 11.8','Removing const does not have negative impact as function does not modify data.');
        Cy_SCB_SPI_WriteArrayBlocking(CY_DFU_SPI_HW, (void *)pData, size);

        /* Wait with timeout 1mS for packet end */
        do
        {
            /* Check for packet end */
            uint32_t numInTxFifo = Cy_SCB_SPI_GetNumInTxFifo(CY_DFU_SPI_HW);
            uint32_t txSrValid = Cy_SCB_GetTxSrValid(CY_DFU_SPI_HW);
            if ((!Cy_SCB_SPI_IsBusBusy(CY_DFU_SPI_HW)) &&
                (0U == numInTxFifo) &&
                (0U == txSrValid))
            {
                *count = size;
                statusLoc = CY_DFU_SUCCESS;
                break;
            }

            Cy_SysLib_DelayUs(SPI_WAIT_1_MS);
            --timeout;
        }
        while (0U != timeout);

        /* Enable data reception into RX FIFO */
        Cy_SCB_SPI_ClearRxFifo(CY_DFU_SPI_HW);
        CY_DFU_SPI_HW->RX_FIFO_CTRL &= (uint32_t)~SCB_RX_FIFO_CTRL_FREEZE_Msk;
    }

    return (statusLoc);
}


/* [] END OF FILE */
