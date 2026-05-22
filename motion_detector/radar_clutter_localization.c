#include "radar_clutter_localization.h"
#include "radar_clutter_localization_internal.h"
#include <math.h>
#include <string.h>
#include "xensiv_bgt60cutrxx_regs.h"


bool clutter_localization_calculate(motion_detection_t* motion_detection, clutter_localization_t* clutter);

/** Sets the detected targets to zero which are located inside the exclusion zones */
int32_t clutter_localization_filter_during_recheck(motion_detection_t* motion_detection,
                                                   clutter_localization_t* clutter);

/** Resets the exclusion zones to their default values
 * The function resets the exclusion zones to their default values and writes the default exclusion zones from the
 * configuration to the radar sensor registers.
 *
 * @param motion_detection Pointer to the motion detection instance
 * @param clutter Pointer to the clutter localization instance which contains the default exclusion zones in its
 * configuration
 * @return 0 on success, negative error code on failure
 */
int32_t clutter_localization_reset_exclusion_zones(motion_detection_t* motion_detection,
                                                   clutter_localization_t* clutter)
{
    memset(clutter->zone_marker_e, 0, sizeof(clutter->zone_marker_e));
    memset(clutter->zone_marker_h, 0, sizeof(clutter->zone_marker_h));
    memset(clutter->zone_marker_e_neighboring, 0, sizeof(clutter->zone_marker_e_neighboring));
    memset(clutter->zone_marker_h_neighboring, 0, sizeof(clutter->zone_marker_h_neighboring));
    memset(&clutter->zone_registers, 0, sizeof(clutter->zone_registers));

    return clutter_localization_write_exclusion_zones(motion_detection, clutter);
}


int32_t clutter_localization_init(motion_detection_t* motion_detection, clutter_localization_t* clutter,
                                  clutter_localization_config_t* config)
{
    // Input validation
    if ((clutter == NULL) || (config == NULL) || (motion_detection == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }
    if ((config->increment_count == 0) ||
        (config->settings_threshold == 0) ||
        (config->settings_threshold > config->saturation_limit))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }
    const size_t zone_marker_element_size = sizeof(zone_marker_array_t) /
                                            (CLUTTER_LOCALIZATION_NUM_RANGE_ZONES *
                                             CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES);
    const size_t zone_marker_max = (1u << (zone_marker_element_size * 8 - 1)) - 1;
    if ((config->saturation_limit > (zone_marker_max - config->increment_count)) ||
        (config->decrement_count > zone_marker_max))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }
    if (config->num_neighboring_zones > (CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES - 1))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }
    if ((config->max_duration_ticks == 0) ||
        (config->max_duration_ticks > config->period_ticks))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }
    if ((config->range_max <= 0.0f) ||
        (config->doppler_gating_speed_mps <= 0.0f))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    clutter->config = config;
    // start immediately after init
    clutter->period_counter_ticks = clutter->config->period_ticks;

    // the state always starts with IDLE
    clutter->register_update_state = CLUTTER_LOCALIZATION_STATE_IDLE;

    // write default exclusion zones to hardware if configured
    if (clutter_localization_has_default_exclusion_zones(config))
    {
        // if any default exclusion zones are configured, use also the defined priority for selecting the plane.
        if (config->default_exclusion_zones.MD_ZONE_CTRL == BGT60CUTR13B_MD_ZONE_CTRL_PLANE_SEL_H_PLANE)
        {
            clutter->config->priority_h_plane = true;
        }
        else
        {
            clutter->config->priority_h_plane = false;
        }
        return clutter_localization_write_exclusion_zones(motion_detection, clutter);
    }
    else
    {
        // Only option to set exclusion value is via default_exclusion_zones in the clutter_localization_config_t
        // structure.
        return XENSIV_BGT60CUTRXX_STATUS_OK;
    }
}


