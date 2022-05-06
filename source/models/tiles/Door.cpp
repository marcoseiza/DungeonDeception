#include "Door.h"

#include "TileHelper.h"
#include "Wall.h"

#define TILE_WIDTH 48

bool Door::initWithData(const cugl::Scene2Loader* loader,
                        const std::shared_ptr<cugl::JsonValue>& data) {
  if (_texture != nullptr) {
    CUAssertLog(false, "%s is already initialized", _classname.c_str());
    return false;
  } else if (!data) {
    return SceneNode::init();
  } else if (!SceneNode::initWithData(loader, data)) {
    return false;
  }

  if (data->has("obstacle")) {
    _obstacle_shape.set(data->get("obstacle"));
  } else {
    cugl::Rect bounds = cugl::Rect::ZERO;
    if (_texture != nullptr) {
      bounds.size = _texture->getSize();
    } else {
      bounds.size = getContentSize();
    }
    _obstacle_shape.set(bounds);
  }

  if (data->has("offset-priority")) {
    _priority = _priority - data->getFloat("offset-priority") / TILE_WIDTH;
  } else {
    _priority = _priority - _obstacle_shape.getBounds().origin.y / TILE_WIDTH;
  }

  std::shared_ptr<cugl::JsonValue> delegates = data->get("delegates");
  if (delegates != nullptr && delegates->isArray()) {
    for (int i = 0; i < delegates->size(); i++) {
      std::shared_ptr<cugl::JsonValue> del_data = delegates->get(i);
      Delegate del;

      if (del_data->has("on-state")) {
        del.on_node_key = del_data->get("on-state")->asStringArray();
      }
      if (del_data->has("off-state")) {
        del.off_node_key = del_data->get("off-state")->asStringArray();
      }
      if (del_data->has("unused")) {
        del.unused_node_key = del_data->get("unused")->asStringArray();
      }

      _delegates.push_back(del);
    }
  }

  return true;
}

void Door::initDelegates() {
  for (Delegate& del : _delegates) {
    for (int i = 0; i < del.on_node_key.size(); i++) {
      if (i == 0) {
        del.on_node = getChildByName(del.on_node_key[i]);
      } else {
        del.on_node = del.on_node->getChildByName(del.on_node_key[i]);
      }
    }

    for (int i = 0; i < del.off_node_key.size(); i++) {
      if (i == 0) {
        del.off_node = getChildByName(del.off_node_key[i]);
      } else {
        del.off_node = del.off_node->getChildByName(del.off_node_key[i]);
      }
    }
    del.off_node->setVisible(false);

    for (int i = 0; i < del.unused_node_key.size(); i++) {
      if (i == 0) {
        del.unused_node = getChildByName(del.unused_node_key[i]);
      } else {
        del.unused_node =
            del.unused_node->getChildByName(del.unused_node_key[i]);
      }
    }
    del.unused_node->setVisible(false);
  }
}

std::shared_ptr<cugl::physics2::PolygonObstacle> Door::initBox2d(
    std::string& sensor_name) {
  _obstacle = cugl::physics2::PolygonObstacle::alloc(_obstacle_shape);

  cugl::Vec2 pos = BasicTile::getWorldPosition() - BasicTile::getPosition();

  if (_obstacle != nullptr) {
    _obstacle->setPosition(pos);
    _obstacle->setName(_classname.c_str());
    _obstacle->setSensor(true);

    _obstacle->setBodyType(b2BodyType::b2_staticBody);

    auto sensor = std::make_shared<b2FixtureDef>();
    sensor->density = 0.0f;
    sensor->isSensor = true;
    _door_sensor_name = std::make_shared<std::string>(sensor_name);
    sensor->userData.pointer =
        reinterpret_cast<uintptr_t>(_door_sensor_name.get());

    b2Vec2 sensor_pos(_obstacle_shape.getBounds().size.width / 2,
                      _obstacle_shape.getBounds().size.height / 2);
    // Sensor dimensions
    _sensor_shape.SetAsBox(_obstacle_shape.getBounds().size.width / 2,
                           _obstacle_shape.getBounds().size.height / 2,
                           sensor_pos, 0);

    sensor->shape = &_sensor_shape;

    std::vector<std::shared_ptr<b2FixtureDef>> sensors{sensor};
    _obstacle->addSensors(sensors);
  }

  return _obstacle;
}

