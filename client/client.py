import argparse
import sys
import os
import threading
import requests
import uuid
import shutil
import hashlib
import atexit
from HttpServer import *

CHUNK_SIZE = 1 * 1024 * 1024
peer_uuid = None
available_files = {}
save_file_path = "/tmp/peer_data.json"

def validate_file(file_name):
    if not os.path.isfile(file_name):
        print(f"{file_name} is not a valid file!")
        return False
    return True

def chunk_file(file_name, chunks_path):
    # base_name = os.path.basename(file_name)
    chunk_hashes = []
    chunk_index = 0
    with open(file_name, 'rb') as f:
        while True:
            chunk_data = f.read(CHUNK_SIZE)
            if not chunk_data:
                break
            chunk_file_name = os.path.join(chunks_path, f"chunk{chunk_index}")
            with open(chunk_file_name, 'wb') as chunk_file:
                chunk_file.write(chunk_data)
            chunk_hash = hashlib.sha256(chunk_data).hexdigest()
            chunk_hashes.append(chunk_hash)
            chunk_index += 1
    return chunk_hashes

def print_available_files():
    if not available_files:
        print("No files available.")
        return
    print("\nAvailable Files:")
    print("-" * 50)
    for file_uuid, info in available_files.items():
        print(f"File name: {info['file_name']}")
        print(f"  UUID: {file_uuid}")
        print(f"  Description: {info['file_description']}")
        print(f"  Size: {info['file_size']} bytes")
        print("-" * 50)

def file_list(print = False):
    try:
        header = {'Content-Type': 'application/json'}
        response = requests.get(f"{server_url}/file_list", headers=header)
        response.raise_for_status()
        response_json = response.json()
        for file_uuid, file_metadata in response_json.items():
            file_info = {
                "file_name" : file_metadata['file_name'],
                "file_description" : file_metadata['file_description'],
                "file_size" : file_metadata['file_size']
            }
            available_files[file_uuid] = file_info
        if print:
            print_available_files()
    except requests.exceptions.RequestException as e:
        print(f"Failed to send file list request, exception: {e}")
        return None

def send_chunk_download_failure(file_uuid, chunk_index, chunk_peer_uuid):
    try:
        chunk_download_fail = {
            "file_uuid" : file_uuid,
            "chunk_index" : chunk_index,
            "peer_uuid" : chunk_peer_uuid
        }
        json_req = json.dumps(chunk_download_fail)
        header = {'Content-Type': 'application/json', 'Content-Length': str(len(json_req))}
        response = requests.post(f"{server_url}/chunk_dl_fail", headers=header, data=json_req)
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"Could not notify server of file:{file_uuid}::chunk:{chunk_index} download failure, exception: {e}")

def download_chunk(file_uuid, chunk_index, chunk_info, download_path):
    peers = chunk_info["peers"]
    for peer in peers:
        (peer_ip, peer_port, chunk_peer_uuid) = (peer["ip"], peer["port"], peer["peer_uuid"])
        peer_url = "http://" + peer_ip + ":" + str(peer_port)
        print(f"Downloading file:{file_uuid}:chunk:{chunk_index} from peer: ({peer_ip}, {peer_port})")
        try:
            chunk_request = {
                "file_uuid" : file_uuid,
                "chunk_id" : chunk_index
            }
            json_req = json.dumps(chunk_request)
            header = {'Content-Type': 'application/json', 'Content-Length': str(len(json_req))}
            response = requests.get(f"{peer_url}/chunk_request", headers=header, data=json_req)
            response.raise_for_status()
            chunk_dir = os.path.join(download_path, "chunks")
            if not os.path.exists(chunk_dir):
                os.makedirs(chunk_dir)
            chunk_file_path = os.path.join(chunk_dir, f"chunk{chunk_index}")
            sha256_hash = hashlib.sha256()
            with open(chunk_file_path, 'wb') as chunk_file:
                for chunk in response.iter_content(chunk_size=4096):
                    if chunk:
                        chunk_file.write(chunk)
                        sha256_hash.update(chunk)
            downloaded_chunk_hash = sha256_hash.hexdigest()
            if(downloaded_chunk_hash == chunk_info["hash"]):
                chunk_advert(file_uuid, chunk_index)
                return True
            else:
                print(f"Mismatch in chunk hash, expected: {chunk_info['hash']}, Actual : f{downloaded_chunk_hash}")
                send_chunk_download_failure(file_uuid, chunk_index, chunk_peer_uuid)
        except requests.exceptions.RequestException as e:
            print(f"Failed to get file:{file_uuid}::chunk:{chunk_index}, exception: {e}")
            send_chunk_download_failure(file_uuid, chunk_index, chunk_peer_uuid)
    print(f"Failed to download file:{file_uuid}::chunk:{chunk_index} from any peer!")

