#!/usr/bin/env python
import zmq

context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://127.0.0.1:8081")

msg = socket.recv_multipart()
print("Server received message: %s" % (msg))
socket.send_multipart(msg)
