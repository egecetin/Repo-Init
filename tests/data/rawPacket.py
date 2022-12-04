import pcapy

max_bytes = 65536
promiscuous = False
read_timeout = 100

reader = pcapy.open_live("test1", max_bytes, promiscuous, read_timeout)

for i in range(1,1000):
    (header, payload) = cap.next()
    if len(payload):
        