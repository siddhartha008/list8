# LIST8 - A Multiplayer Number Guessing Game

LIST8 is a fun and interactive multiplayer game where two players take on the roles of Generator and Guesser. The Generator creates a list of unique numbers, and the Guesser tries to guess the numbers within a limited number of tries. The game is built using HTML, CSS, JavaScript, WebSocket, and C++ for backend logic.

## Table of Contents
- Features
- How to Play
- Installation
- Running the Game
- Technologies Used
- Project Structure
- Contributing
- License

## Features
- **Multiplayer Gameplay:** Two players can play together in real-time.
- **Role Assignment:** Players can choose to be the Generator or the Guesser.
- **Dynamic Gameplay:**
  - The Generator creates a list of 4 unique numbers (0-9).
  - The Guesser tries to guess the numbers within a limited number of tries.
  - The Generator can add numbers to the list if the Guesser guesses incorrectly.
- **Real-Time Updates:** Players see updates in real-time using WebSocket communication.
- **Rematch and Role Switching:** Players can rematch or switch roles after a game ends.
- **Cross-Browser Support:** Works across normal and incognito browsers.

## How to Play
### Create or Join a Room:
1. One player creates a room and shares the room URL with the other player.
2. The second player joins the room using the shared URL.

### Select Roles:
- One player becomes the Generator, and the other becomes the Guesser.

### Gameplay:
1. The Generator creates a list of 4 unique numbers (0-9).
2. The Guesser tries to guess the numbers one by one.
3. If the Guesser guesses correctly, the number is removed from the list.
4. If the Guesser guesses incorrectly, the Generator can add a new number to the list.
5. The game continues until the Guesser guesses all numbers or runs out of tries.

### Winning the Game:
- The Guesser wins if they guess all numbers correctly.
- The Generator wins if the Guesser runs out of tries or the list grows too large.

### Rematch or Switch Roles:
- Players can choose to rematch or switch roles after the game ends.

## Installation
### Prerequisites
- **Node.js:** Install Node.js from [nodejs.org](https://nodejs.org/).
- **Emscripten:** Install Emscripten to compile C++ code to WebAssembly.(https://emscripten.org/).

### Steps
1. Clone the repository:
   ```bash
   git clone https://github.com/siddhartha008/list8/
   cd list8
   ```
2. Install dependencies:
   ```bash
   npm install
   ```
3. Compile the C++ code:
   ```bash
   emcc game.cpp -o game.js -s EXPORTED_FUNCTIONS='["_createGameForRoom", "_addPlayerToRoom", "_setInitialListForRoom", "_getCurrentListForRoom", "_addNumberForRoom", "_processGuessForRoom", "_getTriesLeftForRoom", "_getGameResultForRoom", "_resetGameForRoom"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'
   ```
4. Start the WebSocket server:
   ```bash
   node server.js
   ```
5. Open the game in your browser:
   Navigate to `http://localhost:3000` (or the port you configured).

## Running the Game
### Create a Room:
- Open the game in your browser and click **Create Room**.
- Share the room URL with the second player.

### Join a Room:
- Open the shared URL in another browser (or incognito mode).
- Select your role (Generator or Guesser).

### Play the Game:
- Follow the on-screen instructions to play the game.

## Technologies Used
### Frontend:
- HTML, CSS, JavaScript
- WebSocket for real-time communication

### Backend:
- Node.js with `ws` library for WebSocket server
- C++ for game logic (compiled to WebAssembly using Emscripten)

### Tools:
- Emscripten for compiling C++ to WebAssembly
- UUID for generating unique player IDs

## License
This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgments
- Inspired by classic number-guessing games.
- Built with ❤️ by Sid. 

Enjoy playing LIST8! If you have any questions or feedback, feel free to open an issue or contact me.
