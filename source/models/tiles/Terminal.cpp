#include "Terminal.h"

bool Terminal::initWithData(const cugl::Scene2Loader* loader,
                            const std::shared_ptr<cugl::JsonValue>& data) {
  if (_texture != nullptr) {
    CUAssertLog(false, "%s is already initialized", _classname.c_str());
    return false;
  }

  if (!data) return BasicTile::init();
  if (!BasicTile::initWithData(loader, data)) return false;

  const cugl::AssetManager* assets = loader->getManager();

  _actv = assets->get<cugl::Texture>(data->getString("texture-activated", ""));
  _corr = assets->get<cugl::Texture>(data->getString("texture-corrupted", ""));

  _reg = _texture;
  return true;
}

std::shared_ptr<TerminalSensor> Terminal::initBox2d() {
  cugl::Vec2 pos = getWorldPosition() - getPosition() + getSize() / 2.0f;
  pos.y -= getSize().height / 4.0f;

  _obstacle = TerminalSensor::alloc(pos, getSize());

  if (_obstacle != nullptr) {
    _obstacle->setPosition(pos);
    _obstacle->setName(_classname.c_str());
    _obstacle->setBodyType(b2BodyType::b2_staticBody);
  }

  return _obstacle;
}