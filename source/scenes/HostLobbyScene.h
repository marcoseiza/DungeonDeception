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
  /** The button used the start the game */
  std::shared_ptr<cugl::scene2::Button> _startgame;

  /** Tooltip for when all names have been submitted. */
  std::shared_ptr<cugl::scene2::SceneNode> _names_success;
  /** Tooltip for when not all names have been submitted. */
  std::shared_ptr<cugl::scene2::SceneNode> _names_waiting;
  /** Tooltip for when name is already in use. */
  std::shared_ptr<cugl::scene2::SceneNode> _names_in_use;
  /** Tooltip for when name is set. */
  std::shared_ptr<cugl::scene2::SceneNode> _names_set;

  /** A map from the player id to the name. */
  std::unordered_map<int, std::string> _player_id_to_name;

  /** The number of players in the game. */
  int _num_of_players;

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
   * Send submitted player name over the network.
   * @param name The name to send.
   */
  void sendPlayerName(const std::string& name) override;

  /**
   * Process the received player name
   *
   * Returns:
   * HOST_ACCEPT_PLAYER_NAME
   * HOST_DENY_PLAYER_NAME
   * HOST_REMOVE_PLAYER_NAME
   * HOST_NAME_NO_OP
   *
   * @param player_id The player id.
   * @param name The name received.
   * @return The code for accept, deny, or remove.
   */
  HostResponse processReceivedPlayerName(const int player_id,
                                         const std::string& name);

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

  /**
   * Determine the colors of all the players before the game starts.
   * Afterwards, send the color to the clients.
   *
   * Assignment algorithm assumes that the number of player is [1, 8].
   */
  void determineAndSendColors();
};

#endif /* SCENES_HOST_LOBBY_SCENE_H_ */
