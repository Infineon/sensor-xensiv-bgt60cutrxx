/*
 * radar_motion_detection.h
 *
 * Description: This header file contains the declarations and definitions for radar motion detection.
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

#ifndef RADAR_MOTION_DETECTION_H_
#define RADAR_MOTION_DETECTION_H_

#include <stddef.h>
#include "xensiv_bgt60cutrxx.h"

/**
 * \defgroup radar_motion_detection Radar Motion Detector
 * @{
 */

/** @brief Start motion detector via firmware trigger by setting gtmr control register
 * This is used to start the motion detection sequence in the radar sensor.
 * if defined, the motion detector is triggered via firmware by setting GTMR.
 */
// #define BGT60CUTR13B_MD_START_VIA_FIRMWARE_TRIGGER

/** @brief Number of target registers for motion detection */
#define BGT60CUTR13_MD_TARGET_REGISTERS 32
/** @brief Number of zone registers for motion detection */
#define BGT60CUTR13_MD_ZONE_REGISTERS 4
/** @brief Maximum size of the data buffer for range doppler map */
#define BGT60CUTR13_RD_DATABUFFER_MAX_SIZE 0x6000
/** @brief Maximum size of the processed targets */
#define BGT60CUTR13_PROCESSED_MD_TARGET_MAX 8

typedef enum
{
    Q15 = 15,
    Q31 = 31
} q_exponent_t;

/**
 * @brief Motion detector structure
 * This structure contains the raw values from motion detection target registers
 */
typedef struct
{
    uint32_t magnitude;
    uint32_t location;
    uint32_t angle;
    uint32_t empty;
} motion_detection_targets_t;

/**
 * @brief Motion detector structure
 * This structure contains the necessary variables to perform motion detection using the radar sensor.
 */
typedef struct
{
    float magnitude_dBFS;
    float azimuth_deg;
    float elevation_deg;
    float range_m;
    float velocity_mps;
    uint8_t antenna_rx;
} motion_detection_processed_targets_t;

typedef struct
{
    bool enabled;
    float range_resolution_m;
    const float* velocity_grid_mps;
} sequence_settings_t;



/** @brief Sequence counters structure
 * This structure contains the sequence counters for the motion detector.
 */
typedef struct
{
    uint16_t seq1_counter;
    uint16_t seq2_counter;
} sequence_counters_t;


/**
 * @brief Main structure for radar motion detection.
 *
 * The motion_detection_t structure encapsulates all necessary state, configuration,
 * and data buffers required for radar-based motion detection. It maintains pointers
 * to the radar sensor driver, memory for zone and target registers, processed target
 * results, waveform constants, sequence counters, and event flags. This structure
 * is used throughout the motion detection workflow to manage sensor interaction,
 * data acquisition, and signal processing.
 */
typedef struct motion_detection
{
    /**pointer to bgt60cutrxx driver variables*/
    xensiv_bgt60cutrxx_t* radar_sensor;
    /** allocates the memory for the md zone registers*/
    uint32_t zones[4];
    /** allocates the memory to store the full range doppler map*/
    uint32_t* rd_data_buffer;
    /** pointer sequence constants*/
    const sequence_settings_t* rd_sequence_settings;
    const sequence_settings_t* ulp_sequence_settings;
    /** contains the values for the sequence 1 and 2*/
    sequence_counters_t sequence_counters;
    /** allocates the memory for the md target registers*/
    motion_detection_targets_t targets[BGT60CUTR13_PROCESSED_MD_TARGET_MAX];
    /** range bin index */
    uint8_t range_bin[BGT60CUTR13_PROCESSED_MD_TARGET_MAX];
    /** doppler bin index */
    uint8_t doppler_bin[BGT60CUTR13_PROCESSED_MD_TARGET_MAX];
    /** allocates the memory to store the processed targets */
    motion_detection_processed_targets_t processed_targets[BGT60CUTR13_PROCESSED_MD_TARGET_MAX];
    /** Flag to indicate if the seq 1 is done and further processing is allowed */
    bool seq1_done;
    /** Flag to indicate if the seq 2 is done and further processing is allowed */
    bool seq2_done;
    /** Flag to indicate if a ULP event occurred */
    bool ulp_event;
    /** Flag to indicate if an RD event occurred */
    bool rd_event;
    uint32_t ir_stat; /**< ir_stat register content of the radar sensor */
    /** sequence that triggered the irq */
    uint8_t sequence;
    /** indicates the number of detected targets in the current sequence */
    uint16_t number_of_detected_targets;
} motion_detection_t;


