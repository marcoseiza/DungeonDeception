#ifndef CONTROLLERS_SOUND_CONTROLLER_H_
#define CONTROLLERS_SOUND_CONTROLLER_H_

#include "Controller.h"

class SoundController : public Controller {
 private:
  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

 public:
  /** Construct a new controller. */
  SoundController() {}
  /** Destroy the controller. */
  ~SoundController() { dispose(); }

  bool init(const std::shared_ptr<cugl::AssetManager> &assets);

  static std::shared_ptr<SoundController> alloc(
      const std::shared_ptr<cugl::AssetManager> &assets) {
    auto result = std::make_shared<SoundController>();
    return (result->init(assets)) ? result : nullptr;
  }

  /** Update the controller state. */
  void update(float timestep) override {}

  /** Dispose the controller and all its values. */
  void dispose() override {}
};

#endif  // CONTROLLERS_SOUND_CONTROLLER_H_