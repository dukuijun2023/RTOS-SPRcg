/*
 * Copyright (c) 2025, VeriSilicon Holdings Co., Ltd. All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OSAL_TASK_H
#define OSAL_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "osal_define.h"

/** @addtogroup OSAL_TASK
 *  OSAL Task API and definitions
 *  @ingroup OSAL
 *  @{
 */

typedef void (*TaskHookCb)(void);

/**
 * @brief Create a task
 *
 * @param func A pointer to the function that implements the task
 * @param name A descriptive name for the task
 * @param stack_size The number of words the stack can hold, not the number of
 * bytes
 * @param task_priority The priority at which the task will execute
 * @param param A parameter for task function
 * @return void* Task handle for success, NULL for failure
 */
void *osal_create_task(void *func, char *name, uint32_t stack_size, uint32_t task_priority,
                       void *param);

/**
 * @brief Delete the specific task
 *
 * @param task Task handle
 */
void osal_delete_task(void *task);

/**
 * @brief Suspend a task
 *
 * @param task Task handle
 */
void osal_suspend_task(void *task);

/**
 * @brief Suspend the scheduler
 *
 * @note Suspending the scheduler prevents a context switch from occurring, but
 * leaves interrupts enabled
 * @note For FreeRTOS, API functions must not be called while the scheduler is
 * suspended
 */
void osal_suspend_all(void);

/**
 * @brief Resume a task
 *
 * @param task Task handle
 */
void osal_resume_task(void *task);

/**
 * @brief Resume the scheduler
 */
void osal_resume_all(void);

/**
 * @brief Place the calling task into the sleep(aka blocked) state until the
 * sleep period has expired
 *
 * @param ms Sleep period, in miniseconds
 */
void osal_sleep(int32_t ms);

/**
 * @brief do something before start scheduler
 */
void osal_pre_start_scheduler(void);

/**
 * @brief Start the scheduler
 */
void osal_start_scheduler(void);

/**
 * @brief End the scheduler
 */
void osal_end_scheduler(void);

/**
 * @brief Get the state of scheduler
 *
 * @return true Started
 * @return false Stopped
 */
bool osal_started(void);

/**
 * @brief Enter critical sections
 */
void osal_enter_critical(void);

/**
 * @brief Exit critical sections
 */
void osal_exit_critical(void);

/**
 * @brief Get os highest priority
 *
 * @return the highest priority of os
 */
int osal_highest_priority(void);

/**
 * @brief Set hook function for idle task
 */
void osal_set_idle_hook(TaskHookCb cb);

/** @} */

#endif // OSAL_TASK_H