/**
 * @brief Connects the motion detector module with a dedicated sensor
 *
 * This function initializes the motion detector module with the provided sensor.
 * It takes a pointer to the motion detector object and a pointer to the radar sensor object as parameters.
 * The function returns a status code indicating the success of the operation.
 *
 * @param motion_detection pointer to the motion detector object.
 * @param radar_sensor pointer to the radar sensor object.
 * @param rd_data_buffer pointer to the radar data buffer.
 * @param rd_sequence_settings pointer to the sequence settings for range doppler processing.
 * @param ulp_sequence_settings pointer to the sequence settings for ultra-low power processing.
 * @return status indicating the success of the operation. 0 = success, otherwise failure
 */
int32_t radar_motion_detection_init(motion_detection_t* motion_detection,
                                    xensiv_bgt60cutrxx_t* dev,
                                    uint32_t* rd_data_buffer,
                                    const sequence_settings_t* rd_sequence_settings,
                                    const sequence_settings_t* ulp_sequence_settings);


/**
 * @brief Processes radar motion detection using the specified motion detector instance.
 *
 * This function analyzes radar data to detect motion based on the configuration and state
 * of the provided motion_detection_t structure. It performs signal processing and updates
 * the internal state of the motion detector accordingly.
 *
 * @param motion_detection Pointer to a motion_detection_t instance containing configuration
 *                        and state information for motion detection.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t radar_motion_detection_process(motion_detection_t* motion_detection);

/**
 * @brief Reads the detected motion targets from the radar motion detector.
 *
 * This function retrieves information about the current motion targets detected
 * by the specified radar motion detector instance. The target registers are only read in case of an ulp or rd event.
 * It typically updates internal
 * data structures with the latest target information.
 *
 * @param[in,out] motion_detection Pointer to the motion detector instance from which to read targets.
 *
 * @return
 *   - The number of detected targets on success.
 *   - A negative value if an error occurs.
 */
int32_t radar_motion_detection_targets_read(motion_detection_t* motion_detection);

/**
 * @brief Decodes the raw target registers into the processed target list.
 *
 * Converts the magnitude, location and angle registers held in
 * motion_detection->targets into range, velocity, angle and magnitude, using
 * the resolution and velocity grid of the sequence that produced them. Entries
 * without a receive antenna are left cleared and are not counted.
 *
 * Callers that obtain the target registers by other means than
 * radar_motion_detection_targets_read(), such as a platform that reads them
 * through a kernel driver, use this directly. It performs no register access.
 *
 * @param[in,out] motion_detection Pointer to the motion detector instance.
 * @param sequence_settings Settings of the sequence the targets belong to.
 * @return 0 on success, or a negative error code on failure.
 */
int32_t radar_motion_detection_decode_targets(motion_detection_t* motion_detection,
                                              const sequence_settings_t* sequence_settings);

/**
 * @brief Checks that a sequence counter advanced by exactly one.
 *
 * @param seq_counter Counter read for the current sequence.
 * @param seq_counter_prev Counter read for the previous sequence.
 * @return true when no sequence was missed.
 */
bool radar_motion_detection_evaluate_sequence_counter(uint32_t seq_counter, uint32_t seq_counter_prev);
/**@} end of group radar_motion_detection*/
#endif /* RADAR_MOTION_DETECTION_H_ */
