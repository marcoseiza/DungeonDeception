#include "ParticleController.h"

#pragma mark -
#pragma mark ParticlePropsSetters

#pragma mark Position
ParticleProps* ParticleProps::setPosition(cugl::Vec2 pos) {
  _position.set(pos);
  return this;
};

ParticleProps* ParticleProps::setPosition(float x, float y) {
  _position.set(x, y);
  return this;
};

#pragma mark Velocity
ParticleProps* ParticleProps::setVelocity(cugl::Vec2 vel) {
  _velocity.set(vel);
  return this;
};

ParticleProps* ParticleProps::setVelocity(float vx, float vy) {
  _velocity.set(vx, vy);
  return this;
};

#pragma mark Scale
ParticleProps* ParticleProps::setScaleStart(float scale) {
  _scale_start = scale;
  return this;
}

ParticleProps* ParticleProps::setScaleEnd(float scale) {
  _scale_end = scale;
  return this;
}

#pragma mark Lifetime
ParticleProps* ParticleProps::setLifeTime(float life_time) {
  _life_time = life_time;
  return this;
}

#pragma mark Color
ParticleProps* ParticleProps::setColorStart(cugl::Color4 color) {
  _color_start = color;
  return this;
}

ParticleProps* ParticleProps::setColorEnd(cugl::Color4 color) {
  _color_end = color;
  return this;
}

//
//
//
//

#pragma mark -
#pragma mark Particle Controller

bool ParticleController::init(
    const std::shared_ptr<cugl::scene2::SceneNode>& particle_world) {
  _particle_world = particle_world;

  _particle_pool.resize(kMaxNumOfParticles);

  for (Particle& particle : _particle_pool) {
    particle.node =
        cugl::scene2::PolygonNode::allocWithBounds(cugl::Rect(0, 0, 1, 1));
    particle.node->setVisible(false);

    _particle_world->addChild(particle.node);
  }

  return true;
}

void ParticleController::update(float timestep) {
  for (Particle& particle : _particle_pool) {
    particle.active = (particle.life_remaining > 0);
    particle.life_remaining -= timestep;
    particle.node->setPosition(particle.node->getPosition() +
                               particle.props._velocity * ts);

    float life = particle.life_remaining / particle.props._life_time;
    cugl::Color4 color = particle.props._color_start;
    particle.node->setColor(color.lerp(particle.props._color_end, life));
    particle.node->setScale(particle.props._scale_start * (1.f - life) +
                            particle.props._scale_end * life);

    if (!particle.active) {
      particle.node->setVisible(false);
      continue;
    }
  }
}

void ParticleController::emit(const ParticleProps& props) {
  Particle& particle = _particle_pool[_pool_index];

  particle.props = props;
  particle.active = true;

  particle.life_remaining = props._life_time;

  _pool_index = --_pool_index % _particle_pool.size();
}
