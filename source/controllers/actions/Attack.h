#ifndef CONTROLLERS_ACTIONS_ATTACK_H_
#define CONTROLLERS_ACTIONS_ATTACK_H_
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
class Attack : public Action {
 protected:
  /* Reference to scene2 attack joystick base for updating position. */
  std::shared_ptr<cugl::scene2::PolygonNode> _attack_base;
  /* Reference to attack button for registering listeners to press event. */
  std::shared_ptr<cugl::scene2::Button> _button;

  /* Button was previously down on the last tick. */
  bool _prev_down;
  /* Button is currently down on the current tick. */
  bool _curr_down;
  /* Scene2 button is pressed. */
  bool _butt_down;

  /* Key for all the input listeners, for disposal. */
  Uint32 _listener_key;

  /* Initial position of the joystick, for math operations on the input.
       i.e. Where the player placed their finger first. */
  cugl::Vec2 _joystick_anchor;
  /* Vector between joystick nob and joystick anchor. Used for defining how much
     you move. */
  cugl::Vec2 _joystick_diff;

  // The screen is divided into two zones: Left, Right
  // These are all shown in the diagram below.
  //
  //   |-----------------|
  //   |        |        |
  //   | L      |      R |
  //   |        |        |
  //   |-----------------|
  //
  // Attacking with attack joystick happens on the right side.

  /* Bounds of the right side of screen, for processing input. */
  cugl::Rect _right_screen_bounds;

  /** If the joystick base should be shown. */
  bool _show_joystick_base;

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
  void reset() override {
    _show_joystick_base = false;
    _joystick_anchor.set(cugl::Vec2::ZERO);
    _joystick_diff.set(cugl::Vec2::ZERO);
  }

  /** Pause all input. */
  virtual void pause() override { _button->deactivate(); }

  /** Resume all input. */
  virtual void resume() override { _button->activate(); }

  /**
   * This method allocates Attack and initializes it.
   *
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return A newly allocated Attack action.
   */
  static std::shared_ptr<Attack> alloc(
      const std::shared_ptr<cugl::AssetManager> &assets, cugl::Rect bounds) {
    std::shared_ptr<Attack> result = std::make_shared<Attack>();
    return (result->init(assets, bounds) ? result : nullptr);
  }

  /**
   * @return If player is currently attacking.
   */
  bool isAttacking() const { return _prev_down && !_curr_down; }

  /**
   * @return If player is holding attack button.
   */
  bool holdAttack() const { return _prev_down && _curr_down; }

  /**
   * Toggels activation on attack button. When deactivated, the button cannot
   * be pressed.
   * @param value The activation state.
   */
  void setActive(bool value);

#ifdef CU_TOUCH_SCREEN

  /** Touch listener for when the player moves their finger. */
  void touchMoved(const cugl::TouchEvent &event, const cugl::Vec2 &previous,
                  bool focus);

#endif  // CU_TOUCH_SCREEN

  /**
   * Get input vector for attacking. X and Y values range from -1.0f to 1.0f.
   * @return Movement vector.
   */
  cugl::Vec2 getAttackDir() { return cugl::Vec2(_joystick_diff).normalize(); }

  Attack();
  ~Attack() {}
};

#endif /* CONTROLLERS_ACTIONS_ATTACK_H_ */
