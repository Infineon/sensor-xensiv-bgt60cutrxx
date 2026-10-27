/**
 * @file radar_fifo_read.h
 * @brief Header file for the XENSIV(TM) BGT60CUTRxx radar FIFO interface.
 *
 * This file provides functionality related to the XENSIV(TM) BGT60CUTRxx sensor,
 * related to FIFO data retrieval.
 *
 ********************************************************************************
 * Copyright (c) (2016-2022), Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef RADAR_FIFO_READ_H_
#define RADAR_FIFO_READ_H_

#include <stddef.h>
#include "xensiv_bgt60cutrxx.h"

/**
 * \defgroup radar_fifo_read Radar FIFO interface
 * @{
 */

/**
 * @brief Retrieves FIFO event status from the XENSIV(TM) BGT60CUTRxx sensor.
 *
 * This function checks the interrupt status register of the XENSIV(TM) BGT60CUTRxx sensor
 * to determine if any FIFO-related events have occurred, such as threshold exceeded,
 * underflow, or overflow.
 *
 * In case of underflow or overflow event, the corresponding event flag is cleared.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRxx device structure.
 * @param event Pointer to a variable where the FIFO event status will be stored.
 *
 * @return status code, XENSIV_BGT60CUTRXX_STATUS_OK on success.
 */
int32_t xensiv_bgt60cutrxx_get_fifo_event(xensiv_bgt60cutrxx_t* dev, xensiv_bgt60cutrxx_fifo_events_t* event);

/**
 * @brief Retrieves the current FIFO fill level.
 *
 * This function reads the FIFO status register to determine the number of samples currently stored in the FIFO.
 *
 * @note This function does not read the FIFO, it only checks its fill level.
 *
 * @param dev pointer to the radar device instance.
 * @param num_samples_available pointer to a variable where the FIFO level will be stored.
 *
 * @return status code, XENSIV_BGT60CUTRXX_STATUS_OK on success.
 */
int32_t xensiv_bgt60cutrxx_get_fifo_level(xensiv_bgt60cutrxx_t* dev, uint32_t* num_samples_available);

/**
 * @brief Reads the FIFO event and reports whether the FIFO is healthy.
 *
 * Underflow and overflow flags are cleared as a side effect. Rendering the event is the
 * caller's responsibility; this function performs no I/O beyond the register access.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRxx device structure.
 * @param event Pointer to a variable where the FIFO event will be stored.
 * @param frame_valid Set to false when the event means radar frame data cannot be trusted,
 *                    and also when the FIFO event could not be read.
 *
 * @return status code, XENSIV_BGT60CUTRXX_STATUS_OK on success,
 *         XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER if a required pointer argument is NULL.
 */
int32_t radar_fifo_check_integrity(xensiv_bgt60cutrxx_t* dev,
                                   xensiv_bgt60cutrxx_fifo_events_t* event,
                                   bool* frame_valid);

/**
 * @brief Reads radar samples from the FIFO of the XENSIV(TM) BGT60CUTRxx sensor.
 *
 * This function retrieves radar samples stored in the FIFO of the XENSIV(TM) BGT60CUTRxx sensor
 * into the provided frame buffer. Afterwards, it clears the FIFO threshold event flag.
 *
 * If the FIFO read fails, its status code is returned unchanged and the FIFO threshold event flag
 * is left pending.
 *
 * @note The caller must ensure the frame buffer has enough space to hold the requested number of radar samples.
 * @note The function assumes data production is active and the device is in the expected sequence state.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRxx device structure.
 * @param frame_buffer pointer to the frame buffer.
 * @param num_samples number of radar samples to read from FIFO.
 *
 * @return status code, XENSIV_BGT60CUTRXX_STATUS_OK on success,
 *         XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER if a required pointer argument is NULL.
 */
int32_t radar_fifo_read(xensiv_bgt60cutrxx_t* dev, uint16_t* frame_buffer, uint32_t num_samples);

/**@} end of group radar_fifo_read */

#endif /* RADAR_FIFO_READ_H_ */
