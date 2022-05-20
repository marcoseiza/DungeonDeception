#ifndef CONTROLLERS_ACTIONS_CORRUPT_H_
#define CONTROLLERS_ACTIONS_CORRUPT_H_
#include <cugl/cugl.h>

#include "Action.h"

/**
 * This class is an implementation of Action.
 *
 * This class provides attacking capabilities for the user.
 *
 * As with all Actions attach to InputController by calling allocating using
 * alloc and calling getHook(). This is very similar to Walker White's loader
 * system.
 */
class Corrupt : public Action {
 protected:
  /* Reference to attack button for registering listeners to press event. */
  std::shared_ptr<cugl::scene2::Button> _button;
  /* Reference to button node for animation. */
  std::shared_ptr<cugl::scene2::SpriteNode> _button_node;

  /** The animation buffer for the charge animation. */
  int _anim_buffer;

  /* Button was previously down on the last tick. */
  bool _prev_down;
  /* Button is currently down on the current tick. */
  bool _curr_down;
  /* Scene2 button is pressed. */
  bool _butt_down;

  /** Whether to start the corruption cooldown. */
  bool _start_cooldown;

  /* Key for all the input listeners, for disposal. */
  Uint32 _listener_key;

  /** Time since the start of the cooldown. */
  cugl::Timestamp _time_cooldown_start;

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
  void reset() override {}

  /** Pause all input. */
  virtual void pause() override { _button->deactivate(); }

  /** Resume all input. */
  virtual void resume() override { _button->activate(); }

  /**
   * This method allocates Corrupt and initializes it.
   *
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return A newly allocated Attack action.
   */
  static std::shared_ptr<Corrupt> alloc(
      const std::shared_ptr<cugl::AssetManager> &assets, cugl::Rect bounds) {
    std::shared_ptr<Corrupt> result = std::make_shared<Corrupt>();
    return (result->init(assets, bounds) ? result : nullptr);
  }

  /**
   * @return Whether the player currently pressed corrupt.
   */
  bool pressCorrupt() const { return _prev_down && !_curr_down; }

  /**
   * @return If player is holding corrupt button.
   */
  bool holdCorrupt() const { return _prev_down && _curr_down; }

  /**
   * Toggels activation on corrupt button. When deactivated, the button
   * cannot be pressed.
   * @param value The activation state.
   */
  void setActive(bool value);

  Corrupt();
  ~Corrupt() {}
};

#endif /* CONTROLLERS_ACTIONS_CORRUPT_H_ */
