#ifndef MODELS_TILES_TILE_HELPER_H_
#define MODELS_TILES_TILE_HELPER_H_

#include <cugl/cugl.h>

#include "Wall.h"

class TileHelper {
 public:
  /**
   * As scene node and box2d are separate entities, making the scene2 invisble
   * doesn't turn off physics. This is especially annoying when considering
   * children of scene2s. This method will make this node and any child node
   * a sensor.
   *
   * @param node The node to make as sensor.
   * @param val Sensor value.
   */
  static void setSensorCascade(
      const std::shared_ptr<cugl::scene2::SceneNode>& node, bool val) {
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

  /**
   * Get all the children of type T from the given node.
   *
   * @tparam T, the tile class to get (eg. Attack)
   * @param node The node to make as sensor.
   * @param val Sensor value.
   */
  template <typename T>
  static std::vector<std::shared_ptr<T>> getTile(
      const std::shared_ptr<cugl::scene2::SceneNode>& node) {
    std::vector<std::shared_ptr<T>> result;
    for (std::shared_ptr<cugl::scene2::SceneNode> child : node->getChildren()) {
      auto child_T = std::dynamic_pointer_cast<T>(child);
      if (child_T != nullptr) {
        result.push_back(child_T);
      } else {
        std::vector<std::shared_ptr<T>> child_result = getTile<T>(child);
        result.insert(result.end(), child_result.begin(), child_result.end());
      }
    }
    return result;
  }
};

#endif  // MODELS_TILES_TILE_HELPER_H_