#include "RoomModel.h"

bool RoomModel::init(const std::shared_ptr<cugl::scene2::SceneNode>& node,
                     int key) {
  _node = node;
  auto tiles = node->getChildByName("tiles");
  auto grid_layout =
      std::dynamic_pointer_cast<cugl::scene2::GridLayout>(tiles->getLayout());
  _grid_size = grid_layout->getGridSize();
  _key = key;
  return true;
}

void RoomModel::dispose() { _node = nullptr; }
