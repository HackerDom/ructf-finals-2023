import os
import sys

N = 60

SERVER_DATA = """mode server
tls-server
topology subnet
port {2}

ifconfig 10.{0}.{1}.254 255.255.255.128
ifconfig-pool 10.{0}.{1}.129 10.{0}.{1}.253 255.255.255.128
push "route-gateway 10.{0}.{1}.254"

cipher AES-128-CBC
dev team{3}
dev-type tun
dev team{3}-net
keepalive 10 30
ping-timer-rem
persist-tun
persist-key

duplicate-cn
client-to-client

verb 3

txqueuelen 1000
tun-mtu 1500
fragment 1300
mssfix

<dh>
{4}
</dh>

<ca>
{5}
</ca>

<cert>
{6}
</cert>

<key>
{7}
</key>
"""

if __name__ != "__main__":
    print("I am not a module")
    sys.exit(0)

# gen client configs
os.chdir(os.path.dirname(os.path.realpath(__file__)))
try:
    os.mkdir("server_outside_prod")
except FileExistsError:
    print("Remove ./server_outside_prod dir first")
    sys.exit(1)

for i in range(1, N):
    dh = open("net_certs_prod/team%d-net/dh.pem" % i).read().strip()
    ca = open("net_certs_prod/team%d-net/ca.crt" % i).read().strip()
    cert = open("net_certs_prod/team%d-net/issued/team%d-server.crt" % (i, i)).read().strip()
    cert = cert[cert.index("-----BEGIN CERTIFICATE-----"):]
    key = open("net_certs_prod/team%d-net/private/team%d-server.key" % (i, i)).read().strip()

    data = SERVER_DATA.format(60 + i // 256, i % 256, 30000+i, i, dh, ca, cert, key)
    open("server_outside_prod/%d.conf" % i, "w").write(data)

print("Finished, check ./server_outside_prod dir")
