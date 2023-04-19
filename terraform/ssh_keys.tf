data "digitalocean_ssh_key" "mikhalych" {
  name = "mikhalych_deploy_key"
}

data "digitalocean_ssh_key" "capitan_banana" {
  name = "d.lukshto-deploy-key"
}

data "digitalocean_ssh_key" "bay" {
  name = "bay"
}

data "digitalocean_ssh_key" "vaspahomov" {
  name = "vaspahomov"
}