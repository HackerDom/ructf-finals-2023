The Cloud-server provides participants the console where they can do various actions with their vulnerable images: create, reboot, checkpoint and so on. The images are run in Digital Ocean. In addition to vulnerable image, for every team the special vm called team router is set up. This router contains the OpenVPN server allowing participants to enter the game network. Also the trafic between vulnerable image and game network flows through this router. Participants don't have SSH access to their router, but can connect or disconnect it from the game network using the cloud console.

Participants authenticate in the console by tokens, so they should be sent to them somehow, for example, using email or telegram bots.

### Pre-requirements ###

To deploy Cloud server you should set up the VPN server first (see ../vpn). The client VPN configs should already be generated for connecting team routers to main VPN server.

The Cloud server can be deployed to some Ubuntu 20.04 box. The server can be a dedicated server or virtual private server on some hosting like Digital Ocean.

The server should have at least 8GB RAM and 4 CPU cores. For big competitions server with 32GB RAM and 8 CPU cores
is recommended.

The server must have SSH up configured to accept the root user by key. The server must have network connectivity to Digital Ocean API.

The account on Digital Ocean should be created and paid. The droplet limit should be not less than 2\*N + 10, where N is a maximum number of teams. For every team the vulnerable vm and the router vm are created.

All VMs and other objects should be created in AMS3 zone. If you need prefer other zone, it can be set in cloud\_master/files/api\_srv/do\_api.py and vuln_image/image.pkr.hcl, just search for the ams3 string and replace it to prefered zone.

### Prepare ###

To set up the VPN server you should have it created on some hosting.

The Digital Ocean account must be set up to delegate some zone, like cloud.ructf.org. The A record should point on the Cloud box, other records will be created in this zone during the proccess of team image creation. The domain names are used in VPN configs that console gives to the participants to connect to their team router. The team router, in its turn, connects to the central VPN server using the configs that are were created previously.

