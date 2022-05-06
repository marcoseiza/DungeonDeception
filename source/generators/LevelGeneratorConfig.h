#ifndef GENERATORS_LEVEL_GENERATOR_CONFIG_H
#define GENERATORS_LEVEL_GENERATOR_CONFIG_H
#include <cugl/cugl.h>

namespace level_gen {
/**
 * A class that sets constants for level generation. Used by LevelGenerator.
 */
class LevelGeneratorConfig {
 public:
  /** A structure to represent a layer or "ring" of rooms. */
  struct Layer {
    /** The fraction of the map radius it takes. */
    float frac;
    /** The outer radius of the layer. */
    float radius;
    /** The number of terminals in this layer. */
    int num_terminals;
    /** The */
    int num_out_edges;
    /**
     * Create a new layer.
     * @param frac The fraction of the map radius.
     * @param radius The outer radius of the layer.
     * @param num_terminals The number of terminals.
     * @param num_out_edges The number of out edges from this layer to the next.
     */
    Layer(float frac, float radius, int num_terminals, int num_out_edges)
        : frac(frac),
          radius(radius),
          num_terminals(num_terminals),
          num_out_edges(num_out_edges) {}
  };

 private:
  /** The layers of rooms. */
  std::vector<Layer> _layers;

  /** The radius of the level. */
  float _map_radius;

  /** The radius to which rooms should initially spawn in. */
  float _spawn_radius;

  /** The separation offset between layers. Used to differentiate clearly
   * between room rings. */
  float _separation_between_layers;

  /** The expansion factor between rooms. */
  float _expansion_factor_rooms;

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

  /** The max number of edges to a room. (i.e. max number of doors per room). */
  int _max_num_of_edges;

  /** The size of one grid cell */
  cugl::Vec2 _grid_cell;

 public:
  /** Construct a new level generation config object with default values. */
  LevelGeneratorConfig();
  /** Destroy the level generation config object. */
  ~LevelGeneratorConfig() {}

  /**
   * Add a layer to the level. Layers must be placed from inside out, this
   * function finds the optimal spot to place the given layer.
   * @param layer The layer to add.
   */
  void addLayer(const Layer& layer) {
    auto compare = [=](Layer val, const Layer& cur) {
      return val.frac < cur.frac;
    };
    auto it = std::upper_bound(_layers.begin(), _layers.end(), layer, compare);
    _layers.insert(it, layer);
  }
  /** @return The layers of rooms in the game. */
  std::vector<Layer> getLayers() const { return _layers; }

  /**
   * Set the map radius in grid units.
   * @param radius The radius of the map in grid units.
   */
  void setMapRadius(int radius) { _map_radius = static_cast<float>(radius); }
  /** @return The map radius in grid units. */
  float getMapRadius() const { return _map_radius; }

  /**
   * Set the spawn radius in grid units.
   * @param radius The spawn radius in grid units.
   */
  void setSpawnRadius(int radius) {
    _spawn_radius = static_cast<float>(radius);
  }
  /** @return The spawn radius in grid units. */
  float getSpawnRadius() const { return _spawn_radius; }

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
   * Set expansion factor between rooms
   * @param factor The factor for multiplying positions for expansion.
   */
  void setExpansionFactorRooms(float factor) {
    _expansion_factor_rooms = factor;
  }
  /** @return The expansion factor between rooms. */
  float getExpansionFactorRooms() const { return _expansion_factor_rooms; }

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
   * Set the max number of edges for a room. (i.e. max number of doors).
   * @param num the max number of edges for a room.
   */
  void setMaxNumEdges(int num) { _max_num_of_edges = num; }
  /** @return The max number of edges for a room. (i.e. max number of doors). */
  int getMaxNumEdges() const { return _max_num_of_edges; }

  /**
   * Set the grid cell size.
   * @param num the grid cell size.
   */
  void setGridCell(const cugl::Vec2& size) { _grid_cell = size; }
  /** @return The size of one grid cell. */
  cugl::Vec2 getGridCell() const { return _grid_cell; }
};

}  // namespace level_gen

#endif /* GENERATORS_LEVEL_GENERATOR_CONFIG_H */