int32_t clutter_localization_process(motion_detection_t* motion_detection, clutter_localization_t* clutter)
{
    // Input validation
    if ((motion_detection == NULL) || (clutter == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    bool clutter_detected = false;
    static uint32_t clutter_active_counter = 0;
    static bool clutter_detection_active = false;
    static bool clutter_algo_deactivated = false;
    static uint32_t rd_event_counter_no_targets = 0;

    // process only if seq2 is done
    if (motion_detection->seq2_done)
    {
        // in case there are no targets since some time, deactivate the clutter detection
        // but if afterwards a target is detected, reactivate the clutter detection immediately
        if (motion_detection->number_of_detected_targets != 0)
        {
            // clear the counter
            rd_event_counter_no_targets = 0;
        }
        else
        {
            // no targets detected, increase the counter
            rd_event_counter_no_targets++;
        }

        // check if the clutter detection was deactivated due to missing targets
        if (rd_event_counter_no_targets >= clutter->config->deactivation_counter_threshold_ticks)
        {
            // reactivate clutter detection by setting the period counter to the period ticks
            clutter_algo_deactivated = true;
        }
        else
        {
            clutter_algo_deactivated = false;
        }


        if (clutter->period_counter_ticks >= clutter->config->period_ticks)
        {
            // start clutter detection processing
            if (clutter_algo_deactivated)
            {
                clutter_detection_active = false;
            }
            else
            {
                clutter_detection_active = true;
            }
            // reset counter to start new period
            clutter->period_counter_ticks = 0;
            // reset active counter
            clutter_active_counter = 0;

            // reset the zones only if the zones have been written before
            if (clutter->config->auto_masking_active && clutter_detection_active)
            {
                if (clutter->register_update_state == CLUTTER_LOCALIZATION_STATE_ZONES_SET)
                {
                    // reset the exclusion zones
                    clutter_localization_reset_exclusion_zones(motion_detection, clutter);
                    // exclusion zones are reset, so set the state to cleared
                    clutter->register_update_state = CLUTTER_LOCALIZATION_STATE_ZONES_RESET;
                    clutter->register_update_state_history = CLUTTER_LOCALIZATION_STATE_CLUTTER_CHECK;
                }
            }
        }
        else
        {
            // increase the period counter
            clutter->period_counter_ticks++;
        }


        // start clutter detection processing only if the period has elapsed
        if (clutter_detection_active)
        {
            clutter_active_counter++;
            if (clutter_active_counter < clutter->config->max_duration_ticks)
            {
                clutter_detected = clutter_localization_calculate(motion_detection, clutter);
                // if clutter detected, set the neighboring zones as well and write to sensor
                if (clutter_detected && (clutter->config->auto_masking_active))
                {
                    status |= clutter_localization_set_neighboring_zones(clutter, clutter->zone_marker_e,
                                                                         clutter->zone_marker_e_neighboring,
                                                                         clutter->zone_marker_e_neighboring_history);
                    status |= clutter_localization_set_neighboring_zones(clutter, clutter->zone_marker_h,
                                                                         clutter->zone_marker_h_neighboring,
                                                                         clutter->zone_marker_h_neighboring_history);

                    // calculate register values
                    status |= clutter_localization_map_zones_to_registers(clutter);
                    // write to sensor
                    status |= clutter_localization_write_exclusion_zones(motion_detection, clutter);
                    // set the state to detected
                    clutter->register_update_state = CLUTTER_LOCALIZATION_STATE_ZONES_SET;
                    // disable further processing until next period
                    clutter_detection_active = false;
                }
                status |= clutter_localization_filter_during_recheck(motion_detection, clutter);
            }
            else
            {
                clutter_detection_active = false;
                if (clutter->register_update_state_history == CLUTTER_LOCALIZATION_STATE_CLUTTER_CHECK)
                {
                    clutter->register_update_state_history = CLUTTER_LOCALIZATION_STATE_NO_CLUTTER;
                    for (uint8_t r_zone = 0; r_zone < CLUTTER_LOCALIZATION_NUM_RANGE_ZONES; r_zone++)
                    {
                        for (uint8_t a_zone = 0; a_zone < CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES; a_zone++)
                        {
                            clutter->zone_marker_e_neighboring_history[r_zone][a_zone] = 0;
                            clutter->zone_marker_h_neighboring_history[r_zone][a_zone] = 0;
                        }
                    }
                }
            }
        }
    }
    return status == XENSIV_BGT60CUTRXX_STATUS_OK ? status : XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
}


bool clutter_localization_has_default_exclusion_zones(const clutter_localization_config_t* config)
{
    if (config == NULL)
    {
        return false;
    }
    return (config->default_exclusion_zones.MD_ZONE_EXCL0 |
            config->default_exclusion_zones.MD_ZONE_EXCL1 |
            config->default_exclusion_zones.MD_ZONE_EXCL2 |
            config->default_exclusion_zones.MD_ZONE_EXCL3) != 0U;
}


int32_t clutter_localization_write_exclusion_zones(motion_detection_t* motion_detection,
                                                   clutter_localization_t* clutter)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    if (motion_detection->radar_sensor->power_mode == DEEP_SLEEP_MODE)
    {
        status = xensiv_bgt60cutrxx_wakeup(motion_detection->radar_sensor);
        if (status != XENSIV_BGT60CUTRXX_STATUS_OK)
        {
            return status;
        }
    }
    // if there are default exclusion zones configured, set them in the registers as well, so that the detected clutter
    // zones are added on top of the default zones instead of overwriting them
    if (clutter_localization_has_default_exclusion_zones(clutter->config))
    {
        clutter->zone_registers.MD_ZONE_EXCL0 |= clutter->config->default_exclusion_zones.MD_ZONE_EXCL0;
        clutter->zone_registers.MD_ZONE_EXCL1 |= clutter->config->default_exclusion_zones.MD_ZONE_EXCL1;
        clutter->zone_registers.MD_ZONE_EXCL2 |= clutter->config->default_exclusion_zones.MD_ZONE_EXCL2;
        clutter->zone_registers.MD_ZONE_EXCL3 |= clutter->config->default_exclusion_zones.MD_ZONE_EXCL3;
        clutter->zone_registers.MD_ZONE_CTRL = clutter->config->default_exclusion_zones.MD_ZONE_CTRL;
    }

    status |= xensiv_bgt60cutrxx_set_reg(motion_detection->radar_sensor, BGT60CUTR13B_REG_MD_ZONE_EXCL0,
                                         clutter->zone_registers.MD_ZONE_EXCL0);
    status |= xensiv_bgt60cutrxx_set_reg(motion_detection->radar_sensor, BGT60CUTR13B_REG_MD_ZONE_EXCL1,
                                         clutter->zone_registers.MD_ZONE_EXCL1);
    status |= xensiv_bgt60cutrxx_set_reg(motion_detection->radar_sensor, BGT60CUTR13B_REG_MD_ZONE_EXCL2,
                                         clutter->zone_registers.MD_ZONE_EXCL2);
    status |= xensiv_bgt60cutrxx_set_reg(motion_detection->radar_sensor, BGT60CUTR13B_REG_MD_ZONE_EXCL3,
                                         clutter->zone_registers.MD_ZONE_EXCL3);
    status |= xensiv_bgt60cutrxx_set_reg(motion_detection->radar_sensor, BGT60CUTR13B_REG_MD_ZONE_CTRL,
                                         clutter->zone_registers.MD_ZONE_CTRL);

    if (motion_detection->radar_sensor->power_mode == DEEP_SLEEP_MODE)
    {
        //request power down mode
        status |= xensiv_bgt60cutrxx_set_xosc(motion_detection->radar_sensor, XENSIV_BGT60CUTRXX_XOSC_POWER_DOWN);
    }
    return status == XENSIV_BGT60CUTRXX_STATUS_OK ? status : XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
}


int8_t argmin_abs(float value, float* arr, int8_t arr_len)
{
    int8_t min_idx = 0;
    float min_diff = fabsf(value - arr[0]);
    for (int8_t j = 1; j < arr_len; ++j)
    {
        float diff = fabsf(value - arr[j]);
        if (diff < min_diff)
        {
            min_diff = diff;
            min_idx = j;
        }
    }
    return min_idx;
}


int8_t get_range_zone(float range, float range_max)
{
    // Linspace from 0 to range_max in NUM_POINTS steps
    #define NUM_POINTS 16
    float range_array[NUM_POINTS];
    for (int i = 0; i < NUM_POINTS; ++i)
    {
        range_array[i] = range_max * i / (NUM_POINTS - 1);
    }

    int r_index = argmin_abs(range, range_array, NUM_POINTS);
    return r_index;
}


int8_t get_angle_zone(float angle)
{
    // Map the angle to the corresponding zone starting from  -78.75, until 78.75 in 7 groups
    // Boundaries in degrees:
    // [-78.75, -56.25, -33.75, -11.25,  11.25,  33.75,  56.25,  78.75]

    if ((angle >= -78.75f) && (angle < -56.25f))
    {
        return 0;
    }
    else if ((angle >= -56.25f) && (angle < -33.75f))
    {
        return 1;
    }
    else if ((angle >= -33.75f) && (angle < -11.25f))
    {
        return 2;
    }
    else if ((angle >= -11.25f) && (angle < 11.25f))
    {
        return 3;
    }
    else if ((angle >= 11.25f) && (angle < 33.75f))
    {
        return 4;
    }
    else if ((angle >= 33.75f) && (angle < 56.25f))
    {
        return 5;
    }
    else if ((angle >= 56.25f) && (angle <= 78.75f))
    {
        return 6;
    }
    else
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }
}


