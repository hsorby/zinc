/**
 * FILE : timekeeper.h
 *
 * The public interface of timekeeper which maintains a current time and manages
 * several timenotifiers.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_TIMEKEEPER_H__
#define CMZN_TIMEKEEPER_H__

#include "types/timenotifierid.h"
#include "types/timekeeperid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Access the timekeeper, increase the access count of the time keeper by one.
 *
 * @param timekeeper  handle to the "to be access" zinc timekeeper.
 * @return  handle to timekeeper if successfully access timekeeper.
 */
ZINC_API cmzn_timekeeper_id cmzn_timekeeper_access(cmzn_timekeeper_id timekeeper);

/**
 * Destroys this reference to the time keeper (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param timekeeper_address  The address to the handle to time keeper
 * @return  Status CMZN_OK if successfully destroy the time keeper,
 * any other value on failure.
 */
ZINC_API int cmzn_timekeeper_destroy(cmzn_timekeeper_id *timekeeper_address);

/**
 * Create and returns a time notifier with regular update time in time keeper.
 * The returned time notifier will automatically be added to the time keeper.
 *
 * @param update_frequency  The number of times which time notifier will receive
 *    callback per unit of time in the time keeper.
 * @param time_offset  This value will set the exact time the notification
 *    happenes and allow setting the callback time other than t=0.
 *    Time notifier will receive/send out notification when
 *    time_offset + original callback time is reached.
 * @return  The time notifier if successfully create a time notifier otherwise
 *    NULL.
 */
ZINC_API cmzn_timenotifier_id cmzn_timekeeper_create_timenotifier_regular(
	cmzn_timekeeper_id timekeeper, double update_frequency, double time_offset);

/**
 * Gets the maximum time in the timekeeper.
 *
 * @param timekeeper  The timekeeper to query.
 * @return  Current time or 0 if invalid argument.
 */
ZINC_API double cmzn_timekeeper_get_maximum_time(cmzn_timekeeper_id timekeeper);

/**
 * Sets the maximum time in the timekeeper.
 *
 * @param timenotifier  The timekeeper to modify.
 * @param maximum_time  The new maximum time.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_timekeeper_set_maximum_time(cmzn_timekeeper_id timekeeper, double maximum_time);

/**
 * Gets the minimum time in the timekeeper.
 *
 * @param timekeeper  The timekeeper to query.
 * @return  Current time or 0 if invalid argument.
 */
ZINC_API double cmzn_timekeeper_get_minimum_time(cmzn_timekeeper_id timekeeper);

/**
 * Sets the minimum time in the timekeeper.
 *
 * @param timenotifier  The timekeeper to modify.
 * @param minimum_time  The new minimum time.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_timekeeper_set_minimum_time(cmzn_timekeeper_id timekeeper, double minimum_time);

/**
 * Gets the current time from the timekeeper.
 *
 * @param timekeeper  The timekeeper to query.
 * @return  Current time or 0 if invalid argument.
 */
ZINC_API double cmzn_timekeeper_get_time(cmzn_timekeeper_id timekeeper);

/**
 * Sets the current time in the timekeeper. Timenotifiers are informed of the
 * time change.
 *
 * @param timenotifier  The timekeeper to modify.
 * @param time  The new current time.
 * @return  CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_timekeeper_set_time(cmzn_timekeeper_id timekeeper, double time);

#ifdef __cplusplus
}
#endif

#endif
