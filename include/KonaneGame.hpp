#pragma once

#include <optional>
#include <string>
#include <vector>

#include <Game.hpp>

class KonaneCell {
  public:
    int x, y;

    KonaneCell() : x(-1), y(-1) {}
    KonaneCell(int x_, int y_) : x(x_), y(y_) {}

    bool operator==(const KonaneCell &c) const {
        return x == c.x and y == c.y;
    }

    bool operator!=(const KonaneCell &c) const {
        return !(*this == c);
    }

    operator string() const {
        return "(" + to_string(x) + ", " + to_string(y) + ")";
    }
};

class KonaneMove : public GameMove {
  public:
    KonaneCell ci, cf;

    KonaneMove() : ci(KonaneCell()), cf(KonaneCell()) {}
    KonaneMove(KonaneCell ci_, KonaneCell cf_) : ci(ci_), cf(cf_) {}
    KonaneMove(int xi, int yi, int xf, int yf) : KonaneMove(KonaneCell(xi, yi), KonaneCell(xf, yf)) {}

    operator string() const override {
        if (cf == KonaneCell(-1, -1)) {
            return string(ci);
        }

        return string(ci) + " -> " + string(cf);
    }
};

class KonaneState : public GameState {
  private:
    long long state_;

  public:
    explicit KonaneState(long long state) {
        state_ = state;
        hash_ = std::hash<long long>()(state_);
    }

    string serialize() const override {
        return to_string(state_);
    }

    static KonaneState deserialize(const string &serialized_state) {
        return KonaneState(stoll(serialized_state));
    }

    long long get() const {
        return state_;
    }

    bool operator==(const GameState &rhs) const override {
        return state_ == dynamic_cast<const KonaneState &>(rhs).state_;
    }
};

template <> struct std::hash<KonaneState> : public std::hash<GameState> { using std::hash<GameState>::operator(); };

class KonaneGame : public Game<KonaneState, KonaneMove> {
  private:
    /* Board is N x N. */
    static constexpr int N = 6;

    /* Cell state constants. */
    static constexpr int WHITE = Game<KonaneState, KonaneMove>::PLAYER_MAX;
    static constexpr int BLACK = Game<KonaneState, KonaneMove>::PLAYER_MIN;
    static constexpr int NONE = Game<KonaneState, KonaneMove>::PLAYER_NONE;

    /* Direction constants. */
    static constexpr int UP = 0;
    static constexpr int DOWN = 1;
    static constexpr int LEFT = 2;
    static constexpr int RIGHT = 3;
    static constexpr int DIR[2][4] = {{-1, 1, 0, 0}, {0, 0, -1, 1}};

    /* Returns if the coordinate is inside the board. */
    static bool is_inside(const KonaneCell &c) {
        return 0 <= c.x and c.x < N and 0 <= c.y and c.y < N;
    }

    /* From (x, y) to position. */
    static int convert_cell(const KonaneCell &c) {
        return N * c.x + c.y;
    }

    /* From position to (x, y). */
    static KonaneCell convert_cell(int p) {
        return KonaneCell(p / N, p % N);
    }

    long long board; // Board.

    /* Returns the number of pawns remaining on the board. */
    int count_pawns() const;

    /* Returns what is in cell (x, y). */
    int test(const KonaneCell &) const;

    /* Sets cell (x, y). */
    void set(const KonaneCell &);

    /* Resets cell (x, y). */
    void reset(const KonaneCell &);

    /* Returns true if the next move is the first move of the match. */
    bool is_first_turn() const;

    /* Returns true if the next move is the second move of the match. */
    bool is_second_turn() const;

    /* Returns if the move (x, y) at the start of the game is a valid move. */
    bool is_valid_starting_move(const KonaneCell &) const;

    /* Makes the move (x, y) for the first two moves in the game. Assumes that isValidMove(x, y) is true. */
    void make_starting_move(const KonaneMove &);

    /* Returns all the possible start moves. */
    vector<KonaneMove> get_starting_moves() const;

  protected:
    /* Returns the current game state converted to State. */
    KonaneState get_state_() const override;

    /* Loads the game given a State. */
    void load_game_(const KonaneState &) override;

    /* Performs a move. Assumes that is_valid_move(m) is true. TODO: Remove assumption that is_valid_move(m) is true. */
    void make_move_(const KonaneMove &) override;

    /* Returns a move inputed by the player. */
    optional<KonaneMove> get_player_move_(const string &) const override;

    /* Returns all the current possible moves. */
    vector<KonaneMove> get_moves_() const override;

    /* Returns the winner. */
    using Game<KonaneState, KonaneMove>::get_winner_;

  public:
    KonaneGame();
    KonaneGame(const KonaneState &);

    /* Returns if the move (xi, yi) -> (xf, yf) is a valid move. */
    bool is_valid_move(const KonaneMove &) const override;

    /* Returns a value between -1 and 1 indicating how probable it is for the first player to win (1.0) or the other player to win (-1.0). */
    using Game<KonaneState, KonaneMove>::evaluate;

    /* Returns the board for printing. */
    operator string() const override;
};