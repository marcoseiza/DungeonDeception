#ifndef MODELS_LEVEL_GEN_ROOM_H_
#define MODELS_LEVEL_GEN_ROOM_H_
#include <cugl/cugl.h>

#include "DefaultRooms.h"
#include "RoomTypes.h"

namespace default_rooms {
struct RoomConfig;
}

namespace level_gen {

// Forward declaration for use in Room.
class Edge;

class Room {
 public:
  /** This is the room type of the room. */
  RoomType _type;

  /** If the room is a terminal, it should know how many players are required
   * for the activation. */
  int _num_players_for_terminal;

  /** A reference to the scene2 node for the room. */
  std::shared_ptr<cugl::scene2::PolygonNode> _node;

  /** A reference to the scene2 node for the level. */
  std::shared_ptr<cugl::scene2::SceneNode> _level_node;

  /** A list of all edges to other Rooms, sorted in counter clockwise order. */
  std::vector<std::shared_ptr<Edge>> _edges;

  /** A list of all the doors in the Room given by the grid unit coordinates in
   * relation to the bottom left of the room. */
  std::vector<cugl::Vec2> _doors;

  /** Used to give a door to an edge in the Level Generator algorithm. */
  std::unordered_map<std::shared_ptr<Edge>, cugl::Vec2> _edge_to_door;

  /** If the room has been visited by the level generation algorithm. */
  bool _visited;

  /** If this room should not move. */
  bool _fixed;

  /** The key to the scene2 room layout to refer to it by. */
  std::string _scene2_key;
  /** The source to the scene2 room layout to load it in. */
  std::string _scene2_source;
  /** The key for the room object. */
  int _key;

  /**
   * Create a Room with the given config. The doors are
   * represented by their grid unit coordinates in terlation to the bottom left
   * of the room. The doors must be in counter-clockwise order with right-most
   * first. Everything is in grid units.
   *
   * @param config The config containing size, door positions, and scene2 a key.
   */
  Room(default_rooms::RoomConfig config);

  /**
   * Move the room by given certain distance.
   *
   * @param dist The distance to move the room in grid units.
   */
  void move(cugl::Vec2 dist);

  /**
   * Add an edge in counter clockwise order to the list of edges.
   *
   * @param edge The edge to be added.
   */
  void addEdge(const std::shared_ptr<Edge> &edge);

  /**
   * Returns the SceneNode color for the room type.
   *
   * @return the color corresponding to the room type.
   */
  cugl::Color4 getRoomNodeColor() {
    switch (_type) {
      case RoomType::TERMINAL:
        return cugl::Color4(52, 205, 14, 127);
      case RoomType::SPAWN:
        return cugl::Color4(14, 14, 205, 127);
      case RoomType::STANDARD:
        return cugl::Color4(125, 94, 52, 127);
    }
  }

  /**
   * An edge comparator used to sort edges first by side index they hit and then
   * in counter-clockwise order.
   *
   * @param l Left hand side edge.
   * @param r Right hand side edge.
   * @return Left is smaller than Right.
   */
  bool edgeComparator(const std::shared_ptr<Edge> &l,
                      const std::shared_ptr<Edge> &r) const;

  /**
   * Find if the given room is a neighbor of this room.
   *
   * @param room The room we would like to check for neighborness.
   * @return If the given room is a neighbor of this room.
   */
  bool hasNeighbor(const std::shared_ptr<Room> &room) const;

  /**
   * Find and return the edge that ties this room and the given room.
   *
   * @param room The room that is connected to this room.
   * @return The edge that connects param room and this room. Returns nullptr if
   * no edges found.
   */
  std::shared_ptr<Edge> findEdge(const std::shared_ptr<Room> &room) const;

  /**
   * Get the door that corresponds to this edge.
   *
   * @param edge The edge that is connected to this room.
   * @return The door that it map to.
   */
  cugl::Vec2 getDoorForEdge(const std::shared_ptr<Edge> &edge);

  /**
   * Get a rectangle representation of the room.
   * @return cugl::Rect with room's position and size.
   */
  cugl::Rect getRect() const {
    return cugl::Rect(_node->getPosition(), _node->getSize());
  }

