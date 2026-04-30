import socket

HOST = "127.0.0.1"
PORT = 50551

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((HOST, PORT))

while True:
    msg = input("Enter message: ")

    if msg.lower() == "exit":
        break

    payload = f"LEN:{len(msg)}\n{msg}"
    client.send(payload.encode())

    response = client.recv(4096).decode()
    print("Server:", response)

client.close()
