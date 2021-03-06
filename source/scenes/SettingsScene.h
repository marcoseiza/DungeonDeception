#ifndef SCENES_SETTINGS_SCENE_H_
#define SCENES_SETTINGS_SCENE_H_

#include <cugl/cugl.h>

#include "../controllers/PlayerController.h"

class SettingsScene {
 public:
  /**
   * An enumeration of the end state choices the settings menu can end up with.
   */
  enum Choice {
    /** Default. */
    NONE,
    /** Leave the game. */
    LEAVE,
    /** Resume the game. */
    RESUME,
  };

 protected:
  /** If the scene is currently active. (i.e. showing) */
  bool _active;

  /** If the pause screen should show the confirmation buttons. */
  bool _confirming_leave;

  /** If a leave button has been pressed. */
  bool _leave_button_pressed;

  /** The player choice to cascade upwards to parent. */
  Choice _choice;

  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** The root node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;

  /** The main leave button. */
  std::shared_ptr<cugl::scene2::Button> _leave_button;
  /** The label for the leave button. */
  std::shared_ptr<cugl::scene2::Label> _leave_button_label;

  /** Confirmation yes for leave button. */
  std::shared_ptr<cugl::scene2::Button> _leave_button_yes;
  /** Confirmation no for leave button. */
  std::shared_ptr<cugl::scene2::Button> _leave_button_no;

  /** A label representing a small prompt durring the confriming leave stage. */
  std::shared_ptr<cugl::scene2::Label> _leave_prompt_label_leave;
  /** A label representing a small prompt durring the confriming leave stage. */
  std::shared_ptr<cugl::scene2::Label> _leave_prompt_label_end;

  /** The resume button to resume gameplay. */
  std::shared_ptr<cugl::scene2::Button> _resume_button;

  /** A reference to the player controller. */
  std::shared_ptr<PlayerController> _player_controller;

 public:
  /** Create an empty Settings Scene. */
  SettingsScene() {}
  /** Delete the Settings Scene and dispose of it. */
  ~SettingsScene() { dispose(); }

  /** Dispose of the settings scene and all it's assets. */
  void dispose();

  /**
   * Initialize this settings scene.
   * @param assets The assets for the game.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Allocate and initialize a new shared pointer of the Settings Scene.
   *
   * @param assets The assets for the game.
   * @return The SettingsScene shared pointer.
   */
  static std::shared_ptr<SettingsScene> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto res = std::make_shared<SettingsScene>();
    return (res->init(assets)) ? res : nullptr;
  }

  /** Update the settings scene and its state. */
  void update();

  /**
   * Get the scene2 node for drawing.
   * @return The scene2 node.
   */
  std::shared_ptr<cugl::scene2::SceneNode> getNode() const { return _node; }

  /**
   * Get the choice of the player to propagate up to parents.
   * @return The choice.
   */
  Choice getChoice() { return _choice; }

  /**
   * Set if the scene is currently active (i.e. showing).
   * @param active The activeness of the scene.
   */
  void setActive(bool active);

  /**
   * Get if the scene is currently active (i.e. showing).
   * @return If the scene is active.
   */
  bool isActive() { return _active; }

  /**
   * Set the player controller for player access.
   * @param player_controller The player controller.
   */
  void setPlayerController(
      const std::shared_ptr<PlayerController>& player_controller) {
    _player_controller = player_controller;
  }

 private:
  /** Internal method for when the leave button is pressed. */
  void leaveButtonListener(const std::string& name, bool down);
  /**
   * Internal method for when a confirm leave button is pressed.
   * @param leave If the player confirmed to leave or not.
   */
  void leaveButtonConfirmListener(bool leave);
};

#endif  // SCENES_SETTINGS_SCENE_H_