  /**
   * Get the midpoint of the room.
   * @return cugl::Vec2 of the midpoint of the room.
   */
  cugl::Vec2 getMid() const {
    return _node->getPosition() + _node->getSize() / 2.0f;
  }

  /**
   * Get the diagonal radius of the room.
   * @return The diagonal radius of the room.
   */
  float getRadius() const {
    return cugl::Vec2(_node->getSize()).length() / 2.0f;
  }

 private:
  /**
   * Initialize the scene2 nodes for this room given a certain size in grid
   * units.
   *
   * @param size The size of the room in grid units.
   */
  void initScene2(cugl::Size size);

  /** Calculate the edge to door pairing. */
  void initializeEdgeToDoorPairing();

  float angleBetweenEdgeAndDoor(const std::shared_ptr<Edge> &edge,
                                cugl::Vec2 &door);
};

/**
 * This class represents a connection between two rooms.
 */
class Edge {
 public:
  /** A reference to the source Room. */
  std::shared_ptr<Room> _source;

  /** A reference to the neighbor Room. */
  std::shared_ptr<Room> _neighbor;

  /** A reference to the scene2 node for debug drawing. */
  std::shared_ptr<cugl::scene2::PathNode> _node;

  /** A pair of vectors that represent the line segment connecting both
   * rooms. Will always have a size of 2. */
  std::vector<cugl::Vec2> _path;

  /** The length of the edge. */
  float _weight;

  /** Used by the level generator to define if the edge has already been
   * calculated during level generation. */
  bool _calculated;

  bool _active;

  /**
   * Construct an Edge that connects Rooms s and n.
   * @param s Room for the source of the edge.
   * @param n Room for the neighbor of the edge.
   */
  Edge(const std::shared_ptr<Room> &s, const std::shared_ptr<Room> &n);

  /**
   * Check if the edge intersects a circle with given origin and radius.
   * @param origin The origin of the circle in grid units.
   * @param r The radius of the circle in grid units.
   * @return If the edge intersects the given circle.
   */
  bool doesIntersect(cugl::Vec2 origin, float r);

  /**
   * Get the other Room given one of the rooms in the edge.
   * @param room The room that is in the edge.
   * @return A reference to the other room or nullptr if the given room is not
   * on the edge.
   */
  std::shared_ptr<Room> getOther(const std::shared_ptr<Room> &room) {
    if (room == _source) {
      return _neighbor;
    } else if (room == _neighbor) {
      return _source;
    }
    return nullptr;
  }

  /**
   * Get the index of the side the edge line segment hits.
   * 0 = Right,
   * 1 = Top,
   * 2 = Left,
   * 3 = Bottom,
   * -1 = No intersect
   * @param rect A rectangle that we are checking for collision.
   * @return The index of the side the edge line segment hits.
   */
  int getSideIndex(const cugl::Rect &rect) const;

  /**
   * Get the intersect of the edge and given rectangle sides.
   *
   * @param rect A rectangle that we are checking for collision.
   * @return The intersect of the edge and given rectangle sides.
   */
  cugl::Vec2 getIntersectWithRectSide(const cugl::Rect &rect) const;

  /**
   * An equality operator for Edge that checks if the other Edge has the same
   * rooms in any order of source and neighbor.
   * @param other The other edge to check for equality.
   * @return If the other Edge is equal to this Edge.
   * */
  bool operator==(const Edge &other) const {
    return (this->_source == other._source &&
            this->_neighbor == other._neighbor) ||
           (this->_source == other._neighbor &&
            this->_neighbor == other._source);
  }

  /**
   * Check if the other Edge shares a Room with this Edge.
   * @param other The other edge to check for sharedness.
   * @return If the other Edge shares a room with this Edge.
   */
  bool shareRoom(const std::shared_ptr<Edge> &other) const {
    return this->_source == other->_source ||
           this->_neighbor == other->_source ||
           this->_neighbor == other->_neighbor ||
           this->_source == other->_neighbor;
  }
};

}  // namespace level_gen

#endif  // MODELS_LEVEL_GEN_ROOM_H_
