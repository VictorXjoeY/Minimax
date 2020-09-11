#include <cstring>
#include <cctype>
#include <algorithm>
#include <string>
#include <vector>

#include <Common.hpp>
#include <BaghChalGame.hpp>

using namespace std;

// C++ is weird.
constexpr int BaghChalGame::DIR[8][2];
constexpr int BaghChalGame::INV_DIR[3][3];

/* ---------- PRIVATE ---------- */

/* Returns true if WHITE player is still placing sheeps on the board. */
bool BaghChalGame::is_first_phase() const {
	return sheeps > 0;
}

/* Returns the number sheeps which are alive. */
int BaghChalGame::sheep_count() const {
	int ans = sheeps;

	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			ans += board[x][y] == WHITE;
		}
	}

	return ans;
}

/* Returns the number of wolves currently stuck. */
int BaghChalGame::stuck_wolves_count() const {
	vector<BaghChalMove> moves = get_moves_for_(BLACK);

	if (moves.empty()) {
		return 4;
	}

	int free_wolves = 1;

	for (int i = 1; i < moves.size(); i++) {
		if (moves[i - 1].ci != moves[i].ci) {
			free_wolves++;
		}
	}

	return 4 - free_wolves;
}


/* Auxiliar function for the string conversion. */
void BaghChalGame::fill(char mat[D * (N - 1) + 1][D * (N - 1) + 1], int x, int y, int d) const {
	while (0 <= x and x < D * (N - 1) + 1 and 0 <= y and y < D * (N - 1) + 1) {
		mat[x][y] = '.';
		x += DIR[d][0];
		y += DIR[d][1];
	}
}

/* Returns true if its a valid first phase move. */
bool BaghChalGame::is_valid_first_phase_move_(const BaghChalMove &m) const {
	return is_first_phase() and get_player() == WHITE and board[m.ci.x][m.ci.y] == NONE;
}

/* Performs a first phase move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
void BaghChalGame::make_first_phase_move_(const BaghChalCell &c) {
	board[c.x][c.y] = WHITE;
	sheeps--;
}

/* Returns all the possible first phase moves for WHITE. */
vector<BaghChalMove> BaghChalGame::get_first_phase_moves_() const {
	vector<BaghChalMove> moves;

	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (board[x][y] == NONE) {
				moves.push_back(BaghChalMove(x, y, -1, -1));
			}
		}
	}

	return moves;
}

/* Returns all the possible moves for the current state of the game. */
vector<BaghChalMove> BaghChalGame::get_moves_for_(int player) const {
	vector<BaghChalMove> moves;

	if (sheep_count() <= 15) { // BLACK won.
		return vector<BaghChalMove>();
	}

	if (is_first_phase() and player == WHITE) { // Sheep placement move.
		return get_first_phase_moves_();
	}

	// General moves.
	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (board[x][y] == player) {
				int max_d = (x + y) % 2 == 0 ? 8 : 4;

				for (int d = 0; d < max_d; d++) {
					int xf = x + DIR[d][0];
					int yf = y + DIR[d][1];

					if (is_inside(xf, yf)) {
						// Basic move.
						if (board[xf][yf] == NONE) {
							moves.push_back(BaghChalMove(x, y, xf, yf));
						}

						// Capture.
						if (player == BLACK and board[xf][yf] == WHITE) {
							xf += DIR[d][0];
							yf += DIR[d][1];

							if (is_inside(xf, yf) and board[xf][yf] == NONE) {
								moves.push_back(BaghChalMove(x, y, xf, yf));
							}
						}
					}
				}
			}
		}
	}

	return moves;
}

/* ---------- PROTECTED ---------- */

/* Returns the current game state converted to State. */
long long BaghChalGame::get_state_() const {
	long long state = 0;
	long long pow = 1;

	// Board.
	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (board[x][y] == WHITE) {
				state += 0 * pow;
			}
			else if (board[x][y] == BLACK) {
				state += 1 * pow;
			}
			else {
				state += 2 * pow;
			}

			pow *= 3;
		}
	}

	// Remaining sheeps on hand.
	int sheeps_ = sheeps;

	for (int i = 0; i < 3; i++) {
		state += (sheeps_ % 3) * pow;
		sheeps_ /= 3;
		pow *= 3;
	}

	return state;
}

/* Loads the game given a State. */
void BaghChalGame::load_game_(const long long &state_) {
	long long state = state_;

	// Board.
	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (state % 3 == 0) {
				board[x][y] = WHITE;
			}
			else if (state % 3 == 1) {
				board[x][y] = BLACK;
			}
			else {
				board[x][y] = NONE;
			}

			state /= 3;
		}
	}

	// Remaining sheeps on hand.
	long long pow = 1;
	sheeps = 0;

	for (int i = 0; i < 3; i++) {
		sheeps += pow * (state % 3);
		pow *= 3;
		state /= 3;
	}
}

/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
void BaghChalGame::make_move_(const BaghChalMove &m) {
	if (is_first_phase() and get_player() == WHITE) {
		make_first_phase_move_(m.ci);
	}
	else {
		// Moving.
		swap(board[m.ci.x][m.ci.y], board[m.cf.x][m.cf.y]);

		if (chebyshev_distance(m.ci, m.cf) == 2) { // Capturing.
			board[(m.ci.x + m.cf.x) / 2][(m.ci.y + m.cf.y) / 2] = NONE;
		}

	}
}

/* Returns all the possible moves for the current state of the game. */
vector<BaghChalMove> BaghChalGame::get_moves_() const {
	return get_moves_for_(get_player());
}

/* ---------- PUBLIC ---------- */

