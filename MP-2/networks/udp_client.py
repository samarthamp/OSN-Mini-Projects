#!/usr/bin/env python3
"""
UDP Tic-Tac-Toe Client with GUI
Connects to the server and provides a graphical interface for playing
"""

import socket
import threading
import json
import time
import tkinter as tk
from tkinter import messagebox, font

HOST = '127.0.0.1'
PORT = 65433


class TicTacToeUDPClient:
    def __init__(self):
        self.socket = None
        self.server_addr = (HOST, PORT)
        self.player_id = None
        self.symbol = None
        self.is_my_turn = False
        self.game_active = False
        self.connected = False
        self.running = False
        self.scores = {0: 0, 1: 0}  # Track wins for each player
        
        # Create GUI
        self.root = tk.Tk()
        self.root.title("Tic-Tac-Toe - UDP")
        self.root.resizable(False, False)
        self.root.configure(bg='#2c3e50')
        
        self.setup_gui()
        
    def setup_gui(self):
        """Setup the graphical user interface"""
        # Title
        title_font = font.Font(family='Helvetica', size=24, weight='bold')
        self.title_label = tk.Label(
            self.root, 
            text="Tic-Tac-Toe (UDP)", 
            font=title_font, 
            bg='#2c3e50', 
            fg='#ecf0f1'
        )
        self.title_label.pack(pady=10)
        
        # Scoreboard frame
        scoreboard_frame = tk.Frame(self.root, bg='#34495e', padx=20, pady=10)
        scoreboard_frame.pack(pady=5, fill=tk.X, padx=20)
        
        score_font = font.Font(family='Helvetica', size=14, weight='bold')
        score_label_font = font.Font(family='Helvetica', size=11)
        
        # Player 1 score
        p1_frame = tk.Frame(scoreboard_frame, bg='#34495e')
        p1_frame.pack(side=tk.LEFT, expand=True)
        tk.Label(p1_frame, text="Player 1", font=score_label_font, bg='#34495e', fg='#bdc3c7').pack()
        self.p1_score_label = tk.Label(p1_frame, text="0", font=score_font, bg='#34495e', fg='#000000')
        self.p1_score_label.pack()
        
        # VS label
        tk.Label(scoreboard_frame, text="-", font=score_font, bg='#34495e', fg='#ecf0f1').pack(side=tk.LEFT, expand=True)
        
        # Player 2 score
        p2_frame = tk.Frame(scoreboard_frame, bg='#34495e')
        p2_frame.pack(side=tk.LEFT, expand=True)
        tk.Label(p2_frame, text="Player 2", font=score_label_font, bg='#34495e', fg='#bdc3c7').pack()
        self.p2_score_label = tk.Label(p2_frame, text="0", font=score_font, bg='#34495e', fg='#000000')
        self.p2_score_label.pack()
        
        # Status label
        status_font = font.Font(family='Helvetica', size=12)
        self.status_label = tk.Label(
            self.root, 
            text="Click 'Connect' to join a game", 
            font=status_font, 
            bg='#2c3e50', 
            fg='#bdc3c7',
            wraplength=300
        )
        self.status_label.pack(pady=5)
        
        # Player info
        self.player_label = tk.Label(
            self.root, 
            text="", 
            font=status_font, 
            bg='#2c3e50', 
            fg='#e67e22'  # Orange color to distinguish from TCP client
        )
        self.player_label.pack(pady=5)
        
        # Game board frame
        self.board_frame = tk.Frame(self.root, bg='#34495e', padx=10, pady=10)
        self.board_frame.pack(pady=10)
        
        # Create board buttons
        self.buttons = [[None for _ in range(3)] for _ in range(3)]
        button_font = font.Font(family='Helvetica', size=36, weight='bold')
        
        for i in range(3):
            for j in range(3):
                btn = tk.Button(
                    self.board_frame,
                    text=' ',
                    font=button_font,
                    width=3,
                    height=1,
                    bg='#ecf0f1',
                    activebackground='#bdc3c7',
                    command=lambda r=i, c=j: self.make_move(r, c),
                    state=tk.DISABLED
                )
                btn.grid(row=i, column=j, padx=3, pady=3)
                self.buttons[i][j] = btn
        
        # Control buttons frame
        control_frame = tk.Frame(self.root, bg='#2c3e50')
        control_frame.pack(pady=10)
        
        button_style = {
            'font': font.Font(family='Helvetica', size=11),
            'width': 10,
            'height': 1
        }
        
        self.connect_btn = tk.Button(
            control_frame, 
            text="Connect", 
            command=self.connect_to_server,
            bg='#27ae60', 
            fg='white',
            activebackground='#2ecc71',
            **button_style
        )
        self.connect_btn.pack(side=tk.LEFT, padx=5)
        
        self.ready_btn = tk.Button(
            control_frame, 
            text="Ready", 
            command=self.send_ready,
            bg='#e67e22', 
            fg='white',
            activebackground='#f39c12',
            state=tk.DISABLED,
            **button_style
        )
        self.ready_btn.pack(side=tk.LEFT, padx=5)
        
        self.quit_btn = tk.Button(
            control_frame, 
            text="Quit", 
            command=self.quit_game,
            bg='#e74c3c', 
            fg='white',
            activebackground='#ec7063',
            **button_style
        )
        self.quit_btn.pack(side=tk.LEFT, padx=5)
        
    def connect_to_server(self):
        """Connect to the game server"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.socket.settimeout(1.0)
            self.running = True
            self.connected = True
            
            # Send connect message
            self.send_message({'type': 'connect'})
            
            self.connect_btn.config(state=tk.DISABLED)
            self.status_label.config(text="Connecting to server...")
            
            # Start receive thread
            receive_thread = threading.Thread(target=self.receive_messages)
            receive_thread.daemon = True
            receive_thread.start()
            
            # Start heartbeat thread
            heartbeat_thread = threading.Thread(target=self.send_heartbeat)
            heartbeat_thread.daemon = True
            heartbeat_thread.start()
            
        except Exception as e:
            messagebox.showerror("Connection Error", f"Could not connect to server: {e}")
            
    def send_message(self, message):
        """Send a message to the server"""
        try:
            if self.socket and self.running:
                data = json.dumps(message).encode()
                self.socket.sendto(data, self.server_addr)
        except Exception as e:
            print(f"Error sending message: {e}")
            
    def send_heartbeat(self):
        """Send periodic heartbeat to server"""
        while self.running:
            try:
                if self.connected:
                    self.send_message({'type': 'heartbeat'})
                time.sleep(3)  # Send heartbeat every 3 seconds
            except:
                break
            
    def send_ready(self):
        """Send ready signal to server"""
        self.send_message({'type': 'ready'})
        self.ready_btn.config(state=tk.DISABLED)
        self.status_label.config(text="Waiting for opponent...")
        
    def make_move(self, row, col):
        """Send a move to the server"""
        if self.is_my_turn and self.game_active:
            self.send_message({
                'type': 'move',
                'row': row,
                'col': col
            })
            
    def update_board(self, board):
        """Update the GUI board"""
        colors = {'X': '#3498db', 'O': '#e74c3c', ' ': '#ecf0f1'}  # X=blue, O=red, empty=background
        
        for i in range(3):
            for j in range(3):
                symbol = board[i][j]
                self.buttons[i][j].config(
                    text=symbol if symbol != ' ' else ' ',
                    fg=colors.get(symbol, '#2c3e50'),
                    disabledforeground=colors.get(symbol, '#2c3e50')
                )
                
    def enable_board(self, enable):
        """Enable or disable the board buttons"""
        state = tk.NORMAL if enable else tk.DISABLED
        for i in range(3):
            for j in range(3):
                if self.buttons[i][j]['text'] == ' ':
                    self.buttons[i][j].config(state=state)
                else:
                    self.buttons[i][j].config(state=tk.DISABLED)
                    
    def reset_board_gui(self):
        """Reset the board GUI for a new game"""
        for i in range(3):
            for j in range(3):
                self.buttons[i][j].config(text=' ', state=tk.DISABLED)
                
    def update_scoreboard(self):
        """Update the scoreboard display"""
        self.p1_score_label.config(text=str(self.scores[0]))
        self.p2_score_label.config(text=str(self.scores[1]))
                
    def receive_messages(self):
        """Receive and process messages from the server"""
        while self.running:
            try:
                data, addr = self.socket.recvfrom(4096)
                message = json.loads(data.decode())
                self.root.after(0, self.process_message, message)
            except socket.timeout:
                continue
            except json.JSONDecodeError:
                print("Received invalid JSON")
            except Exception as e:
                if self.running:
                    print(f"Receive error: {e}")
                break
                
    def process_message(self, message):
        """Process a message from the server"""
        msg_type = message.get('type')
        
        if msg_type == 'heartbeat_ack':
            return  # Ignore heartbeat acknowledgments
            
        if msg_type == 'assignment':
            self.player_id = message['player_id']
            self.symbol = message['symbol']
            self.player_label.config(
                text=f"You are Player {self.player_id + 1} ({self.symbol})"
            )
            self.ready_btn.config(state=tk.NORMAL)
            self.status_label.config(text="Connected! Click 'Ready' when you're ready to play.")
            
        elif msg_type == 'waiting':
            self.status_label.config(text=message['message'])
            
        elif msg_type == 'game_start':
            self.game_active = True
            self.update_board(message['board'])
            current = message['current_player']
            self.is_my_turn = (current == self.player_id)
            
            if self.is_my_turn:
                self.status_label.config(text="Game started! Your turn.")
                self.enable_board(True)
            else:
                self.status_label.config(text="Game started! Waiting for opponent...")
                self.enable_board(False)
                
        elif msg_type == 'board_update':
            self.update_board(message['board'])
            current = message['current_player']
            self.is_my_turn = (current == self.player_id)
            
            if self.is_my_turn:
                self.status_label.config(text="Your turn!")
                self.enable_board(True)
            else:
                self.status_label.config(text="Opponent's turn...")
                self.enable_board(False)
                
        elif msg_type == 'error':
            if "Game is full" in message.get('message', ''):
                messagebox.showerror("Error", message['message'])
                self.cleanup()
            else:
                self.status_label.config(text=message['message'])
            
        elif msg_type == 'symbol_swap':
            self.symbol = message['symbol']
            self.player_label.config(
                text=f"You are Player {self.player_id + 1} ({self.symbol})"
            )
            self.status_label.config(text=message['message'])
            
        elif msg_type == 'game_over':
            self.game_active = False
            self.update_board(message['board'])
            self.enable_board(False)
            
            result_msg = message['message']
            self.status_label.config(text=result_msg)
            
            # Update scores if there's a winner
            if message['result'] == 'win':
                winner_id = message['winner']
                self.scores[winner_id] += 1
                self.update_scoreboard()
            
            # Ask to play again
            play_again = messagebox.askyesno("Game Over", f"{result_msg}\n\nWould you like to play again?\n(Sides will be swapped)")
            self.send_message({
                'type': 'play_again',
                'response': play_again
            })
            
            if not play_again:
                self.status_label.config(text="Waiting for game to end...")
            else:
                self.status_label.config(text="Waiting for opponent's response...")
                
        elif msg_type == 'new_game':
            self.reset_board_gui()
            self.game_active = True
            self.status_label.config(text=message['message'])
            
        elif msg_type == 'game_ended':
            self.status_label.config(text=message['message'])
            messagebox.showinfo("Game Ended", message['message'])
            self.cleanup()
            
        elif msg_type == 'opponent_disconnected':
            self.status_label.config(text="Opponent disconnected!")
            messagebox.showinfo("Disconnected", "Your opponent has disconnected.")
            self.cleanup()
            
    def cleanup(self):
        """Clean up connection and reset GUI"""
        self.connected = False
        self.game_active = False
        self.running = False
        if self.socket:
            try:
                self.send_message({'type': 'disconnect'})
                self.socket.close()
            except:
                pass
            self.socket = None
        
        self.connect_btn.config(state=tk.NORMAL)
        self.ready_btn.config(state=tk.DISABLED)
        self.enable_board(False)
        self.player_label.config(text="")
        
    def quit_game(self):
        """Quit the game"""
        self.running = False
        if self.connected and self.socket:
            try:
                self.send_message({'type': 'disconnect'})
                self.socket.close()
            except:
                pass
        self.root.destroy()
        
    def run(self):
        """Run the client GUI"""
        self.root.protocol("WM_DELETE_WINDOW", self.quit_game)
        self.root.mainloop()


if __name__ == '__main__':
    client = TicTacToeUDPClient()
    client.run()
