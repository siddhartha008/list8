const WebSocket = require("ws");
const wss = new WebSocket.Server({ port: 8080 });
const { v4: uuidv4 } = require("uuid");

const Module = require("./game.js");


// Store rooms and their players
const rooms = new Map();

wss.on("connection", (ws) => {
    console.log("New client connected");

    // Assign a unique playerId to the player using uuid
    const playerId = uuidv4();
    ws.playerId = playerId;

    ws.on("message", (message) => {
        let data;
        try {
            data = JSON.parse(message);
        } catch (error) {
            console.error("Invalid JSON received:", message);
            return;
        }

        console.log(`Received: ${JSON.stringify(data)}`);
        handleMessage(ws, data);
    });

    ws.on("close", () => {
        console.log("Client disconnected");

        // Handle player disconnection
        rooms.forEach((room, roomId) => {
            if (room.players.includes(ws)) {
                room.players = room.players.filter((player) => player !== ws);
                // Reset roles if a player disconnects
                if (room.roles) {
                    delete room.roles[ws.playerId]; // Remove the disconnected player's role
                }

                if (room.players.length === 1) {
                    room.players.forEach((player) => {
                        player.send(
                            JSON.stringify({ type: "playerDisconnected", message: "Your opponent has disconnected." })
                        );
                        player.send(JSON.stringify({ type: "resetRoles" })); // Reset role selection
                    });
                }
            }
        });
    });
});

