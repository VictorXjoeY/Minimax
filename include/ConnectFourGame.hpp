#pragma once

#include <optional>
#include <string>
#include <vector>

#include <Game.hpp>

class ConnectFourMove : public GameMove {
  public:
    int y;

    ConnectFourMove() : y(-1) {}
    ConnectFourMove(int y_) : y(y_) {}

    operator string() const override {
        return to_string(y);
    }
};

class ConnectFourState : public GameState {
  private:
    string state_;

  public:
    explicit ConnectFourState(const string &state) {
        state_ = state;
        hash_ = std::hash<string>()(state_);
    }

    string serialize() const override {
        return state_;
    }

    static ConnectFourState deserialize(const string &serialized_state) {
        return ConnectFourState(serialized_state);
    }

    string get() const {
        return state_;
    }

    bool operator==(const GameState &rhs) const override {
        return state_ == dynamic_cast<const ConnectFourState &>(rhs).state_;
    }
};

template <> struct std::hash<ConnectFourState> : public std::hash<GameState> { using std::hash<GameState>::operator(); };

class ConnectFourGame : public Game<ConnectFourState, ConnectFourMove> {
  private:
    /* Board is N x M. */
    static constexpr int N = 6;
    static constexpr int M = 7;

    /* Cell state constants. */
    static constexpr int YELLOW = Game<ConnectFourState, ConnectFourMove>::PLAYER_MAX;
    static constexpr int RED = Game<ConnectFourState, ConnectFourMove>::PLAYER_MIN;
    static constexpr int NONE = Game<ConnectFourState, ConnectFourMove>::PLAYER_NONE;

    /* UPRIGHT, RIGHT, DOWNRIGHT, DOWN */
    static constexpr int DIR[4][2] = {{-1, 1}, {0, 1}, {1, 1}, {1, 0}};

    /* Score constants. */
    static constexpr int UPRIGHT_POSSIBILITIES = 12;
    static constexpr int RIGHT_POSSIBILITIES = 24;
    static constexpr int DOWNRIGHT_POSSIBILITIES = 12;
    static constexpr int DOWN_POSSIBILITIES = 21;
    static constexpr int TOTAL_POSSIBILITIES = UPRIGHT_POSSIBILITIES + RIGHT_POSSIBILITIES + DOWNRIGHT_POSSIBILITIES + DOWN_POSSIBILITIES;
    static constexpr int MAX_SCORE = 5 * TOTAL_POSSIBILITIES; // Considering every cell is filled with the same color.

    /* Returns if the coordinate is inside the board. */
    static bool is_inside(int x, int y) {
        return 0 <= x and x < N and 0 <= y and y < M;
    }

    int board[N][M]; // Board.

    /* Returns the score of the board for the given player. */
    int get_score_(int) const;

    /* Checks if there are 4 pieces connected. */
    bool has_someone_won_() const;

  protected:
    /* Returns the current game state converted to State. */
    ConnectFourState get_state_() const override;

    /* Loads the game given a State. */
    void load_game_(const ConnectFourState &) override;

    /* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
    void make_move_(const ConnectFourMove &) override;

    /* Returns a move inputed by the player. */
    optional<ConnectFourMove> get_player_move_(const string &) const override;

    /* Returns all the current possible moves. */
    vector<ConnectFourMove> get_moves_() const override;

    /* Returns the winner. */
    int get_winner_() const override;

    /* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
    double evaluate_() const override;

  public:
    ConnectFourGame();
    ConnectFourGame(const ConnectFourState &);

    /* Returns if the move (x, y) is a valid move. */
    bool is_valid_move(const ConnectFourMove &) const override;

    /* Returns the board for printing. */
    operator string() const override;
};