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

#ifndef SYS_COMMON_H
#define SYS_COMMON_H

/**
 * @addtogroup PEGASUS_SYS_COMMON
 * System Common definition
 * @ingroup SOC_PEGASUS
 * @{
 */

/** noncritical functions section */
#define NONCRITICAL_SECTION __attribute__((section(".stext")))
/** critical functions section */
#define CRITICAL_SECTION __attribute__((section(".itext")))
/** driver ISR functions section */
#define DRV_ISR_SECTION __attribute__((section(".itext")))
/** driver critical functions section */
#define DRV_CRITICAL_SECTION __attribute__((section(".itext")))

/** noncritical data section */
#define NONCRITICAL_DATA __attribute__((section(".sdram_data")))
/** sensor data section */
#define SENSOR_DATA_SECTION __attribute__((section(".sensor_data")))
/** noncache data section */
#define NONCACHE_DATA_SECTION __attribute__((section(".noncache_data")))

/** 2KB Bootloader reserved data section, can be reused for data */
#define RSV_DATA_SECTION __attribute__((section(".rsv_data")))

#define NON_XIP_TEXT __attribute__((section(".nonxip_text")))
#define NON_XIP_DATA __attribute__((section(".sdram_data")))

/** @} */

#endif // SYS_COMMON_H
