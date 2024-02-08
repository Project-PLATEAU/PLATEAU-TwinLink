resource "google_cloud_scheduler_job" "sftp-batch-job-scheduler" {
  name      = "sftp-batch-job-scheduler-${var.service_suffix}"
  region    = var.gcp_region
  schedule  = "*/3 * * * *"
  time_zone = "Asia/Tokyo"

  http_target {
    uri         = "https://asia-northeast1-run.googleapis.com/apis/run.googleapis.com/v1/namespaces/${var.gcp_project_name}/jobs/sftp-batch-job-${var.service_suffix}:run"
    http_method = "POST"

    oauth_token {
      service_account_email = var.service_account_email
    }
  }
  retry_config {
    max_backoff_duration = "3600s"
    max_doublings        = 5
    max_retry_duration   = "0s"
    min_backoff_duration = "5s"
    retry_count          = 0
  }
}
