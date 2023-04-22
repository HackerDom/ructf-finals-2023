import requests


def send_payload(host, port, token):
    url = f"http://{host}:{port}/api/books/"

    payload={'title ': 'var and peace'}

    files=[
      ('video', ('outputFilename.webm', open('test.avi', 'rb'), 'video/webm')),
      ('text', ('outputFilename.txt', 'HA HA YOU HACKED', 'text/plain'))
    ]
    headers = {
      'Authorization': f'Token {token}'
    }
    response = requests.request("POST", url, headers=headers, data=payload, files=files)
    print(response.text)


if __name__ == '__main__':
    PORT = 8900
    HOSTNAME = 'localhost'  # sys.argv[1]
    TOKEN = '6ff8e094f372e100146e885ce7651fce3736e90d'  # sys.argv[2]
    send_payload(HOSTNAME, PORT, TOKEN)