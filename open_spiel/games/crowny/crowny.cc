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

#include <algorithm>
#include <cstdlib>
#include <set>
#include <utility>
#include <vector>

#include "open_spiel/abseil-cpp/absl/strings/str_cat.h"
#include "open_spiel/game_parameters.h"
#include "open_spiel/spiel.h"
#include "open_spiel/spiel_utils.h"
#include "open_spiel/games/crowny/crowny.h"

namespace open_spiel {
namespace crowny {
namespace {

// A few constants to help with the conversion to human-readable string formats.
// TODO: remove these once we've changed kBarPos and kScorePos (see TODO in
// header).
constexpr int kNumNonDoubleOutcomes = 15;

const std::vector<std::pair<Action, double>> kChanceOutcomes = {
    std::pair<Action, double>(0, 1.0 / 18),
    std::pair<Action, double>(1, 1.0 / 18),
    std::pair<Action, double>(2, 1.0 / 18),
    std::pair<Action, double>(3, 1.0 / 18),
    std::pair<Action, double>(4, 1.0 / 18),
    std::pair<Action, double>(5, 1.0 / 18),
    std::pair<Action, double>(6, 1.0 / 18),
    std::pair<Action, double>(7, 1.0 / 18),
    std::pair<Action, double>(8, 1.0 / 18),
    std::pair<Action, double>(9, 1.0 / 18),
    std::pair<Action, double>(10, 1.0 / 18),
    std::pair<Action, double>(11, 1.0 / 18),
    std::pair<Action, double>(12, 1.0 / 18),
    std::pair<Action, double>(13, 1.0 / 18),
    std::pair<Action, double>(14, 1.0 / 18),
    std::pair<Action, double>(15, 1.0 / 36),
    std::pair<Action, double>(16, 1.0 / 36),
    std::pair<Action, double>(17, 1.0 / 36),
    std::pair<Action, double>(18, 1.0 / 36),
    std::pair<Action, double>(19, 1.0 / 36),
    std::pair<Action, double>(20, 1.0 / 36),
};

const std::vector<std::vector<int>> kChanceOutcomeValues = {
    {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {2, 3}, {2, 4},
    {2, 5}, {2, 6}, {3, 4}, {3, 5}, {3, 6}, {4, 5}, {4, 6},
    {5, 6}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}};

int NumCheckersPerPlayer(const Game* game) {
  return static_cast<const CrownyGame*>(game)->NumCheckersPerPlayer();
}

// Facts about the game
const GameType kGameType{
    /*short_name=*/"crowny",
    /*long_name=*/"Backgammon",
    GameType::Dynamics::kSequential,
    GameType::ChanceMode::kExplicitStochastic,
    GameType::Information::kPerfectInformation,
    GameType::Utility::kZeroSum,
    GameType::RewardModel::kTerminal,
    /*min_num_players=*/2,
    /*max_num_players=*/2,
    /*provides_information_state_string=*/false,
    /*provides_information_state_tensor=*/false,
    /*provides_observation_string=*/true,
    /*provides_observation_tensor=*/true,
    /*parameter_specification=*/
    {{"scoring_type",
      GameParameter(static_cast<std::string>(kDefaultScoringType))}}};

static std::shared_ptr<const Game> Factory(const GameParameters& params) {
  return std::shared_ptr<const Game>(new CrownyGame(params));
}

REGISTER_SPIEL_GAME(kGameType, Factory);

RegisterSingleTensorObserver single_tensor(kGameType.short_name);
}  // namespace

ScoringType ParseScoringType(const std::string& st_str) {
  if (st_str == "winloss_scoring") {
    return ScoringType::kWinLossScoring;
  } else if (st_str == "full_scoring") {
    return ScoringType::kFullScoring;
  } else {
    SpielFatalError("Unrecognized scoring_type parameter: " + st_str);
  }
}

std::string PositionToString(std::pair<int, int> pos) {
      return absl::StrCat(pos.first) + absl::StrCat(pos.second);
}

std::string CurPlayerToString(Player cur_player) {
  switch (cur_player) {
    case kXPlayerId:
      return "x";
    case kOPlayerId:
      return "o";
    case kChancePlayerId:
      return "*";
    case kTerminalPlayerId:
      return "T";
    default:
      SpielFatalError(absl::StrCat("Unrecognized player id: ", cur_player));
  }
}

std::string PositionToStringHumanReadable(std::pair<int, int> pos) {
    return PositionToString(pos);
}

// int CrownyState::AugmentCheckerMove(CheckerMove* cmove, int player,
//                                         int start) const {
//   int end = cmove->num;
//   if (end != kPassPos) {
//     // Not a pass, so work out where the piece finished
//     end = start - cmove->num;
//     if (end <= 0) {
//       end = kNumOffPosHumanReadable;  // Off
//     } else if (board_[Opponent(player)]
//                      [player == kOPlayerId ? (end - 1) : (kNumPoints - end)] ==
//                1) {
//       cmove->hit = true;  // Check to see if move is a hit
//     }
//   }
//   return end;
// }

std::string CrownyState::ActionToString(Player player,
                                            Action move_id) const {
                                                //TODO: implement
//   if (player == kChancePlayerId) {
//     if (turns_ >= 0) {
//       // Normal chance roll.
//       return absl::StrCat("chance outcome ", move_id,
//                           " (roll: ", kChanceOutcomeValues[move_id][0],
//                           kChanceOutcomeValues[move_id][1], ")");
//     } else {
//       // Initial roll to determine who starts.
//       const char* starter = (move_id < kNumNonDoubleOutcomes ?
//                              "X starts" : "O starts");
//       if (move_id >= kNumNonDoubleOutcomes) {
//         move_id -= kNumNonDoubleOutcomes;
//       }
//       return absl::StrCat("chance outcome ", move_id, " ", starter, ", ",
//                           "(roll: ", kChanceOutcomeValues[move_id][0],
//                           kChanceOutcomeValues[move_id][1], ")");
//     }
//   } else {
//     // Assemble a human-readable string representation of the move using
//     // standard backgammon notation:
//     //
//     // - Always show the numbering going from Bar->24->0->Off, irrespective of
//     //   which player is moving.
//     // - Show the start position followed by end position.
//     // - Show hits with an asterisk, e.g. 9/7*.
//     // - Order the moves by highest number first, e.g. 22/7 10/8 not 10/8 22/7.
//     //   Not an official requirement, but seems to be standard convention.
//     // - Show duplicate moves as 10/8(2).
//     // - Show moves on a single piece as 10/8/5 not 10/8 8/5
//     //
//     // Note that there are tests to ensure the ActionToString follows this
//     // output format. Any changes would need to be reflected in the tests as
//     // well.
//     std::vector<CheckerMove> cmoves = SpielMoveToCheckerMoves(player, move_id);

//     int cmove0_start;
//     int cmove1_start;
//     if (player == kOPlayerId) {
//       cmove0_start = cmoves[0].pos + 1;
//       cmove1_start = cmoves[1].pos + 1;
//     } else {
//       // swap the board numbering round for Player X so player is moving
//       // from 24->0
//       cmove0_start = kNumPoints - cmoves[0].pos;
//       cmove1_start = kNumPoints - cmoves[1].pos;
//     }

//     // Add hit information and compute whether the moves go off the board.
//     int cmove0_end = AugmentCheckerMove(&cmoves[0], player, cmove0_start);
//     int cmove1_end = AugmentCheckerMove(&cmoves[1], player, cmove1_start);

//     std::string returnVal = "";
//     if (cmove0_start == cmove1_start &&
//         cmove0_end == cmove1_end) {     // same move, show as (2).
//       if (cmoves[1].num == kPassPos) {  // Player can't move at all!
//         returnVal = "Pass";
//       } else {
//         returnVal = absl::StrCat(move_id, " - ",
//                                  PositionToStringHumanReadable(cmove0_start),
//                                  "/", PositionToStringHumanReadable(cmove0_end),
//                                  cmoves[0].hit ? "*" : "", "(2)");
//       }
//     } else if ((cmove0_start < cmove1_start ||
//                 (cmove0_start == cmove1_start && cmove0_end < cmove1_end) ||
//                 cmoves[0].num == kPassPos) &&
//                cmoves[1].num != kPassPos) {
//       // tradition to start with higher numbers first,
//       // so swap moves round if this not the case. If
//       // there is a pass move, put it last.
//       if (cmove1_end == cmove0_start) {
//         // Check to see if the same piece is moving for both
//         // moves, as this changes the format of the output.
//         returnVal = absl::StrCat(
//             move_id, " - ", PositionToStringHumanReadable(cmove1_start), "/",
//             PositionToStringHumanReadable(cmove1_end), cmoves[1].hit ? "*" : "",
//             "/", PositionToStringHumanReadable(cmove0_end),
//             cmoves[0].hit ? "*" : "");
//       } else {
//         returnVal = absl::StrCat(
//             move_id, " - ", PositionToStringHumanReadable(cmove1_start), "/",
//             PositionToStringHumanReadable(cmove1_end), cmoves[1].hit ? "*" : "",
//             " ",
//             (cmoves[0].num != kPassPos)
//                 ? PositionToStringHumanReadable(cmove0_start)
//                 : "",
//             (cmoves[0].num != kPassPos) ? "/" : "",
//             PositionToStringHumanReadable(cmove0_end),
//             (cmoves[0].hit && !double_hit) ? "*" : "");
//       }
//     } else {
//       if (cmove0_end == cmove1_start) {
//         // Check to see if the same piece is moving for both
//         // moves, as this changes the format of the output.
//         returnVal = absl::StrCat(
//             move_id, " - ", PositionToStringHumanReadable(cmove0_start), "/",
//             PositionToStringHumanReadable(cmove0_end), cmoves[0].hit ? "*" : "",
//             "/", PositionToStringHumanReadable(cmove1_end),
//             cmoves[1].hit ? "*" : "");
//       } else {
//         returnVal = absl::StrCat(
//             move_id, " - ", PositionToStringHumanReadable(cmove0_start), "/",
//             PositionToStringHumanReadable(cmove0_end), cmoves[0].hit ? "*" : "",
//             " ",
//             (cmoves[1].num != kPassPos)
//                 ? PositionToStringHumanReadable(cmove1_start)
//                 : "",
//             (cmoves[1].num != kPassPos) ? "/" : "",
//             PositionToStringHumanReadable(cmove1_end),
//             (cmoves[1].hit && !double_hit) ? "*" : "");
//       }
//     }

//     return returnVal;
//   }
}

std::string CrownyState::ObservationString(Player player) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);
  return ToString();
}

