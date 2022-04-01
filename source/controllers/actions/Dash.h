#ifndef CONTROLLERS_ACTIONS_DASH_H_
#define CONTROLLERS_ACTIONS_DASH_H_
#include <cugl/cugl.h>

#include "Action.h"

/**
 * This class is an implementation of Action.
 *
 * This class provides dashing capabilities for the user.
 *
 * As with all Actions attach to InputController by calling allocating using
 * alloc and calling getHook(). This is very similar to Walker White's loader
 * system.
 */
class Dash : public Action {
 protected:
  /* Reference to map button for registering listeners to press event. */
  std::shared_ptr<cugl::scene2::Button> _button;

  /* Button was previously down on the last tick. */
  bool _prev_down;
  /* Button is currently down on the current tick. */
  bool _curr_down;
  /* Scene2 button is pressed. */
  bool _butt_down;

  /* The duration of the dash */
  int _dash_frames;

  /* The counter for the dash duration*/
  int _dash_frame_counter;

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

  /** Reset all the internal input values. */
  virtual void reset() override {
    _prev_down = false;
    _curr_down = false;
    _butt_down = false;
    _dash_frame_counter = 0;
  }

  /** Pause all input. */
  virtual void pause() override { _button->deactivate(); }

  /** Resume all input. */
  virtual void resume() override { _button->activate(); }

  /**
   * This method allocates Dash and initializes it.
   *
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return A newly allocated Dash action.
   */
  static std::shared_ptr<Dash> alloc(
      const std::shared_ptr<cugl::AssetManager> &assets, cugl::Rect bounds) {
    std::shared_ptr<Dash> result = std::make_shared<Dash>();
    return (result->init(assets, bounds) ? result : nullptr);
  }

  /**
   * @return If the player pressed the dash button.
   */
  bool isDashing() const { return _dash_frame_counter > 0; }

  /**
   * Toggles the activation of the map button. When deactivated, the button
   * cannot be pressed.
   * @param value The activation state.
   */
  void setActive(bool value);

  Dash();
  ~Dash() {}
};

#endif /* CONTROLLERS_ACTIONS_DASH_H_ */
