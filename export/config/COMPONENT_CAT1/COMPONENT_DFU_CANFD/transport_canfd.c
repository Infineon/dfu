/***************************************************************************//**
* \file transport_canfd.c
* \version 5.2
*
* This file provides the source code of the DFU communication APIs
* for the CANFD driver.
*
********************************************************************************
* \copyright
* (c) (2024), Cypress Semiconductor Corporation (an Infineon company) or
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

#include "cy_canfd.h"
#include "transport_canfd.h"

/* Includes driver configuration */
#include "cycfg_peripherals.h"


#if !defined DFU_CANFD_HW
#error "The CANFD personality alias must be DFU_CANFD to support DFU communication API."

/* Dummy configuration to generate only error above during a build */
#define CY_DFU_CANFD_HW             NULL
#define CY_DFU_CANFD_CH_NUM         (0U)
#define CY_DFU_CANFD_CFG_PTR        NULL
#define CY_DFU_CANFD_IRQ_SOURCE     (0U)

#else

/* USER CONFIGURABLE: the pointer to the base address of the hardware */
#define CY_DFU_CANFD_HW             DFU_CANFD_HW

/* USER CONFIGURABLE: the number of the hardware channel */
#define CY_DFU_CANFD_CH_NUM         DFU_CANFD_CHANNEL_NUM

/* USER CONFIGURABLE: the pointer to the configuration */
#define CY_DFU_CANFD_CFG_PTR        (&DFU_CANFD_config)

/* USER CONFIGURABLE: Interrupt configuration for the CANFD block */
#if defined (COMPONENT_CAT1C)
#define CY_DFU_CANFD_CPU_IRQ_NUM    NvicMux3_IRQn
#define CY_DFU_CANFD_IRQ_SOURCE     ((CY_DFU_CANFD_CPU_IRQ_NUM << CY_SYSINT_INTRSRC_MUXIRQ_SHIFT) | DFU_CANFD_IRQ_0)
#else
#error "Unsupported Device Family"
#endif

#endif /* !defined DFU_CANFD_HW */

/* Interrupt priority. Valid range: 0 to 7. */
#ifndef DFU_CANFD_IRQ_PRIORITY
#define DFU_CANFD_IRQ_PRIORITY      (7U)
#endif /* DFU_CANFD_IRQ_PRIORITY */

/** The instance-specific context structure.
* It is used while the driver operation for internal configuration and
* data keeping for the CANFD. The user should not modify anything in this
* structure.
*/
static cy_stc_canfd_context_t   CANFD_context;
#define CY_DFU_CANFD_CONTEXT    CANFD_context

/**
* CANFD_initVar indicates whether the CANFD driver has been initialized. The
* variable is initialized to false and set to true the first time
* \ref CANFD_CanfdCyBtldrCommStart is called. This allows the driver to restart
* without re-initialization after the first call to the
* \ref CANFD_CanfdCyBtldrCommStart routine.
* For re-initialization set \ref CANFD_initVar to false and call
* \ref CANFD_CanfdCyBtldrCommStart.
*/
bool CANFD_initVar = false;

/**
* CANFD_rxBufferAvailable indicates whether the CANFD RX buffer has new data available.
* The variable is initialized to false and set to true in the interrupt handler,
* if new data in RX buffer was received, and set to false in the Read function,
* after new data was processed. This allows the Read function to wait
* if no new data available yet.
*/
static volatile bool CANFD_rxBufferAvailable = false;

/* Timeout unit in millisecond */
#define CANFD_WAIT_1_MS             (1U)

/* Indexes of the Read/Write buffers for CANFD DFU */
#define CANFD_TX_BUFFER_INDEX       (0U)
#define CANFD_RX_BUFFER_INDEX       (0U)

/* RX buffer element for CANFD DFU */
static cy_stc_canfd_r0_t CANFD_RX_r0_f;             /* R0 register */
static cy_stc_canfd_r1_t CANFD_RX_r1_f;             /* R1 register */
static cy_stc_canfd_rx_buffer_t CANFD_rxBuffer =    /* Rx buffer */
{
    .r0_f = &CANFD_RX_r0_f,
    .r1_f = &CANFD_RX_r1_f,
};

/**
* Lookup table for the CANFD_DlcToSize function to convert
* the Data Length Code of the CAN message to a value in bytes.
*/
static const uint32_t sizeLookupTable[] =
{
    8U,
    12U,
    16U,
    20U,
    24U,
    32U,
    48U,
    64U,
};


