#include "LoadingScene.h"

#include "../models/level_gen/DefaultRooms.h"

/** The ideal size of the logo. */
#define SCENE_SIZE 1024

bool LoadingScene::init(const std::shared_ptr<cugl::AssetManager> &assets) {
  // Initialize the scene to a locked width.
  cugl::Size dim = cugl::Application::get()->getDisplaySize();
  // Lock the scene to a reasonable resolution.
  if (dim.width > dim.height) {
    dim *= SCENE_SIZE / dim.width;
  } else {
    dim *= SCENE_SIZE / dim.height;
  }
  if (assets == nullptr || !cugl::Scene2::init(dim)) {
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
  layer->setContentSize(dim);
  layer->doLayout();  // This rearranges the children to fit the screen.

  _bar = std::dynamic_pointer_cast<cugl::scene2::ProgressBar>(
      assets->get<cugl::scene2::SceneNode>("load_bar"));

  _brand = assets->get<cugl::scene2::SceneNode>("load_name");

  _button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      assets->get<cugl::scene2::SceneNode>("load_play"));
  _button->addListener(
      [=](const std::string &name, bool down) { this->_active = down; });

  cugl::Application::get()->setClearColor(cugl::Color4(192, 192, 192, 255));
  cugl::Scene2::addChild(layer);

  return true;
}

void LoadingScene::dispose() {
  // Deactivate the button (platform dependent).
  if (isPending()) {
    _button->deactivate();
  }
  _button = nullptr;
  _brand = nullptr;
  _bar = nullptr;
  _assets = nullptr;
  _progress = 0.0f;
}

void LoadingScene::update(float progress) {
  if (_progress < 1) {
    _progress = _assets->progress();
    if (_progress >= 1) {
      _progress = 1.0f;
      _bar->setVisible(false);
      _brand->setVisible(false);
      _button->setVisible(true);
      _button->activate();
    }
    _bar->setProgress(_progress);
  }
}

bool LoadingScene::isPending() const {
  return _button != nullptr && _button->isVisible();
}
