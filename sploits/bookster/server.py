import socket
import sys
import threading

# Define server settings


def listen_flag(host='0.0.0.0', port=12345):
    def handle_client(client_socket, client_addr):
        print(f"Connected to {client_addr}")
        sys.stdout.flush()
        # Continuously receive data from the client and echo it back
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            print(f"Received from {client_addr}: {data.decode('utf-8')}")
            sys.stdout.flush()
            client_socket.sendall(data)

        # Close the client socket
        client_socket.close()
        print(f"Connection closed: {client_addr}")
        sys.stdout.flush()

    # Create the socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Allow the socket to be reused after the process is terminated
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Bind the socket to the specified host and port
    server_socket.bind((host, port))

    # Start listening for incoming connections
    server_socket.listen(5)

    print(f"Listening on {host}:{port}")

    sys.stdout.flush()
    client_socket, client_addr = server_socket.accept()
    client_thread = threading.Thread(target=handle_client, args=(client_socket, client_addr))
    return client_thread


# Continuously accept incoming connections and spawn a new thread for each one
if __name__ == '__main__':
    while True:
        client_thread = listen_flag()
        client_thread.start()
