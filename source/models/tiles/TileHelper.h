#ifndef MODELS_TILES_TILE_HELPER_H_
#define MODELS_TILES_TILE_HELPER_H_

#include <cugl/cugl.h>

#include "Wall.h"

namespace tile_helper {

/**
 * As scene node and box2d are separate entities, making the scene2 invisble
 * doesn't turn off physics. This is especially annoying when considering
 * children of scene2s. This method will make this node and any child node
 * a sensor.
 *
 * @param node The node to make as sensor.
 * @param val Sensor value.
 */
void setSensorCascade(const std::shared_ptr<cugl::scene2::SceneNode>& node,
                      bool val) {
  auto wall = std::dynamic_pointer_cast<Wall>(node);
  if (wall) {
    auto obs = wall->getObstacle();
    if (obs) {
      obs->setSensor(val);
    } else {
      wall->setInitializeAsSensor(val);
    }
  }

  for (const auto& child : node->getChildren()) {
    setSensorCascade(child, val);
  }
}

};  // namespace tile_helper

#endif  // MODELS_TILES_TILE_HELPER_H_