#include "ParticleController.h"

#include "Random.h"

#pragma mark -
#pragma mark ParticlePropsSetters

ParticleProps* ParticleProps::setForce(bool force) {
  _force = force;
  return this;
}

ParticleProps* ParticleProps::setScreenCoord(bool val) {
  _is_screen_coord = val;
  return this;
}

ParticleProps* ParticleProps::setOrder(bool val) {
  _order = val;
  return this;
}

ParticleProps* ParticleProps::setRoomId(int id) {
  _room_id = id;
  return this;
}

#pragma mark Emit Particle Properties
ParticleProps* ParticleProps::setPosition(const cugl::Vec2& pos) {
  if (_type == EMIT) _position.set(pos);
  return this;
};

ParticleProps* ParticleProps::setPosition(float x, float y) {
  if (_type == EMIT) _position.set(x, y);
  return this;
};

ParticleProps* ParticleProps::setPositionVariation(const cugl::Vec2& var) {
  _position_variation.set(var);
  return this;
}

ParticleProps* ParticleProps::setPositionVariation(float var_x, float var_y) {
  _position_variation.set(var_x, var_y);
  return this;
}

ParticleProps* ParticleProps::setVelocity(const cugl::Vec2& vel) {
  if (_type == EMIT) _velocity.set(vel);
  return this;
};

ParticleProps* ParticleProps::setVelocity(float vx, float vy) {
  if (_type == EMIT) _velocity.set(vx, vy);
  return this;
};

ParticleProps* ParticleProps::setVelocityVariation(const cugl::Vec2& var) {
  if (_type == EMIT) _velocity_variation.set(var);
  return this;
}

ParticleProps* ParticleProps::setVelocityVariation(float vx_var, float vy_var) {
  if (_type == EMIT) _velocity_variation.set(vx_var, vy_var);
  return this;
}

#pragma mark Path Particle Properties

ParticleProps* ParticleProps::setPosStart(const cugl::Vec2& pos) {
  if (_type == PATH) _pos_start.set(pos);
  return this;
}
ParticleProps* ParticleProps::setPosStart(float x, float y) {
  if (_type == PATH) _pos_start.set(x, y);
  return this;
}

ParticleProps* ParticleProps::setPosEnd(const cugl::Vec2& pos) {
  if (_type == PATH) _pos_end.set(pos);
  return this;
}
ParticleProps* ParticleProps::setPosEnd(float x, float y) {
  if (_type == PATH) _pos_end.set(x, y);
  return this;
}

ParticleProps* ParticleProps::setPathVariation(float var) {
  if (_type == PATH) _path_variation = var;
  return this;
}

ParticleProps* ParticleProps::setEasingFunction(
    const std::function<float(float)>& easing_function) {
  if (_type == PATH) {
    _easing = easing_function;
    _easing_separate = false;
  }
  return this;
}

ParticleProps* ParticleProps::setEasingFunctionPosX(
    const std::function<float(float)>& easing_function) {
  if (_type == PATH) {
    _easing_pos_x = easing_function;
    _easing_separate = true;
  }
  return this;
}

ParticleProps* ParticleProps::setEasingFunctionPosY(
    const std::function<float(float)>& easing_function) {
  if (_type == PATH) {
    _easing_pos_y = easing_function;
    _easing_separate = true;
  }
  return this;
}

#pragma mark Scale
ParticleProps* ParticleProps::setSizeStart(float size) {
  _scale_start = size;
  return this;
}

ParticleProps* ParticleProps::setSizeEnd(float size) {
  _scale_end = size;
  return this;
}

#pragma mark Lifetime
ParticleProps* ParticleProps::setLifeTime(float life_time) {
  _life_time = life_time;
  return this;
}

ParticleProps* ParticleProps::setWaitTime(float wait_time) {
  _wait_time = wait_time;
  return this;
}

#pragma mark Color
ParticleProps* ParticleProps::setColorStart(const cugl::Color4& color) {
  _color_start = color;
  return this;
}

ParticleProps* ParticleProps::setColorEnd(const cugl::Color4& color) {
  _color_end = color;
  return this;
}

#pragma mark Rotation
ParticleProps* ParticleProps::setAngularSpeed(float speed) {
  _angular_speed = speed;
  return this;
}

ParticleProps* ParticleProps::setRotateClockwise(bool clockwise) {
  _clockwise = (clockwise) ? 1.0f : -1.0f;
  return this;
}

//
//
//
//

#pragma mark -
#pragma mark Particle Controller

