/***************************************************************************//**
* \file cy_dfu_logging.h
*
* Provides API for DFU logging.
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