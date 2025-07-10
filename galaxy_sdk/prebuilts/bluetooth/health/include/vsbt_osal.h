/*-
 * Copyright (C) 2025. VeriSilicon Holdings Co., Ltd. All rights reserved.
 * Author: Hao Zhang.
 * Date: 2025-3-18
 *
 * This is a VSBT osal interface definition.
 */

#ifndef _VSBT_OSAL_H_
#define _VSBT_OSAL_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#define VSBT_OSAL_INVALID_HANDLE (0)

int vsbt_osal_init(int id);

typedef long unsigned int vsbt_sem_t;

/**
 * @brief Create semaphore.
 *
 * Creates a new counting semaphore instance, and returns a handle by which the
 * new counting semaphore can be referenced.
 *
 * @param maxCnt The maximum count value that can be reached.  When the
 *        semaphore reaches this value it can no longer be 'given'.
 *
 * @param initCnt The count value assigned to the semaphore when it is
 *        created.
 *
 * @return Handle to the created semaphore. VSBT_OSAL_INVALID_HANDLE if the semaphore
 *         could not be created.
 */
vsbt_sem_t vsbt_sem_create(uint32_t maxCnt, uint32_t initCnt);

/**
 * @brief Release a semaphore.
 *
 * The semaphore must have previously been created.
 *
 * This macro will determine if it is used from an ISR.
 *
 * @param sem A handle to the semaphore being released.  This is the
 * handle returned when the semaphore was created.
 *
 * @return 0 if the semaphore was released. (negative) error code if an error occurred.
 */
int vsbt_sem_give(vsbt_sem_t sem);

/**
 * @brief Obtain a semaphore.
 *
 * The semaphore must have previously been created.
 *
 * This macro will determine if it is used from an ISR.
 *
 * @param sem A handle to the semaphore being taken - obtained when
 * the semaphore was created.
 *
 * @param timeout The time in ms to wait for the semaphore to become
 * available.
 *
 * @return 0 if the semaphore was obtained.  (negative) error code
 * if timeout expired without the semaphore becoming available.
 */
int vsbt_sem_take(vsbt_sem_t sem, uint32_t timeout);

/**
 * @brief Delete a semaphore.
 *
 * The semaphore must have previously been created.
 *
 * @param sem A handle to the semaphore to be deleted.
 */
void vsbt_sem_delete(vsbt_sem_t sem);

/**
 * @brief Get a semaphore count.
 *
 * The semaphore must have previously been created.
 *
 * @param sem A handle to the semaphore being queried.
 *
 * @return current semaphore count.
 */
unsigned int vsbt_sem_count_get(vsbt_sem_t sem);

/**
 * @brief Resets a semaphore's count to zero.
 *
 * The semaphore must have previously been created.
 *
 * @param sem A handle to the semaphore being reset.
 *
 * @return The return value is now obsolete and is always set to 0.
 */
int vsbt_sem_reset(vsbt_sem_t sem);

typedef long unsigned int vsbt_mutex_t;

/**
 * @brief Create mutex.
 *
 * Creates a new mutex type semaphore instance, and returns a handle by which
 * the new mutex can be referenced.
 *
 * @return If the mutex was successfully created then a handle to the created
 * mutex is returned, otherwise VSBT_OSAL_INVALID_HANDLE.
 */
vsbt_mutex_t vsbt_mutex_create(void);

/**
 * @brief Lock a mutex.
 *
 * The mutex must have previously been created.
 *
 * This macro will determine if it is used from an ISR.
 *
 * @param mutex A handle to the mutex being locked.
 *
 * @param timeout The time in ms to wait for locking mutex.
 *
 * @return 0 if the mutex is locked. (negative) error code
 * if timeout expired without the mutex becoming locked.
 */
int vsbt_mutex_lock(vsbt_mutex_t mutex, uint32_t timeout);

/**
 * @brief Unlock a mutex.
 *
 * The mutex must have previously been created.
 *
 * This macro will determine if it is used from an ISR.
 *
 * @param mutex A handle to the mutex being locked.
 *
 * @return 0 if the mutex was unlocked. (negative) error code if an error occurred.
 */
int vsbt_mutex_unlock(vsbt_mutex_t mutex);
//
typedef long unsigned int vsbt_timer_t;
typedef void (*vsbt_timer_cb_t)(vsbt_timer_t timer, void *priv_data);

/**
 * @brief Create a timer.
 *
 * Creates a new software timer instance, and returns a handle by which the
 * created software timer can be referenced.
 *
 * @param period_ms The timer period.
 *
 * @param reload If reload is set to true then the timer will
 * expire repeatedly with a frequency set by the ticks parameter.
 * If reload is set to false then the timer will be a one-shot timer and
 * enter the dormant state after it expires.
 *
 * @param priv_data Parameter of the timer callback function.
 *
 * @param callback The function to call when the timer expires.
 * Callback functions must have the prototype defined by vsbt_timer_cb_t,
 * which is "void (*vsbt_timer_cb_t)(vsbt_timer_t timer, void *priv_data);".
 *
 * @return If the timer is successfully created then a handle to the newly
 * created timer is returned.
 * If the timer could not be created then VSBT_OSAL_INVALID_HANDLE.
 * If the created timer exceeds TIMER_MAX_ENTRY then -ENOMEM.
 */
