import zmq
import struct

context = zmq.Context()
socketMng = context.socket(zmq.REQ)
socketMng.connect("ipc://zmq.sock")

# Ask version
socketMng.send_multipart(
    [
        struct.pack("L", 1230128470)
    ]
)
msg = socketMng.recv_multipart()
print("Server received message: %s" % (msg))

# Ask log level change
socketMng.send_multipart(
    [
        struct.pack("L", 1279741772),
        b"v"
    ]
)
msg = socketMng.recv_multipart()
print("Server received message: %s" % (msg))

# Send unknown command
socketMng.send_multipart(
    [
        struct.pack("L", 1111111),
        b"v"
    ]
)
msg = socketMng.recv_multipart()
print("Server received message: %s" % (msg))
