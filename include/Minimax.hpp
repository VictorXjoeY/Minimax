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
		MoveType move = MoveType();
		double score = 0.0; // [-1, +1]
		bool is_solved = false; // True if one player can force a victory or if all states in this node's subtree have known results.
		optional<int> winner;
		int turn = -1; // In which turn this move ends, relative to the start of the game.

		OptimalMove() {}

		OptimalMove(const MoveType &move_, double score_, bool is_solved_, optional<int> winner_, int turn_) {
			#ifdef DEBUG
			assert(static_cast<double>(GameType::PLAYER_MIN) <= score_ and score_ <= static_cast<double>(GameType::PLAYER_MAX));
			#endif

			move = move_;
			score = score_;
			is_solved = is_solved_;
			winner = winner_;
			turn = turn_;
		}
	};

private:
	static constexpr int DP_RESERVE = 10000000;
	static constexpr int IN_STACK_RESERVE = 10000;

	unordered_map<StateType, OptimalMove> dp; // Dynamic Programming for already seen game states.
	unordered_set<StateType> in_stack; // Cycle detection.
	vector<OptimalMove> move_history; // Moves returned.
	GameType game; // Game.

	/* Recursive function that runs the Minimax algorithm with alpha-beta pruning. */
	OptimalMove solve(double alpha, double beta, int height) {
		// Leaf node.
		if (game.is_game_over()) {
			return OptimalMove(MoveType(), game.get_winner().value(), true, game.get_winner(), game.get_turn());
		}

		vector<MoveType> moves = game.get_moves();
		assert(!moves.empty());

		// If we are trying to calculate a state which is still open (in stack) we have hit a cycle and we return 0.
		if (in_stack.count(game.get_state())) {
			return OptimalMove(moves[0], GameType::PLAYER_NONE, true, nullopt, game.get_turn()); // isOptimal is true because there are no more possibilities for this subtree.
		}

		// If we are trying to calculate a state which was already calculated higher up on the tree then return its value.
		OptimalMove &ans = dp[game.get_state()];

		if (ans.is_solved or ans.turn >= game.get_turn() + height) {
			return ans;
		}

		// If we are too deep then evaluate the board.
		if (height <= 0) {
			return OptimalMove(moves[0], game.evaluate(), false, nullopt, game.get_turn());
		}

		// Marking the state as open.
		in_stack.insert(game.get_state());

		// Initializing with worst possible score.
		ans.score = 2.0 * game.get_enemy();
		int non_solved_count = 0;

		for (const MoveType &move : moves) {
			// Recurse.
			game.make_move(move);
			OptimalMove ret = solve(alpha, beta, height - 1);
			game.rollback();

			// Counting the amount of times that we reach a nonoptimal solution.
			if (!ret.is_solved) {
				non_solved_count++;
			}

			if (game.get_player() == GameType::PLAYER_MAX) {
				// Max.
				if (ret.score > ans.score or (ret.score == ans.score and ret.turn > ans.turn)) {
					ans = OptimalMove(move, ret.score, false, ret.winner, ret.turn);
				}

				// Alpha-beta pruning.
				alpha = max(alpha, ans.score);
			}
			else if (game.get_player() == GameType::PLAYER_MIN) {
				// Min.
				if (ret.score < ans.score or (ret.score == ans.score and ret.turn > ans.turn)) {
					ans = OptimalMove(move, ret.score, false, ret.winner, ret.turn);
				}

				// Alpha-beta pruning.
				beta = min(beta, ans.score);
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

		// The answer is optimal if the current player won or if we don't hit any nonoptimal states.
		ans.is_solved = ans.score == static_cast<double>(game.get_player()) or non_solved_count == 0;

		return ans;
	}
public:
	Minimax() {
		dp.reserve(DP_RESERVE);
		in_stack.reserve(IN_STACK_RESERVE);
	}

	~Minimax() = default;

	/* Returns the best move obtained with minimax given a time limit in milliseconds. */
	pair<OptimalMove, int> get_move(const GameType &game_, chrono::milliseconds timeout) {
		chrono::milliseconds t;
		OptimalMove ans;

		// Initializing.
		int max_depth = 0;
		game = game_;

		// Timing.
		chrono::time_point<chrono::high_resolution_clock> t_start = chrono::high_resolution_clock::now();

		// Iterative Deepening Search.
		do {
			ans = solve(2.0 * GameType::PLAYER_MIN, 2.0 * GameType::PLAYER_MAX, max_depth);
			max_depth++;
			t = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t_start); // Time since start of function.
		} while (!ans.is_solved and t < timeout);

		// Let's not blow up my memory.
		if (dp.size() >= DP_RESERVE) {
			dp.clear();
		}

		// TODO: Fix BaghChal dumb sheep bug
		#ifdef DEBUG
		static bool crash_next_turn = false;

		if (crash_next_turn) {
			assert(false);
		}

		if (!move_history.empty() and !move_history.back().is_solved and ans.is_solved) {
			if (move_history.back().turn >= ans.turn) {
				crash_next_turn = true;
			}
		}

		move_history.push_back(ans);
		#endif

		// Returning optimal move.
		return {ans, max_depth - 1};
	}
};