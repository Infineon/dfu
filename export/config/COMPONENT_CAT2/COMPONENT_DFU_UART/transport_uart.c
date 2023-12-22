/***************************************************************************//**
* \file transport_uart.c
* \version 5.1
*
* This file provides the source code of the DFU communication APIs
* for the SCB Component UART mode.
*
* Note
* This file supports the PSoC Creator and ModusToolbox flows.
* For the PSoC Creator flow, the default UART component instance name is
* "UART". For the ModusToolbox flow, the UART personality alias must be
* "DFU_UART".
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

#include "transport_uart.h"
#include "cy_scb_uart.h"
#include "cy_sysint.h"
#include <string.h>

/*
* USER CONFIGURABLE: Byte to byte time interval: calculated basing on current
* component baud rate configuration.
* Set it to approximately to (50e6 / baud_rate) value in microseconds.
* E.g. baud_rate = 115200, UART_BYTE_TO_BYTE_TIMEOUT_US ~ 434
*/
#ifndef UART_BYTE_TO_BYTE_TIMEOUT_US
    #define UART_BYTE_TO_BYTE_TIMEOUT_US  (868U)
#endif /* UART_BYTE_TO_BYTE_TIMEOUT_US */

/* Includes driver configuration */
#include "cycfg_peripherals.h"

#if !defined DFU_UART_HW
    #error The UART personality alias must be DFU_UART to support DFU communication API.

    /* Dummy configuration to generate only error above during a build */
    #define CY_DFU_UART_HW          NULL
    #define CY_DFU_UART_CFG_PTR     NULL
    #define UART_INTR_SOURCE        (0U)

#else

    /* USER CONFIGURABLE: the pointer to the base address of the hardware */
    #define CY_DFU_UART_HW          DFU_UART_HW

    /* USER CONFIGURABLE: the pointer to the configuration */
    #define CY_DFU_UART_CFG_PTR     (&DFU_UART_config)

    /* USER CONFIGURABLE: Interrupt configuration for the I2C block. */
    #define UART_INTR_SOURCE         DFU_UART_IRQ

#endif /* !defined DFU_UART_HW */

/** The instance-specific context structure.
* It is used while the driver operation for internal configuration and
* data keeping for the UART. The user should not modify anything in this
* structure.
*/
static cy_stc_scb_uart_context_t     UART_context;
#define CY_DFU_UART_CONTEXT          UART_context


/*******************************************************************************
* Interrupt configuration
*******************************************************************************/

/** Interrupt priority for Cortex-M0. Valid range: 0 to 3. */
#ifndef UART_INTR_PRIORITY
    #define UART_INTR_PRIORITY               (3U)
#endif /* UART_INTR_PRIORITY */

/*******************************************************************************
* UART transport buffers
*******************************************************************************/

/* Size of Read buffer for UART DFU  */
#define UART_BTLDR_SIZEOF_RX_BUFFER   (64U)

/* UART reads to this buffer */
static uint8_t UART_RxBuf[UART_BTLDR_SIZEOF_RX_BUFFER];
/* Number items in the buffer */
static uint32_t UART_RxBufIdx;

/* Returns a number of bytes to copy into a DFU buffer */
#define UART_BYTES_TO_COPY(actBufSize, bufSize) \
                            ( ((uint32_t)(actBufSize) < (uint32_t)(bufSize)) ? \
                                ((uint32_t) (actBufSize)) : ((uint32_t) (bufSize)) )

/**
* UART_initVar indicates whether the UART driver has been initialized. The
* variable is initialized to false and set to true the first time
* \ref UART_UartCyBtldrCommStart is called. This allows  the driver to restart
* without re-initialization after the first call to the
* \ref UART_UartCyBtldrCommStart routine.
* For re-initialization set \ref UART_initVar to false and call
* \ref UART_UartCyBtldrCommStart.
* Note that PSoC Creator UART component uses its own initVar variable.
*/
bool UART_initVar = false;


/*******************************************************************************
* Function Name: UART_Interrupt
****************************************************************************//**
*
* The SCB UART driver interrupt handler.
*
*******************************************************************************/
void UART_Interrupt(void);
void UART_Interrupt(void)
{
    if (0UL != (CY_SCB_RX_INTR_NOT_EMPTY & Cy_SCB_GetRxInterruptStatusMasked(CY_DFU_UART_HW)))
    {
        if(UART_RxBufIdx < sizeof(UART_RxBuf))
        {
            UART_RxBuf[UART_RxBufIdx] = (uint8_t)Cy_SCB_UART_Get(CY_DFU_UART_HW);
        }
        UART_RxBufIdx++;

        Cy_SCB_ClearRxInterrupt(CY_DFU_UART_HW, CY_SCB_RX_INTR_NOT_EMPTY);
    }
}


