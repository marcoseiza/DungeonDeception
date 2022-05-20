#include "Terminal.h"

bool Terminal::initWithData(const cugl::Scene2Loader* loader,
                            const std::shared_ptr<cugl::JsonValue>& data) {
  if (_texture != nullptr) {
    CUAssertLog(false, "%s is already initialized", _classname.c_str());
    return false;
  }

  if (!data) return Wall::init();
  if (!Wall::initWithData(loader, data)) return false;

  const cugl::AssetManager* assets = loader->getManager();

  _actv = assets->get<cugl::Texture>(data->getString("texture-activated", ""));
  _corr = assets->get<cugl::Texture>(data->getString("texture-corrupted", ""));

  _reg = _texture;

  return true;
}

std::shared_ptr<cugl::physics2::PolygonObstacle> Terminal::initBox2d() {
  _obstacle = Wall::initBox2d();

  cugl::Vec2 pos = getWorldPosition() - getPosition() + getSize() / 2.0f;
  pos.y -= getSize().height / 4.0f;

  if (_obstacle != nullptr) {
    _obstacle->setUserDataPointer(reinterpret_cast<uintptr_t>(this));
    auto sensor = std::make_shared<b2FixtureDef>();
    sensor->density = 0.0f;
    sensor->isSensor = true;
    _terminal_sensor_name = std::make_shared<std::string>("terminal-sensor");
    sensor->userData.pointer =
        reinterpret_cast<uintptr_t>(_terminal_sensor_name.get());

    b2Vec2 sensor_pos(_obstacle->getSize().width / 2 + 5,
                      _obstacle->getSize().height * 2 / 3 + 5);

    _sensor_shape.m_p = sensor_pos;
    _sensor_shape.m_radius = _obstacle->getSize().width * 4 / 2;

    sensor->shape = &_sensor_shape;

    std::vector<std::shared_ptr<b2FixtureDef>> sensors{sensor};
    _obstacle->addSensors(sensors);
  }

  return _obstacle;
}