#ifndef SCENES_PEER_SCENE_H_
#define SCENES_PEER_SCENE_H_
#include <cugl/cugl.h>

#include <vector>

/**
 * This class provides the interface for a networking peer - either host or
 * client.
 */
class PeerScene : public cugl::Scene2 {
 public:
  /**
   * The configuration status.
   */
  enum Status {
    /** Host is waiting on a connection or Client has not yet entered a room */
    IDLE,
    /** Client is connecting to the host, or host is moving to lobby */
    JOIN,
    /** Client or Host is waiting to start game */
    WAIT,
    /** Game was aborted; back to main menu */
    ABORT
  };

 protected:
  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** The network connection (as made by this scene) */
  std::shared_ptr<cugl::NetworkConnection> _network;
  /** The serializer used to serialize complex data to send through the network.
   */
  cugl::NetworkSerializer _serializer;
  /** The deserializer used to deserialize complex data that was sent through
   * the network. */
  cugl::NetworkDeserializer _deserializer;

  /** The menu button for starting a game */
  std::shared_ptr<cugl::scene2::Button> _startgame;
  /** The back button for the menu scene */
  std::shared_ptr<cugl::scene2::Button> _backout;
  /** The game id label (for updating) */
  std::shared_ptr<cugl::scene2::Label> _gameid;
  /** Reference to the cloud layer scene graph. */
  std::shared_ptr<cugl::scene2::SceneNode> _cloud_layer;
  /** The x position of the cloud. */
  float _cloud_x_pos = 0;

  /** A reference to the clipboard button. */
  std::shared_ptr<cugl::scene2::Button> _clipboard;

  /** The network configuration */
  cugl::NetworkConnection::ConnectionConfig _config;

  /** The current status */
  Status _status;

  const Sint32 HOST_SEND_THAT_LOBBY_IS_OPEN = 0;

 public:
#pragma mark -
#pragma mark Constructors
  /**
   * Creates a new peer scene with the default values without starting the game.
   */
  PeerScene() : cugl::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  ~PeerScene() { dispose(); }

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  virtual void dispose() override{};

  /**
   * Initializes the controller contents, and starts the game
   *
   * @param assets    The (loaded) assets for this game mode
   *
   * @return true if the controller is initialized properly, false otherwise.
   */
  virtual bool init(const std::shared_ptr<cugl::AssetManager>& assets) {
    return true;
  };

  /**
   * Sets whether the scene is currently active
   *
   * @param value whether the scene is currently active
   */
  virtual void setActive(bool value) override{};

  /**
   * Returns the network connection (as made by this scene)
   *
   * @return the network connection (as made by this scene)
   */

  std::shared_ptr<cugl::NetworkConnection> getConnection() const {
    return _network;
  }

  /**
   * Returns the scene status. Any value other than WAIT will transition to
   * a new scene.
   *
   * @return the scene status
   */
  Status getStatus() const { return _status; }

  /**
   * Returns the text representing the game_id
   *
   * @return the game_id in the label
   */
  std::string getGameId() const { return _gameid->getText(); }

  /**
   * The method called to update the scene by constantly talking to the server
   *
   * @param timestep  The amount of time (in seconds) since the last frame
   */
  virtual void update(float timestep) override{};

  /**
   * Disconnects this scene from the network controller.
   *
   * Technically, this method does not actually disconnect the network
   * controller. Since the network controller is a smart pointer, it is only
   * fully disconnected when ALL scenes have been disconnected.
   */
  void disconnect() { _network = nullptr; }

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

 private:
  /**
   * Updates the text in the given button.
   *
   * @param button    The button to modify
   * @param text      The new text value
   */
  virtual void updateText(const std::shared_ptr<cugl::scene2::Button>& button,
                          const std::string text){};

  /**
   * Reconfigures the start button for this scene
   */
  virtual void configureStartButton(){};

  /**
   * Processes data (byte vectors) sent over the network.
   *
   * Note that this function may be called *multiple times* per animation frame,
   * as the messages can come from several sources.
   *
   * @param data  The data received
   */
  virtual void processData(const std::vector<uint8_t>& data){};

  /**
   * Checks that the network connection is still active.
   *
   * @return true if the network connection is still active.
   */
  virtual bool checkConnection() { return false; };
};

#endif /* SCENES_PEER_SCENE_H_ */
