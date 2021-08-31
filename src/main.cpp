#include <cassert>
#include <cstdio>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <typeindex>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <thread>
#endif

/* Common. */
#include <Common.hpp>
#include <Game.hpp>
#include <Minimax.hpp>

/* Games. */
#include <KonaneGame.hpp>
#include <MuTorereGame.hpp>
#include <BaghChalGame.hpp>
#include <TicTacToeGame.hpp>
#include <ConnectFourGame.hpp>

using namespace std;

/* Game constants. */
const vector<string> GAMES = {"Konane", "MuTorere", "BaghChal", "TicTacToe", "ConnectFour"};
const unordered_map<type_index, string> GAME_NAME = {{typeid(KonaneGame), "Konane"},
													 {typeid(MuTorereGame), "MuTorere"},
													 {typeid(BaghChalGame), "BaghChal"},
													 {typeid(TicTacToeGame), "TicTacToe"},
													 {typeid(ConnectFourGame), "ConnectFour"}};

/* Game option constants. */
constexpr int PLAYER_VS_PLAYER = 1;
constexpr int PLAYER_VS_CPU = 2;
constexpr int CPU_VS_PLAYER = 3;
constexpr int CPU_VS_CPU = 4;

/* Constants. */
const filesystem::path SAVES_FOLDER_PATH("./saves");
constexpr chrono::duration<long double> DEFAULT_TIMEOUT = 2.0s;
constexpr int MAX_COMMAND_LENGTH = 128;

/* Clears typeahead from stdin. */
void clear_input() {
	char c;

	do {
		c = getchar();
	} while (c != '\r' and c != '\n' and c != EOF);
}

/* Gets the game. */
string get_game_name() {
	int game_op;

	printf("Select Game:\n");

	for (int i = 0; i < GAMES.size(); i++) {
		printf("(%d) %s\n", i + 1, GAMES[i].c_str());
	}

	do {
		printf("Game: ");
		scanf("%d", &game_op);
		clear_input();
	} while (game_op < 1 or game_op > GAMES.size());

	printf("\n");
	
	return GAMES[game_op - 1];
}

/* Gets the game mode. */
int get_game_mode() {
	int op;

	printf("Select Game Mode:\n");
	printf("(1) Player vs Player\n");
	printf("(2) Player vs CPU (Player goes first)\n");
	printf("(3) Player vs CPU (CPU goes first)\n");
	printf("(4) CPU vs CPU\n");

	do {
		printf("Game mode: ");
		scanf("%d", &op);
		clear_input();
	} while (op < PLAYER_VS_PLAYER or op > CPU_VS_CPU);

	printf("\n");

	return op;
}

/* Returns the format string to read a command from stdin. */
string get_command_format_string() {
	return "%" + to_string(MAX_COMMAND_LENGTH) + "[^\n]";
}

/* Returns read input. */
string get_player_command() {
	char line[MAX_COMMAND_LENGTH + 1];

	if (scanf(get_command_format_string().c_str(), line) == 1) {
		clear_input();
		return string(line);
	}

	clear_input();
	return "";
}

/* Returns 1 if it's the first player or 2 if it's the second player. */
template <class GameType>
int get_player_number(int player) {
	if (player == GameType::PLAYER_MAX) {
		return 1;
	}

	if (player == GameType::PLAYER_MIN) {
		return 2;
	}

	return 0;
}

/* Prints "Player 1" or "Player 2". */
template <class GameType>
void print_player(int player) {
	if (get_player_number<GameType>(player) == 1) {
		printf(COLOR_RED "Player 1" COLOR_WHITE);
	}
	else if (get_player_number<GameType>(player) == 2) {
		printf(COLOR_BLUE "Player 2" COLOR_WHITE);
	}
	else {
		printf("Player ?");
	}
}

/* Prints the current possible moves. */
template <class GameType, class MoveType = typename GameType::move_type>
void print_possible_moves(const GameType &game) {
	vector<MoveType> moves = game.get_moves();

	if (moves.size() == 1) {
		printf(COLOR_CYAN "Only one possible move (press Enter to use it):\n" COLOR_WHITE);
	}
	else {
		printf(COLOR_CYAN "%d possible moves:\n" COLOR_WHITE, static_cast<int>(moves.size()));
	}

	for (const MoveType &move : moves) {
		printf("%s\n", string(move).c_str());
	}

	printf("\n");
}

/* Returns filename given the game turn. */
string get_filename(int turn) {
	return string("turn") + to_string(turn) + ".dat";
}

/* Clears all game saves for a certain GameType. */
template <class GameType>
void clear_game_saves() {
	// Getting path.
	string game_name = GAME_NAME.at(type_index(typeid(GameType)));
	filesystem::path saves_path = SAVES_FOLDER_PATH / game_name;

	// Clearing folder.
	filesystem::remove_all(saves_path);
}

