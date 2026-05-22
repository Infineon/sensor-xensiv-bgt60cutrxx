#include "radar_clustering.h"
#include "radar_clustering_internal.h"
#include <stdbool.h>
#include <stdint.h>

static uint8_t count_rows_in_history(const clustering_t* clustering)
{
    uint8_t num_rows;
    if (clustering->free_row >= clustering->last_row)
    {
        num_rows = clustering->free_row - clustering->last_row;
    }
    else
    {
        // Handle wrap-around case
        num_rows = ((CLUSTERING_MAX_HISTORY_ROWS + 1) - clustering->last_row) + clustering->free_row;
    }
    return num_rows;
}


int32_t clustering_init(clustering_t* clustering, clustering_settings_t* settings)
{
    // Input validation
    if ((clustering == NULL) || (settings == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    if ((settings->max_rows <= 0) || (settings->max_rows > CLUSTERING_MAX_HISTORY_ROWS))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    if (settings->min_range_m < 0.0f)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    if (settings->dbscan_eps_m <= 0.0f)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    if (settings->dbscan_min_samples <= 0)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    // Initialize configuration and internal state
    clustering->max_rows = settings->max_rows;
    clustering->min_range_m = settings->min_range_m;
    clustering->static_check_frame_count = settings->static_check_frame_count;
    clustering->cluster_clear_counter_threshold = settings->cluster_clear_counter_threshold;
    clustering->static_check_frame_counter = 0;
    clustering->cluster_clear_counter = 0;
    clustering->num_clusters = 0;
    clustering->last_row = 0;
    clustering->free_row = 0;
    clustering->ulp_seq_done_flag = false;
    for (uint8_t i = 0; i < CLUSTERING_MAX_CENTROIDS; i++)
    {
        clustering->centroids[i].range_m = 0.0f;
        clustering->centroids[i].azimuth_deg = 0.0f;
        clustering->centroids[i].elevation_deg = 0.0f;
    }

    // Initialize DBSCAN
    clustering->dbscan.eps_m = settings->dbscan_eps_m;
    clustering->dbscan.min_samples = settings->dbscan_min_samples;
    return dbscan_initialize(&clustering->dbscan);
}


int32_t clustering_append_row(clustering_t* clustering, uint8_t num_points, point_polar_t* points)
{
    // Input validation
    if ((clustering == NULL) || (points == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    if (num_points > CLUSTERING_MAX_POINTS_PER_ROW)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    // Purge oldest row if needed (when at or beyond max_rows)
    const uint8_t num_rows = count_rows_in_history(clustering);
    if (num_rows >= clustering->max_rows)
    {
        int32_t status = clustering_purge_oldest_row(clustering);
        if (status < 0)
        {
            return status; // Error during purge
        }
    }

    // Add points meeting range criteria to history
    int32_t points_added = 0;
    for (uint8_t i = 0; i < num_points; i++)
    {
        // Check if point meets minimum range requirement
        if (points[i].range_m >= clustering->min_range_m)
        {
            int32_t status = dbscan_add_point(&clustering->dbscan, &points[i], clustering->free_row);
            if (status < 0)
            {
                return status; // Error adding point
            }
            points_added++;
        }
    }

    // Advance free_row to the next row (wrap around if needed)
    clustering->free_row = (clustering->free_row + 1) % (CLUSTERING_MAX_HISTORY_ROWS + 1);

    return points_added;
}


int32_t clustering_purge_oldest_row(clustering_t* clustering)
{
    // Input validation
    if (clustering == NULL)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    // Return if there are no rows to purge
    const uint8_t num_rows = count_rows_in_history(clustering);
    if (num_rows == 0)
    {
        return 0; // Nothing to purge
    }

    // Remove points associated with the oldest row from history
    const int32_t points_removed = dbscan_remove_points(&clustering->dbscan, clustering->last_row);

    // Advance last_row to the next row (wrap around if needed)
    clustering->last_row = (clustering->last_row + 1) % (CLUSTERING_MAX_HISTORY_ROWS + 1);

    return points_removed;
}


int32_t clustering_runner(clustering_t* clustering, uint8_t num_targets, motion_detection_processed_targets_t* targets)
{
    if ((clustering == NULL) || (targets == NULL) || (num_targets > CLUSTERING_MAX_POINTS_PER_ROW))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    // Append new points to clustering history
    static point_polar_t points[CLUSTERING_MAX_POINTS_PER_ROW];
    for (uint8_t i = 0; i < num_targets; i++)
    {
        points[i].range_m = targets[i].range_m;
        points[i].azimuth_deg = targets[i].azimuth_deg;
        points[i].elevation_deg = targets[i].elevation_deg;
    }
    const int32_t points_added = clustering_append_row(clustering, num_targets, points);
    if (points_added < 0)
    {
        return points_added; // Propagate error when adding points
    }

    // Perform DBSCAN clustering
    const int num_clusters = dbscan_cluster(&clustering->dbscan);
    if (num_clusters < 0)
    {
        return num_clusters; // Propagate error during DBSCAN clustering
    }

    return num_clusters;
}


int32_t clustering_get_centroids(clustering_t* clustering, point_polar_t* centroids)
{
    if ((clustering == NULL) || (centroids == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    int32_t index = 0;
    while (index < clustering->dbscan.num_clusters)
    {
        if (index >= CLUSTERING_MAX_CENTROIDS)
        {
            break;  // Prevent overflow
        }

        int32_t status = dbscan_get_cluster_centroid_polar(&clustering->dbscan, index, &centroids[index]);
        if (status < 0)
        {
            return status;
        }
        index++;
    }

    return index;  // number of centroids retrieved
}


int32_t clustering_process(motion_detection_t* motion_detection, clustering_t* clustering)
{
    if ((clustering == NULL) || (motion_detection == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    // process RD sequence results
    if (motion_detection->seq2_done)
    {
        // check if RD sequence detected any moving targets
        const uint8_t rd_events = motion_detection->number_of_detected_targets;
        if (rd_events > 0)
        {
            // pause clustering as long as moving targets are being detected
            clustering->static_check_frame_counter = 0;
        }
        else if (rd_events == 0)
        {
            // count consecutive number of frames in absence of moving target detections
            if (clustering->static_check_frame_counter < clustering->static_check_frame_count)
            {
                clustering->static_check_frame_counter++;
            }
        }
    }

    // detect ULP sequence completion
    if (motion_detection->seq1_done)
    {
        // toggle flag to track ULP sequence completion at every 2nd event
        clustering->ulp_seq_done_flag = !clustering->ulp_seq_done_flag;

        // ensure flag stays synchronized to ULP event
        if (motion_detection->ulp_event)
        {
            clustering->ulp_seq_done_flag = true;
        }
    }

    // process ULP sequence results
    if (clustering->ulp_seq_done_flag)
    {
        int32_t num_clusters = 0;

        // skip clustering until moving targets have been absent for a certain number of frames
        if (clustering->static_check_frame_counter >= clustering->static_check_frame_count)
        {
            num_clusters = clustering_runner(clustering, motion_detection->number_of_detected_targets,
                                             motion_detection->processed_targets);
            if (num_clusters < 0)
            {
                return num_clusters; // Propagate error running clustering
            }
            // limit the number of clusters to max centroids
            if (num_clusters > CLUSTERING_MAX_CENTROIDS)
            {
                num_clusters = CLUSTERING_MAX_CENTROIDS;
            }
            if (num_clusters > 0)
            {
                // update detected centroid results
                int32_t status = clustering_get_centroids(clustering, clustering->centroids);
                if (status < 0)
                {
                    return status; // Propagate error getting centroids
                }
                clustering->num_clusters = num_clusters;  // update number of clusters
                clustering->cluster_clear_counter = 0;
            }
        }

        if (num_clusters == 0)
        {
            // if no clusters were detected, we keep reporting last detected centroids for a given number of frames
            if (clustering->cluster_clear_counter < clustering->cluster_clear_counter_threshold)
            {
                clustering->cluster_clear_counter++;
            }
            else
            {
                clustering->num_clusters = 0;
            }
        }

        // clear remaining centroids
        for (uint8_t i = clustering->num_clusters; i < CLUSTERING_MAX_CENTROIDS; i++)
        {
            clustering->centroids[i].range_m = 0.0f;
            clustering->centroids[i].azimuth_deg = 0.0f;
            clustering->centroids[i].elevation_deg = 0.0f;
        }
    }

    return clustering->num_clusters;
}