bool clutter_localization_update_markers_value(clutter_localization_t* clutter, zone_marker_array_t zone_marker,
                                               zone_marker_array_t temp_zone_marker, int8_t range, int8_t angle)
{
    bool clutter_detected = false;
    // increment by increment_count if temp marker is set and decrement by decrement_count if not set
    if (temp_zone_marker[range][angle])
    {
        // increment, but limit to saturation_limit
        zone_marker[range][angle] += clutter->config->increment_count;
        if (zone_marker[range][angle] > clutter->config->saturation_limit)
        {
            zone_marker[range][angle] = clutter->config->saturation_limit;
        }
        // check if clutter threshold has been reached
        if (zone_marker[range][angle] >= clutter->config->settings_threshold)
        {
            clutter_detected = true;
        }
    }
    else
    {
        // avoid that it goes below zero
        zone_marker[range][angle] -= clutter->config->decrement_count;
        if (zone_marker[range][angle] < 0)
        {
            zone_marker[range][angle] = 0;
        }
    }
    return clutter_detected;
}


bool clutter_localization_update_registers(clutter_localization_t* clutter, zone_marker_array_t zone_marker,
                                           int8_t range, int8_t angle)
{
    bool clutter_detected = false;
    // calculate the bit position
    uint8_t bit_position = angle * CLUTTER_LOCALIZATION_NUM_RANGE_ZONES + range;
    // check for h-plane first
    if (zone_marker[range][angle] >= CLUTTER_LOCALIZATION_EXCLUSION_VALUE)
    {
        // set register bit
        // if the bit position is less than 32, then set the bit in the register MD_ZONE_EXCL0
        if (bit_position < 32)
        {
            clutter->zone_registers.MD_ZONE_EXCL0 |= (1LU << bit_position);
        }
        else if (bit_position < 64)
        {
            clutter->zone_registers.MD_ZONE_EXCL1 |= (1LU << (bit_position - 32));
        }
        else if (bit_position < 96)
        {
            clutter->zone_registers.MD_ZONE_EXCL2 |= (1LU << (bit_position - 64));
        }
        else
        {
            clutter->zone_registers.MD_ZONE_EXCL3 |= (1LU << (bit_position - 96));
        }
        clutter_detected = true;
    }
    else
    {
        // clear register bit
        if (bit_position < 32)
        {
            clutter->zone_registers.MD_ZONE_EXCL0 &= ~(1LU << bit_position);
        }
        else if (bit_position < 64)
        {
            clutter->zone_registers.MD_ZONE_EXCL1 &= ~(1LU << (bit_position - 32));
        }
        else if (bit_position < 96)
        {
            clutter->zone_registers.MD_ZONE_EXCL2 &= ~(1LU << (bit_position - 64));
        }
        else
        {
            clutter->zone_registers.MD_ZONE_EXCL3 &= ~(1LU << (bit_position - 96));
        }
    }
    return clutter_detected;
}


