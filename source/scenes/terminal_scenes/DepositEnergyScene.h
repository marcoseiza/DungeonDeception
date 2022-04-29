#ifndef SCENES_TERMINAL_SCENES_DEPOSIT_ENERGY_SCENE_H
#define SCENES_TERMINAL_SCENES_DEPOSIT_ENERGY_SCENE_H

#include <cugl/cugl.h>

#include "../../controllers/PlayerController.h"
#include "../../controllers/LevelController.h"
#include "../../models/Player.h"

class DepositEnergyScene {
  
  /** If the scene has been initialized */
  bool _initialized;

  /** If the scene is currently active. */
  bool _active;

  /** If the scene is done. */
  bool _done;
  
  /** If the scene should exit. */
  bool _exit;
  
  /** The room id for the terminal */
  int _terminal_room_id;

  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** A reference to the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;
  
  /** A reference to the energy bar for this scene. */
  std::shared_ptr<cugl::scene2::ProgressBar> _energy_bar;
  
  /** A reference to the corrupted energy bar for this scene. */
  std::shared_ptr<cugl::scene2::ProgressBar> _corrupted_energy_bar;

  /** The deposit energy button */
  std::shared_ptr<cugl::scene2::Button> _deposit_butt;
  
  /** The done  button */
  std::shared_ptr<cugl::scene2::Button> _done_butt;

  /** A reference to the player controller. */
  std::shared_ptr<PlayerController> _player_controller;
  
  /** A reference to the level controller. */
  std::shared_ptr<LevelController> _level_controller;

 public:
  DepositEnergyScene()
      : _active(false),
        _done(false),
        _initialized(false) {}
  ~DepositEnergyScene() { dispose(); }

  /**
   * Initialize a wait for player scene.
   *
   * @param assets The assets for the game.
   * */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Allocate a new deposit energy scene.
   *
   * @param assets The assets for the game.
   * @return A shared pointer of the initialized wait for players scene.
   */
  static std::shared_ptr<DepositEnergyScene> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto result = std::make_shared<DepositEnergyScene>();
    if (result->init(assets)) return result;
    return nullptr;
  }

  /** Dispose of this DepositEnergyScene. */
  void dispose() {
    _active = false;
    _exit = false;
    _done = false;
    _deposit_butt->setVisible(true);
    _deposit_butt->clearListeners();
    _deposit_butt->deactivate();
    _done_butt->setVisible(true);
    _done_butt->clearListeners();
    _done_butt->deactivate();
    _node->setVisible(false);
  }

  /**
   * Start this deposit energy scene.
   */
  void start(int terminal_room_id);

  /** Update the wait for players scene. */
  void update();

  /** Return the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> getNode() { return _node; }

  /** If the scene is currently active. */
  bool isActive() { return _active; }

  /** If the scene is done. */
  bool isDone() { return _done; }

  /** Set whether the scene is done. */
  void setDone(bool val) { _done = val; }
  
  /** If the player exited the scene. */
  bool didExit() { return _exit; }

  void setPlayerController(
      const std::shared_ptr<PlayerController>& player_controller) {
    _player_controller = player_controller;
  }
  
  void setLevelController(
      const std::shared_ptr<LevelController>& level_controller) {
    _level_controller = level_controller;
  }

  void depositButtonListener(const std::string& name, bool down);
  
  void doneButtonListener(const std::string& name, bool down);
};

#endif  // SCENES_SCAR_SCENES_CHOOSE_SCAR_SCENE_H_
