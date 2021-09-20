#pragma once

#include <string>
#include <vector>
#include <optional>

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

class MuTorereState : public GameState {
private:
	int state_;

public:
	explicit MuTorereState(int state) {
		state_ = state;
		hash_ = std::hash<int>()(state_);
	}

	string serialize() const override {
		return to_string(state_);
	}

	static MuTorereState deserialize(const string &serialized_state) {
		return MuTorereState(stoi(serialized_state));
	}

	int get() const {
		return state_;
	}

	bool operator==(const GameState &rhs) const override {
		return state_ == dynamic_cast<const MuTorereState &>(rhs).state_;
	}
};

template<>
struct std::hash<MuTorereState> : public std::hash<GameState> {
	using std::hash<GameState>::operator();
};

class MuTorereGame : public Game<MuTorereState, MuTorereMove> {
	/* Circle has N cells and 1 in the center. */
	static constexpr int N = 8;

	/* Cell state constants. */
	static constexpr int WHITE = Game<MuTorereState, MuTorereMove>::PLAYER_MAX;
	static constexpr int BLACK = Game<MuTorereState, MuTorereMove>::PLAYER_MIN;
	static constexpr int NONE = Game<MuTorereState, MuTorereMove>::PLAYER_NONE;

	int board[N + 1]; // 0 through 7 belong to the circle. 8 is the center.

	/* Returns the position which is empty. */
	int get_empty_position();

protected:
	/* Returns the current game state converted to State. */
	MuTorereState get_state_() const override;

	/* Loads the game given a State. */
	void load_game_(const MuTorereState &) override;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	void make_move_(const MuTorereMove &) override;

	/* Returns a move inputed by the player. */
	optional<MuTorereMove> get_player_move_(const string &) const override;

	/* Returns all the possible moves for the current state of the game. */
	vector<MuTorereMove> get_moves_() const override;

	/* Returns the winner. */
	using Game<MuTorereState, MuTorereMove>::get_winner_;

public:
	MuTorereGame();
	MuTorereGame(const MuTorereState &);

	/* Returns true if the movement is valid. */
	bool is_valid_move(const MuTorereMove &) const override;

	/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
	using Game<MuTorereState, MuTorereMove>::evaluate;

	/* Returns the board for printing. */
	operator string() const override;
};