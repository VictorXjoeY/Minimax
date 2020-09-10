#include <cassert>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <string>
#include <vector>

#include <Common.hpp>

#include <MuTorereGame.hpp>

/* ---------- PRIVATE ---------- */

/* Returns the position which is empty. */
int MuTorereGame::get_empty_position() {
	for (int p = 0; p < N + 1; p++) {
		if (board[p] == NONE) {
			return p;
		}
	}

	assert(false);
	return -1;
}

/* ---------- PROTECTED ---------- */

/* Returns the current game state converted to State. */
int MuTorereGame::get_state_() const {
	int state = 0;
	int pow = 1;

	for (int i = 0; i < N + 1; i++) {
		if (board[i] == WHITE) {
			state += 0 * pow;
		}
		else if (board[i] == BLACK) {
			state += 1 * pow;
		}
		else {
			state += 2 * pow;
		}

		pow *= 3;
	}

	return state;
}

/* Initializes a new game. This function should be called at the end of initialize_game_() of the derived classes. */
void MuTorereGame::initialize_game_() {
	for (int p = 0; p < N / 2; p++) {
		board[p] = WHITE;
	}

	for (int p = N / 2; p < N; p++) {
		board[p] = BLACK;
	}

	board[N] = NONE;

	Game<int, MuTorereMove>::initialize_game_();
}

/* Loads the game given a State. */
void MuTorereGame::load_game_(const int &state_) {
	int state = state_;

	for (int i = 0; i < N + 1; i++) {
		if (state % 3 == 0) {
			board[i] = WHITE;
		}
		else if (state % 3 == 1) {
			board[i] = BLACK;
		}
		else {
			board[i] = NONE;
		}

		state /= 3;
	}
}

/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
void MuTorereGame::make_move_(const MuTorereMove &move) {
	swap(board[move.pos], board[get_empty_position()]);
}

/* Returns all the possible moves for the current state of the game. */
vector<MuTorereMove> MuTorereGame::get_moves_() const {
	vector<MuTorereMove> moves;

	for (int p = 0; p < N + 1; p++) {
		if (is_valid_move(MuTorereMove(p))) {
			moves.push_back(MuTorereMove(p));
		}
	}

	return moves;
}

/* ---------- PUBLIC ---------- */

/* Returns true if the movement is valid. */
bool MuTorereGame::is_valid_move(const MuTorereMove &move) const {
	if (board[move.pos] != get_player()) { // Can't move a pawn if it doesn't belong to the current player.
		return false;
	}

	if (move.pos == N) { // Can always move from the center.
		return true;
	}

	if (board[N] == NONE) { // Can move to the center if adjacent to an enemy. 
		return board[(move.pos + N - 1) % N] == get_enemy() or board[(move.pos + 1) % N] == get_enemy();
	}

	// Can move around the circle if adjacent to an empty space.
	return board[(move.pos + N - 1) % N] == NONE or board[(move.pos + 1) % N] == NONE;
}

/* Returns a move inputed by the player. */
MuTorereMove MuTorereGame::get_player_move() const {
	int p;

	do {
		scanf("%d", &p);
	} while (p < 0 or p >= 9 or !is_valid_move(MuTorereMove(p)));

	printf("\n");

	return p;
}

MuTorereGame::operator string() const {
	// Creating the char matrix.
	int pos[N + 1][2] = {{7, 1}, {8, 4}, {7, 7}, {4, 8}, {1, 7}, {0, 4}, {1, 1}, {4, 0}, {4, 4}};
	char c[N + 1][N + 1];
	memset(c, ' ', sizeof(c));

	for (int p = 0; p < N + 1; p++) {
		if (board[p] == WHITE) {
			c[pos[p][0]][pos[p][1]] = is_valid_move(MuTorereMove(p)) ? 'W' : 'w';
		}
		else if (board[p] == BLACK) {
			c[pos[p][0]][pos[p][1]] = is_valid_move(MuTorereMove(p)) ? 'B' : 'b';
		}
		else {
			c[pos[p][0]][pos[p][1]] = '.';
		}
	}

	// Placing everything in str.
	string str = "";

	for (int i = 0; i < N + 1; i++) {
		for (int j = 0; j < N + 1; j++) {
			if (tolower(c[i][j]) == 'w') {
				if (c[i][j] == 'W') {
					str += COLOR_BRIGHT_MAGENTA;
				}
				else {
					str += COLOR_RED;
				}

				str += "w ";
				str += COLOR_WHITE;
			}
			else if (tolower(c[i][j]) == 'b') {
				if (c[i][j] == 'B') {
					str += COLOR_MAGENTA;
				}
				else {
					str += COLOR_BLUE;
				}

				str += "b ";
				str += COLOR_WHITE;
			}
			else if (c[i][j] == '.') {
				str += ". ";
			}
			else {
				str += "  ";
			}
		}

		str += "\n";
	}

	return str;
}