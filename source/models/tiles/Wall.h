#ifndef MODELS_TILES_WALL_H_
#define MODELS_TILES_WALL_H_

#include <cugl/cugl.h>

#include "BasicTile.h"

/**
 * This class implements the BasicTile class and adds a physics object that the
 * player can collide with.
 */
class Wall : public BasicTile {
 protected:
  /** A reference to the physics object of the tile. */
  std::shared_ptr<cugl::physics2::PolygonObstacle> _obstacle;

  /** The shape of the obstacle as defined in scene2 json. */
  cugl::Poly2 _obstacle_shape;

 public:
  /**
   * Creates an empty scene graph node with the degenerate texture.
   *
   * This constructor should never be called directly, as this is an abstract
   * class.
   */
  Wall() : _obstacle(nullptr), BasicTile() { _classname = "Wall"; }

  /**
   * Deletes this node, releasing all resources.
   */
  ~Wall() { dispose(); }

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
    std::shared_ptr<Wall> result = std::make_shared<Wall>();
    if (!result->initWithData(loader, data)) result = nullptr;
    return std::dynamic_pointer_cast<SceneNode>(result);
  }

  /**
   * Returns a new wall node.
   *
   * @return A new wall node.
   */
  static std::shared_ptr<SceneNode> alloc() {
    std::shared_ptr<Wall> result = std::make_shared<Wall>();
    if (!result->init()) result = nullptr;
    return std::dynamic_pointer_cast<SceneNode>(result);
  }

  /**
   * Initializes the box 2d object for the tile including setting the position
   * and size.
   *
   * @return The obstacle it created for easy chaining.
   */
  virtual std::shared_ptr<cugl::physics2::PolygonObstacle> initBox2d();

  /**
   * @return Returns the physics object for the tile.
   */
  std::shared_ptr<cugl::physics2::PolygonObstacle> getObstacle() {
    return _obstacle;
  }
};

#endif  // MODELS_TILES_WALL_H_