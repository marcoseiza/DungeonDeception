#ifndef CONTROLLERS_SOUND_CONTROLLER_H_
#define CONTROLLERS_SOUND_CONTROLLER_H_

#include <cugl/cugl.h>

#include "Controller.h"

class SoundController : public Controller {
 private:
  /** The key for the network data listener for disposal. */
  Uint32 _network_listener_key;

  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** The time stamp for when the music started. */
  std::chrono::system_clock::time_point _music_start;
  /** If the host has sent the music start time. */
  bool _has_sent_music_start;
  /** If all the players are in the game. */
  bool _all_players_in_game;

 public:
  /** Construct a new controller. */
  SoundController() {}
  /** Destroy the controller. */
  ~SoundController() { dispose(); }

  /**
   * Initialize the Sound Controller. Please use alloc().
   *
   * @param assets The game assets.
   * @return If initialized properly.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Allocates and initializes a new SoundController shared_ptr.
   *
   * @param assets The game assets.
   * @return The shared pointer SoundController.
   */
  static std::shared_ptr<SoundController> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto result = std::make_shared<SoundController>();
    return (result->init(assets)) ? result : nullptr;
  }

  /** Update the controller state. */
  void update(float timestep) override;

  /** Dispose the controller and all its values. */
  void dispose() override;

  /**
   * Process the network information and update the terminal controller data.
   *
   * @param code The message code
   * @param msg The deserialized message
   */
  void processNetworkData(const Sint32& code,
                          const cugl::NetworkDeserializer::Message& msg);

  /**
   * Set that all the players are present in the game. Used by controller to
   * start synchronizing sound.
   * */
  void allPlayersPresent() { _all_players_in_game = true; }
};

#endif  // CONTROLLERS_SOUND_CONTROLLER_H_