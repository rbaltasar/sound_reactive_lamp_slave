import socket
import sys
from time import sleep


if __name__== "__main__":

    UDP_IP = "192.168.2.120"
    UDP_IP_OTHERS = "192.168.2.120"
    UDP_PORT = 7001
    MESSAGE = sys.argv[1]
    num_loops = int(sys.argv[2])

    print "UDP target IP:", UDP_IP
    print "UDP target port:", UDP_PORT
    print "message:", MESSAGE

    sock = socket.socket(socket.AF_INET, # Internet
                 socket.SOCK_DGRAM) # UDP

    for i in range(0,num_loops):
        sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
        #sleep(0.01)
        #for j in range(0,4):
        #    sleep(0.002)
        #    sock.sendto(MESSAGE, (UDP_IP_OTHERS, UDP_PORT))
        sleep(0.01)
