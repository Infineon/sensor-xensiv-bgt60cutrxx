/*
 * radar_convergence.h
 *
 * Description: Convergence module for fusing Range-Doppler (RD) motion detection
 * results with Ultra-Low-Power (ULP) clustering results into a single, temporally
 * persistent output.
 *
 * Each detection type is held for a configurable persistence duration (in seconds,
 * converted to frame counts) so that brief gaps in detections do not cause
 * flickering in the reported output.
 *
 * Typical call sequence:
 * 1. convergence_init()    — once at startup
 * 2. convergence_process() — once per radar interrupt, after motion detection
 *                            and clustering have been processed
 * 3. Check convergence.results_updated to decide whether to consume the output
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
#ifndef RADAR_CONVERGENCE_H
#define RADAR_CONVERGENCE_H

#include <stddef.h>
#include "radar_clustering.h"
#include "radar_motion_detection.h"

/*******************************************************************************
 * Data Structures
 ********************************************************************************/

/**
 * @brief Configuration parameters for the convergence module.
 *
 * Persistence thresholds are specified as a duration in seconds together with
 * the frame rate of the corresponding sequence. The init function computes:
 *   threshold_frames = fps * persistence_s
 */
typedef struct
{
    uint32_t rd_fps;            /**< RD sequence frame rate (frames per second) */
    uint32_t ulp_fps;           /**< ULP sequence frame rate (frames per second) */
    uint32_t rd_persistence_s;  /**< Duration (seconds) to hold RD results after last detection */
    uint32_t ulp_persistence_s; /**< Duration (seconds) to hold ULP results after last detection */
    uint32_t ulp_reporting_s;   /**< Duration (seconds) to report new ULP results after last detection */
} convergence_settings_t;

/**
 * @brief Runtime state and output of the convergence module.
 *
 * The convergence state machine tracks RD and ULP detection streams independently.
 *
 * When RD targets are detected, ULP reporting is enabled. This helps distinguish
 * between persistent objects of interest (e.g. humans) and transient clutter
 * (e.g. robot vacuum cleaner) that may only appear in the ULP sequence.
 *
 * Persistent copies of both RD detections and ULP centroids are maintained
 * and held for the configured persistence duration after the last detection.
 *
 * RD detections persistence is only gated on the presence of RD detections,
 * while ULP detections are kept alive by the presence of RD detections even if
 * ULP detections are temporarily lost.
 *
 * ULP reporting is enabled by the presence of RD detections, and is only disabled
 * after prolonged absence of both RD and ULP detections. While enabled, it allows
 * the system to continue reporting new ULP centroids in the absence of new RD detections.
 *
 * The results_updated flag indicates whether num_rd_detections or
 * num_ulp_centroids were modified during the last convergence_process() call.
 */
typedef struct
{
    // Configuration (set by convergence_init, do not modify at runtime)
    uint32_t rd_persistence_threshold;  /**< RD persistence in frames (rd_fps * rd_persistence_s) */
    uint32_t ulp_persistence_threshold; /**< ULP persistence in frames (ulp_fps * ulp_persistence_s) */
    uint32_t ulp_reporting_threshold;   /**< ULP reporting in frames (ulp_fps * ulp_reporting_s) */

    // Runtime state tracking
    uint32_t rd_frames_without_detections;  /**< Consecutive RD frames with no targets */
    uint32_t ulp_frames_without_detections; /**< Consecutive ULP frames with no clusters */
    bool report_ulp_targets;                /**< True while ULP reporting is active */
    bool results_updated;                   /**< True when output changed during last process call */

    // Output
    int32_t num_ulp_centroids;                                                  /**< Number of ULP centroids */
    point_polar_t ulp_centroids[CLUSTERING_MAX_CENTROIDS];                      /**< ULP cluster centroids */
    uint16_t num_rd_detections;                                                 /**< Number of RD detections */
    motion_detection_processed_targets_t rd_detections[BGT60CUTR13_PROCESSED_MD_TARGET_MAX]; /**< RD detections */
} convergence_t;


/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/

/**
 * @brief Initialise the convergence module.
 *
 * Zeroes all runtime state and computes frame-count persistence thresholds
 * from the supplied settings (threshold = fps * persistence_s).
 *
 * @note ulp_reporting_s must be greater than or equal to ulp_persistence_s to ensure
 * that ULP reporting is only disabled after the persistent ULP centroids are cleared.
 *
 * @param convergence Pointer to the convergence state to initialise
 * @param settings    Pointer to configuration parameters
 * @return 0 on success, -1 if convergence or settings are NULL, -2 if settings are invalid
 */
int32_t convergence_init(convergence_t* convergence, convergence_settings_t* settings);

/**
 * @brief Process one radar frame through the convergence state machine.
 *
 * Should be called once per radar interrupt after radar_motion_detection_process()
 * and clustering_process() have completed.
 *
 * The function updates the persistent RD detections and ULP centroids:
 * - RD path: When an RD frame completes (motion_detection->seq2_done), new targets
 *   overwrite the persistent store and enable ULP reporting. If no targets are seen
 *   for longer than rd_persistence_threshold frames, the persistent RD detections
 *   are cleared.
 * - ULP path: When a ULP frame completes (clustering->ulp_seq_done_flag) and
 *   reporting is active, new clusters overwrite the persistent store. If no clusters
 *   are seen for longer than ulp_persistence_threshold frames, the persistent
 *   ULP centroids are cleared. ULP reporting is disabled after prolonged absence
 *   of both RD and ULP detections i.e. longer than ulp_reporting_threshold frames.
 *
 * The results_updated flag is set to true whenever num_rd_detections or
 * num_ulp_centroids are modified, and false otherwise.
 *
 * @param motion_detection Current motion detection state (RD results)
 * @param clustering       Current clustering state (ULP results)
 * @param convergence      Convergence state to update
 */
void convergence_process(motion_detection_t* motion_detection, clustering_t* clustering, convergence_t* convergence);

#endif // RADAR_CONVERGENCE_H
