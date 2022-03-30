#ifndef CONTROLLERS_ACTIONS_OPEN_MAP_H_
#define CONTROLLERS_ACTIONS_OPEN_MAP_H_
#include <cugl/cugl.h>

#include "Action.h"

/**
 * This class is an implementation of Action.
 *
 * This class provides map opening/closing capabilities to the user.
 *
 * As with all Actions attach to InputController by calling allocating using
 * alloc and calling getHook(). This is very similar to Walker White's loader
 * system.
 */
class OpenMap : public Action {
 protected:
  /* Reference to map button for registering listeners to press event. */
  std::shared_ptr<cugl::scene2::Button> _button;

  /* Button was previously down on the last tick. */
  bool _prev_down;
  /* Button is currently down on the current tick. */
  bool _curr_down;
  /* Scene2 button is pressed. */
  bool _butt_down;

  /* Key for all the input listeners, for disposal. */
  Uint32 _listener_key;

 public:
  /**
   * Creates input listeners and sets default variable.
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return If initialized correctly.
   */
  virtual bool init(const std::shared_ptr<cugl::AssetManager> &assets,
                    cugl::Rect bounds) override;

  /**
   * Updates action state.
   * @return If updated correctly.
   */
  virtual bool update() override;

  /**
   * Disposes input listeners and default variables.
   * @return If disposed correctly.
   */
  virtual bool dispose() override;

  /**
   * This method allocates OpenMap and initializes it.
   *
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return A newly allocated OpenMap action.
   */
  static std::shared_ptr<OpenMap> alloc(
      const std::shared_ptr<cugl::AssetManager> &assets, cugl::Rect bounds) {
    std::shared_ptr<OpenMap> result = std::make_shared<OpenMap>();
    return (result->init(assets, bounds) ? result : nullptr);
  }

  /**
   * @return If the player opened the map.
   */
  bool didOpenMap() const { return _prev_down && !_curr_down; }

  /**
   * Toggles the activation of the map button. When deactivated, the button cannot be pressed.
   * @param value The activation state.
   */
  void setActive(bool value);

#ifdef CU_TOUCH_SCREEN

  /** Touch listener for when the player moves their finger. */
  void touchMoved(const cugl::TouchEvent &event, const cugl::Vec2 &previous,
                  bool focus);

#endif  // CU_TOUCH_SCREEN

  OpenMap();
  ~OpenMap() {}
};

#endif /* CONTROLLERS_ACTIONS_OPEN_MAP_H_ */
