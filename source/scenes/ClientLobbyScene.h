#ifndef SCENES_CLIENT_LOBBY_SCENE_H_
#define SCENES_CLIENT_LOBBY_SCENE_H_
#include <cugl/cugl.h>

#include <vector>

/**
 * This class provides the interface for clients to wait for a game to start.
 *
 */
class ClientLobbyScene : public cugl::Scene2 {
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
  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** The network connection (as made by this scene) */
  std::shared_ptr<cugl::NetworkConnection> _network;

  /** The number of players label */
  std::shared_ptr<cugl::scene2::Label> _player;
  /** The game id label (for updating) */
  std::shared_ptr<cugl::scene2::Label> _gameid;
  /** The player name label */
  std::shared_ptr<cugl::scene2::Label> _name;

  /** The serializer used to serialize complex data to send through the network.
   */
  cugl::NetworkSerializer _serializer;
  /** The deserializer used to deserialize complex data sent through the
   * network. */
  cugl::NetworkDeserializer _deserializer;

  /** The current status */
  Status _status;

  /** The map seed. */
  Uint64 _seed;

  /** If the user is a betrayer (true) or cooperator (false). */
  bool _is_betrayer;

 public:
#pragma mark -
#pragma mark Constructors
  /**
   * Creates a new client scene with the default values.
   */
  ClientLobbyScene() : cugl::Scene2() {}

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  ~ClientLobbyScene() { dispose(); }

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
  virtual bool init(const std::shared_ptr<cugl::AssetManager>& assets);

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
                         std::shared_ptr<cugl::NetworkConnection> network);

  /**
   * Sets the game id from the network to the lobby
   *
   * @param id the game id
   */
  virtual void setGameId(string id) { _gameid->setText(id); };

  /**
   * Returns the seed to be broadcast to all clients.
   * @return The seed for the map
   */
  Uint64 getSeed() { return _seed; }

  /**
   * The method called to update the scene.
   *
   * @param timestep  The amount of time (in seconds) since the last frame
   */
  virtual void update(float timestep) override;

 private:
  /**
   * Checks that the network connection is still active.
   *
   * @return true if the network connection is still active.
   */
  virtual bool checkConnection();

  /**
   * Processes data (byte vectors) sent over the network.
   *
   * Note that this function may be called *multiple times* per animation
   * frame, as the messages can come from several sources.
   *
   * @param data  The data received
   */
  virtual void processData(const std::vector<uint8_t>& data);

  /**
   * Disconnects this scene from the network controller.
   *
   * Technically, this method does not actually disconnect the network
   * controller. Since the network controller is a smart pointer, it is only
   * fully disconnected when ALL scenes have been disconnected.
   */
  void disconnect() { _network = nullptr; }
};

#endif /* SCENES_CLIENT_LOBBY_SCENE_H_ */
