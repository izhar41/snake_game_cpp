#include <conio.h>
#include <windows.h>

#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

// Constants
constexpr int WIDTH = 25;
constexpr int HEIGHT = 20;
constexpr int DELAY_MS = 150;  // higher = slower snake, lower = faster
constexpr int START_LENGTH = 3;

constexpr int KEY_ARROW_PREFIX = 224;
constexpr int KEY_UP = 72;
constexpr int KEY_DOWN = 80;
constexpr int KEY_LEFT = 75;
constexpr int KEY_RIGHT = 77;

// Types
enum class Direction { Up, Down, Left, Right };

struct Position {
    int x;
    int y;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Console helpers

void clearScreen() {
    system("cls");
}

// Moves the cursor to the top-left instead of clearing: avoids flickering.
void resetCursor() {
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
}

void hideCursor() {
    CONSOLE_CURSOR_INFO info{1, FALSE};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void flushKeyboard() {
    while (_kbhit()) {
        _getch();
    }
}

// Game
class SnakeGame {
public:
    SnakeGame() : generator_(random_device{}()) {}

    void reset() {
        // The LAST element is the head: tail -> body -> head
        snake_.clear();
        for (int i = 0; i < START_LENGTH; i++) {
            snake_.push_back({3 + i, HEIGHT / 2});
        }

        direction_ = Direction::Right;
        gameOver_ = false;
        quit_ = false;
        spawnFood();
    }

    void handleInput() {
        if (!_kbhit()) {
            return;
        }

        int key = _getch();

        if (key == 'q' || key == 'Q') {
            gameOver_ = true;
            quit_ = true;
            return;
        }

        if (key == KEY_ARROW_PREFIX) {
            changeDirection(_getch());
        }
    }

    void update() {
        if (gameOver_) {
            return;
        }

        Position newHead = snake_.back();

        switch (direction_) {
            case Direction::Up:    newHead.y--; break;
            case Direction::Down:  newHead.y++; break;
            case Direction::Left:  newHead.x--; break;
            case Direction::Right: newHead.x++; break;
        }

        if (hitsWall(newHead) || hitsSnake(newHead)) {
            gameOver_ = true;
            return;
        }

        snake_.push_back(newHead);

        if (newHead == food_) {
            spawnFood();  // keep the tail -> snake grows
        } else {
            snake_.erase(snake_.begin());
        }
    }

    void draw() const {
        resetCursor();

        const string border = "+" + string(WIDTH, '-') + "+\n";
        string screen;

        screen += "================================\n";
        screen += "           SNAKE GAME           \n";
        screen += "================================\n";
        screen += border;

        for (int y = 0; y < HEIGHT; y++) {
            screen += '|';
            for (int x = 0; x < WIDTH; x++) {
                screen += symbolAt({x, y});
            }
            screen += "|\n";
        }

        screen += border;
        screen += "\nArrow Keys = Move\n";
        screen += "Q = Quit\n";
        screen += "Score: " + to_string(score()) + "    \n";

        cout << screen << flush;
    }

    bool isOver() const { return gameOver_; }
    bool wantsToQuit() const { return quit_; }
    int score() const { return static_cast<int>(snake_.size()) - START_LENGTH; }

private:
    vector<Position> snake_;
    Position food_{0, 0};
    Direction direction_ = Direction::Right;
    bool gameOver_ = false;
    bool quit_ = false;
    mt19937 generator_;

    char symbolAt(const Position& pos) const {
        if (pos == food_) {
            return 'O';
        }
        if (pos == snake_.back()) {
            return '@';
        }
        return isOnSnake(pos) ? '#' : ' ';
    }

    bool isOnSnake(const Position& pos) const {
        for (const auto& part : snake_) {
            if (part == pos) {
                return true;
            }
        }
        return false;
    }

    static bool hitsWall(const Position& pos) {
        return pos.x < 0 || pos.x >= WIDTH || pos.y < 0 || pos.y >= HEIGHT;
    }

    // The current head is never checked: the new head is one step away from it.
    bool hitsSnake(const Position& pos) const {
        for (size_t i = 0; i + 1 < snake_.size(); i++) {
            if (snake_[i] == pos) {
                return true;
            }
        }
        return false;
    }

    void changeDirection(int arrowKey) {
        switch (arrowKey) {
            case KEY_UP:
                if (direction_ != Direction::Down) direction_ = Direction::Up;
                break;
            case KEY_DOWN:
                if (direction_ != Direction::Up) direction_ = Direction::Down;
                break;
            case KEY_LEFT:
                if (direction_ != Direction::Right) direction_ = Direction::Left;
                break;
            case KEY_RIGHT:
                if (direction_ != Direction::Left) direction_ = Direction::Right;
                break;
        }
    }

    void spawnFood() {
        uniform_int_distribution<int> xRandom(0, WIDTH - 1);
        uniform_int_distribution<int> yRandom(0, HEIGHT - 1);

        do {
            food_ = {xRandom(generator_), yRandom(generator_)};
        } while (isOnSnake(food_));
    }
};

// ---------------------------------------------------------------------------
// Game over screen. Returns true if the player wants to play again.
// ---------------------------------------------------------------------------
bool askRestart(int finalScore) {
    clearScreen();

    cout << "\n"
              << "================================\n"
              << "           GAME OVER!           \n"
              << "================================\n\n"
              << "Final Score: " << finalScore << "\n\n"
              << "R = Restart\n"
              << "Q = Quit\n";

    flushKeyboard();  // ignore keys pressed while the snake was crashing

    while (true) {
        int key = _getch();

        if (key == 'r' || key == 'R') {
            return true;
        }
        if (key == 'q' || key == 'Q') {
            return false;
        }
    }
}

// Main

int main() {
    hideCursor();

    SnakeGame game;
    bool playAgain = true;

    while (playAgain) {
        game.reset();
        clearScreen();

        while (!game.isOver()) {
            game.draw();
            game.handleInput();
            game.update();
            Sleep(DELAY_MS);
        }

        if (game.wantsToQuit()) {
            break;  // Q during play exits immediately
        }

        playAgain = askRestart(game.score());
    }

    clearScreen();
    cout << "Thanks for playing!\n";
    return 0;
}
