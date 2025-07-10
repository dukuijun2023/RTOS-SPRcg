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

#ifndef OSAL_SPINLOCK_H
#define OSAL_SPINLOCK_H

#include <stdint.h>
#include "osal_define.h"

/**
 * @addtogroup OSAL_SPINLOCK
 * OSAL Spinlock API and definitions
 * @ingroup OSAL
 * @{
 */

typedef struct OsalSpinlock_st OsalSpinlock;

/**
 * @brief Create a spinlock
 *
 * @param spinlock The spinlock being created
 * @return int OSAL_SUCCESS for success, others for failure
 */
int osal_create_spinlock(OsalSpinlock **spinlock);

/**
 * @brief Delete a spinlock
 *
 * @param spinlock The spinlock being deleted
 * @return int OSAL_SUCCESS for success, others for failure
 */
int osal_delete_spinlock(OsalSpinlock *spinlock);

/**
 * @brief Unlock spinlock
 *
 * @param spinlock The spinlock to be unlocked
 * @return int OSAL_SUCCESS for success, others for failure
 */
int osal_unlock_spinlock(OsalSpinlock *spinlock);

/**
 * @brief Lock spinlock
 *
 * @param spinlock The spinlock to be locked
 * @return int OSAL_SUCCESS for success, others for failure
 */
int osal_lock_spinlock(OsalSpinlock *spinlock);

/** @} */

#endif // OSAL_SPINLOCK_H
