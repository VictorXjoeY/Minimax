#pragma once

#include <chrono>
#include <limits>
#include <functional>
#include <optional>
#include <utility>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <Game.hpp>

using namespace std;

/* Minimax for games derived from Game class. */
template <class GameType, class StateHash = hash<typename GameType::state_type>, class StateEqualTo = equal_to<typename GameType::state_type>>
class Minimax {
public:
	using StateType = typename GameType::state_type;
	using MoveType = typename GameType::move_type;

	class OptimalMove {
	public:
		MoveType move = MoveType(); // The move itself
		double score = 0.0; // [-1, +1]
		bool is_solved = false; // True if one player can force a victory or if all states in this node's subtree have known results.
		optional<int> winner = nullopt; // Did someone win? Who? Max, Min or Draw (including infinite loop)?
		int turn = -1; // In which turn this move ends, relative to the start of the game.
		int height = -1; // How many moves ahead will I explore after the current move?

		OptimalMove() {}

		OptimalMove(const MoveType &move_, double score_, bool is_solved_, optional<int> winner_, int turn_, int height_) {
			#ifdef DEBUG
			assert(static_cast<double>(GameType::PLAYER_MIN) <= score_ and score_ <= static_cast<double>(GameType::PLAYER_MAX));
			#endif

			move = move_;
			score = score_;
			is_solved = is_solved_;
			winner = winner_;
			turn = turn_;
			height = height_;
		}
	};

private:
	static constexpr int DP_RESERVE = 1024 * 1024 * 1024 / (sizeof(StateType) + sizeof(OptimalMove)); // 1GB for dp
	static constexpr int IN_STACK_RESERVE = 1024 * 1024 / sizeof(StateType); // 1MB for in_stack

	unordered_map<StateType, OptimalMove> dp; // Dynamic Programming for already seen game states.
	unordered_set<StateType> in_stack; // Cycle detection.
	GameType game; // Game.

	/* Returns true if A is a better move than B for PLAYER_MAX. */
	bool better_max(const OptimalMove &a, const OptimalMove &b) {
		if (a.score != b.score) { // Take best score.
			return a.score > b.score;
		}

		if (a.score == static_cast<double>(GameType::PLAYER_MAX)) { // Already won, so take the shortest path.
			#ifdef DEBUG
			assert(a.is_solved and b.is_solved); // Best score is only possible when it has been solved.
			#endif
			return a.turn < b.turn;
		}

		if (a.is_solved == b.is_solved) { // Not a definitive victory and both are solved/unsolved, so take the longest path.
			return a.turn > b.turn;
		}

		if (a.score >= static_cast<double>(GameType::PLAYER_NONE)) { // Not losing, so prefer solved.
			return a.is_solved > b.is_solved;
		}
		
		// Losing, so prefer unsolved.
		return a.is_solved < b.is_solved;
	}

	/* Returns true if A is a better move than B for PLAYER_MIN. */
	bool better_min(const OptimalMove &a, const OptimalMove &b) {
		if (a.score != b.score) { // Take best score.
			return a.score < b.score;
		}

		if (a.score == static_cast<double>(GameType::PLAYER_MIN)) { // Already won, so take the shortest path.
			#ifdef DEBUG
			assert(a.is_solved and b.is_solved); // Best score is only possible when it has been solved.
			#endif
			return a.turn < b.turn;
		}

		if (a.is_solved == b.is_solved) { // Not a definitive victory and both are solved/unsolved, so take the longest path.
			return a.turn > b.turn;
		}

		if (a.score <= static_cast<double>(GameType::PLAYER_NONE)) { // Not losing, so prefer solved.
			return a.is_solved > b.is_solved;
		}
		
		// Losing, so prefer unsolved.
		return a.is_solved < b.is_solved;
	}

	/* Recursive function that runs the Minimax algorithm with alpha-beta pruning. */
	OptimalMove solve(double alpha, double beta, int height) {
		// Leaf node.
		if (game.is_game_over()) {
			return OptimalMove(MoveType(), game.get_winner().value(), true, game.get_winner(), game.get_turn(), 0);
		}

		vector<MoveType> moves = game.get_moves();

		// If we are trying to calculate a state which is still open (in stack) we have hit a cycle and we return 0.
		if (in_stack.count(game.get_state())) {
			return OptimalMove(moves[0], GameType::PLAYER_NONE, true, nullopt, numeric_limits<int>::max(), 0); // is_solved is true because there are no more possibilities for this "subtree".
		}

		// If we are trying to calculate a state which was explored further than it will be explored now then return its value.
		OptimalMove &ans = dp[game.get_state()];

		if (ans.is_solved or ans.height >= height) {
			return ans;
		}

		// If we are too deep then evaluate the board.
		if (height == 0) {
			return OptimalMove(moves[0], game.evaluate(), false, nullopt, game.get_turn(), 0);
		}

		// Marking the state as open.
		in_stack.insert(game.get_state());

		// Initializing with worst possible score.
		ans.score = 2.0 * game.get_enemy();

		for (const MoveType &move : moves) {
			// Recurse.
			game.make_move(move);
			OptimalMove ret = solve(alpha, beta, height - 1);
			game.rollback();

			if (game.get_player() == GameType::PLAYER_MAX) {
				// Alpha-beta pruning.
				alpha = max(alpha, ret.score);

				// Max.
				if (better_max(ret, ans)) {
					ans = ret;
					ans.move = move;
				}
			}
			else if (game.get_player() == GameType::PLAYER_MIN) {
				// Alpha-beta pruning.
				beta = min(beta, ret.score);
				
				// Min.
				if (better_min(ret, ans)) {
					ans = ret;
					ans.move = move;
				}
			}
			else {
				assert(false);
			}

			if (alpha == static_cast<double>(GameType::PLAYER_MAX) or beta == static_cast<double>(GameType::PLAYER_MIN) or beta <= alpha) {
				break;
			}
		}

		// Marking the state as closed.
		in_stack.erase(game.get_state());

		// The current answer is solved if the chosen move goes to a solved state.
		ans.height = height;

		return ans;
	}
public:
	Minimax() {
		dp.reserve(DP_RESERVE);
		in_stack.reserve(IN_STACK_RESERVE);
	}

	~Minimax() = default;

	/* Returns the best move obtained with minimax given a time limit in milliseconds. */
	pair<OptimalMove, int> get_move(const GameType &game_, chrono::duration<long double> timeout) {
		chrono::duration<long double> t;
		OptimalMove ans;

		// Timing.
		chrono::time_point<chrono::high_resolution_clock> t_start = chrono::high_resolution_clock::now();

		// Initializing.
		int max_depth = 0;
		game = game_;

		// Filling in_stack with states that were already seen.
		for (const StateType &state : game.get_states()) {
			in_stack.insert(state);
		}

		// But not the current state.
		in_stack.erase(game.get_state());

		// Iterative Deepening Search.
		do {
			ans = solve(2.0 * GameType::PLAYER_MIN, 2.0 * GameType::PLAYER_MAX, max_depth);
			max_depth++;
			t = chrono::high_resolution_clock::now() - t_start; // Time since start of function.
		} while (!ans.is_solved and t < timeout);

		// Let's not blow up my memory.
		if (dp.size() >= DP_RESERVE) {
			dp.clear();
		}

		// Returning optimal move.
		return {ans, max_depth - 1};
	}
};