resource "digitalocean_floating_ip" "cs-master" {
  region = var.region
}

resource "digitalocean_floating_ip_assignment" "cs-master" {
  ip_address = digitalocean_floating_ip.cs-master.ip_address
  droplet_id = digitalocean_droplet.cs-master.id
}