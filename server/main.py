import socket
import time

if __name__ == '__main__':
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('0.0.0.0', 8909))
    s.listen(0)

    while True:
        sock, addr = s.accept()
        print(f'New connected: {addr}')
        tot = 0
        t0  = time.time()
        cnt = 0
        while True:
            d = sock.recv(4096)
            tot += len(d)
            cnt += len(d)

            if (time.time() - t0) > 1:
                print(cnt / 1000)
                cnt = 0
                t0 = time.time()