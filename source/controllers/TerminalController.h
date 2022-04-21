#ifndef CONTROLLERS_TERMINAL_CONTROLLER_H_
#define CONTROLLERS_TERMINAL_CONTROLLER_H_

#include <cugl/cugl.h>

#include "../models/Player.h"
#include "../models/tiles/TerminalSensor.h"
#include "../network/structs/VotingInfo.h"
#include "../scenes/voting_scenes/ActivateTerminalScene.h"
#include "../scenes/voting_scenes/VoteForLeaderScene.h"
#include "../scenes/voting_scenes/VoteForTeamScene.h"
#include "../scenes/voting_scenes/WaitForPlayersScene.h"
#include "Controller.h"
#include "InputController.h"
#include "LevelController.h"
#include "PlayerController.h"

class TerminalController : public Controller {
  /** If a terminal is currently being voted on. */
  bool _active;

  /** The terminal controller just finished all the voting. */
  bool _just_finished;

  /** A map between the terminal room id and the voting info. */
  std::unordered_map<int, std::shared_ptr<VotingInfo>> _voting_info;

  /** A reference to the terminal voting scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _scene;

  /** A reference to the waiting for players scene. */
  std::shared_ptr<WaitForPlayersScene> _wait_for_players_scene;

  /** A reference to the vote for leader scene. */
  std::shared_ptr<VoteForLeaderScene> _vote_for_leader_scene;

  /** A reference to the vote for team scene. */
  std::shared_ptr<VoteForTeamScene> _vote_for_team_scene;

  /** A reference to the activate terminal scene. */
  std::shared_ptr<ActivateTerminalScene> _activate_terminal_scene;

  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** Player Controller */
  std::shared_ptr<PlayerController> _player_controller;

  TerminalSensor* _terminal_sensor;

  /** The number of players required for this terminal. */
  int _num_players_req;

  /** The most recent terminal room id voted at. */
  int _latest_terminal_room_id;

  /** The terminal room this controller is handling. */
  int _terminal_room_id;

  /** The chosen team leader player id. */
  int _leader_id;

  /**
   * True if the terminal was activated, false if the terminal was corrupted.
   */
  bool _terminal_was_activated;

  /** The terminal voting stage. */
  enum Stage {
    NONE = 0,
    WAIT_FOR_PLAYERS = 1,
    VOTE_LEADER = 2,
    VOTE_TEAM = 3,
    ACTIVATE_TERMINAL = 4
  } _stage;

 public:
  TerminalController()
      : _stage(Stage::NONE),
        _active(false),
        _just_finished(false),
        _latest_terminal_room_id(0) {}
  ~TerminalController() { dispose(); }

  /**
   * Initialize a new terminal controller with the given terminal voting scene.
   *
   * @param assets The assets for the game.
   * */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Allocate a new terminal controller with the given terminal voting scene.
   *
   * @param assets The assets for the game.
   * @return A shared pointer of the initialized Terminal Controller.
   */
  static std::shared_ptr<TerminalController> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto result = std::make_shared<TerminalController>();
    InputController::get()->resume();
    if (result->init(assets)) return result;
    return nullptr;
  }

  /** Update the controller state. */
  void update(float timestep) override;

  /** Dispose the controller and all its values. */
  void dispose() override {
    _scene = nullptr;
    _active = false;
  }

  void sendNetworkData();

  /**
   * Set the terminal controller as active due to a terminal being hit.
   *
   * @param terminal_room_id The room this controller will handle.
   * @param num_players_req The number of players required for this terminal.
   */
  void setActive(int terminal_room_id, int num_players_req,
                 TerminalSensor* sensor) {
    if (_active) return;

    // If the voting room has already started.
    if (_voting_info.find(terminal_room_id) != _voting_info.end() &&
        _voting_info[terminal_room_id]->buffer_timer >
            WAIT_TIME_AFTER_REQUIRED_ACCOMPLISHED) {
      return;
    }

    _stage = Stage::WAIT_FOR_PLAYERS;
    _wait_for_players_scene->setDone(false);
    _vote_for_leader_scene->setDone(false);
    _vote_for_team_scene->setDone(false);
    _activate_terminal_scene->setDone(false);

    _active = true;
    _num_players_req = num_players_req;
    _terminal_room_id = terminal_room_id;
    _terminal_sensor = sensor;
    _scene->setVisible(true);
    InputController::get()->pause();
  }

  /**
   * Get the terminal room id this terminal controller is currently handleing.
   * @return The terminal room id this terminal controller is currently
   * handleing.
   */
  int getRoomId() { return _terminal_room_id; }

  /**
   * Process the network information and update the terminal controller data.
   *
   * @param code The message code
   * @param msg The deserialized message
   */
  void processNetworkData(const Sint32& code,
                          const cugl::NetworkDeserializer::Message& msg);

  /**
   * Get the state of all the voting info. Returns an unordered map with the
   * key being the terminal room id and the value being the voting info.
   *
   * @return An unordered map with the key being the terminal room id and the
   * value being the voting info.
   */
  std::unordered_map<int, std::shared_ptr<VotingInfo>> getVotingInfo() {
    return _voting_info;
  }

  void setPlayerController(
      const std::shared_ptr<PlayerController>& player_controller) {
    _player_controller = player_controller;
    _wait_for_players_scene->setPlayerController(_player_controller);
    _vote_for_leader_scene->setPlayerController(_player_controller);
    _vote_for_team_scene->setPlayerController(_player_controller);
    _activate_terminal_scene->setPlayerController(_player_controller);
  }

  /**
   * @return True if the terminal was activated, false if it was corrupted.
   */
  bool getTerminalWasActivated() { return _terminal_was_activated; }

  /**
   * @return If the terminal has just finished voting.
   */
  bool hasJustFinished() {
    bool tmp = _just_finished;
    _just_finished = false;
    return tmp;
  }

  /**
   * Returns the latest terminal the player voted at.
   * @return the latest terminal the player voted at.
   */
  int getLatestTerminalRoomId() const { return _latest_terminal_room_id; }

 private:
  /** Called when the terminal voting is done. */
  void done() {
    _active = false;
    _scene->setVisible(false);
    _stage = Stage::WAIT_FOR_PLAYERS;
    InputController::get()->resume();
    _terminal_sensor->activate();
  }
};

#endif  // CONTROLLERS_TERMINAL_CONTROLLER_H_
