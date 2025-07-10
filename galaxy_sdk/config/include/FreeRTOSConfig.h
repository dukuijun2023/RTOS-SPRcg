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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "soc_sysctl.h"

/**
 * @addtogroup FREERTOS_CONFIG
 * FreeRTOS Config
 * @{
 */

#define USER_MODE_TASKS 0

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configSYSTICK_CLOCK_HZ                  soc_rtc_clock_get_freq()
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      soc_cpu_clock_get_freq()
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP   20
#define configIDLE_TASK_NAME                    "sys_idle"
#define configTICK_RATE_HZ                      ((TickType_t)100L)
#define configMAX_PRIORITIES                    8
#define configMINIMAL_STACK_SIZE                ((size_t)256)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0 /* 64 bits */
#define configCLI_BAUD_RATE                     115200
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_QUEUE_SETS                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION  1
#define configSUPPORT_DYNAMIC_ALLOCATION 1
/** Total cacheable heap size, in byte */
#define configTOTAL_HEAP_SIZE ((size_t)(53 * 1024))
/** Total non cacheable heap size, in byte */
#define configTOTAL_NON_CACHEABLE_HEAP_SIZE ((size_t)(32 * 1024))
#define configAPPLICATION_ALLOCATED_HEAP    1

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                1
#define configUSE_TICK_HOOK                0
#define configCHECK_FOR_STACK_OVERFLOW     1
#define configUSE_MALLOC_FAILED_HOOK       1
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS        1
#define configUSE_TRACE_FACILITY             1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#if configGENERATE_RUN_TIME_STATS
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() \
    do {                                         \
    } while (0) /* we use the timer */
#define portALT_GET_RUN_TIME_COUNTER_VALUE(dest) \
    {                                            \
        dest = xTickCount;                       \
    }
#endif
#if 0
#define configUSE_STATS_FORMATTING_FUNCTIONS     1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE()         ulGetRunTimeCounterValue()
#endif

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES 2

/* Software timer related definitions. */
#define configUSE_TIMERS              1
#define configTIMER_TASK_PRIORITY     7
#define configTIMER_QUEUE_LENGTH      15
#define configTIMER_TASK_STACK_DEPTH  configMINIMAL_STACK_SIZE
#define configTIMER_SERVICE_TASK_NAME "sys_timer"

#define configKERNEL_INTERRUPT_PRIORITY      0
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 7

/* Define to trap errors during development. */
#define configASSERT(x)           \
    if ((x) == 0) {               \
        taskDISABLE_INTERRUPTS(); \
        for (;;)                  \
            ;                     \
    }

/* FreeRTOS MPU specific definitions. */
// #define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* FreeRTOS CLI definitions */
#ifdef OS_PLUS_CLI_ENA
#define configCOMMAND_INT_MAX_OUTPUT_SIZE 2048
#endif

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_xResumeFromISR              1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetIdleTaskHandle      1
#define INCLUDE_eTaskGetState               0
#define INCLUDE_xEventGroupSetBitFromISR    1
#define INCLUDE_xTimerPendFunctionCall      1
#define INCLUDE_xTaskAbortDelay             0
#define INCLUDE_xTaskGetHandle              1
#define INCLUDE_xTaskResumeFromISR          1

/* A header file that defines trace macro can be included here. */

/** @} */

#endif // FREERTOS_CONFIG_H
