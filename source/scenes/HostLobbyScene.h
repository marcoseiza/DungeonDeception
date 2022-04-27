#ifndef SCENES_HOST_LOBBY_SCENE_H_
#define SCENES_HOST_LOBBY_SCENE_H_
#include <cugl/cugl.h>

#include <vector>

#include "PeerLobbyScene.h"

/**
 * This class provides the interface for hosts to wait for a game to start.
 *
 */
class HostLobbyScene : public PeerLobbyScene {
 protected:
  /** The serializer used to serialize complex data to send through the network.
   */
  cugl::NetworkSerializer _serializer;

  /** The button used the start the game */
  std::shared_ptr<cugl::scene2::Button> _startgame;

 public:
#pragma mark -
#pragma mark Constructors
  /**
   * Creates a new host scene with the default values.
   */
  HostLobbyScene() : PeerLobbyScene() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  ~HostLobbyScene() { dispose(); }

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
  virtual void setActive(
      bool value, std::shared_ptr<cugl::NetworkConnection> network) override;

  /**
   * The method called to update the scene.
   *
   * @param timestep  The amount of time (in seconds) since the last frame
   */
  virtual void update(float timestep) override;

  /**
   * Sets the game id from the network to the lobby
   *
   * @param id the game id
   */
  virtual void setGameId(string id) { _gameid->setText(id); };

 private:
  /**
   * Checks that the network connection is still active.
   *
   * @return true if the network connection is still active.
   */
  virtual bool checkConnection() override;

  /**
   * Processes data (byte vectors) sent over the network.
   *
   * Note that this function may be called *multiple times* per animation
   * frame, as the messages can come from several sources.
   *
   * @param data  The data received
   */
  virtual void processData(const std::vector<uint8_t>& data) override;

  /**
   * Starts the game.
   */
  void startGame();

  /**
   * Determine the roles of all the players before the game starts.
   * Afterwards, send number of betrayers and ids to all clients.
   *
   * Assignment algorithm assumes that the number of betrayers is [1, 2].
   */
  void determineAndSendRoles();
};

#endif /* SCENES_HOST_LOBBY_SCENE_H_ */