vsbt_timer_t vsbt_timer_create(const uint32_t period_ms, const bool reload, void *const priv_data,
                               vsbt_timer_cb_t callback);

/**
 * @brief Start a timer.
 *
 * vsbt_timer_start() starts a timer that was previously created using the
 * vsbt_timer_create() API function.  If the timer had already been started and was
 * already in the active state, then vsbt_timer_start() has equivalent functionality
 * to the vsbt_timer_reset() API function.
 *
 * Starting a timer ensures the timer is in the active state.  If the timer
 * is not stopped, deleted, or reset in the mean time, the callback function
 * associated with the timer will get called 'n' ticks after vsbt_timer_start() was
 * called, where 'n' is the timers defined period.
 *
 * This macro will determine if it is used from an ISR.
 *
 * @param timer The handle of the timer being started/restarted.
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int vsbt_timer_start(vsbt_timer_t timer);

/**
 * @brief Stop a timer.
 *
 * vsbt_timer_stop() stops a timer that was previously started using either of the
 * The vsbt_timer_start(), vsbt_timer_reset(), vsbt_timer_change() API functions.
 *
 * Stopping a timer ensures the timer is not in the active state.
 *
 * This macro will determine if it is used from an ISR.
 *
 * @param timer The handle of the timer being stopped.
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int vsbt_timer_stop(vsbt_timer_t timer);

/**
 * @brief Delete a timer.
 *
 * vsbt_timer_delete() deletes a timer that was previously created using the
 * vsbt_timer_start() API function.
 *
 * @param timer The handle of the timer being deleted.
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int vsbt_timer_delete(vsbt_timer_t timer);

/**
 * @brief Reset a timer.
 *
 * vsbt_timer_reset() re-starts a timer that was previously created using the
 * vsbt_timer_start() API function.  If the timer had already been started and was
 * already in the active state, then vsbt_timer_reset() will cause the timer to
 * re-evaluate its expiry time so that it is relative to when vsbt_timer_reset() was
 * called.  If the timer was in the dormant state then vsbt_timer_reset() has
 * equivalent functionality to the vsbt_timer_start() API function.
 *
 * Resetting a timer ensures the timer is in the active state.  If the timer
 * is not stopped, deleted, or reset in the mean time, the callback function
 * associated with the timer will get called 'n' ticks after xTimerStart() was
 * called, where 'n' is the timers defined period.
 *
 * This macro will determine if it is used from an ISR.
 *
 * @param timer The handle of the timer being reset/started/restarted.
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int vsbt_timer_reset(vsbt_timer_t timer);

/**
 * @brief Change the period of timer.
 *
 * vsbt_timer_change() changes the period of a timer that was previously
 * created using the vsbt_timer_start() API function.
 *
 * vsbt_timer_change() can be called to change the period of an active or
 * dormant state timer.
 *
 * This macro will determine if it is used from an ISR.
 *
 * @param timer The handle of the timer that is having its period changed.
 *
 * @param period_ms The new period for timer.
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int vsbt_timer_change(vsbt_timer_t timer, const uint32_t period_ms);

/**
 * @brief Get the remaining time(in ms) of timer.
 *
 * vsbt_timer_get_remain_ms() get the remaining time(in ms) of a timer that was previously
 * created using the vsbt_timer_start() API function.
 *
 * @param timer The handle of the timer being queried.
 *
 * @return If the timer is running then the time in ms before the timer expires.
 * If the timer is not running then the return value is undefined.
 */
int vsbt_timer_get_remain_ms(vsbt_timer_t timer);

typedef long unsigned int vsbt_task_t;
typedef void (*vsbt_task_entry_t)(void *param);

/**
 * @brief Create a new task.
 *
 * Create a new task and add it to the list of tasks that are ready to run.
 *
 * @param entry Pointer to the task entry function.
 *
 * @param stack_size The size of the task stack specified as the number of
 * bytes the stack can hold.
 *
 * @param param Pointer that will be used as the parameter for the task
 * being created.
 *
 * @param priority The priority at which the task should run.
 *
 * @return Handle to the created task if the task was successfully created and added to a ready
 * list, otherwise VSBT_OSAL_INVALID_HANDLE.
 */
vsbt_task_t vsbt_task_create(vsbt_task_entry_t entry, void *param, int stack_size, int priority);

/**
 * @brief Set the name of a task.
 *
 * @param task The handle of the task to be set.
 *
 * @param name The name that is expected to be set.
 */
void vsbt_task_set_name(vsbt_task_t task, const char *name);

/**
 * @brief Set the name of a task.
 *
 * Remove a task from the kernel's management. The task being
 * deleted will be removed from all ready, blocked, suspended and event lists.
 *
 * @param task The handle of the task to be deleted. Passing 0 will
 * cause the calling task to be deleted.
 */
void vsbt_task_delete(vsbt_task_t task);

/*
 * Return the handle of the calling task.
 */
vsbt_task_t vsbt_task_current_get(void);

/**
 * @brief Delay a task for a given number of millisecond.
 *
 * @param ms The amount of time, in millisecond periods, that
 * the calling task should block.
 */
void vsbt_task_sleep(const uint32_t ms);

/*
 * Forcing a context switch.
 */
void vsbt_task_yield(void);

/**
 * @brief Get the count of system timer.
 *
 * @return The count of system timer(in ms).
*/
uint64_t vsbt_uptime_get_ms(void);

#endif
