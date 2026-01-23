#!/usr/bin/env python3
"""
UDP Tic-Tac-Toe Server
Manages game state and communication between two players using UDP
"""

import socket
import json
import time
from threading import Lock

HOST = '127.0.0.1'
PORT = 65433


class TicTacToeUDPServer:
    def __init__(self):
        self.board = [[' ' for _ in range(3)] for _ in range(3)]
        self.players = {}  # addr -> player_id
        self.player_addrs = {}  # player_id -> addr
        self.current_player = 0  # 0 for Player 1, 1 for Player 2
        self.player_symbols = {0: 'X', 1: 'O'}  # Maps player_id to symbol
        self.game_active = False
        self.ready_players = set()
        self.play_again_responses = {}
        self.game_count = 0  # Track number of games played
        self.lock = Lock()
        self.last_heartbeat = {}  # Track player heartbeats
        self.socket = None
        
    def reset_board(self):
        """Reset the game board for a new game"""
        self.board = [[' ' for _ in range(3)] for _ in range(3)]
        self.game_active = True
        self.play_again_responses = {}
        
    def swap_symbols(self):
        """Swap player symbols for the next game"""
        self.player_symbols[0], self.player_symbols[1] = self.player_symbols[1], self.player_symbols[0]
        # Player with X always goes first
        self.current_player = 0 if self.player_symbols[0] == 'X' else 1
        
    def check_winner(self):
        """Check if there's a winner. Returns 'X', 'O', 'Draw', or None"""
        # Check rows
        for row in self.board:
            if row[0] == row[1] == row[2] != ' ':
                return row[0]
        
        # Check columns
        for col in range(3):
            if self.board[0][col] == self.board[1][col] == self.board[2][col] != ' ':
                return self.board[0][col]
        
        # Check diagonals
        if self.board[0][0] == self.board[1][1] == self.board[2][2] != ' ':
            return self.board[0][0]
        if self.board[0][2] == self.board[1][1] == self.board[2][0] != ' ':
            return self.board[0][2]
        
        # Check for draw
        if all(self.board[i][j] != ' ' for i in range(3) for j in range(3)):
            return 'Draw'
        
        return None
    
    def is_valid_move(self, row, col):
        """Check if a move is valid"""
        if 0 <= row < 3 and 0 <= col < 3:
            return self.board[row][col] == ' '
        return False
    
    def make_move(self, row, col, symbol):
        """Make a move on the board"""
        self.board[row][col] = symbol
        
    def send_to_player(self, player_id, message):
        """Send a message to a specific player"""
        try:
            addr = self.player_addrs.get(player_id)
            if addr:
                data = json.dumps(message).encode()
                self.socket.sendto(data, addr)
        except Exception as e:
            print(f"Error sending to player {player_id + 1}: {e}")
            
    def broadcast(self, message):
        """Send a message to all connected players"""
        for player_id in self.player_addrs:
            self.send_to_player(player_id, message)
            
    def process_message(self, addr, message):
        """Process a message from a player"""
        msg_type = message.get('type')
        
        if msg_type == 'connect':
            with self.lock:
                if addr in self.players:
                    player_id = self.players[addr]
                    self.last_heartbeat[player_id] = time.time()
                    self.send_to_player(player_id, {
                        'type': 'assignment',
                        'player_id': player_id,
                        'symbol': self.symbols[player_id]
                    })
                    return
                    
                if len(self.players) >= 2:
                    data = json.dumps({
                        'type': 'error',
                        'message': 'Game is full. Please try again later.'
                    }).encode()
                    self.socket.sendto(data, addr)
                    return
                
                # Assign player ID
                player_id = 0 if 0 not in self.player_addrs else 1
                self.players[addr] = player_id
                self.player_addrs[player_id] = addr
                self.last_heartbeat[player_id] = time.time()
                
                print(f"Player {player_id + 1} connected from {addr}")
                
                self.send_to_player(player_id, {
                    'type': 'assignment',
                    'player_id': player_id,
                    'symbol': self.player_symbols[player_id]
                })
                return
        
        # Get player ID from address
        if addr not in self.players:
            return
            
        player_id = self.players[addr]
        self.last_heartbeat[player_id] = time.time()
        
        if msg_type == 'heartbeat':
            self.send_to_player(player_id, {'type': 'heartbeat_ack'})
            return
            
        if msg_type == 'ready':
            with self.lock:
                self.ready_players.add(player_id)
                print(f"Player {player_id + 1} is ready")
                
                if len(self.ready_players) == 2:
                    self.start_game()
                else:
                    self.send_to_player(player_id, {
                        'type': 'waiting',
                        'message': 'Waiting for opponent to be ready...'
                    })
                    
        elif msg_type == 'move':
            with self.lock:
                if not self.game_active:
                    return
                    
                if player_id != self.current_player:
                    self.send_to_player(player_id, {
                        'type': 'error',
                        'message': "It's not your turn!"
                    })
                    return
                    
                row = message.get('row')
                col = message.get('col')
                
                if not self.is_valid_move(row, col):
                    self.send_to_player(player_id, {
                        'type': 'error',
                        'message': 'Invalid move! Cell is already occupied or out of bounds.'
                    })
                    return
                
                # Make the move
                self.make_move(row, col, self.player_symbols[player_id])
                
                # Check for winner
                result = self.check_winner()
                
                if result:
                    self.game_active = False
                    if result == 'Draw':
                        self.broadcast({
                            'type': 'game_over',
                            'board': self.board,
                            'result': 'draw',
                            'message': "It's a Draw!"
                        })
                    else:
                        # Find which player has the winning symbol
                        winner_id = 0 if self.player_symbols[0] == result else 1
                        self.broadcast({
                            'type': 'game_over',
                            'board': self.board,
                            'result': 'win',
                            'winner': winner_id,
                            'message': f"Player {winner_id + 1} Wins!"
                        })
                else:
                    # Switch turns
                    self.current_player = 1 - self.current_player
                    
                    self.broadcast({
                        'type': 'board_update',
                        'board': self.board,
                        'current_player': self.current_player
                    })
                    
        elif msg_type == 'play_again':
            with self.lock:
                response = message.get('response')
                self.play_again_responses[player_id] = response
                print(f"Player {player_id + 1} play again: {response}")
                
                if len(self.play_again_responses) == 2:
                    if all(self.play_again_responses.values()):
                        # Both want to play again
                        self.reset_board()
                        self.swap_symbols()  # Swap X and O for fairness
                        self.game_count += 1
                        self.ready_players.clear()
                        
                        # Notify players of their new symbols
                        for pid in self.player_addrs:
                            self.send_to_player(pid, {
                                'type': 'symbol_swap',
                                'symbol': self.player_symbols[pid],
                                'message': f'Sides swapped! You are now {self.player_symbols[pid]}'
                            })
                        
                        self.start_game()
                    elif not any(self.play_again_responses.values()):
                        # Both don't want to play
                        self.broadcast({
                            'type': 'game_ended',
                            'message': 'Both players chose not to continue. Goodbye!'
                        })
                        self.cleanup_game()
                    else:
                        # One wants to play, one doesn't
                        for pid, wants_play in self.play_again_responses.items():
                            if wants_play:
                                self.send_to_player(pid, {
                                    'type': 'game_ended',
                                    'message': 'Your opponent did not wish to play again. Goodbye!'
                                })
                            else:
                                self.send_to_player(pid, {
                                    'type': 'game_ended',
                                    'message': 'You chose not to continue. Goodbye!'
                                })
                        self.cleanup_game()
                else:
                    self.send_to_player(player_id, {
                        'type': 'waiting',
                        'message': 'Waiting for opponent to respond...'
                    })
                    
        elif msg_type == 'disconnect':
            with self.lock:
                self.remove_player(player_id)
                    
    def cleanup_game(self):
        """Clean up game state after game ends"""
        self.players.clear()
        self.player_addrs.clear()
        self.ready_players.clear()
        self.play_again_responses.clear()
        self.last_heartbeat.clear()
        self.reset_board()
        self.game_active = False
        print("Game cleaned up, waiting for new players...")
        
    def remove_player(self, player_id):
        """Remove a player from the game"""
        if player_id in self.player_addrs:
            addr = self.player_addrs[player_id]
            del self.player_addrs[player_id]
            if addr in self.players:
                del self.players[addr]
            if player_id in self.last_heartbeat:
                del self.last_heartbeat[player_id]
            self.ready_players.discard(player_id)
            
            print(f"Player {player_id + 1} disconnected")
            
            # Notify other player
            other_player = 1 - player_id
            if other_player in self.player_addrs:
                self.send_to_player(other_player, {
                    'type': 'opponent_disconnected'
                })
            
            self.game_active = False
                    
    def start_game(self):
        """Start the game"""
        self.reset_board()
        # Player with X always goes first
        self.current_player = 0 if self.player_symbols[0] == 'X' else 1
        
        first_player = self.current_player + 1
        print(f"Game {self.game_count + 1} starting! Player {first_player} (X) goes first.")
        
        self.broadcast({
            'type': 'game_start',
            'board': self.board,
            'current_player': self.current_player,
            'message': f'Game is starting! Player {first_player} (X) goes first.'
        })
        
    def check_heartbeats(self):
        """Check for disconnected players based on heartbeat timeout"""
        current_time = time.time()
        timeout = 10  # 10 seconds timeout
        
        with self.lock:
            disconnected = []
            for player_id, last_time in list(self.last_heartbeat.items()):
                if current_time - last_time > timeout:
                    disconnected.append(player_id)
                    
            for player_id in disconnected:
                print(f"Player {player_id + 1} timed out")
                self.remove_player(player_id)
        
    def run(self):
        """Run the server"""
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.bind((HOST, PORT))
        self.socket.settimeout(1.0)  # 1 second timeout for heartbeat checking
        
        print(f"UDP Tic-Tac-Toe Server running on {HOST}:{PORT}")
        print("Waiting for players to connect...")
        
        try:
            while True:
                try:
                    data, addr = self.socket.recvfrom(4096)
                    message = json.loads(data.decode())
                    self.process_message(addr, message)
                except socket.timeout:
                    self.check_heartbeats()
                except json.JSONDecodeError:
                    print("Received invalid JSON")
                except Exception as e:
                    print(f"Error: {e}")
                    
        except KeyboardInterrupt:
            print("\nServer shutting down...")
        finally:
            self.socket.close()


if __name__ == '__main__':
    server = TicTacToeUDPServer()
    server.run()
