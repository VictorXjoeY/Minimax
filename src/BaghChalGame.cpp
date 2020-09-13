#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <optional>

#include <Common.hpp>
#include <BaghChalGame.hpp>

using namespace std;

// C++ is weird.
constexpr int BaghChalGame::DIR[8][2];
constexpr int BaghChalGame::INV_DIR[3][3];

/* ---------- PRIVATE ---------- */

/* Returns true if SHEEP player is still placing sheeps on the board. */
bool BaghChalGame::is_first_phase() const {
	return sheeps > 0;
}

/* Returns the number sheeps which are alive. */
int BaghChalGame::sheep_count() const {
	int ans = sheeps;

	for (int x = 0; x < N; x++) {
		for (int y = 0; y < N; y++) {
			ans += board[x][y] == SHEEP;
		}
	}

	return ans;
}

/* Returns the number of wolves currently stuck. */
int BaghChalGame::stuck_wolves_count() const {
	vector<BaghChalMove> moves = get_moves_for_(WOLF);

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
bool BaghChalGame::is_valid_sheep_placement_move_(const BaghChalMove &m) const {
	return is_first_phase() and get_player() == SHEEP and is_inside(m.ci) and board[m.ci.x][m.ci.y] == NONE;
}

/* Performs a first phase move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
void BaghChalGame::make_sheep_placement_move_(const BaghChalCell &c) {
	board[c.x][c.y] = SHEEP;
	sheeps--;
}

/* Returns all the possible first phase moves for SHEEP. */
vector<BaghChalMove> BaghChalGame::get_sheep_placement_moves_() const {
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

	if (sheep_count() <= 15) { // WOLF won.
		return vector<BaghChalMove>();
	}

	if (is_first_phase() and player == SHEEP) { // Sheep placement move.
		return get_sheep_placement_moves_();
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
						if (player == WOLF and board[xf][yf] == SHEEP) {
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
			if (board[x][y] == SHEEP) {
				state += 0 * pow;
			}
			else if (board[x][y] == WOLF) {
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

	// Current player.
	if (get_player() == SHEEP) {
		state += 0 * pow;
	}
	else if (get_player() == WOLF) {
		state += 1 * pow;
	}
	else {
		state += 2 * pow;
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
				board[x][y] = SHEEP;
			}
			else if (state % 3 == 1) {
				board[x][y] = WOLF;
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
		state /= 3;
		pow *= 3;
	}

	// Current player.
	if (state % 3 == 0) {
		set_player_(SHEEP);
	}
	else if (state % 3 == 1) {
		set_player_(WOLF);
	}
	else {
		set_player_(NONE);
	}
}

/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
void BaghChalGame::make_move_(const BaghChalMove &m) {
	if (is_first_phase() and get_player() == SHEEP) {
		make_sheep_placement_move_(m.ci);
	}
	else {
		// Moving.
		swap(board[m.ci.x][m.ci.y], board[m.cf.x][m.cf.y]);

		if (chebyshev_distance(m.ci, m.cf) == 2) { // Capturing.
			board[(m.ci.x + m.cf.x) / 2][(m.ci.y + m.cf.y) / 2] = NONE;
		}
	}
}

/* Returns a move inputed by the player. */
optional<BaghChalMove> BaghChalGame::get_player_move_(const string &command) const {
	int xi, yi, xf, yf;

	if (is_first_phase() and get_player() == SHEEP) {
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

	if (is_valid_move(BaghChalMove(xi, yi, xf, yf))) {
		return BaghChalMove(xi, yi, xf, yf);
	}

	return nullopt;
}

/* Returns all the possible moves for the current state of the game. */
vector<BaghChalMove> BaghChalGame::get_moves_() const {
	return get_moves_for_(get_player());
}

/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
double BaghChalGame::evaluate_() const {
	// Pretending that we are SHEEP.
	int stuck_wolves = stuck_wolves_count();
	int dead_sheep = 20 - sheep_count();
	return stuck_wolves * 0.04 - dead_sheep * 0.16;
}

/* ---------- PUBLIC ---------- */

BaghChalGame::BaghChalGame() {
	memset(board, NONE, sizeof(board));
	board[0][0] = board[0][N - 1] = board[N - 1][0] = board[N - 1][N - 1] = WOLF;
	sheeps = 20;
	Game<long long, BaghChalMove>::initialize_game_();
}

/* Returns true if the movement is valid. */
bool BaghChalGame::is_valid_move(const BaghChalMove &m) const {
	if (m.cf == BaghChalCell(-1, -1)) { // Sheep move during first phase.
		return is_valid_sheep_placement_move_(m);
	}

	if (!is_inside(m.ci) or !is_inside(m.cf)) { // Move out of bounds.
		return false;
	}

	if (board[m.ci.x][m.ci.y] != get_player()) { // Can't move a pawn that doesn't belong to the current player.
		return false;
	}

	if (board[m.cf.x][m.cf.y] != NONE) { // Can only move to empty cells.
		return false;
	}

	if ((m.ci.x + m.ci.y) % 2 == 0) { // 8 directions.
		if (chebyshev_distance(m.ci, m.cf) == 1) { // Non-capturing move.
			return true;
		}
	}
	else { // 4 directions.
		if (manhattan_distance(m.ci, m.cf) == 1) { // Non-capturing move.
			return true;
		}
	}

	// Sheep can only move 1 cell, treated above.
	if (get_player() == SHEEP) {
		return false;
	}

	int vx = m.cf.x - m.ci.x;
	int vy = m.cf.y - m.ci.y;
	int d = INV_DIR[1 + vx / 2][1 + vy / 2];
	BaghChalCell c = m.ci;

	// First step.
	c.x += DIR[d][0];
	c.y += DIR[d][1];

	// Checking if this is a sheep to be captured.
	if (!is_inside(c) or board[c.x][c.y] != SHEEP) {
		return false;
	}

	// Second step.
	c.x += DIR[d][0];
	c.y += DIR[d][1];

	// Checking if this is the final cell m.cf.
	return c == m.cf;
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
			if (board[x][y] == SHEEP) {
				mat[D * x][D * y] = 's';
			}
			else if (board[x][y] == WOLF) {
				mat[D * x][D * y] = 'w';
			}
			else {
				mat[D * x][D * y] = 'o';
			}
		}
	}

	// Highlighting.
	vector<BaghChalMove> moves = get_moves();

	for (const BaghChalMove &move : moves) {
		mat[D * move.ci.x][D * move.ci.y] = toupper(mat[D * move.ci.x][D * move.ci.y]);
	}

	// Filling the string.
	string str = "       ";

	// Y indexes.
	for (int y = 0; y < D * (N - 1) + 1; y++) {
		if (y % D == 0) {
			str += to_string(y / D) + " ";
		}
		else {
			str += "  ";
		}
	}

	str += "\n\n";

	for (int x = 0; x < D * (N - 1) + 1; x++) {
		// X indexes.
		if (x % D == 0) {
			str += "   " + to_string(x / D) + "   ";
		}
		else {
			str += "       ";
		}

		for (int y = 0; y < D * (N - 1) + 1; y++) {
			if (tolower(mat[x][y]) == 's') {
				if (mat[x][y] == 'S') {
					str += COLOR_BRIGHT_MAGENTA;
				}
				else {
					str += COLOR_RED;
				}

				str += "S ";
				str += COLOR_WHITE;
			}
			else if (tolower(mat[x][y]) == 'w') {
				if (mat[x][y] == 'W') {
					str += COLOR_MAGENTA;
				}
				else {
					str += COLOR_BLUE;
				}

				str += "W ";
				str += COLOR_WHITE;
			}
			else if (tolower(mat[x][y]) == 'o') {
				if (mat[x][y] == 'O') {
					str += COLOR_BRIGHT_MAGENTA;
				}
				else {
					str += COLOR_YELLOW;
				}

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

	str += "\n";
	str += "       Dead sheep: " + to_string(20 - sheep_count()) + "\n";

	return str;
}