bool clutter_localization_update_markers(clutter_localization_t* clutter, zone_marker_array_t temp_zone_marker_e,
                                         zone_marker_array_t temp_zone_marker_h)
{
    bool clutter_detected = false;
    bool skip_e_plane = false;
    bool skip_h_plane = false;

    // if default exclusion zones are configured, only update the markers for the plane which is defined in the default
    // exclusion zone configuration
    if (clutter_localization_has_default_exclusion_zones(clutter->config))
    {
        if ((clutter->config->default_exclusion_zones.MD_ZONE_CTRL & BGT60CUTR13B_MD_ZONE_CTRL_PLANE_SEL_msk) ==
            (BGT60CUTR13B_MD_ZONE_CTRL_PLANE_SEL_H_PLANE << BGT60CUTR13B_MD_ZONE_CTRL_PLANE_SEL_pos))
        {
            // only update h-plane markers
            skip_e_plane = true;
        }
        else
        {
            // only update e-plane markers
            skip_h_plane = true;
        }
    }

    // Update the clutter's zone markers with the temporary markers
    for (int8_t range = 0; range < CLUTTER_LOCALIZATION_NUM_RANGE_ZONES; range++)
    {
        for (int8_t angle = 0; angle < CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES; angle++)
        {
            // Update the zone markers based on the temporary markers
            if (!skip_h_plane)
            {
                clutter_detected |= clutter_localization_update_markers_value(clutter, clutter->zone_marker_h,
                                                                              temp_zone_marker_h, range, angle);
            }
            if (!skip_e_plane)
            {
                clutter_detected |= clutter_localization_update_markers_value(clutter, clutter->zone_marker_e,
                                                                              temp_zone_marker_e, range, angle);
            }
        }
    }

    return clutter_detected;
}


