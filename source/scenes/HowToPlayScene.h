#ifndef SCENES_HOW_TO_PLAY_SCENE_H_
#define SCENES_HOW_TO_PLAY_SCENE_H_
#include <cugl/cugl.h>

#include <vector>

/**
 * This class presents the menu to the player.
 */
class HowToPlayScene : public cugl::Scene2 {
 public:
  /**
   * The menu choice made by the user.
   */
  enum Choice {
    /** User has not yet made a choice */
    NONE,
    /** User goes back to menu */
    GOTOMENU, 
  };

  enum Change {
    /** User has not changed slides */
    NO,
    /** User goes left on how to play */
    LEFT,
    /** User goes left on how to play */
    RIGHT, 
  };

 protected:
  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** The scene node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _scene;
  /** The back button for the menu scene */
  std::shared_ptr<cugl::scene2::Button> _backout;
  /** The left button for the how to play scene */
  std::shared_ptr<cugl::scene2::Button> _left;
  /** The right button for the how to play scene */
  std::shared_ptr<cugl::scene2::Button> _right;
  /** The player menu choice */
  Choice _choice;
  /** The player slide change */
  Change _change;
  /** The current slide the player is on */
  int _current_slide;

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
   * Returns the user's menu choice.
   *
   * @return the user's menu choice.
   */
  Choice getChoice() const { return _choice; }

  /**
   * Initializes the controller contents, i.e. the scene user interface.
   *
   * @param assets    The (loaded) assets for this game mode
   *
   * @return true if the controller is initialized properly, false otherwise.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * The method called to update the how to play menu.
   *
   * @param timestep  The amount of time (in seconds) since the last frame.
   */
  void update(float timestep) override;

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

  /**
   * Sets what slide the how to play scene is on

   * @param value what slide the scene is on
   */
  void setCurrentSlide(int value) { _current_slide = value; }

  /**
   * Returns the current slide.
   *
   * @return the current slide.
   */
  int getCurrentSlide() { return _current_slide; }
};

#endif /* SCENES_HOW_TO_PLAY_SCENE_H_ */
#pragma once
