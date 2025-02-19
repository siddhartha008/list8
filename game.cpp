#include "game.h"
#include <algorithm>
#include <emscripten.h>

Game::Game() : remainingTries(MAX_TRIES), gameOver(false), gameResult("No Winners Yet") {}

bool Game::setInitialList(const std::vector<int> &numbers)
{
    if (numbers.size() != INITIAL_LIST_SIZE)
    {
        emscripten_log(EM_LOG_CONSOLE, "Error: Initial list must contain exactly %d elements.", INITIAL_LIST_SIZE);
        return false;
    }

    std::set<int> uniqueNumbers(numbers.begin(), numbers.end());
    if (uniqueNumbers.size() != INITIAL_LIST_SIZE)
    {
        emscripten_log(EM_LOG_CONSOLE, "Error: Initial list contains duplicate elements.");
        return false;
    }

    for (int num : numbers)
    {
        if (!isValidNumber(num))
        {
            emscripten_log(EM_LOG_CONSOLE, "Error: Number %d is out of range.", num);
            return false;
        }
    }

    numberList = numbers;
    emscripten_log(EM_LOG_CONSOLE, "Initial list set successfully.");
    return true;
}

bool Game::addNumber(int number)
{
    if (gameOver)
    {
        emscripten_log(EM_LOG_CONSOLE, "Error: Game is over. Cannot add numbers.");
        return false;
    }

    if (!isValidNumber(number))
    {
        emscripten_log(EM_LOG_CONSOLE, "Error: Invalid or duplicate number %d.", number);
        return false;
    }

    if (!isUniqueNumber(number))
    {
        emscripten_log(EM_LOG_CONSOLE, "Error: Number %d is already in the list.", number);
        return false;
    }

    numberList.push_back(number);
    emscripten_log(EM_LOG_CONSOLE, "Number %d added to the list.", number);

    checkGameOver();
    return true;
}

int Game::processGuess(int guess1)
{
    if (gameOver)
    {
        emscripten_log(EM_LOG_CONSOLE, "Error: Game is over. Cannot process guesses.");
        return -1;
    }

    if (!isValidNumber(guess1))
    {
        emscripten_log(EM_LOG_CONSOLE, "Error: Guess: %d are out of range.", guess1);
        return -1;
    }

    int correctCount = 0;

    auto it = std::find(numberList.begin(), numberList.end(), guess1);
    if (it != numberList.end())
    {
        correctCount++;
        numberList.erase(it);
        emscripten_log(EM_LOG_CONSOLE, "Guess %d is correct and removed from the list.", guess1);
    }
    else
    {
        remainingTries--;
    }
    checkGameOver();
    return correctCount;
}

void Game::checkGameOver()
{
    if (numberList.empty())
    {
        gameOver = true;
        gameResult = "Guesser Wins";
        emscripten_log(EM_LOG_CONSOLE, "Game Over: Guesser Wins!");
    }
    else if (remainingTries <= 0 || numberList.size() >= MAX_LIST_SIZE)
    {
        gameOver = true;
        gameResult = "Generator Wins";
        emscripten_log(EM_LOG_CONSOLE, "Game Over: Generator Wins!");
    }
}

bool Game::isValidNumber(int number) const
{
    return number >= MIN_NUMBER && number <= MAX_NUMBER;
}

bool Game::isUniqueNumber(int number) const
{
    return std::find(numberList.begin(), numberList.end(), number) == numberList.end();
}

std::string Game::getGameResult() const
{
    return gameResult;
}