BaghChalGame::BaghChalGame() {
	memset(board, NONE, sizeof(board));
	board[0][0] = board[0][N - 1] = board[N - 1][0] = board[N - 1][N - 1] = BLACK;
	sheeps = 20;
	Game<long long, BaghChalMove>::initialize_game_();
}

/* Returns true if the movement is valid. */
bool BaghChalGame::is_valid_move(const BaghChalMove &m) const {
	if (m.cf == BaghChalCell(-1, -1)) { // Sheep move during first phase.
		return is_valid_first_phase_move_(m);
	}

	if (board[m.ci.x][m.ci.y] != get_player()) { // Can't move a pawn that doesn't belong to the current player.
		return false;
	}

	if (board[m.cf.x][m.cf.y] != NONE) { // Can only move to empty cells.
		return false;
	}

	if (get_player() == WHITE) { // Sheeps only move one cell.
		if ((m.ci.x + m.ci.y) % 2 == 0) { // 8 directions.
			return chebyshev_distance(m.ci, m.cf) == 1;
		}
		else { // 4 directions.
			return manhattan_distance(m.ci, m.cf) == 1;
		}
	}
	else { // Wolves move one cell or capture a sheep.
		int vx = m.cf.x - m.ci.x;
		int vy = m.cf.y - m.ci.y;

		if (vx % max(vx, vy) != 0 or vy % max(vx, vy) != 0) {
			return false;
		}

		int d = INV_DIR[1 + vx / max(vx, vy)][1 + vy / max(vx, vy)];
		
		if ((m.ci.x + m.ci.y) % 2 != 0 and d >= 4) { // Only 4 directions. Can't move diagonally.
			return false;
		}

		BaghChalCell c = m.ci;

		// One step.
		c.x += DIR[d][0];
		c.y += DIR[d][1];

		if (c == m.cf) { // Basic move.
			return true;
		}

		if (board[c.x][c.y] != WHITE) { // Can't capture non-white.
			return false;
		}

		// Two steps.
		c.x += DIR[d][0];
		c.y += DIR[d][1];

		return c == m.cf;
	}
}

/* Returns a move inputed by the player. */
BaghChalMove BaghChalGame::get_player_move() const {
	int xi, yi, xf, yf;

	if (is_first_phase() and get_player() == WHITE) {
		xf = yf = -1;

		do {
			scanf("%d %d", &xi, &yi);
		} while (!is_inside(xi, yi) or !is_valid_move(BaghChalMove(xi, yi, -1, -1)));
	}
	else {
		do {
			scanf("%d %d %d %d", &xi, &yi, &xf, &yf);
		} while (!is_inside(xi, yi) or !is_inside(xf, yf) or !is_valid_move(BaghChalMove(xi, yi, xf, yf)));
	}

	printf("\n");

	return BaghChalMove(xi, yi, xf, yf);
}

/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
double BaghChalGame::evaluate() const {
	// Pretending that we are WHITE.
	int stuck_wolves = stuck_wolves_count();
	int dead_sheep = 20 - sheep_count();

	return clamp(stuck_wolves * 0.05 - dead_sheep * 0.20, -1.0, 1.0);
}

/* Returns the board for printing. */
BaghChalGame::operator string() const {
	// Initializing board with dots.
	char mat[D * (N - 1) + 1][D * (N - 1) + 1];
	memset(mat, ' ', sizeof(mat));

	// For each row.
	for (int x = 0; x < N; x++) {
		fill(mat, D * x, 0, RIGHT);

		if (x % 2 == 0) {
			fill(mat, D * x, 0, UPRIGHT);
			fill(mat, D * x, 0, DOWNRIGHT);
		}
	}

	// For each column.
	for (int y = 0; y < N; y++) {
		fill(mat, 0, D * y, DOWN);

		if (y % 2 == 0) {
			fill(mat, D * (N - 1), D * y, UPRIGHT);
			fill(mat, 0, D * y, DOWNRIGHT);
		}
	}

	// Filling board with pawns.
	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			if (board[x][y] == WHITE) {
				mat[D * x][D * y] = 'w';
			}
			else if (board[x][y] == BLACK) {
				mat[D * x][D * y] = 'b';
			}
			else {
				mat[D * x][D * y] = 'o';
			}
		}
	}

	// Highlighting.
	if (!is_first_phase() or get_player() == BLACK) {
		vector<BaghChalMove> moves = get_moves_();

		for (const BaghChalMove &move : moves) {
			mat[D * move.ci.x][D * move.ci.y] = toupper(mat[D * move.ci.x][D * move.ci.y]);
		}
	}

	// Filling the string.
	string str = "";

	for (int x = 0; x < D * (N - 1) + 1; x++) {
		for (int y = 0; y < D * (N - 1) + 1; y++) {
			if (tolower(mat[x][y]) == 'w') {
				if (mat[x][y] == 'W') {
					str += COLOR_BRIGHT_MAGENTA;
				}
				else {
					str += COLOR_RED;
				}

				str += "w ";
				str += COLOR_WHITE;
			}
			else if (tolower(mat[x][y]) == 'b') {
				if (mat[x][y] == 'B') {
					str += COLOR_MAGENTA;
				}
				else {
					str += COLOR_BLUE;
				}

				str += "b ";
				str += COLOR_WHITE;
			}
			else if (mat[x][y] == 'o') {
				str += COLOR_YELLOW;
				str += "o ";
				str += COLOR_WHITE;
			}
			else if (mat[x][y] == '.') {
				str += COLOR_YELLOW;
				str += ". ";
				str += COLOR_WHITE;
			}
			else {
				str += "  ";
			}
		}

		str += "\n";
	}

	return str;
}