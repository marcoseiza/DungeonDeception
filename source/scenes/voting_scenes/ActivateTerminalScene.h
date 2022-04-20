#ifndef SCENES_VOTING_SCENES_ACTIVATE_TERMINAL_SCENE_H_
#define SCENES_VOTING_SCENES_ACTIVATE_TERMINAL_SCENE_H_

#include <cugl/cugl.h>

#include "../../controllers/PlayerController.h"
#include "../../controllers/VotingInfo.h"
#include "../../models/Player.h"

class ActivateTerminalScene {
  // The voting info for this terminal.
  std::shared_ptr<VotingInfo> _voting_info;

  /** The room id for this terminal. */
  int _terminal_room_id;

  /** The number of players required for this terminal. */
  int _num_players_req;

  /** If the scene has been initialized */
  bool _initialized;

  /** If the scene is currently active. */
  bool _active;

  /** If the scene is done. */
  bool _done;

  /** If this person is a betrayer. */
  bool _is_betrayer;

  /** If the player activated the terminal, will be false if corrupted. */
  bool _did_activate;

  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** A reference to the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;

  /** The activate button */
  std::shared_ptr<cugl::scene2::Button> _activate_butt;
  /** The corrupt button */
  std::shared_ptr<cugl::scene2::Button> _corrupt_butt;

  /** A reference to the player controller. */
  std::shared_ptr<PlayerController> _player_controller;

 public:
  ActivateTerminalScene()
      : _active(false),
        _done(false),
        _initialized(false),
        _is_betrayer(false),
        _did_activate(false) {}
  ~ActivateTerminalScene() { dispose(); }

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
  static std::shared_ptr<ActivateTerminalScene> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto result = std::make_shared<ActivateTerminalScene>();
    if (result->init(assets)) return result;
    return nullptr;
  }

  /** Dispose of this ActivateTerminalScene. */
  void dispose() {
    _active = false;
    _done = false;
    _is_betrayer = false;
    _did_activate = false;
    _activate_butt->setVisible(true);
    _corrupt_butt->setVisible(true);
    _activate_butt->clearListeners();
    _corrupt_butt->clearListeners();
    _node->setVisible(false);
  }

  /**
   * Start this ActivateTerminalScene
   */
  void start(std::shared_ptr<VotingInfo> voting_info, int terminal_room_id,
             int num_players_req);

  /** Update the wait for players scene. */
  void update();

  /** Return the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> getNode() { return _node; }

  /** If the scene is currently active. */
  bool isActive() { return _active; }

  /** If the scene is done. */
  bool isDone() { return _done; }

  /** If the scene is done. */
  void setDone(bool val) { _done = val; }

  void setPlayerController(
      const std::shared_ptr<PlayerController>& player_controller) {
    _player_controller = player_controller;
  }

  /** True if the player activated the terminal, false if they corrupted it. */
  bool didActivate() { return _did_activate; }

  void buttonListener(const std::string& name, bool down);
};

#endif  // SCENES_VOTING_SCENES_ACTIVATE_TERMINAL_SCENE_H_
