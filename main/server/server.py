from time import sleep

import cv2
import socket
import threading
import os
import struct
import random
import numpy as np

SOCK_PATH = "/tmp/my_socket.sock"
PORT = 5006
MAXLINE = 512

thread_local_data = threading.local()


def generate_random_image_name(extension):
    # Generate a random image name
    return "image_{}{}".format(random.randint(0, 9999), extension)

def send_image(conn):
    img = thread_local_data.image_global
    _, img_encoded = cv2.imencode('.jpg', img)
    img_bytes = img_encoded.tobytes()
    # Send the size of the image
    print('Sending image of size: {}'.format(len(img_bytes)))
    conn.send(len(img_bytes).to_bytes(4, 'big'))
    conn.sendall(img_bytes)

def receive_image(conn):
        # Receive the size of the image
        img_size_data = conn.recv(4)
        img_size = int.from_bytes(img_size_data, 'big')

        # Receive the image data
        img_data = b''
        print('img size',img_size)
        remaining = img_size
        while remaining > 0:
            packet_size = min(remaining, 4096)  # Determine how much to read
            packet = conn.recv(packet_size)  # Read the determined size
            remaining -= len(packet)  # Subtract the actual amount read
            if not packet:
                break
            img_data += packet

        # Convert bytes data to a numpy array
        nparr = np.frombuffer(img_data, np.uint8)
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

        thread_local_data.image_global = img

def rotate_image(conn, angle):
    image = thread_local_data.image_global
    angle =  float(angle)
    height, width = image.shape[:2]
    center = (width / 2, height / 2)
    rotation_matrix = cv2.getRotationMatrix2D(center, angle, 1.0)
    rotated_image = cv2.warpAffine(image, rotation_matrix, (width, height))
    thread_local_data.image_global = rotated_image


def resize_image(conn, scale):
    print("Receiving scale to be resized:...")
    resize = float(scale)
    # print("resize :", resize)
    image = thread_local_data.image_global
    new_width = int(image.shape[1] * resize)
    new_height = int(image.shape[0] * resize)
    resized_image = cv2.resize(image, (new_width, new_height))
    thread_local_data.image_global = resized_image


def grey_scale(conn):
    thread_local_data.image_global = cv2.cvtColor(thread_local_data.image_global, cv2.COLOR_BGR2GRAY)

def send_text(conn, message):
    message_bytes = message.encode('utf-8')

    try:
        conn.sendall(message_bytes)
        print("Message sent to client.")
    except Exception as e:
        print(f"Error sending message: {e}")

def receive_text(conn):
    buffer = conn.recv(MAXLINE).decode()
    print(f"Server: {buffer}")
    return buffer
def apply_sobel(conn):
    image = thread_local_data.image_global
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    grad_x = cv2.Sobel(gray, cv2.CV_64F, 1, 0, ksize=3, scale=1, delta=0)
    grad_y = cv2.Sobel(gray, cv2.CV_64F, 0, 1, ksize=3, scale=1, delta=0)

    abs_grad_x = cv2.convertScaleAbs(grad_x)
    abs_grad_y = cv2.convertScaleAbs(grad_y)

    sobel_combined = cv2.addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0)
    thread_local_data.image_global = sobel_combined

def client_handler(conn, addr):
    print("Connected by", addr)
    thread_local_data.image_global = None
    try:
        while True:
            send_choice_text(conn)
            # Example: Receiving a command to receive or send an image
            choise = receive_text(conn)
            print("The client chose:", choise)
            if choise == '1':
                # "\t1. Edit a new image\n"
                send_text(conn, 'Give me you image')
                receive_image(conn)
            elif choise == '7':
                # "\t7. Exit\n"
                send_text(conn, "Closing program...")
                break
            else:
                # if type(thread_local_data.image_global) == type(None):
                #     send_text(conn, "No image was found load one first!")
                if choise == '2':
                    # "\t2. Save the edited image\n"
                    send_text(conn, "Returning image")
                    # sleep(0.5)
                    send_image(conn)
                elif choise == '3':
                    # "\t3. Apply gray scale to the image\n"
                    send_text(conn, "Applying grayscale scaling")
                    grey_scale(conn)
                    pass
                elif choise == '4':
                    # "\t4. Resize the image\n"
                    send_text(conn, "resize")
                    scale = receive_text(conn)
                    resize_image(conn, scale)
                elif choise == '5':
                    # "\t5. Rotate the image\n"
                    send_text(conn,"rotate")
                    angle = receive_text(conn)
                    rotate_image(conn, angle)
                    pass
                elif choise == '6':
                    # "\t6. Apply sobel to the image\n"
                    send_text(conn, "Applying sobel ...")
                    apply_sobel(conn)
                    pass

    finally:
        conn.close()


def send_choice_text(conn):
    choice_text = \
        """------------------------------------------------------
        Please choose an option from the list below:
        \t1. Edit a new image
        \t2. Save the edited image
        \t3. Apply gray scale to the image
        \t4. Resize the image
        \t5. Rotate the image
        \t6. Apply sobel to the image
        \t7. Exit
        ------------------------------------------------------"""
    send_text(conn, choice_text)


def main():
    # Create an INET, STREAMing socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind(('', PORT))
        # Become a server socket
        s.listen()

        print("Server listening on port", PORT)

        while True:
            conn, addr = s.accept()
            threading.Thread(target=client_handler, args=(conn, addr)).start()


if __name__ == "__main__":
    main()
