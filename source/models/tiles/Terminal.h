#ifndef MODELS_TILES_TERMINAL_H_
#define MODELS_TILES_TERMINAL_H_

#include <cugl/cugl.h>

#include "BasicTile.h"
#include "TerminalSensor.h"
#include "Wall.h"

/**
 * This class implements the BasicTile class and adds a physics object that the
 * player can collide with.
 */
class Terminal : public Wall {
 protected:
  /** A reference to the physics object of the tile. */
  std::shared_ptr<cugl::physics2::PolygonObstacle> _obstacle;
  /** A reference to the physics object name of the tile. */
  std::shared_ptr<std::string> _terminal_sensor_name;
  /** A reference to the sensor shape to keep it alive for instantiation. */
  b2CircleShape _sensor_shape;

  /** A terminal radius texture when the terminal is neither activated nor
   * corrupted. */
  std::shared_ptr<cugl::Texture> _reg;
  /** A terminal radius texture when the terminal is activated. */
  std::shared_ptr<cugl::Texture> _actv;
  /** A terminal radius texture when the terminal is corrupted. */
  std::shared_ptr<cugl::Texture> _corr;

 public:
  /**
   * Creates an empty scene graph node with the degenerate texture.
   *
   * This constructor should never be called directly, as this is an abstract
   * class.
   */
  Terminal() : Wall() { _classname = "Terminal"; }

  /**
   * Deletes this node, releasing all resources.
   */
  ~Terminal() { dispose(); }

  /**
   * Disposes all of the resources used by this node.
   *
   * A disposed Node can be safely reinitialized. Any children owned by this
   * node will be released.  They will be deleted if no other object owns them.
   *
   * It is unsafe to call this on a Node that is still currently inside of
   * a scene graph.
   */
  virtual void dispose() override {
    _obstacle = nullptr;
    BasicTile::dispose();
  }

  virtual std::shared_ptr<SceneNode> copy(
      const std::shared_ptr<SceneNode>& dst) const override {
    return BasicTile::copy(dst);
  }

  /**
   * Initializes a tile node with the given JSON specificaton.
   *
   * This initializer is designed to receive the "data" object from the
   * JSON passed to {@link Scene2Loader}. This JSON format supports all
   * of the attribute values of its parent class.
   *
   * @param loader    The scene loader passing this JSON file
   * @param data      The JSON object specifying the node
   *
   * @return true if initialization was successful.
   */
  virtual bool initWithData(
      const cugl::Scene2Loader* loader,
      const std::shared_ptr<cugl::JsonValue>& data) override;

  /**
   * Returns a new tile node with the given JSON specificaton.
   *
   * This initializer is designed to receive the "data" object from the
   * JSON passed to {@link Scene2Loader}. This JSON format supports all
   * of the attribute values of its parent class.
   *
   * @param loader    The scene loader passing this JSON file
   * @param data      The JSON object specifying the node
   *
   * @return a new polygon with the given JSON specificaton.
   */
  static std::shared_ptr<SceneNode> allocWithData(
      const cugl::Scene2Loader* loader,
      const std::shared_ptr<cugl::JsonValue>& data) {
    std::shared_ptr<Terminal> result = std::make_shared<Terminal>();
    if (!result->initWithData(loader, data)) {
      result = nullptr;
    }
    return std::dynamic_pointer_cast<SceneNode>(result);
  }

  /**
   * Initializes the box 2d terminal sensor
   *
   * @return The obstacle it created for easy chaining.
   */
  virtual std::shared_ptr<cugl::physics2::PolygonObstacle> initBox2d() override;

  /**
   * Set the terminal to the activated state.
   */
  void setActivated() {
    if (_actv != nullptr) setTexture(_actv);
  }

  /**
   * Set the terminal to the corrupted state.
   */
  void setCorrupted() {
    if (_corr != nullptr) setTexture(_corr);
  }

  /**
   * Set the terminal to the neutral state.
   */
  void setRegular() {
    if (_reg != nullptr) setTexture(_reg);
  }
};

#endif  // MODELS_TILES_TERMINAL_H_#pragma once
