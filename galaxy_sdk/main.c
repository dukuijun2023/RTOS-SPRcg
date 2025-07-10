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

#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include "stdint.h"
#include "vs_conf.h"
#include "soc_init.h"
#include "soc_sysctl.h"
#include "bsp.h"
#include "uart_printf.h"
#include "board.h"
#include "osal.h"
#include "vpi_error.h"
#include "main.h"
#include "osal_task_api.h"
#include "vpi_event.h"
#include "hal_pdm.h"
#include "vsd_error.h"
#include "algo.h"

// 设置双缓冲
#define BUF_SIZE 1024
//static int buf_pool[2][BUF_SIZE];
//static int battery = 100;//电量
 uint16_t pdm_buf[48000];//6秒的数据
static volatile int total_bytes = 0;
volatile uint8_t *current_pdm_write_buffer;
volatile uint8_t *current_pdm_read_buffer;
static bool buffer_full = false;
OsalSemaphore *sem;

static int speech_init_handle(EventManager mgr, EventId event_id, EventParam param){
	if (event_id == EVENT_AUD_PARAM) {
		    // 确保 param 不是 NULL
	        if (param == NULL) {
	            printf("[Audio] Error: Received NULL parameter for battery event in custom handler!\n");
	            return -1;
	        }
	        //uint8_t *tmp = (uint8_t*)param; // 将void*参数转换为实际类型
		    //printf("[Battery] speech: %d%%\n", battery_level);
	        printf("[Audio] speech: %%\n");
		    return 0;
	}
	return -1;
}

static int algo_handle(EventManager mgr, EventId event_id, EventParam param){
	if (event_id == EVENT_AUD_PARAM) {
		    // 确保 param 不是 NULL
	        if (param == NULL) {
	            printf("[Audio] Error: Received NULL parameter for battery event in custom handler!\n");
	            return -1;
	        }
	        uint16_t* raw_data = (uint16_t*)param;
	        //进行算法处理
	        dataloader(raw_data);
	        uint8_t *tmp = (uint8_t*)param; // 将void*参数转换为实际类型
	        int zanshi = 0;
		    //printf("[Battery] algo: %d%%\n", battery_level);
	        	printf("[Audio] algo: %d%d%d%d%d%d%d%d%d%d%%\n",
	        			tmp[0 + zanshi],tmp[1+ zanshi],tmp[2+ zanshi],tmp[3+ zanshi],tmp[4+ zanshi],tmp[5+ zanshi],tmp[6],tmp[7],tmp[8],tmp[9]);
	        	zanshi  = (zanshi + 8000) % 96000;
	        	printf("audio: %d\n", tmp[0 + zanshi]);
		    return 0;
	}
	return -1;
}
static void task_sample(void *param)
{
    int count = 0;

    while (count < 10) {
        count++;
        //uart_printf("Sample task count %d\r\n", count);
        osal_sleep(1000);
    }

    //uart_printf("Finish sample task!\r\n");
    osal_delete_task(NULL);
}

static void task_init_app(void *param)
{
    int ret;
    BoardDevice board_dev;

    ret = board_register(board_get_ops());
    ret = vsd_to_vpi(ret);
    if (ret != VPI_SUCCESS) {
        uart_printf("board register failed %d", ret);
        goto exit;
    }
    ret = board_init((void *)&board_dev);
    ret = vsd_to_vpi(ret);
    if (ret != VPI_SUCCESS) {
        uart_printf("board init failed %d", ret);
        goto exit;
    }
    if (board_dev.name) {
        uart_printf("Board: %s", board_dev.name);
    }

    uart_printf("Hello VeriHealthi!\r\n");

    osal_create_task(task_sample, "task_sample", 512, 4, NULL);
exit:
    osal_delete_task(NULL);
}

