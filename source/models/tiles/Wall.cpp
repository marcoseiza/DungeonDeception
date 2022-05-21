#include "Wall.h"

#include "../../controllers/CollisionFiltering.h"

#define TILE_WIDTH 48

bool Wall::initWithData(const cugl::Scene2Loader* loader,
                        const std::shared_ptr<cugl::JsonValue>& data) {
  if (_texture != nullptr) {
    CUAssertLog(false, "%s is already initialized", _classname.c_str());
    return false;
  } else if (!data) {
    return BasicTile::init();
  } else if (!BasicTile::initWithData(loader, data)) {
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

  return true;
}

std::shared_ptr<cugl::physics2::PolygonObstacle> Wall::initBox2d() {
  _obstacle = cugl::physics2::PolygonObstacle::alloc(_obstacle_shape);

  cugl::Vec2 pos = getWorldPosition() - getPosition();
  if (_decoration_order) pos += getPosition();

  if (_obstacle != nullptr) {
    _obstacle->setSensor(_init_as_sensor);
    _obstacle->setPosition(pos);
    _obstacle->setName(_classname.c_str());

    _obstacle->setBodyType(b2BodyType::b2_staticBody);

    b2Filter filter_data = _obstacle->getFilterData();
    filter_data.categoryBits = CATEGORY_WALL;
    filter_data.maskBits = MASK_WALL;
    _obstacle->setFilterData(filter_data);
  }

  return _obstacle;
}

std::shared_ptr<cugl::scene2::SceneNode> Wall::copy(
    const std::shared_ptr<cugl::scene2::SceneNode>& dst) const {
  BasicTile::copy(dst);

  std::shared_ptr<Wall> node = std::dynamic_pointer_cast<Wall>(dst);
  node->_obstacle_shape = _obstacle_shape;
  node->_init_as_sensor = _init_as_sensor;

  return dst;
}