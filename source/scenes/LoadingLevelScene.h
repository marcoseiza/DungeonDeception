#ifndef SCENES_LEVEL_GENERATION_DEMO_SCENE_H
#define SCENES_LEVEL_GENERATION_DEMO_SCENE_H
#include <cugl/cugl.h>

#include "../generators/LevelGenerator.h"
#include "../generators/LevelGeneratorConfig.h"
#include "../models/level_gen/Room.h"

class LoadingLevelScene : public cugl::Scene2 {
 private:
  /** A level generator for this scene. */
  std::shared_ptr<level_gen::LevelGenerator> _level_generator;

  /** A level generator config for this scene. */
  level_gen::LevelGeneratorConfig _config;

  /** A reference to the scene2 map for rendering. */
  std::shared_ptr<cugl::scene2::SceneNode> _map;

  /** A reference to the assets for the game. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** The network connection (as made by this scene). */
  std::shared_ptr<cugl::NetworkConnection> _network;

  /** Whether this player is the host. */
  bool _ishost;

  enum {
    /** Generate Rooms using the Level Generator. */
    GENERATE_ROOMS,
    /** Load in all the used room scene2 graphs. */
    LOAD_ROOM_SCENE2,
    /** Level loader is done. */
    DONE
  } _loading_phase;

 public:
  /** Initializes the level generation scene2. */
  LoadingLevelScene() : cugl::Scene2(), _loading_phase(GENERATE_ROOMS) {}

  /** Disposes of all resources allocated to this mode. */
  ~LoadingLevelScene() { dispose(); }

  /** Get the level generator, to pass on to the game scene. */
  std::shared_ptr<level_gen::LevelGenerator> getLevelGenerator() {
    return _level_generator;
  }

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  void dispose() override;

  /**
   * Initializes the controller contents, and starts generating the level.
   *
   * @param assets    The (loaded) assets for this loading mode
   * @return true if the controller is initialized properly, false otherwise.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Initializes the controller contents, and starts generating the level.
   *
   * @param assets    The (loaded) assets for this loading mode
   * @param seed        The seed to be used in the map generation
   * @return true if the controller is initialized properly, false otherwise.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets, Uint64 seed);

  /**
   * The method called to update the level loading mode.
   *
   * @param timestep  The amount of time (in seconds) since the last frame
   */
  void update(float timestep) override;

  /**
   * Draws all this scene to the given SpriteBatch.
   *
   * @param batch     The SpriteBatch to draw with.
   */
  void render(const std::shared_ptr<cugl::SpriteBatch>& batch) override;

  /**
   * Returns the network connection (as made by this scene).
   *
   * This value will be reset every time the scene is made active.
   *
   * @return the network connection (as made by this scene)
   */
  void setConnection(const std::shared_ptr<cugl::NetworkConnection>& network) {
    _network = network;
  }

  /**
   * Sets whether the player is host.
   *
   * We may need to have gameplay specific code for host.
   *
   * @param host  Whether the player is host.
   */
  void setHost(bool host) { _ishost = host; }

  /**
   * Returns whether the player is the host.
   *
   * @return Whether the player is host.
   */
  bool getIsHost() { return _ishost; }

  /**
   * Checks that the network connection is still active.
   *
   * Even if you are not sending messages all that often, you need to be calling
   * this method regularly. This method is used to determine the current state
   * of the scene.
   *
   * @return true if the network connection is still active.
   */
  bool checkConnection();

  /**
   * Returns the network connection (as made by this scene)
   *
   * @return the network connection (as made by this scene)
   */

  std::shared_ptr<cugl::NetworkConnection> getConnection() const {
    return _network;
  }

  /**
   * Processes data sent over the network.
   *
   * Once connection is established, all data sent over the network consistes of
   * byte vectors. This function is a call back function to process that data.
   * Note that this function may be called *multiple times* per animation frame,
   * as the messages can come from several sources.
   *
   * @param data  The data received
   */
  void processData(const std::vector<uint8_t>& data);

  /**
   * Disconnects this scene from the network controller.
   *
   * Technically, this method does not actually disconnect the network
   * controller. Since the network controller is a smart pointer, it is only
   * fully disconnected when ALL scenes have been disconnected.
   */
  void disconnect() { _network = nullptr; }

  /**
   * Get the map scene2 node.
   */
  std::shared_ptr<cugl::scene2::SceneNode> getMap() { return _map; }
};

#endif /* SCENES_LEVEL_GENERATION_DEMO_SCENE_H */