void CrownyState::ObservationTensor(Player player,
                                        absl::Span<float> values) const {
    //TODO: implement
//   SPIEL_CHECK_GE(player, 0);
//   SPIEL_CHECK_LT(player, num_players_);

//   int opponent = Opponent(player);
//   SPIEL_CHECK_EQ(values.size(), kStateEncodingSize);
//   auto value_it = values.begin();
//   // The format of this vector is described in Section 3.4 of "G. Tesauro,
//   // Practical issues in temporal-difference learning, 1994."
//   // https://link.springer.com/article/10.1007/BF00992697
//   // The values of the dice are added in the last two positions of the vector.
//   for (int count : board_[player]) {
//     *value_it++ = ((count == 1) ? 1 : 0);
//     *value_it++ = ((count == 2) ? 1 : 0);
//     *value_it++ = ((count == 3) ? 1 : 0);
//     *value_it++ = ((count > 3) ? (count - 3) : 0);
//   }
//   for (int count : board_[opponent]) {
//     *value_it++ = ((count == 1) ? 1 : 0);
//     *value_it++ = ((count == 2) ? 1 : 0);
//     *value_it++ = ((count == 3) ? 1 : 0);
//     *value_it++ = ((count > 3) ? (count - 3) : 0);
//   }
//   *value_it++ = (bar_[player]);
//   *value_it++ = (scores_[player]);
//   *value_it++ = ((cur_player_ == player) ? 1 : 0);

//   *value_it++ = (bar_[opponent]);
//   *value_it++ = (scores_[opponent]);
//   *value_it++ = ((cur_player_ == opponent) ? 1 : 0);

//   *value_it++ = ((!dice_.empty()) ? dice_[0] : 0);
//   *value_it++ = ((dice_.size() > 1) ? dice_[1] : 0);

//   SPIEL_CHECK_EQ(value_it, values.end());
}

