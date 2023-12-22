/***************************************************************************//**
* \file cy_dfu_logging.h
* \version 5.1
*
* Provides API for DFU logging.
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

#if !defined(CY_DFU_LOGGING_H)
#define CY_DFU_LOGGING_H

#include <stdio.h>
#include "dfu_user.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
* \addtogroup group_dfu_macro
* \{
*/

/**
* \defgroup group_dfu_macro_log DFU Log Levels
* \{
* The DFU logging levels
*/
#define CY_DFU_LOG_LEVEL_OFF      0 /**< Logging is disabled (default) */
#define CY_DFU_LOG_LEVEL_ERROR    1 /**< Logs only error messages */
#define CY_DFU_LOG_LEVEL_WARNING  2 /**< Logs only error, warning messages */
#define CY_DFU_LOG_LEVEL_INFO     3 /**< Logs error, warning, and info messages */
#define CY_DFU_LOG_LEVEL_DEBUG    4 /**< Logs all messages (including debug) */
/** \} group_dfu_macro_log */

/** \} group_dfu_macro */

#ifdef CY_DFU_CUSTOM_LOG
    #ifndef CY_DFU_LOG_BUF
        #define     CY_DFU_LOG_BUF      (60U)
    #endif /* CY_DFU_LOG_BUF */
    extern char cy_dfu_msg[CY_DFU_LOG_BUF];
    void Cy_DFU_Log(const char* msg);
#endif /* CY_DFU_CUSTOM_LOG */

#ifdef CY_DFU_CUSTOM_LOG
    #define CY_DFU_LOG_WRITE(_fmt, ...)                 \
        do                                              \
        {                                               \
            sprintf(cy_dfu_msg, _fmt, ##__VA_ARGS__);   \
            Cy_DFU_Log(cy_dfu_msg);                     \
        } while (false)
#else
    #define CY_DFU_LOG_WRITE(_fmt, ...)    \
        do                                 \
        {                                  \
            (void) printf(_fmt, ##__VA_ARGS__);   \
        } while (false)
#endif /* CY_DFU_CUSTOM_LOG */


#if CY_DFU_LOG_LEVEL >= CY_DFU_LOG_LEVEL_ERROR
    #define CY_DFU_LOG_ERR(_fmt, ...)                               \
        CY_DFU_LOG_WRITE("[DFU_ERR] " _fmt "\n\r", ##__VA_ARGS__)
#else
    #define CY_DFU_LOG_ERR(...)
#endif /* CY_DFU_LOG_LEVEL >= CY_DFU_LOG_LEVEL_ERROR */

#if CY_DFU_LOG_LEVEL >= CY_DFU_LOG_LEVEL_WARNING
    #define CY_DFU_LOG_WRN(_fmt, ...)                               \
        CY_DFU_LOG_WRITE("[DFU_WRN] " _fmt "\n\r", ##__VA_ARGS__)
#else
    #define CY_DFU_LOG_WRN(...)
#endif /* CY_DFU_LOG_LEVEL >= CY_DFU_LOG_LEVEL_WARNING */

#if CY_DFU_LOG_LEVEL >= CY_DFU_LOG_LEVEL_INFO
    #define CY_DFU_LOG_INF(_fmt, ...)                               \
        CY_DFU_LOG_WRITE("[DFU_INF] " _fmt "\n\r", ##__VA_ARGS__)
#else
    #define CY_DFU_LOG_INF(...)
#endif /* CY_DFU_LOG_LEVEL >= CY_DFU_LOG_LEVEL_INFO */

#if CY_DFU_LOG_LEVEL >= CY_DFU_LOG_LEVEL_DEBUG
    #define CY_DFU_LOG_DBG(_fmt, ...)                               \
        CY_DFU_LOG_WRITE("[DFU_DBG] " _fmt "\n\r", ##__VA_ARGS__)
#else
    #define CY_DFU_LOG_DBG(...)
#endif /* CY_DFU_LOG_LEVEL >= CY_DFU_LOG_LEVEL_DEBUG */


#ifdef __cplusplus
}
#endif

#endif /* CY_DFU_LOGGING_H */