To communicate with Digital Ocean API, you need the key, which can be created on Digital Ocean control panel (https://cloud.digitalocean.com/account/api/tokens). This key should be stored secretly and never be commited in git.

After obtaining the token, the command ```echo 'TOKEN = "dop_v1_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"' > cloud_master/files/api_srv/do_token.py``` should be executed with your token.

To communicate with team routers, the SSH key pair is needed. it can be created with the following command: ```ssh-keygen -N "" -f cloud_master/files/api_srv/do_deploy_key```. The public key and the private key will be created, the public key should be added to Digital Ocean using this page: https://cloud.digitalocean.com/account/security.

To test the token and get ssh key ids, execute ```python3 cloud_master/files/api_srv/list_ssh_keys.py```. The key id will be needed in the next steps.


#### Generate Configs ####

To generate configs, execute: ```./init_cloud.sh <vpn_ip> <vpn_domain>```

For example: ```./init_cloud.sh 188.166.118.28 cloud.ructf.org```

The script patches an IP address in 'inventory.cfg' file and domain name in 'cloud_master/files/api_srv/cloud_common.py', in gen/gen_conf_client_entergame_prod.py, in cloud_master/files/wsgi/cloudapi.wsgi, in cloud_master/files/apache2/000-default.conf and cloud_master/files/nginx/cloud files.


After that, it generates a directory with init cloud state and copies it in cloud_master/files/api_srv/db_init_state_prod. Every subdirectory in this directory contains cloud state for single team. The states of cloud, VPN configs, root password and its hash, hash of the team token.

The proccess of config generation can take about 30 minutes.

The team tokens are in "gen/tokens_prod" directory. They should be sent to participants before the game.


#### Obtain the SSL Certificates ####

The cloud is accessed by the browser with https protocol, so it need valid certificates. The easiest way to obtain them is to log in on the cloud host with SSH, make sure that its domain name resolves to its IP address, and execute these commands, replacing cloud.ructf.org with your domain:

```
apt update && apt install certbot
certbot -d cloud.ructf.org certonly
```

Now copy the contents of file ```/etc/letsencrypt/archive/<your_domain>/fullchain1.pem``` from remote host to ```cloud_master/files/nginx/cert_cloud.pem``` on your local machine and file ```/etc/letsencrypt/archive/<your_domain>/privkey1.pem``` to ```cloud_master/files/nginx/key_cloud.pem```.

Also the file dhparams_cloud.pem should be generated. This can be done with a ```openssl dhparam -out cloud_master/files/nginx/dhparams_cloud.pem 4096``` command. This takes about 10 minutes.

The last thing to do for the webserver is to set up the password for cloud console to protect it from team access before the competition starts:
```htpasswd -c cloud_master/files/apache2/htpasswd cloud```


#### Deploy Router Host ####

Both team VMs: vulnerable VM and router VM are deployed from snapshots, so the snapshots have to be created first. The snapshot for router is made only once.

The router host should be created as a s-1vcpu-1gb (1 CPU and 1 GB RAM) droplet. Enter these commands to configure it:

```
apt update && apt upgrade -y
apt install -y openvpn iptables-persistent net-tools
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
iptables-save > /etc/iptables/rules.v4

echo 'net.ipv4.ip_forward = 1' >> /etc/sysctl.conf
echo 'net.nf_conntrack_max=524288' >> /etc/sysctl.conf
echo 'net.netfilter.nf_conntrack_max=524288' >> /etc/sysctl.conf
sysctl -p

sed -i 's/#Port 22/Port 2222/' /etc/ssh/sshd_config
systemctl restart sshd

shutdown -P now
```

To connect with SSH to this VM again in future, use port 2222 instead of 22.

After poweroff, use the Digital Ocean console to make the snapshot of this VM.

The ID of snapshot can be obtained with ```python3 cloud_master/files/api_srv/list_all_snapshots.py```. This ID will be needed on next steps.


#### Deploy Vurnerable Host ####

The vurnerable host contains services for CTF. It is recommended to build it using packer software. The installation instructions is available on https://www.packer.io/downloads. The approximate command sequence is:

```
cd vuln_image
packer init image.pkr.hcl
packer build -var "api_token=dop_v1_XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" image.pkr.hcl
```

This command builds a typical image with docker-compose and single service. It usually takes about 15 minutes. Also the packer prepares the image for cloud: enables password authentication in SSH and sets apropriate network mask. The root password and the network address will be set automatically on the first run, if the distro is Ubuntu. For other distros some tweaks will be needed, see USERDATA\_TEMPLATE constant in cloud_master/files/api_srv/create_team_instance.py.

The ID of snapshot can be obtained with ```python3 cloud_master/files/api_srv/list_all_snapshots.py```. This ID will be needed on next step.


#### Specifying Image IDS ####

In order to create VMs, the cloud needs to know the ids of router and vm images and SSH key id. They can be specified in cloud_master/files/api_srv/create_team_instance.py file. Example:

```
ROUTER_DO_IMAGE = 112309225
VULNIMAGE_DO_IMAGE = 108811203
DO_SSH_KEYS = [34519247]
```

Use the ids you've obtained on previous steps. Now the cloud is ready for deploy.

#### Deploy Cloud Master Role ####

To deploy Cloud master role, run ```ansible-playbook cloud_master.yaml```

This command will set up the remote server.

The directory "db_init_state_prod" in /cloud/backend should be renamed to "db" after the deploy on the remote host. This was made to protect database from accidental redeploy.

#### Preparing the Cloud for Game

Before the game, the init state directory should be renamed or copied to db directory on the cloud master host. This was made to prevent db corruption on ansible runs: ```rsync -a /cloud/backend/db_init_state_prod/ /cloud/backend/db```

The web interface is protected by additional password, which guarantees that only orgs can enter the commands in the console. Just few minutes before the game starts, this password should be removed. This can be done by commenting "Require valid-user" and uncommenting "#Require all granted" lines in /etc/apache2/sites-enabled/000-default.conf file and executing ```systemctl restart apache2```

Before removing the password the cloud should be in the initial state, if some test teams were created before the competition, they should be removed.

The cloud console should be reachable with browser by domain name

#### Administering the Cloud on the Game ####


Most scripts are in /cloud/backend directory on the cloud master server.

Some useful commands, for team 10 (to use them on other teams replace 10 with something else

```
./create_team_instance.py 10               # creates the vm and router
./switch_team_net_to_not_cloud.py 10       # disconnects the team from the central router
./switch_team_net_to_cloud.py 10           # connects the team to the central router
./delete_team_instance_vm.py 10            # deletes vulnerable vm (not recovable)
./delete_team_instance_net.py 10           # deletes team router (not recovable)
./reboot_vm.py 10                          # reboots vulnerable vm
./list_team_instances.py                   # get states of all VMs
./check_cloud_state.py                     # compare the state in local db and DO state
./list_snapshots.py 10                     # list snapshots for team
./take_snapshot.py 10 somename             # take the snapshot
./restore_vm_from_snapshot.py 10 somename  # restore from snapshot
```

#### If Something Goes Wrong ####

The cloud console webserver error logs are available on /var/log/apache2/error.log. They contain commands entered by teams and their status.

The database for team is available in /cloud/backend/db/teamX directory, where X is the team number. The tasks.log file contains information about all tasks initiated by the team, the detailed log about every operation is available in files like task_create_vm.out, where create_vm is the action name.

#### After The Game ####

Do not forget to remove all vms created for teams and their snapshots to prevent overbilling.
