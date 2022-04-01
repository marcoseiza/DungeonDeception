#ifndef CONTROLLERS_ACTIONS_ACTION_H_
#define CONTROLLERS_ACTIONS_ACTION_H_
#include <cugl/cugl.h>

/**
 * This class provides a polymorphic base for all actions in the game.
 *
 * Every action subclass must override the virtual methods.
 *
 * Used by InputController to keep track of all actions.
 */
class Action : public std::enable_shared_from_this<Action> {
 protected:
  /* Action has been initialized correctly. */
  bool _action_init = false;

  /* Bounds of the current display, used to scale input */
  cugl::Rect _display_coord_bounds;

  /* Bounds of the scene2 world, used to scale input */
  cugl::Rect _screen_coord_bounds;

 public:
  Action() {}
  ~Action() {}

  /**
   * Creates input listeners and sets default variable.
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return If initialized correctly.
   */
  virtual bool init(const std::shared_ptr<cugl::AssetManager> &assets,
                    cugl::Rect bounds) {
    _screen_coord_bounds = bounds;
    _display_coord_bounds = cugl::Application::get()->getDisplayBounds();
    _action_init = true;
    return _action_init;
  }

  /**
   * Updates action state.
   * @return If updated correctly.
   */
  virtual bool update() = 0;

  /**
   * Disposes input listeners and default variables.
   * @return If disposed correctly.
   */
  virtual bool dispose() = 0;

  /** Reset all the internal input values. */
  virtual void reset() = 0;

  /** Pause all input. */
  virtual void pause() = 0;

  /** Resume all input. */
  virtual void resume() = 0;

  /**
   * Smart pointers are great, and all Actions should be referenced by one.
   * However, polymorphism and smart pointers really do not mix and type
   * casting can be quite tricky. This method provides a simple interface
   * for handling this type case.
   *
   * @return A pointer for attaching this Action to an InputController.
   */
  std::shared_ptr<Action> getHook() { return shared_from_this(); }

  /**
   * Returns the scene location of a touch
   *
   * Touch coordinates are inverted, with y origin in the top-left
   * corner. This method corrects for this and scales the screen
   * coordinates down on to the scene graph size.
   *
   * @return the scene location of a touch
   */
  cugl::Vec2 displayToScreenCoord(const cugl::Vec2 pos) const {
    cugl::Vec2 result;

    if (_action_init) {
      float px = pos.x / _display_coord_bounds.size.width -
                 _display_coord_bounds.origin.x;
      float py = pos.y / _display_coord_bounds.size.height -
                 _display_coord_bounds.origin.y;
      result.x =
          px * _screen_coord_bounds.size.width + _screen_coord_bounds.origin.x;
      result.y = (1 - py) * _screen_coord_bounds.size.height +
                 _screen_coord_bounds.origin.y;
    }
    return result;
  }
};

#endif /* CONTROLLERS_ACTIONS_ACTION_H_ */