CrownyState::CrownyState(std::shared_ptr<const Game> game,
                                 ScoringType scoring_type)
    : State(game),
      scoring_type_(scoring_type),
      cur_player_(kChancePlayerId),
      prev_player_(kChancePlayerId),
      turns_(-1),
      x_turns_(0),
      o_turns_(0),
      double_turn_(false),
      dice_({}),
      scores_({0, 0}),
      board_(11, std::vector(11, std::vector<Piece>()))
{
  SetupInitialBoard();
}

void CrownyState::SetupInitialBoard() {
    for (int row = board_.size() - 3; row < board_.size(); ++row) {
        for (int col = 0; col < 3; ++col) {
            board_[row][col].emplace_back(Pawn(PieceType::ENUM_RED));
        }
    }

    for (auto [row, col] : std::vector<std::pair<int, int>>{{8, 0}, {9, 0}, {10, 0}, {9, 1}, {10, 1}, {10, 2}}) {
        board_[row][col].emplace_back(Pawn(PieceType::ENUM_RED));
    }

    for (auto [row, col] : std::vector<std::pair<int, int>>{{9, 0}, {10, 0}, {9, 1}, {10, 1}}) {
        board_[row][col].emplace_back(Archer(PieceType::ENUM_RED));
    }

    board_[10][0].emplace_back(King(PieceType::ENUM_RED));

    for (int row = 0; row < 3; ++row) {
        for (int col = board_.size() - 3; col < board_.size(); ++col) {
            board_[row][col].emplace_back(Pawn(PieceType::ENUM_BLUE));
        }
    }

    for (auto [row, col] : std::vector<std::pair<int, int>>{{0, 10}, {0, 9}, {0, 8}, {1, 9}, {1, 10}, {2, 10}}) {
        board_[row][col].emplace_back(Pawn(PieceType::ENUM_BLUE));
    }

    for (auto [row, col] : std::vector<std::pair<int, int>>{{0, 9}, {0, 10}, {1, 9}, {1, 10}}) {
        board_[row][col].emplace_back(Archer(PieceType::ENUM_BLUE));
    }

    board_[0][10].emplace_back(King(PieceType::ENUM_BLUE));
}

