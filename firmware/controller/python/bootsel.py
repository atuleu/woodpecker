#!/usr/bin/env python3
import socket

DEST_IP = "10.42.84.2"
DEST_PORT = 8000
OSC_ADDRESS = "/Woodpecker/Bootsel"


def osc_string(value: str) -> bytes:
    data = value.encode("utf-8") + b"\x00"
    while len(data) % 4 != 0:
        data += b"\x00"
    return data


msg = osc_string(OSC_ADDRESS) + osc_string(",T")

with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
    sock.bind(("",DEST_PORT))
    sock.sendto(msg, (DEST_IP, DEST_PORT))

print(f"Sent {OSC_ADDRESS} to {DEST_IP}:{DEST_PORT}")
