#ifndef GENERATORS_LEVEL_GENERATOR_CONFIG_H
#define GENERATORS_LEVEL_GENERATOR_CONFIG_H
#include <cugl/cugl.h>

namespace level_gen {
/**
 * A class that sets constants for level generation. Used by LevelGenerator.
 */
class LevelGeneratorConfig {
 public:
  // TODO: make getters and setters.
  std::vector<float> _circle_fractions;
  std::vector<float> _circle_radius;
  int _num_circles;
  std::vector<int> _circle_num_terminals;
  std::vector<int> _circle_num_out_edges;

  float _spawn_radius;

 private:
  /** The radius of the level. */
  float _map_radius;

  /** The separation offset between layers. Used to differentiate clearly
   * between room rings. */
  float _separation_between_layers;

  /** The radius of the inner circle of rooms. */
  float _inner_circle_radius;
  /** A fraction of the map radius that represents the inner circle of rooms. */
  float _inner_circle_frac;

  /** The radius of the middle circle of rooms. */
  float _middle_circle_radius;
  /** A fraction of the map radius that represents the middle circle
   * of rooms. */
  float _middle_circle_frac;

  /** A max hallway length in grid units. for when we're defining what edges are
   * acceptable. */
  float _max_hallway_length;

  /** A radius for the hallways in grid units. */
  float _hallway_radius;

  /** A probability used to define how likely edges should return after the
   * minimum spanning tree algorithm of level generation. */
  float _add_edges_back_prob;

  /** The total number of room in the level, including terminals. */
  float _num_rooms;

  /** The number of terminal rooms in the level. */
  float _num_terminal_rooms;

  /** The number of terminal rooms in the inner circle of rooms. */
  float _num_terminal_rooms_inner;

  /** The number of terminal rooms in the middle circle of rooms. */
  float _num_terminal_rooms_middle;

  /** The number of terminal rooms in the outer circle of rooms. */
  float _num_terminal_rooms_outer;

  /** The max number of edges to a room. (i.e. max number of doors per room). */
  int _max_num_of_edges;

 public:
  /** Construct a new level generation config object with default values. */
  LevelGeneratorConfig();
  /** Destroy the level generation config object. */
  ~LevelGeneratorConfig() {}

  /**
   * Set the map radius in grid units.
   * @param radius The radius of the map in grid units.
   */
  void setMapRadius(int radius) {
    _map_radius = static_cast<float>(radius);
    _inner_circle_radius = floor(_map_radius * _inner_circle_frac);
    _middle_circle_radius = floor(_map_radius * _middle_circle_frac);
  }
  /** @return The map radius in grid units. */
  float getMapRadius() const { return _map_radius; }

  /**
   * Set separation between layers in grid units
   * @param separation The separation between layers in grid units.
   */
  void setSeparationBetweenLayers(int separation) {
    _separation_between_layers = static_cast<float>(separation);
  }
  /** @return The separation between layers in grid units. */
  float getSeparationBetweenLayers() const {
    return _separation_between_layers;
  }

  /**
   * Set what fraction of the map radius the inner circle takes up.
   * @param frac The frac of the map radius that represents the radius.
   */
  void setInnerCircleFrac(float frac) {
    _inner_circle_frac = frac;
    _inner_circle_radius = floor(_map_radius * frac);
  }
  /** @return The inner circle radius in grid units. */
  float getInnerCircleRadius() const { return _inner_circle_radius; }
  /** @return The inner circle frac in relation to the map radius. */
  float getInnerCircleFrac() const { return _inner_circle_frac; }

  /**
   * Set what fraction of the map radius the middle circle takes up.
   * @param frac The frac of the map radius that represents the radius.
   */
  void setMiddleCircleFrac(float frac) {
    _middle_circle_frac = frac;
    _middle_circle_radius = _map_radius * frac;
  }
  /** @return The middle circle radius in grid units. */
  float getMiddleCircleRadius() const { return _middle_circle_radius; }
  /** @return The middle circle frac in relation to the map radius. */
  float getMiddleCircleFrac() const { return _middle_circle_frac; }

  /**
   * Set max hallway length in grid units
   * @param length The max hallway length in grid units.
   */
  void setMaxHallwayLength(int length) {
    _max_hallway_length = static_cast<float>(length);
  }
  /** @return The maximum hallway length in grid units. */
  float getMaxHallwayLength() const { return _max_hallway_length; }

  /**
   * Set hallway radius (width / 2.0f) in grid units
   * @param radius The hallway radius in grid units.
   */
  void setHallwayRadius(int radius) {
    _hallway_radius = static_cast<float>(radius);
  }
  /** @return The hallway radius (width / 2.0f) in grid units. */
  float getHallwayRadius() const { return _hallway_radius; }

  /**
   * Set the probability of giving edges back after minimum spanning tree step.
   * @param radius The probability to give edges back.
   */
  void setAddEdgesBackProb(float prob) { _add_edges_back_prob = prob; }
  /** @return The probability to give edges back after minimum spanning tree
   * step. */
  float getAddEdgesBackProb() const { return _add_edges_back_prob; }

  /**
   * Set the number of rooms in the level, including terminal rooms.
   * @param num The number of rooms.
   */
  void setNumRooms(int num) { _num_rooms = static_cast<float>(num); }
  /** @return The number of rooms in the level. */
  float getNumRooms() const { return _num_rooms; }

  /**
   * Set the number of terminal rooms in the level. By default, it will split
   * the number of rooms into three. It will give a floored number of terminals
   * / 3.0f to the inner and middle circles and a ceilinged number of terminals
   * to the outer circle. (e.g. 7 terminals -> 2 inside, 2 middle, 3 outside).
   *
   * @param num The number of terminal rooms.
   */
  void setNumTerminalRooms(int num) {
    float num_f = static_cast<float>(num);
    _num_terminal_rooms = num_f;

    // Minimum of dividing by 3. (eg. 7 -> 2)
    _num_terminal_rooms_inner = floor(num_f / 3);
    _num_terminal_rooms_middle = floor(num_f / 3);

    // Majority of dividing by 3. (eg. 7 -> 3)
    _num_terminal_rooms_outer = ceil(num_f / 3);
  }

  /** @return The number of terminal rooms in the level. */
  float getNumTerminalRooms() const { return _num_terminal_rooms; }

  /** @return The number of rooms in the inner circle. */
  float getNumTerminalRoomsInner() const { return _num_terminal_rooms_inner; }

  /** @return The number of rooms in the middle circle. */
  float getNumTerminalRoomsMiddle() const { return _num_terminal_rooms_middle; }

  /** @return The number of rooms in the outer circle. */
  float getNumTerminalRoomsOuter() const { return _num_terminal_rooms_outer; }

  /**
   * Set the max number of edges for a room. (i.e. max number of doors).
   * @param num the max number of edges for a room.
   */
  void setMaxNumEdges(int num) { _max_num_of_edges = num; }
  /** @return The max number of edges for a room. (i.e. max number of doors). */
  int getMaxNumEdges() const { return _max_num_of_edges; }
};

}  // namespace level_gen

#endif /* GENERATORS_LEVEL_GENERATOR_CONFIG_H */
