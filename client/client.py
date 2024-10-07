import sys
import requests
import json
import socket
import os
import shutil
import hashlib
import uuid

CHUNK_SIZE = 1 * 1024 * 1024
peer_uuid = None
advertised_files = {}

def validate_file(file_name):
    if not os.path.isfile(file_name):
        print(f"{file_name} is not a valid file!")
        return False
    return True

def chunk_file(file_name, chunk_dir):
    base_name = os.path.basename(file_name)
    if not os.path.exists(chunk_dir):
        os.makedirs(chunk_dir)
    chunk_hashes = []
    chunk_index = 0
    with open(file_name, 'rb') as f:
        while True:
            chunk_data = f.read(CHUNK_SIZE)
            if not chunk_data:
                break
            chunk_file_name = os.path.join(chunk_dir, f"{base_name}_chunk_{chunk_index}")
            with open(chunk_file_name, 'wb') as chunk_file:
                chunk_file.write(chunk_data)
            chunk_hash = hashlib.sha256(chunk_data).hexdigest()
            chunk_hashes.append(chunk_hash)
            chunk_index += 1
    return chunk_hashes

def file_list():
    pass

def file_request():
    pass

def file_advert(file_name, chunk_size):
    if not validate_file(file_name):
        return
    file_dir_name = os.path.dirname(file_name)
    base_name = os.path.basename(file_name)
    temp_dir = os.path.join(file_dir_name, "file_copy")
    if not os.path.exists(temp_dir):
        os.makedirs(temp_dir)
    destination_file_path = os.path.join(temp_dir, base_name)
    shutil.copy2(file_name, destination_file_path)
    chunks_path = os.path.join(temp_dir, "chunks")
    chunk_hashes = chunk_file(destination_file_path, chunks_path)
    try:
        file_uuid = str(uuid.uuid4())
        file_advert_msg = {
            "peer_uuid": peer_uuid,
            "file_name": base_name,
            "file_uuid": file_uuid,
            "file_size": os.path.getsize(file_name),
            "file_description": "Hello",
            "total_chunks": len(chunk_hashes),
            "chunk_hashes": chunk_hashes
        }
        header = {'Content-Type': 'application/json'}
        response = requests.post(f"{server_url}/file_advert", headers=header, data=json.dumps(file_advert_msg))
        response.raise_for_status()
        advertised_files[file_uuid] = {"path" : destination_file_path, "chunk_dir": chunks_path}
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Failed to send file request, exception: {e}")
        return None

def connect_to_server(server_url):
    header = {'Content-Type': 'application/json'}
    try:
        discovery_request = {
            "peer_ip": "192.168.0.190",
            "peer_port": 1234
        }
        response = requests.post(f"{server_url}/discovery", headers=header, data=json.dumps(discovery_request))
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Failed to connect to server, exception: {e}")
        return None

commands = {
    "file_advert": {
        "usage": "file_advert <path_to_file>",
        "description": "Send an advertisement to the server with file details."
    },
    "file_list": {
        "usage": "file_list",
        "description": "Get a list of files that the server is currently aware of."
    },
    "file_request": {
        "usage": "file_request <file_name>",
        "description": "Get file details from server and download if all chunks can be found."
    }
}

def generate_command_options(commands):
    message = "\nCommand options:\n"
    for idx, (command, details) in enumerate(commands.items(), 1):
        message += f"{idx}. {command} : {details['description']}\n"
        message += f"   Usage : {details['usage']}\n\n"
    message += "> "
    return message

command_message = generate_command_options(commands)

def wait_for_command():
    while True:
        command = input(command_message).strip()
        command_parts = command.split(" ", 1)
        command_type = command_parts[0].lower()
        if command_type == "file_advert":
            if(len(command_parts) == 2):
                file_advert(command_parts[1].strip(), 1024*1024)
            else:
                print(commands["file_advert"]["usage"])

if __name__ == "__main__":
    server_url = "http://192.168.0.190:5100"
    discovery_response = connect_to_server(server_url)
    if discovery_response == None:
        sys.exit(1)
    peer_uuid = discovery_response['uuid']
    print(f"\nConnected to server!")
    wait_for_command()