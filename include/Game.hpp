#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <optional>

using namespace std;

/* Base class for Game moves. */
class GameMove {
protected:
	GameMove() = default;

public:
	virtual ~GameMove() = default;

	virtual operator string() const = 0;
};

/* Base class for 2-player minimax games. */
template <class StateType, class MoveType>
class Game {
private:
	vector<StateType> states_stack; // Game state history.
	vector<vector<MoveType>> moves_stack; // Possible moves.
	vector<optional<int>> winner_stack; // Winner, if any.

protected:
	Game() = default;

	/* Returns the current game state converted to State. */
	virtual StateType get_state_() const = 0;

	/* Initializes a new game. This function should be called at the end of initialize_game_() of the derived classes. */
	virtual void initialize_game_() {
		states_stack.push_back(get_state_());
		moves_stack.push_back(get_moves_());
		winner_stack.push_back(is_game_over() ? optional(get_winner_()) : nullopt);
	}

	/* Loads the game given a State. */
	virtual void load_game_(const StateType &) = 0;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	virtual void make_move_(const MoveType &) = 0;

	/* Returns all the possible moves for the current state of the game. */
	virtual vector<MoveType> get_moves_() const = 0;

	/* Returns the winner. */
	virtual int get_winner_() const {
		return get_enemy();
	}

public:
	/* Using typedef to be able to retrieve State type and Move type after declaration. */
	using state_type = StateType;
	using move_type = MoveType;

	static constexpr int PLAYER_MAX = 1;
	static constexpr int PLAYER_MIN = -1;
	static constexpr int PLAYER_NONE = 0;

	virtual ~Game() = default;

	/* Current player. */
	int get_player() const {
		assert(!states_stack.empty()); // Game::initialize_game_() has to be called!

		if (states_stack.size() % 2 != 0) {
			return PLAYER_MAX;
		}

		return PLAYER_MIN;
	}

	/* Current enemy. */
	int get_enemy() const {
		return -get_player();
	}

	/* Returns the current state. */
	const StateType& get_state() const {
		assert(!states_stack.empty()); // Game::initialize_game_() has to be called!
		return states_stack.back();
	}

	/* Returns true if the movement is valid. */
	virtual bool is_valid_move(const MoveType &) const = 0;

	/* Returns all the possible moves for the current state of the game. */
	const vector<MoveType>& get_moves() const {
		return moves_stack.back();
	}

	/* Returns a move inputed by the player. */
	virtual MoveType get_player_move() const = 0;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Maybe remove assumption that is_valid_move(m) is true. */
	void make_move(const MoveType &m) {
		assert(!states_stack.empty()); // Game::initialize_game_() has to be called!
		make_move_(m);
		states_stack.push_back(get_state_());
		moves_stack.push_back(get_moves_());
		winner_stack.push_back(is_game_over() ? optional(get_winner_()) : nullopt);
	}

	/* Rolls back to the previous state of the game. */
	void rollback() {
		assert(!states_stack.empty()); // Game::initialize_game_() has to be called!
		if (states_stack.size() > 1) {
			states_stack.pop_back();
			moves_stack.pop_back();
			winner_stack.pop_back();
			load_game_(states_stack.back());
		}
	}

	/* Returns the turn number. */
	int get_turn() const {
		return states_stack.size();
	}

	/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
	virtual double evaluate() const {
		return 0.0;
	}

	/* Returns if the game is over (current player can't make any more moves). */
	bool is_game_over() const {
		return get_moves().empty();
	}

	/* Returns the winner, if any. */
	optional<int> get_winner() const {
		return winner_stack.back();
	}

	/* Returns the game for printing. */
	virtual operator string() const = 0;
};

