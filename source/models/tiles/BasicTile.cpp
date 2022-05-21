#include "BasicTile.h"

bool BasicTile::initWithData(const cugl::Scene2Loader* loader,
                             const std::shared_ptr<cugl::JsonValue>& data) {
  if (_texture != nullptr) {
    CUAssertLog(false, "%s is already initialized", _classname.c_str());
    return false;
  }
  _decoration_order = false;

  if (!data) return init();

  _decoration_order = data->getBool("decoration-order", false);
  _decoration_offset = data->getFloat("decoration-offset", 0.0f);

  return PolygonNode::initWithData(loader, data);
}