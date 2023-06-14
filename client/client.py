import socket
import os
import struct
import threading
import pathlib

MAXLINE = 512
MAX_THREADS = 100
PORT = 5006

directoryPath = None
sockfd_global = None

def get_current_dir():
    global directoryPath
    directoryPath = os.getcwd() 
    print(f"Current working directory: {directoryPath}")

def file_thread(entry):
    fullPath = os.path.join(directoryPath, entry)
    fileStat = os.stat(fullPath)
    perms = "---------"
    fileMode = fileStat.st_mode
    perms = ''.join([perms[i-1] if fileMode & (1<<(9-i)) else '-' for i in range(9, 0, -1)])
    print(f"\tFile: {entry}, Size: {fileStat.st_size}  bytes, Permissions: {perms}")

def check_directory():
    directoryPath = os.getcwd()
    print(f"\nReading from: {directoryPath}")
    for item in os.listdir(directoryPath):
        print(item)

def receive_text():
    buffer = sockfd_global.recv(MAXLINE).decode()
    print(f"Server: {buffer}")
    return buffer

def my_read_size(nr1, remainSize):
    if nr1 < remainSize:
        remainSize -= nr1
    else:
        nr1 = remainSize
        remainSize = 0
    return nr1, remainSize

def receive_image():
    image_name = receive_text()
    print(f"Image name received is {image_name}")
    with open(image_name, 'wb') as picture:
        sizePic = struct.unpack('i', sockfd_global.recv(4))[0]
        print(f"Received Picture Size: {sizePic}")
        read_size, sizePic = my_read_size(MAXLINE, sizePic)
        while read_size > 0:
            recv_buffer = sockfd_global.recv(read_size)
            picture.write(recv_buffer)
            read_size, sizePic = my_read_size(MAXLINE, sizePic)
    return image_name

def send_text(buffer):
    sockfd_global.send(buffer.encode())
    print("Message sent to server.")

def file_exists(image_path):
    if os.path.exists(image_path):
        print(f"File '{image_path}' exists.")
        if image_path.endswith(".png") or image_path.endswith(".jpg"):
            return True
    print(f"File '{image_path}' does not exist or has wrong extension.")
    return False

def send_image():
    image_path = input("Enter a image path: ").strip()
    while not file_exists(image_path):
        check_directory()
        print("Wrong path or extension (not .jpg or .png)\nTry again ...")
        image_path = input("Enter a image path: ").strip()
    with open(image_path, 'rb') as picture:
        sizePic = os.path.getsize(image_path)
        sockfd_global.send(struct.pack('i', sizePic))
        print(f"Sent Picture Size: {sizePic}")
        while (send_buffer := picture.read(MAXLINE)):
            sockfd_global.send(send_buffer)

def main():
    global sockfd_global
    sockfd_global = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    server_address = ('localhost', PORT)
    sockfd_global.connect(server_address)
    print("Connected to the server..")

    while True:
        text = receive_text() # choice list
        if "Please choose an option" in text:
            choice = int(input("Your choice: "))
            print(f"Choice selected : {choice}")
            send_text(str(choice)) # send choice
            
            text = receive_text()
            if "Give me you image" in text: #1
                print("Sending image option selected.")
                send_image()
            elif "Returning image" in text: #2
                print("Receiving image ...")
                receive_image()
            elif "Closing program" in text: #7
                print("Finishing program ...")
                break
            elif "resize" in text: #4
                resize = float(input("Enter resize value: "))
                send_text(str(resize))
                receive_text()
            elif "rotate" in text: #5
                angle = float(input("Enter angle value: "))
                send_text(str(angle))
                receive_text()
            elif "Changes apply" in text:
                print("good")
            else:
                print("Nothing done.")
    
    sockfd_global.close()

if __name__ == "__main__":
    main()
