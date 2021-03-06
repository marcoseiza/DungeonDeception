#ifndef SCENES_PEER_LOBBY_SCENE_H_
#define SCENES_PEER_LOBBY_SCENE_H_
#include <cugl/cugl.h>

#include <vector>

/**
 * This class provides the interface for clients to wait for a game to start.
 *
 */
class PeerLobbyScene : public cugl::Scene2 {
 public:
  /**
   * The configuration status.
   */
  enum Status {
    /** Client or Host is waiting to start game */
    WAIT,
    /** Time to start the game */
    START,
    /** Game was aborted; back to main menu */
    ABORT
  };

 protected:
  /** The serializer used to serialize complex data to send through the network.
   */
  cugl::NetworkSerializer _serializer;
  /** The deserializer used to deserialize complex data that was sent through
   * the network. */
  cugl::NetworkDeserializer _deserializer;

  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** The network connection (as made by this scene) */
  std::shared_ptr<cugl::NetworkConnection> _network;

  /** The number of players label */
  std::shared_ptr<cugl::scene2::Label> _player;
  /** The game id label (for updating) */
  std::shared_ptr<cugl::scene2::Label> _gameid;
  /** The player name label */
  std::shared_ptr<cugl::scene2::TextField> _name;
  /** The back button for the scene */
  std::shared_ptr<cugl::scene2::Button> _backout;
  /** Reference to the cloud layer scene graph. */
  std::shared_ptr<cugl::scene2::SceneNode> _cloud_layer;
  /** The x position of the cloud. */
  float _cloud_x_pos = 0;

  /** The copy for the game id */
  std::shared_ptr<cugl::scene2::Button> _copy;
  /** The copy tooltip for the game id */
  std::shared_ptr<cugl::scene2::SceneNode> _copy_tooltip;
  /** The copy tooltip lifetime. */
  float _copy_tooltip_lifetime;

  /** The current status */
  Status _status;

  /** The map seed. */
  Uint64 _seed;

  /** If the user is a betrayer (true) or cooperator (false). */
  bool _is_betrayer;

  /** The map from player id to color id. */
  std::unordered_map<int, int> _color_ids;

  enum HostResponse : Sint32 {
    HOST_ACCEPT_PLAYER_NAME = 2,
    HOST_DENY_PLAYER_NAME = 3,
    HOST_REMOVED_PLAYER_NAME = 4,
    HOST_NAME_NO_OP = 5
  };

  const Sint32 CLIENT_SEND_PLAYER_NAME = 1;
  const Sint32 CLIENT_REMOVE_PLAYER_NAME = 6;
  const Sint32 HOST_SEND_THAT_LOBBY_IS_OPEN = 0;

 public:
#pragma mark -
#pragma mark Constructors
  /**
   * Creates a new scene with the default values.
   */
  PeerLobbyScene() : cugl::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  ~PeerLobbyScene() { dispose(); }

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
    return false;
  };

  /**
   * Returns the scene status. Any value other than WAIT will transition to
   * a new scene.
   *
   * @return the scene status
   */
  Status getStatus() const { return _status; }

  /**
   * Returns if the scene represents a host or betrayer
   * @return The role of the player, true if betrayer, false otherwise.
   */
  bool isBetrayer() { return _is_betrayer; }

  /**
   * Sets whether the scene is currently active
   *
   * @param value whether the scene is currently active
   */
  virtual void setActive(bool value,
                         std::shared_ptr<cugl::NetworkConnection> network){};

  /**
   * Returns the seed to be broadcast to all clients.
   * @return The seed for the map
   */
  Uint64 getSeed() { return _seed; }

  /**
   * Returns the player name to be used in game.
   * @return The player name
   */
  std::string getPlayerName() { return _name->getText(); }

  /**
   * Returns the color id for all player.
   * @return A map from the player id to the color id.
   */
  std::unordered_map<int, int> getColorIds() { return _color_ids; }

  /**
   * The method called to update the scene.
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
   * Checks that the network connection is still active.
   *
   * @return true if the network connection is still active.
   */
  virtual bool checkConnection() { return false; };

  /**
   * Processes data (byte vectors) sent over the network.
   *
   * Note that this function may be called *multiple times* per animation
   * frame, as the messages can come from several sources.
   *
   * @param data  The data received
   */
  virtual void processData(const std::vector<uint8_t>& data){};

  /**
   * Send submitted player name over the network.
   * @param name The name to send.
   */
  virtual void sendPlayerName(const std::string& name) {}
};

#endif /* SCENES_PEER_LOBBY_SCENE_H_ */
