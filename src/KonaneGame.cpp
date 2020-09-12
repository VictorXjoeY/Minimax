#include <cstdio>
#include <cassert>
#include <string>
#include <vector>
#include <optional>

#include <Common.hpp>
#include <KonaneGame.hpp>

// C++ is weird.
constexpr int KonaneGame::DIR[2][4];

/* ---------- PRIVATE ---------- */

/* Returns the number of pawns remaining on the board. */
int KonaneGame::count_pawns() const {
	return __builtin_popcountll(board);
}

/* Returns what is in cell (x, y). */
int KonaneGame::test(const KonaneCell &c) const {
	if ((board >> (long long)convert_cell(c)) & 1ll) {
		return (c.x + c.y) % 2 ? WHITE : BLACK;
	}

	return NONE;
}

/* Sets cell (x, y). */
void KonaneGame::set(const KonaneCell &c) {
	board |= (1ll << (long long)convert_cell(c));
}

/* Resets cell (x, y). */
void KonaneGame::reset(const KonaneCell &c) {
	board &= ~(1ll << (long long)convert_cell(c));
}

/* Returns true if the next move is the first move of the match. */
bool KonaneGame::is_first_turn() const {
	return count_pawns() == N * N;
}

/* Returns true if the next move is the second move of the match. */
bool KonaneGame::is_second_turn() const {
	return count_pawns() == N * N - 1;
}

/* Returns if the move (x, y) at the start of the game is a valid move. */
bool KonaneGame::is_valid_starting_move(const KonaneCell &c) const {
	if (!is_inside(c)) {
		return false;
	}

	if (is_first_turn()) { // First move.
		if (c == KonaneCell(3, 2) or c == KonaneCell(2, 3)) { // Center.
			return true;
		}

		if (c == KonaneCell(5, 0)) { // Bottom corner.
			return true;
		}


		if (c == KonaneCell(0, 5)) { // Top corner.
			return true;
		}

		// Not one of the 4 possibilities.
		return false;
	}

	if (is_second_turn()) { // Second move.
		if (test(KonaneCell(3, 2)) == NONE or test(KonaneCell(2, 3)) == NONE) { // Center.
			return c == KonaneCell(2, 2) or c == KonaneCell(3, 3);
		}

		if (test(KonaneCell(5, 0)) == NONE) { // Bottom corner.
			return c == KonaneCell(4, 0) or c == KonaneCell(5, 1);
		}
		
		if (test(KonaneCell(0, 5)) == NONE) { // Top corner.
			return c == KonaneCell(0, 4) or c == KonaneCell(1, 5);
		}

		// The current state is corrupted.
		assert(false);
		return false;
	}

	// Single cell move is only valid for the first two moves of the game.
	return false;
}

/* Makes the move (x, y) for the first two moves in the game. Assumes that isValidMove(x, y) is true. */
void KonaneGame::make_starting_move(const KonaneMove &m) {
	reset(m.ci);
}

/* Returns all the possible start moves. */
vector<KonaneMove> KonaneGame::get_starting_moves() const {
	vector<KonaneMove> moves;

	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (is_valid_starting_move(KonaneCell(x, y))) {
				moves.push_back(KonaneMove(x, y, -1, -1));
			}
		}
	}

	return moves;
}

/* ---------- PROTECTED ---------- */

/* Returns the current game state converted to State. */
long long KonaneGame::get_state_() const {
	return board;
}

/* Loads the game given a State. */
void KonaneGame::load_game_(const long long &state) {
	board = state;
}

/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
void KonaneGame::make_move_(const KonaneMove &m_) {
	KonaneMove m = m_;

	if (m.cf == KonaneCell(-1, -1)) {
		make_starting_move(m);
	}
	else {
		reset(m.ci); // Move pawn from (xi, yi).
		set(m.cf); // To (xf, yf).

		if (m.ci.x == m.cf.x) { // Moving horizontally.
			int d = m.ci.y < m.cf.y ? RIGHT : LEFT;

			while (m.ci.y != m.cf.y) {
				m.ci.y += DIR[1][d];
				reset(m.ci); // Enemy pawn at (xi, yi) is killed.
				m.ci.y += DIR[1][d];
			}
		}
		else if (m.ci.y == m.cf.y) { // Moving vertically.
			int d = m.ci.x < m.cf.x ? DOWN : UP;

			while (m.ci.x != m.cf.x) {
				m.ci.x += DIR[0][d];
				reset(m.ci); // Enemy pawn at (xi, yi) is killed.
				m.ci.x += DIR[0][d];
			}
		}
	}
}

/* Returns a move inputed by the player. */
optional<KonaneMove> KonaneGame::get_player_move_(const string &command) const {
	int xi, yi, xf, yf;

	if (is_first_turn() or is_second_turn()) {
		xf = yf = -1;

		if (sscanf(command.c_str(), "%d %d", &xi, &yi) != 2) {
			return nullopt;
		}
	}
	else {
		if (sscanf(command.c_str(), "%d %d %d %d", &xi, &yi, &xf, &yf) != 4) {
			return nullopt;
		}
	}

	if (is_valid_move(KonaneMove(xi, yi, xf, yf))) {
		return KonaneMove(xi, yi, xf, yf);
	}

	return nullopt;
}

