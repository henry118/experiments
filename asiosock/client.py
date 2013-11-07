#!/usr/bin/env python

import struct
import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 5685))

line = raw_input(">>> ")
while len(line):
    if line == "exit":
        break
    sock.send(line)
    line = raw_input(">>> ")

sock.close()
