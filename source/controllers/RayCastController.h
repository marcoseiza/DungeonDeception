#ifndef CONTROLLERS_RAYCASTCONTROLLER_H_
#define CONTROLLERS_RAYCASTCONTROLLER_H_

#include <box2d/b2_world_callbacks.h>
#include <cugl/cugl.h>

class RayCastController : public b2RayCastCallback {
 public:
  RayCastController() : m_fixture(NULL) {}

  /**
   * Inititalize the ray cast controller.
   */
  bool init() {
    m_fixture = NULL;
    return true;
  }

  /**
   * Returns a new ray cast controller.
   *
   * @return a new ray cast controller
   */
  static std::shared_ptr<RayCastController> alloc() {
    std::shared_ptr<RayCastController> result =
        std::make_shared<RayCastController>();

    if (result->init()) {
      return result;
    }
    return nullptr;
  }

  float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
                      const b2Vec2& normal, float fraction) {
    m_fixture = fixture;
    m_point = point;
    m_normal = normal;
    m_fraction = fraction;
    return fraction;
  }

  b2Fixture* m_fixture;
  b2Vec2 m_point;
  b2Vec2 m_normal;
  float m_fraction;
};

#endif /* CONTROLLERS_RAYCASTCONTROLLER_H */