/* Returns all the current possible moves. */
vector<KonaneMove> KonaneGame::get_moves_() const {
	vector<KonaneMove> moves;

	if (is_first_turn() or is_second_turn()) {
		return get_starting_moves();
	}

	for (int xi = 0; xi < N; xi++) {
		for (int yi = 0; yi < N; yi++) {
			if (test(KonaneCell(xi, yi)) == get_player()) { // For every pawn of the current player.
				for (int d = 0; d < 4; d++) { // For every direction.
					int xf = xi + 2 * DIR[0][d];
					int yf = yi + 2 * DIR[1][d];

					while (is_inside(KonaneCell(xf, yf))) {
						if (is_valid_move(KonaneMove(xi, yi, xf, yf))) {
							moves.push_back(KonaneMove(xi, yi, xf, yf));
						}

						xf += 2 * DIR[0][d];
						yf += 2 * DIR[1][d];
					}
				}
			}
		}
	}

	return moves;
}

/* ---------- PUBLIC ---------- */

KonaneGame::KonaneGame() {
	board = (1ll << (N * N)) - 1; // All 36 cells are filled in with pawns. White starts.
	Game<long long, KonaneMove>::initialize_game_();
}

/* Returns if the move (xi, yi) -> (xf, yf) is a valid move. */
bool KonaneGame::is_valid_move(const KonaneMove &m_) const {
	KonaneMove m = m_;

	if (m.cf == KonaneCell(-1, -1)) { // One of the two first moves.
		return is_valid_starting_move(m.ci);
	}

	if (!is_inside(m.ci) or !is_inside(m.cf)) { // Move out of bounds.
		return false;
	}

	if (test(m.ci) != get_player()) { // Can't move a pawn that is not yours.
		return false;
	}

	if (test(m.cf) != NONE) { // Can only move to empty cells.
		return false;
	}

	if ((m.ci.x + m.ci.y) % 2 != (m.cf.x + m.cf.y) % 2) { // White can only move on odd cells and Black can only move on even cells.
		return false;
	}

	if (m.ci.x == m.cf.x) { // Moving horizontally.
		int d = m.ci.y < m.cf.y ? RIGHT : LEFT;

		while (m.ci.y != m.cf.y) {
			m.ci.y += DIR[1][d];

			if (test(m.ci) != get_enemy()) { // Has to jump over an enemy.
				return false;
			}

			m.ci.y += DIR[1][d];

			if (test(m.ci) != NONE) { // And land on an empty cell.
				return false;
			}
		}

		return true;
	}
	
	if (m.ci.y == m.cf.y) { // Moving vertically.
		int d = m.ci.x < m.cf.x ? DOWN : UP;

		while (m.ci.x != m.cf.x) {
			m.ci.x += DIR[0][d];

			if (test(m.ci) != get_enemy()) { // Has to jump over an enemy.
				return false;
			}

			m.ci.x += DIR[0][d];

			if (test(m.ci) != NONE) { // And land on an empty cell.
				return false;
			}
		}

		return true;
	}

	// Can only move horizontally or vertically.
	return false;
}

/* Returns the board for printing. */
KonaneGame::operator string() const {
	vector<KonaneMove> moves = get_moves();
	long long highlighted = 0;

	// Marking all (xi, yi).
	for (const KonaneMove &move : moves) {
		highlighted |= 1ll << (long long)convert_cell(move.ci);
	}

	string str = "       ";

	for (int y = 0; y < N; y++) {
		str += "   " + to_string(y) + "  ";
	}

	str += " \n";
	str += "       ";

	for (int y = 0; y < N; y++) {
		str += "______";
	}

	str += "_\n";

	// Printing board.
	for (int x = 0; x < N; x++) {
		str += "       ";

		for (int y = 0; y < N; y++) {
			str += "|     ";
		}

		str += "|\n";
		str += "   " + to_string(x) + "   ";
		
		for (int y = 0; y < N; y++) {
			str += "|  ";

			if (test(KonaneCell(x, y)) == WHITE) {
				if ((highlighted >> (long long)convert_cell(KonaneCell(x, y))) & 1ll) {
					str += COLOR_BRIGHT_MAGENTA;
				}
				else {
					str += COLOR_RED;
				}

				str += "W";
				str += COLOR_WHITE;
			}
			else if (test(KonaneCell(x, y)) == BLACK) {
				if ((highlighted >> (long long)convert_cell(KonaneCell(x, y))) & 1ll) {
					str += COLOR_MAGENTA;
				}
				else {
					str += COLOR_BLUE;
				}

				str += "B";
				str += COLOR_WHITE;
			}
			else {
				str += " ";
			}

			str += "  ";
		}

		str += "|\n";
		str += "       ";

		for (int y = 0; y < N; y++) {
			str += "|_____";
		}
		
		str += "|\n";
	}

	return str;
}