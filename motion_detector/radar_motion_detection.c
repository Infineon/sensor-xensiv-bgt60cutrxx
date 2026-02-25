/**
 * Description:
 * The radar_motion_detection module is responsible for detecting motion using radar technology.
 * It provides functions to process radar data and detect motion based on predefined algorithms.
 * This file contains the implementation of the radar motion detection functionality.
 *
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
#include <math.h>
#include <limits.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#include "radar_motion_detection.h"
#include "xensiv_bgt60cutrxx_regs.h"
#include "xensiv_bgt60cutrxx_platform.h"

/**
 * \addtogroup radar_motion_detection
 * @{
 */

/*
    Helper to convert fixed point signed numbers into float.

        Args:
            x (uint32_t): input fixed number which is of integer datatype
            e (q_exponent_t): number of fractional bits, e.g. Q0.15, e = 15

        Usage example:
 # Q0.31 fixed point to float conversion
            val_float = q0_to_float(np.int32(45456543), 31)

 # Q0.15 fixed point to float conversion
            val_float = q0_to_float(np.int16(65523), 15)
 */

static float q0_to_float(int x, q_exponent_t e)
{
    float f;
    int c;
    float sign = 1.0f;

    // Check for INT_MIN to avoid undefined behavior in abs()
    if (x == INT_MIN)
    {
        // Handle INT_MIN as a special case
        c = INT_MAX;
        sign = -1.0f;
    }
    else
    {
        c = abs(x);

        if (x < 0)
        {
            // Convert back from two's complement
            c = x - 1;
            c = ~c;
            sign = -1.0f;
        }
    }

    f = (sign * (float)c) / (float)(pow(2, e));

    return f;
}


int32_t radar_motion_detection_init(motion_detection_t* motion_detection,
                                    xensiv_bgt60cutrxx_t* radar_sensor,
                                    uint32_t* rd_data_buffer,
                                    const sequence_settings_t* rd_sequence_settings,
                                    const sequence_settings_t* ulp_sequence_settings)
{
    int32_t ret = XENSIV_BGT60CUTRXX_STATUS_OK;

    // Input validation
    if ((motion_detection == NULL) || (radar_sensor == NULL) || (rd_data_buffer == NULL) ||
        (rd_sequence_settings == NULL) || (ulp_sequence_settings == NULL))
    {
        return -1;
    }

    if ((rd_sequence_settings->enabled == false) && (ulp_sequence_settings->enabled == false))
    {
        return -1;  // at least one sequence must be enabled
    }

    //connect motion detector with radar sensor
    motion_detection->radar_sensor = radar_sensor;
    motion_detection->rd_data_buffer = rd_data_buffer;
    motion_detection->radar_sensor->mode = ACTIVE_MODE;

    motion_detection->rd_sequence_settings = rd_sequence_settings;
    motion_detection->ulp_sequence_settings = ulp_sequence_settings;

    // start motion detection
    #if defined (BGT60CUTR13B_MD_START_VIA_FIRMWARE_TRIGGER)
    ret = xensiv_bgt60cutrxx_set_motion_detection(radar_sensor, 0, true);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != ret)
    {
        DEBUG_PRINT("xensiv_bgt60cutrxx_set_motion_detection failed\n");
    }
    #endif

    return ret;
}


