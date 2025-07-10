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

#ifndef OSAL_QUEUE_H
#define OSAL_QUEUE_H

#include <stdint.h>
#include "osal_define.h"

/**
 * @addtogroup OSAL_QUEUE
 * OSAL Queue API and definitions
 * @ingroup OSAL
 * @{
 */

typedef struct OsalQueue_st OsalQueue;

/**
 * @brief Create a queue
 *
 * @param queue The queue to be created
 * @param num The number of item
 * @param item_size The size of item
 * @return int OSAL_SUCCESS for success, others for failure
 */
int osal_create_queue(OsalQueue **queue, int num, int item_size);

/**
 * @brief Delete a queue
 *
 * @param queue The queue being deleted
 * @return int OSAL_SUCCESS for success, others for failure
 */
int osal_delete_queue(OsalQueue *queue);

/**
 * @brief Send queue
 *
 * @param queue The queue to be sent
 * @param data The pointer of send data
 * @param timeout The maximum amount of wait time
 * @return int OSAL_SUCCESS for success, others for failure
 */
int osal_send_queue(OsalQueue *queue, void *data, uint32_t timeout);

/**
 * @brief Send queue from ISR
 *
 * @param queue The queue to be sent
 * @param data The pointer of send data
 * @return int OSAL_SUCCESS for success, others for failure
 */
int osal_send_queue_isr(OsalQueue *queue, void *data);

/**
 * @brief Receive queue
 *
 * @param queue The queue to be received
 * @param data The pointer of receive data
 * @param timeout The maximum amount of wait time
 * @return int OSAL_SUCCESS for success, others for failure
 */
int osal_receive_queue(OsalQueue *queue, void *data, uint32_t timeout);

/** @} */

#endif // OSAL_QUEUE_H
