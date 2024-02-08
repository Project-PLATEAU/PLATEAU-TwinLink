resource "google_cloud_run_v2_job" "sftp-batch-job" {
  client   = "cloud-console"
  name     = "sftp-batch-job-${var.service_suffix}"
  location = var.gcp_region

  template {
    task_count = 1
    template {
      containers {
        image = "asia.gcr.io/${var.gcp_project_name}/plateau-spatialid-bim-batch:latest"

        ports {
          container_port = 8080
          name           = "h2c"
        }

        env {
          name  = "SFTP_HOST"
          value = var.sftp_config.host
        }
        env {
          name  = "SFTP_PORT"
          value = var.sftp_config.port
        }
        env {
          name  = "SFTP_USERNAME"
          value = var.sftp_config.user
        }
        env {
          name  = "SFTP_PASSPHRASE"
          value = var.sftp_config.passphrase
        }
        env {
          name  = "MONGO_URI"
          value = var.mongo_config.uri
        }
        env {
          name  = "MONGO_DB_NAME"
          value = var.mongo_config.db_name
        }
        env {
          name  = "CAMERAS"
          value = var.cameras
        }
        env {
          name  = "DATABASE_TYPE" // batchではmongoのみ対応している
          value = "mongo"
        }
        env {
          name = "SFTP_PRIVATE_KEY"

          value_source {
            secret_key_ref {
              secret  = "sftp-pem"
              version = "latest"
            }
          }
        }
      }

      vpc_access {
        connector = "projects/${var.gcp_project_name}/locations/asia-northeast1/connectors/plateatu-spacial-id"
        egress    = "ALL_TRAFFIC"
      }
    }
  }

}