/** @brief Calculates the motion detection targets based on the radar data.
 *
 * This function processes the radar data to extract motion detection targets.
 * It takes into account the current configuration and state of the motion detector.
 *
 * @param motion_detection Pointer to the motion detector instance.
 * @param sequence_settings Pointer to the sequence settings used for decoding.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t radar_motion_detection_decode_targets(motion_detection_t* motion_detection,
                                              const sequence_settings_t* sequence_settings)
{
    float magnitude;
    uint8_t range_bin;
    uint8_t velocity_bin;
    uint8_t rx_antenna_index;
    int16_t elevation_raw;
    int16_t azimuth_raw;

    // clear the target buffer
    memset(motion_detection->processed_targets, 0, sizeof(motion_detection->processed_targets));


    // Process motion targets
    for (int i = 0; i < BGT60CUTR13_PROCESSED_MD_TARGET_MAX; i++)
    {
        /* Parse results from target register */
        range_bin = (motion_detection->targets[i].location & BGT60CUTR13B_MD_TARGET0_LOC_RBIN_msk);
        velocity_bin = (motion_detection->targets[i].location & BGT60CUTR13B_MD_TARGET0_LOC_DBIN_msk) >>
                       BGT60CUTR13B_MD_TARGET0_LOC_DBIN_pos;
        motion_detection->range_bin[i] = range_bin;
        motion_detection->doppler_bin[i] = velocity_bin;
        rx_antenna_index = (motion_detection->targets[i].location & BGT60CUTR13B_MD_TARGET0_LOC_RX_msk) >>
                           BGT60CUTR13B_MD_TARGET0_LOC_RX_pos;

        /* IFX recommended to orient the device in the E-plane mode.
           In this case the azimuth angle belong to the E plane and the H-plane is the elevation*/
        azimuth_raw = (motion_detection->targets[i].angle & BGT60CUTR13B_MD_TARGET0_ANGLE_E_PLANE_msk) >>
                      BGT60CUTR13B_MD_TARGET0_ANGLE_E_PLANE_pos;
        elevation_raw = (motion_detection->targets[i].angle & BGT60CUTR13B_MD_TARGET0_ANGLE_H_PLANE_msk);

        magnitude = q0_to_float(motion_detection->targets[i].magnitude, Q31);
        motion_detection->processed_targets[i].azimuth_deg = q0_to_float(azimuth_raw, Q15) * 360;
        motion_detection->processed_targets[i].elevation_deg = q0_to_float(elevation_raw, Q15) * 360;

        if (rx_antenna_index != 0)
        {
            motion_detection->processed_targets[i].range_m = (float)range_bin *
                                                             sequence_settings->range_resolution_m;
            motion_detection->processed_targets[i].velocity_mps =
                (float)sequence_settings->velocity_grid_mps[velocity_bin];
            motion_detection->processed_targets[i].magnitude_dBFS = 20.0f * log10f(magnitude);
            motion_detection->processed_targets[i].antenna_rx = rx_antenna_index;
            motion_detection->number_of_detected_targets++;
        }
    }
    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


bool radar_motion_detection_evaluate_sequence_counter(uint32_t seq_counter, uint32_t seq_counter_prev)
{
    if (seq_counter != ((seq_counter_prev + 1) & BGT60CUTR13B_SEQ1_STAT_SEQ_CNT_msk))
    {
        return false;
    }

    return true;
}


void radar_motion_detection_update_sequence_counters(motion_detection_t* motion_detection, uint32_t seq1_status,
                                                     uint32_t seq2_status)
{
    motion_detection->sequence_counters.seq1_counter = (uint16_t)(seq1_status & BGT60CUTR13B_SEQ1_STAT_SEQ_CNT_msk);
    motion_detection->sequence_counters.seq2_counter = (uint16_t)(seq2_status & BGT60CUTR13B_SEQ2_STAT_SEQ_CNT_msk);
}


