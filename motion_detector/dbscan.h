/*
 * dbscan.h
 *
 * Description: This header file contains the definitions for DBSCAN clustering algorithm used in all code examples.
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

#ifndef DBSCAN_H
#define DBSCAN_H

#include <stdint.h>

/** @brief Maximum number of points that can be processed by DBSCAN algorithm */
#define DBSCAN_MAX_POINTS       (25 * 8)  // 8 == BGT60CUTR13_PROCESSED_MD_TARGET_MAX

/** @brief Label assigned to noise points (outliers) in DBSCAN clustering */
#define DBSCAN_NOISE_LABEL      -1

/** @brief Label assigned to points that haven't been processed yet */
#define DBSCAN_UNASSIGNED_LABEL -2

/** @brief Marker value indicating an unused point slot in the points array */
#define DBSCAN_UNUSED_POINT     ((uint8_t)0xff)

/**
 * @brief 3D point in Cartesian coordinates
 */
typedef struct
{
    float x, y, z;
} point_3d_t;

/**
 * @brief 3D point in polar (spherical) coordinates
 * range_m:      distance from origin
 * azimuth_deg:  angle in x-y plane from +x toward +y, in degrees (wrapped to (-180,180])
 * elevation_deg:angle from x-y plane toward +z, in degrees ([-90,90])
 */
typedef struct
{
    float range_m, azimuth_deg, elevation_deg;
} point_polar_t;

/**
 * @brief DBSCAN algorithm state and data structure
 *
 * Contains all input parameters, point data, and output results for the DBSCAN clustering algorithm.
 * The structure maintains both input data (points and algorithm parameters) and output results
 * (cluster labels and cluster count).
 */
typedef struct
{
    // Input parameters
    float eps_m;                                /**< Maximum distance between two points to be considered neighbors */
    int32_t min_samples;                        /**< Minimum number of points required to form a cluster */
    uint8_t points_index[DBSCAN_MAX_POINTS];    /**< Index mapping for points in the dataset */
    point_3d_t points[DBSCAN_MAX_POINTS];        /**< Array of 3D points in Cartesian coordinates */

    // Output results
    int32_t labels[DBSCAN_MAX_POINTS];          /**< Cluster labels for each point */
    int32_t num_clusters;                       /**< Total number of clusters found */
} dbscan_t;

/**
 * @brief Initialize the DBSCAN data structure
 *
 * Resets all internal state variables and prepares the DBSCAN structure for a new clustering operation.
 * This function must be called before adding points or performing clustering.
 *
 * @param result Pointer to DBSCAN structure to initialize
 * @return 0 on success, negative value on error
 */
int32_t dbscan_initialize(dbscan_t* result);

/**
 * @brief Get the current number of points in the DBSCAN dataset
 *
 * @param result Pointer to DBSCAN result structure
 * @return Number of points currently stored, or -1 on error
 */
int32_t dbscan_get_num_points(dbscan_t* result);

/**
 * @brief Add a point to the DBSCAN dataset
 *
 * Converts a polar coordinate point to Cartesian coordinates and adds it to the DBSCAN
 * point set for clustering. The point is assigned the specified index for tracking purposes.
 *
 * @param result Pointer to DBSCAN result structure
 * @param point Pointer to polar coordinate point to add
 * @param index Index value to associate with this point
 * @return 0 on success, negative value on error (e.g., dataset full)
 */
int32_t dbscan_add_point(dbscan_t* result, const point_polar_t* point, uint8_t index);

/**
 * @brief Remove all points with the specified index from the dataset
 *
 * Removes all points that were added with the given index value. This is useful for
 * removing points from a specific frame or time period from the clustering dataset.
 *
 * @param result Pointer to DBSCAN result structure
 * @param index Index value of points to remove
 * @return Number of points removed, or negative value on error
 */
int32_t dbscan_remove_points(dbscan_t* result, uint8_t index);

/**
 * @brief Perform DBSCAN clustering on the current point set
 *
 * Executes the DBSCAN clustering algorithm on all points currently in the dataset.
 * After completion, each point will be assigned a cluster label or marked as noise.
 * The total number of clusters found is stored in num_clusters.
 *
 * @param result Pointer to DBSCAN result structure containing points and parameters
 * @return 0 on success, negative value on error
 */
int32_t dbscan_cluster(dbscan_t* result);

/**
 * @brief Get cluster centroid in cartesian coordinates for given cluster index
 * @param result Pointer to DBSCAN result structure
 * @param cluster_index Index of the cluster to get centroid for
 * @param centroid Output pointer for cluster centroid (in cartesian coordinates)
 * @return Size of cluster associated to given cluster index, or -1 on error
 */
int32_t dbscan_get_cluster_centroid_3d(const dbscan_t* result, int32_t cluster_index, point_3d_t* centroid);

/**
 * @brief Get cluster centroid in polar coordinates for given cluster index
 * @param result Pointer to DBSCAN result structure
 * @param cluster_index Index of the cluster to get centroid for
 * @param centroid Output pointer for cluster centroid (in polar coordinates)
 * @return Size of cluster associated to given cluster index, or -1 on error
 */
int32_t dbscan_get_cluster_centroid_polar(const dbscan_t* result, int32_t cluster_index, point_polar_t* centroid);

#endif // DBSCAN_H
