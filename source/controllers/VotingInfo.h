#ifndef CONTROLLERS_VOTING_INFO_H_
#define CONTROLLERS_VOTING_INFO_H_

struct VotingInfo {
  /** The terminal room id. */
  int terminal_room_id;
  /** The players participating. */
  std::vector<int> players;
  /** A map that represents the votes of each player for other players. */
  std::unordered_map<int, std::vector<int>> votes;
  /** A list of all players done with voting (pressed read). */
  std::vector<int> done;
};

#endif  // CONTROLLERS_VOTING_INFO_H_
