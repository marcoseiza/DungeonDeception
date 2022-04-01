#ifndef CONTROLLERS_CONTROLLER_H_
#define CONTROLLERS_CONTROLLER_H_

/**
 * Parent class for all controllers. init() methods should be done in the
 * controller, as initializations are very personal.
 */
class Controller : public std::enable_shared_from_this<Controller> {
 public:
  /** Construct a new controller. */
  Controller() {}
  /** Destroy the controller. */
  ~Controller() { dispose(); }

  /** Update the controller state. */
  virtual void update(float timestep) {}

  /** Dispose the controller and all its values. */
  virtual void dispose() {}

  /**
   * Smart pointers are great, and all Controllers should be referenced by one.
   * However, polymorphism and smart pointers really do not mix and type
   * casting can be quite tricky. This method provides a simple interface
   * for handling this type case.
   *
   * @return A pointer for attaching this Controller to an controller manager.
   */
  std::shared_ptr<Controller> getHook() { return shared_from_this(); }
};

#endif  // CONTROLLERS_CONTROLLER_H_