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
template <class GameType>
class Minimax {
public:
	using StateType = typename GameType::state_type;
	using MoveType = typename GameType::move_type;

	class OptimalMove {
	public:
		MoveType move = MoveType(); // The move itself
		double score = 0.0; // [-1, +1]
		optional<int> winner = nullopt; // Is it solved? Who wins? Max, Min or Draw?
		int turn = -1; // In which turn this move ends, relative to the start of the game.
		bool pruned = false; // True if the other moves were not considered due to pruning, implying that the results in this OptimalMove are unreliable.
		bool hits_cycle = false; // (CURRENTLY UNUSED) True if movement can end up hitting a previously seen game state.

		OptimalMove() {}

		OptimalMove(const MoveType &move_, double score_, optional<int> winner_, int turn_) {
			move = move_;
			score = score_;
			winner = winner_;
			turn = turn_;
		}
	};

private:
	GameType game; // Game.
	long long previous_depths_move_count, next_depth_move_count; // Used for estimating the time cost of Minimax::solve

	/* Returns true if A is a better move than B for PLAYER_MAX. */
	bool better_max(const OptimalMove &a, const OptimalMove &b) {
		if (a.score != b.score) { // Take best score.
			return a.score > b.score;
		}

		if (a.pruned != b.pruned) { // Prioritize reliable moves.
			return a.pruned < b.pruned;
		}

		if (a.score == static_cast<double>(GameType::PLAYER_MAX)) { // Already won, so take the shortest path.
			return a.turn < b.turn;
		}

		if (a.score == static_cast<double>(GameType::PLAYER_MIN)) { // Already lost, so take the longest path.
			return a.turn > b.turn;
		}

		if (a.score >= static_cast<double>(GameType::PLAYER_NONE)) { // Not losing, so prefer solved.
			return a.winner.has_value() > b.winner.has_value();
		}
		
		// Losing, so prefer unsolved.
		return a.winner.has_value() < b.winner.has_value();
	}

	/* Returns true if A is a better move than B for PLAYER_MIN. */
	bool better_min(const OptimalMove &a, const OptimalMove &b) {
		if (a.score != b.score) { // Take best score.
			return a.score < b.score;
		}

		if (a.pruned != b.pruned) { // Prioritize reliable moves.
			return a.pruned < b.pruned;
		}

		if (a.score == static_cast<double>(GameType::PLAYER_MIN)) { // Already won, so take the shortest path.
			return a.turn < b.turn;
		}

		if (a.score == static_cast<double>(GameType::PLAYER_MAX)) { // Already lost, so take the longest path.
			return a.turn > b.turn;
		}

		if (a.score <= static_cast<double>(GameType::PLAYER_NONE)) { // Not losing, so prefer solved.
			return a.winner.has_value() > b.winner.has_value();
		}
		
		// Losing, so prefer unsolved.
		return a.winner.has_value() < b.winner.has_value();
	}

	/* Recursive function that runs the Minimax algorithm with alpha-beta pruning. */
	OptimalMove solve(double alpha, double beta, int height) {
		// Leaf node.
		if (game.is_game_over()) {
			return OptimalMove(MoveType(), game.get_winner().value(), game.get_winner().value(), game.get_turn());
		}

		const vector<MoveType> moves = game.get_moves();

		// If we are too deep then evaluate the board.
		if (height == 0) {
			next_depth_move_count += moves.size();
			return OptimalMove(moves[0], game.evaluate(), nullopt, game.get_turn());
		}

		previous_depths_move_count += moves.size();

		// Initializing with worst possible score.
		OptimalMove ans;
		ans.score = 2.0 * game.get_enemy();

		for (int i = 0; i < moves.size(); i++) {
			// Recurse.
			game.make_move(moves[i]);
			OptimalMove ret = solve(alpha, beta, height - 1);
			game.rollback();

			if (game.get_player() == GameType::PLAYER_MAX) {
				// Alpha-beta pruning.
				alpha = max(alpha, ret.score);

				// Max.
				if (better_max(ret, ans)) {
					ans = ret;
					ans.move = moves[i];
				}
			}
			else if (game.get_player() == GameType::PLAYER_MIN) {
				// Alpha-beta pruning.
				beta = min(beta, ret.score);
				
				// Min.
				if (better_min(ret, ans)) {
					ans = ret;
					ans.move = moves[i];
				}
			}
			else {
				assert(false);
			}

			if (alpha == static_cast<double>(GameType::PLAYER_MAX) or beta == static_cast<double>(GameType::PLAYER_MIN) or beta <= alpha) {
				ans.pruned = ans.pruned or i < moves.size() - 1; // Pruned if not all moves were considered.
				break;
			}
		}

		return ans;
	}
public:
	Minimax() = default;
	~Minimax() = default;

	/* Returns the best move obtained with minimax given a time limit in milliseconds. */
	pair<OptimalMove, int> get_move(const GameType &game_, chrono::duration<long double> timeout) {
		chrono::time_point<chrono::high_resolution_clock> get_move_start_time_point, previous_solve_start_time_point;
		chrono::duration<long double> total_time, last_solve_time, next_solve_time;
		OptimalMove prev_ans, cur_ans;

		// Timing.
		get_move_start_time_point = chrono::high_resolution_clock::now();

		// Initializing.
		int max_depth = 0;
		game = game_;

		// Iterative Deepening Search.
		do {
			// Calling solve.
			previous_solve_start_time_point = chrono::high_resolution_clock::now();
			previous_depths_move_count = next_depth_move_count = 0;
			prev_ans = cur_ans;
			cur_ans = solve(2.0 * GameType::PLAYER_MIN, 2.0 * GameType::PLAYER_MAX, max_depth++);
			last_solve_time = chrono::high_resolution_clock::now() - previous_solve_start_time_point;

			// Predicting how much it will take for another solve call.
			if (previous_depths_move_count == 0) {
				next_solve_time = 0.0s;
			}
			else {
				double average_branching_factor = static_cast<double>(previous_depths_move_count + next_depth_move_count) / static_cast<double>(previous_depths_move_count);
				next_solve_time = average_branching_factor * last_solve_time;
			}

			// Calculating total time elapsed so far.
			total_time = chrono::high_resolution_clock::now() - get_move_start_time_point;
		} while (!cur_ans.winner.has_value() and total_time + next_solve_time < 1.5 * timeout);

		// Optimal move in case of a loss.
		if (cur_ans.winner.has_value() and cur_ans.winner.value() == game.get_enemy() and max_depth > 1) {
			cur_ans.move = prev_ans.move;
		}

		// Returning optimal move.
		return {cur_ans, max_depth - 1};
	}
};