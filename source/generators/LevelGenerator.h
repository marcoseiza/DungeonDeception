#ifndef GENERATORS_LEVEL_GENERATOR_H
#define GENERATORS_LEVEL_GENERATOR_H
#include <cugl/cugl.h>

#include "../models/level_gen/Room.h"
#include "LevelGeneratorConfig.h"

namespace level_gen {

/** A level generator that creates a random level with hallway connections. */
class LevelGenerator {
 private:
  /** A reference to the scene2 map for level drawing. */
  std::shared_ptr<cugl::scene2::SceneNode> _map;

  /** The level generator config with constants for generation. */
  LevelGeneratorConfig _config;

  /** A list of all rooms in the level. */
  std::vector<std::shared_ptr<Room>> _rooms;

  /** A list of all rooms in the inside circle of the level. */
  std::vector<std::shared_ptr<Room>> _inside_rooms;

  /** A list of all rooms in the middle ring of the level. */
  std::vector<std::shared_ptr<Room>> _middle_rooms;

  /** A list of all rooms in the outside ring of the level. */
  std::vector<std::shared_ptr<Room>> _outside_rooms;

  /** A reference to the spawn room of the level. */
  std::shared_ptr<Room> _spawn_room;

  /** If the level generator is currently active. Is set to true after init, and
   * reset on dispose. */
  bool _active;

  /** A function reference for the next step in the generator. Used to separate
   * steps for drawing. */
  std::function<void(void)> _generator_step;

  /**
   * A generator for random numbers. The seed for the generator is given by a
   * C++ random_device. If given a seed, levels will always generate the same.
   */
  std::default_random_engine _generator;

 public:
#pragma mark Initializers

  /**
   * Construct an empty Level Generator. Must call init() to start the
   * generator.
   */
  LevelGenerator();
  /** Disposes of the Level Generator. */
  ~LevelGenerator() { dispose(); }

#pragma mark Game Loop

  /**
   * Initialize the generator given a config and a scene2 map.
   * @param config A LevelGeneratorConfig used for defining generation
   * constants.
   * @param map A reference to the scene2 map for drawing.
   */
  void init(LevelGeneratorConfig &config,
            const std::shared_ptr<cugl::scene2::SceneNode> &map);

  /**
   * Initialize the generator given a config, scene2 map and seed
   * @param config A LevelGeneratorConfig used for defining generation
   * constants.
   * @param map A reference to the scene2 map for drawing.
   * @param seed The seed to use.
   */
  void init(LevelGeneratorConfig &config,
            const std::shared_ptr<cugl::scene2::SceneNode> &map, Uint64 seed);

  /**
   * Returns the pointer to the map SceneNode.
   *
   * @return The map SceneNode
   */
  std::shared_ptr<cugl::scene2::SceneNode> getMap() { return _map; }

  /**
   * Update the level generator. Calls the next generator step function.
   * Separates the generation steps to be able to draw the generation.
   *
   * @return False if the level generation is done.
   */
  bool update();

  /**
   * Dispose of the level generator. Clear all the references and variables in
   * the class.
   */
  void dispose();

#pragma mark Attributes
  /** Get all Rooms in the level generator. */
  std::vector<std::shared_ptr<Room>> getRooms() const { return _rooms; }

  /** Get the spawn room in the level generator. */
  std::shared_ptr<Room> getSpawnRoom() const { return _spawn_room; }

 private:
#pragma mark Main Generator Steps
  /**
   * Generate all the regular rooms in the level inside of the middle circle
   * radius, and separate them.
   *
   * Sets the _generator_step to separateRooms with a callback to spawn terminal
   * rooms.
   */
  void generateRooms();

  /**
   * Loop through all the rooms, find the distance between them and move them by
   * opposite the normalized distance. Sets the _generator_step to
   * next_generator_step when done.
   *
   * @param next_generator_step The function that _generator_step should be set
   * to when this method is done calculating.
   */
  void separateRooms(std::function<void(void)> next_generator_step = nullptr);

  /**
   * Mark the rooms with different types, and fill the hallways between them.
   */
  void markAndFillHallways();

  /**
   * Create the gates between the rooms and the hallways.
   */
  void establishGates();

#pragma mark Helpers

  void placeTerminals();

  void placeRegularRooms(int num_rooms, float min_radius, float max_radius);

  /**
   * Place terminal rooms in the level by replacing the default room that it
   * most overlaps. The terminal rooms are placed in between the min and max
   * radius given, and are separated evenly around the ring.
   *
   * @param num_rooms The number of terminal rooms to be spawned.
   * @param min_radius The minimum radius for spawning terminals in grid units.
   * @param max_radius The maximum radius for spawning terminals in grid units.
   * @return The list of terminal rooms placed.
   */
  std::vector<std::shared_ptr<Room>> placeTerminalRooms(int num_rooms,
                                                        float min_radius,
                                                        float max_radius);

  /**
   * Find if any of the rooms in the level are overlapping.
   *
   * @return If any of the rooms in the level are overlapping.
   */
  bool anyRoomsOverlapping();

  /**
   * Find the room that is most overlapping with the given room.
   *
   * @param room The room we are checking for overlap.
   * @return The room that is most overlapping with the given room.
   */
  std::shared_ptr<Room> roomMostOverlappingWith(
      const std::shared_ptr<Room> &room);

  /**
   * Separate the three rings by moving each layer farther from the center.
   */
  void segregateLayers();

  /**
   * Create delaunay triangles between all the rooms for edge generation that
   * does not intersect. It removes all edges that intersect the minimum radius
   * given. Uses a third party implementation from:
   * https://github.com/abellgithub/delaunator-cpp.
   *
   * @param rooms The rooms to create delaunay triangles with.
   * @param min_r The minimum radius of the ring layer.
   */
  void calculateDelaunayTriangles(std::vector<std::shared_ptr<Room>> &rooms,
                                  float min_r);

  /**
   * Create a minimum spanning tree between the rooms and their edges.
   *
   * @param rooms The rooms to create a minimum spanning tree with.
   */
  void calculateMinimumSpanningTree(std::vector<std::shared_ptr<Room>> &rooms);

  /**
   * Add some edges back within the room to make the tree a little easier to
   * traverse by the player. Remove the edges that aren't active and are not
   * added back.
   *
   * @param rooms The rooms to add edges back to.
   */
  void addEdgesBackAndRemoveUnecessary(
      std::vector<std::shared_ptr<Room>> &rooms);

  /**
   * Connect two layers with some number of connections. Spaces the connections
   * evenly between the layers and at the closest points between them.
   *
   * @param layer_a A vector of Rooms that makes up layer A.
   * @param layer_b A vector of Rooms that makes up layer B.
   * @param num_connections The number of connections between the layers.
   */
  void connectLayers(std::vector<std::shared_ptr<Room>> &layer_a,
                     std::vector<std::shared_ptr<Room>> &layer_b,
                     int num_connections);

  /**
   * Fill the hallways between the rooms using the generator config constants.
   */
  void fillHallways();
};

}  // namespace level_gen

#endif /* GENERATORS_LEVELGENERATOR_H */
