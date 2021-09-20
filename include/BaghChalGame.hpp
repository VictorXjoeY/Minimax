#pragma once

#include <string>
#include <vector>
#include <optional>

#include <Game.hpp>

using namespace std;

class BaghChalCell {
public:
	int x, y;

	BaghChalCell() : x(-1), y(-1) {}
	BaghChalCell(int x_, int y_) : x(x_), y(y_) {}

	bool operator == (const BaghChalCell &c) const {
		return x == c.x and y == c.y;
	}

	bool operator != (const BaghChalCell &c) const {
		return !(*this == c);
	}

	operator string() const {
		return "(" + to_string(x) + ", " + to_string(y) + ")";
	}
};

class BaghChalMove : public GameMove {
public:
	BaghChalCell ci, cf;

	BaghChalMove() : ci(BaghChalCell()), cf(BaghChalCell()) {}
	BaghChalMove(BaghChalCell ci_, BaghChalCell cf_) : ci(ci_), cf(cf_) {}
	BaghChalMove(int xi, int yi, int xf, int yf) : BaghChalMove(BaghChalCell(xi, yi), BaghChalCell(xf, yf)) {}

	operator string() const override {
		if (cf == BaghChalCell(-1, -1)) {
			return string(ci);
		}

		return string(ci) + " -> " + string(cf);
	}
};

class BaghChalState : public GameState {
private:
	long long state_;

public:
	explicit BaghChalState(long long state) {
		state_ = state;
		hash_ = std::hash<long long>()(state_);
	}

	string serialize() const override {
		return to_string(state_);
	}

	static BaghChalState deserialize(const string &serialized_state) {
		return BaghChalState(stoll(serialized_state));
	}

	long long get() const {
		return state_;
	}

	bool operator==(const GameState &rhs) const override {
		return state_ == dynamic_cast<const BaghChalState &>(rhs).state_;
	}
};

template<>
struct std::hash<BaghChalState> : public std::hash<GameState> {
	using std::hash<GameState>::operator();
};

class BaghChalGame : public Game<BaghChalState, BaghChalMove> {
private:
	/* Circle has N cells and 1 in the center. */
	static constexpr int N = 5;
	static constexpr int D = 5;

	/* Cell state constants. */
	static constexpr int SHEEP = Game<BaghChalState, BaghChalMove>::PLAYER_MAX;
	static constexpr int WOLF = Game<BaghChalState, BaghChalMove>::PLAYER_MIN;
	static constexpr int NONE = Game<BaghChalState, BaghChalMove>::PLAYER_NONE;

	// UP, RIGHT, DOWN, LEFT, UPRIGHT, DOWNRIGHT, DOWNLEFT, UPLEFT
	static constexpr int UP = 0;
	static constexpr int RIGHT = 1;
	static constexpr int DOWN = 2;
	static constexpr int LEFT = 3;
	static constexpr int UPRIGHT = 4;
	static constexpr int DOWNRIGHT = 5;
	static constexpr int DOWNLEFT = 6;
	static constexpr int UPLEFT = 7;
	static constexpr int DIR[8][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}, {-1, 1}, {1, 1}, {1, -1}, {-1, -1}};
	static constexpr int INV_DIR[3][3] = {{UPLEFT, UP, UPRIGHT}, {LEFT, -1, RIGHT}, {DOWNLEFT, DOWN, DOWNRIGHT}};

	/* From (x, y) to position. */
	static int convert_cell(const BaghChalCell &c) {
		return N * c.x + c.y;
	}

	/* From position to (x, y). */
	static BaghChalCell convert_cell(int p) {
		return BaghChalCell(p / N, p % N);
	}

	/* Returns if the coordinate is inside the board. */
	static bool is_inside(int x, int y) {
		return 0 <= x and x < N and 0 <= y and y < N;
	}

	/* Returns if the coordinate is inside the board. */
	static bool is_inside(const BaghChalCell &c) {
		return is_inside(c.x, c.y);
	}

	/* Returns the Chebyshev Distance of two cells. */
	static int chebyshev_distance(const BaghChalCell &ci, const BaghChalCell &cf) {
		return max(abs(cf.x - ci.x), abs(cf.y - ci.y));
	}

	/* Returns the Manhattan Distance of two cells. */
	static int manhattan_distance(const BaghChalCell &ci, const BaghChalCell &cf) {
		return abs(cf.x - ci.x) + abs(cf.y - ci.y);
	}

	int board[N][N]; // Board.
	int sheeps; // Sheeps to be placed on the board.

	/* Returns true if SHEEP player is still placing sheeps on the board. */
	bool is_first_phase() const;

	/* Returns the number sheeps which are alive. */
	int sheep_count() const;

	/* Returns the number of wolves currently stuck. */
	int stuck_wolves_count() const;

	/* Auxiliar function for the string conversion. */
	void fill(char mat[D * (N - 1) + 1][D * (N - 1) + 1], int x, int y, int d) const;

	/* Returns true if its a valid first phase move. */
	bool is_valid_sheep_placement_move_(const BaghChalMove &) const;

	/* Performs a first phase move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	void make_sheep_placement_move_(const BaghChalCell &);

	/* Returns all the possible first phase moves for SHEEP. */
	vector<BaghChalMove> get_sheep_placement_moves_() const;

	/* Returns all the possible moves for the current state of the game. */
	vector<BaghChalMove> get_moves_for_(int) const;

protected:
	/* Returns the current game state converted to State. */
	BaghChalState get_state_() const override;

	/* Loads the game given a State. */
	void load_game_(const BaghChalState &) override;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	void make_move_(const BaghChalMove &) override;

	/* Returns a move inputed by the player. */
	optional<BaghChalMove> get_player_move_(const string &) const override;

	/* Returns all the possible moves for the current state of the game. */
	vector<BaghChalMove> get_moves_() const override;

	/* Returns the winner. */
	using Game<BaghChalState, BaghChalMove>::get_winner_;

	/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
	double evaluate_() const override;

public:
	BaghChalGame();
	BaghChalGame(const BaghChalState &);

	/* Returns true if the movement is valid. */
	bool is_valid_move(const BaghChalMove &) const override;

	/* Returns the board for printing. */
	operator string() const override;
};