#include "dbscan.h"
#include "xensiv_bgt60cutrxx.h"
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifndef M_PI
#define M_PI (3.14159265f)
#endif

#define DBSCAN_SIN_COS_SAFE_LIMIT_RAD (31.0f)

/**
 * @brief Convert polar coordinates to Cartesian coordinates
 *
 * @param polar Pointer to polar coordinates
 * @param cartesian Pointer to output Cartesian coordinates
 * @return int32_t Returns 0 on success, or a negative error code on failure
 */
static int32_t polar_to_cartesian(const point_polar_t* polar, point_3d_t* cartesian)
{
    if ((!isfinite(polar->range_m)) || (!isfinite(polar->azimuth_deg)) || (!isfinite(polar->elevation_deg)))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    if ((polar->azimuth_deg < -90.0f) || (polar->azimuth_deg > 90.0f) || (polar->elevation_deg < -90.0f) ||
        (polar->elevation_deg > 90.0f))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    float range = polar->range_m;
    float azimuth_rad = polar->azimuth_deg * (float)M_PI / 180.0f;
    float elevation_rad = polar->elevation_deg * (float)M_PI / 180.0f;

    // satisfy math library sinf/cosf bounding checks (prevents static analysis warnings)
    if ((azimuth_rad < -DBSCAN_SIN_COS_SAFE_LIMIT_RAD) || (azimuth_rad > DBSCAN_SIN_COS_SAFE_LIMIT_RAD) ||
        (elevation_rad < -DBSCAN_SIN_COS_SAFE_LIMIT_RAD) || (elevation_rad > DBSCAN_SIN_COS_SAFE_LIMIT_RAD))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    float cos_elevation = cosf(elevation_rad);

    cartesian->x = range * cos_elevation * cosf(azimuth_rad);
    cartesian->y = range * cos_elevation * sinf(azimuth_rad);
    cartesian->z = range * sinf(elevation_rad);

    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


static inline float rad2deg(float rad)
{
    return rad * (180.0f / (float)M_PI);
}


static inline float wrap_deg_pm180(float a)
{
    // Wrap to (-180, 180]
    bool needs_wrapping = true;
    while (needs_wrapping)
    {
        if (a <= -180.0f)
        {
            a += 360.0f;
        }
        else
        {
            needs_wrapping = false;
        }
    }
    needs_wrapping = true;
    while (needs_wrapping)
    {
        if (a > 180.0f)
        {
            a -= 360.0f;
        }
        else
        {
            needs_wrapping = false;
        }
    }
    return a;
}


/**
 * @brief Convert Cartesian coordinates to polar coordinates
 *
 * @param cartesian Pointer to Cartesian coordinates
 * @param polar Pointer to output polar coordinates
 */
static void cartesian_to_polar(const point_3d_t* cartesian, point_polar_t* polar)
{
    if ((!isfinite(cartesian->x)) || (!isfinite(cartesian->y)) || (!isfinite(cartesian->z)))
    {
        polar->range_m = 0.0f;
        polar->azimuth_deg = 0.0f;
        polar->elevation_deg = 0.0f;
        return;
    }

    // Range (Euclidean norm)
    float rxy = hypotf(cartesian->x, cartesian->y); // distance projected onto x-y plane
    float r   = sqrtf(cartesian->x*cartesian->x + cartesian->y*cartesian->y + cartesian->z*cartesian->z);
    polar->range_m = r;

    if (r <= FLT_EPSILON)
    {
        // For zero vector, define angles as 0
        polar->azimuth_deg   = 0.0f;
        polar->elevation_deg = 0.0f;
        return;
    }

    // Azimuth: angle around z-axis from +x toward +y
    float az = atan2f(cartesian->y, cartesian->x); // radians
    // Elevation: angle above x-y plane toward +z
    float el = atan2f(cartesian->z, rxy); // radians

    polar->azimuth_deg   = wrap_deg_pm180(rad2deg(az));
    polar->elevation_deg = rad2deg(el);
}


static int32_t get_unused_point_index(dbscan_t* result)
{
    int32_t index = 0;
    while (index < DBSCAN_MAX_POINTS)
    {
        if (result->points_index[index] == DBSCAN_UNUSED_POINT)
        {
            break;
        }
        index++;
    }
    return index;
}


int32_t dbscan_initialize(dbscan_t* result)
{
    if (result == NULL)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    // Basic parameter validation
    if (result->eps_m <= 0.0f)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    if (result->min_samples <= 0)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    // Initialize state
    result->num_clusters = 0;

    // Initialize point storage and clustering results
    for (int i = 0; i < DBSCAN_MAX_POINTS; i++)
    {
        result->labels[i] = DBSCAN_UNASSIGNED_LABEL;
        result->points_index[i] = DBSCAN_UNUSED_POINT;
    }

    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


int32_t dbscan_get_num_points(dbscan_t* result)
{
    if (result == NULL)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    int num_points = 0;
    for (int i = 0; i < DBSCAN_MAX_POINTS; i++)
    {
        if (result->points_index[i] != DBSCAN_UNUSED_POINT)
        {
            num_points++;
        }
    }

    return num_points;
}


int32_t dbscan_add_point(dbscan_t* result, const point_polar_t* point, uint8_t index)
{
    if ((result == NULL) || (point == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    if (index == DBSCAN_UNUSED_POINT)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    const int32_t new_point = get_unused_point_index(result);
    if (new_point >= DBSCAN_MAX_POINTS)
    {
        return XENSIV_BGT60CUTRXX_STATUS_NO_MEMORY; // No space for more points
    }

    // Convert point to Cartesian and add
    int32_t status = polar_to_cartesian(point, &result->points[new_point]);
    if (status < 0)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    // Mark point as used
    result->points_index[new_point] = index;  // associate to given index

    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


int32_t dbscan_remove_points(dbscan_t* result, uint8_t index)
{
    if (result == NULL)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    if (index == DBSCAN_UNUSED_POINT)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    // Remove all points associated to given index
    int points_removed = 0;
    for (int32_t i = 0; i < DBSCAN_MAX_POINTS; i++)
    {
        if (result->points_index[i] == index)
        {
            // Mark point as unused
            result->points_index[i] = DBSCAN_UNUSED_POINT;
            points_removed++;
        }
    }

    return points_removed;
}


// Stack-based neighbor finding with early termination
static int32_t find_neighbors(const dbscan_t* result, int point_idx,
                              int32_t neighbors[DBSCAN_MAX_POINTS])
{
    int neighbor_count           = 0;
    const point_3d_t* query_point = &result->points[point_idx];
    const float eps_squared      = result->eps_m * result->eps_m;

    // Optimized neighbor search with squared distance comparison
    for (int i = 0; i < DBSCAN_MAX_POINTS; i++)
    {
        // Skip unused points
        if (result->points_index[i] == DBSCAN_UNUSED_POINT)
        {
            continue;
        }

        if (i == point_idx)
        {
            continue;
        }

        const point_3d_t* candidate = &result->points[i];

        // Quick rejection tests before expensive distance calculation
        float dx = query_point->x - candidate->x;
        if ((dx > result->eps_m) || (dx < -result->eps_m))
        {
            continue;
        }

        float dy = query_point->y - candidate->y;
        if ((dy > result->eps_m) || (dy < -result->eps_m))
        {
            continue;
        }

        float dz = query_point->z - candidate->z;
        if ((dz > result->eps_m) || (dz < -result->eps_m))
        {
            continue;
        }

        // Only calculate actual distance if quick tests pass
        float dist_squared = dx * dx + dy * dy + dz * dz;
        if (dist_squared <= eps_squared)
        {
            neighbors[neighbor_count++] = i;
        }
    }

    return neighbor_count;
}


// Optimized region growing with stack-based recursion avoidance
static void expand_cluster(dbscan_t* result, int point_idx,
                           int32_t neighbors[DBSCAN_MAX_POINTS], int neighbor_count,
                           int32_t cluster_id)
{
    // Stack for iterative processing (avoiding recursion)
    int32_t stack[DBSCAN_MAX_POINTS];
    int stack_size = 0;

    // Mark seed point as part of cluster
    result->labels[point_idx] = cluster_id;

    // Add all neighbors to processing stack
    for (int i = 0; i < neighbor_count; i++)
    {
        stack[stack_size++] = neighbors[i];
    }

    // Process stack iteratively
    while (stack_size > 0)
    {
        int current_point = stack[--stack_size];

        // Skip unused points
        if (result->points_index[current_point] == DBSCAN_UNUSED_POINT)
        {
            continue;
        }

        // Skip if already processed
        if ((result->labels[current_point] != DBSCAN_UNASSIGNED_LABEL) &&
            (result->labels[current_point] != DBSCAN_NOISE_LABEL))
        {
            continue;
        }

        // Add to cluster
        result->labels[current_point] = cluster_id;

        // Find neighbors of current point
        int32_t current_neighbors[DBSCAN_MAX_POINTS];
        int current_neighbor_count = find_neighbors(result, current_point, current_neighbors);

        // If enough neighbors, expand further
        if (current_neighbor_count >= result->min_samples)
        {
            for (int i = 0; i < current_neighbor_count; i++)
            {
                int neighbor_idx = current_neighbors[i];

                // Only add unprocessed points to stack
                if ((result->labels[neighbor_idx] == DBSCAN_UNASSIGNED_LABEL) ||
                    (result->labels[neighbor_idx] == DBSCAN_NOISE_LABEL))
                {
                    // Avoid duplicates in stack
                    int already_in_stack = 0;
                    for (int j = 0; j < stack_size; j++)
                    {
                        if (stack[j] == neighbor_idx)
                        {
                            already_in_stack = 1;
                            break;
                        }
                    }

                    if (!already_in_stack && (stack_size < DBSCAN_MAX_POINTS))
                    {
                        stack[stack_size++] = neighbor_idx;
                    }
                }
            }
        }
    }
}


int32_t dbscan_cluster(dbscan_t* result)
{
    // Input validation
    if (result == NULL)
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }
    if ((result->eps_m <= 0.0f) || (result->min_samples <= 0))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    // Reset all cluster results
    for (int i = 0; i < DBSCAN_MAX_POINTS; i++)
    {
        result->labels[i] = DBSCAN_UNASSIGNED_LABEL;
    }

    int32_t cluster_id = 0;

    // Main DBSCAN algorithm loop
    for (int i = 0; i < DBSCAN_MAX_POINTS; i++)
    {
        // Skip unused points
        if (result->points_index[i] == DBSCAN_UNUSED_POINT)
        {
            continue;
        }

        // Skip already processed points
        if (result->labels[i] != DBSCAN_UNASSIGNED_LABEL)
        {
            continue;
        }

        // Find neighbors
        int32_t neighbors[DBSCAN_MAX_POINTS];
        int neighbor_count = find_neighbors(result, i, neighbors);

        // Check if point is core point
        int32_t min_neighbors = result->min_samples - 1;
        if (neighbor_count < min_neighbors)
        {
            // Mark as noise (may be changed later if becomes border point)
            result->labels[i] = DBSCAN_NOISE_LABEL;
        }
        else
        {
            // Start new cluster
            expand_cluster(result, i, neighbors, neighbor_count, cluster_id);
            cluster_id++;
        }
    }

    result->num_clusters = cluster_id;
    return cluster_id;
}


// Utility function to get cluster centroid
int32_t dbscan_get_cluster_centroid_3d(const dbscan_t* result, int32_t cluster_index, point_3d_t* centroid)
{
    // Input validation
    if ((result == NULL) || (centroid == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    if ((cluster_index < 0) || (cluster_index >= result->num_clusters))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    // Initialize centroid accumulator
    point_3d_t accumulator = { 0.0f, 0.0f, 0.0f };
    int32_t cluster_size = 0;

    // Accumulate points for the specified cluster
    for (int i = 0; i < DBSCAN_MAX_POINTS; i++)
    {
        if (result->labels[i] == cluster_index)
        {
            accumulator.x += result->points[i].x;
            accumulator.y += result->points[i].y;
            accumulator.z += result->points[i].z;
            cluster_size++;
        }
    }

    // Calculate averages
    if (cluster_size > 0)
    {
        float inv_size = 1.0f / (float)cluster_size;
        centroid->x = accumulator.x * inv_size;
        centroid->y = accumulator.y * inv_size;
        centroid->z = accumulator.z * inv_size;
    }
    else
    {
        // Empty cluster
        centroid->x = 0.0f;
        centroid->y = 0.0f;
        centroid->z = 0.0f;
    }

    return cluster_size;
}


int32_t dbscan_get_cluster_centroid_polar(const dbscan_t* result, int32_t cluster_index, point_polar_t* centroid)
{
    point_3d_t centroid_cartesian;
    int32_t cluster_size = dbscan_get_cluster_centroid_3d(result, cluster_index, &centroid_cartesian);
    if (cluster_size < 0)
    {
        return cluster_size; // Propagate error
    }

    if (cluster_size > 0)
    {
        cartesian_to_polar(&centroid_cartesian, centroid);
    }

    return cluster_size;
}
