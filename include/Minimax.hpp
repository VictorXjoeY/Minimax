#pragma once

#include <cstdio>
#include <chrono>
#include <limits>
#include <functional>
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
		double winner = 0.0; // [-1, +1]
		bool is_optimal = false;
		int turn = -1;
		int height = -1;

		OptimalMove() {}

		OptimalMove(const MoveType &move_, double winner_, bool is_optimal_, int turn_) {
			move = move_;
			winner = winner_;
			is_optimal = is_optimal_;
			turn = turn_;
		}
	};

	static constexpr double PLAYER_MAX = 1.0;
	static constexpr double PLAYER_MIN = -1.0;

private:
	static constexpr int DP_RESERVE = 10000000;
	static constexpr int IN_STACK_RESERVE = 10000;

	GameType game;
	unordered_map<StateType, OptimalMove> dp;
	unordered_set<StateType> in_stack;
	int first_player;

	/* Recursive function that runs the Minimax algorithm with alpha-beta pruning. */
	OptimalMove solve(double player, double alpha, double beta, int height) {
		vector<MoveType> moves = game.get_moves();

		if (moves.empty()) { // TODO: Replace this with game.is_over(). Replace -player with game.winner() (or similar)
			return OptimalMove(MoveType(), -player, true, game.get_turn());
		}

		// If we are trying to calculate a state which is still open (in stack) we have hit a cycle and we return 0.
		if (in_stack.count(game.get_current_state())) {
			return OptimalMove(moves[0], 0.0, true, game.get_turn()); // isOptimal is true because there are no more possibilities for this subtree.
		}

		// If we are trying to calculate a state which was already calculated higher up on the tree then return its value.
		OptimalMove &ans = dp[game.get_current_state()];

		if (ans.is_optimal or ans.height >= height) {
			return ans;
		}

		// If we are too deep then evaluate the board.
		if (height <= 0) {
			return OptimalMove(moves[0], game.evaluate(), false, game.get_turn());
		}

		// Marking the state as open.
		in_stack.insert(game.get_current_state());
		int nonoptimal_count = 0;

		if (player == PLAYER_MAX) {
			ans.winner = 2.0 * PLAYER_MIN;

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
				if (ret.winner > ans.winner or (ret.winner == ans.winner and ret.turn > ans.turn)) {
					ans = OptimalMove(move, ret.winner, false, ret.turn);
				}

				// Alpha-beta pruning.
				alpha = max(alpha, ans.winner);

				if (alpha == PLAYER_MAX or beta == PLAYER_MIN or beta <= alpha) {
					break;
				}
			}
		}
		else if (player == PLAYER_MIN) {
			ans.winner = 2.0 * PLAYER_MAX;

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
				if (ret.winner < ans.winner or (ret.winner == ans.winner and ret.turn > ans.turn)) {
					ans = OptimalMove(move, ret.winner, false, ret.turn);
				}

				// Alpha-beta pruning.
				beta = min(beta, ans.winner);

				if (alpha == PLAYER_MAX or beta == PLAYER_MIN or beta <= alpha) {
					break;
				}
			}
		}
		else {
			assert(false);
		}

		// Marking the state as closed.
		in_stack.erase(game.get_current_state());

		// The answer is optimal if the current player won or if we don't hit any nonoptimal states.
		ans.is_optimal = ans.winner == player or nonoptimal_count == 0;
		ans.height = height;

		return ans;
	}
public:
	Minimax() {
		dp.reserve(DP_RESERVE);
		in_stack.reserve(IN_STACK_RESERVE);
		first_player = GameType().get_current_player();
	}

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
			ans = solve(first_player == game.get_current_player() ? PLAYER_MAX : PLAYER_MIN, 2.0 * PLAYER_MIN, 2.0 * PLAYER_MAX, max_depth);
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