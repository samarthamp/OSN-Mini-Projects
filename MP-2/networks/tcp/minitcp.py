import socket
import struct
import time
import threading
import sys
import random

# Configuration
CHUNK_SIZE = 5            # Small chunk size to demonstrate splitting
TIMEOUT = 0.1             # 0.1 seconds retransmission timeout
DROP_RATE_N = 3           # Drop every Nth ACK to test retransmission
PACKET_FMT = '!III'       # Struct format: SeqNum, TotalChunks, Type (0=Data, 1=ACK)
HEADER_SIZE = struct.calcsize(PACKET_FMT)

# Packet Types
TYPE_DATA = 0
TYPE_ACK = 1

class MiniTCPNode:
    def __init__(self, port, peer_port, peer_ip='127.0.0.1'):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('0.0.0.0', port))
        self.peer_addr = (peer_ip, peer_port)
        self.running = True
        
        # Sender state
        self.sent_packets = {}    # Map: seq_num -> {data, timestamp, acked}
        self.total_chunks_sent = 0
        self.lock = threading.Lock()
        
        # Receiver state
        self.received_chunks = {} # Map: seq_num -> data
        self.ack_counter = 0      # To simulate packet loss

    def create_packet(self, seq_num, total_chunks, type, data=b''):
        header = struct.pack(PACKET_FMT, seq_num, total_chunks, type)
        return header + data

    def parse_packet(self, raw_bytes):
        header = raw_bytes[:HEADER_SIZE]
        payload = raw_bytes[HEADER_SIZE:]
        seq_num, total_chunks, type = struct.unpack(PACKET_FMT, header)
        return seq_num, total_chunks, type, payload

    # ------------------ RECEIVER LOGIC ------------------
    
    def start_receiver(self):
        print(f"[*] Listening on port {self.sock.getsockname()[1]}...")
        while self.running:
            try:
                data, addr = self.sock.recvfrom(1024)
                seq, total, type, payload = self.parse_packet(data)

                if type == TYPE_ACK:
                    self.handle_ack(seq)
                elif type == TYPE_DATA:
                    self.handle_data(seq, total, payload, addr)
            except Exception as e:
                print(f"Error: {e}")

    def handle_ack(self, seq):
        with self.lock:
            if seq in self.sent_packets:
                self.sent_packets[seq]['acked'] = True
                # print(f"[Sender] Received ACK for chunk {seq}")

    def handle_data(self, seq, total, payload, addr):
        # 1. Store Data
        if seq not in self.received_chunks:
            self.received_chunks[seq] = payload
            print(f"[Receiver] Received Chunk {seq}/{total}: {payload.decode()}")

        # 2. Send ACK (with simulated loss)
        self.ack_counter += 1
        
        # [2] Simulate Packet Loss: Skip every 3rd ACK
        if self.ack_counter % DROP_RATE_N == 0:
            print(f"[Receiver] SIMULATING LOSS: Dropping ACK for chunk {seq}")
            return 

        ack_packet = self.create_packet(seq, total, TYPE_ACK)
        self.sock.sendto(ack_packet, addr)
        # print(f"[Receiver] Sent ACK for chunk {seq}")

        # 3. Check if done
        if len(self.received_chunks) == total:
            self.assemble_message(total)

    def assemble_message(self, total):
        print("\n" + "="*30)
        print("MESSAGE COMPLETE:")
        sorted_data = [self.received_chunks[i].decode() for i in range(total)]
        full_text = "".join(sorted_data)
        print(f"'{full_text}'")
        print("="*30 + "\n")
        self.received_chunks.clear() # Reset for next message

    # ------------------ SENDER LOGIC ------------------

    def send_text(self, text):
        data_bytes = text.encode()
        chunks = [data_bytes[i:i+CHUNK_SIZE] for i in range(0, len(data_bytes), CHUNK_SIZE)]
        total_chunks = len(chunks)
        self.total_chunks_sent = total_chunks
        self.sent_packets = {}

        print(f"[*] Sending '{text}' in {total_chunks} chunks...")

        # 1. Prepare and send all chunks (Pipelining - do not wait)
        with self.lock:
            for i, chunk in enumerate(chunks):
                packet = self.create_packet(i, total_chunks, TYPE_DATA, chunk)
                self.sent_packets[i] = {
                    'packet': packet,
                    'time': time.time(),
                    'acked': False
                }
                self.sock.sendto(packet, self.peer_addr)
                print(f"[Sender] Sent chunk {i}")

        # 2. Monitor for Timeouts (Retransmission Loop)
        self.monitor_acks()

    def monitor_acks(self):
        while True:
            all_acked = True
            with self.lock:
                for seq, info in self.sent_packets.items():
                    if not info['acked']:
                        all_acked = False
                        # Check timeout
                        if time.time() - info['time'] > TIMEOUT:
                            print(f"[Sender] Timeout! Retransmitting chunk {seq}")
                            self.sock.sendto(info['packet'], self.peer_addr)
                            info['time'] = time.time() # Reset timer
            
            if all_acked and len(self.sent_packets) > 0:
                print("[*] All chunks acknowledged successfully.")
                break
            
            time.sleep(0.01) # Prevent CPU spinning

# ------------------ DRIVER CODE ------------------

def main():
    if len(sys.argv) != 3:
        print("Usage: python minitcp.py <mode: server|client> <port>")
        return

    mode = sys.argv[1]
    port = int(sys.argv[2])

    # To run on localhost, we define fixed ports for demo
    # Server listens on 'port', sends to 'client_port'
    # Client listens on 'client_port', sends to 'port'
    
    if mode == 'server':
        # Server listens on `port`, assumes client is on `port + 1`
        node = MiniTCPNode(port, port + 1)
        
        # Start listener thread
        t = threading.Thread(target=node.start_receiver)
        t.daemon = True
        t.start()

        print("Server ready. Waiting for data... (or type to send)")
        while True:
            msg = input()
            if msg:
                node.send_text(msg)

    elif mode == 'client':
        # Client listens on `port + 1`, sends to `port` (Server)
        my_port = port + 1
        node = MiniTCPNode(my_port, port)
        
        # Start listener thread
        t = threading.Thread(target=node.start_receiver)
        t.daemon = True
        t.start()

        # Allow time for threads to start
        time.sleep(1)
        node.send_text("Hello_World_This_Is_TCP_Implementation")
        
        # Keep alive to receive ACKs/responses
        while True:
            msg = input()
            if msg:
                node.send_text(msg)

if __name__ == "__main__":
    main()