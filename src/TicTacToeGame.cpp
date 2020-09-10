#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <Common.hpp>
#include <TicTacToeGame.hpp>

/* ---------- PRIVATE ---------- */

bool TicTacToeGame::is_game_over_() const {
	int x, y;

	// Checking rows.
	for (x = 0; x < N; x++) {
		if (board[x][0] != NONE) {
			for (y = 1; y < N; y++) {
				if (board[x][y] != board[x][0]) {
					break;
				}
			}

			if (y == N) {
				return true;
			}
		}
	}

	// Checking columns.
	for (y = 0; y < N; y++) {
		if (board[0][y] != NONE) {
			for (x = 1; x < N; x++) {
				if (board[x][y] != board[0][y]) {
					break;
				}
			}

			if (x == N) {
				return true;
			}
		}
	}

	// Checking main diagonal.
	if (board[0][0] != NONE) {
		for (x = 1; x < N; x++) {
			if (board[x][x] != board[0][0]) {
				break;
			}
		}

		if (x == N) {
			return true;
		}
	}

	// Checking secondary diagonal.
	if (board[N - 1][0] != NONE) {
		for (x = 1; x < N; x++) {
			if (board[N - 1 - x][x] != board[N - 1][0]) {
				break;
			}
		}

		if (x == N) {
			return true;
		}
	}

	return false;
}

/* ---------- PROTECTED ---------- */

/* Returns the current game state converted to State. */
int TicTacToeGame::get_state_() const {
	int state = 0;
	int pow = 1;

	// Board.
	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (board[x][y] == CROSS) {
				state += 0 * pow;
			}
			else if (board[x][y] == CIRCLE) {
				state += 1 * pow;
			}
			else {
				state += 2 * pow;
			}

			pow *= 3;
		}
	}

	return state;
}

/* Loads the game given a State. */
void TicTacToeGame::load_game_(const int &state_) {
	int state = state_;

	// Board.
	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (state % 3 == 0) {
				board[x][y] = CROSS;
			}
			else if (state % 3 == 1) {
				board[x][y] = CIRCLE;
			}
			else {
				board[x][y] = NONE;
			}

			state /= 3;
		}
	}
}

/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
void TicTacToeGame::make_move_(const TicTacToeMove &m_) {
	board[m_.c.x][m_.c.y] = get_player();
}

/* Returns all the current possible moves. */
vector<TicTacToeMove> TicTacToeGame::get_moves_() const {
	vector<TicTacToeMove> moves;

	if (is_game_over_()) {
		return vector<TicTacToeMove>();
	}

	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (is_valid_move(TicTacToeMove(x, y))) {
				moves.push_back(TicTacToeMove(x, y));
			}
		}
	}

	return moves;
}

/* Returns the winner. */
int TicTacToeGame::get_winner_() const {
	if (is_game_over_()) {
		return Game<int, TicTacToeMove>::get_winner_();
	}

	return NONE;
}

/* ---------- PUBLIC ---------- */

TicTacToeGame::TicTacToeGame() {
	memset(board, NONE, sizeof(board));
	Game<int, TicTacToeMove>::initialize_game_();
}

/* Returns if the move (x, y) is a valid move. */
bool TicTacToeGame::is_valid_move(const TicTacToeMove &m_) const {
	return is_inside(m_.c) and board[m_.c.x][m_.c.y] == NONE;
}

/* Returns a move inputed by the player. */
TicTacToeMove TicTacToeGame::get_player_move() const {
	int x, y;

	do {
		scanf("%d %d", &x, &y);
	} while (!is_valid_move(TicTacToeMove(x, y)));

	printf("\n");

	return TicTacToeMove(x, y);
}

/* Returns the board for printing. */
TicTacToeGame::operator string() const {
	string str;

	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			str += "     ";
			str += y < N - 1 ? "|" : "";
		}

		str += "\n";

		for (int y = 0; y < N; y++) {
			str += "  ";
			str += get_player_piece(board[x][y]);
			str += "  ";
			str += y < N - 1 ? "|" : "";
		}

		str += "\n";

		for (int y = 0; y < N; y++) {
			str += x < N - 1 ? "_____" : "     ";
			str += y < N - 1 ? "|" : "";
		}
		
		str += "\n";
	}

	return str;
}