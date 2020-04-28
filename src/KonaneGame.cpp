#include <cstdio>
#include <cassert>
#include <string>
#include <vector>

#include <Common.hpp>
#include <KonaneGame.hpp>

// C++ is weird.
constexpr int KonaneGame::DIR[2][4];

/* ---------- PRIVATE ---------- */

/* Returns the number of pawns remaining on the board. */
int KonaneGame::count_pawns() const {
	return __builtin_popcountll(board & ((1ll << (N * N)) - 1));
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

/* Toggles the current player. */
void KonaneGame::toggle_player() {
	board ^= (1ll << (N * N));
}

/* Returns if the move (x, y) at the start of the game is a valid move. */
bool KonaneGame::is_valid_starting_move(const KonaneCell &c) const {
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
long long KonaneGame::get_current_state_() const {
	return board;
}

/* Initializes a new game. This function should be called at the end of initialize_game_() of the derived classes. */
void KonaneGame::initialize_game_() {
	board = (1ll << (N * N)) - 1; // All 36 cells are filled in with pawns. White starts.
	Game<long long, KonaneMove>::initialize_game_();
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

	// Toggle player.
	toggle_player();
}

/* ---------- PUBLIC ---------- */

/* Current player. */
int KonaneGame::get_current_player() const {
	return (board >> (N * N)) & 1ll ? BLACK : WHITE;
}

/* Returns winner if there is one. */
int KonaneGame::get_winner() const {
	return is_game_over() ? get_enemy(get_current_player()) : NONE;
}

/* Returns true if the next move is the first move of the match. */
bool KonaneGame::is_first_turn() const {
	return count_pawns() == N * N;
}

/* Returns true if the next move is the second move of the match. */
bool KonaneGame::is_second_turn() const {
	return count_pawns() == N * N - 1;
}

/* Returns if the move (xi, yi) -> (xf, yf) is a valid move. */
bool KonaneGame::is_valid_move(const KonaneMove &m_) const {
	KonaneMove m = m_;

	if (m.cf == KonaneCell(-1, -1)) { // One of the two first moves.
		return is_valid_starting_move(m.ci);
	}

	if (test(m.ci) != get_current_player()) { // Can't move a pawn that is not yours.
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

			if (test(m.ci) != get_enemy(get_current_player())) { // Has to jump over an enemy.
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

			if (test(m.ci) != get_enemy(get_current_player())) { // Has to jump over an enemy.
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

/* Returns all the current possible moves. */
vector<KonaneMove> KonaneGame::get_moves() const {
	vector<KonaneMove> moves;

	if (is_first_turn() or is_second_turn()) {
		return get_starting_moves();
	}

	for (int xi = 0; xi < N; xi++) {
		for (int yi = 0; yi < N; yi++) {
			if (test(KonaneCell(xi, yi)) == get_current_player()) { // For every pawn of the current player.
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

/* Returns a move inputed by the player. */
KonaneMove KonaneGame::get_player_move() const {
	int xi, yi, xf, yf;

	if (is_first_turn() or is_second_turn()) {
		xf = yf = -1;

		do {
			scanf("%d %d", &xi, &yi);
		} while (!is_inside(KonaneCell(xi, yi)) or !is_valid_move(KonaneMove(xi, yi, -1, -1)));
	}
	else {
		do {
			scanf("%d %d %d %d", &xi, &yi, &xf, &yf);
		} while (!is_inside(KonaneCell(xi, yi)) or !is_inside(KonaneCell(xf, yf)) or !is_valid_move(KonaneMove(xi, yi, xf, yf)));
	}

	printf("\n");

	return KonaneMove(xi, yi, xf, yf);
}

/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
double KonaneGame::evaluate() {
	return 0.0;
}

/* Returns if the game is over (current player can't make any more moves). */
bool KonaneGame::is_game_over() const {
	return get_moves().empty();
}

/* Returns the board for printing. */
KonaneGame::operator string() const {
	vector<KonaneMove> moves = get_moves();
	long long highlighted = 0;
	string str;

	// Marking all (xi, yi).
	for (const KonaneMove &move : moves) {
		highlighted |= 1ll << (long long)convert_cell(move.ci);
	}

	// Printing board.
	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (test(KonaneCell(x, y)) == WHITE) {
				if ((highlighted >> (long long)convert_cell(KonaneCell(x, y))) & 1ll) {
					str += COLOR_BRIGHT_MAGENTA;
				}
				else {
					str += COLOR_RED;
				}

				str += "w ";
				str += COLOR_WHITE;
			}
			else if (test(KonaneCell(x, y)) == BLACK) {
				if ((highlighted >> (long long)convert_cell(KonaneCell(x, y))) & 1ll) {
					str += COLOR_MAGENTA;
				}
				else {
					str += COLOR_BLUE;
				}

				str += "b ";
				str += COLOR_WHITE;
			}
			else {
				str += ". ";
			}
		}

		str += "\n";
	}

	return str;
}