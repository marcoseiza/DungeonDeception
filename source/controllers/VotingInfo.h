#ifndef CONTROLLERS_VOTING_INFO_H_
#define CONTROLLERS_VOTING_INFO_H_

#define WAIT_TIME_AFTER_REQUIRED_ACCOMPLISHED 600

struct VotingInfo {
  /** The terminal room id. */
  int terminal_room_id;
  /** Terminal */
  bool terminal_done;
  /** true if terminal was activated, false if corrupted. */
  bool was_activated;
  /** The number of players required for the terminal. */
  int num_players_req;
  /** The buffer timer between waiting for players and starting the vote. */
  int buffer_timer;
  /** The players participating. */
  std::vector<int> players;
  /** A map that represents the votes of each player for other players. */
  std::unordered_map<int, std::vector<int>> votes;
  /** A list of all players done with voting (pressed read). */
  std::vector<int> done;

  VotingInfo()
      : terminal_room_id(-1),
        terminal_done(false),
        was_activated(true),
        num_players_req(0),
        buffer_timer(0) {}
};

#endif  // CONTROLLERS_VOTING_INFO_H_