int32_t radar_motion_detection_targets_read(motion_detection_t* motion_detection)
{
    int32_t ret = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint32_t irq_status;
    uint32_t seq1_status;
    uint32_t seq2_status;

    // Input validation
    if (motion_detection == NULL)
    {
        return -1;
    }

    // clear the flags
    motion_detection->seq1_done = false;
    motion_detection->seq2_done = false;
    motion_detection->ulp_event = false;
    motion_detection->rd_event = false;
    motion_detection->sequence = 0;

    ret = xensiv_bgt60cutrxx_get_irq_status(motion_detection->radar_sensor, &irq_status);
    if (ret != XENSIV_BGT60CUTRXX_STATUS_OK)
    {
        DEBUG_PRINT("Error reading IRQ status: %ld\n", ret);
        return ret;
    }
    DEBUG_PRINT("IR_STAT: 0x%08lX\n", irq_status);

    xensiv_bgt60cutrxx_get_reg(motion_detection->radar_sensor, BGT60CUTR13B_REG_SEQ1_STAT, &seq1_status);
    uint16_t seq1_counter_new = (uint16_t)(seq1_status & BGT60CUTR13B_SEQ1_STAT_SEQ_CNT_msk);
    DEBUG_PRINT("Sequence 1 counter actual: %d\n\r", seq1_counter_new);
    xensiv_bgt60cutrxx_get_reg(motion_detection->radar_sensor, BGT60CUTR13B_REG_SEQ2_STAT, &seq2_status);
    uint16_t seq2_counter_new = (uint16_t)(seq2_status & BGT60CUTR13B_SEQ2_STAT_SEQ_CNT_msk);
    DEBUG_PRINT("Sequence 2 counter actual: %d\n\r", seq2_counter_new);

    //first check for sequence 1
    if (irq_status & BGT60CUTR13B_IR_STAT_SEQ1_DONE_MSK)
    {
        DEBUG_PRINT("Event triggered ==> SEQ1 done\n\r");
        motion_detection->seq1_done = true;
        motion_detection->sequence = 1;
        if (radar_motion_detection_evaluate_sequence_counter(seq1_counter_new,
                                                             motion_detection->sequence_counters.seq1_counter) == false)
        {
            DEBUG_PRINT("Sequence counter mismatch: prev. %d, actual %d\n\r",
                        motion_detection->sequence_counters.seq1_counter, seq1_counter_new);
        }
        // Sequence 1 is done, but target registers may not be available yet.
        if ((irq_status & BGT60CUTR13B_IR_STAT_ULP_EVENT_MSK) == 0)
        {
            DEBUG_PRINT("ULP event not set, waiting for it 5ms and read ir_stat again\n\r");
            xensiv_bgt60cutrxx_platform_delay(5);
            ret = xensiv_bgt60cutrxx_get_irq_status(motion_detection->radar_sensor, &irq_status);
            if (ret != XENSIV_BGT60CUTRXX_STATUS_OK)
            {
                DEBUG_PRINT("Error reading IRQ status: %ld\n", ret);
                return ret;
            }
            DEBUG_PRINT("IR_STAT: 0x%08lX\n", irq_status);
        }
    }


    // check if ULP event is set
    if (irq_status & BGT60CUTR13B_IR_STAT_ULP_EVENT_MSK)
    {
        DEBUG_PRINT("ULP event set\n\r");
        motion_detection->seq1_done = true;
        motion_detection->sequence = 1;
        motion_detection->ulp_event = true;
        if (radar_motion_detection_evaluate_sequence_counter(seq1_counter_new,
                                                             motion_detection->sequence_counters.seq1_counter) == false)
        {
            DEBUG_PRINT("Sequence counter mismatch: prev. %d, actual %d\n\r",
                        motion_detection->sequence_counters.seq1_counter, seq1_counter_new);
        }
        ret = xensiv_bgt60cutrxx_get_md_targets(motion_detection->radar_sensor, (uint32_t*)motion_detection->targets,
                                                BGT60CUTR13_MD_TARGET_REGISTERS);
        if (ret != XENSIV_BGT60CUTRXX_STATUS_OK)
        {
            DEBUG_PRINT("Error reading motion detection targets: %ld\n", ret);
            return ret;
        }
    }

    // then check for sequence 2
    if (irq_status & BGT60CUTR13B_IR_STAT_SEQ2_DONE_MSK)
    {
        DEBUG_PRINT("Event triggered ==> SEQ2 done\n\r");
        motion_detection->seq2_done = true;
        motion_detection->sequence = 2;
        if (radar_motion_detection_evaluate_sequence_counter(seq2_counter_new,
                                                             motion_detection->sequence_counters.seq2_counter) == false)
        {
            DEBUG_PRINT("Sequence counter mismatch: prev. %d, actual %d\n\r",
                        motion_detection->sequence_counters.seq2_counter, seq2_counter_new);
        }
        // Sequence 2 is done, but target registers may not be available yet.
        if ((irq_status & BGT60CUTR13B_IR_STAT_RD_EVENT_MSK) == 0)
        {
            DEBUG_PRINT("RD event not set, waiting for it 5ms and read ir_stat again\n\r");
            xensiv_bgt60cutrxx_platform_delay(5);
            ret = xensiv_bgt60cutrxx_get_irq_status(motion_detection->radar_sensor, &irq_status);
            if (ret != XENSIV_BGT60CUTRXX_STATUS_OK)
            {
                DEBUG_PRINT("Error reading IRQ status: %ld\n", ret);
                return ret;
            }
            DEBUG_PRINT("IR_STAT: 0x%08lX\n", irq_status);
        }
    }

    // check for RD event
    if (irq_status & BGT60CUTR13B_IR_STAT_RD_EVENT_MSK)
    {
        DEBUG_PRINT("RD event set\n\r");
        motion_detection->seq2_done = true;
        motion_detection->sequence = 2;
        motion_detection->rd_event = true;
        if (radar_motion_detection_evaluate_sequence_counter(seq2_counter_new,
                                                             motion_detection->sequence_counters.seq2_counter) == false)
        {
            DEBUG_PRINT("Sequence counter mismatch: prev. %d, actual %d\n\r",
                        motion_detection->sequence_counters.seq2_counter, seq2_counter_new);
        }
        ret = xensiv_bgt60cutrxx_get_md_targets(motion_detection->radar_sensor, (uint32_t*)motion_detection->targets,
                                                BGT60CUTR13_MD_TARGET_REGISTERS);
        if (ret != XENSIV_BGT60CUTRXX_STATUS_OK)
        {
            DEBUG_PRINT("Error reading motion detection targets: %ld\n", ret);
            return ret;
        }
    }

    // update sequence counters
    radar_motion_detection_update_sequence_counters(motion_detection, seq1_status, seq2_status);

    motion_detection->ir_stat = irq_status;
    // clear all ir_stat bits
    ret = xensiv_bgt60cutrxx_set_irq_clear(motion_detection->radar_sensor, BGT60CUTR13B_IR_CLEAR_ALL);

    return ret;
}