def reassemble_chunks(file_name, download_path, number_of_chunks):
    chunk_dir = os.path.join(download_path, "chunks")
    target_file_path = os.path.join(download_path, file_name)
    with open(target_file_path, 'wb') as f:
        for chunk_index in range(number_of_chunks):
            chunk_file_path = os.path.join(chunk_dir, f"chunk{chunk_index}")
            with open(chunk_file_path, 'rb') as chunk_file:
                chunk_data = chunk_file.read()
                f.write(chunk_data)

def download_file(file_uuid, file_details, download_path):
    chunk_index = 0
    chunk_list = file_details["chunks"]
    entire_file_downloaded = True
    missing_chunks = []
    for chunk_info in chunk_list:
        chunk_downloaded = download_chunk(file_uuid, chunk_index, chunk_info, download_path)
        if not chunk_downloaded:
            entire_file_downloaded = False
            missing_chunks.append(chunk_index)
        chunk_index += 1
    file_name = available_files[file_uuid]["file_name"]
    if entire_file_downloaded:
        reassemble_chunks(file_name, download_path, len(chunk_list))
        print(f"{file_name} has been downloaded successfully.")

# HTTP requests

def file_request(file_uuid):
    if file_uuid not in available_files:
        print("Entered file UUID is not valid!")
        return
    download_path = f"/tmp/p2p_client_data/{file_uuid}/"
    try:
        header = {'Content-Type': 'application/json'}
        file_request_msg = {
            "file_uuid" : file_uuid
        }
        response = requests.get(f"{server_url}/file_request", headers=header, data=json.dumps(file_request_msg))
        response.raise_for_status()
        download_file(file_uuid, response.json(), download_path)
    except requests.exceptions.RequestException as e:
        print(f"Failed to send the file request, exception: {e}")

def file_advert(file_name, chunk_size):
    if not validate_file(file_name):
        return
    file_uuid = str(uuid.uuid4())
    destination_path = os.path.join("/tmp/p2p_client_data/", file_uuid)
    if not os.path.exists(destination_path):
        os.makedirs(destination_path)
    chunks_path = os.path.join(destination_path, "chunks")
    if not os.path.exists(chunks_path):
        os.makedirs(chunks_path)
    chunk_hashes = chunk_file(file_name, chunks_path)
    try:
        file_advert_msg = {
            "peer_uuid": peer_uuid,
            "file_name": os.path.basename(file_name),
            "file_uuid": file_uuid,
            "file_size": os.path.getsize(file_name),
            "file_description": "Hello",
            "total_chunks": len(chunk_hashes),
            "chunk_hashes": chunk_hashes
        }
        header = {'Content-Type': 'application/json'}
        response = requests.post(f"{server_url}/file_advert", headers=header, data=json.dumps(file_advert_msg))
        response.raise_for_status()
        file_info = {
                "file_name" : os.path.basename(file_name),
                "file_description" : "",
                "file_size" : os.path.getsize(file_name)
        }
        available_files[file_uuid] = file_info
    except requests.exceptions.RequestException as e:
        print(f"Failed to send file advert, exception: {e}")

