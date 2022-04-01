#ifndef SCENES_VOTING_SCENES_WAIT_FOR_PLAYERS_H_
#define SCENES_VOTING_SCENES_WAIT_FOR_PLAYERS_H_

#include <cugl/cugl.h>

#include "../../models/Player.h"

class WaitForPlayersScene {
  /** If the scene has been initialized */
  bool _initialized;

  /** If the scene is currently active. */
  bool _active;

  /** If the scene is done. */
  bool _done;

  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** A reference to the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;

  /** The number of people required to activate the terminal. */
  int _num_players_req;

  /** The current number of players present. */
  int _curr_num_players;

  /** A timer used to define how long to wait after the number of required
   * people is accomplished. */
  int _buffer_time_after_required;

 public:
  WaitForPlayersScene()
      : _num_players_req(-1),
        _curr_num_players(0),
        _buffer_time_after_required(0),
        _active(false),
        _done(false),
        _initialized(false) {}
  ~WaitForPlayersScene() { dispose(); }

  /**
   * Initialize a wait for player scene.
   *
   * @param assets The assets for the game.
   * */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Allocate a new wait for player scene.
   *
   * @param assets The assets for the game.
   * @return A shared pointer of the initialized wait for players scene.
   */
  static std::shared_ptr<WaitForPlayersScene> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto result = std::make_shared<WaitForPlayersScene>();
    if (result->init(assets)) return result;
    return nullptr;
  }

  /** Dispose of this WaitForPlayersScene. */
  void dispose() {
    _active = false;
    _done = false;
    _num_players_req = -1;
    _curr_num_players = 0;
    _buffer_time_after_required = 0;
    _node->setVisible(false);
  }

  /**
   * Start this WaitForPlayersScene
   * @param num_players_req The number of players required for this terminal.
   */
  void start(int num_players_req);

  /** Update the wait for players scene. */
  void update();

  /** Return the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> getNode() { return _node; }

  /** If the scene is currently active. */
  bool isActive() { return _active; }

  /** If the scene is done. */
  bool isDone() { return _done; }

  /**
   * Set the current number of players on the terminal.
   * @param num The current number of players on the terminal.
   */
  void setCurrentNumPlayers(int num) { _curr_num_players = num; }
};

#endif  // SCENES_VOTING_SCENES_WAIT_FOR_PLAYERS_H_