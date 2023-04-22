import socket
import sys
import threading


def listen_flag(host='0.0.0.0', port=12345):

    def handle_client(client_socket, client_addr):
        print(f"Connected to {client_addr}")
        sys.stdout.flush()
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            print(f"Received from {client_addr}: {data.decode('utf-8')}")
            sys.stdout.flush()
            client_socket.sendall(data)

        client_socket.close()
        print(f"Connection closed: {client_addr}")
        sys.stdout.flush()

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    server_socket.bind((host, port))

    server_socket.listen(5)

    print(f"Listening on {host}:{port}")

    sys.stdout.flush()
    client_socket, client_addr = server_socket.accept()
    client_thread = threading.Thread(target=handle_client, args=(client_socket, client_addr))
    return client_thread


if __name__ == '__main__':
    while True:
        client_thread = listen_flag()
        client_thread.start()
