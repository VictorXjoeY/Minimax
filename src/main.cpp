#include <cstdio>
#include <cassert>
#include <type_traits>
#include <chrono>
#include <thread>
#include <vector>

/* Common. */
#include <Common.hpp>
#include <Game.hpp>
#include <Minimax.hpp>

/* Games. */
#include <KonaneGame.hpp>
#include <MuTorereGame.hpp>
#include <BaghChalGame.hpp>
#include <TicTacToeGame.hpp>

using namespace std;

/* Game constants. */
const vector<string> GAMES = {"Konane", "MuTorere", "BaghChal", "TicTacToe"};

/* Game option constants. */
constexpr int PLAYER_VS_PLAYER = 1;
constexpr int PLAYER_VS_CPU = 2;
constexpr int CPU_VS_PLAYER = 3;
constexpr int CPU_VS_CPU = 4;

/* Gets the game. */
string get_game_name() {
	int game_op;

	for (int i = 0; i < GAMES.size(); i++) {
		printf("(%d) %s\n", i + 1, GAMES[i].c_str());
	}

	do {
		printf("Game: ");
		scanf("%d", &game_op);
	} while (game_op < 1 or game_op > GAMES.size());

	printf("\n");
	
	return GAMES[game_op - 1];
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
	} while (op < PLAYER_VS_PLAYER or op > CPU_VS_CPU);

	printf("\n");

	return op;
}

/* Returns a move given by the AI. */
template <class AIType, class GameType, class MoveType = typename GameType::move_type>
MoveType get_ai_move(AIType &ai, const GameType &game, long long timeout = 1000) {
	typename AIType::OptimalMove ans;
	int depth;

	// Getting optimal move.
	chrono::time_point<chrono::high_resolution_clock> t_start = chrono::high_resolution_clock::now();
	tie(ans, depth) = ai.get_move(game, timeout);
	chrono::milliseconds t = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t_start);

	// Pretending that the AI is thinking for at least timeout milliseconds.
	this_thread::sleep_for(max(chrono::milliseconds(0), chrono::milliseconds(timeout) - t));

	// Printing move.
	printf("%s\n", string(ans.move).c_str());

	// Printing real thinking time.
	if (t <= chrono::milliseconds(timeout)) {
		printf(COLOR_GREEN "(%d / %lld.%03llds) " COLOR_WHITE, depth, t.count() / 1000ll, t.count() % 1000ll);
	}
	else if (t <= 2 * chrono::milliseconds(timeout)) {
		printf(COLOR_YELLOW "(%d / %lld.%03llds) " COLOR_WHITE, depth, t.count() / 1000ll, t.count() % 1000ll);
	}
	else {
		printf(COLOR_RED "(%d / %lld.%03llds) " COLOR_WHITE, depth, t.count() / 1000ll, t.count() % 1000ll);
	}

	// Printing if the AI is playing optimally or not.
	if (ans.is_optimal) {
		printf("CPU is playing " COLOR_GREEN "optimally\n" COLOR_WHITE);

		if (ans.winner.has_value()) {
			if (ans.winner.value() != GameType::PLAYER_NONE) {
				string winner_color = game.get_player() == ans.winner.value() ? COLOR_GREEN : COLOR_RED;
				printf("%s(Player %d will win in at most %d moves)\n" COLOR_WHITE, winner_color.c_str(), (3 + ans.winner.value()) % 3, ans.turn - game.get_turn());
			}
			else {
				printf("(The game will end in a draw in at most %d moves)\n", ans.turn - game.get_turn());
			}
		}
		else {
			printf("(The game can go on forever!)\n");
		}
	}
	else {
		printf("CPU might be playing " COLOR_RED "non optimally\n" COLOR_WHITE);
	}

	printf("\n");

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

/* Prints "Player 1" or "Player 2". */
template <class GameType>
void print_player(int player) {
	if (player == GameType::PLAYER_MAX) {
		printf(COLOR_RED "Player 1" COLOR_WHITE);
	}
	else {
		printf(COLOR_BLUE "Player 2" COLOR_WHITE);
	}
}

/* Prints what should be printed every iteration of the game loop. */
template <class GameType>
void game_loop_print(const GameType &game) {
	// Printing the board.
	printf("%s\n", string(game).c_str());
	printf(COLOR_BRIGHT_BLACK "========================\n\n" COLOR_WHITE);
	
	// Printing possible moves.
	printf(COLOR_CYAN "Possible moves:\n" COLOR_WHITE);
	print_possible_moves(game);
	printf("\n");

	// Printing who's turn it is.
	print_player<GameType>(game.get_player());
	printf(" moves: ");
	fflush(stdout);
}

/* Returns true if its the player's turn. */
template <class GameType>
bool is_player_turn(const GameType &game, int op) {
	if (game.get_player() == GameType::PLAYER_MAX) {
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

	// Game loop.
	while (!game.is_game_over()) {
		// Printing the current state of the game.
		game_loop_print(game);
		
		// Move input.
		if (is_player_turn(game, op)) { // Human.
			game.make_move(game.get_player_move());
		}
		else { // CPU.
			game.make_move(get_ai_move(ai, game));
		}
	}

	// Printing the final board.
	printf("%s\n", string(game).c_str());
	printf(COLOR_BRIGHT_BLACK "========================\n\n" COLOR_WHITE);

	// Printing winner.
	assert(game.get_winner().has_value());

	if (game.get_winner().value() == GameType::PLAYER_NONE) {
		printf("Draw!\n");
	}
	else {
		print_player<GameType>(game.get_winner().value());
		printf(" won!\n");
	}
}

int main() {
	// Initializing.
	printf(COLOR_WHITE);

	// Choosing game to play.
	string game_name = get_game_name();

	// Running game.
	if (game_name == "Konane") {
		game_loop(KonaneGame());
	}
	else if (game_name == "MuTorere") {
		game_loop(MuTorereGame());
	}
	else if (game_name == "BaghChal") {
		game_loop(BaghChalGame());
	}
	else if (game_name == "TicTacToe") {
		game_loop(TicTacToeGame());
	}
	else {
		printf("%s has not been implemented.\n", game_name.c_str());
	}

	// Reseting.
	printf(COLOR_RESET);

	return 0;
}