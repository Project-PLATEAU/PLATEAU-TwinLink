variable "gcp_project_name" {
  type        = string
  default     = null
  description = "GCPのプロジェクト名を指定してください"
}

variable "gcp_region" {
  type        = string
  default     = "asia-northeast1"
  description = "GCPで使用するregionを指定してください"
}

variable "service_suffix" {
  type        = string
  default     = null
  description = "特定のリソースに付与するためのsuffixを指定してください"
}

variable "subnet_cidr" {
  type        = string
  default     = null
  description = "Cloud NATで使用するsubnetのCIDRを指定してください"
}

variable "service_account_email" {
  type        = string
  default     = null
  description = "cloud schedulerに付与するためのservice account emailを指定してください"
}

variable "sftp_config" {
  type = object({
    host       = string
    port       = string
    user       = string
    passphrase = string
  })
  default     = null
  description = "sftpの接続情報を指定してください"
}

variable "mongo_config" {
  type = object({
    uri     = string
    db_name = string
  })
  default     = null
  description = "mongodbの接続情報を指定してください"
}

variable "cameras" {
  type        = string
  default     = null
  description = "カメラ情報のJSONを記載してください"
}

variable "database_type" {
  type        = string
  default     = "mock"
  description = "データベースの種類を指定してください(mock, inMemory, mongo)"
}