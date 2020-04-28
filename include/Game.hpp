#pragma once

#include <cassert>
#include <string>
#include <vector>

using namespace std;

/* Base class for Game moves. */
class GameMove {
public:
	virtual operator string() const = 0;
};

/* Base class for 2-player minimax games. */
template <class StateType, class MoveType>
class Game {
private:
	vector<StateType> states;

protected:
	/* Returns the current game state converted to State. */
	virtual StateType get_current_state_() const = 0;

	/* Initializes a new game. This function should be called at the end of initialize_game_() of the derived classes. */
	virtual void initialize_game_() {
		states.push_back(get_current_state_());
	}

	/* Loads the game given a State. */
	virtual void load_game_(const StateType &) = 0;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	virtual void make_move_(const MoveType &) = 0;

public:
	/* Using typedef to be able to retrieve State type and Move type after declaration. */
	using state_type = StateType;
	using move_type = MoveType;

	/* Current player. */
	virtual int get_current_player() const = 0;

	/* Returns all the states. */
	vector<StateType> get_states() const {
		assert(!states.empty()); // Game::initialize_game_() has to be called!
		return states;
	}

	/* Returns the current state. */
	StateType get_current_state() const {
		assert(!states.empty()); // Game::initialize_game_() has to be called!
		return states.back();
	}

	/* Returns true if the movement is valid. */
	virtual bool is_valid_move(const MoveType &) const = 0;

	/* Returns all the possible moves for the current state of the game. */
	virtual vector<MoveType> get_moves() const = 0;

	/* Returns a move inputed by the player. */
	virtual MoveType get_player_move() const = 0;

	/* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
	void make_move(const MoveType &m) {
		assert(!states.empty()); // Game::initialize_game_() has to be called!
		make_move_(m);
		states.push_back(get_current_state_());
	}

	/* Rolls back to the previous state of the game. */
	void rollback() {
		assert(!states.empty()); // Game::initialize_game_() has to be called!
		if (states.size() > 1) {
			states.pop_back();
			load_game_(states.back());
		}
	}

	/* Returns the turn number. */
	int get_turn() const {
		return states.size();
	}

	/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
	virtual double evaluate() = 0;

	/* Returns if the game is over (current player can't make any more moves). */
	virtual bool is_game_over() const = 0;

	/* Returns the game for printing. */
	virtual operator string() const = 0;
};

