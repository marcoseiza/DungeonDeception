#pragma once

#ifndef MODELS_TILES_TERMINALSENSOR_H
#define MODELS_TILES_TERMINALSENSOR_H

#include <cugl/cugl.h>
#include <stdio.h>

class TerminalSensor : public cugl::physics2::BoxObstacle {
 private:
  /** Represents the area for terminal activation. */
  b2Fixture* _terminal_sensor;
  /** Keeps an instance of the name alive for collision detection. */
  std::shared_ptr<std::string> _terminal_sensor_name;
  /** The node for debugging the terminal sensor */
  std::shared_ptr<cugl::scene2::WireNode> _terminal_sensor_node;
  /** Whether the terminal has been activated or not. */
  bool _activated = false;

 public:
#pragma mark Constructors
  /**
   * Creates a terminal sensor with the given position and data.
   *
   * @param pos The sensor position.
   */
  TerminalSensor(void)
      : BoxObstacle(),
        _terminal_sensor(nullptr),
        _terminal_sensor_name(nullptr) {}

  /**
   * Disposes the grunt.
   */
  ~TerminalSensor() { dispose(); }

  /**
   * Disposes the grunt.
   */
  void dispose() { _terminal_sensor = nullptr; }

  /**
   * Initializes a new terminal sensor with the given position and name.
   *
   * @param  pos      Initial position in world coordinates.
   * @param  name     The name of the sensor.
   *
   * @return  true if the obstacle is initialized properly, false otherwise.
   */
  virtual bool init(const cugl::Vec2 pos, string name);

#pragma mark Static Constructors
  /**
   * Returns a new box object at the given point with no size.
   *
   * @param pos   Initial position in world coordinates.
   *
   * @return a new box object at the given point with no size.
   */
  static std::shared_ptr<TerminalSensor> alloc(const cugl::Vec2 pos,
                                               string name) {
    std::shared_ptr<TerminalSensor> result = std::make_shared<TerminalSensor>();
    return (result->init(pos, name) ? result : nullptr);
  }

#pragma mark -
#pragma mark Physics Methods
  /**
   * Creates the physics Body(s) for this object, adding them to the world.
   *
   * This method overrides the base method to keep your ship from spinning.
   *
   * @param world Box2D world to store body
   *
   * @return true if object allocation succeeded
   */
  void createFixtures() override;

  /**
   * Release the fixtures for this body, reseting the shape
   *
   * This is the primary method to override for custom physics objects.
   */
  void releaseFixtures() override;

  /**
   * @return Returns the activation state of the terminal.
   */
  bool isActivated() const { return _activated; }

  /**
   * Activates a terminal.
   */
  void activate() { _activated = true; }

  /**
   * Deactivates a terminal.
   */
  void deactivate() { _activated = false; }
};
#endif  // MODELS_TILES_TERMINALSENSOR_H_
#pragma once
