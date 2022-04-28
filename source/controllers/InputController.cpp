#include "InputController.h"

#include <cugl/cugl.h>

#include "actions/Action.h"
#include "actions/Attack.h"
#include "actions/TargetPlayer.h"
#include "actions/Dash.h"
#include "actions/Corrupt.h"
#include "actions/Movement.h"
#include "actions/OpenMap.h"

// static
std::shared_ptr<InputController> InputController::_singleton = nullptr;

InputController::InputController() : _active(false) {}

bool InputController::init(const std::shared_ptr<cugl::AssetManager> &assets,
                           cugl::Rect bounds) {
#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *input = cugl::Input::get<cugl::Touchscreen>();
#else
  cugl::Mouse *input = cugl::Input::get<cugl::Mouse>();
#endif

  if (input) {
    // Create, initialize and register all basic actions.
    _active = true;

    _active = InputController::attachAction<Attack>(
        Attack::alloc(assets, bounds)->getHook());

    _active = InputController::attachAction<Movement>(
        Movement::alloc(assets, bounds)->getHook());

    _active = InputController::attachAction<OpenMap>(
        OpenMap::alloc(assets, bounds)->getHook());

    _active = InputController::attachAction<TargetPlayer>(
        TargetPlayer::alloc(assets, bounds)->getHook());
    
    _active = InputController::attachAction<Dash>(
        Dash::alloc(assets, bounds)->getHook());
    
    _active = InputController::attachAction<Corrupt>(
        Corrupt::alloc(assets, bounds)->getHook());
  }
  return _active;
}

bool InputController::update() {
  if (_pause) return true;

  for (auto it = _actions.begin(); it != _actions.end(); ++it) {
    (it->second)->update();
  }
  return true;
}

void InputController::pause() {
  if (_pause) return;
  _pause = true;
  for (auto it = _actions.begin(); it != _actions.end(); ++it) {
    (it->second)->reset();
    (it->second)->pause();
  }
}

void InputController::resume() {
  if (!_pause) return;
  _pause = false;
  for (auto it = _actions.begin(); it != _actions.end(); ++it) {
    (it->second)->resume();
  }
}

bool InputController::dispose() {
  bool successful = false;
  if (_active) {
    successful = true;
    for (auto it = _actions.begin(); it != _actions.end(); ++it) {
      successful = (it->second)->dispose();
    }
    _active = false;
  }
  _actions.clear();
  return successful;
}
