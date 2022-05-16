#ifndef CONTROLLERS_PARTICLE_CONTROLLER_H_
#define CONTROLLERS_PARTICLE_CONTROLLER_H_

#include <cugl/cugl.h>

#include "Controller.h"

#pragma mark -
#pragma mark ParticleProps

/**
 * This class will help you build a particle.
 */
class ParticleProps {
 public:
  /** The type of particle, one that emits or follows a path.*/
  enum Type {
    /* Emit particles will spawn from one place and displace with the given
       velocity. */
    EMIT,
    /** Path particles will move in a straight line from a starting position to
       an ending position. */
    PATH
  };

 private:
  Type _type;

  bool _force, _is_screen_coord, _order;
  int _room_id;

  // Emit particle properties
  cugl::Vec2 _position, _position_variation, _velocity, _velocity_variation;

  // Path particle properties
  cugl::Vec2 _pos_start, _pos_end;
  float _path_variation;
  std::function<float(float)> _easing;

  // Shared values between state
  float _scale_start, _scale_end, _angular_speed, _life_time, _wait_time;
  float _clockwise;
  cugl::Color4 _color_start, _color_end;

 public:
  /**
   * Construct a new particle with given type.
   * @param type Particle Type
   */
  ParticleProps(Type type)
      : _type(type),
        _force(true),
        _is_screen_coord(false),
        _order(false),
        _room_id(0),
        _scale_start(1.0f),
        _scale_end(1.0f),
        _life_time(1.0f),
        _angular_speed(1.0f),
        _clockwise(true),
        _path_variation(1.0f),
        _easing(nullptr),
        _color_start(cugl::Color4::WHITE),
        _color_end(cugl::Color4::WHITE) {}

  /** Construct a new particle of type EMIT */
  ParticleProps() : ParticleProps(EMIT) {}
  /** Destroy the Particle Props object */
  ~ParticleProps() {}

  /**
   * Set if the particle can force other particles off the pool despite them
   * still being active.
   * @param force To force particles off or not.
   */
  ParticleProps* setForce(bool force);
  /**
   * Get if particles will force other particles off of the pool.
   * @return Wether this particle will force.
   */
  bool getForce() const { return _force; }

  /**
   * Set whether this particle will be drawn in screen coordinates. Screen
   * coordinates in this case are OpenGL coordinates.
   * @param val Wether to draw in screen coords.
   */
  ParticleProps* setScreenCoord(bool val);
  /** @return Whether this particle is drawn in screen coords */
  bool isWorldCoord() const { return _is_screen_coord; }

  /**
   * Set if this particle should be given a priority for orderend rendering.
   * Default priority is -0.9, which is always above grass.
   * @param val Wether to give order priority values.
   */
  ParticleProps* setOrder(bool val);
  /** @return Wether this particle is give order priority values. */
  bool isOrder() const { return _order; }

  /**
   * Set the room in which this particle was spawned in, so that they can be
   * given draw ordering. ONLY APPLICABLE if setOrder() is set to true.
   * @param id The room id.
   */
  ParticleProps* setRoomId(int id);
  /**
   * Get the room id the particle spawned in. ONLY APPLICABLE if setOrder() is
   * set to true.
   * @return The room Id.
   */
  int getRoomId() const { return _room_id; }

  /**
   * Set the starting position of an EMIT particle. If this particle isn't an
   * EMIT type, this function will not do anything.
   * @param pos The vector position.
   */
  ParticleProps* setPosition(const cugl::Vec2& pos);
  /**
   * Set the starting position of an EMIT particle. If this particle isn't an
   * EMIT type, this function will not do anything.
   * @param x The x value of the position.
   * @param y The y value of the position.
   */
  ParticleProps* setPosition(float x, float y);
  /**
   * Get the starting position of an EMIT particle.
   * @return The starting position.
   */
  cugl::Vec2 getPosition() const { return _position; }

  /**
   * Set the starting position variation of the particle. If it is an EMIT
   * particle, it is only applied to the starting position. If it is a PATH
   * particle, it is also applied to the end position.
   * @param pos The vector position.
   */
  ParticleProps* setPositionVariation(const cugl::Vec2& var);
  /**
   * Set the starting position variation of the particle. If it is an EMIT
   * particle, it is only applied to the starting position. If it is a PATH
   * particle, it is also applied to the end position.
   * @param var_x The x value of the position variation.
   * @param var_y The y value of the position variation.
   */
  ParticleProps* setPositionVariation(float var_x, float var_y);
  /**
   * Get the starting position variation of the particle.
   * @return The starting position variation.
   */
  cugl::Vec2 getPositionVariation() const { return _position_variation; }

