/*
 * radar_clutter_localization_internal.h
 *
 * Description: This header file contains the internal function declarations for clutter localization.
 * The internal files are not intended for public use and should only be included by other internal files, e.g. for unit
 * tests.
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

#ifndef RADAR_CLUTTER_LOCALIZATION_INTERNAL_H
#define RADAR_CLUTTER_LOCALIZATION_INTERNAL_H

#include <stddef.h>
#include "radar_clutter_localization.h"


/** Update clutter localization registers
 * This function updates the radar sensor registers with the current exclusion zones if the zone marker values are above
 * a certain threshold.
 * @param clutter Pointer to the clutter localization instance
 * @param zone_marker Zone marker array for the current detection
 * @param range Range index of the detection
 * @param angle Angle index of the detection
 * @return true if clutter has been detected, false if no clutter has been detected
 */
bool clutter_localization_update_registers(clutter_localization_t* clutter, zone_marker_array_t zone_marker,
                                           int8_t range, int8_t angle);

/** Update clutter localization markers value
 * This function updates the clutter localization markers value based on the current detection.
 * @param clutter Pointer to the clutter localization instance
 * @param zone_marker Zone marker array for the current detection
 * @param temp_zone_marker Temporary zone marker array for the current detection
 * @param range Range index of the detection
 * @param angle Angle index of the detection
 *
 * @return true if clutter has been detected, false if no clutter has been detected
 */
bool clutter_localization_update_markers_value(clutter_localization_t* clutter, zone_marker_array_t zone_marker,
                                               zone_marker_array_t temp_zone_marker, int8_t range, int8_t angle);

/** Update clutter localization markers
 * This function updates the clutter localization markers based on the current detection.
 * @param clutter Pointer to the clutter localization instance
 * @param temp_zone_marker_e Temporary zone marker array for the elevation plane
 * @param temp_zone_marker_h Temporary zone marker array for the horizontal plane
 * @return true if clutter has been detected, false if no clutter has been detected
 */
bool clutter_localization_update_markers(clutter_localization_t* clutter, zone_marker_array_t temp_zone_marker_e,
                                         zone_marker_array_t temp_zone_marker_h);

/** Set neighboring zones
 * This function sets the neighboring zones for the clutter localization instance.
 * @param clutter Pointer to the clutter localization instance
 * @param zone_marker Zone marker array for the current detection
 * @param zone_marker_neighboring Zone marker array for the neighboring zones
 * @return 0 on success, negative error code on failure
 */
int32_t clutter_localization_set_neighboring_zones(clutter_localization_t* clutter, zone_marker_array_t zone_marker,
                                                   zone_marker_array_t zone_marker_neighboring);

/** Map zones to registers
 * This function maps the clutter localization zones to the corresponding radar sensor registers.
 * @param clutter Pointer to the clutter localization instance
 * @return 0 on success, negative error code on failure
 */
int32_t clutter_localization_map_zones_to_registers(clutter_localization_t* clutter);
#endif // RADAR_CLUTTER_LOCALIZATION_INTERNAL_H
