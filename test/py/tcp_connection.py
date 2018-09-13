#!/usr/bin/python3
# test target: test_tcp_connection

import socket
import threading
import time

HOST = "0.0.0.0"
PORT = 9999

clinet_nums = 10000
client_list = []
cond = threading.Condition()

def ClientConnect():
    time.sleep(3);
    for i in range(50):
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
        client.connect((HOST, PORT))

        cond.acquire()
        client_list.append(client)
        print("connect nums:peer:", client.getpeername(), "local:", client.getsockname())
        cond.notify()

        cond.release()

def ClientDisconnect():
    for i in range(50):
        cond.acquire()
        while(len(client_list) == 0):
            cond.wait();

        ##import pdb;pdb.set_trace();
        client = client_list.pop()
        print("disconnect num peer:", client.getpeername(), "local:", client.getsockname())
        client.close()

        cond.release()

if "__main__" == __name__:

    t_connect = []
    t_disconnect = []
    for i in range(clinet_nums):
        t_connect.append(threading.Thread(target=ClientConnect));
        t_disconnect.append(threading.Thread(target=ClientDisconnect));

    for i in range(clinet_nums):
        t_connect[i].start();
        t_disconnect[i].start();

    for i in range(clinet_nums):
        t_connect[i].join();
        t_disconnect[i].join();

    print("finished")