std::vector<Piece> CrownyState::board(std::pair<int, int> pos) const {
    SPIEL_CHECK_GE(pos.first, 0);
    SPIEL_CHECK_GE(pos.second, 0);
    return board_[pos.first][pos.second];
}

Player CrownyState::CurrentPlayer() const {
  return IsTerminal() ? kTerminalPlayerId : Player{cur_player_};
}

int CrownyState::Opponent(int player) const { return 1 - player; }

void CrownyState::RollDice(int outcome) {
  dice_.push_back(kChanceOutcomeValues[outcome][0]);
  dice_.push_back(kChanceOutcomeValues[outcome][1]);
}

int CrownyState::DiceValue(int i) const {
  SPIEL_CHECK_GE(i, 0);
  SPIEL_CHECK_LT(i, dice_.size());

  if (dice_[i] >= 1 && dice_[i] <= 6) {
    return dice_[i];
  } else if (dice_[i] >= 7 && dice_[i] <= 12) {
    // This die is marked as chosen, so return its proper value.
    // Note: dice are only marked as chosen during the legal moves enumeration.
    return dice_[i] - 6;
  } else {
    SpielFatalError(absl::StrCat("Bad dice value: ", dice_[i]));
  }
}

void CrownyState::DoApplyAction(Action move) {
  if (IsChanceNode()) {
    if (turns_ == -1) {
      // The first chance node determines who goes first: X or O.
      // The move is between 0 and 29 and the range determines whether X starts
      // or O starts. The value is then converted to a number between 0 and 15,
      // which represents the non-double chance outcome that the first player
      // starts with (see RollDice(move) below). These 30 possibilities are
      // constructed in GetChanceOutcomes().
      SPIEL_CHECK_TRUE(dice_.empty());
      if (move < kNumNonDoubleOutcomes) {
        // X starts.
        cur_player_ = prev_player_ = kXPlayerId;
      } else {
        // O Starts
        cur_player_ = prev_player_ = kOPlayerId;
        move -= kNumNonDoubleOutcomes;
      }
      RollDice(move);
      turns_ = 0;
      return;
    } else {
      // Normal chance node.
      SPIEL_CHECK_TRUE(dice_.empty());
      RollDice(move);
      cur_player_ = Opponent(prev_player_);
      return;
    }
  }

  // Normal move action.
  std::vector<CheckerMove> moves = SpielMoveToCheckerMoves(cur_player_, move);
  ApplyCheckerMove(cur_player_, moves[0]);
  ApplyCheckerMove(cur_player_, moves[1]);

  prev_player_ = cur_player_;
}

void CrownyState::UndoAction(int player, Action action) {
    //TODO: implement
//   {
//     SPIEL_CHECK_EQ(thi.player, player);
//     SPIEL_CHECK_EQ(action, thi.action);
//     cur_player_ = thi.player;
//     prev_player_ = thi.prev_player;
//     dice_ = thi.dice;
//     double_turn_ = thi.double_turn;
//     if (player != kChancePlayerId) {
//       std::vector<CheckerMove> moves = SpielMoveToCheckerMoves(player, action);
//       SPIEL_CHECK_EQ(moves.size(), 2);
//       UndoCheckerMove(player, moves[1]);
//       UndoCheckerMove(player, moves[0]);
//       turns_--;
//       if (!double_turn_) {
//         if (player == kXPlayerId) {
//           x_turns_--;
//         } else if (player == kOPlayerId) {
//           o_turns_--;
//         }
//       }
//     }
//   }
//   --move_number_;
}

// bool CrownyState::IsHit(Player player, std::pair<int,int> from_pos, int num) const {
//   if (from_pos != kPassPos) {
//     std::pair<int, int>to = PositionFrom(player, from_pos, num);
//     return to != kScorePos && board(Opponent(player), to) == 1;
//   } else {
//     return false;
//   }
// }

// Action CrownyState::TranslateAction(std::pair<int, int> from1, std::pair<int, int> from2,
//                                         bool use_high_die_first) const {   
// //TOOD: implement
//   int player = CurrentPlayer();
//   int num1 = use_high_die_first ? dice_.at(1) : dice_.at(0);
//   int num2 = use_high_die_first ? dice_.at(0) : dice_.at(1);
//   std::vector<CheckerMove> moves = {{from1, num1}, {from2, num2}};
//   return CheckerMovesToSpielMove(moves);
// }

