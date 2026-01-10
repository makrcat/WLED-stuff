import socket

UDP_IP = "192.168.1.100"
UDP_PORT = 5568

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

frame = bytearray([255, 0, 0] * 50)  
sock.sendto(frame, (UDP_IP, UDP_PORT))
