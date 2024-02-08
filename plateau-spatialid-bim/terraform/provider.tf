provider "google" {
  project = var.gcp_project_name
  region  = var.gcp_region
}

terraform {
  required_providers {
    google = {
      source  = "hashicorp/google"
      version = "~> 4.62.0"
    }
    google-beta = {
      source = "hashicorp/google-beta"
    }
    random = {
      source = "hashicorp/random"
    }
  }
  required_version = ">= v1.3.7"

  backend "gcs" {
    bucket = "plateau-spatialid-bim-terraform-tfstate"
  }
}