Action CrownyState::CheckerMovesToSpielMove(
    const std::vector<CheckerMove>& moves) const {
  SPIEL_CHECK_EQ(moves.size(), 2);
  bool high_roll_first = false;
  int high_roll = DiceValue(0) >= DiceValue(1) ? DiceValue(0) : DiceValue(1);

    std::pair<int, int> from1 = moves[0].from;
    std::pair<int, int> to1 = moves[0].to;
    int dig0 = from1.first*11 + from1.second;
    int dig1 = to1.first*11 + to1.second;
    high_roll_first = GetDistance(from1, to1) == high_roll;
    
    std::pair<int, int> from2 = moves[1].from;
    std::pair<int, int> to2 = moves[1].to;
    int dig2 = from2.first*11 + from2.second;
    int dig3 = to2.first*11 + to2.second;

  Action move = dig3 * 121*121*121 + dig2 * 121*121 + dig1 * 121 + dig0;
//   if (!high_roll_first) {
//     move += 121*121*121*121;  
//   }
  SPIEL_CHECK_GE(move, 0);
  SPIEL_CHECK_LT(move, kNumDistinctActions);
  return move;
}

std::vector<CheckerMove> CrownyState::SpielMoveToCheckerMoves(
    int player, Action spiel_move) const {
  SPIEL_CHECK_GE(spiel_move, 0);
  SPIEL_CHECK_LT(spiel_move, kNumDistinctActions);

  std::vector<Action> digits = {spiel_move % 26, spiel_move / 26};
  std::vector<CheckerMove> cmoves;

//TODO: check this (maybe add test the spidel to checker to spiel equals)
  for (int i = 0; i < 2; ++i) {
    auto to_digit = spiel_move % 121;
    spiel_move /= 121;
    auto from_digit = spiel_move % 121;
    spiel_move /= 121;

      cmoves.push_back(CheckerMove(std::pair(from_digit/11, from_digit%11), std::pair(to_digit/11, to_digit%11)));
  }

  return cmoves;
}

// bool CrownyState::IsPosInHome(int player, std::pair<int, int> pos) const {
//   switch (player) {
//     case kXPlayerId:
//       return (pos >= 18 && pos <= 23);
//     case kOPlayerId:
//       return (pos >= 0 && pos <= 5);
//     default:
//       SpielFatalError(absl::StrCat("Unknown player ID: ", player));
//   }
// }

// int CrownyState::CheckersInHome(int player) const {
//   int c = 0;
//   for (int i = 0; i < 6; i++) {
//     c += board(player, (player == kXPlayerId ? (23 - i) : i));
//   }
//   return c;
// }

// bool CrownyState::AllInHome(int player) const {
//   if (bar_[player] > 0) {
//     return false;
//   }

//   SPIEL_CHECK_GE(player, 0);
//   SPIEL_CHECK_LE(player, 1);

//   // Looking for any checkers outside home.
//   // --> XPlayer scans 0-17.
//   // --> OPlayer scans 6-23.
//   int scan_start = (player == kXPlayerId ? 0 : 6);
//   int scan_end = (player == kXPlayerId ? 17 : 23);

//   for (int i = scan_start; i <= scan_end; ++i) {
//     if (board_[player][i] > 0) {
//       return false;
//     }
//   }

//   return true;
// }

int CrownyState::HighestUsableDiceOutcome() const {
  if (UsableDiceOutcome(dice_[1])) {
    return dice_[1];
  } else if (UsableDiceOutcome(dice_[0])) {
    return dice_[0];
  } else {
    return -1;
  }
}

// int CrownyState::FurthestCheckerInHome(int player) const {
//   // Looking for any checkers in home.
//   // --> XPlayer scans 23 -> 18
//   // --> OPlayer scans  0 -> 5
//   int scan_start = (player == kXPlayerId ? 23 : 0);
//   int scan_end = (player == kXPlayerId ? 17 : 6);
//   int inc = (player == kXPlayerId ? -1 : 1);

//   int furthest = (player == kXPlayerId ? 24 : -1);

//   for (int i = scan_start; i != scan_end; i += inc) {
//     if (board_[player][i].size() > 0) {
//       furthest = i;
//     }
//   }

//   if (furthest == 24 || furthest == -1) {
//     return -1;
//   } else {
//     return furthest;
//   }
// }

bool CrownyState::UsableDiceOutcome(int outcome) const {
  return (outcome >= 1 && outcome <= 6);
}

// int CrownyState::NumOppCheckers(int player, std::pair<int, int> pos) const {
//   return board_[Opponent(player)][pos];
// }

int CrownyState::GetDistance(std::pair<int, int>from, std::pair<int, int>to) const {
  SPIEL_CHECK_NE(from.first, kScorePos);
  SPIEL_CHECK_NE(from.second, kScorePos);
  SPIEL_CHECK_NE(to.first, kScorePos);
  SPIEL_CHECK_NE(to.second, kScorePos);

  return std::max(std::abs(to.second - from.second), std::abs(to.first - from.first));
}

