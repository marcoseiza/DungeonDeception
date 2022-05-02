#ifndef CONTROLLERS_TRAIL_MANAGER_H_
#define CONTROLLERS_TRAIL_MANAGER_H_

#include <cugl/cugl.h>

class TrailManager {
 public:
  struct Config {
    int freq;
    int max_length;
    int min_opacity;
    int max_opacity;
    cugl::Color4 color;
  };

 private:
  std::shared_ptr<cugl::scene2::SpriteNode> _sprite;

  Config _config;

  int _buffer;
  bool _run;

  std::deque<std::shared_ptr<cugl::scene2::SpriteNode>> _trail_nodes_pool;

 public:
  TrailManager() {}
  ~TrailManager() { dispose(); }

  bool init(const std::shared_ptr<cugl::scene2::SpriteNode>& sprite,
            Config& config);

  bool init(const std::shared_ptr<cugl::scene2::SpriteNode>& sprite);

  void dispose();

  static std::shared_ptr<TrailManager> alloc(
      const std::shared_ptr<cugl::scene2::SpriteNode>& sprite, Config& config) {
    auto result = std::make_shared<TrailManager>();
    return (result->init(sprite, config)) ? result : nullptr;
  }

  static std::shared_ptr<TrailManager> alloc(
      const std::shared_ptr<cugl::scene2::SpriteNode>& sprite) {
    auto result = std::make_shared<TrailManager>();
    return (result->init(sprite)) ? result : nullptr;
  }

  void update();

  void start() { _run = true; }

  void stop() { _run = false; }
};

#endif  // CONTROLLERS_TRAIL_MANAGER_H_