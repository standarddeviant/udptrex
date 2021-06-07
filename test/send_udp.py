
import sys
import socket
import msgpack

print(sys.argv)
if len(sys.argv) < 2:
    sys.exit(0)

UDP_IP = "127.0.0.1"
UDP_PORT = 42042
MESSAGE = msgpack.packb(dict(type=sys.argv[1]))

print("UDP target IP: %s" % UDP_IP)
print("UDP target port: %s" % UDP_PORT)
print("message: %s" % MESSAGE)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

out = sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

print(f"sock.sendto({MESSAGE}, ({UDP_IP}, {UDP_PORT})) = {out}")
