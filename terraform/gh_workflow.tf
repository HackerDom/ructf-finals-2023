resource "digitalocean_droplet" "gh-workflow" {
  image              = "ubuntu-20-04-x64"
  name               = "gh-workflow"
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