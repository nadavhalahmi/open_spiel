// Copyright 2019 DeepMind Technologies Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OPEN_SPIEL_GAMES_CROWNY_H_
#define OPEN_SPIEL_GAMES_CROWNY_H_

#include <array>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "open_spiel/spiel.h"

// An implementation of the classic: https://en.wikipedia.org/wiki/Backgammon
// using rule set from
// http://usbgf.org/learn-backgammon/backgammon-rules-and-terms/rules-of-backgammon/
// where red -> 'x' (player 0) and white -> 'o' (player 1).
//
// Currently does not support the doubling cube nor "matches" (multiple games
// where outcomes are scored and tallied to 21).
//
// Parameters:
//   "hyper_backgammon"  bool    Use Hyper-backgammon variant [1] (def: false)
//   "scoring_type"      string  Type of scoring for the game: "winloss_scoring"
//                               (default), "enable_gammons", or "full_scoring"
//
// [1] https://bkgm.com/variants/HyperBackgammon.html. Hyper-backgammon is a
// simplified backgammon start setup which is small enough to solve. Note that
// it is not the full Hyper-backgammon sinc do not have cube is not implemented.

namespace open_spiel {
namespace crowny {

inline constexpr const int kNumPlayers = 2;
inline constexpr const int kNumChanceOutcomes = 21;
inline constexpr const int kNumPoints = 24;
inline constexpr const int kNumDiceOutcomes = 6;
inline constexpr const int kXPlayerId = 0;
inline constexpr const int kOPlayerId = 1;

// Number of checkers per player in the standard game. For varaints, use
// CrownyGame::NumCheckersPerPlayer.
inline constexpr const int kNumCheckersPerPlayer = 15;

// TODO: look into whether these can be set to 25 and -2 to avoid having a
// separate helper function (PositionToStringHumanReadable) to convert moves
// to strings.
inline constexpr const int kScorePos = 101;

// The action encoding stores a number in { 0, 1, ..., 1351 }. If the high
// roll is to move first, then the number is encoded as a 2-digit number in
// base 26 ({0, 1, .., 23, kBarPos, Pass}) (=> first 676 numbers). Otherwise,
// the low die is to move first and, 676 is subtracted and then again the
// number is encoded as a 2-digit number in base 26.
inline constexpr const int kNumDistinctActions = 1352;

// See ObservationTensorShape for details.
inline constexpr const int kBoardEncodingSize = 4 * kNumPoints * kNumPlayers;
inline constexpr const int kStateEncodingSize =
    3 * kNumPlayers + kBoardEncodingSize + 2;
inline constexpr const char* kDefaultScoringType = "winloss_scoring";

// Game scoring type, whether to score gammons/backgammons specially.
enum class ScoringType {
  kWinLossScoring,  // "winloss_scoring": Score only 1 point per player win.
  kFullScoring,     // "full_scoring": Score gammons as well as 3 points for a
                    // "backgammon".
};

struct CheckerMove {
  std::pair<int, int> from;  // 0-24  (0-23 for locations on the board and kBarPos)
  std::pair<int, int> to;  // 0-24  (0-23 for locations on the board and kBarPos)
  CheckerMove(std::pair<int, int> _from, std::pair<int, int> _to)
      : from(_from), to(_to) {}
  bool operator<(const CheckerMove& rhs) const {
        if (this->from.first != rhs.from.first) return this->from.first < rhs.from.first;
        if (this->from.second != rhs.from.second) return this->from.second < rhs.from.second;
        if (this->to.first != rhs.to.first) return this->to.first < rhs.to.first;
        if (this->to.second != rhs.to.second) return this->to.second < rhs.to.second;
        return false; // equals
    }
};

enum class PieceType { ENUM_RED, ENUM_BLUE };

class Piece {
public:
    explicit Piece(PieceType type) : type_(type) {}
private:
    PieceType type_;
};

class Pawn : public Piece {
public:
    explicit Pawn(PieceType type) : Piece(type) {}
};

class Archer : public Piece {
public:
    explicit Archer(PieceType type) : Piece(type) {}
};

class King : public Piece {
public:
    explicit King(PieceType type) : Piece(type) {}
};

class CrownyGame;

class CrownyState : public State {
 public:
  CrownyState(const CrownyState&) = default;
  CrownyState(std::shared_ptr<const Game>, ScoringType scoring_type);

