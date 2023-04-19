resource "digitalocean_droplet" "vpn" {
  image              = "ubuntu-20-04-x64"
  name               = "vpn"
  region             = var.region
  size               = "g-2vcpu-8gb"
  private_networking = true
  vpc_uuid           = digitalocean_vpc.jury.id
  ssh_keys = [
    data.digitalocean_ssh_key.mikhalych.id,
    data.digitalocean_ssh_key.capitan_banana.id,
    data.digitalocean_ssh_key.bay.id,
    data.digitalocean_ssh_key.vaspahomov.id
  ]
}

resource "digitalocean_volume" "vpn" {
  region                  = var.region
  name                    = "vpn"
  size                    = 500
  initial_filesystem_type = "ext4"
}

resource "digitalocean_volume_attachment" "vpn" {
  droplet_id = digitalocean_droplet.vpn.id
  volume_id  = digitalocean_volume.vpn.id
}
