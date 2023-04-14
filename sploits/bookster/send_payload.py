import requests
import sys
PORT = 8900
HOSTNAME = 'localhost' #sys.argv[1]
TOKEN = '6ff8e094f372e100146e885ce7651fce3736e90d' #sys.argv[2]

url = f"http://{HOSTNAME}:{PORT}/api/books/"

payload={'title ': 'var and peace'}

files=[
  ('video', ('outputFilename.webm', open('test.avi', 'rb'), 'video/webm')),
  ('text', ('outputFilename.txt', 'HA HA YOU HACKED', 'text/plain'))
]
headers = {
  'Authorization': f'Token {TOKEN}'
}

response = requests.request("POST", url, headers=headers, data=payload, files=files)

print(response.text)