/* Loads or starts a new game. */
template <class GameType, class StateType = typename GameType::state_type>
GameType load_game() {
	string game_name = GAME_NAME.at(type_index(typeid(GameType)));
	filesystem::path save_path;
	int turn;

	do {
		printf("Load a previous save? (0 for new game): ");
		scanf("%d", &turn);
		clear_input();

		string filename = get_filename(turn);
		save_path = SAVES_FOLDER_PATH / game_name / filename;
	} while (turn != 0 and !filesystem::exists(save_path));

	printf("\n");

	// New game.
	if (turn == 0) {
		return GameType();
	}

	// Reading file.
	ifstream file(save_path);
	string serialized_game_state((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));

	return GameType(StateType::deserialize(serialized_game_state));
}

/* Auto-saves game state into a file. */
template <class GameType>
void save_game(const GameType &game) {
	// Getting path.
	string game_name = GAME_NAME.at(type_index(typeid(GameType)));
	string filename = get_filename(game.get_turn());
	filesystem::path save_path = SAVES_FOLDER_PATH / game_name / filename;

	// Writing to file.
	filesystem::create_directories(save_path.parent_path());
	ofstream file(save_path);
	file << game.get_state().serialize();
}

/* Prints what should be printed every iteration of the game loop. */
template <class GameType>
void game_loop_print(const GameType &game) {
	// Printing the board.
	printf(COLOR_BRIGHT_BLACK "\n========== TURN %03d ==========\n\n" COLOR_WHITE, game.get_turn());
	printf("%s\n", string(game).c_str());

	// Saving the board.
	save_game(game);
	
	// Printing possible moves.
	print_possible_moves(game);

	// Printing who's turn it is.
	print_player<GameType>(game.get_player());
	printf(" moves: ");
	fflush(stdout);
}