/*******************************************************************************
* CANFD transport internal function prototypes
*******************************************************************************/
static uint32_t CANFD_DlcToSize(uint32_t dlc);
static uint32_t CANFD_SizeToDlc(uint32_t size);


/*******************************************************************************
* Function Name: CANFD_Interrupt
****************************************************************************//**
*
* The CANFD driver interrupt handler.
*
*******************************************************************************/
__STATIC_INLINE void CANFD_Interrupt(void)
{
    if (CY_CANFD_RX_BUFFER_NEW_MESSAGE == (Cy_CANFD_GetInterruptStatus(CY_DFU_CANFD_HW, CY_DFU_CANFD_CH_NUM) & CY_CANFD_RX_BUFFER_NEW_MESSAGE))
    {
        Cy_CANFD_ClearInterrupt(CY_DFU_CANFD_HW, CY_DFU_CANFD_CH_NUM, CY_CANFD_RX_BUFFER_NEW_MESSAGE);
        CANFD_rxBufferAvailable = true;
    }
}


/*******************************************************************************
* Function Name: CANFD_CanfdCyBtldrCommStart
****************************************************************************//**
*
* Starts the CANFD transport.
*
*******************************************************************************/
void CANFD_CanfdCyBtldrCommStart(void)
{
    if (!CANFD_initVar)
    {
        cy_en_canfd_status_t status;

        status = Cy_CANFD_Init(CY_DFU_CANFD_HW, CY_DFU_CANFD_CH_NUM, CY_DFU_CANFD_CFG_PTR, &CY_DFU_CANFD_CONTEXT);
        CY_ASSERT(CY_CANFD_SUCCESS == status);

        Cy_CANFD_SetInterruptMask(CY_DFU_CANFD_HW, CY_DFU_CANFD_CH_NUM, CY_CANFD_RX_BUFFER_NEW_MESSAGE);

        static const cy_stc_sysint_t CANFD_IRQ_cfg =
        {
            .intrSrc = CY_DFU_CANFD_IRQ_SOURCE,
            .intrPriority = DFU_CANFD_IRQ_PRIORITY,
        };

        (void)Cy_SysInt_Init(&CANFD_IRQ_cfg, CANFD_Interrupt);
        NVIC_EnableIRQ(CY_DFU_CANFD_CPU_IRQ_NUM);

        CANFD_initVar = true;
    }
}


/*******************************************************************************
* Function Name: CANFD_CanfdCyBtldrCommStop
****************************************************************************//**
*
*  Stops the CANFD transport.
*
*******************************************************************************/
void CANFD_CanfdCyBtldrCommStop(void)
{
    if (!CANFD_initVar)
    {
        cy_en_canfd_status_t status;

        status = Cy_CANFD_DeInit(CY_DFU_CANFD_HW, CY_DFU_CANFD_CH_NUM, &CY_DFU_CANFD_CONTEXT);
        CY_ASSERT(CY_CANFD_SUCCESS == status);

        CANFD_initVar = false;
    }
}


/*******************************************************************************
* Function Name: CANFD_CanfdCyBtldrCommReset
****************************************************************************//**
*
*  Resets the receive and transmit communication buffers.
*
*******************************************************************************/
void CANFD_CanfdCyBtldrCommReset(void)
{
    CANFD_rxBufferAvailable = false;
    Cy_CANFD_AckRxBuf(CY_DFU_CANFD_HW, CY_DFU_CANFD_CH_NUM, CANFD_RX_BUFFER_INDEX);
}


