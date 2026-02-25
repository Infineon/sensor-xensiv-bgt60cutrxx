#ifndef RADAR_CLUSTERING_INTERNAL_H
#define RADAR_CLUSTERING_INTERNAL_H

#include "radar_clustering.h"
#include "dbscan.h"
#include "radar_motion_detection.h"
#include <stdint.h>

/**
 * @brief Appends polar points to the clustering history as part of a new row
 *
 * This function appends a given number of points (between 0 and CLUSTERING_MAX_POINTS_PER_ROW)
 * to the clustering history while updating the row tracking information to maintain insertion order.
 *
 * The history is treated as a FIFO, so when the maximum history size (max_rows) is reached,
 * the oldest row is purged before a new row is appended.
 *
 * Points with range less than min_range_m are ignored and not added to history.
 *
 * @param clustering Pointer to the clustering_t structure
 * @param num_points Number of polar points to append as a new row
 * @param points Array of polar points
 * @return int32_t Number of points added, negative value on error
 */
int32_t clustering_append_row(clustering_t* clustering, uint8_t num_points, point_polar_t* points);

/**
 * @brief Purges all points associated with the oldest row from clustering history
 *
 * This function identifies points associated with the oldest row and removes them from clustering history.
 *
 * @param clustering Pointer to the clustering_t structure
 * @return int32_t Number of points removed, negative value on error
 */
int32_t clustering_purge_oldest_row(clustering_t* clustering);

/**
 * @brief Executes clustering on motion detection targets
 *
 * Converts motion detection targets to polar points, appends them to the
 * clustering history, and performs DBSCAN clustering to identify clusters.
 *
 * @param clustering Pointer to the clustering instance
 * @param num_targets Number of targets in the targets array
 * @param targets Array of motion detection processed targets
 *
 * @return Number of clusters found on success, negative error code on failure
 *         -1: Invalid input parameters
 *         -2: Error adding points or during clustering
 */
int32_t clustering_runner(clustering_t* clustering, uint8_t num_targets, motion_detection_processed_targets_t* targets);

/**
 * @brief Extract cluster centroids in polar coordinates
 *
 * Calculates and retrieves the centroid (center point) of each cluster found by
 * the DBSCAN algorithm. The centroids are returned in polar coordinate format
 * (range, azimuth, elevation) which is the coordinate system used by the sensor.
 *
 * This function should be called after clustering_process() has successfully
 * identified clusters. The caller is responsible for ensuring the centroids array
 * has sufficient space for all clusters.
 *
 * @param clustering Pointer to the clustering state containing cluster results
 * @param centroids Output array to store cluster centroids in polar coordinates.
 *                  Must have space for at least clustering->dbscan.num_clusters elements.
 *
 * @return Number of centroids successfully extracted (should equal num_clusters)
 *         Negative error code on failure:
 *         -1: Invalid input parameters (NULL pointers)
 *         -2: No clusters available (num_clusters == 0)
 *         -3: Error calculating centroid from DBSCAN data
 *         Other negative values: Propagated errors from dbscan_get_cluster_centroid_polar()
 *
 * @note The centroids array must be allocated by the caller with sufficient space.
 *       Use clustering->dbscan.num_clusters to determine the required array size.
 */
int32_t clustering_get_centroids(clustering_t* clustering, point_polar_t* centroids);

#endif // RADAR_CLUSTERING_INTERNAL_H