bool CrownyState::IsOff(std::pair<int, int> pos) const {
  return pos.first < 0 || pos.first >= board_.size() || pos.second < 0 || pos.second >= board_.size();
}

bool CrownyState::IsFurther(int player, std::pair<int, int> pos1, std::pair<int, int> pos2) const {
  if (pos1 == pos2) {
    return false;
  }

  return ((player == kXPlayerId && pos1 < pos2) ||
          (player == kOPlayerId && pos1 > pos2));
}

// int CrownyState::GetToPos(int player, std::pair<int,int> from_pos, int pips) const {
//   if (player == kXPlayerId) {
//     return (from_pos == kBarPos ? -1 : from_pos) + pips;
//   } else if (player == kOPlayerId) {
//     return (from_pos == kBarPos ? 24 : from_pos) - pips;
//   } else {
//     SpielFatalError(absl::StrCat("Player (", player, ") unrecognized."));
//   }
// }

// Basic from_to check (including bar checkers).
bool CrownyState::IsLegalFromTo(int player, std::pair<int,int> from_pos, std::pair<int,int> to_pos,
                                    int my_checkers_from,
                                    int opp_checkers_to) const {
  // Must have at least one checker the from position.
  if (my_checkers_from == 0) {
    return false;
  }

  if (opp_checkers_to > 1) {
    return false;
  }

  return true;
}

std::string CrownyState::DiceToString(int outcome) const {
  if (outcome > 6) {
    return std::to_string(outcome - 6) + "u";
  } else {
    return std::to_string(outcome);
  }
}

std::set<CheckerMove> CrownyState::LegalCheckerMoves(int player) const {
    //TODO: implement
//   std::set<CheckerMove> moves;

//   // Regular board moves.
//   bool all_in_home = AllInHome(player);
//   for (int i = 0; i < kNumPoints; ++i) {
//     if (board_[player][i].size() > 0) {
//       for (int outcome : dice_) {
//         if (UsableDiceOutcome(outcome)) {
//           std::pair<int, int> pos = PositionFrom(player, i, outcome);
//           if (pos == kScorePos && all_in_home) {
//             // Check whether a bear off move is legal.

//             // It is ok to bear off if all the checkers are at home and the
//             // point being used to move from exactly matches the distance from
//             // just stepping off the board.
//             if ((player == kXPlayerId && i + outcome == 24) ||
//                 (player == kOPlayerId && i - outcome == -1)) {
//               moves.insert(CheckerMove(i, outcome, false));
//             } else {
//               // Otherwise, a die can only be used to move a checker off if
//               // there are no checkers further than it in the player's home.
//               if (i == FurthestCheckerInHome(player)) {
//                 moves.insert(CheckerMove(i, outcome, false));
//               }
//             }
//           } else if (pos != kScorePos && NumOppCheckers(player, pos) <= 1) {
//             // Regular move.
//             bool hit = NumOppCheckers(player, pos) == 1;
//             moves.insert(CheckerMove(i, outcome, hit));
//           }
//         }
//       }
//     }
//   }
//   return moves;
}

std::pair<int, int> CrownyState::PositionFrom(int player, std::pair<int, int> from, std::pair<int, int> to) const {
    return to; //TODO: fix 
//   if (player == kXPlayerId) {
//     int new_pos = pos + spaces;
//     return (new_pos > 23 ? kScorePos : new_pos);
//   } else if (player == kOPlayerId) {
//     int new_pos = pos - spaces;
//     return (new_pos < 0 ? kScorePos : new_pos);
//   } else {
//     SpielFatalError(absl::StrCat("Invalid player: ", player));
//   }
}

void CrownyState::ApplyCheckerMove(int player, const CheckerMove& move) {

  // First, remove the checker.
  std::pair<int, int> next_pos;

    auto piece = board_[move.from.first][move.from.second].back();
    board_[move.from.first][move.from.second].pop_back();
    next_pos = PositionFrom(player, move.from, move.to);

  // Mark the die as used.
  for (int i = 0; i < 2; ++i) {
    if (dice_[i] == GetDistance(move.from, move.to)) {
      dice_[i] += 6;
      break;
    }
  }

  // Now add the checker (or score).
    board_[move.to.first][move.to.second].push_back(piece);
  
}

