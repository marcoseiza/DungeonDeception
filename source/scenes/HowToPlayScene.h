#ifndef SCENES_HOW_TO_PLAY_SCENE_H_
#define SCENES_HOW_TO_PLAY_SCENE_H_
#include <cugl/cugl.h>

#include <vector>

/**
 * This class presents the menu to the player.
 */
class HowToPlayScene : public cugl::Scene2 {

 protected:
  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;

 public:
#pragma mark -
#pragma mark Constructors
  /**
   * Creates a new  menu scene with the default values.
   */
  HowToPlayScene() : cugl::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  ~HowToPlayScene() { dispose(); }

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  void dispose() override;

  /**
   * Initializes the controller contents, i.e. the scene user interface.
   *
   * @param assets    The (loaded) assets for this game mode
   *
   * @return true if the controller is initialized properly, false otherwise.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Sets whether the scene is currently active
   *
   * This method should be used to toggle all the UI elements.  Buttons
   * should be activated when it is made active and deactivated when
   * it is not.
   *
   * @param value whether the scene is currently active
   */
  virtual void setActive(bool value) override;
};

#endif /* SCENES_HOW_TO_PLAY_SCENE_H_ */
#pragma once
