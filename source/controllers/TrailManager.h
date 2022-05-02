#ifndef CONTROLLERS_TRAIL_MANAGER_H_
#define CONTROLLERS_TRAIL_MANAGER_H_

#include <cugl/cugl.h>

/**
 * This is a general use class to create a trail from a sprite node.
 */
class TrailManager {
 public:
  /** The trail config. */
  struct Config {
    /** How many ticks between trail node draws. */
    int freq;
    /** The max amount of nodes in the trail. */
    int max_length;
    /** The minimum opacity for the trail nodes. */
    int min_opacity;
    /** The max opacity for the trail nodes. */
    int max_opacity;
    /** The color of the nodes. */
    cugl::Color4 color;
  };

 private:
  /** A reference to the sprite that this manager is creating a trail for. */
  std::shared_ptr<cugl::scene2::SpriteNode> _sprite;

  /** The config for the trail. */
  Config _config;

  /** A tick buffer for calculating the frequency of node draws. */
  int _buffer;

  /** If the manager should be creating trails. */
  bool _run;

  /** The pool of trail nodes. */
  std::deque<std::shared_ptr<cugl::scene2::SpriteNode>> _trail_nodes_pool;

 public:
  TrailManager() {}
  ~TrailManager() { dispose(); }

  /**
   * Initialize a trail manager to create a trail for the given sprite.
   *
   * @param sprite The sprite to create a trail from.
   * @param config The trail manager config.
   * @return If successfully initialized
   */
  bool init(const std::shared_ptr<cugl::scene2::SpriteNode>& sprite,
            Config& config);

  /**
   * Initialize a trail manager to create a trail for the given sprite.
   *
   * @param sprite The sprite to create a trail from.
   * @return If successfully initialized.
   */
  bool init(const std::shared_ptr<cugl::scene2::SpriteNode>& sprite);

  /** Dispose of the manager and all its assets. */
  void dispose();

  /**
   * Allocate and initialize a new Trail Manager shared pointer.
   *
   * @param sprite The sprite to create a trail from.
   * @param config The trail manager config.
   * @return A shared pointer of the allocated Trail Manager.
   */
  static std::shared_ptr<TrailManager> alloc(
      const std::shared_ptr<cugl::scene2::SpriteNode>& sprite, Config& config) {
    auto result = std::make_shared<TrailManager>();
    return (result->init(sprite, config)) ? result : nullptr;
  }

  /**
   * Allocate and initialize a new Trail Manager shared pointer.
   *
   * @param sprite The sprite to create a trail from.
   * @return A shared pointer of the allocated Trail Manager.
   */
  static std::shared_ptr<TrailManager> alloc(
      const std::shared_ptr<cugl::scene2::SpriteNode>& sprite) {
    auto result = std::make_shared<TrailManager>();
    return (result->init(sprite)) ? result : nullptr;
  }

  /**
   * Update the trail manager to spawn, modify and delete trail nodes.
   */
  void update();

  /**
   * If the trail manager should run.
   * @param val If the trail manager should run.
   */
  void run(bool val) { _run = val; }
};

#endif  // CONTROLLERS_TRAIL_MANAGER_H_