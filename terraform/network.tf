resource "digitalocean_domain" "cloud" {
  name = "cloud.ructf.org"
}

resource "digitalocean_record" "cloud_a" {
  domain = digitalocean_domain.cloud.id
  type   = "A"
  name   = "@"
  value  = "159.223.216.233"
  ttl    = 30
}


resource "digitalocean_domain" "vpn" {
  name = "vpn.ructf.org"
}

resource "digitalocean_record" "vpn_a" {
  domain = digitalocean_domain.vpn.id
  type   = "A"
  name   = "@"
  value  = digitalocean_droplet.vpn.ipv4_address
  ttl    = 30
}

resource "digitalocean_record" "vpn_wildcard" {
  domain = digitalocean_domain.vpn.id
  type   = "A"
  name   = "*"
  value  = digitalocean_droplet.vpn.ipv4_address
  ttl    = 30
}


resource "digitalocean_vpc" "jury" {
  name     = "jury"
  ip_range = "10.10.10.0/24"
  region   = var.region
}