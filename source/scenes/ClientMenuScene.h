#ifndef SCENES_CLIENT_MENU_SCENE_H_
#define SCENES_CLIENT_MENU_SCENE_H_
#include <cugl/cugl.h>

#include <vector>

#include "PeerScene.h"

/**
 * This class provides the interface to join an existing game.
 *
 * This class is an implementation of PeerScene.
 */
class ClientMenuScene : public PeerScene {
 protected:
  /** The game id label (for updating) */
  std::shared_ptr<cugl::scene2::TextField> _gameid;

 public:
#pragma mark -
#pragma mark Constructors
  /**
   * Creates a new client scene with the default values.
   */
  ClientMenuScene() : PeerScene() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  ~ClientMenuScene() { dispose(); }

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  virtual void dispose() override;

  /**
   * Initializes the controller contents, and starts the game
   *
   * @param assets    The (loaded) assets for this game mode
   *
   * @return true if the controller is initialized properly, false otherwise.
   */
  virtual bool init(const std::shared_ptr<cugl::AssetManager>& assets) override;

  /**
   * Sets whether the scene is currently active
   *
   * @param value whether the scene is currently active
   */
  virtual void setActive(bool value) override;

  /**
   * The method called to update the scene.
   *
   * @param timestep  The amount of time (in seconds) since the last frame
   */
  virtual void update(float timestep) override;

 private:
  /**
   * Updates the text in the given button.
   *
   * @param button    The button to modify
   * @param text      The new text value
   */
  virtual void updateText(const std::shared_ptr<cugl::scene2::Button>& button,
                          const std::string text) override;

  /**
   * Reconfigures the start button for this scene
   */
  virtual void configureStartButton() override;

  /**
   * Connects to the game server as specified in the assets file
   *
   * @param room  The room ID to use
   *
   * @return true if the connection was successful
   */
  bool connect(const std::string room);

  /**
   * Processes data (byte vectors) sent over the network.
   *
   * Note that this function may be called *multiple times* per animation frame,
   * as the messages can come from several sources.
   *
   * @param data  The data received
   */
  virtual void processData(const std::vector<uint8_t>& data) override;

  /**
   * Checks that the network connection is still active.
   *
   * @return true if the network connection is still active.
   */
  virtual bool checkConnection() override;
};

#endif /* SCENES_CLIENT_SCENE_H_ */