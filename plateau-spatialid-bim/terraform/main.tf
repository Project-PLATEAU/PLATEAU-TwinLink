module "prod" {
  source = "./modules"

  gcp_project_name      = var.gcp_project_name
  service_account_email = var.service_account_email
  subnet_cidr           = var.subnet_cidr
  sftp_config           = var.sftp_config
  mongo_config          = var.mongo_config
  cameras               = var.cameras
  database_type         = var.database_type

  service_suffix = "prod"
}

module "dev" {
  source = "./modules"

  gcp_project_name      = var.gcp_project_name
  service_account_email = var.service_account_email
  subnet_cidr           = var.subnet_cidr
  sftp_config           = var.sftp_config
  mongo_config          = var.mongo_config
  cameras               = var.cameras
  database_type         = var.database_type

  service_suffix = "dev"
}
