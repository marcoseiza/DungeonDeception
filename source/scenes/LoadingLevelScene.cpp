#include "LoadingLevelScene.h"

#include <cugl/cugl.h>

#include "../generators/LevelGeneratorConfig.h"

#define SCENE_HEIGHT 720

bool LoadingLevelScene::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  std::random_device my_random_device;
  return init(assets, (Uint64)my_random_device(), 5);
}

bool LoadingLevelScene::init(const std::shared_ptr<cugl::AssetManager>& assets,
                             Uint64 seed, int num_players) {
  if (_active) return false;
  _active = true;

  // Initialize the scene to a locked width.
  cugl::Size dim = cugl::Application::get()->getDisplaySize();
  dim *= SCENE_HEIGHT / ((dim.width > dim.height) ? dim.width : dim.height);
  if (!cugl::Scene2::init(dim)) return false;

  _assets = assets;

  _map = cugl::scene2::SceneNode::alloc();
  _map->setPosition(dim / 2);

  auto map_bkg = cugl::scene2::SceneNode::alloc();
  map_bkg->setName("background");
  _map->addChild(map_bkg);

  auto map_rooms = cugl::scene2::SceneNode::alloc();
  _map->addChild(map_rooms);

  auto rooms = cugl::scene2::SceneNode::alloc();
  rooms->setName("rooms");
  auto edges = cugl::scene2::SceneNode::alloc();
  edges->setName("edges");
  map_rooms->addChild(edges);
  map_rooms->addChild(rooms);

  _config.setNumRooms(25 + 6 * (std::max(num_players, 3) - 3));
  _level_generator = std::make_shared<level_gen::LevelGenerator>();
  _level_generator->init(_config, map_rooms, seed);

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
          room->_key = i;
          auto loader = std::dynamic_pointer_cast<cugl::Scene2Loader>(
              _assets->access<cugl::scene2::SceneNode>());
          auto reader = cugl::JsonReader::allocWithAsset(room->_scene2_source);
          auto json = (reader == nullptr ? nullptr : reader->readJson());
          room->_level_node = loader->build("", json);
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
