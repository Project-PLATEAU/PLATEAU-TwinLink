resource "google_compute_network" "net" {
  name = "default"

  lifecycle {
    ignore_changes = all
  }
}

resource "google_compute_subnetwork" "subnet" {
  name          = "plateatu-spacial-id-nat-subnet"
  network       = google_compute_network.net.id
  ip_cidr_range = var.subnet_cidr
  region        = var.gcp_region
}

resource "google_vpc_access_connector" "connector" {
  name           = "plateatu-spacial-id"
  region         = var.gcp_region
  max_throughput = 1000

  subnet {
    name       = google_compute_subnetwork.subnet.name
    project_id = var.gcp_project_name
  }
}

resource "google_compute_router" "router" {
  name    = "plateatu-spacial-id"
  region  = var.gcp_region
  network = google_compute_network.net.id

  lifecycle {
    ignore_changes = [bgp]
  }
}

resource "google_compute_router_nat" "nat" {
  name                                = "plateatu-spacial-id"
  min_ports_per_vm                    = 64
  enable_endpoint_independent_mapping = false
  router                              = google_compute_router.router.name
  region                              = google_compute_router.router.region
  nat_ip_allocate_option              = "MANUAL_ONLY"
  nat_ips = [
    "https://www.googleapis.com/compute/v1/projects/${var.gcp_project_name}/regions/${var.gcp_region}/addresses/plateau-spatial-id-prod",
  ] // sftpサーバーのホワイトリストに登録されてある静的IPアドレスを指定する
  source_subnetwork_ip_ranges_to_nat = "ALL_SUBNETWORKS_ALL_IP_RANGES"

  log_config {
    enable = false
    filter = "ALL"
  }
}