static void pdm_irq_handler(const PdmDevice *pdm, int size, void *cb_ctx)
{
	//PdmSubstream *s = (PdmSubstream *)cb_ctx;

	// 切换缓冲区
	// Switch buffers
	total_bytes = (total_bytes + size);
	//PdmSubstream *s = (PdmSubstream *)cb_ctx;
	// 通知任务
	if(total_bytes >= 96000){
		total_bytes %= 96000;//设置回零
		buffer_full  = true;//设置满标志
		printf("PDM缓冲区满了\n");
	}
	if(buffer_full){
		buffer_full = false;
		osal_sem_wait(sem, 1000);
		printf("pdm_irq_handler\n");
		vpi_event_notify(EVENT_AUD_PARAM, pdm_buf);
		osal_sem_post_isr(sem);
	}
	//printf("pdm_irq_handler Finish\n");

}
static void speech_task(void* param)//读取PDM数据
{
	EventManagerId COBT_SPEECH_MGR = EVENT_MGR_AUD;
	EventHandler custom_speech_handler = speech_init_handle;
	EventId id = EVENT_AUD_PARAM;

	//创建一个信号量
	osal_create_sem(&sem);
	//battery = 18;
	EventParam param_speech_mgr = (EventParam)pdm_buf;
	EventManager Speech_Manager = vpi_event_new_manager(COBT_SPEECH_MGR, custom_speech_handler);
		int result = speech_init_handle(Speech_Manager, id, param_speech_mgr);
		if(result < 0){
			goto exit;
		}
		    // 检查管理器是否成功创建
		if (Speech_Manager == NULL) {
		    uart_printf("Error: Failed to create custom event manager!\r\n");
		    goto exit;
		}

		vpi_event_register(id, Speech_Manager); // 将事件注册到新创建的管理器
		vpi_event_notify(id, param_speech_mgr); // 通知事件，这将触发 task_sys_mgr 中的 handle_battery_event


	//初始化PDM
	PdmDevice* PDMDevice = hal_pdm_get_device(0);
	if(hal_pdm_init(PDMDevice) != VSD_SUCCESS){
		uart_printf("Error: PDM initialization failed!\r\n");
		goto exit;
	}
	//int active_size = 0;

	PdmSubstream stream = {
	        .sample_rate = 8000,
	        .sample_width = 16,
	        .chan_num = 1,
	        .xfer_mode = XFER_MODE_INTR,
	        .buffer = { .base = (void*)pdm_buf, .size =  sizeof(pdm_buf),
	        			.pos = 0, .available = 0},
			.cb_context = &stream,
			.cb = pdm_irq_handler//调用的时候stream.cb(argument)

	    };
	printf("开始讲话!\n");
	stream.cb(PDMDevice, 8000, stream.cb_context);
	 // 启动 PDM
	//hal_pdm_start(PDMDevice, &stream);
	hal_pdm_start(PDMDevice, &stream);
	// 主循环
	while (1) {
		printf("循环进行缓冲!\n");
		vpi_event_listen(Speech_Manager);
		//osal_sleep(1500);

		//delay_ms(15000);
	}
	//hal_pdm_stop(PDMDevice, &stream);
	hal_pdm_finalize(PDMDevice);
exit:
	osal_delete_task(NULL);
}

static void algo_task(void* param)//进行算法处理
{
	EventManagerId COBT_ALGO_MGR = EVENT_MGR_AUD;
	EventHandler custom_algo_handler = algo_handle;
	EventId id = EVENT_AUD_PARAM;

	//battery = 18;
	EventParam param_algo_mgr = (EventParam)pdm_buf;
	EventManager Algo_Manager = vpi_event_new_manager(COBT_ALGO_MGR, custom_algo_handler);

	algo_handle(Algo_Manager, id, param_algo_mgr);
	vpi_event_register(id, Algo_Manager); // 将事件注册到新创建的管理器
	while (1) {
		vpi_event_listen(Algo_Manager);
		int ret = osal_sem_wait(sem, 1000);
		if(ret == OSAL_SUCCESS){
			//算法处理

		}
		osal_sem_post(sem);
		    // 事件处理程序会自动执行
	}
}

int main(void)
{
    int ret;

    ret = soc_init();
    ret = vsd_to_vpi(ret);
    if (ret != VPI_SUCCESS) {
        if (ret == VPI_ERR_LICENSE) {
            uart_printf("no sdk license!");
        } else {
            uart_printf("soc init error %d", ret);
        }
        goto exit;
    } else {
        uart_printf("soc init done");
    }
    osal_pre_start_scheduler();
    osal_create_task(task_init_app, "init_app", 512, 7, NULL);
    osal_create_task(speech_task, "speech_task", 512, 5, NULL);
    osal_create_task(algo_task, "algo_task", 512, 6, NULL);
    osal_start_scheduler();
exit:
    goto exit;
}