/*******************************************************************************
* Function Name: CANFD_CanfdCyBtldrCommRead
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
*   Returns CY_DFU_SUCCESS if no problem was encountered or returns the value
*   that best describes the problem. For more information refer to the
*   \ref cy_en_dfu_status_t description in the API Reference Guide.
*
*******************************************************************************/
cy_en_dfu_status_t CANFD_CanfdCyBtldrCommRead(uint8_t pData[], uint32_t size, uint32_t* count, uint32_t timeout)
{
    cy_en_dfu_status_t status = CY_DFU_ERROR_BAD_PARAM;
    uint32_t counter = timeout;

    if ((pData != NULL) && (count != NULL) && (size > 0U))
    {
        status = CY_DFU_ERROR_TIMEOUT;

        do
        {
            if (CANFD_rxBufferAvailable)
            {
                status = CY_DFU_SUCCESS;
                break;
            }
            else
            {
                if (counter > 0U)
                {
                    Cy_SysLib_Delay(CANFD_WAIT_1_MS);
                    --counter;
                }
            }
        } while (0U != counter);

        if (CY_DFU_SUCCESS == status)
        {
            uint32_t address = Cy_CANFD_CalcRxBufAdrs(CY_DFU_CANFD_HW, CY_DFU_CANFD_CH_NUM, CANFD_RX_BUFFER_INDEX, &CY_DFU_CANFD_CONTEXT);
            CY_ASSERT(0UL != address);

            CANFD_rxBuffer.data_area_f = (uint32_t*)pData;

            cy_en_canfd_status_t canfdStatus = Cy_CANFD_GetRxBuffer(CY_DFU_CANFD_HW, CY_DFU_CANFD_CH_NUM, address, &CANFD_rxBuffer);
            CY_ASSERT(CY_CANFD_BAD_PARAM != canfdStatus);

            CANFD_rxBufferAvailable = false;
            Cy_CANFD_AckRxBuf(CY_DFU_CANFD_HW, CY_DFU_CANFD_CH_NUM, CANFD_RX_BUFFER_INDEX);

            *count = CANFD_DlcToSize(CANFD_rxBuffer.r1_f->dlc);
        }
    }

    return (status);
}


/*******************************************************************************
* Function Name: CANFD_CanfdCyBtldrCommWrite
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
*   Returns CY_DFU_SUCCESS if no problem was encountered or returns the value
*   that best describes the problem. For more information refer to the
*   \ref cy_en_dfu_status_t description in the API Reference Guide.
*
*******************************************************************************/
cy_en_dfu_status_t CANFD_CanfdCyBtldrCommWrite(const uint8_t pData[], uint32_t size, uint32_t* count, uint32_t timeout)
{
    cy_en_dfu_status_t dfuStatus  = CY_DFU_ERROR_BAD_PARAM;
    (void)timeout;

    if ((pData != NULL) && (count != NULL) && (size > 0U))
    {

        DFU_CANFD_txBuffer_0.t1_f->dlc = CANFD_SizeToDlc(size);
        DFU_CANFD_txBuffer_0.data_area_f = (uint32_t*)pData;

        if (CY_CANFD_SUCCESS == Cy_CANFD_UpdateAndTransmitMsgBuffer(CY_DFU_CANFD_HW,
                                                                    CY_DFU_CANFD_CH_NUM,
                                                                    &DFU_CANFD_txBuffer_0,
                                                                    CANFD_TX_BUFFER_INDEX,
                                                                    &CY_DFU_CANFD_CONTEXT))
        {
            *count = CANFD_DlcToSize(DFU_CANFD_txBuffer_0.t1_f->dlc);
            dfuStatus = CY_DFU_SUCCESS;
        }
        else
        {
            dfuStatus = CY_DFU_ERROR_UNKNOWN;
        }
    }

    return (dfuStatus);
}


/*******************************************************************************
* Function Name: CANFD_DlcToSize
****************************************************************************//**
*
*  Converts the Data Length Code of the CAN message to a value in bytes.
*
*  \param dlc: Data length code.
*
*  \return
*   Returns the actual size of the data field of the CAN message in bytes.
*
*******************************************************************************/
static uint32_t CANFD_DlcToSize(uint32_t dlc)
{
    uint32_t size;

    if (dlc < CY_CANFD_CLASSIC_CAN_DATA_LENGTH)
    {
        size = dlc;
    }
    else
    {
        size = sizeLookupTable[(dlc - CY_CANFD_CLASSIC_CAN_DATA_LENGTH)];
    }

    return size;
}


/*******************************************************************************
* Function Name: CANFD_SizeToDlc
****************************************************************************//**
*
*  Converts the size of data in bytes to a Data Length Code of the CAN message.
*
*  \param size: Size in bytes.
*
*  \return
*   Returns the corresponding Data Length Code of the CAN message.
*
*******************************************************************************/
static uint32_t CANFD_SizeToDlc(uint32_t size)
{
    uint32_t dlc;

    if (size > 48U)
    {
        dlc = 15U;
    }
    else if (size > 32U)
    {
        dlc = 14U;
    }
    else if (size > 24U)
    {
        dlc = 13U;
    }
    else if (size > 20U)
    {
        dlc = 12U;
    }
    else if (size > 16U)
    {
        dlc = 11U;
    }
    else if (size > 12U)
    {
        dlc = 10U;
    }
    else if (size > 8U)
    {
        dlc = 9U;
    }
    else
    {
        dlc = size;
    }

    return dlc;
}

/* [] END OF FILE */