// Undoes a checker move. Important note: this checkermove needs to have
// move.hit set from the history to properly undo a move (this information is
// not tracked in the action value).
void CrownyState::UndoCheckerMove(int player, const CheckerMove& move) {

  // First, figure out the next position.
  std::pair<int, int> next_pos;
  
  // Remove the moved checker or decrement score.

    Piece piece = board_[move.to.first][move.to.second].back();
    board_[move.to.first][move.to.second].pop_back();
  

  // Mark the die as unused.
  for (int i = 0; i < 2; ++i) {
    if (dice_[i] == GetDistance(move.from, move.to) + 6) {
      dice_[i] -= 6;
      break;
    }
  }

    board_[move.from.first][move.from.second].push_back(piece);
  
}

// Returns the maximum move size (2, 1, or 0)
int CrownyState::RecLegalMoves(
    std::vector<CheckerMove> moveseq,
    std::set<std::vector<CheckerMove>>* movelist) {
  if (moveseq.size() == 2) {
    movelist->insert(moveseq);
    return moveseq.size();
  }

  std::set<CheckerMove> moves_here = LegalCheckerMoves(cur_player_);

  if (moves_here.empty()) {
    movelist->insert(moveseq);
    return moveseq.size();
  }

  int max_moves = -1;
  for (const auto& move : moves_here) {
    moveseq.push_back(move);
    ApplyCheckerMove(cur_player_, move);
    int child_max = RecLegalMoves(moveseq, movelist);
    UndoCheckerMove(cur_player_, move);
    max_moves = std::max(child_max, max_moves);
    moveseq.pop_back();
  }

  return max_moves;
}

std::vector<Action> CrownyState::ProcessLegalMoves(
    int max_moves, const std::set<std::vector<CheckerMove>>& movelist) const {
//TODO: implement
//   // Rule 2 in Movement of Checkers:
//   // A player must use both numbers of a roll if this is legally possible (or
//   // all four numbers of a double). When only one number can be played, the
//   // player must play that number. Or if either number can be played but not
//   // both, the player must play the larger one. When neither number can be used,
//   // the player loses his turn. In the case of doubles, when all four numbers
//   // cannot be played, the player must play as many numbers as he can.
//   std::vector<Action> legal_actions;
//   int max_roll = -1;
//   for (const auto& move : movelist) {
//     if (max_moves == 2) {
//       // Only add moves that are size 2.
//       if (move.size() == 2) {
//         legal_actions.push_back(CheckerMovesToSpielMove(move));
//       }
//     } else if (max_moves == 1) {
//       // We are just finding the maximum roll.
//       max_roll = std::max(max_roll, move[0].num);
//     }
//   }

//   if (max_moves == 1) {
//     // Another round to add those that have the max die roll.
//     for (const auto& move : movelist) {
//       if (move[0].num == max_roll) {
//         legal_actions.push_back(CheckerMovesToSpielMove(move));
//       }
//     }
//   }

//   SPIEL_CHECK_FALSE(legal_actions.empty());
//   return legal_actions;
}

std::vector<Action> CrownyState::LegalActions() const {
  if (IsChanceNode()) return LegalChanceOutcomes();
  if (IsTerminal()) return {};

  std::unique_ptr<State> cstate = this->Clone();
  CrownyState* state = dynamic_cast<CrownyState*>(cstate.get());
  std::set<std::vector<CheckerMove>> movelist;
  int max_moves = state->RecLegalMoves({}, &movelist);
  SPIEL_CHECK_GE(max_moves, 0);
  SPIEL_CHECK_LE(max_moves, 2);
  std::vector<Action> legal_actions = ProcessLegalMoves(max_moves, movelist);
  std::sort(legal_actions.begin(), legal_actions.end());
  return legal_actions;
}

std::vector<std::pair<Action, double>> CrownyState::ChanceOutcomes() const {
  SPIEL_CHECK_TRUE(IsChanceNode());
  if (turns_ == -1) {
    // Doubles not allowed for the initial roll to determine who goes first.
    // Range 0-14: X goes first, range 15-29: O goes first.
    std::vector<std::pair<Action, double>> outcomes;
    int num_outcomes = kNumNonDoubleOutcomes * 2;
    outcomes.reserve(num_outcomes);
    const double uniform_prob = 1.0 / num_outcomes;
    for (Action action = 0; action < num_outcomes; ++action) {
      outcomes.push_back({action, uniform_prob});
    }
    return outcomes;
  } else {
    return kChanceOutcomes;
  }
}

