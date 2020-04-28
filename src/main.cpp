#include <cstdio>
#include <cassert>
#include <type_traits>
#include <chrono>
#include <thread>

/* Common. */
#include <Common.hpp>
#include <Game.hpp>
#include <Minimax.hpp>

/* Games. */
#include <KonaneGame.hpp>
#include <MuTorereGame.hpp>
#include <BaghChalGame.hpp>

using namespace std;

/* Game constants. */
constexpr int KONANE = 1;
constexpr int MUTORERE = 2;
constexpr int BAGHCHAL = 3;

/* Game option constants. */
constexpr int PLAYER_VS_PLAYER = 1;
constexpr int PLAYER_VS_CPU = 2;
constexpr int CPU_VS_PLAYER = 3;
constexpr int CPU_VS_CPU = 4;

/* Gets the game. */
int get_game() {
	int game_op;

	printf("(1) Konane\n");
	printf("(2) MuTorere\n");
	printf("(3) BaghChal\n");

	do {
		printf("Game: ");
		scanf("%d", &game_op);
	} while (game_op < 1 or game_op > 3);

	printf("\n");
	
	return game_op;
}

/* Gets the game mode. */
int get_game_mode() {
	int op;

	printf("(1) Player as White vs Player as Black\n");
	printf("(2) Player as White vs CPU    as Black\n");
	printf("(3) CPU    as White vs Player as Black\n");
	printf("(4) CPU    as White vs CPU    as Black\n");

	do {
		printf("Game mode: ");
		scanf("%d", &op);
	} while (op < 1 or op > 4);

	printf("\n");

	return op;
}

/* Returns a move given by the AI. */
template <class AIType, class GameType, class MoveType = typename GameType::move_type>
MoveType get_ai_move(AIType &ai, const GameType &game, int player, long long timeout = 2000) {
	typename AIType::OptimalMove ans;
	int depth;

	// Getting optimal move.
	chrono::time_point<chrono::high_resolution_clock> t_start = chrono::high_resolution_clock::now();
	tie(ans, depth) = ai.get_move(game, timeout);
	chrono::milliseconds t = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t_start);

	// Pretending that the AI is thinking for at least timeout milliseconds.
	this_thread::sleep_for(max(chrono::milliseconds(0), chrono::milliseconds(timeout) - t));

	// Printing real thinking time.
	if (t <= chrono::milliseconds(timeout)) {
		printf(COLOR_GREEN "(%d / %ld.%03lds) " COLOR_WHITE, depth, t.count() / 1000, t.count() % 1000);
	}
	else if (t <= 2 * chrono::milliseconds(timeout)) {
		printf(COLOR_YELLOW "(%d / %ld.%03lds) " COLOR_WHITE, depth, t.count() / 1000, t.count() % 1000);
	}
	else {
		printf(COLOR_RED "(%d / %ld.%03lds) " COLOR_WHITE, depth, t.count() / 1000, t.count() % 1000);
	}

	// Printing if the AI is playing optimally or not.
	if (ans.is_optimal) {
		int winner = ans.winner == AIType::PLAYER_MAX ? 1 : 2;
		string winner_color = player == winner ? COLOR_GREEN : COLOR_RED;
		printf("CPU is playing " COLOR_GREEN "optimally %s(Player %d wins in %d moves)" COLOR_WHITE ": ", winner_color.c_str(), winner, ans.turn - game.get_turn());
	}
	else {
		printf("CPU might be playing " COLOR_RED "non optimally" COLOR_WHITE ": ");
	}

	printf("%s\n\n", string(ans.move).c_str());

	return ans.move;
}

/* Prints all the possible moves. */
template <class GameType, class MoveType = typename GameType::move_type>
void print_possible_moves(const GameType &game) {
	vector<MoveType> moves = game.get_moves();

	for (const MoveType &move : moves) {
		printf("%s\n", string(move).c_str());
	}
}

/* Prints what should be printed every iteration of the game loop. */
template <class GameType>
void game_loop_print(const GameType &game, int player) {
	// Printing the board.
	printf("%s\n", string(game).c_str());
	printf(COLOR_BRIGHT_BLACK "========================\n\n" COLOR_WHITE);
	
	// Printing possible moves.
	printf(COLOR_CYAN "Possible moves:\n" COLOR_WHITE);
	print_possible_moves(game);

	// Printing who's turn it is.
	if (player == 1) {
		printf(COLOR_RED "\nPlayer 1" COLOR_WHITE " moves: ");
		fflush(stdout);
	}
	else {
		printf(COLOR_BLUE "\nPlayer 2" COLOR_WHITE " moves: ");
		fflush(stdout);
	}
}

/* Returns true if its the player's turn. */
bool is_player_turn(int player, int op) {
	if (player == 1) {
		return op == PLAYER_VS_PLAYER or op == PLAYER_VS_CPU;
	}
	else {
		return op == PLAYER_VS_PLAYER or op == CPU_VS_PLAYER;
	}
}

/* Game loop. */
template <class GameType>
void game_loop(GameType game) {
	// Creating AI.
	Minimax<GameType> ai = Minimax<GameType>();

	// Initializing.
	int op = get_game_mode();
	int player = 1;

	// Game loop.
	while (!game.is_game_over()) {
		// Printing the current state of the game.
		game_loop_print(game, player);
		
		// Move input.
		if (is_player_turn(player, op)) { // Human.
			game.make_move(game.get_player_move());
		}
		else { // CPU.
			game.make_move(get_ai_move(ai, game, player));
		}

		// Next turn.
		player = player == 1 ? 2 : 1;
	}

	// Printing the final board.
	printf("%s\n", string(game).c_str());
	printf(COLOR_BRIGHT_BLACK "========================\n\n" COLOR_WHITE);

	// Printing winner.
	if (player == 1) {
		printf(COLOR_BLUE "Player 2" COLOR_WHITE " won!\n");
	}
	else {
		printf(COLOR_RED "Player 1" COLOR_WHITE " won!\n");
	}
}

int main() {
	// Initializing.
	printf(COLOR_WHITE);

	// Choosing game to play.
	int game_op = get_game();

	// Running game.
	if (game_op == KONANE) {
		game_loop(KonaneGame());
	}
	else if (game_op == MUTORERE) {
		game_loop(MuTorereGame());
	}
	else if (game_op == BAGHCHAL) {
		game_loop(BaghChalGame());
	}

	// Reseting.
	printf(COLOR_RESET);

	return 0;
}