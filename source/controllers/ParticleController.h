#ifndef CONTROLLERS_PARTICLE_CONTROLLER_H_
#define CONTROLLERS_PARTICLE_CONTROLLER_H_

#include <cugl/cugl.h>

#include "Controller.h"

#pragma mark -
#pragma mark ParticleProps

class ParticleProps {
 public:
  enum Type { EMIT, PATH };

 private:
  Type _type;

  bool _force;

  bool _is_world_coord;

  // Emit particle properties
  cugl::Vec2 _position, _velocity, _velocity_variation;

  // Path particle properties
  cugl::Vec2 _pos_start, _pos_end;
  float _path_variation;
  std::function<float(float)> _easing;

  // Shared values between state
  float _scale_start, _scale_end, _angular_speed, _life_time;
  float _clockwise;
  cugl::Color4 _color_start, _color_end;

 public:
  ParticleProps(Type type)
      : _type(type),
        _force(true),
        _is_world_coord(false),
        _scale_start(1.0f),
        _scale_end(1.0f),
        _life_time(1.0f),
        _angular_speed(1.0f),
        _clockwise(true),
        _path_variation(1.0f),
        _easing(cugl::EasingFunction::linear),
        _color_start(cugl::Color4::WHITE),
        _color_end(cugl::Color4::WHITE) {}
  ParticleProps() : ParticleProps(EMIT) {}

  ~ParticleProps() {}

  ParticleProps* setForce(bool force);
  bool getForce() { return _force; }

  ParticleProps* setWorldCoord(bool val);
  bool isWorldCoord() { return _is_world_coord; }

  ParticleProps* setPosition(const cugl::Vec2& pos);
  ParticleProps* setPosition(float x, float y);
  cugl::Vec2 getPosition() { return _position; }

  ParticleProps* setVelocity(const cugl::Vec2& vel);
  ParticleProps* setVelocity(float vx, float vy);
  cugl::Vec2 getVelocity() { return _velocity; }

  ParticleProps* setVelocityVariation(const cugl::Vec2& var);
  ParticleProps* setVelocityVariation(float vx_var, float vy_var);
  cugl::Vec2 getVelocityVariation() { return _velocity_variation; }

  ParticleProps* setPosStart(const cugl::Vec2& pos);
  ParticleProps* setPosStart(float x, float y);
  cugl::Vec2 getPosStart() { return _position; }

  ParticleProps* setPosEnd(const cugl::Vec2& pos);
  ParticleProps* setPosEnd(float x, float y);
  cugl::Vec2 getPosEnd() { return _position; }

  ParticleProps* setPathVariation(float var);
  float getPathVariation() { return _path_variation; }

  ParticleProps* setEasingFunction(
      const std::function<float(float)>& easing_function);
  std::function<float(float)> getEasingFunction() { return _easing; }

  ParticleProps* setSizeStart(float scale);
  float getScaleStart() { return _scale_start; }

  ParticleProps* setSizeEnd(float scale);
  float getScaleEnd() { return _scale_end; }

  ParticleProps* setLifeTime(float life_time);
  float getLifeTime() { return _life_time; }

  ParticleProps* setColorStart(const cugl::Color4& color);
  cugl::Color4 getColorStart() { return _color_start; }

  ParticleProps* setColorEnd(const cugl::Color4& color);
  cugl::Color4 getColorEnd() { return _color_end; }

  ParticleProps* setAngularSpeed(float speed);
  float getAngularSpeed() { return _angular_speed; }

  ParticleProps* setRotateClockwise(bool clockwise);
  bool getRotateClockwise() { return _clockwise == 1.0f; }

  friend class ParticleController;
};

//
//
//
//
//

#pragma mark -
#pragma mark ParticleController

class ParticleController : public Controller {
  struct Particle {
    ParticleProps props;
    std::shared_ptr<cugl::scene2::PolygonNode> node = nullptr;
    float life_remaining = 1.0f;
    bool active = false;
    cugl::Vec2 dist_from_path;
  };

  std::shared_ptr<cugl::scene2::SceneNode> _particle_world;

  std::vector<Particle> _particle_pool;

  const int kMaxNumOfParticles = 1000;

  int _pool_index;

 public:
  ParticleController() : _pool_index(kMaxNumOfParticles - 1) {}
  ~ParticleController() { dispose(); }

  bool init(const std::shared_ptr<cugl::scene2::SceneNode>& particle_world);

  void dispose() override;

  static std::shared_ptr<ParticleController> alloc(
      const std::shared_ptr<cugl::scene2::SceneNode>& particle_world) {
    auto res = std::make_shared<ParticleController>();
    return (res->init(particle_world)) ? res : nullptr;
  }

  void update(float timestep) override;

  void emit(const ParticleProps& props);
};

#endif  // CONTROLLERS_PARTICLE_CONTROLLER_H_