/*
 * radar_clutter_localization.c
 *
 * Description: This source file contains the implementations for clutter localization functions used in all code
 * examples.
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

#ifndef RADAR_CLUTTER_LOCALIZATION_H
#define RADAR_CLUTTER_LOCALIZATION_H

#include <stddef.h>
#include "radar_motion_detection.h"

/*******************************************************************************
 * Data Structures
 ********************************************************************************/

/** Number of range zones for clutter localization */
#define CLUTTER_LOCALIZATION_NUM_RANGE_ZONES 16
/** Number of angle zones for clutter localization */
#define CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES 7
/** value that defines that a zone must be excluded */
#define CLUTTER_LOCALIZATION_EXCLUSION_VALUE 1

/** Clutter localization configuration structure*/
typedef struct
{
    /** Doppler gating speed in meters per second */
    float doppler_gating_speed_mps;
    /** Increment counter value for clutter localization detection */
    uint8_t increment_count;
    /** Decrement counter value for clutter localization detection */
    uint8_t decrement_count;
    /** Saturation limit for clutter localization detection */
    uint8_t saturation_limit;
    /** Settings threshold for clutter localization detection */
    uint8_t settings_threshold;
    /** Priority for horizontal plane clutter localization
     * if set to 1, the h-plane has priority over the e-plane when updating the registers
     */
    uint8_t priority_h_plane;
    /** number of neighboring zones to mask */
    uint8_t num_neighboring_zones;
    /** Timer in ticks when clutter masking is periodically restarted. A tick is derived from the seq2_done interrupt*/
    uint16_t period_ticks;
    /** Maximum duration in ticks for which the clutter masking is active. A tick is derived from the seq2_done
       interrupt*/
    uint16_t max_duration_ticks;
    /** Enable or disable automatic masking based on presence detection */
    bool auto_masking_active;
    /** deactivates the clutter detection in case of consecutive rd event without a target */
    uint16_t deactivation_counter_threshold_ticks;
    /** maximum range value */
    float range_max;
} clutter_localization_config_t;

/** Zone marker array type */
typedef int8_t zone_marker_array_t[CLUTTER_LOCALIZATION_NUM_RANGE_ZONES][CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES];

/** defines the states for clutter localization */
typedef enum
{
    /** Idle state */
    CLUTTER_LOCALIZATION_STATE_IDLE,
    /** Zones are written to the sensor */
    CLUTTER_LOCALIZATION_STATE_ZONES_SET,
    /** Zones are reset on the sensor */
    CLUTTER_LOCALIZATION_STATE_ZONES_RESET
} clutter_localization_state_t;

/** Clutter localization instance structure */
typedef struct
{
    /** Pointer to the clutter localization configuration */
    clutter_localization_config_t* config;
    /** value for exclusion zone register 0 */
    uint32_t MD_ZONE_EXCL0;
    /** value for exclusion zone register 1 */
    uint32_t MD_ZONE_EXCL1;
    /** value for exclusion zone register 2 */
    uint32_t MD_ZONE_EXCL2;
    /** value for exclusion zone register 3 */
    uint32_t MD_ZONE_EXCL3;
    /** value for exclusion zone control register */
    uint32_t MD_ZONE_CTRL;

    /** Zone marker array for elevation plane, each element represents the number of detections in a range-angle bin */
    zone_marker_array_t zone_marker_e;
    /** Zone marker array for horizontal plane, each element represents the number of detections in a range-angle bin */
    zone_marker_array_t zone_marker_h;
    /** Zone marker array for elevation plane with marked neighboring zones, each element represents the number of
       detections in a range-angle bin */
    zone_marker_array_t zone_marker_e_neighboring;
    /** Zone marker array for horizontal plane with marked neighboring zones, each element represents the number of
       detections in a range-angle bin */
    zone_marker_array_t zone_marker_h_neighboring;
    /** Counter that defines the period when the clutter localization is active again*/
    uint32_t period_counter_ticks;
    /** indicates the state of the exclusion zone registers */
    clutter_localization_state_t register_update_state;
} clutter_localization_t;


/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/

/** Initialize clutter localization
 * This function initializes the clutter localization instance with the provided configuration.
 * @param clutter Pointer to the clutter localization instance
 * @param config Pointer to the clutter localization configuration
 * @return 0 on success, negative error code on failure
 */
int32_t clutter_localization_init(clutter_localization_t* clutter, clutter_localization_config_t* config);

/** Process clutter localization
 * This function processes the motion detection data and updates the clutter localization instance.
 * It calculates the clutter localization and writes the exclusion zones to the radar sensor registers.
 * @param motion_detection Pointer to the motion detection instance
 * @param clutter Pointer to the clutter localization instance
 * @return 0 on success, negative error code on failure
 */
int32_t clutter_localization_process(motion_detection_t* motion_detection, clutter_localization_t* clutter);

#endif // RADAR_CLUTTER_LOCALIZATION_H
