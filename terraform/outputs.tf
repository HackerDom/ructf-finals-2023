output "cs-master-public-ip" {
  value = digitalocean_droplet.cs-master.ipv4_address
}
output "cs-master-private-ip" {
  value = digitalocean_droplet.cs-master.ipv4_address_private
}

output "cs-checker-public-ip" {
  value = digitalocean_droplet.cs-checker.ipv4_address
}
output "cs-checker-private-ip" {
  value = digitalocean_droplet.cs-checker.ipv4_address_private
}

output "gh-workflow-public-ip" {
  value = digitalocean_droplet.gh-workflow.ipv4_address
}

output "vpn-public-ip" {
  value = digitalocean_droplet.vpn.ipv4_address
}