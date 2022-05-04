#include "LoadingLevelScene.h"

#include <cugl/cugl.h>

#include "../generators/LevelGeneratorConfig.h"

#define SCENE_HEIGHT 720

bool LoadingLevelScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  std::random_device my_random_device;
  return init(assets, (Uint64)my_random_device());
}

bool LoadingLevelScene::init(const std::shared_ptr<cugl::AssetManager>& assets,
                             Uint64 seed) {
  if (_active) return false;
  _active = true;

  // Initialize the scene to a locked width.
  cugl::Size dim = cugl::Application::get()->getDisplaySize();
  dim *= SCENE_HEIGHT / ((dim.width > dim.height) ? dim.width : dim.height);
  if (!cugl::Scene2::init(dim)) return false;

  _assets = assets;

  _map = cugl::scene2::SceneNode::alloc();
  _map->setPosition(dim / 2);
  _level_generator = std::make_shared<level_gen::LevelGenerator>();
  _level_generator->init(_config, _map, seed);

  _map->doLayout();
  cugl::Scene2::addChild(_map);

  return true;
}

void LoadingLevelScene::dispose() {
  if (_active) return;
  _active = false;
  _level_generator = nullptr;
  _map = nullptr;
  _loading_phase = GENERATE_ROOMS;
}

void LoadingLevelScene::update(float timestep) {
  cugl::Application::get()->setClearColor(cugl::Color4(230, 228, 211));
  switch (_loading_phase) {
    case GENERATE_ROOMS:
      if (!_level_generator->update()) {
        _loading_phase = LOAD_ROOM_SCENE2;

        std::vector<std::shared_ptr<level_gen::Room>> rooms =
            _level_generator->getRooms();

        for (int i = 0; i < rooms.size(); i++) {
          std::shared_ptr<level_gen::Room> room = rooms[i];
          std::string key = "room-" + std::to_string(i);
          room->_scene2_key = key;  // Update unique key for future reference.
          room->_key = i;
          _assets->loadAsync<cugl::scene2::SceneNode>(key, room->_scene2_source,
                                                      nullptr);
        }
      }
      break;
    case LOAD_ROOM_SCENE2:
      if (_assets->progress() >= 1) {
        _loading_phase = DONE;
      }
      break;
    case DONE:
      _active = false;
      break;
  }
}

void LoadingLevelScene::render(
    const std::shared_ptr<cugl::SpriteBatch>& batch) {
  Scene2::render(batch);
}
