#ifndef SCENES_MENU_SCENE_H_
#define SCENES_MENU_SCENE_H_
#include <cugl/cugl.h>

#include <vector>

/**
 * This class presents the menu to the player.
 */
class MenuScene : public cugl::Scene2 {
 public:
  /**
   * The menu choice made by the user.
   */
  enum Choice {
    /** User has not yet made a choice */
    NONE,
    /** User wants to host a game */
    HOST,
    /** User wants to join a game */
    JOIN,
    /** User wants to see how to play */
    HTP,
  };

 protected:
  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** The menu button for hosting a game */
  std::shared_ptr<cugl::scene2::Button> _hostbutton;
  /** The menu button for joining a game */
  std::shared_ptr<cugl::scene2::Button> _joinbutton;
  /** The menu button for how to play */
  std::shared_ptr<cugl::scene2::Button> _htpbutton;
  /** Reference to the cloud layer scene graph. */
  std::shared_ptr<cugl::scene2::SceneNode> _cloud_layer;
  /** The x position of the cloud. */
  float _cloud_x_pos = 0;
  /** The player menu choice */
  Choice _choice;

 public:
#pragma mark -
#pragma mark Constructors
  /**
   * Creates a new  menu scene with the default values.
   */
  MenuScene() : cugl::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  ~MenuScene() { dispose(); }

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

  /**
   * The method called to update the scene
   *
   * @param timestep  The amount of time (in seconds) since the last frame
   */
  virtual void update(float timestep) override;

  /**
   * Get the cloud X position in the scene
   *
   * @return the x position of the cloud layer
   */
  float getCloudXPosition() { return _cloud_x_pos; };

  /**
   * Sets the cloud X position in the scene
   *
   * @param x the x position to set the cloud to.
   */
  void setCloudXPosition(float x) { _cloud_x_pos = x; };
};

#endif /* SCENES_MENU_SCENE_H_ */
