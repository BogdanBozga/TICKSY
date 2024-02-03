import random
import socket
import os
import struct
from time import sleep

import cv2
import numpy as np
import threading
import pathlib

MAXLINE = 512
MAX_THREADS = 100
PORT = 5006

directoryPath = None
conn = None

def generate_random_image_name():
    # Generate a random image name
    return "image_{}{}".format(random.randint(0, 9999), '.jpg')
def get_current_dir():
    global directoryPath
    directoryPath = os.getcwd() 
    print(f"Current working directory: {directoryPath}")

def check_directory():
    directoryPath = os.getcwd()
    print(f"\nReading from: {directoryPath}")
    for item in os.listdir(directoryPath):
        print(item)

def receive_text(conn):
    buffer = conn.recv(MAXLINE).decode()
    print(f"Server: {buffer}")
    return buffer


def send_text(conn, text):
    conn.send(str(text).encode())
    print("Message sent to server.")

def file_exists(image_path):
    if os.path.exists(image_path):
        print(f"File '{image_path}' exists.")
        if image_path.endswith(".png") or image_path.endswith(".jpg"):
            return True
    print(f"File '{image_path}' does not exist or has wrong extension.")
    return False


def send_image(conn):
    # Read the image in binary mode

    image_path = input("Enter the path to image: ")
    img = cv2.imread(image_path)

    # Encode the image as JPG
    _, img_encoded = cv2.imencode('.jpg', img)
    img_bytes = img_encoded.tobytes()

    # Send the size of the image
    conn.send(len(img_bytes).to_bytes(4, 'big'))

    # Send the image data
    conn.sendall(img_bytes)


def receive_image(conn):
    # Receive the size of the image
    img_size_data = conn.recv(4)
    img_size = int.from_bytes(img_size_data, 'big')
    print(f"Image size: {img_size}")
    # Receive the image data
    img_data = b''

    remaining = img_size
    while remaining > 0:
        packet_size = min(remaining, 4096)  # Determine how much to read
        packet = conn.recv(packet_size)  # Read the determined size
        remaining -= len(packet)  # Subtract the actual amount read
        if not packet:
            break
        img_data += packet
    print('Finish receiving image')

    # Convert bytes data to a numpy array
    nparr = np.frombuffer(img_data, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

    # Save or process the image
    save_path = 'received_image_{}'.format(generate_random_image_name())
    cv2.imwrite('return_images/'+save_path, img)



def main():
    # global sockfd_global
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    server_address = ('localhost', PORT)
    conn.connect(server_address)
    print("Connected to the server..")

    while True:
        text = receive_text(conn) # choice list
        if "Please choose an option" in text:
            choice = int(input("Your choice: "))
            print(f"Choice selected : {choice}")
            send_text(conn,str(choice)) # send choice
        else:
            if "Give me you image" in text: #1
                send_image(conn)
            elif "Returning image" in text: #2
                receive_image(conn)
            elif "gray" in text: #3
                pass
                # receive_text(conn)
            elif "Closing program" in text: #7
                print("Finishing program ...")
                break
            elif "resize" in text: #4
                resize = float(input("Enter resize value: "))
                send_text(conn,resize)
                # receive_text(conn)
            elif "rotate" in text: #5
                angle = float(input("Enter angle value: "))
                send_text(conn, str(angle))
                # receive_text(conn)
            elif "sobel" in text: #6
                # receive_text(conn)
                pass
            elif "Changes apply" in text:
                print("good")
            else:
                print("Nothing done.")
    
    conn.close()

if __name__ == "__main__":
    main()
