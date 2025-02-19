#include <emscripten.h>
#include "game.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <set>

// Map to store rooms and their game instances
std::unordered_map<std::string, std::unique_ptr<Game>> games;

// Map to store players in each room
std::unordered_map<std::string, std::set<std::string>> roomPlayers;

extern "C"
{
    // Create a game instance for a room
    EMSCRIPTEN_KEEPALIVE
    const char *createGameForRoom(const char *roomId)
    {
        std::string room(roomId);

        if (games.find(room) == games.end())
        {
            games[room] = std::make_unique<Game>();
            emscripten_log(EM_LOG_CONSOLE, "cpp Game created for Room: %s", roomId);
            return "Game created successfully!";
        }

        emscripten_log(EM_LOG_ERROR, "cpp Room already exists: %s", roomId);
        return "Room already exists.";
    }

    // Add a player to a room
    EMSCRIPTEN_KEEPALIVE
    const char *addPlayerToRoom(const char *roomId, const char *playerId)
    {
        std::string room(roomId);
        std::string player(playerId);

        if (roomPlayers.find(room) == roomPlayers.end())
        {
            roomPlayers[room] = std::set<std::string>();
        }

        auto &players = roomPlayers[room];
        if (players.size() >= 2)
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s is full", roomId);
            return "Room is full.";
        }

        players.insert(player);
        emscripten_log(EM_LOG_CONSOLE, "cpp Player %s added to Room %s", playerId, roomId);

        if (players.size() == 2)
        {
            emscripten_log(EM_LOG_CONSOLE, "cpp Room %s is ready with 2 players", roomId);
            return "Room is ready.";
        }

        return "Waiting for another player.";
    }

    // Set the initial list for a room
    EMSCRIPTEN_KEEPALIVE
    int setInitialListForRoom(const char *roomId, int *numbers, int size)
    {
        std::string room(roomId);

        if (games.find(room) == games.end())
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s does not exist", roomId);
            return 0;
        }

        if (!numbers || size != Game::INITIAL_LIST_SIZE)
        {
            emscripten_log(EM_LOG_ERROR, "cpp Invalid numbers or size for Room %s", roomId);
            return 0;
        }

        std::vector<int> numList(numbers, numbers + size);
        if (games[room]->setInitialList(numList))
        {
            emscripten_log(EM_LOG_CONSOLE, "cpp Initial list set for Room %s", roomId);
            return 1;
        }
        else
        {
            emscripten_log(EM_LOG_ERROR, "cpp Failed to set initial list for Room %s", roomId);
            return 0;
        }
    }

    // Get the current list for a room
    EMSCRIPTEN_KEEPALIVE
    int *getCurrentListForRoom(const char *roomId)
    {
        std::string room(roomId);

        if (games.find(room) == games.end())
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s does not exist", roomId);
            return nullptr;
        }

        std::vector<int> gameList = games[room]->getCurrentList();
        if (gameList.empty())
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s has an empty list", roomId);
            return nullptr;
        }

        int *array = (int *)malloc(gameList.size() * sizeof(int));
        if (!array)
        {
            emscripten_log(EM_LOG_ERROR, "cpp Memory allocation failed");
            return nullptr;
        }

        std::copy(gameList.begin(), gameList.end(), array);
        emscripten_log(EM_LOG_CONSOLE, "cpp List retrieved for Room %s", roomId);
        return array;
    }

    // Add a number to the list in a room
    EMSCRIPTEN_KEEPALIVE
    int addNumberForRoom(const char *roomId, int number)
    {
        std::string room(roomId);

        if (games.find(room) == games.end())
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s does not exist", roomId);
            return 0;
        }
        emscripten_log(EM_LOG_CONSOLE, "cpp %d added at room %s", number, roomId);

        return games[room]->addNumber(number) ? 1 : 0;
    }

    // Process a guess for a room
    EMSCRIPTEN_KEEPALIVE
    int processGuessForRoom(const char *roomId, int guess)
    {
        std::string room(roomId);

        if (games.find(room) == games.end())
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s does not exist", roomId);
            return -1;
        }

        return games[room]->processGuess(guess);
    }

    // Get the number of tries left for a room
    EMSCRIPTEN_KEEPALIVE
    int getTriesLeftForRoom(const char *roomId)
    {
        std::string room(roomId);

        if (games.find(room) == games.end())
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s does not exist", roomId);
            return 0;
        }

        return games[room]->getTriesLeft();
    }

    EMSCRIPTEN_KEEPALIVE
    int resetGameForRoom(const char *roomId)
    {
        std::string room(roomId);

        if (games.find(room) == games.end())
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s does not exist", roomId);
            return 0;
        }

        games[room]->resetGame();
        emscripten_log(EM_LOG_CONSOLE, "cpp Game reset for Room %s", roomId);
        return 1;
    }

    // Get the game result for a room
    EMSCRIPTEN_KEEPALIVE
    const char *getGameResultForRoom(const char *roomId)
    {
        std::string room(roomId);

        if (games.find(room) == games.end())
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s does not exist", roomId);
            return "Game not initialized.";
        }

        static std::string result;
        result = games[room]->getGameResult();
        return result.c_str();
    }

    // Get the size of the list for a room
    EMSCRIPTEN_KEEPALIVE
    int getListSizeForRoom(const char *roomId)
    {
        std::string room(roomId);

        if (games.find(room) == games.end())
        {
            emscripten_log(EM_LOG_ERROR, "cpp Room %s does not exist", roomId);
            return 0;
        }

        return games[room]->getCurrentList().size();
    }
}