function handleMessage(ws, data) {
    console.log(`Handling message of type: ${data.type} for room: ${data.roomId}`);

    if (data.type === "createRoom") {
        const roomId = data.roomId; // Use the roomId from the message
        if (!roomId) {
            console.error("Room ID is missing in createRoom message");
            return;
        }

        // Initialize the room with an empty roles object
        rooms.set(roomId, {
            players: [ws],
            roomId,
            game: null,
            gameStarted: false,
            roles: {},
            listSet: false // Add this flag
        });

        console.log(`Room created with ID: ${roomId}`);
        ws.send(JSON.stringify({ type: "roomCreated", roomId }));

        // Create a game instance in C++
        const result = Module.ccall("createGameForRoom", "string", ["string"], [roomId]);
        console.log(`C++ Game creation result: ${result}`);

        // Initialize the game object in the room
        rooms.get(roomId).game = {
            getCurrentList: () => {
                const listPtr = Module.ccall("getCurrentListForRoom", "number", ["string"], [roomId]);
                const size = Module.ccall("getListSizeForRoom", "number", ["string"], [roomId]);
                const list = new Int32Array(Module.HEAP32.buffer, listPtr, size);
                Module._free(listPtr);
                return Array.from(list);
            },
            getTriesLeft: () => {
                return Module.ccall("getTriesLeftForRoom", "number", ["string"], [roomId]);
            },
            getGameResult: () => {
                return Module.ccall("getGameResultForRoom", "string", ["string"], [roomId]);
            }
        };
    }

    if (data.type === "joinRoom") {
        const roomId = data.roomId;
        const room = rooms.get(roomId);

        if (room && room.players.length < 2) {
            room.players.push(ws);
            ws.send(JSON.stringify({ type: "roomJoined", roomId }));
            console.log(`Player joined room: ${roomId}. Current players: ${room.players.length}`);

            // Add player to the room in C++
            const result = Module.ccall("addPlayerToRoom", "string", ["string", "string"], [roomId, ws.playerId]);
            console.log(`C++ Player addition result: ${result}`);

            if (room.players.length === 2) {
                room.players.forEach((player) => {
                    player.send(JSON.stringify({ type: "roomReady", roomId }));
                });
                console.log(`Room ${roomId} is ready with 2 players.`);
            }
        } else {
            ws.send(JSON.stringify({ type: "roomFull", roomId }));
            console.error(`Room ${roomId} is full or does not exist.`);
        }
    }

    if (data.type === "setInitialList") {
        const room = rooms.get(data.roomId);
        if (room) {
            console.log(`Relaying setinit message for room ${data.roomId}`);

            // Set initial list in C++
            const numbers = data.numbers;
            const numbersPtr = Module._malloc(numbers.length * 4);
            Module.HEAP32.set(numbers, numbersPtr / 4);
            const result = Module.ccall("setInitialListForRoom", "number", ["string", "number", "number"], [data.roomId, numbersPtr, numbers.length]);
            Module._free(numbersPtr);
            console.log(`C++ Set initial list result: ${result}`);

            room.listSet = true;

            // Send the initial list back to the generator
            room.players.forEach((player) => {
                player.send(JSON.stringify({
                    type: "listSet",
                    numbers: data.numbers,
                }));
            });
        } else {
            console.error(`Room ${data.roomId} not found while processing setinit.`);
        }
    }

    if (data.type === "guess") {
        const room = rooms.get(data.roomId);
        if (room) {
            if (!room.listSet) {
                ws.send(JSON.stringify({
                    type: "error",
                    message: "The list has not been set yet. Please wait for the generator to set the list."
                }));
                return;
            }

            console.log(`Relaying guess message for room ${data.roomId}`);

            // Process guess in C++
            const result = Module.ccall("processGuessForRoom", "number", ["string", "number"], [data.roomId, data.guess]);
            console.log(`C++ Guess processing result: ${result}`);

            // Get the current list and remaining tries
            const currentList = room.game.getCurrentList();
            const remainingTries = room.game.getTriesLeft();

            // Check winning conditions
            const gameResult = room.game.getGameResult();

            const guessResult = result === 1 ? "correct" : "incorrect";

            room.players.forEach((player) => {
                player.send(JSON.stringify({
                    type: "guessResult",
                    guess: data.guess,
                    result: guessResult,
                    remainingTries: remainingTries,
                    currentList: currentList,
                    gameResult: gameResult // Include game result in the message
                }));
            });

            // If the guess is incorrect, allow the generator to add an element
            if (guessResult === "incorrect") {
                room.players.forEach((player) => {
                    if (room.roles[player.playerId] === "generator") {
                        player.send(JSON.stringify({
                            type: "allowAddElement",
                            gameResult: gameResult,
                        }));
                    }
                });
            }

            // If the game is over, reset the room
            // if (gameResult) {
            //     rooms.delete(data.roomId);
            // }
        } else {
            console.error(`Room ${data.roomId} not found while processing guess.`);
        }
    }


    if (data.type === 'addElement') {
        const room = rooms.get(data.roomId);
        if (room) {
            // const currentList = room.game.getCurrentList();

            // // Check if the number is currently in the list
            // if (currentList.includes(data.number)) {
            //     ws.send(JSON.stringify({
            //         type: "error",
            //         message: "NUMBER ALREADY IN LIST"
            //     }));
            //     return; // Stop further execution if the number is already in the list
            // }

            // Add the element to the list in C++
            const number = data.number;
            const result = Module.ccall("addNumberForRoom", "number", ["string", "number"], [data.roomId, number]);

            if (result === 1) { // Assuming the C++ function returns 1 on success
                // Send the updated list to both players
                room.players.forEach((player) => {
                    player.send(JSON.stringify({
                        type: "listUpdated",
                        currentList: room.game.getCurrentList(),
                    }));
                });
            } else {
                console.error("Failed to add number to the list.");
            }
        } else {
            console.error(`Room ${data.roomId} not found while processing addElement.`);
        }
    }

    if (data.type === "selectRole") {
        const room = rooms.get(data.roomId);
        if (room) {
            // Track the selected role for the current player
            room.roles = room.roles || {};
            room.roles[ws.playerId] = data.role; // Associate the role with the playerId

            // Notify both players of the selected roles
            room.players.forEach((player) => {
                player.send(JSON.stringify({ type: "roleAssigned", role: data.role, playerId: ws.playerId }));
            });

            // Check if both roles are selected
            const roles = Object.values(room.roles);
            if (roles.includes("generator") && roles.includes("guesser")) {
                room.players.forEach((player) => {
                    player.send(JSON.stringify({ type: "gameReady" }));
                });
            }
        }
    }

    // Handle game updates
    if (data.type === "gameUpdate") {
        const room = rooms.get(data.roomId);
        if (room) {
            room.players.forEach((player) => {
                player.send(JSON.stringify(data));
            });
        }
    }

    if (data.type === "reconnect") {
        const room = rooms.get(data.roomId);
        if (room) {
            // Add the player back to the room
            room.players.push(ws);

            // Notify the player of their role (if roles exist)
            if (room.roles && room.roles[ws.playerId]) {
                const playerRole = room.roles[ws.playerId];
                ws.send(JSON.stringify({ type: "roleAssigned", role: playerRole, playerId: ws.playerId }));
            }

            // Send the current game state to the reconnected player
            // const gameState = {
            //     type: "gameUpdate",
            //     currentList: room.game.getCurrentList(),
            //     remainingTries: room.game.getTriesLeft(),
            //     gameResult: room.game.getGameResult()
            // };
            // ws.send(JSON.stringify(gameState));
        }
    }

    if (data.type === "requestRematch") {
        const room = rooms.get(data.roomId);
        if (room) {
            // Notify the other player of the rematch request
            room.players.forEach((player) => {
                if (player !== ws) {
                    player.send(JSON.stringify({ type: "rematchRequested" }));
                }
            });
        }
    }

    if (data.type === "acceptRematch") {
        const room = rooms.get(data.roomId);
        if (room) {
            // Check if both players have accepted the rematch
           
                // Reset the game state in C++
                const result = Module.ccall("resetGameForRoom", "number", ["string"], [data.roomId]);
                if (result === 1) {
                    room.players.forEach((player) => {
                        player.send(JSON.stringify({ type: "rematchAccepted" }));
                    });
                }
        }
    }

    if (data.type === "rejectRematch") {
        const room = rooms.get(data.roomId);
        if (room) {
            // Notify the requesting player that the rematch was rejected
            room.players.forEach((player) => {
                if (player !== ws) {
                    player.send(JSON.stringify({ type: "rematchRejected" }));
                }
            });
        }
    }

    if (data.type === "quit") {
        const room = rooms.get(data.roomId);
        if (room) {
            // Remove the player from the room
            room.players = room.players.filter((player) => player !== ws);

            // Notify the remaining player that their opponent has quit
            if (room.players.length === 1) {
                room.players.forEach((player) => {
                    player.send(JSON.stringify({ type: "playerQuit" }));
                });
            }

            // Reset roles if a player quits
            if (room.roles) {
                delete room.roles[ws.playerId]; // Remove the disconnected player's role
            }
        }
    }

}



console.log("WebSocket server is running on ws://localhost:8080");