import requests
import sys
PORT = 8900
HOSTNAME = 'localhost' #sys.argv[1]
TOKEN = '0c8bf2fe58bed4cc9f1af6c9fb15b59817219a6b' #sys.argv[2]
FLAG_ID = '66374b52-f67b-4b57-a007-5a2310069131' #sys.argv[3]

url = f"http://{HOSTNAME}:{PORT}/api/books/"

payload={'title ': 'var and peace',
'text': '22'}

files=[
  ('video', ('outputFilename.webm', open('test.avi', 'rb'), 'video/webm'))
]
headers = {
  'Authorization': f'Token {TOKEN}'
}

response = requests.request("POST", url, headers=headers, data=payload, files=files)

print(response.text)