  /**
   * Set the velocity of an EMIT particle. If this particle isn't an
   * EMIT type, this function will not do anything.
   * @param vel The velocity.
   */
  ParticleProps* setVelocity(const cugl::Vec2& vel);
  /**
   * Set the velocity of an EMIT particle. If this particle isn't an
   * EMIT type, this function will not do anything.
   * @param vx The x velocity.
   * @param vy The y velocity.
   */
  ParticleProps* setVelocity(float vx, float vy);
  /**
   * Get the velocity of an EMIT particle.
   * @return The velocity of the particle.
   */
  cugl::Vec2 getVelocity() const { return _velocity; }

  /**
   * Set the velocity variation of an EMIT particle. If this particle isn't an
   * EMIT type, this function will not do anything.
   * @param var The velocity variation.
   */
  ParticleProps* setVelocityVariation(const cugl::Vec2& var);
  /**
   * Set the velocity variation of an EMIT particle. If this particle isn't an
   * EMIT type, this function will not do anything.
   * @param vx_var The x velocity variation.
   * @param vy_var The y velocity variation.
   */
  ParticleProps* setVelocityVariation(float vx_var, float vy_var);
  /**
   * Get the velocity variation of an EMIT particle.
   * @return Get the velocity variation of the particle.
   */
  cugl::Vec2 getVelocityVariation() const { return _velocity_variation; }

  /**
   * Set the starting position of an PATH particle. If this particle isn't an
   * PATH type, this function will not do anything.
   * @param pos The vector position.
   */
  ParticleProps* setPosStart(const cugl::Vec2& pos);
  /**
   * Set the starting position of an PATH particle. If this particle isn't an
   * PATH type, this function will not do anything.
   * @param x The x value of the position.
   * @param y The y value of the position.
   */
  ParticleProps* setPosStart(float x, float y);
  /**
   * Get the starting position of an PATH particle
   * @return Get the starting position of he particle.
   */
  cugl::Vec2 getPosStart() const { return _position; }

  /**
   * Set the ending position of an PATH particle. If this particle isn't an
   * PATH type, this function will not do anything.
   * @param pos The vector position.
   */
  ParticleProps* setPosEnd(const cugl::Vec2& pos);
  /**
   * Set the ending position of an PATH particle. If this particle isn't an
   * PATH type, this function will not do anything.
   * @param x The x value of the position.
   * @param y The y value of the position.
   */
  ParticleProps* setPosEnd(float x, float y);
  /**
   * Get the ending position of an PATH particle
   * @return Get the ending position of he particle.
   */
  cugl::Vec2 getPosEnd() const { return _position; }

  /**
   * Set how far the particle will deviate from the path of a PATH particle. If
   * this particle isn't an PATH type, this function will not do anything.
   * @param var The path variation.
   */
  ParticleProps* setPathVariation(float var);
  /**
   * Get how far the particle will deviate from the path of a PATH particle.
   * @param var Get how far the particle will deviate from the path.
   */
  float getPathVariation() const { return _path_variation; }

  /**
   * Set the easing function for traversing the path of a PATH particle. If
   * this particle isn't an PATH type, this function will not do anything.
   * @param easing_function The easing function cugl::EasingFunction::...
   */
  ParticleProps* setEasingFunction(
      const std::function<float(float)>& easing_function);
  /**
   * Get the easing function for traversing the path of a PATH particle.
   * @return The easing function.
   */
  std::function<float(float)> getEasingFunction() const { return _easing; }

  /**
   * Set the starting size of the particle.
   * @param scale The starting size.
   */
  ParticleProps* setSizeStart(float size);
  /**
   * Get the starting size of the particle.
   * @return The starting size.
   */
  float getSizeStart() const { return _scale_start; }

  /**
   * Set the ending size of the particle.
   * @param scale The ending size.
   */
  ParticleProps* setSizeEnd(float size);
  /**
   * Get the ending size of the particle.
   * @return The ending size.
   */
  float getSizeEnd() const { return _scale_end; }

  /**
   * Set the lifetime of the particle in seconds.
   * @param life_time The lifetime in seconds.
   */
  ParticleProps* setLifeTime(float life_time);
  /**
   * Get the lifetime of the particle in seconds.
   * @return The lifetime in seconds.
   */
  float getLifeTime() const { return _life_time; }

  /**
   * Set the wait time of the particle in seconds.
   * @param wait_time The wait time in seconds.
   */
  ParticleProps* setWaitTime(float wait_time);
  /**
   * Get the wait time of the particle in seconds.
   * @return The wait time in seconds.
   */
  float getWaitTime() const { return _wait_time; }

