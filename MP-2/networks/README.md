# XOXO - Multiplayer Tic-Tac-Toe Game

A networked multiplayer Tic-Tac-Toe game implemented with both TCP and UDP protocols, featuring a graphical user interface.

## Features

- **TCP and UDP implementations** - Both protocols for learning networking concepts
- **GUI Interface** - Clean tkinter-based graphical interface
- **Two-player support** - Server manages game state between two clients
- **Turn-based gameplay** - Server ensures proper turn alternation
- **Invalid move rejection** - Server validates all moves
- **Play again option** - Players can choose to play again after a game ends
- **Connection handling** - Proper handling of disconnections and timeouts

## Requirements

- Python 3.x
- tkinter (usually comes with Python)

## Files

- `tcp_server.py` - TCP Tic-Tac-Toe server
- `tcp_client.py` - TCP Tic-Tac-Toe client with GUI
- `udp_server.py` - UDP Tic-Tac-Toe server
- `udp_client.py` - UDP Tic-Tac-Toe client with GUI

## How to Run

### TCP Version

1. Start the server:
```bash
python3 tcp_server.py
```

2. In separate terminals, start two clients:
```bash
python3 tcp_client.py
```
```bash
python3 tcp_client.py
```

### UDP Version

1. Start the server:
```bash
python3 udp_server.py
```

2. In separate terminals, start two clients:
```bash
python3 udp_client.py
```
```bash
python3 udp_client.py
```

## How to Play

1. **Connect** - Click the "Connect" button in each client window
2. **Ready** - Click "Ready" when you're ready to play
3. **Play** - Click on empty cells to place your symbol (X or O)
4. **Win/Draw** - Get three in a row, column, or diagonal to win!
5. **Play Again** - After game ends, choose whether to play again

## Game Rules

- Player 1 uses 'X', Player 2 uses 'O'
- Player 1 (X) always goes first
- Players take turns clicking on empty cells
- First player to get 3 symbols in a row/column/diagonal wins
- If all cells are filled with no winner, it's a draw

## Network Details

### TCP (Port 65432)
- Reliable, connection-oriented protocol
- Server uses threads to handle multiple clients
- Messages are JSON-encoded with newline delimiters

### UDP (Port 65433)
- Connectionless protocol
- Uses heartbeat mechanism to detect disconnections
- 10-second timeout for inactive players