/*******************************************************************************
* Function Name: UART_Start
****************************************************************************//**
*
* Starts SCB UART operation.
*
* \globalvars
* \ref UART_initVar - used to check initial configuration, modified on first
*                     function call.
*
*******************************************************************************/
static void UART_Start(void);
static void UART_Start(void)
{
    if (false == UART_initVar)
    {
        cy_en_scb_uart_status_t status;

        /* Interrupt configuration structure */
        static const cy_stc_sysint_t UART_SCB_IRQ_cfg =
        {
            .intrSrc      = (IRQn_Type) UART_INTR_SOURCE,
            .intrPriority = UART_INTR_PRIORITY
        };

        /* Configure component */
        status = Cy_SCB_UART_Init(CY_DFU_UART_HW, CY_DFU_UART_CFG_PTR, &CY_DFU_UART_CONTEXT);

        /* A UART initialization error - stops the execution */
        CY_ASSERT(CY_SCB_UART_SUCCESS == status);
        (void) status;

        (void) Cy_SysInt_Init(&UART_SCB_IRQ_cfg, &UART_Interrupt);
        NVIC_EnableIRQ((IRQn_Type) UART_SCB_IRQ_cfg.intrSrc);

        /* Component is configured */
        UART_initVar = true;
    }

    Cy_SCB_UART_Enable(CY_DFU_UART_HW);
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommStart
****************************************************************************//**
*
* Starts the UART component.
*
* \note
* This function does not configure an infrastructure required for the SCB UART
* operation: clocks and pins. For the PSoC Creator and ModusToolbox flows, the
* generated files configure clocks and pins. This configuration must be
* performed by the application when the project uses only PDL.
*
*******************************************************************************/
void UART_UartCyBtldrCommStart(void)
{
#if defined(CY_PSOC_CREATOR_USED)
    UART_API(_Start)();
#else
    UART_Start();
#endif /* defined(CY_PSOC_CREATOR_USED) */

    UART_RxBufIdx = 0U;
    Cy_SCB_SetRxInterruptMask(CY_DFU_UART_HW, CY_SCB_RX_INTR_NOT_EMPTY);
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommStop
****************************************************************************//**
*
* Disables the UART component.
*
*******************************************************************************/
void UART_UartCyBtldrCommStop(void)
{
#if defined(CY_PSOC_CREATOR_USED)
    UART_API(_Disable)();
#else
    Cy_SCB_UART_Disable(CY_DFU_UART_HW, NULL);
#endif /* defined(CY_PSOC_CREATOR_USED) */

    Cy_SCB_SetRxInterruptMask(CY_DFU_UART_HW, 0U);
}


/*******************************************************************************
* Function Name: UART_UartCyBtldrCommReset
****************************************************************************//**
*
* Resets the receive and transmit communication buffers.
*
*******************************************************************************/
void UART_UartCyBtldrCommReset(void)
{
    /* Clear RX and TX buffers */
    Cy_SCB_UART_ClearRxFifo(CY_DFU_UART_HW);
    Cy_SCB_UART_ClearTxFifo(CY_DFU_UART_HW);
    UART_RxBufIdx = 0U;
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
    uint32_t byteCount;

    status = CY_DFU_ERROR_UNKNOWN;

    if ((pData != NULL) && (size > 0U))
    {
        status = CY_DFU_ERROR_TIMEOUT;
        /* Wait with timeout 1 ms for packet end */
        byteCount = 0U;
        do
        {
            /* Check packet start */
            if (UART_RxBufIdx != 0U)
            {
                /* Wait for end of packet */
                do
                {
                    byteCount = UART_RxBufIdx;
                    Cy_SysLib_DelayUs(UART_BYTE_TO_BYTE_TIMEOUT_US);
                }
                while (byteCount != UART_RxBufIdx);

                byteCount = UART_BYTES_TO_COPY(byteCount, size);
                *count = byteCount;

                /* Get data from RX buffer into DFU buffer */
                (void) memcpy((void *) pData, (const void *) UART_RxBuf, (uint32_t) byteCount);
                UART_RxBufIdx = 0U;

                status = CY_DFU_SUCCESS;
                break;
            }

            Cy_SysLib_Delay(1U);
            --timeout;
        }
        while (timeout != 0U);
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
        Cy_SCB_UART_PutArrayBlocking(CY_DFU_UART_HW, (void*)pData, size);

        *count = size;
        status = CY_DFU_SUCCESS;

        if (timeout != 0U)
        {
            /* empty */
        }
    }

    return (status);
}


/* [] END OF FILE */
