output "cs-master-public-ip" {
  value = digitalocean_droplet.cs-master.ipv4_address
}

output "cs-checker-public-ip" {
  value = digitalocean_droplet.cs-checker.ipv4_address
}

output "gh-workflow-public-ip" {
  value = digitalocean_droplet.gh-workflow.ipv4_address
}

output "vpn-public-ip" {
  value = digitalocean_droplet.vpn.ipv4_address
}


resource "local_file" "cs_ansible_inventory" {
  filename = "../ansible/cs/inventory.cfg"
  content  = <<-EOF
[master]
cs-master ansible_host=${digitalocean_droplet.cs-master.ipv4_address} private_addr=${digitalocean_droplet.cs-master.ipv4_address_private}

[flags]
cs-master

[db]
cs-master

[monitoring]
cs-master

[checkers]
ch1 ansible_host=${digitalocean_droplet.cs-checker.ipv4_address} private_addr=${digitalocean_droplet.cs-checker.ipv4_address_private} private_ip=${digitalocean_droplet.cs-checker.ipv4_address_private}

[cs:children]
master
flags
db
checkers
monitoring
EOF
}

resource "local_file" "vpn_ansible_inventory" {
  filename = "../ansible/vpn/inventory.cfg"
  content  = <<-EOF
[vpn]
vpn.a ansible_host=${digitalocean_droplet.vpn.ipv4_address}
EOF
}