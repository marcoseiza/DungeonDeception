#ifndef CONTROLLERS_PARTICLE_CONTROLLER_H_
#define CONTROLLERS_PARTICLE_CONTROLLER_H_

#include <cugl/cugl.h>

#pragma mark -
#pragma mark ParticleProps

class ParticleProps {
  cugl::Vec2 _position, _velocity;
  float _scale_start, _scale_end, _life_time;
  cugl::Color4 _color_start, _color_end;

 public:
  ParticleProps()
      : _scale_start(1.0f),
        _scale_end(1.0f),
        _life_time(1.0f),
        _color_start(cugl::Color4::WHITE),
        _color_end(cugl::Color4::WHITE) {}

  ~ParticleProps() {}

  ParticleProps* setPosition(cugl::Vec2 pos);
  ParticleProps* setPosition(float x, float y);
  cugl::Vec2 getPosition() { return _position; }

  ParticleProps* setVelocity(cugl::Vec2 vel);
  ParticleProps* setVelocity(float vx, float vy);
  cugl::Vec2 getVelocity() { return _velocity; }

  ParticleProps* setScaleStart(float scale);
  float getScaleStart() { return _scale_start; }

  ParticleProps* setScaleEnd(float scale);
  float getScaleEnd() { return _scale_end; }

  ParticleProps* setLifeTime(float life_time);
  float getLifeTime() { return _life_time; }

  ParticleProps* setColorStart(cugl::Color4 color);
  float getColorStart() { return _color_start; }

  ParticleProps* setColorEnd(cugl::Color4 color);
  float getColorStart() { return _color_end; }

  friend class ParticleController;
};

//
//
//
//
//

#pragma mark -
#pragma mark ParticleController

class ParticleController {
  struct Particle {
    const ParticleProps& props;

    std::shared_ptr<cugl::scene2::PolygonNode> node;

    float life_remaining = 1.0f;
    bool active = false;
  };

  std::shared_ptr<cugl::scene2::SceneNode> _particle_world;

  std::vector<Particle> _particle_pool;

  const int kMaxNumOfParticles = 1000;

  int _pool_index;

 public:
  ParticleController() : _pool_index(kMaxNumOfParticles - 1) {}
  ~ParticleController() {}

  bool init(const std::shared_ptr<cugl::scene2::SceneNode>& particle_world);

  static std::shared_ptr<ParticleController> alloc(
      const std::shared_ptr<cugl::scene2::SceneNode>& particle_world) {
    auto res = std::make_shared<ParticleController>();
    return (res->init(particle_world)) ? res : nullptr;
  }

  void update(float timestep);

  void emit(const ParticleProps& props);
};

#endif  // CONTROLLERS_PARTICLE_CONTROLLER_H_