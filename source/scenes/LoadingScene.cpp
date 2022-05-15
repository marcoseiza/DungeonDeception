#include "LoadingScene.h"

#include "../models/level_gen/DefaultRooms.h"

/** The ideal size of the logo. */
#define SCENE_SIZE 1024
#define NUM_PLAYERS 6
#define PLAYER_SEPARATION 200
#define SPEED 5

bool LoadingScene::init(const std::shared_ptr<cugl::AssetManager> &assets) {
  // Initialize the scene to a locked width.
  _dim = cugl::Application::get()->getDisplaySize();
  // Lock the scene to a reasonable resolution.
  _dim *= SCENE_SIZE / ((_dim.width > _dim.height) ? _dim.width : _dim.height);
  if (assets == nullptr || !cugl::Scene2::init(_dim)) {
    return false;
  }

  // Immediately load the splash screen assets.
  _assets = assets;
  // Queue up the other assets (EMPTY in this case).
  _assets->loadDirectoryAsync(
      "json/assets.json", [=](const std::string &key, bool success) {
        for (default_rooms::RoomConfig room_config : default_rooms::kAllRooms) {
          _assets->loadAsync<cugl::scene2::SceneNode>(
              room_config.scene2_key, room_config.scene2_source, nullptr);
        }
      });
  _assets->loadDirectoryAsync("json/tiles.json", nullptr);
  _assets->loadDirectory("json/loading.json");

  auto layer = assets->get<cugl::scene2::SceneNode>("load");
  layer->setContentSize(_dim);

  _loading_text = std::dynamic_pointer_cast<cugl::scene2::Label>(
      layer->getChildByName("loading"));

  for (int i = 0; i < NUM_PLAYERS; i++) {
    auto player = cugl::scene2::SpriteNode::alloc(
        assets->get<cugl::Texture>("player-loading"), 1, 10);
    player->setScale(1.2);
    player->setRelativeColor(false);
    player->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_CENTER);
    player->setFrame(i % player->getSize());
    float pos_x = PLAYER_SEPARATION * i;
    player->setPosition(pos_x, _dim.height * 0.05f);
    _players.push_back(player);
    layer->addChild(player);
  }

  layer->doLayout();  // This rearranges the children to fit the screen.

  cugl::Application::get()->setClearColor(cugl::Color4(192, 192, 192, 255));
  cugl::Scene2::addChild(layer);

  return true;
}

void LoadingScene::dispose() {
  _assets = nullptr;
  _progress = 0.0f;
}

void LoadingScene::update(float progress) {
  if (_progress < 1) {
    _progress = _assets->progress();

    if (_loading_text->getText() == "loading...") {
      _loading_text->setText("loading.");
    }

    std::stringstream ss;
    ss << "loading";
    if (_loading_text_frame_cooldown > 0) {
      _loading_text_frame_cooldown--;
    } else {
      for (int i = 0; i < _num_of_dots; i++) ss << ".";
      _loading_text->setText(ss.str());
      _num_of_dots = (_num_of_dots + 1) % 3 + 1;
      _loading_text_frame_cooldown =
          cugl::Application::get()->getAverageFPS() / 5;
      if (_num_of_dots == 3) _loading_text_frame_cooldown *= 2;
    }

    for (int i = 0; i < _players.size(); i++) {
      auto player = _players[i];
      player->setPositionX(player->getPositionX() + SPEED);

      if (player->getPositionX() > _dim.width) {
        player->setPositionX(-PLAYER_SEPARATION);
      }

      if (_frame_cooldown <= 0) {
        player->setFrame((player->getFrame() + 1) % player->getSize());
      }
    }

    if (_frame_cooldown > 0) {
      _frame_cooldown--;
    } else {
      _frame_cooldown = cugl::Application::get()->getAverageFPS() / 30;
    }

    if (_progress >= 1) {
      _progress = 1.0f;
      _active = false;
    }
  }
}
