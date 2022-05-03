#ifndef CONTROLLERS_TERMINAL_CONTROLLER_H_
#define CONTROLLERS_TERMINAL_CONTROLLER_H_

#include <cugl/cugl.h>

#include "../models/Player.h"
#include "../models/tiles/TerminalSensor.h"
#include "../scenes/terminal_scenes/DepositEnergyScene.h"
#include "Controller.h"
#include "InputController.h"
#include "LevelController.h"
#include "PlayerController.h"

class TerminalController : public Controller {
  /** If a terminal is currently being voted on. */
  bool _active;

  /** A reference to the terminal voting scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _scene;
  /** A reference to the deposit energy scene. */
  std::shared_ptr<DepositEnergyScene> _deposit_energy_scene;

  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;
  /** Player Controller */
  std::shared_ptr<PlayerController> _player_controller;
  /** Level Controller */
  std::shared_ptr<LevelController> _level_controller;

  TerminalSensor* _terminal_sensor;
  /** The terminal room this controller is handling. */
  int _terminal_room_id;

  /** The number of terminals activated. */
  int _num_terminals_activated;
  /** The number of terminals corrupted. */
  int _num_terminals_corrupted;

 public:
  TerminalController() : _active(false) {}
  ~TerminalController() { dispose(); }

  /**
   * Initialize a new terminal controller with the given terminal voting scene.
   *
   * @param assets The assets for the game.
   * */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Allocate a new terminal controller with the given terminal voting scene.
   *
   * @param assets The assets for the game.
   * @return A shared pointer of the initialized Terminal Controller.
   */
  static std::shared_ptr<TerminalController> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto result = std::make_shared<TerminalController>();
    InputController::get()->resume();
    if (result->init(assets)) return result;
    return nullptr;
  }

  /** Update the controller state. */
  void update(float timestep) override;

  /** Dispose the controller and all its values. */
  void dispose() override {
    _scene = nullptr;
    _active = false;
  }

  /**
   * Set the terminal controller as active due to a terminal being hit.
   *
   * @param terminal_room_id The room this controller will handle.
   */
  void setActive(int terminal_room_id, TerminalSensor* sensor,
                 const std::shared_ptr<Player>& player) {
    if (_active) return;

    // If the voting room has already started.
    _deposit_energy_scene->setDone(false);
    _active = true;
    _terminal_room_id = terminal_room_id;
    _terminal_sensor = sensor;
    _scene->setVisible(true);
    InputController::get()->pause();
  }

  /**
   * Get the terminal room id this terminal controller is currently handleing.
   * @return The terminal room id this terminal controller is currently
   * handleing.
   */
  int getRoomId() { return _terminal_room_id; }

  /**
   * Process the network information and update the terminal controller data.
   *
   * @param code The message code
   * @param msg The deserialized message
   */
  void processNetworkData(
      const Sint32& code,
      const cugl::CustomNetworkDeserializer::CustomMessage& msg);

  /**
   * Send a terminal update to all clients, includes the player info after
   * depositing, the terminal info after depositing, and how many terminals are
   * activated/corrupted.
   *
   * @param player The player that deposited.
   * @param room The room where the player deposited the energy.
   */
  void sendTerminalUpdate(const std::shared_ptr<Player>& player,
                          const std::shared_ptr<RoomModel>& room);

  /**
   * Set the player controller for access to the players.
   * @param player_controller The player controller.
   */
  void setPlayerController(
      const std::shared_ptr<PlayerController>& player_controller) {
    _player_controller = player_controller;
    _deposit_energy_scene->setPlayerController(_player_controller);
  }

  /**
   * Set the level controller for access to terminal rooms.
   * @param level_controller The level controller.
   */
  void setLevelController(
      const std::shared_ptr<LevelController>& level_controller) {
    _level_controller = level_controller;
    _deposit_energy_scene->setLevelController(level_controller);
  }

  /** @return The number of terminals activated. */
  int getNumTerminalsActivated() { return _num_terminals_activated; }
  /** @return The number of terminals corrupted. */
  int getNumTerminalsCorrupted() { return _num_terminals_corrupted; }

 private:
  /** Called when the terminal voting is done. */
  void done();
};

#endif  // CONTROLLERS_TERMINAL_CONTROLLER_H_
