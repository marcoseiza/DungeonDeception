#ifndef CONTROLLERS_TRAIL_CONTROLLER_H_
#define CONTROLLERS_TRAIL_CONTROLLER_H_

#include <cugl/cugl.h>

class TrailController {
 public:
  TrailController() {}
  ~TrailController() { dispose(); }

  bool init();

  void dispose();

  static std::shared_ptr<TrailController> alloc() {
    auto result = std::make_shared<TrailController>();
    return (result->init()) ? result : nullptr;
  }
};

#endif  // CONTROLLERS_TRAIL_CONTROLLER_H_