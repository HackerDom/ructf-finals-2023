The VPN server is a core of CTF network. It routes trafic between team routers of every team. In the cloud setup, these team routers set up automaticaly.

### Pre-requirements ###

To deploy VPN you should have some Ubuntu 20.04 box with fast network (1Gbit/second is recommended).
The server can be a dedicated server (better) or virtual private server on some hosting like Digital Ocean.

The server should have at least 8GB RAM and 4 CPU cores. For big competitions server with 64GB RAM and 16 CPU cores
is recommended. If you want to record the traffic, the server should have enough free space to store it in compressed form.

The server must have SSH up configured to accept the root user by key.

The server must have connectivity to other infrastructure servers (checksystem, db, ...). For best performance, 
all infrastructure servers should be in the same local network. The infrastructure network is 10.10.10.0/24.

### Prepare ###

To set up the VPN server you should have it created on some hosting. The server must be able to reach other infrastructure services in 10.10.10.0/24 network. The simplest way to do it is to set up the hosting to use this network as a private network.

To be able to migrate to another VPN server in case of problems, the server must have a domain name, for example vpn.ructf.org, which resolves to this server. To not depend on DNS caching time, the clients use random subdomain in this domain, so all subdomains should resolve to this IP, e.g aaa.bbb.vpn.ructf.org should point the VPN server IP.

#### Generate Configs ####

Clone the repository on you Linux host, it can be laptop or remote server.

To generate configs, execute: ```./init_vpn.sh <vpn_ip> <vpn_domain>```

For example: ```./init_vpn.sh 146.190.16.194 'vpn.ructf.org'```

The script patches ip in 'inventory.cfg' file and domain name in 'gen/gen_conf_client_prod.py' file.

After that, it generates VPN configs for both server and client and copies server configs to vpn/files/openvpn_prod for further deploying them in ansible.

#### Deploy VPN Role ####

To deploy VPN role, run ```ansible-playbook vpn.yaml```

This command will set up the remote server.

#### Prepare Host for the Game ####

Some actions are needed after deploying to initialize the host state.

Log in with ssh to the host as root user and execute this command to close the communication between teams: ```./openclosenetwork/close_network.sh```

```./snat/add_snat_rules.sh```. This commands add rules for source NAT and tunes parameters in Linux kernel for high-performace NAT on multicore servers. The source NAT hides the source IP from teams to make harder for them to filter checksystem by IP.

```./trafdump/add_trafdump_rules.sh``` This command turns on the traffic recording. The traffic is recorded to /home/dump/big in compressed form.

### Testing the VPN ###

To test VPN, execute this command on the host where configs were generated: ```openvpn gen/client_prod/42.conf```

After that you should ping 10.80.42.1 address. Connection to any TCP-port on this address should give the "The network is closed" message. This can be check with browser or with ```nc 10.80.42.1 42``` command.

### Administrating VPN Host on the Game ###

Here are some useful commands to be run on the VPN host.

To open the network, execute ```./openclosenetwork/open_network.sh```

To check the network state (open/closed) use: ```./openclosenetwork/check_network.sh```

To temporary disable SNAT: ```./snat/del_snat_rules.sh```

To check SNAT (enabled/disabled) ```./snat/check_snat_rules.sh```

To temporary ban the team 42 for flooding the network: ```./antidos/block_team.sh 42``` The team will get message "Please, stop the flood attack on the game network." on every tcp connection.

To unban it: ```./antidos/unblock_team.sh 42```

To the the list of blocked teams in iptables-style format: ```./antidos/list_blocked_teams.sh```

It is recommended to read the source of all these scripts to understand how they work.