  /**
   * Set the starting color of the particle.
   * @param color The starting color.
   */
  ParticleProps* setColorStart(const cugl::Color4& color);
  /**
   * Get the starting color of the particle.
   * @return The starting color.
   */
  cugl::Color4 getColorStart() const { return _color_start; }

  /**
   * Set the ending color of the particle.
   * @param color The ending color.
   */
  ParticleProps* setColorEnd(const cugl::Color4& color);
  /**
   * Get the ending color of the particle.
   * @return The ending color.
   */
  cugl::Color4 getColorEnd() const { return _color_end; }

  /**
   * Set the angular speed of the particle for rotation.
   * @param speed The angular speed.
   */
  ParticleProps* setAngularSpeed(float speed);
  /**
   * Get the angular speed of the particle for rotation.
   * @return The angular speed.
   */
  float getAngularSpeed() const { return _angular_speed; }

  /**
   * Set whether the particle should rotate clockwise. False = counter
   * clockwise.
   * @param clockwise Wether to rotate clockwise.
   */
  ParticleProps* setRotateClockwise(bool clockwise);
  /**
   * Get whether the particle will rotate clockwise. False = counter
   * clockwise.
   * @return Wether to rotate clockwise.
   */
  bool getRotateClockwise() const { return _clockwise == 1.0f; }

  friend class ParticleController;
};

//
//
//
//
//

#pragma mark -
#pragma mark ParticleController

/**
 * This is the particle controller that handles creating and destroying
 * particles in the game.
 */
class ParticleController : public Controller {
 public:
  /** A struct to represent a particle on the screen. */
  struct Particle {
    /** A copy of the props that define the particle. */
    ParticleProps props;
    /** The node for displaying the particle. */
    std::shared_ptr<cugl::scene2::PolygonNode> node = nullptr;
    /** The life remaining in the particle. */
    float life_remaining = 0.0f;
    /** The wait remaining in the particle. */
    float wait_remaining = 0.0f;
    /** Weather the particle is active. */
    bool active = false;
    /** The distance from the path, used by PATH particles.*/
    cugl::Vec2 dist_from_path;
  };

 private:
  /** A reference to the scene2 world for particles.*/
  std::shared_ptr<cugl::scene2::SceneNode> _particle_world;

  /** A reference to the scene2 screen for particles.*/
  std::shared_ptr<cugl::scene2::SceneNode> _particle_screen;

  /**
   * A pool of all the particles.
   * [ World Particles | Screen Particles]
   * */
  std::vector<Particle> _particle_pool;

  /** The max number of particles in world. */
  const int kMaxNumOfParticles = 800;
  /** The max number of particles in screen. */
  const int kMaxNumOfParticlesScreen = 200;

  /** The current pool index. */
  int _pool_index;

  /** The current pool index screen. */
  int _pool_index_screen;

 public:
  /** Construct an empty particle controller, use alloc instead */
  ParticleController()
      : _pool_index(kMaxNumOfParticles - 1),
        _pool_index_screen(kMaxNumOfParticlesScreen + kMaxNumOfParticles - 1) {}
  /** Destroy the particle controller */
  ~ParticleController() { dispose(); }

  /**
   * Initialize the particle controller with the given particle world scene2
   * node.
   * @param particle_world The world to draw particles to.
   * @param particle_screen The screen to draw particles to.
   */
  bool init(const std::shared_ptr<cugl::scene2::SceneNode>& particle_world,
            const std::shared_ptr<cugl::scene2::SceneNode>& particle_screen);

  /** Dispose of all internal resources/references in the class. */
  void dispose() override;

  /**
   * Allocate a new particle controller in a shared_ptr.
   * @param particle_world The world to draw particles to.
   * @param particle_screen The screen to draw particles to.
   * @return A shared pointer holding the new ParticleController.
   */
  static std::shared_ptr<ParticleController> alloc(
      const std::shared_ptr<cugl::scene2::SceneNode>& particle_world,
      const std::shared_ptr<cugl::scene2::SceneNode>& particle_screen) {
    auto res = std::make_shared<ParticleController>();
    return (res->init(particle_world, particle_screen)) ? res : nullptr;
  }

  /**
   * Update the controller.
   * @param timestep The amount of time since last frame in seconds.
   */
  void update(float timestep) override;

  /**
   * Emit one particle with the given props.
   * @param props The particle props to define the particle.
   * @param num The number of particles to emit.
   * @param buff_time The time in seconds between emits.
   */
  void emit(const ParticleProps& props, int num = 1, float buff_time = 0.0f);

  /**
   * Get the pool of particles.
   * @return The pool of particles.
   */
  std::vector<Particle>& getParticles() { return _particle_pool; }
};

#endif  // CONTROLLERS_PARTICLE_CONTROLLER_H_