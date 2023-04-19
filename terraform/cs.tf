resource "digitalocean_droplet" "cs-master" {
  image              = "ubuntu-20-04-x64"
  name               = "cs-master"
  region             = var.region
  size               = "s-8vcpu-16gb"
  private_networking = true
  vpc_uuid           = digitalocean_vpc.jury.id
  ssh_keys = [
    data.digitalocean_ssh_key.mikhalych.id,
    data.digitalocean_ssh_key.capitan_banana.id,
    data.digitalocean_ssh_key.bay.id,
    data.digitalocean_ssh_key.vaspahomov.id
  ]
}

resource "digitalocean_droplet" "cs-checker" {
  image              = "ubuntu-20-04-x64"
  name               = "cs-checker"
  region             = var.region
  size               = "s-2vcpu-4gb"
  private_networking = true
  vpc_uuid           = digitalocean_vpc.jury.id
  ssh_keys = [
    data.digitalocean_ssh_key.mikhalych.id,
    data.digitalocean_ssh_key.capitan_banana.id,
    data.digitalocean_ssh_key.bay.id,
    data.digitalocean_ssh_key.vaspahomov.id
  ]
}