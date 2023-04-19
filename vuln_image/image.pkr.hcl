packer {
  required_plugins {
    digitalocean = {
      version = ">= 1.0.0"
      source  = "github.com/hashicorp/digitalocean"
    }
  }
}

variable "api_token" {
  type = string
}

source "digitalocean" "vuln_image" {
  droplet_name  = "ructf-finals-2023-{{timestamp}}"
  snapshot_name = "ructf-finals-2023-{{timestamp}}"
  api_token     = var.api_token
  image         = "ubuntu-20-04-x64"
  region        = "ams3"
  size          = "s-4vcpu-8gb"
  ssh_username  = "root"
}

build {
  sources = ["source.digitalocean.vuln_image"]

  provisioner "shell" {
    inline_shebang = "/bin/sh -ex"
    environment_vars = [
      "DEBIAN_FRONTEND=noninteractive",
    ]
    inline = [
      # Wait apt-get lock
      "while ps -opid= -C apt-get > /dev/null; do sleep 1; done",
      "apt-get clean",
      # apt-get update sometime may fail
      "for i in `seq 1 3`; do apt-get update && break; sleep 10; done",

      # Wait apt-get lock
      "while ps -opid= -C apt-get > /dev/null; do sleep 1; done",

      "apt-get dist-upgrade -y -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold'",
      "for i in `seq 1 3`; do apt-get update && break; sleep 10; done",
      "apt-get upgrade -y -q -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold'",

      # Install docker and docker-compose
      "apt-get install -y -q apt-transport-https ca-certificates nfs-common",
      "curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -",
      "add-apt-repository \"deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable\"",
      "for i in `seq 1 3`; do apt-get update && break; sleep 10; done",
      "apt-get install -y -q docker-ce",
      "curl -L \"https://github.com/docker/compose/releases/download/1.29.2/docker-compose-$(uname -s)-$(uname -m)\" -o /usr/local/bin/docker-compose",
      "chmod +x /usr/local/bin/docker-compose",

      # Install haveged, otherwise docker-compose may hang: https://stackoverflow.com/a/68172225/1494610
      "apt-get install -y -q haveged",

      # Add users for services
      "useradd -m -s /bin/bash bookster",
      "useradd -m -s /bin/bash example",
      "useradd -m -s /bin/bash hermicache",
      "useradd -m -s /bin/bash sneakers",
      "useradd -m -s /bin/bash solaris",
    ]
  }

  ### Ructf motd
  provisioner "shell" {
    inline = [
      "rm -rf /etc/update-motd.d/*",
    ]
  }
  provisioner "file" {
    source = "motd/ructf-banner.txt"
    destination = "/ructf-banner.txt"
  }
  provisioner "file" {
    source = "motd/00-header"
    destination = "/etc/update-motd.d/00-header"
  }
  provisioner "file" {
    source = "motd/10-help-text"
    destination = "/etc/update-motd.d/10-help-text"
  }
  provisioner "shell" {
    inline = [
      "chmod +x /etc/update-motd.d/*",
    ]
  }

  ## Onboot docker-compose run service
  provisioner "file" {
    source = "service-boot/ructf-service@.service"
    destination = "/etc/systemd/system/ructf-service@.service"
  }

  # Copy services
  provisioner "file" {
    source = "../services/bookster/"
    destination = "/home/bookster/"
  }
  provisioner "file" {
    source = "../services/example/"
    destination = "/home/example/"
  }
  provisioner "file" {
    source = "../services/hermicache/"
    destination = "/home/hermicache/"
  }
  provisioner "file" {
    source = "../services/sneakers/"
    destination = "/home/sneakers/"
  }
  provisioner "file" {
    source = "../services/solaris/"
    destination = "/home/solaris/"
  }


  # Build and run services for the first time
  provisioner "shell" {
    inline = [
      "cd ~bookster",
      "docker-compose build || true",

      "cd ~example",
      "docker-compose build || true",

      "cd ~hermicache",
      "docker-compose build || true",

      "cd ~sneakers",
      "docker-compose build || true",

      "cd ~solaris",
      "docker-compose build || true",

      "systemctl daemon-reload",
      "systemctl enable ructf-service@bookster",
      "systemctl enable ructf-service@example",
      "systemctl enable ructf-service@hermicache",
      "systemctl enable ructf-service@sneakers",
      "systemctl enable ructf-service@solaris",
    ]
  }

  # Fix some internal digitalocean+cloud-init scripts to be compatible with our cloud infrastructure
  provisioner "shell" {
    script = "digital_ocean_specific_setup.sh"
  }

  # cleanup after multistage build
  provisioner "shell" {
    inline = [
      "docker image prune --filter label=stage=builder -f || true",
      # remove all image after build completed
      "docker image rm python:3.10-slim node:18.2.0-alpine nginx:1.16.0-alpine julia:1.8.5-alpine3.17 golang:1.20-alpine alpine:3.14 mcr.microsoft.com/dotnet/sdk:7.0 mcr.microsoft.com/dotnet/aspnet:7.0 php:8-fpm-alpine alpine:latest || true",
    ]
  }
}
