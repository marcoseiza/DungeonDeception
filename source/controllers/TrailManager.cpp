#include "TrailManager.h"

bool TrailManager::init(const std::shared_ptr<cugl::scene2::SpriteNode>& sprite,
                        Config& config) {
  _sprite = sprite;
  _config = config;
  _buffer = 0;
  _run = true;
  _trail_nodes_pool.clear();

  return true;
}

bool TrailManager::init(
    const std::shared_ptr<cugl::scene2::SpriteNode>& sprite) {
  _sprite = sprite;
  _buffer = 0;
  _run = true;
  _trail_nodes_pool.clear();

  _config.freq = 4;
  _config.max_length = 5;
  _config.max_opacity = 255;
  _config.min_opacity = 200;
  _config.color = cugl::Color4::WHITE;

  return true;
}

void TrailManager::dispose() {
  _buffer = 0;
  _run = false;
  _trail_nodes_pool.clear();
}

void TrailManager::update() {
  std::size_t num_nodes = _trail_nodes_pool.size();
  for (int i = 0; i < num_nodes; i++) {
    float t = (i / num_nodes);
    int opacity =
        (_config.max_opacity - _config.min_opacity) * t + _config.min_opacity;

    cugl::Color4 color = _config.color;
    color.a = opacity;

    _trail_nodes_pool[i]->setColor(color);
  }

  if (_run) {
    _buffer++;
    if (_buffer == _config.freq) {
      _buffer = 0;

      if (num_nodes == _config.max_length) {
        auto node = _trail_nodes_pool.front();
        _trail_nodes_pool.pop_front();
        node->getParent()->removeChild(node);
      }

      auto trail_node =
          cugl::scene2::SpriteNode::alloc(_sprite->getTexture(), 0, 0);
      _sprite->copy(trail_node);
      _trail_nodes_pool.push_back(trail_node);
      _sprite->getParent()->addChild(trail_node);
    }
  } else if (num_nodes > 0) {
    auto node = _trail_nodes_pool.front();
    node->getParent()->removeChild(node);
    _trail_nodes_pool.pop_front();
  }
}
