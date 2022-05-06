#ifndef MODELS_TILES_DOOR_H_
#define MODELS_TILES_DOOR_H_

#include <cugl/cugl.h>

#include "BasicTile.h"

/**
 * This class implements the BasicTile class and adds a physics object that the
 * player can collide with.
 */
class Door : public BasicTile {
 public:
  /** The state of the door, used for drawing and physics. Unused, on or off. */
  enum State {
    /** The door is unused, so it should hide. */
    UNUSED,
    /** The door is on, so a laser should appear. */
    ON,
    /** The door is off, so a laser should dissapear. */
    OFF
  };

 protected:
  /** The current enumerated state of the Door. */
  State _state;

  /**
   * Handles the delegate nodes to the door. Allows for multitile structures.
   */
  struct Delegate {
    /** A list of strings that represent the path of children to the off node of
     * a delegate. The off node will be active when the Door state is OFF. */
    std::vector<std::string> off_node_key;
    /** The node that is used for the OFF state. */
    std::shared_ptr<cugl::scene2::SceneNode> off_node;

    /** A list of strings that represent the path of children to the on node of
     * a delegate. The off node will be active when the Door state is ON. */
    std::vector<std::string> on_node_key;
    /** The node that is used for the ON state. */
    std::shared_ptr<cugl::scene2::SceneNode> on_node;

    /**
     * A list of strings that represent the path of children to the unused node
     * of a delegate. The off node will be active when the Door state is UNUSED.
     */
    std::vector<std::string> unused_node_key;
    /** The node that is used for the UNUSED state. */
    std::shared_ptr<cugl::scene2::SceneNode> unused_node;
  };

  /** A list of all the delegate nodes to this door. */
  std::vector<Delegate> _delegates;

  /** A reference to the physics object of the tile. */
  std::shared_ptr<cugl::physics2::PolygonObstacle> _obstacle;

  /** The shape of the obstacle as defined in scene2 json. */
  cugl::Poly2 _obstacle_shape;

  /** A reference to the sensor name to keep it alive for collisions. */
  std::shared_ptr<std::string> _door_sensor_name;

  /** A reference to the sensor shape to keep it alive for instantiation. */
  b2PolygonShape _sensor_shape;

 public:
  /**
   * Creates an empty scene graph node with the degenerate texture.
   *
   * This constructor should never be called directly, as this is an abstract
   * class.
   */
  Door()
      : _door_sensor_name(nullptr),
        _obstacle(nullptr),
        _state(State::ON),
        BasicTile() {
    _classname = "Door";
  }

  /**
   * Deletes this node, releasing all resources.
   */
  ~Door() { dispose(); }

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
    _door_sensor_name = nullptr;
    _obstacle = nullptr;
    BasicTile::dispose();
  }

  virtual std::shared_ptr<SceneNode> copy(
      const std::shared_ptr<SceneNode>& dst) const override;

  /**
   * Performs a deep copy of this Node into dst.
   *
   * Children are inherited into the dst node.
   *
   * @param dst   The Node to copy into
   *
   * @return A reference to dst for chaining.
   */
  virtual std::shared_ptr<SceneNode> deepcopy() const override;

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

  static std::shared_ptr<Door> alloc() {
    std::shared_ptr<Door> result = std::make_shared<Door>();
    return (result->init() ? result : nullptr);
  }

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
    std::shared_ptr<Door> result = std::make_shared<Door>();
    if (!result->initWithData(loader, data)) {
      result = nullptr;
    }
    return std::dynamic_pointer_cast<SceneNode>(result);
  }

  /**
   * Initialize the delegates for multi-tile structure doors.
   */
  void initDelegates();

  /**
   * Initializes the box 2d object for the tile including setting the position
   * and size.
   *
   * @param sensor_name The name for the sensor for future reference.
   *
   * @return The obstacle it created for easy chaining.
   */
  virtual std::shared_ptr<cugl::physics2::PolygonObstacle> initBox2d(
      std::string& sensor_name);

  /**
   * @return Returns the physics object for the tile.
   */
  std::shared_ptr<cugl::physics2::PolygonObstacle> getObstacle() {
    return _obstacle;
  }

  /**
   * Set the door state and change the drawing and physics attributes.
   */
  void setState(State state);

  /**
   * Set that the door is positive. Positive for a horizontal door means that
   * it's at the top of the level. Positive for a vertical door means that it's
   * at the right of the level. This function is used for drawing the edges of
   * the room.
   */
  void setPositive();

  /**
   * Set that the door is negative. Negative for a horizontal door means that
   * it's at the bottom of the level. Negative for a vertical door means that
   * it's at the left of the level. This function is used for drawing the edges
   * of the room.
   */
  void setNegative();
};

#endif  // MODELS_TILES_DOOR_H_