std::string CrownyState::ToString() const {
    //TODO: implement
//   std::vector<std::string> board_array = {
//       "+------|------+", "|......|......|", "|......|......|",
//       "|......|......|", "|......|......|", "|......|......|",
//       "|      |      |", "|......|......|", "|......|......|",
//       "|......|......|", "|......|......|", "|......|......|",
//       "+------|------+"};

//   // Fill the board.
//   for (std::pair<int, int> pos = 0; pos < 24; pos++) {
//     if (board_[kXPlayerId][pos] > 0 || board_[kOPlayerId][pos] > 0) {
//       int start_row = (pos < 12 ? 11 : 1);
//       int col = (pos < 12 ? (pos >= 6 ? 12 - pos : 13 - pos)
//                           : (pos < 18 ? pos - 11 : pos - 10));

//       int row_offset = (pos < 12 ? -1 : 1);

//       int owner = board_[kXPlayerId][pos] > 0 ? kXPlayerId : kOPlayerId;
//       char piece = (owner == kXPlayerId ? 'x' : 'o');
//       int my_checkers = board_[owner][pos];

//       for (int i = 0; i < 5 && i < my_checkers; i++) {
//         board_array[start_row + i * row_offset][col] = piece;
//       }

//       // Check for special display of >= 10 and >5 pieces
//       if (my_checkers >= 10) {
//         char lsd = std::to_string(my_checkers % 10)[0];
//         // Make sure it reads downward.
//         if (pos < 12) {
//           board_array[start_row + row_offset][col] = '1';
//           board_array[start_row][col] = lsd;
//         } else {
//           board_array[start_row][col] = '1';
//           board_array[start_row + row_offset][col] = lsd;
//         }
//       } else if (my_checkers > 5) {
//         board_array[start_row][col] = std::to_string(my_checkers)[0];
//       }
//     }
//   }

//   std::string board_str = absl::StrJoin(board_array, "\n") + "\n";

//   // Extra info like whose turn it is etc.
//   absl::StrAppend(&board_str, "Turn: ");
//   absl::StrAppend(&board_str, CurPlayerToString(cur_player_));
//   absl::StrAppend(&board_str, "\n");
//   absl::StrAppend(&board_str, "Dice: ");
//   absl::StrAppend(&board_str, !dice_.empty() ? DiceToString(dice_[0]) : "");
//   absl::StrAppend(&board_str, dice_.size() > 1 ? DiceToString(dice_[1]) : "");
//   absl::StrAppend(&board_str, "\n");
//   absl::StrAppend(&board_str, "Bar:");
//   absl::StrAppend(&board_str,
//                   (bar_[kXPlayerId] > 0 || bar_[kOPlayerId] > 0 ? " " : ""));
//   for (int p = 0; p < 2; p++) {
//     for (int n = 0; n < bar_[p]; n++) {
//       absl::StrAppend(&board_str, (p == kXPlayerId ? "x" : "o"));
//     }
//   }
//   absl::StrAppend(&board_str, "\n");
//   absl::StrAppend(&board_str, "Scores, X: ", scores_[kXPlayerId]);
//   absl::StrAppend(&board_str, ", O: ", scores_[kOPlayerId], "\n");

//   return board_str;
}

bool CrownyState::IsTerminal() const {
  return (scores_[kXPlayerId] == NumCheckersPerPlayer(game_.get()) ||
          scores_[kOPlayerId] == NumCheckersPerPlayer(game_.get()));
}

std::vector<double> CrownyState::Returns() const {
  int winner = -1;
  int loser = -1;
  if (scores_[kXPlayerId] == 15) {
    winner = kXPlayerId;
    loser = kOPlayerId;
  } else if (scores_[kOPlayerId] == 15) {
    winner = kOPlayerId;
    loser = kXPlayerId;
  } else {
    return {0.0, 0.0};
  }

  // Magnify the util based on the scoring rules for this game.
  int util_mag = 1;
  switch (scoring_type_) {
    case ScoringType::kWinLossScoring:
    default:
      break;

    case ScoringType::kFullScoring:
      util_mag = 1;
      break;
  }

  std::vector<double> returns(kNumPlayers);
  returns[winner] = util_mag;
  returns[loser] = -util_mag;
  return returns;
}

std::unique_ptr<State> CrownyState::Clone() const {
  return std::unique_ptr<State>(new CrownyState(*this));
}

void CrownyState::SetState(int cur_player, bool double_turn,
                               const std::vector<int>& dice,
                               const std::vector<int>& bar,
                               const std::vector<int>& scores,
                               const std::vector<std::vector<std::vector<Piece>>>& board) {
  cur_player_ = cur_player;
  double_turn_ = double_turn;
  dice_ = dice;
  bar_ = bar;
  scores_ = scores;
  board_ = board;

}

CrownyGame::CrownyGame(const GameParameters& params)
    : Game(kGameType, params),
      scoring_type_(
          ParseScoringType(ParameterValue<std::string>("scoring_type"))) {}

double CrownyGame::MaxUtility() const {

  switch (scoring_type_) {
    case ScoringType::kWinLossScoring:
      return 1;
    case ScoringType::kFullScoring:
      return 2;
    default:
      SpielFatalError("Unknown scoring_type");
  }
}

int CrownyGame::NumCheckersPerPlayer() const {
    return kNumCheckersPerPlayer;
}

}  // namespace crowny
}  // namespace open_spiel