int32_t radar_motion_detection_wakeup(motion_detection_t* motion_detection, bool val)
{
    int32_t ret = XENSIV_BGT60CUTRXX_STATUS_OK;
    ret = xensiv_bgt60cutrxx_set_xosc(motion_detection->radar_sensor, val);
    if (ret != XENSIV_BGT60CUTRXX_STATUS_OK)
    {
        DEBUG_PRINT("Error setting XOSC: %ld\n", ret);
        return ret;
    }
    if (val)
    {
        xensiv_bgt60cutrxx_platform_delay(BGT60CUTRxx_CLK_SETUP_TIME_MS);
        while (!motion_detection->radar_sensor->xosc_active)
        {
            ret = xensiv_bgt60cutrxx_get_xosc_state(motion_detection->radar_sensor);
            if (ret != XENSIV_BGT60CUTRXX_STATUS_OK)
            {
                DEBUG_PRINT("Error reading XOSC state: %ld\n", ret);
                return ret;
            }
            if (motion_detection->radar_sensor->xosc_active)
            {
                break;
            }
            xensiv_bgt60cutrxx_platform_delay(BGT60CUTRxx_CLK_SETUP_TIME_MS);
        }
    }
    return ret;
}


int32_t radar_motion_detection_process(motion_detection_t* motion_detection)
{
    int32_t ret = XENSIV_BGT60CUTRXX_STATUS_OK;

    // Input validation
    if (motion_detection == NULL)
    {
        return -1;
    }

    motion_detection->number_of_detected_targets = 0;

    // read motion detection target & zone information
    ret = radar_motion_detection_targets_read(motion_detection);
    if (ret != XENSIV_BGT60CUTRXX_STATUS_OK)
    {
        DEBUG_PRINT("Error reading motion detection targets: %ld\n", ret);
        return ret;
    }

    if (motion_detection->rd_event)
    {
        ret = radar_motion_detection_decode_targets(motion_detection, motion_detection->rd_sequence_settings);
    }
    else if (motion_detection->ulp_event)
    {
        ret = radar_motion_detection_decode_targets(motion_detection, motion_detection->ulp_sequence_settings);
    }
    else
    {
        // clear the target buffer
        memset(motion_detection->processed_targets, 0, sizeof(motion_detection->processed_targets));
        DEBUG_PRINT("No targets: \n");
    }

    return ret;
}


/**@} end of group radar_motion_detection*/
