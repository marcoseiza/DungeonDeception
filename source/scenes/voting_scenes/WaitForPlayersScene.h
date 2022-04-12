#ifndef SCENES_VOTING_SCENES_WAIT_FOR_PLAYER_SCENE_H_
#define SCENES_VOTING_SCENES_WAIT_FOR_PLAYER_SCENE_H_

#include <cugl/cugl.h>

#include "../../controllers/VotingInfo.h"
#include "../../models/Player.h"

class WaitForPlayersScene {
  // The voting info for this terminal.
  std::shared_ptr<VotingInfo> _voting_info;

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

 public:
  WaitForPlayersScene()
      : _num_players_req(-1),
        _curr_num_players(0),
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
    _node->setVisible(false);
  }

  /**
   * Start this WaitForPlayersScene
   * @param num_players_req The number of players required for this terminal.
   */
  void start(std::shared_ptr<VotingInfo> voting_info, int num_players_req);

  /** Update the wait for players scene. */
  void update();

  /** Return the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> getNode() { return _node; }

  /** If the scene is currently active. */
  bool isActive() { return _active; }

  /** If the scene is done. */
  bool isDone() { return _done; }
};

#endif  // SCENES_VOTING_SCENES_WAIT_FOR_PLAYER_SCENE_H_
