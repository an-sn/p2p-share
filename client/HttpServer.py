from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import threading
import logging
import os

logging.getLogger("http.server").setLevel(logging.ERROR)

class HttpFileHandler(BaseHTTPRequestHandler):
    def __init__(self, *args, available_files={}, **kwargs):
        self.available_files = available_files
        super().__init__(*args, **kwargs)
    def do_GET(self):
        if self.headers['Content-Type'] != 'application/json':
            self.send_response(400)
            self.end_headers()
            return
        try:
            content_length = int(self.headers['Content-Length'])
            request_body = self.rfile.read(content_length)
            json_data = json.loads(request_body)
            (file_uuid, chunk_id) = (json_data["file_uuid"], json_data["chunk_id"])
            if file_uuid not in self.available_files:
                self.send_response(404)
                self.end_headers()
                return
            chunk_path = os.path.join("/tmp/p2p_client_data", file_uuid, "chunks/", f"chunk{str(chunk_id)}")
            if not os.path.isfile(chunk_path):
                self.send_response(404)
                self.end_headers()
                return
            with open(chunk_path, 'rb') as chunk_file:
                chunk_data = chunk_file.read()
            self.send_response(200)
            self.send_header('Content-Type', 'application/octet-stream')
            self.send_header('Content-Length', str(len(chunk_data)))
            self.end_headers()
            self.wfile.write(chunk_data)
        except Exception as e:
            print("Exception during file retrieve: ", e)
            self.send_response(500)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()

def run_http_file_server(ip='', port=9192, available_files={}):
    def handler(*args, **kwargs):
        HttpFileHandler(*args, available_files=available_files, **kwargs)
    server_address = (ip, port)
    httpd = HTTPServer(server_address, handler)
    httpd.serve_forever()

def start_http_server_thread(ip='', port=9192, available_files={}):
    server_thread = threading.Thread(target=run_http_file_server, args=(ip, port, available_files))
    server_thread.daemon = True
    server_thread.start()
    return server_thread