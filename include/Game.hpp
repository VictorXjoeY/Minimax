#pragma once

#include <cassert>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
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

/* Base class for Game states. */
class GameState {
protected:
	optional<size_t> hash_;

public:	
	virtual ~GameState() = default;

	virtual string serialize() const = 0;

	/* Enforce implementation of this static method */
	// virtual static GameState deserialize(const string &) = 0;

	virtual size_t hash() const final {
		#ifdef DEBUG
		assert(hash_.has_value());
		#endif
		return hash_.value();
	}

	virtual bool operator==(const GameState &rhs) const = 0;
};

template<>
struct std::hash<GameState> {
	size_t operator()(const GameState &game_state) const {
		return game_state.hash();
	}
};

/* Base class for 2-player minimax games. Override required methods and call Game::initialize_game_() at the END of the constructor. */
template <class StateType, class MoveType>
class Game {
private:
	vector<StateType> states_stack; // Game state history.
	vector<vector<MoveType>> moves_stack; // Possible moves.
	vector<optional<int>> winner_stack; // Winner, if any.
	int player_; // Current player.

protected:
	Game() = default;

	/* Enforce the implementation of this constructor. Make it call load_game_(state) and then initialize_game()_. */ 
	// Game(const StateType &);

	/* Returns the current game state converted to State. */
	virtual StateType get_state_() const = 0;

	/* Loads the game given a State. */
	virtual void load_game_(const StateType &) = 0;

	/* Performs a move. is_valid_move(m) should be true. */
	virtual void make_move_(const MoveType &) = 0;

	/* Returns a move inputed by the player. */
	virtual optional<MoveType> get_player_move_(const string &) const = 0;

	/* Returns all the possible moves for the current state of the game. */
	virtual vector<MoveType> get_moves_() const = 0;

	/* Returns the winner. */
	virtual int get_winner_() const {
		return get_enemy();
	}

	/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
	virtual double evaluate_() const {
		return 0.0;
	}

	/* ---------- FINAL ---------- */

	/* Initializes a new game. This function should be called at the end of initialize_game_() of the derived classes. */
	virtual void initialize_game_() final;

	/* Sets the current player. */
	virtual void set_player_(int) final;

public:
	/* Using typedef to be able to retrieve State type and Move type after declaration. */
	using state_type = StateType;
	using move_type = MoveType;

	/* Player constants. */
	static constexpr int PLAYER_MAX = 1;
	static constexpr int PLAYER_MIN = -1;
	static constexpr int PLAYER_NONE = 0;

	virtual ~Game() = default;

	/* Returns true if the movement is valid. */
	virtual bool is_valid_move(const MoveType &) const = 0;

	/* Returns the game for printing. */
	virtual operator string() const = 0;

	/* ---------- FINAL ---------- */

	/* Current player. */
	virtual int get_player() const final;

	/* Current enemy. */
	virtual int get_enemy() const final;

	/* Returns the current state. */
	virtual const StateType& get_state() const final;

	/* Returns a move inputed by the player. */
	virtual optional<MoveType> get_player_move(const string &) const final;

	/* Returns all the possible moves for the current state of the game. */
	virtual const vector<MoveType>& get_moves() const final;

	/* Performs a move. Assumes that is_valid_move(m) is true. */
	virtual void make_move(const MoveType &m) final;

	/* Rolls back to the previous state of the game. */
	virtual void rollback() final;

	/* Returns the turn number. */
	virtual int get_turn() const final;

	/* Returns if the game is over (current player can't make any more moves). */
	virtual bool is_game_over() const final;

	/* Returns the winner, if any. */
	virtual optional<int> get_winner() const final;

	/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
	virtual double evaluate() const final;
};

/* ---------- PROTECTED ---------- */

/* Initializes a new game. This function should be called at the end of initialize_game_() of the derived classes. */
template <class StateType, class MoveType>
void Game<StateType, MoveType>::initialize_game_() {
	states_stack.push_back(get_state_());
	moves_stack.push_back(get_moves_());
	winner_stack.push_back(is_game_over() ? optional(get_winner_()) : nullopt);
}

/* Sets the current player. */
template <class StateType, class MoveType>
void Game<StateType, MoveType>::set_player_(int player) {
	player_ = player;
}

/* ---------- PUBLIC ---------- */

/* Current player. */
template <class StateType, class MoveType>
int Game<StateType, MoveType>::get_player() const {
	return player_;
}

/* Current enemy. */
template <class StateType, class MoveType>
int Game<StateType, MoveType>::get_enemy() const {
	return -get_player();
}

/* Returns the current state. */
template <class StateType, class MoveType>
const StateType& Game<StateType, MoveType>::get_state() const {
	return states_stack.back();
}

/* Returns a move inputed by the player. */
template <class StateType, class MoveType>
optional<MoveType> Game<StateType, MoveType>::get_player_move(const string &command) const {
	if (command.empty()) {
		if (moves_stack.back().size() == 1) {
			return moves_stack.back().back();
		}

		return nullopt;
	}

	return get_player_move_(command);
}

/* Returns all the possible moves for the current state of the game. */
template <class StateType, class MoveType>
const vector<MoveType>& Game<StateType, MoveType>::get_moves() const {
	return moves_stack.back();
}

/* Performs a move. Assumes that is_valid_move(m) is true. */
template <class StateType, class MoveType>
void Game<StateType, MoveType>::make_move(const MoveType &m) {
	#ifdef DEBUG
	assert(!states_stack.empty()); // Game::initialize_game_() has to be called!
	assert(is_valid_move(m));
	#endif

	make_move_(m);

	// Updating Game state.
	player_ = get_enemy();
	states_stack.push_back(get_state_());
	moves_stack.push_back(get_moves_());
	winner_stack.push_back(is_game_over() ? optional(get_winner_()) : nullopt);
}

/* Rolls back to the previous state of the game. */
template <class StateType, class MoveType>
void Game<StateType, MoveType>::rollback() {
	#ifdef DEBUG
	assert(!states_stack.empty()); // Game::initialize_game_() has to be called!
	#endif

	if (states_stack.size() > 1) {
		// Updating Game state.
		winner_stack.pop_back();
		moves_stack.pop_back();
		states_stack.pop_back();

		load_game_(states_stack.back());
	}
}

/* Returns the turn number. */
template <class StateType, class MoveType>
int Game<StateType, MoveType>::get_turn() const {
	return states_stack.size();
}

/* Returns if the game is over (current player can't make any more moves). */
template <class StateType, class MoveType>
bool Game<StateType, MoveType>::is_game_over() const {
	return get_moves().empty();
}

/* Returns the winner, if any. */
template <class StateType, class MoveType>
optional<int> Game<StateType, MoveType>::get_winner() const {
	return winner_stack.back();
}

/* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
template <class StateType, class MoveType>
double Game<StateType, MoveType>::evaluate() const {
	double score_max = static_cast<double>(PLAYER_MAX);
	double score_min = static_cast<double>(PLAYER_MIN);
	double score_none = static_cast<double>(PLAYER_NONE);

	if (is_game_over()) {
		if (get_winner() == PLAYER_MAX) {
			return score_max;
		}

		if (get_winner() == PLAYER_MIN) {
			return score_min;
		}

		return score_none;
	}

	return clamp(evaluate_(), nextafter(score_min, score_max), nextafter(score_max, score_min));
}