void Door::setState(State state) {
  _state = state;

  for (Delegate& del : _delegates) {
    if (del.on_node && del.off_node && del.unused_node) {
      switch (_state) {
        case ON:
          del.on_node->setVisible(true);
          del.off_node->setVisible(false);
          del.unused_node->setVisible(false);

          TileHelper::setSensorCascade(del.on_node, false);
          TileHelper::setSensorCascade(del.off_node, true);
          TileHelper::setSensorCascade(del.unused_node, true);
          break;
        case OFF:
          del.on_node->setVisible(false);
          del.off_node->setVisible(true);
          del.unused_node->setVisible(false);

          TileHelper::setSensorCascade(del.on_node, true);
          TileHelper::setSensorCascade(del.off_node, false);
          TileHelper::setSensorCascade(del.unused_node, true);
          break;
        case UNUSED:
          del.on_node->setVisible(false);
          del.off_node->setVisible(false);
          del.unused_node->setVisible(true);

          TileHelper::setSensorCascade(del.on_node, true);
          TileHelper::setSensorCascade(del.off_node, true);
          TileHelper::setSensorCascade(del.unused_node, false);
          break;
      }
    }
  }
}

void Door::setPositive() {
  auto negative_node = getChildByName<cugl::scene2::SceneNode>("negative-edge");

  for (const auto& tile_wrapper : negative_node->getChildren()) {
    auto tile =
        std::dynamic_pointer_cast<Wall>(tile_wrapper->getChildByName("tile"));
    tile->setInitializeAsSensor(true);
    tile->setVisible(false);
  }

  auto positive_node = getChildByName<cugl::scene2::SceneNode>("positive-edge");

  for (const auto& tile_wrapper : positive_node->getChildren()) {
    auto tile =
        std::dynamic_pointer_cast<Wall>(tile_wrapper->getChildByName("tile"));
    tile->setInitializeAsSensor(false);
    tile->setVisible(true);
  }
}

void Door::setNegative() {
  auto positive_node = getChildByName<cugl::scene2::SceneNode>("positive-edge");

  for (const auto& tile_wrapper : positive_node->getChildren()) {
    auto tile =
        std::dynamic_pointer_cast<Wall>(tile_wrapper->getChildByName("tile"));
    tile->setInitializeAsSensor(true);
    tile->setVisible(false);
  }

  auto negative_node = getChildByName<cugl::scene2::SceneNode>("negative-edge");

  for (const auto& tile_wrapper : negative_node->getChildren()) {
    auto tile =
        std::dynamic_pointer_cast<Wall>(tile_wrapper->getChildByName("tile"));
    tile->setInitializeAsSensor(false);
    tile->setVisible(true);
  }
}

std::shared_ptr<cugl::scene2::SceneNode> Door::copy(
    const std::shared_ptr<cugl::scene2::SceneNode>& dst) const {
  BasicTile::copy(dst);

  std::shared_ptr<Door> node = std::dynamic_pointer_cast<Door>(dst);
  node->_state = _state;
  node->_delegates = _delegates;
  node->_obstacle_shape = _obstacle_shape;

  return dst;
}

std::shared_ptr<cugl::scene2::SceneNode> Door::deepcopy() const {
  auto dst = copy(Door::alloc());
  dst->setLayout(_layout);
  for (std::shared_ptr<cugl::scene2::SceneNode> child : _children) {
    dst->addChild(child->deepcopy());
  }
  dst->doLayout();
  return dst;
}
