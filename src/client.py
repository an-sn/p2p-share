import stun import argparse

    def perform_stun_request(stun_server = "stun.l.google.com", stun_port = 19302) :
#Perform STUN request to get the public IP and port
                             nat_type, external_ip, external_port = stun.get_ip_info(stun_host = stun_server, stun_port = stun_port)

                                                                                         if external_ip and external_port: return external_ip, external_port else :raise Exception("STUN request failed. Could not get public IP and port.")

                                                                                                                                                   def main() :parser = argparse.ArgumentParser(description = "Perform a STUN request and display the public IP and port.") parser.add_argument('--stun-server', default = 'stun.l.google.com', help = 'STUN server hostname') parser.add_argument('--stun-port', type = int, default = 19302, help = 'STUN server port')

                                                                                                                                                                                                                                                                                                                                                                                                                                                   args = parser.parse_args()

#Perform the STUN request
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                try :public_ip, public_port = perform_stun_request(args.stun_server, args.stun_port) print(f "Discovered public IP: {public_ip}, public Port: {public_port}")

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         except Exception as e:print(f "Error: {e}")

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             if __name__ == "__main__" :main()
