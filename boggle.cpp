#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include <cctype>
#include "lexicon.h"

using namespace std;

const string DICTIONARY_FILE = "EnglishWords.txt";
const string END_TURN = "???";
constexpr int MIN_WORD_LENGTH = 4;
constexpr int MAX_BOARD_SIZE = 20;

class BoggleGame {
private:
    vector<vector<char>> board;
    int boardSize;
    Lexicon dictionary;
    set<string> player1Words;
    set<string> player2Words;
    int player1Score;
    int player2Score;

    bool isValidWord(const string& word, const set<string>& playerWords) const {
        string upperWord = toUpper(word);
        return word.length() >= MIN_WORD_LENGTH &&
               dictionary.contains(upperWord) &&
               findWordOnBoard(upperWord) &&
               !containsIgnoreCase(playerWords, upperWord);
    }

    bool findWordOnBoard(const string& word) const {
        if (word.empty() || word.length() > static_cast<size_t>(boardSize * boardSize)) {
            return false;
        }
        vector<vector<bool>> visited(boardSize, vector<bool>(boardSize, false));
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                if (findWordFromCell(word, 0, i, j, visited)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool findWordFromCell(const string& word, size_t index, int row, int col, vector<vector<bool>>& visited) const {
        if (index == word.length()) return true;
        if (row < 0 || row >= boardSize || col < 0 || col >= boardSize ||
            visited[row][col] || board[row][col] != word[index]) {
            return false;
        }

        visited[row][col] = true;
        bool found = false;
        for (int i = -1; i <= 1 && !found; i++) {
            for (int j = -1; j <= 1 && !found; j++) {
                if (i != 0 || j != 0) {
                    found = findWordFromCell(word, index + 1, row + i, col + j, visited);
                }
            }
        }
        visited[row][col] = false;
        return found;
    }

    static void playerTurn(int playerNum, set<string>& playerWords, int& playerScore, const BoggleGame& game) {
        cout << "Player " << playerNum << " Score: " << playerScore << endl;
        string word;
        while (true) {
            if (!(cin >> word)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please try again." << endl;
                continue;
            }
            if (word == END_TURN) break;

            if (game.isValidWord(word, playerWords)) {
                cout << "Correct." << endl;
                playerWords.insert(toUpper(word));
                playerScore += static_cast<int>(word.length()) - MIN_WORD_LENGTH + 1;
            } else if (word.length() < MIN_WORD_LENGTH) {
                cout << word << " is too short." << endl;
            } else if (!game.dictionary.contains(toUpper(word))) {
                cout << word << " is not a word." << endl;
            } else if (!game.findWordOnBoard(toUpper(word))) {
                cout << word << " is not on board." << endl;
            } else {
                cout << word << " is already found." << endl;
            }
            cout << "Player " << playerNum << " Score: " << playerScore << endl;
        }
    }

    void findAllWordsHelper(int row, int col, string& currentWord, vector<vector<bool>>& visited, set<string>& allWords) const {
        if (row < 0 || row >= boardSize || col < 0 || col >= boardSize || visited[row][col]) {
            return;
        }

        currentWord += board[row][col];
        visited[row][col] = true;

        if (currentWord.length() >= MIN_WORD_LENGTH && dictionary.contains(currentWord)) {
            allWords.insert(currentWord);
        }

        if (dictionary.containsPrefix(currentWord)) {
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    if (i != 0 || j != 0) {
                        findAllWordsHelper(row + i, col + j, currentWord, visited, allWords);
                    }
                }
            }
        }

        currentWord.pop_back();
        visited[row][col] = false;
    }

    void findAllWords(set<string>& allWords) const {
        string currentWord;
        vector<vector<bool>> visited(boardSize, vector<bool>(boardSize, false));
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                findAllWordsHelper(i, j, currentWord, visited, allWords);
            }
        }
    }

    static string toUpper(const string& s) {
        string result = s;
        transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    static bool containsIgnoreCase(const set<string>& words, const string& word) {
        return any_of(words.begin(), words.end(), [&word](const string& w) {
            return toUpper(w) == word;
        });
    }

public:
    BoggleGame() : dictionary(DICTIONARY_FILE), boardSize(0), player1Score(0), player2Score(0) {}

    void readBoard() {
        if (!(cin >> boardSize) || boardSize <= 0 || boardSize > MAX_BOARD_SIZE) {
            throw runtime_error("Invalid board size. Must be between 1 and " + to_string(MAX_BOARD_SIZE));
        }
        board.resize(boardSize, vector<char>(boardSize));
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                char c;
                if (!(cin >> c) || !isalpha(c)) {
                    throw runtime_error("Invalid input. Board must contain only alphabetic characters.");
                }
                board[i][j] = static_cast<char>(toupper(static_cast<unsigned char>(c)));
            }
        }
    }

    void play() {
        playerTurn(1, player1Words, player1Score, *this);
        playerTurn(2, player2Words, player2Score, *this);

        cout << "Player 1 Score: " << player1Score << endl;
        cout << "Player 2 Score: " << player2Score << endl;
        if (player1Score > player2Score) {
            cout << "Player 1 wins!" << endl;
        } else if (player2Score > player1Score) {
            cout << "Player 2 wins!" << endl;
        } else {
            cout << "It's a tie!" << endl;
        }

        set<string> allWords;
        findAllWords(allWords);
        cout << "All Possible Words: ";
        for (const auto& word : allWords) {
            cout << word << " ";
        }
        cout << endl;
    }
};

int main() {
    try {
        BoggleGame game;
        game.readBoard();
        game.play();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}