#!/bin/bash

# Replace /24 netmask with /25 one
sed -i '/nic_type = nic.get/a \        if nic_type == "private": nic["ipv4"]["netmask"] = "255.255.255.128"' /usr/lib/python3/dist-packages/cloudinit/sources/helpers/digitalocean.py
# Newer DigitalOcean uses OpenStack's mechanics
sed -i '/subnets.append(subnet)/a \            if subnet.get("address", "").startswith("10.6"): subnet["netmask"] = "255.255.255.128"' /usr/lib/python3/dist-packages/cloudinit/sources/helpers/openstack.py

# Set PasswordAuthentication yes
sed -i '/^[^#]*PasswordAuthentication[[:space:]]no/c\PasswordAuthentication yes' /etc/ssh/sshd_config

# Prohibit to set PasswordAuthentication no
sed -i 's/updated = update_ssh_config/\    updated = False/' /usr/lib/python3/dist-packages/cloudinit/config/cc_set_passwords.py
 
