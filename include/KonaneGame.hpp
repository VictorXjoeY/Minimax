#pragma once

#include <string>
#include <vector>

#include <Game.hpp>

class KonaneCell {
public:
	int x, y;

	KonaneCell() {
		x = y = -1;
	}

	KonaneCell(int x_, int y_) {
		x = x_;
		y = y_;
	}

	bool operator == (const KonaneCell &c) const {
		return x == c.x and y == c.y;
	}

	bool operator != (const KonaneCell &c) const {
		return !(*this == c);
	}

	operator string() const {
		return "(" + to_string(x) + ", " + to_string(y) + ")";
	}
};

class KonaneMove : public GameMove {
public:
	KonaneCell ci, cf;

	KonaneMove() {
		ci = cf = KonaneCell();
	}

	KonaneMove(KonaneCell ci_, KonaneCell cf_) {
		ci = ci_;
		cf = cf_;
	}

	KonaneMove(int xi, int yi, int xf, int yf) : KonaneMove(KonaneCell(xi, yi), KonaneCell(xf, yf)) {}

	operator string() const override {
		if (cf == KonaneCell(-1, -1)) {
			return string(ci);
		}

		return string(ci) + " -> " + string(cf);
	}
};

class KonaneGame : public Game<long long, KonaneMove> {
public:
	/* Board is N x N. */
	static constexpr int N = 6;

	/* Cell state constants. */
	static constexpr int NONE = 0;
	static constexpr int WHITE = 1;
	static constexpr int BLACK = -1;

	/* Returns if the coordinate is inside the board. */
	static bool is_inside(const KonaneCell &c) {
		return 0 <= c.x and c.x < N and 0 <= c.y and c.y < N;
	}

	/* Returns the enemy color. */
	static int get_enemy(int p) {
		return -p;
	}

private:
	/* Direction constants. */
	static constexpr int UP = 0;
	static constexpr int DOWN = 1;
	static constexpr int LEFT = 2;
	static constexpr int RIGHT = 3;
	static constexpr int DIR[2][4] = {{-1, 1, 0, 0}, {0, 0, -1, 1}};

	/* From (x, y) to position. */
	static int convert_cell(const KonaneCell &c) {
		return N * c.x + c.y;
	}

	/* From position to (x, y). */
	static KonaneCell convert_cell(int p) {
		return KonaneCell(p / N, p % N);
	}

	long long board; // Board.

	/* Returns the number of pawns remaining on the board. */
	int count_pawns() const;

	/* Returns what is in cell (x, y). */
	int test(const KonaneCell &) const;

	/* Sets cell (x, y). */
	void set(const KonaneCell &);

	/* Resets cell (x, y). */
	void reset(const KonaneCell &);

	/* Toggles the current player. */
	void toggle_player();

	/* Returns if the move (x, y) at the start of the game is a valid move. */
	bool is_valid_starting_move(const KonaneCell &) const;

	/* Makes the move (x, y) for the first two moves in the game. Assumes that isValidMove(x, y) is true. */
	void make_starting_move(const KonaneMove &);

	/* Returns all the possible start moves. */
	vector<KonaneMove> get_starting_moves() const;

protected:
	/* Returns the current game state converted to State. */
	long long get_current_state_() const override;

	/* Initializes a new game. This function should be called at the end of initialize_game_() of the derived classes. */
	void initialize_game_() override;

	/* Loads the game given a State. */
	void load_game_(const long long &state) override;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	void make_move_(const KonaneMove &m_) override;

public:
	KonaneGame() {
		initialize_game_();
	}

	/* Current player. */
	int get_current_player() const override;

	/* Returns true if the next move is the first move of the match. */
	bool is_first_turn() const;

	/* Returns true if the next move is the second move of the match. */
	bool is_second_turn() const;

	/* Returns if the move (xi, yi) -> (xf, yf) is a valid move. */
	bool is_valid_move(const KonaneMove &m_) const override;

	/* Returns all the current possible moves. */
	vector<KonaneMove> get_moves() const override;

	/* Returns a move inputed by the player. */
	KonaneMove get_player_move() const override;

	/* Returns a value between -1 and 1 indicating how probable it is for the current player to win (1.0) or the other player to win (-1.0). */
	double evaluate() override;

	/* Returns if the game is over (current player can't make any more moves). */
	bool is_game_over() const override;

	/* Returns the board for printing. */
	operator string() const override;
};