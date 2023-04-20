import os
import sys

N = 60

SERVER = "team%d.cloud.ructf.org"

CLIENT_DATA = """client
tls-client
remote-cert-tls server
topology subnet
cipher AES-128-CBC
remote {0} {1}
dev tun
route 10.60.0.0 255.252.0.0
route 10.80.0.0 255.252.0.0
route 10.10.10.0 255.255.255.0
keepalive 10 30
nobind
verb 3

tun-mtu 1500
fragment 1300
mssfix

<ca>
{2}
</ca>

<cert>
{3}
</cert>

<key>
{4}
</key>
"""

if __name__ != "__main__":
    print("I am not a module")
    sys.exit(0)

# gen client configs
os.chdir(os.path.dirname(os.path.realpath(__file__)))
try:
    os.mkdir("client_entergame_prod")
except FileExistsError:
    print("Remove ./client_entergame_prod dir first")
    sys.exit(1)

for i in range(1, N):
    ca = open("net_certs_prod/team%d-net/ca.crt" % i).read().strip()
    cert = open("net_certs_prod/team%d-net/issued/team%d-client.crt" % (i, i)).read().strip()
    cert = cert[cert.index("-----BEGIN CERTIFICATE-----"):]
    key = open("net_certs_prod/team%d-net/private/team%d-client.key" % (i, i)).read().strip()

    data = CLIENT_DATA.format(SERVER % i, 30000+i, ca, cert, key)
    open("client_entergame_prod/%d.conf" % i, "w").write(data)

print("Finished, check ./client_entergame_prod dir")