/* Prints what should be printed at the end of the game. */
template <class GameType>
void game_end_print(const GameType &game) {
	printf(COLOR_BRIGHT_BLACK "\n========== TURN %03d ==========\n\n" COLOR_WHITE, game.get_turn());
	printf("%s\n", string(game).c_str());

	// Printing winner.
	#ifdef DEBUG
	assert(game.get_winner().has_value());
	#endif

	if (game.get_winner().value() == GameType::PLAYER_NONE) {
		printf(COLOR_YELLOW "Draw!\n" COLOR_WHITE);
	}
	else {
		print_player<GameType>(game.get_winner().value());
		printf(" won!\n");
	}
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

/* Returns the string in lowercase. */
string lower(const string &str) {
	string ans = str;

	for (char &c : ans) {
		c = tolower(c);
	}

	return ans;
}

bool is_undo_command(const string &command) {
	return lower(command) == "undo";
}

bool is_new_game_command(const string &command) {
	return lower(command) == "new game";
}

bool is_select_game_mode_command(const string &command) {
	return lower(command) == "select game mode" or lower(command) == "select gamemode" or lower(command) == "select mode" or lower(command) == "change game mode" or lower(command) == "change gamemode" or lower(command) == "change mode";
}

template <class GameType>
bool is_valid_undo_command(const GameType &game, int game_mode, const string &command) {
	if (!is_undo_command(command)) {
		return false;
	}

	if (game_mode == PLAYER_VS_PLAYER) {
		return game.get_turn() > 1;
	}

	return game.get_turn() > 2;
}

template <class GameType>
bool is_valid_new_game_command(const GameType &game, const string &command) {
	if (!is_new_game_command(command)) {
		return false;
	}

	return game.get_turn() > 1;
}

bool is_valid_select_game_mode_command(const string &command) {
	return is_select_game_mode_command(command);
}

template <class GameType>
bool is_valid_command(const GameType &game, int game_mode, const string &command) {
	return is_valid_undo_command(game, game_mode, command) or is_valid_new_game_command(game, command) or is_valid_select_game_mode_command(command);
}

template <class GameType>
bool continue_playing(GameType &game, int &game_mode) {
	string command;

	// Post game command handling.
	do {
		printf("Issue a command (or press Enter to exit): ");
		fflush(stdout);

		// Checking command.
		command = get_player_command();

		if (is_valid_command(game, game_mode, command)) {
			// Handling command.
			if (is_valid_undo_command(game, game_mode, command)) {
				if (game_mode == PLAYER_VS_PLAYER) {
					game.rollback();
				}
				else {
					game.rollback();
					game.rollback();
				}

				return true;
			}
			else if (is_valid_new_game_command(game, command)) {
				game = GameType();
				return true;
			}
			else if (is_valid_select_game_mode_command(command)) {
				printf("\nCurrent game mode is %d\n\n", game_mode);
				game_mode = get_game_mode();
			}
		}
	} while (!command.empty());

	return false;
}

template <class GameType, class MoveType = typename GameType::move_type>
optional<MoveType> get_player_move(GameType &game, int &game_mode) {
	optional<string> command;
	optional<MoveType> move;

	do {
		if (command.has_value()) {
			printf(COLOR_YELLOW "Invalid command." COLOR_WHITE " Try again: ");
			fflush(stdout);
		}

		// Checking command.
		command = get_player_command();

		if (is_valid_command(game, game_mode, command.value())){
			// Handling command.
			if (is_valid_undo_command(game, game_mode, command.value())) {
				if (game_mode == PLAYER_VS_PLAYER) {
					game.rollback();
				}
				else {
					game.rollback();
					game.rollback();
				}
			}
			else if (is_valid_new_game_command(game, command.value())) {
				game = GameType();
			}
			else if (is_valid_select_game_mode_command(command.value())) {
				printf("\nCurrent game mode is %d\n\n", game_mode);
				game_mode = get_game_mode();
			}

			return nullopt;
		}

		move = game.get_player_move(command.value());
	} while (!move.has_value());

	return move.value();
}

/* Returns a move given by the AI. */
template <class GameType, class MoveType = typename GameType::move_type>
MoveType get_ai_move(const GameType &game, Minimax<GameType> &ai, chrono::duration<long double> timeout = DEFAULT_TIMEOUT) {
	typename Minimax<GameType>::OptimalMove ans;
	int depth;

	// Getting optimal move.
	chrono::time_point<chrono::high_resolution_clock> t_start = chrono::high_resolution_clock::now();
	tie(ans, depth) = ai.get_move(game, timeout);
	chrono::duration<long double> t = chrono::high_resolution_clock::now() - t_start;

	// Pretending that the AI is thinking for at least timeout milliseconds.
	#ifdef _WIN32
	Sleep(max(0ll, chrono::round<chrono::milliseconds>(timeout - t).count()));
	#else
	this_thread::sleep_for(max(0.0s, timeout - t));
	#endif

	// Printing move.
	printf("%s\n", string(ans.move).c_str());

	string score_color = ans.score == 0.0 ? COLOR_YELLOW : (ans.score > 0.0 ? COLOR_RED : COLOR_BLUE);
	string thinking_time_color = t <= timeout ? COLOR_GREEN : (t <= 2.0 * timeout ? COLOR_YELLOW : COLOR_RED);

	// Printing thinking time.
	printf("(%sscore = %.3lf" COLOR_WHITE " / %stime = %.3Lfs" COLOR_WHITE " / depth = %d) ", score_color.c_str(), ans.score, thinking_time_color.c_str(), t.count(), depth);

	// Printing if the AI is playing optimally or not.
	if (ans.is_solved) {
		printf("CPU is playing " COLOR_GREEN "optimally\n" COLOR_WHITE);

		if (ans.winner.has_value()) {
			if (ans.winner.value() != GameType::PLAYER_NONE) {
				print_player<GameType>(ans.winner.value());
				printf(" will " COLOR_GREEN "win" COLOR_WHITE " in at most %d moves\n", ans.turn - game.get_turn() - 1);
			}
			else {
				printf("(The game will end in a " COLOR_YELLOW "draw" COLOR_WHITE " in at most %d moves)\n", ans.turn - game.get_turn() - 1);
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

/* Game loop. */
template <class GameType, class MoveType = typename GameType::move_type>
void game_loop() {
	// Initializing.
	Minimax<GameType> ai;
	int game_mode = get_game_mode();
	GameType game = load_game<GameType>();

	// Clearing old saves.
	clear_game_saves<GameType>();

	do {
		// Game loop.
		while (!game.is_game_over()) {
			// Printing the current state of the game.
			game_loop_print(game);
			
			// Move input.
			if (is_player_turn(game, game_mode)) { // Human.
				optional<MoveType> move = get_player_move(game, game_mode);

				if (move.has_value()) {
					game.make_move(move.value());
				}
			}
			else { // CPU.
				game.make_move(get_ai_move(game, ai));
			}
		}

		// Printing the final board.
		game_end_print(game);
	} while (continue_playing(game, game_mode));
}

int main() {
	// Initializing.
	printf(COLOR_WHITE);

	// Choosing game to play.
	string game_name = get_game_name();

	// Running game.
	if (game_name == "Konane") {
		game_loop<KonaneGame>();
	}
	else if (game_name == "MuTorere") {
		game_loop<MuTorereGame>();
	}
	else if (game_name == "BaghChal") {
		game_loop<BaghChalGame>();
	}
	else if (game_name == "TicTacToe") {
		game_loop<TicTacToeGame>();
	}
	else if (game_name == "ConnectFour") {
		game_loop<ConnectFourGame>();
	}
	else {
		printf("%s has not been implemented.\n", game_name.c_str());
	}

	// Reseting.
	printf(COLOR_RESET);

	return 0;
}