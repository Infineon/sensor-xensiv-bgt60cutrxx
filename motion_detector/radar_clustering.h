/*
 * radar_clustering.h
 *
 * Description: This header file contains the definitions for radar clustering functions used in all code examples.
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

#ifndef RADAR_CLUSTERING_H
#define RADAR_CLUSTERING_H

#include "dbscan.h"
#include "radar_motion_detection.h"
#include <stddef.h>
#include <stdint.h>

/*******************************************************************************
 * Data Structures
 ********************************************************************************/

/** @brief Maximum number of history rows (frames) that can be stored for clustering */
#define CLUSTERING_MAX_HISTORY_ROWS 25

/** @brief Maximum number of detection points per history row (frame) */
#define CLUSTERING_MAX_POINTS_PER_ROW (BGT60CUTR13_PROCESSED_MD_TARGET_MAX)

#if (CLUSTERING_MAX_HISTORY_ROWS >= (UINT8_MAX - 1))
#error "CLUSTERING_MAX_HISTORY_ROWS must be smaller than (UINT8_MAX - 1)"
#endif

/** @brief Total maximum number of points across all history rows */
#define CLUSTERING_MAX_POINTS (CLUSTERING_MAX_HISTORY_ROWS * CLUSTERING_MAX_POINTS_PER_ROW)
#if (CLUSTERING_MAX_POINTS > DBSCAN_MAX_POINTS)
#error "CLUSTERING_MAX_HISTORY_ROWS * CLUSTERING_MAX_POINTS_PER_ROW exceeds DBSCAN_MAX_POINTS"
#endif

/** @brief Maximum number of centroids that can be obtained from clustering */
#define CLUSTERING_MAX_CENTROIDS 8

/**
 * @brief Clustering configuration structure
 *
 * Contains all configurable parameters for the radar clustering algorithm.
 * These parameters control the behavior of both the history management
 * and the DBSCAN clustering algorithm.
 */
typedef struct
{
    int32_t max_rows;                          /**< Maximum history size (number of frames), up to
                                                  CLUSTERING_MAX_HISTORY_ROWS */
    float min_range_m;                         /**< Minimum range in meters; points with range < min_range_m are ignored
                                                */
    int32_t static_check_frame_count;          /**< Minimum number of frames in history before clustering is performed
                                                */
    int32_t cluster_clear_counter_threshold;   /**< Threshold for clearing cluster history counters */
    float dbscan_eps;                          /**< DBSCAN epsilon parameter - maximum distance between neighboring
                                                  points */
    int32_t dbscan_min_samples;               /**< DBSCAN min_samples parameter - minimum points required to form a
                                                 cluster */
} clustering_settings_t;

/**
 * @brief Clustering algorithm state structure
 *
 * Maintains the complete state of the clustering algorithm including configuration,
 * history management, and the underlying DBSCAN algorithm state.
 *
 * @details The clustering history is implemented as a FIFO buffer of frames (rows),
 * where each frame contains detection points from a single radar measurement cycle.
 * The history allows the clustering algorithm to identify persistent targets over time
 * and reduce false detections caused by noise.
 *
 * The frame management uses circular buffer logic:
 * - last_row: Points to the oldest frame in the history buffer
 * - free_row: Points to the next available slot for new frames
 * - The DBSCAN algorithm processes all points across the entire history buffer
 *
 * The clustering process is controlled by frame counting logic that ensures sufficient
 * history is available before performing computationally expensive clustering operations.
 */
typedef struct
{
    // Configuration (copied from settings during initialization)
    int32_t max_rows;                          /**< Maximum number of frames to keep in history */
    float min_range_m;                         /**< Minimum detection range threshold */
    int32_t static_check_frame_count;          /**< Required frames before clustering starts */
    int32_t cluster_clear_counter_threshold;   /**< Counter threshold for clearing clustering state */

    // Runtime state tracking
    int32_t static_check_frame_counter;        /**< Helper counter for required frames before clustering starts */
    int32_t cluster_clear_counter;             /**< Helper counter for clearing clustering state */
    uint8_t last_row;                          /**< Index of the oldest frame in circular buffer */
    uint8_t free_row;                          /**< Index of next free slot in circular buffer */
    point_polar_t centroids[CLUSTERING_MAX_CENTROIDS]; /**< Array to hold computed cluster centroids */
    int32_t num_clusters;                      /**< Number of clusters found in the previous clustering operation */
    bool ulp_seq_done_flag;                    /**< Flag indicating if ULP sequence was detected as done */

    // DBSCAN algorithm state
    dbscan_t dbscan;                          /**< Embedded DBSCAN algorithm instance */
} clustering_t;


/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/

/**
 * @brief Initialize the clustering algorithm with configuration settings
 *
 * Sets up the clustering state structure with the provided configuration and
 * initializes the underlying DBSCAN algorithm. This function must be called
 * before using any other clustering functions.
 *
 * @param clustering Pointer to clustering state structure to initialize
 * @param settings Pointer to clustering configuration settings
 * @return 0 on success, negative error code on failure
 *         -1: Invalid input parameters (NULL pointers)
 *         -2: Invalid configuration values
 *         Other negative values: Error from DBSCAN initialization
 */
int32_t clustering_init(clustering_t* clustering, clustering_settings_t* settings);

/**
 * @brief Process motion detection data through the clustering pipeline
 *
 * This is the main entry point for radar clustering functionality. The function:
 * 1. Extracts detection targets from the motion detection structure
 * 2. Adds new targets to the clustering history buffer
 * 3. Manages the circular history buffer (removes old frames when full)
 * 4. Performs clustering when sufficient history is available
 * 5. Updates frame counters and sequence control logic
 * 6. Returns the number of clusters found or appropriate status codes
 *
 * The clustering is only performed when:
 * - ULP (Ultra Low Power) sequence is complete (motion_detection->ulp_event == true)
 * - Sufficient frames are available (static_check_frame_counter >= static_check_frame_count)
 *
 * @param motion_detection Pointer to motion detection data containing new targets
 * @param clustering Pointer to the clustering algorithm state
 *
 * @return Number of clusters found when clustering was performed
 *         0 if clustering was not performed (insufficient history or no ULP event)
 *         Negative error code on failure:
 *         -1: Invalid input parameters
 *         -2: Error adding points to clustering history
 *         -3: Error during DBSCAN clustering
 *
 * @note The centroids array is located within the clustering_t structure.
 *       Use clustering->centroids and clustering->num_clusters to access the clustering results.
 */
int32_t clustering_process(motion_detection_t* motion_detection, clustering_t* clustering);

#endif // RADAR_CLUSTERING_H
