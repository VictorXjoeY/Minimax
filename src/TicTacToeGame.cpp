#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <optional>

#include <Common.hpp>
#include <TicTacToeGame.hpp>

/* ---------- PRIVATE ---------- */

bool TicTacToeGame::has_someone_won_() const {
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
TicTacToeState TicTacToeGame::get_state_() const {
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

	// Current player.
	if (get_player() == CROSS) {
		state += 0 * pow;
	}
	else if (get_player() == CIRCLE) {
		state += 1 * pow;
	}
	else {
		state += 2 * pow;
	}

	return TicTacToeState(state);
}

/* Loads the game given a State. */
void TicTacToeGame::load_game_(const TicTacToeState &state_) {
	int state = state_.get();

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

	// Current player.
	if (state % 3 == 0) {
		set_player_(CROSS);
	}
	else if (state % 3 == 1) {
		set_player_(CIRCLE);
	}
	else {
		set_player_(NONE);
	}
}

/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
void TicTacToeGame::make_move_(const TicTacToeMove &m_) {
	board[m_.c.x][m_.c.y] = get_player();
}

/* Returns a move inputed by the player. */
optional<TicTacToeMove> TicTacToeGame::get_player_move_(const string &command) const {
	int x, y;

	if (sscanf(command.c_str(), "%d %d", &x, &y) != 2) {
		return nullopt;
	}

	if (is_valid_move(TicTacToeMove(x, y))) {
		return TicTacToeMove(x, y);
	}

	return nullopt;
}

/* Returns all the current possible moves. */
vector<TicTacToeMove> TicTacToeGame::get_moves_() const {
	vector<TicTacToeMove> moves;

	if (has_someone_won_()) {
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
	if (has_someone_won_()) {
		return Game<TicTacToeState, TicTacToeMove>::get_winner_();
	}

	return NONE;
}

/* ---------- PUBLIC ---------- */

TicTacToeGame::TicTacToeGame() {
	memset(board, NONE, sizeof(board));
	set_player_(CROSS);
	Game<TicTacToeState, TicTacToeMove>::initialize_game_();
}

TicTacToeGame::TicTacToeGame(const TicTacToeState &state) {
	load_game_(state);
	Game<TicTacToeState, TicTacToeMove>::initialize_game_();
}

/* Returns if the move (x, y) is a valid move. */
bool TicTacToeGame::is_valid_move(const TicTacToeMove &m_) const {
	return is_inside(m_.c) and board[m_.c.x][m_.c.y] == NONE;
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

			if (board[x][y] == CROSS) {
				str += "X";
			}
			else if (board[x][y] == CIRCLE) {
				str += "O";
			}
			else {
				str += " ";
			}

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