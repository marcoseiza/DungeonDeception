#ifndef SCENES_WIN_SCENE_H_
#define SCENES_WIN_SCENE_H_
#include <cugl/cugl.h>

#include <vector>

/**
 * This class presents the menu to the player.
 */
class WinScene : public cugl::Scene2 {
 public:
  /**
   * The menu choice made by the user.
   */
  enum Choice {
    /** User has not yet made a choice */
    NONE,
    /** User wants to host a game */
    PLAYAGAIN,
    /** User wants to join a game */
    QUIT
  };

 protected:
  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** The menu button for hosting a game */
  std::shared_ptr<cugl::scene2::Button> _playagainbutton;
  /** The menu button for joining a game */
  std::shared_ptr<cugl::scene2::Button> _quitbutton;
  /** The player menu choice */
  Choice _choice;

 public:
#pragma mark -
#pragma mark Constructors
  /**
   * Creates a new  menu scene with the default values.
   */
  WinScene() : cugl::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  ~WinScene() { dispose(); }

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
   * Returns the user's menu choice.
   *
   * @return the user's menu choice.
   */
  Choice getChoice() const { return _choice; }

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

#endif /* SCENES_WIN_SCENE_H_ */
