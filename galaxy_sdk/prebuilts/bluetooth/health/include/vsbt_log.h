/** @file
 *  @brief Bluetooth subsystem logging helpers.
 */

/*
 * Copyright (C) 2025. VeriSilicon Holdings Co., Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __VSBT_LOG_H
#define __VSBT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum VSBT_LOG_LEVEL {
    VSBT_LOG_LEVEL_NONE,
    VSBT_LOG_LEVEL_ERR,
    VSBT_LOG_LEVEL_WRN,
    VSBT_LOG_LEVEL_INF,
    VSBT_LOG_LEVEL_DBG,
} VSBT_LOG_LEVEL;

/**
 * @brief Print log
 * @param lvl logging level
 * @param format C string that contains the text to be written
 * @return On success, -1 if lvl is less than the set logging level.
 */
int vsbt_log_printf(VSBT_LOG_LEVEL log_lvl, const char *format, ...);

/**
 * @brief Print the name of the currently running task.
 * @param lvl logging level
 */
void vsbt_log_show_task(VSBT_LOG_LEVEL log_lvl);

#ifdef __cplusplus
}
#endif

#endif
