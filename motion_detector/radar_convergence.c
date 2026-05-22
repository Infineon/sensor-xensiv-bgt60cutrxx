#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "radar_convergence.h"


int32_t convergence_init(convergence_t* convergence, convergence_settings_t* settings)
{
    if ((convergence == NULL) || (settings == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER;
    }

    if ((settings->rd_fps == 0) || (settings->ulp_fps == 0) ||
        (settings->rd_persistence_s == 0) || (settings->ulp_persistence_s == 0) ||
        (settings->ulp_reporting_s < settings->ulp_persistence_s))
    {
        return XENSIV_BGT60CUTRXX_STATUS_INVALID_PARAMETER;
    }

    memset(convergence, 0, sizeof(convergence_t));
    convergence->rd_persistence_threshold = settings->rd_persistence_s * settings->rd_fps;
    convergence->ulp_persistence_threshold = settings->ulp_persistence_s * settings->ulp_fps;
    convergence->ulp_reporting_threshold = settings->ulp_reporting_s * settings->ulp_fps;

    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


void convergence_process(motion_detection_t* motion_detection, clustering_t* clustering, convergence_t* convergence)
{
    convergence->results_updated = false;

    bool rd_frame_done = motion_detection->seq2_done;
    bool rd_targets_detected = (rd_frame_done && motion_detection->number_of_detected_targets > 0);
    bool ulp_frame_done = (motion_detection->seq1_done && clustering->ulp_seq_done_flag);
    bool ulp_targets_detected = (ulp_frame_done && clustering->num_clusters > 0);

    if (rd_frame_done)
    {
        if (rd_targets_detected)
        {
            // existence of RD detections triggers activation of ULP clustering report
            convergence->report_ulp_targets = true;

            // existence of RD detections also keeps both persistent RD targets and ULP centroids alive
            convergence->ulp_frames_without_detections = 0;
            convergence->rd_frames_without_detections = 0;

            // update persistent RD detections
            convergence->num_rd_detections = motion_detection->number_of_detected_targets;
            memcpy(convergence->rd_detections, motion_detection->processed_targets, sizeof(convergence->rd_detections));
            convergence->results_updated = true;
        }
        else
        {
            convergence->rd_frames_without_detections +=
                (convergence->rd_frames_without_detections == UINT32_MAX) ? 0 : 1;
            if ((convergence->rd_frames_without_detections > convergence->rd_persistence_threshold) &&
                (convergence->num_rd_detections > 0))
            {
                // clear persistent RD targets after prolonged absence of detections
                convergence->num_rd_detections = 0;
                memset(convergence->rd_detections, 0, sizeof(convergence->rd_detections));
                convergence->results_updated = true;
            }
        }
    }

    if (ulp_frame_done && convergence->report_ulp_targets)
    {
        if (ulp_targets_detected)
        {
            // existence of ULP detections keeps persistent ULP centroids alive
            convergence->ulp_frames_without_detections = 0;

            // update persistent ULP centroids
            convergence->num_ulp_centroids = clustering->num_clusters;
            memcpy(convergence->ulp_centroids, clustering->centroids, sizeof(convergence->ulp_centroids));
            convergence->results_updated = true;
        }
        else
        {
            convergence->ulp_frames_without_detections +=
                (convergence->ulp_frames_without_detections == UINT32_MAX) ? 0 : 1;
            if ((convergence->ulp_frames_without_detections > convergence->ulp_persistence_threshold) &&
                (convergence->num_ulp_centroids > 0))
            {
                // clear persistent ULP centroids after prolonged absence of detections
                convergence->num_ulp_centroids = 0;
                memset(convergence->ulp_centroids, 0, sizeof(convergence->ulp_centroids));
                convergence->results_updated = true;
            }

            if ((convergence->ulp_frames_without_detections > convergence->ulp_reporting_threshold) &&
                (convergence->report_ulp_targets))
            {
                // prolonged absence of detections also triggers deactivation of ULP clustering report
                convergence->report_ulp_targets = false;
            }
        }
    }
}