int32_t clutter_localization_map_zones_to_registers(clutter_localization_t* clutter)
{
    bool clutter_detected = false;
    zone_marker_array_t* zone_marker_high_prio = NULL;
    zone_marker_array_t* zone_marker_low_prio = NULL;

    if (clutter->config->priority_h_plane)
    {
        // set to h-plane priority
        zone_marker_high_prio = &clutter->zone_marker_h_neighboring;
        zone_marker_low_prio = &clutter->zone_marker_e_neighboring;
        clutter->zone_registers.MD_ZONE_CTRL = BGT60CUTR13B_MD_ZONE_CTRL_PLANE_SEL_H_PLANE <<
                                               BGT60CUTR13B_MD_ZONE_CTRL_PLANE_SEL_pos;
    }
    else
    {
        // set to e-plane priority
        zone_marker_high_prio = &clutter->zone_marker_e_neighboring;
        zone_marker_low_prio = &clutter->zone_marker_h_neighboring;
        clutter->zone_registers.MD_ZONE_CTRL = BGT60CUTR13B_MD_ZONE_CTRL_PLANE_SEL_E_PLANE <<
                                               BGT60CUTR13B_MD_ZONE_CTRL_PLANE_SEL_pos;
    }


    // Update the clutter's zone markers with the temporary markers
    for (int8_t range = 0; range < CLUTTER_LOCALIZATION_NUM_RANGE_ZONES; range++)
    {
        for (int8_t angle = 0; angle < CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES; angle++)
        {
            clutter_detected |= clutter_localization_update_registers(clutter, *zone_marker_high_prio, range, angle);
        }
    }

    // only update the registers in case clutter has not been detected in the other plane
    if (!clutter_detected)
    {
        // iterate again through the zone markers
        for (int8_t range = 0; range < CLUTTER_LOCALIZATION_NUM_RANGE_ZONES; range++)
        {
            for (int8_t angle = 0; angle < CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES; angle++)
            {
                // update registers
                clutter_detected |= clutter_localization_update_registers(clutter, *zone_marker_low_prio, range, angle);
            }
        }
        // check again if clutter has been detected if yes low prio plane is used, so set the MD_ZONE_CTRL accordingly
        // by inverting the MD_ZONE_CTRL value
        if (clutter_detected)
        {
            // set to e-plane priority
            clutter->zone_registers.MD_ZONE_CTRL = (~clutter->zone_registers.MD_ZONE_CTRL) &
                                                   BGT60CUTR13B_MD_ZONE_CTRL_PLANE_SEL_msk;
        }
    }

    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


int32_t clutter_localization_map_registers_to_zones(clutter_localization_t* clutter, zone_marker_array_t* zone_marker)
{
    if ((clutter == NULL) || (zone_marker == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    for (int8_t angle = 0; angle < CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES; angle++)
    {
        for (int8_t range = 0; range < CLUTTER_LOCALIZATION_NUM_RANGE_ZONES; range++)
        {
            uint8_t bit_position = (uint8_t)(angle * CLUTTER_LOCALIZATION_NUM_RANGE_ZONES + range);
            uint32_t bit;
            if (bit_position < 32)
            {
                bit = (clutter->zone_registers.MD_ZONE_EXCL0 >> bit_position) & 1U;
            }
            else if (bit_position < 64)
            {
                bit = (clutter->zone_registers.MD_ZONE_EXCL1 >> (bit_position - 32)) & 1U;
            }
            else if (bit_position < 96)
            {
                bit = (clutter->zone_registers.MD_ZONE_EXCL2 >> (bit_position - 64)) & 1U;
            }
            else
            {
                bit = (clutter->zone_registers.MD_ZONE_EXCL3 >> (bit_position - 96)) & 1U;
            }
            (*zone_marker)[range][angle] = (int8_t)bit;
        }
    }

    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


bool clutter_localization_calculate(motion_detection_t* motion_detection, clutter_localization_t* clutter)
{
    zone_marker_array_t temp_zone_marker_e = { 0 };
    zone_marker_array_t temp_zone_marker_h = { 0 };

    // iterate through all targets and update zone markers if the target is above the doppler gating speed
    for (uint8_t i = 0; i < BGT60CUTR13_PROCESSED_MD_TARGET_MAX; i++)
    {
        if (motion_detection->processed_targets[i].antenna_rx != 0)
        {
            // Update the temporary zone markers based on the target's position
            int8_t range_zone = get_range_zone(motion_detection->processed_targets[i].range_m,
                                               clutter->config->range_max);
            if (range_zone < 0)
            {
                // invalid range zone
                continue;
            }

            // if range_zone is out of bounds, clutter is detected
            const float target_speed = motion_detection->processed_targets[i].velocity_mps;
            const float gating_speed = clutter->config->doppler_gating_speed_mps;
            if ((target_speed > gating_speed) ||
                (target_speed < -gating_speed))
            {
                int8_t azimuth_zone = get_angle_zone(motion_detection->processed_targets[i].azimuth_deg);
                int8_t elevation_zone = get_angle_zone(motion_detection->processed_targets[i].elevation_deg);
                if ((azimuth_zone < 0) || (elevation_zone < 0))
                {
                    // invalid angle zone
                    continue;
                }

                temp_zone_marker_e[range_zone][azimuth_zone] = CLUTTER_LOCALIZATION_EXCLUSION_VALUE;
                temp_zone_marker_h[range_zone][elevation_zone] = CLUTTER_LOCALIZATION_EXCLUSION_VALUE;
            }
        }
    }
    return clutter_localization_update_markers(clutter, temp_zone_marker_e, temp_zone_marker_h);
}


int32_t clutter_localization_filter_during_recheck(motion_detection_t* motion_detection,
                                                   clutter_localization_t* clutter)
{
    // iterate through all targets and update zone markers if the target is above the doppler gating speed
    for (uint8_t i = 0; i < BGT60CUTR13_PROCESSED_MD_TARGET_MAX; i++)
    {
        if (motion_detection->processed_targets[i].antenna_rx != 0)
        {
            // Update the temporary zone markers based on the target's position
            int8_t range_zone = get_range_zone(motion_detection->processed_targets[i].range_m,
                                               clutter->config->range_max);
            if (range_zone < 0)
            {
                // invalid range zone
                continue;
            }

            int8_t azimuth_zone = get_angle_zone(motion_detection->processed_targets[i].azimuth_deg);
            int8_t elevation_zone = get_angle_zone(motion_detection->processed_targets[i].elevation_deg);
            if ((azimuth_zone < 0) || (elevation_zone < 0))
            {
                // invalid angle zone
                continue;
            }

            if ((clutter->zone_marker_e_neighboring_history[range_zone][azimuth_zone]) ||
                (clutter->zone_marker_h_neighboring_history[range_zone][elevation_zone]))
            {
                motion_detection->processed_targets[i].range_m = 0.0f;
                motion_detection->processed_targets[i].azimuth_deg = 0.0f;
                motion_detection->processed_targets[i].elevation_deg = 0.0f;
                motion_detection->processed_targets[i].magnitude_dBFS = 0.0f;
                motion_detection->processed_targets[i].antenna_rx = 0.0f;
                motion_detection->processed_targets[i].velocity_mps = 0.0f;
            }
        }
    }
    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


int32_t clutter_localization_set_neighboring_zones(clutter_localization_t* clutter, zone_marker_array_t zone_marker,
                                                   zone_marker_array_t zone_marker_neighboring,
                                                   zone_marker_array_t zone_marker_neighboring_history)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;

    // iterate through all zone markers and set the neighboring zones if the zone marker is above the threshold
    for (int8_t range = 0; range < CLUTTER_LOCALIZATION_NUM_RANGE_ZONES; range++)
    {
        for (int8_t angle = 0; angle < CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES; angle++)
        {
            // if the zone marker is above the threshold, set the neighboring zones
            if (zone_marker[range][angle] >= clutter->config->settings_threshold)
            {
                // set the neighboring zones
                const int8_t num_neighboring_zones = (int8_t)clutter->config->num_neighboring_zones;
                for (int8_t r = -num_neighboring_zones; r <= num_neighboring_zones; r++)
                {
                    for (int8_t a = -num_neighboring_zones; a <= num_neighboring_zones; a++)
                    {
                        int8_t neighbor_range = range + r;
                        int8_t neighbor_angle = angle + a;
                        // check if the neighboring zone is within bounds
                        if ((neighbor_range >= 0) && (neighbor_range < CLUTTER_LOCALIZATION_NUM_RANGE_ZONES) &&
                            (neighbor_angle >= 0) && (neighbor_angle < CLUTTER_LOCALIZATION_NUM_ANGLE_ZONES))
                        {
                            zone_marker_neighboring[neighbor_range][neighbor_angle] =
                                CLUTTER_LOCALIZATION_EXCLUSION_VALUE;
                            zone_marker_neighboring_history[neighbor_range][neighbor_angle] =
                                CLUTTER_LOCALIZATION_EXCLUSION_VALUE;
                        }
                    }
                }
            }
        }
    }
    return status;
}
