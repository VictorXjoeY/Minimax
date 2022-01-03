#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include <Common.hpp>
#include <ConnectFourGame.hpp>

// C++ is weird.
constexpr int ConnectFourGame::DIR[4][2];

/* ---------- PRIVATE ---------- */

/* Returns the score of the board for the given player. */
int ConnectFourGame::get_score_(int player) const {
    int enemy = player == YELLOW ? RED : YELLOW;
    int score = 0;
    int i;

    for (int x = 0; x < N; x++) {
        for (int y = 0; y < M; y++) {
            for (int d = 0; d < 4; d++) {
                // Considering all possible win conditions (4 cells connected).
                if (!is_inside(x + 3 * DIR[d][0], y + 3 * DIR[d][1])) {
                    continue;
                }

                int player_count = 0;

                for (i = 0; i < 4; i++) {
                    int xf = x + i * DIR[d][0];
                    int yf = y + i * DIR[d][1];

                    if (board[xf][yf] == enemy) {
                        break;
                    }

                    player_count += board[xf][yf] == player;
                }

                // If no enemies were found in this 4 sequence.
                if (i == 4) {
                    score += player_count + 1;
                }
            }
        }
    }

    return score;
}

/* Checks if there are 4 pieces connected. */
bool ConnectFourGame::has_someone_won_() const {
    int i;

    for (int x = 0; x < N; x++) {
        for (int y = 0; y < M; y++) {
            if (board[x][y] != NONE) {
                for (int d = 0; d < 4; d++) {
                    if (!is_inside(x + 3 * DIR[d][0], y + 3 * DIR[d][1])) {
                        continue;
                    }

                    for (i = 1; i < 4; i++) {
                        int xf = x + i * DIR[d][0];
                        int yf = y + i * DIR[d][1];

                        if (!is_inside(xf, yf) or board[xf][yf] != board[x][y]) {
                            break;
                        }
                    }

                    if (i == 4) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

/* ---------- PROTECTED ---------- */

/* Returns the current game state converted to State. */
ConnectFourState ConnectFourGame::get_state_() const {
    long long yellow = 0, red = 0, pow = 1;

    // Board.
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < M; y++) {
            if (board[x][y] == YELLOW) {
                yellow |= pow;
            } else if (board[x][y] == RED) {
                red |= pow;
            }

            pow *= 2;
        }
    }

    // Current player.
    if (get_player() == YELLOW) {
        return ConnectFourState(to_string(yellow) + "y" + to_string(red));
    }

    if (get_player() == RED) {
        return ConnectFourState(to_string(yellow) + "r" + to_string(red));
    }

    return ConnectFourState(to_string(yellow) + "n" + to_string(red));
}

/* Loads the game given a State. */
void ConnectFourGame::load_game_(const ConnectFourState &state_) {
    const string &state = state_.get();
    int pos = state.find("y");

    // Current player.
    if (pos == string::npos) {
        pos = state.find("r");

        if (pos == string::npos) {
            pos = state.find("n");
            set_player_(NONE);
        } else {
            set_player_(RED);
        }
    } else {
        set_player_(YELLOW);
    }

    long long yellow = stoll(state.substr(0, pos));
    long long red = stoll(state.substr(pos + 1));

    // Board.
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < M; y++) {
            if (yellow & 1) {
                board[x][y] = YELLOW;
            } else if (red & 1) {
                board[x][y] = RED;
            } else {
                board[x][y] = NONE;
            }

            yellow /= 2;
            red /= 2;
        }
    }
}

/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
void ConnectFourGame::make_move_(const ConnectFourMove &m) {
    for (int x = N - 1; x >= 0; x--) {
        if (board[x][m.y] == NONE) {
            board[x][m.y] = get_player();
            break;
        }
    }
}

/* Returns a move inputed by the player. */
optional<ConnectFourMove> ConnectFourGame::get_player_move_(const string &command) const {
    int p;

    if (sscanf(command.c_str(), "%d", &p) != 1) {
        return nullopt;
    }

    if (is_valid_move(ConnectFourMove(p))) {
        return ConnectFourMove(p);
    }

    return nullopt;
}

/* Returns all the current possible moves. */
vector<ConnectFourMove> ConnectFourGame::get_moves_() const {
    vector<ConnectFourMove> moves;

    if (has_someone_won_()) {
        return vector<ConnectFourMove>();
    }

    for (int y = 0; y < M; y++) {
        if (is_valid_move(ConnectFourMove(y))) {
            moves.push_back(ConnectFourMove(y));
        }
    }

    return moves;
}

/* Returns the winner. */
int ConnectFourGame::get_winner_() const {
    if (has_someone_won_()) {
        return Game<ConnectFourState, ConnectFourMove>::get_winner_();
    }

    return NONE;
}

/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
double ConnectFourGame::evaluate_() const {
    int score = get_score_(YELLOW) - get_score_(RED);
    return score / static_cast<double>(MAX_SCORE);
}

/* ---------- PUBLIC ---------- */

ConnectFourGame::ConnectFourGame() {
    memset(board, NONE, sizeof(board));
    set_player_(YELLOW);
    Game<ConnectFourState, ConnectFourMove>::initialize_game_();
}

ConnectFourGame::ConnectFourGame(const ConnectFourState &state) {
    load_game_(state);
    Game<ConnectFourState, ConnectFourMove>::initialize_game_();
}

/* Returns if the move (x, y) is a valid move. */
bool ConnectFourGame::is_valid_move(const ConnectFourMove &m) const {
    return 0 <= m.y and m.y < M and board[0][m.y] == NONE;
}

/* Returns the board for printing. */
ConnectFourGame::operator string() const {
    string str;

    for (int y = 0; y < M; y++) {
        str += "   " + to_string(y) + "  ";
    }

    str += " \n";

    for (int y = 0; y < M; y++) {
        str += "......";
    }

    str += ".\n";

    for (int x = 0; x < N; x++) {
        for (int y = 0; y < M; y++) {
            str += "|     ";
        }

        str += "|\n";

        for (int y = 0; y < M; y++) {
            str += "|  ";

            if (board[x][y] == YELLOW) {
                str += COLOR_YELLOW "X" COLOR_WHITE;
            } else if (board[x][y] == RED) {
                str += COLOR_RED "O" COLOR_WHITE;
            } else {
                str += " ";
            }

            str += "  ";
        }

        str += "|\n";

        for (int y = 0; y < M; y++) {
            str += "|_____";
        }

        str += "|\n";
    }

    return str;
}