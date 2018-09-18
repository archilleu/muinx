#!/usr/bin/python3
#for test_tcp_server
import socket
import inspect
import struct
import select
import sys
import random
import pdb
import subprocess
import signal
import os

HOST = "0.0.0.0"
PORT = 9999

#客户端数量
CLIENT_NUM = 64 

#操作次数
#TIMES = 1
TIMES = 1000 * 10

#请求响应还是通知
REPLY = 1
NOTIFY = 2

#发送数据最大带下
MAX_SEND_LEN = 1024*64

#打印源码所在行
def no():
    try:
        raise Exception
    except:
        f = sys.exc_info()[2].tb_frame.f_back
        return f.f_lineno

class EpollConnector:
    '''generic epoll connectors which connect to down stream server'''

    def __init__(self, client_num, srv, times):
        self.client_num = client_num
        self.srv_addr = srv
        self.times= times
        self.epoll = select.epoll()
        self.connections = {}
        self.dats = {}

    def Connect(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect(self.srv_addr)
            fileno = s.fileno()
            self.epoll.register(fileno, select.EPOLLOUT | select.EPOLLIN |
                                select.EPOLLET)
        except socket.error:
            print('no:', no(), 'ERROR in connect to ',  self.srv_addr)
            sys.exit(1)
        else:
            self.connections[fileno] = s

            #构造随机数据
            byte_data = bytearray(random.randint(1, MAX_SEND_LEN))
            byte_len = len(byte_data)
            for i in range(byte_len):
                v = random.randint(0, 255)
                byte_data[i] = v;
            self.dats[fileno] = byte_data
            print('no:', no(), "connect client_nums:peer:", s.getpeername(), "local:",s.getsockname())

    def Reconnect(self, fileno):
        self.epoll.unregister(fileno)
        self.connections[fileno].close()
        del self.connections[fileno]
        del self.dats[fileno]
        self.Connect()

    def random_reconnect(self):
        keys = self.connections.keys()
        if 0 == len(keys):
            return

        idx = random.randint(0, len(keys)-1)
        self.Reconnect(list(keys)[idx])

    def handle_read(self, fileno):
        try:
            dat = self.connections[fileno].recv(8, socket.MSG_WAITALL)
            if 8 > len(dat):
                print('no:', no(), "handle_read:", dat, " need reconnect")
                self.Reconnect(fileno)
                return

            codec = struct.Struct(">ii")
            header =codec.unpack(dat)
            print('no:', no(), "type:", header[1], "len:", header[0])

            dat = self.connections[fileno].recv(header[0], socket.MSG_WAITALL)
            if len(dat) == header[0]:
                if REPLY == header[1]:
                    if dat != self.dats[fileno]:
                        assert False, "recv dat error"
                    else:
                        print('no:', no(), "recv dat success!!!!!")

                elif NOTIFY == header[1]:

                    #pdb.set_trace()
                    addr = self.connections[fileno].getsockname()
                    str_addr = addr[0] + ":" + str(addr[1])
                    if str_addr != dat.decode("utf-8"):
                        assert False,"notify error"
                    else:
                        print('no:', no(), "notify dat success!!!!!")

                else:
                    assert False, "recv error type"
            else:
                print('no:', no(), "handle_read:", dat, " need reconnect")
                self.Reconnect(fileno)
                return
        except:
            print('no:', no(), "handle_read:", dat, " need reconnect")
            self.Reconnect(fileno)

        finally:
            self.epoll.modify(fileno, select.EPOLLOUT | select.EPOLLIN |
                              select.EPOLLET)

        pass

    def handle_write(self, fileno):
        codec = struct.Struct(">ii")
        header = codec.pack(len(self.dats[fileno]), REPLY)
        try:
            if 8 != self.connections[fileno].send(header):
                assert False, "send false"
            if len(self.dats[fileno]) != self.connections[fileno].send(self.dats[fileno]):
                assert False, "send false"

            print('no:', no(), "handle_write success!!!!    len:", len(self.dats[fileno]))
        except:
            print('no:', no(), "handle_write close, need Reconnect")
            self.Reconnect(fileno)

        return;

    def handle_close(self, fileno):
        print('no:', no(), "handle_close need reconnect")
        self.Reconnect(fileno)
        pass

    def handle_error(self, fileno):
        print('no:', no(), "handle_error:need reconnect")
        self.Reconnect(fileno)
        pass

    def do_epoll(self, fileno, event):
        try:
            if event & select.EPOLLHUP:
                self.handle_close(fileno)
            elif event & select.EPOLLERR:
                self.handle_error(fileno)
            elif event & select.EPOLLIN:
                self.handle_read(fileno)
            elif event & select.EPOLLOUT:
                self.handle_write(fileno)
        except:
            raise

    def loop_epoll(self):
        for i in range(self.client_num):
            self.Connect()
        try:
            for i in range(self.times):
                if 0 == random.randint(0, 64):
                    self.random_reconnect()

                events = self.epoll.poll()
                for fileno, event in events:
                    self.do_epoll(fileno, event)
        finally:
            pass


if __name__ == "__main__":
    svr = EpollConnector(CLIENT_NUM, (HOST, PORT), TIMES)
    svr.loop_epoll()

    p = subprocess.Popen(['ps','-A'], stdout=subprocess.PIPE)
    out,err = p.communicate()

    for line in out.splitlines():
        if 'test_server' in line.decode('utf-8'):
            pid = int(line.split(None, 1)[0])
         #   os.kill(pid, signal.SIGQUIT)
