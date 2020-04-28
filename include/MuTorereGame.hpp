#pragma once

#include <string>
#include <vector>

#include <Game.hpp>

using namespace std;

class MuTorereMove : public GameMove {
public:
	int pos;

	MuTorereMove() {
		pos = -1;
	}

	MuTorereMove(int pos_) {
		pos = pos_;
	}

	operator string() const override {
		return "(" + to_string(pos) + ")";
	}
};

class MuTorereGame : public Game<int, MuTorereMove> {
public:
	/* Circle has N cells and 1 in the center. */
	static constexpr int N = 8;

	/* Cell state constants. */
	static constexpr int NONE = 0;
	static constexpr int WHITE = 1;
	static constexpr int BLACK = -1;

	/* Returns the enemy color. */
	static int get_enemy(int p) {
		return -p;
	}

private:
	int board[N + 1]; // 0 through 7 belong to the circle. 8 is the center.
	int player; // Current player.

	/* Toggles the current player. */
	void toggle_player();

	/* Returns the position which is empty. */
	int get_empty_position();

protected:
	/* Returns the current game state converted to State. */
	int get_current_state_() const override;

	/* Initializes a new game. This function should be called at the end of initialize_game_() of the derived classes. */
	void initialize_game_() override;

	/* Loads the game given a State. */
	void load_game_(const int &) override;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	void make_move_(const MuTorereMove &) override;

public:
	MuTorereGame() {
		initialize_game_();
	}

	/* Current player. */
	int get_current_player() const override;

	/* Returns true if the movement is valid. */
	bool is_valid_move(const MuTorereMove &) const override;

	/* Returns all the possible moves for the current state of the game. */
	vector<MuTorereMove> get_moves() const override;

	/* Returns a move inputed by the player. */
	MuTorereMove get_player_move() const override;

	/* Returns a value between -1 and 1 indicating how probable it is for the current player to win (1.0) or the other player to win (-1.0). */
	double evaluate() override;

	/* Returns if the game is over (current player can't make any more moves). */
	bool is_game_over() const override;

	/* Returns the board for printing. */
	operator string() const override;
};