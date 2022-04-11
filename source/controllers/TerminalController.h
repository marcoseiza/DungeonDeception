#ifndef CONTROLLERS_TERMINAL_CONTROLLER_H_
#define CONTROLLERS_TERMINAL_CONTROLLER_H_

#include <cugl/cugl.h>

#include "../models/Player.h"
#include "../scenes/voting_scenes/WaitForPlayersScene.h"
#include "Controller.h"
#include "InputController.h"
#include "LevelController.h"

class TerminalController : public Controller {
 public:
  struct VotingInfo {
    int terminal_room_id;
    std::vector<int> players;
  };

 private:
  /** If a terminal is currently being voted on. */
  bool _active;

  /** A map between the terminal room id and the voting info. */
  std::unordered_map<int, VotingInfo> _voting_info;

  /** A reference to the terminal voting scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _scene;

  /** A reference to the waiting for players scene. */
  std::shared_ptr<WaitForPlayersScene> _wait_for_players_scene;

  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** The list of players participating in the voting. */
  std::vector<std::shared_ptr<Player>> _players;

  /** The number of players required for this terminal. */
  int _num_players_req;

  /** The terminal room this controller is handleing. */
  int _terminal_room_id;

  /** The terminal voting stage. */
  enum Stage {
    NONE,
    WAIT_FOR_PLAYERS,
    VOTE_LEADER,
    VOTE_TEAM,
    ACTIVATE_TERMINAL
  } _stage;

 public:
  TerminalController() : _stage(Stage::NONE) {}
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
    result->_active = false;
    result->init(assets);
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

  /**
   * Set the terminal controller as active due to a terminal being hit.
   *
   * @param terminal_room_id The room this controller will handle.
   * @param num_players_req The number of players required for this terminal.
   */
  void setActive(int terminal_room_id, int num_players_req) {
    if (_active) return;
    _active = true;
    _num_players_req = num_players_req;
    _terminal_room_id = terminal_room_id;
    _scene->setVisible(true);
    InputController::get()->pause();
  }

  /**
   * Add this player to the terminal voting scene. Will only add if the player
   * is not currently in the scene and the terminal is waiting for players.
   * @param player The player to add.
   */
  void addPlayer(std::shared_ptr<Player> player) {
    if (_stage != Stage::WAIT_FOR_PLAYERS) return;

    if (std::find(_players.begin(), _players.end(), player) == _players.end()) {
      _players.push_back(player);
    }
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
  void processNetworkData(Sint32 code, cugl::NetworkDeserializer::Message msg);

  /**
   * Get the state of all the voting info. Returns an unordered map with the
   * key being the terminal room id and the value being the voting info.
   *
   * @return An unordered map with the key being the terminal room id and the
   * value being the voting info.
   */
  std::unordered_map<int, VotingInfo> getVotingInfo() { return _voting_info; }

 private:
  /** Called when the terminal voting is done. */
  void done() {
    _active = false;
    _scene->setVisible(false);
    InputController::get()->resume();
  }
};

#endif  // CONTROLLERS_TERMINAL_CONTROLLER_H_