  Player CurrentPlayer() const override;
  void UndoAction(Player player, Action action) override;
  std::vector<Action> LegalActions() const override;
  std::string ActionToString(Player player, Action move_id) const override;
  std::vector<std::pair<Action, double>> ChanceOutcomes() const override;
  std::string ToString() const override;
  bool IsTerminal() const override;
  std::vector<double> Returns() const override;
  std::string ObservationString(Player player) const override;
  void ObservationTensor(Player player,
                         absl::Span<float> values) const override;
  std::unique_ptr<State> Clone() const override;

  // Setter function used for debugging and tests. Note: this does not set the
  // historical information properly, so Undo likely will not work on states
  // set this way!
  void SetState(int cur_player, bool double_turn, const std::vector<int>& dice,
                const std::vector<int>& bar, const std::vector<int>& scores,
                const std::vector<std::vector<std::vector<Piece>>>& board);

  // Returns the opponent of the specified player.
  int Opponent(int player) const;

  // Compute a distance between 'from' and 'to'. The from can be kBarPos. The
  // to can be a number below 0 or above 23, but do not use kScorePos directly.

  int GetDistance(std::pair<int, int> from, std::pair<int, int> to) const;

  // Is this position off the board, i.e. >23 or <0?
  bool IsOff(std::pair<int, int> pos) const;

  // Returns whether pos2 is further (closer to scoring) than pos1 for the
  // specifed player.
  bool IsFurther(int player, std::pair<int, int> pos1, std::pair<int, int> pos2) const;

  // Is this a legal from -> to checker move? Here, the to_pos can be a number
  // that is outside {0, ..., 23}; if so, it is counted as "off the board" for
  // the corresponding player (i.e. >23 is a bear-off move for XPlayerId, and
  // <0 is a bear-off move for OPlayerId).
  bool IsLegalFromTo(int player, std::pair<int,int> from_pos, std::pair<int,int> to_pos, int my_checkers_from,
                     int opp_checkers_to) const;

  // Get the To position for this play given the from position and number of
  // pips on the die. This function simply adds the values: the return value
  // will be a position that might be off the the board (<0 or >23).
  int GetToPos(int player, std::pair<int,int> from_pos, int pips) const;

  // Returns if moving from the position for the number of spaces is a hit.
  bool IsHit(Player player, std::pair<int,int> from_pos, int num) const;

  // Accessor functions for some of the specific data.
  int player_turns() const { return turns_; }
  int player_turns(int player) const {
    return (player == kXPlayerId ? x_turns_ : o_turns_);
  }
  int bar(int player) const { return bar_[player]; }
  int score(int player) const { return scores_[player]; }
  int dice(int i) const { return dice_[i]; }
  bool double_turn() const { return double_turn_; }

  // Get the number of checkers on the board in the specified position belonging
  // to the specified player. The position can be kBarPos or any valid position
  // on the main part of the board, but kScorePos (use score() to get the number
  // of checkers born off).
  std::vector<Piece> board(std::pair<int, int> pos) const;

  // Action encoding / decoding functions. Note, the converted checker moves
  // do not contain the hit information; use the AddHitInfo function to get the
  // hit information.
  Action CheckerMovesToSpielMove(const std::vector<CheckerMove>& moves) const;
  std::vector<CheckerMove> SpielMoveToCheckerMoves(int player,
                                                   Action spiel_move) const;
  Action TranslateAction(std::pair<int, int> from1, std::pair<int, int> from2, bool use_high_die_first) const;