def chunk_advert(file_uuid, chunk_index):
    try:
        chunk_advert_msg = {
            "file_uuid" : file_uuid,
            "peer_uuid" : peer_uuid,
            "chunk_index" : chunk_index
        }
        header = {'Content-Type': 'application/json'}
        response = requests.post(f"{server_url}/chunk_advert", headers=header, data=json.dumps(chunk_advert_msg))
        response.raise_for_status()
    except requests.exceptions.RequestException as e:
        print(f"Failed to send chunk advertisement, exception: {e}")

def connect_to_server(server_url, client_ip, client_port, reconnect):
    global peer_uuid
    header = {'Content-Type': 'application/json'}
    try:
        discovery_request = {}
        if reconnect:
            discovery_request = {
                "peer_uuid" : peer_uuid,
                "peer_ip": client_ip,
                "peer_port": client_port
            }
        else:
            discovery_request = {
                "peer_ip": client_ip,
                "peer_port": client_port
            }
        response = requests.post(f"{server_url}/discovery", headers=header, data=json.dumps(discovery_request))
        response.raise_for_status()
        if reconnect:
            return True
        else:
            peer_uuid = response.json()['uuid']
            return True
    except requests.exceptions.RequestException as e:
        print(f"Failed to connect to server, exception: {e}")
        return None

# Command handling

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
        "usage": "file_request <file_uuid>",
        "description": "Get file details from server and download if all chunks can be found."
    },
    "exit": {
        "usage": "exit",
        "description": "Shutdown the client. Peer UUID and available_file list will be saved."
    }
}

def generate_command_options(commands):
    message = "\nCommand options:\n"
    for idx, (command, details) in enumerate(commands.items(), 1):
        message += f"{idx}. {command} : {details['description']}\n"
        message += f"   Usage : {details['usage']}\n\n"
    message += "> "
    return message

def shutdown_client():
    print("Exiting client...")
    sys.exit(0)

def load_data_on_startup():
    global peer_uuid, available_files
    try:
        with open(save_file_path, 'r') as file:
            data_loaded = json.load(file)
            peer_uuid = data_loaded.get("peer_uuid")
            available_files = data_loaded.get("available_files", {})
    except FileNotFoundError:
        print(f"No saved data found at {save_file_path}, starting fresh.")
    except json.JSONDecodeError:
        print(f"Error decoding JSON data from {save_file_path}, starting fresh.")
    except Exception as e:
        print(f"Failed to load data on startup: {e}")

def save_data_on_shutdown():
    if peer_uuid is None:
        return
    data_to_save = {
        "peer_uuid": peer_uuid,
        "available_files": available_files
    }
    try:
        with open(save_file_path, 'w') as file:
            json.dump(data_to_save, file, indent=4)
        print(f"Data saved to {save_file_path}")
    except Exception as e:
        print(f"Failed to save data on shutdown: {e}")

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
        elif command_type == "file_list":
            file_list(True)
        elif command_type == "file_request":
            if(len(command_parts) == 2):
                file_request(command_parts[1].strip())
        elif command_type == "exit":
            shutdown_client()

# Main function

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="P2P file transfer client")
    parser.add_argument('--server_ip', type=str, required=True, help="IP address of the machine where the server is running")
    parser.add_argument('--server_port', type=str, required=True, help="Port on which the server is listening (default: 5100).")
    parser.add_argument('--client_ip', type=str, required=True, help="IP address of the machine where the client is running")
    parser.add_argument('--client_port', type=str, required=True, help="Any user-defined port that the client will use for communication (choose a port that is not in use).")
    parser.add_argument('--reconnect_peer', action='store_true', help="Peer will reuse its old context while connecting back to the tracker.")
    args = parser.parse_args()
    server_url = f"http://{args.server_ip}:{args.server_port}"
    reconnect = args.reconnect_peer
    atexit.register(save_data_on_shutdown)
    if reconnect:
        load_data_on_startup()
    connect_response = connect_to_server(server_url, args.client_ip, args.client_port, reconnect)
    if connect_response == None:
        sys.exit(1)
    file_list()
    start_http_server_thread(args.client_ip, int(args.client_port), available_files)
    wait_for_command()