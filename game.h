#ifndef GAME_H
#define GAME_H

#include <vector>
#include <set>
#include <string>
#include <stdexcept>

class Game
{
public:
    static const int MAX_TRIES = 10;
    static const int INITIAL_LIST_SIZE = 4;
    static const int MAX_LIST_SIZE = 8;
    static const int MIN_NUMBER = 0;
    static const int MAX_NUMBER = 9;

    Game();
    ~Game() = default;

    // Generator functions
    bool setInitialList(const std::vector<int> &numbers);
    bool addNumber(int number);

    // Guesser functions
    // int processGuess(int guess1, int guess2);
    int processGuess(int guess1);

    // Game state
    int getTriesLeft() const { return remainingTries; }
    bool isGameOver() const { return gameOver; }
    std::vector<int> getCurrentList() const { return numberList; }
    std::string getGameResult() const;

    void resetGame()
    {
        numberList.clear();
        remainingTries = MAX_TRIES;
        gameOver = false;
        gameResult = "No Winners Yet";
    }

private:
    std::vector<int> numberList;
    int remainingTries;
    bool gameOver;
    std::string gameResult; // Stores the result of the game (e.g., "Guesser Wins", "Generator Wins")

    void checkGameOver();
    bool isValidNumber(int number) const;
    bool isUniqueNumber(int number) const;
};

#endif