  // Return checker moves with extra hit information.
  std::vector<CheckerMove>
  AugmentWithHitInfo(Player player,
                     const std::vector<CheckerMove> &cmoves) const;

 protected:
  void DoApplyAction(Action move_id) override;

 private:
  void SetupInitialBoard();
  void RollDice(int outcome);
  bool IsPosInHome(int player, std::pair<int, int> pos) const;
  bool AllInHome(int player) const;
  int CheckersInHome(int player) const;
  bool UsableDiceOutcome(int outcome) const;
  int NumOppCheckers(int player, std::pair<int, int> pos) const;
  std::string DiceToString(int outcome) const;
  int DiceValue(int i) const;
  int HighestUsableDiceOutcome() const;

  // A helper function used by ActionToString to add necessary hit information
  // and compute whether the move goes off the board.
  int AugmentCheckerMove(CheckerMove* cmove, int player, int start) const;

  std::pair<int, int> PositionFrom(int player, std::pair<int, int> from, std::pair<int, int> to) const;

  void ApplyCheckerMove(int player, const CheckerMove &move);
  void UndoCheckerMove(int player, const CheckerMove& move);
  std::set<CheckerMove> LegalCheckerMoves(int player) const;
  int RecLegalMoves(std::vector<CheckerMove> moveseq,
                    std::set<std::vector<CheckerMove>>* movelist);
  std::vector<Action> ProcessLegalMoves(
      int max_moves, const std::set<std::vector<CheckerMove>>& movelist) const;

  ScoringType scoring_type_;  // Which rules apply when scoring the game.

  Player cur_player_;
  Player prev_player_;
  int turns_;
  int x_turns_;
  int o_turns_;
  bool double_turn_;
  std::vector<int> dice_;    // Current dice.
  std::vector<int> bar_;     // Checkers of each player in the bar.
  std::vector<int> scores_;  // Checkers returned home by each player.
  std::vector<std::vector<std::vector<Piece>>> board_;  // Checkers for each player on points.
};

class CrownyGame : public Game {
 public:
  explicit CrownyGame(const GameParameters& params);

  int NumDistinctActions() const override { return kNumDistinctActions; }

  std::unique_ptr<State> NewInitialState() const override {
    return std::unique_ptr<State>(new CrownyState(
        shared_from_this(), scoring_type_));
  }

  // On the first turn there are 30 outcomes: 15 for each player (rolls without
  // the doubles).
  int MaxChanceOutcomes() const override { return 30; }

  // There is arbitrarily chosen number to ensure the game is finite.
  int MaxGameLength() const override { return 1000; }

  // Upper bound: chance node per move, with an initial chance node for
  // determining starting player.
  int MaxChanceNodesInHistory() const override { return MaxGameLength() + 1; }

  int NumPlayers() const override { return 2; }
  double MinUtility() const override { return -MaxUtility(); }
  absl::optional<double> UtilitySum() const override { return 0; }
  double MaxUtility() const override;

  std::vector<int> ObservationTensorShape() const override {
    // Encode each point on the board as four doubles:
    // - One double for whether there is one checker or not (1 or 0).
    // - One double for whether there are two checkers or not (1 or 0).
    // - One double for whether there are three checkers or not (1 or 0).
    // - One double if there are more than 3 checkers, the number of checkers.
    //   more than three that are on that point.
    //
    // Return a vector encoding:
    // Every point listed for the current player.
    // Every point listed for the opponent.
    // One double for the number of checkers on the bar for the current player.
    // One double for the number of checkers scored for the current player.
    // One double for whether it's the current player's turn (1 or 0).
    // One double for the number of checkers on the bar for the opponent.
    // One double for the number of checkers scored for the opponent.
    // One double for whether it's the opponent's turn (1 or 0).
    // One double for the first dice's value.
    // One double for the second dice's value.

    return {kStateEncodingSize};
  }

  int NumCheckersPerPlayer() const;

 private:
  ScoringType scoring_type_;  // Which rules apply when scoring the game.
};

}  // namespace crowny
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_CROWNY_H_
