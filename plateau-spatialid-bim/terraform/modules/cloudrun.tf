resource "google_cloud_run_service" "plateau_spatialid_bim" {
  name                       = var.service_suffix == "dev" ? "plateau-spatialid-bim-api" : "plateau-spatialid-bim-api-${var.service_suffix}"
  location                   = "asia-northeast1"
  autogenerate_revision_name = true
  metadata {
    annotations = {
      "run.googleapis.com/client-name"    = "gcloud"
      "run.googleapis.com/vpc-access-connector" = google_vpc_access_connector.connector.name
      "run.googleapis.com/ingress"        = "all"
      "run.googleapis.com/ingress-status" = "all"
    }
  }

  template {
    spec {
      containers {
        image = "asia.gcr.io/${var.gcp_project_name}/plateau-spatialid-bim-api:latest"

        ports {
          container_port = 8080
          name           = "http1"
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
          name  = "DATABASE_TYPE"
          value = "mock"
        }
      }
    }
  }
}

data "google_iam_policy" "noauth" {
  binding {
    role = "roles/run.invoker"
    members = [
      "allUsers",
    ]
  }
}

resource "google_cloud_run_service_iam_policy" "plateau_spatialid_bim" {
  location = google_cloud_run_service.plateau_spatialid_bim.location
  project  = google_cloud_run_service.plateau_spatialid_bim.project
  service  = google_cloud_run_service.plateau_spatialid_bim.name

  policy_data = data.google_iam_policy.noauth.policy_data
}
