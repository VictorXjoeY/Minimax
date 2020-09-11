#pragma once

#include <string>
#include <vector>
#include <optional>

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
private:
	/* Board is N x N. */
	static constexpr int N = 3;

	/* Cell state constants. */
	static constexpr int CROSS = Game<int, TicTacToeMove>::PLAYER_MAX;
	static constexpr int CIRCLE = Game<int, TicTacToeMove>::PLAYER_MIN;
	static constexpr int NONE = Game<int, TicTacToeMove>::PLAYER_NONE;

	/* Returns if the coordinate is inside the board. */
	static bool is_inside(const TicTacToeCell &c) {
		return 0 <= c.x and c.x < N and 0 <= c.y and c.y < N;
	}

	int board[N][N]; // Board.

	/* Checks if a row, column or diagonal has been filled. */
	bool has_someone_won_() const;

protected:
	/* Returns the current game state converted to State. */
	int get_state_() const override;

	/* Loads the game given a State. */
	void load_game_(const int &) override;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	void make_move_(const TicTacToeMove &) override;

	/* Returns a move inputed by the player. */
	optional<TicTacToeMove> get_player_move_(const string &) const override;

	/* Returns all the current possible moves. */
	vector<TicTacToeMove> get_moves_() const override;

	/* Returns the winner. */
	int get_winner_() const override;

public:
	TicTacToeGame();

	/* Returns if the move (x, y) is a valid move. */
	bool is_valid_move(const TicTacToeMove &) const override;

	/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
	using Game<int, TicTacToeMove>::evaluate;

	/* Returns the board for printing. */
	operator string() const override;
};