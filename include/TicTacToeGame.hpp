#pragma once

#include <string>
#include <vector>

#include <Game.hpp>

class TicTacToeCell {
public:
	int x, y;

	TicTacToeCell() : x(-1), y(-1) {}
	TicTacToeCell(int x_, int y_) : x(x_), y(y_) {}

	bool operator == (const TicTacToeCell &c) const {
		return x == c.x and y == c.y;
	}

	bool operator != (const TicTacToeCell &c) const {
		return !(*this == c);
	}

	operator string() const {
		return "(" + to_string(x) + ", " + to_string(y) + ")";
	}
};

class TicTacToeMove : public GameMove {
public:
	TicTacToeCell c;

	TicTacToeMove() : c(TicTacToeCell()) {}
	TicTacToeMove(TicTacToeCell c_) : c(c_) {}
	TicTacToeMove(int x_, int y_) : TicTacToeMove(TicTacToeCell(x_, y_)) {}

	operator string() const override {
		return string(c);
	}
};

class TicTacToeGame : public Game<int, TicTacToeMove> {
public:
	/* Board is N x N. */
	static constexpr int N = 3;

	/* Cell state constants. */
	static constexpr int NONE = 0;
	static constexpr int CROSS = 1;
	static constexpr int CIRCLE = -1;

	/* Returns if the coordinate is inside the board. */
	static bool is_inside(const TicTacToeCell &c) {
		return 0 <= c.x and c.x < N and 0 <= c.y and c.y < N;
	}

	/* Returns the enemy color. */
	static int get_enemy(int p) {
		return -p;
	}

private:
	static char get_player_piece(int p) {
		if (p == CROSS) {
			return 'x';
		}

		if (p == CIRCLE) {
			return 'o';
		}

		return ' ';
	}

	int board[N][N];
	int player; // Current player.

	/* Toggles the current player. */
	void toggle_player();

	/* Checks if a row, column or diagonal has been filled. */
	bool is_game_over_() const;

protected:
	/* Returns the current game state converted to State. */
	int get_current_state_() const override;

	/* Initializes a new game. This function should be called at the end of initialize_game_() of the derived classes. */
	void initialize_game_() override;

	/* Loads the game given a State. */
	void load_game_(const int &state) override;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	void make_move_(const TicTacToeMove &m_) override;

public:
	TicTacToeGame() {
		initialize_game_();
	}

	/* Current player. */
	int get_current_player() const override;

	/* Returns if the move (x, y) is a valid move. */
	bool is_valid_move(const TicTacToeMove &m_) const override;

	/* Returns all the current possible moves. */
	vector<TicTacToeMove> get_moves() const override;

	/* Returns a move inputed by the player. */
	TicTacToeMove get_player_move() const override;

	/* Returns a value between -1 and 1 indicating how probable it is for the current player to win (1.0) or the other player to win (-1.0). */
	double evaluate() override;

	/* Returns if the game is over (current player can't make any more moves). */
	bool is_game_over() const override;

	/* Returns the board for printing. */
	operator string() const override;
};