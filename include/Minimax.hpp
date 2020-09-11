#pragma once

#include <cstdio>
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
		bool is_optimal = false; // True if one player can force a victory or if all states in this node's subtree have known results.
		optional<int> winner;
		int turn = -1; // In which turn this move ends, relative to the start of the game.
		int height = -1; // Height of the node of this move or number of steps ahead that were observed.

		OptimalMove() {}

		OptimalMove(const MoveType &move_, double score_, bool is_optimal_, optional<int> winner_, int turn_) {
			assert(static_cast<double>(PLAYER_MIN) <= score_ and score_ <= static_cast<double>(PLAYER_MAX));
			move = move_;
			score = score_;
			is_optimal = is_optimal_;
			winner = winner_;
			turn = turn_;
		}
	};

	static constexpr int PLAYER_MAX = GameType::PLAYER_MAX;
	static constexpr int PLAYER_MIN = GameType::PLAYER_MIN;
	static constexpr int PLAYER_NONE = GameType::PLAYER_NONE;

private:
	static constexpr int DP_RESERVE = 10000000;
	static constexpr int IN_STACK_RESERVE = 10000;

	GameType game;
	unordered_map<StateType, OptimalMove> dp;
	unordered_set<StateType> in_stack;

	/* Recursive function that runs the Minimax algorithm with alpha-beta pruning. */
	OptimalMove solve(int player, double alpha, double beta, int height) {
		if (game.is_game_over()) {
			return OptimalMove(MoveType(), game.get_winner().value(), true, game.get_winner(), game.get_turn());
		}

		const vector<MoveType>& moves = game.get_moves();

		// If we are trying to calculate a state which is still open (in stack) we have hit a cycle and we return 0.
		if (in_stack.count(game.get_state())) {
			return OptimalMove(moves[0], PLAYER_NONE, true, nullopt, game.get_turn()); // isOptimal is true because there are no more possibilities for this subtree.
		}

		// If we are trying to calculate a state which was already calculated higher up on the tree then return its value.
		OptimalMove &ans = dp[game.get_state()];

		if (ans.is_optimal or ans.height >= height) {
			return ans;
		}

		// If we are too deep then evaluate the board.
		if (height <= 0) {
			return OptimalMove(moves[0], game.evaluate(), false, nullopt, game.get_turn());
		}

		// Marking the state as open.
		in_stack.insert(game.get_state());
		int nonoptimal_count = 0;

		if (player == PLAYER_MAX) {
			ans.score = 2.0 * PLAYER_MIN;

			for (const MoveType &move : moves) {
				// Recurse.
				game.make_move(move);
				OptimalMove ret = solve(-player, alpha, beta, height - 1);
				game.rollback();

				// Counting the amount of times that we reach a nonoptimal solution.
				if (!ret.is_optimal) {
					nonoptimal_count++;
				}

				// Max.
				if (ret.score > ans.score or (ret.score == ans.score and ret.turn > ans.turn)) {
					ans = OptimalMove(move, ret.score, false, ret.winner, ret.turn);
				}

				// Alpha-beta pruning.
				alpha = max(alpha, ans.score);

				if (alpha == static_cast<double>(PLAYER_MAX) or beta == static_cast<double>(PLAYER_MIN) or beta <= alpha) {
					break;
				}
			}
		}
		else if (player == PLAYER_MIN) {
			ans.score = 2.0 * PLAYER_MAX;

			for (const MoveType &move : moves) {
				// Recurse.
				game.make_move(move);
				OptimalMove ret = solve(-player, alpha, beta, height - 1);
				game.rollback();

				// Counting the amount of times that we reach a nonoptimal solution.
				if (!ret.is_optimal) {
					nonoptimal_count++;
				}

				// Min.
				if (ret.score < ans.score or (ret.score == ans.score and ret.turn > ans.turn)) {
					ans = OptimalMove(move, ret.score, false, ret.winner, ret.turn);
				}

				// Alpha-beta pruning.
				beta = min(beta, ans.score);

				if (alpha == static_cast<double>(PLAYER_MAX) or beta == static_cast<double>(PLAYER_MIN) or beta <= alpha) {
					break;
				}
			}
		}
		else {
			assert(false);
		}

		// Marking the state as closed.
		in_stack.erase(game.get_state());

		// The answer is optimal if the current player won or if we don't hit any nonoptimal states.
		ans.is_optimal = ans.score == static_cast<double>(player) or nonoptimal_count == 0;
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
	pair<OptimalMove, int> get_move(const GameType &game_, long long timeout) {
		chrono::milliseconds t;
		OptimalMove ans;

		// Initializing.
		int max_depth = 0;
		game = game_;

		// Timing.
		chrono::time_point<chrono::high_resolution_clock> t_start = chrono::high_resolution_clock::now();

		// Iterative Deepening Search.
		do {
			ans = solve(game.get_player(), 2.0 * PLAYER_MIN, 2.0 * PLAYER_MAX, max_depth);
			max_depth++;
			t = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t_start); // Time since start of function.
		} while (!ans.is_optimal and t < chrono::milliseconds(timeout));

		// Let's not blow up my memory.
		if (dp.size() >= DP_RESERVE) {
			dp.clear();
		}

		// Returning optimal move.
		return {ans, max_depth - 1};
	}
};