bool ParticleController::init(
    const std::shared_ptr<cugl::scene2::SceneNode>& particle_world,
    const std::shared_ptr<cugl::scene2::SceneNode>& particle_screen) {
  _particle_world = particle_world;
  _particle_screen = particle_screen;

  _particle_pool.resize(kMaxNumOfParticles + kMaxNumOfParticlesScreen);

  // Add children backwards to render oldest first.
  for (int i = kMaxNumOfParticles - 1; i >= 0; i--) {
    using namespace cugl;
    Particle& particle = _particle_pool[i];
    particle.node = scene2::PolygonNode::allocWithPoly(Rect(0, 0, 1, 1));
    particle.node->setVisible(false);
    particle.active = false;
    _particle_world->addChild(particle.node);
  }

  // Add children backwards to render oldest first.
  for (int i = (int)_particle_pool.size() - 1; i >= kMaxNumOfParticles; i--) {
    using namespace cugl;
    Particle& particle = _particle_pool[i];
    particle.node = scene2::PolygonNode::allocWithPoly(Rect(0, 0, 1, 1));
    particle.node->setVisible(false);
    particle.active = false;
    particle.node->setPosition(_particle_screen->getContentSize() / 2);
    _particle_screen->addChild(particle.node);
  }

  return true;
}

void ParticleController::dispose() {
  _particle_pool.clear();
  _particle_world = nullptr;
}

void ParticleController::update(float timestep) {
  for (Particle& particle : _particle_pool) {
    if (!particle.active) continue;

    if (particle.wait_remaining > 0) {
      particle.wait_remaining -= timestep;
      continue;
    }

    if (particle.life_remaining <= 0) {
      particle.active = false;
      particle.node->setVisible(false);
      continue;
    }

    particle.life_remaining -= timestep;
    float life = 1.0f - particle.life_remaining / particle.props._life_time;

    if (particle.props._type == ParticleProps::Type::EMIT) {
      cugl::Vec2 diff = particle.props._velocity * timestep;
      particle.node->setPosition(particle.node->getPosition() + diff);

    } else if (particle.props._type == ParticleProps::Type::PATH) {
      float a = particle.props._easing(life);

      // Variate path pos from path slightly.
      cugl::Vec2 path_pos;

      if (particle.props._easing_separate) {
        float a_x = particle.props._easing_pos_x(life);
        path_pos.x = particle.props._pos_end.x * a_x +
                     particle.props._pos_start.x * (1 - a_x);
        float a_y = particle.props._easing_pos_y(life);
        path_pos.y = particle.props._pos_end.y * a_y +
                     particle.props._pos_start.y * (1 - a_y);
      } else {
        cugl::Vec2 dist = particle.props._pos_end - particle.props._pos_start;
        path_pos = dist * a + particle.props._pos_start;
      }

      cugl::Vec2 dS = path_pos - particle.node->getPosition();
      cugl::Vec2 perp = (dS).getNormalization().getPerp();

      // Quadratic function, max at a = 0.5.
      // So max variation on middle of path.
      float diff = -0.4f * (2 * a - 1) * (2 * a - 1) +
                   1 * particle.props._path_variation;
      // Vary between going towards center and away.
      diff *= (Random::Float() * 2 - 1);

      particle.dist_from_path +=
          (perp * diff * timestep) * (Random::Float() - a);

      particle.node->setPosition(path_pos + particle.dist_from_path);
    }

    particle.node->setAngle(particle.node->getAngle() +
                            particle.props._angular_speed *
                                particle.props._clockwise * timestep);

    cugl::Color4 color = particle.props._color_start;
    particle.node->setColor(color.lerp(particle.props._color_end, life));
    particle.node->setScale(particle.props._scale_start * (1.f - life) +
                            particle.props._scale_end * life);
  }
}

void ParticleController::emit(const ParticleProps& props, int num,
                              float buff_time) {
  for (int i = 0; i < num; i++) {
    Particle* particle = &_particle_pool[_pool_index];
    if (props._is_screen_coord) {
      particle = &_particle_pool[_pool_index_screen];
    }

    if (!props._force && particle->active) {
      return;
    }

    if (props._is_screen_coord) {
      _pool_index_screen = --_pool_index_screen;
      if (_pool_index_screen == kMaxNumOfParticles - 1)
        _pool_index_screen = (int)_particle_pool.size() - 1;
    } else {
      _pool_index = --_pool_index;
      if (_pool_index == -1) _pool_index = kMaxNumOfParticles - 1;
    }

    particle->props = props;
    particle->active = true;
    particle->node->setVisible(true);
    particle->life_remaining = props._life_time;
    particle->wait_remaining = props._wait_time + (buff_time * i);

    if (props._type == ParticleProps::Type::EMIT) {
      particle->props._position +=
          particle->props._position_variation *
          cugl::Vec2(Random::Float() * 2 - 1, Random::Float() * 2 - 1);
      particle->node->setPosition(props._position);

      particle->props._velocity +=
          particle->props._velocity_variation *
          cugl::Vec2(Random::Float() * 2 - 1, Random::Float() * 2 - 1);
    } else if (props._type == ParticleProps::Type::PATH) {
      particle->props._pos_start +=
          particle->props._position_variation *
          cugl::Vec2(Random::Float() * 2 - 1, Random::Float() * 2 - 1);
      particle->props._pos_end +=
          particle->props._position_variation *
          cugl::Vec2(Random::Float() * 2 - 1, Random::Float() * 2 - 1);

      particle->node->setPosition(props._pos_start);
    }

    particle->node->setColor(props._color_start);
    particle->node->setScale(props._scale_start);
    if (!props._is_screen_coord)
      particle->node->setPriority(std::numeric_limits<float>::max());
  }
}
