#ifndef CONTROLLERS_INPUT_CONTROLLER_H_
#define CONTROLLERS_INPUT_CONTROLLER_H_
#include <cugl/cugl.h>

// Forward declaration of Action to prevent circular dependencies.
class Action;

/**
 * A singleton class that handles reads inputs and handles actions.
 */
class InputController {
 protected:
  /* Single instance of InputController. */
  static std::shared_ptr<InputController> _singleton;

  /* Has been initialized correctly. */
  bool _active;

  /** If the input controller should be paused. */
  bool _pause;

  /* A list of all input actions in the game. */
  std::unordered_map<std::type_index, std::shared_ptr<Action>> _actions;

 public:
  /**
   * @return Singelton instance of InputController
   */
  static std::shared_ptr<InputController> get() {
    if (_singleton == nullptr) {
      _singleton = std::shared_ptr<InputController>{new InputController};
    }
    return _singleton;
  }

  /**
   * Finds and returns the action stored in the class that matches the parameter
   * T class.
   * @tparam T, action class (eg. Attack)
   * @return Instance of the action.
   */
  template <typename T>
  static std::shared_ptr<T> get() {
    if (!_singleton) {
      return nullptr;
    }
    auto it = _singleton->_actions.find(std::type_index(typeid(T)));
    if (it != _singleton->_actions.end()) {
      return std::dynamic_pointer_cast<T>(it->second);
    }
    return nullptr;
  }

  /**
   * Attaches action of type T
   * @tparam T, action class (eg. Attack)
   * @return Instance of the action.
   */
  template <typename T>
  static bool attachAction(std::shared_ptr<Action> action) {
    if (_singleton && _singleton->_active && action != nullptr) {
      _singleton->_actions[std::type_index(typeid(T))] = action;
      return true;
    }
    return false;
  }

  /**
   * Initialize the InputController and all the actions.
   *
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return If all actions and cugl input classes initialize correctly.
   */
  bool init(const std::shared_ptr<cugl::AssetManager> &assets,
            cugl::Rect bounds);

  /**
   * Call dispose on all the actions and clear the action cache.
   *
   * @return If all actions dispose correctly.
   */
  bool dispose();

  /**
   * Call update on all the actions.
   *
   * @return If all actions update correctly.
   */
  bool update();

  /** Pause the Input Controller. */
  void pause();

  /** Resume the Input Controller. */
  void resume();

  /**
   * @return If InputController has been activated by calling init()
   */
  bool isActive() { return _active; }

  InputController(InputController const &) = delete;
  void operator=(InputController const &) = delete;
  ~InputController() { dispose(); }

 private:
  InputController();
};

#endif /* CONTROLLERS_INPUT_CONTROLLER_H_ */