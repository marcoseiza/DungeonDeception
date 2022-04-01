#ifndef CONTROLLERS_ACTIONS_MOVEMENT_H_
#define CONTROLLERS_ACTIONS_MOVEMENT_H_
#include <cugl/cugl.h>

#include "Action.h"

/**
 * This class is an implementation of Action.
 *
 * This class provides moving capabilities for the user.
 *
 * As with all Actions attach to InputController by calling allocating using
 * alloc and calling getHook(). This is very similar to Walker White's loader
 * system.
 */
class Movement : public Action {
 protected:
  /* Reference to scene2 joystick base for updating position. */
  std::shared_ptr<cugl::scene2::PolygonNode> _joystick_base;

  /* Reference to scene2 joystick nob for updating position. */
  std::shared_ptr<cugl::scene2::PolygonNode> _joystick;

  /* Key for all the input listeners, for disposal. */
  Uint32 _listener_key;

  // The screen is divided into two zones: Left, Right
  // These are all shown in the diagram below.
  //
  //   |-----------------|
  //   |        |        |
  //   | L      |      R |
  //   |        |        |
  //   |-----------------|
  //
  // Movement with joystick happens on the left side.

  /* Bounds of the left side of screen, for processing input. */
  cugl::Rect _left_screen_bounds;

#ifdef CU_TOUCH_SCREEN
  /* Set of all touch id references to keep track of fingers. */
  std::unordered_set<cugl::TouchID> _touch_ids;
#endif

  /* For activating joystick in scene2. */
  bool _show_joystick;

  /* Initial position of the joystick, for math operations on the input.
     i.e. Where the player placed their finger first. */
  cugl::Vec2 _joystick_anchor;
  /* Vector between joystick nob and joystick anchor. Used for defining how much
     you move. */
  cugl::Vec2 _joystick_diff;

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
    _show_joystick = false;
#ifdef CU_TOUCH_SCREEN
    _touch_ids.clear();
#endif
    _joystick_anchor.set(cugl::Vec2::ZERO);
    _joystick_diff.set(cugl::Vec2::ZERO);
  }

  /** Pause all input. */
  virtual void pause() override {}

  /** Resume all input. */
  virtual void resume() override {}

  /**
   * This method allocates Movement and initializes it.
   *
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return A newly allocated Attack action.
   */
  static std::shared_ptr<Movement> alloc(
      const std::shared_ptr<cugl::AssetManager> &assets, cugl::Rect bounds) {
    std::shared_ptr<Movement> result = std::make_shared<Movement>();
    return (result->init(assets, bounds) ? result : nullptr);
  }

#pragma mark Listeners

#ifdef CU_TOUCH_SCREEN

  /** Touch listener for when the player touches the screen. */
  void touchBegan(const cugl::TouchEvent &event, bool focus);

  /** Touch listener for when the player releases finger from the screen. */
  void touchEnded(const cugl::TouchEvent &event, bool focus);

  /** Touch listener for when the player moves their finger. */
  void touchMoved(const cugl::TouchEvent &event, const cugl::Vec2 &previous,
                  bool focus);

#endif  // CU_TOUCH_SCREEN

  /**
   * Get input vector for movement. X and Y values range from -1.0f to 1.0f.
   * @return Movement vector.
   */
  cugl::Vec2 getMovement() { return cugl::Vec2(_joystick_diff).normalize(); }

  /**
   * Get input float for movement in the X direction. X values range from -1.0f
   * to 1.0f.
   * @return Amount of movement in X direction.
   */
  float getMovementX() { return getMovement().x; }

  /**
   * Get input float for movement in the Y direction. Y values range from -1.0f
   * to 1.0f.
   * @return Amount of movement in Y direction.
   */
  float getMovementY() { return getMovement().y; }

  /**
   * Is currently pressing on screen to create a joystick. Not necessarily when
   * there is a diff from the joystick anchor to the nob, i.e. when the movement
   * vector is {0,0}
   * @return Player is currently pressing down to create a joystick.
   */
  bool isMoving() { return _show_joystick; }

  Movement();
  ~Movement() {}
};

#endif /* CONTROLLERS_ACTIONS_MOVEMENT_H_ */
