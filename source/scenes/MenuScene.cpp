#include "MenuScene.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#pragma mark -
#pragma mark Level Layout

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT 720
/** Set cloud wrap x position based on width and scale of cloud layer **/
#define CLOUD_WRAP -1689.6

#pragma mark -
#pragma mark Constructors

bool MenuScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  if (_active) return false;
  _active = true;

  // Initialize the scene to a locked width
  cugl::Size dimen = cugl::Application::get()->getDisplaySize();
  dimen *= SCENE_HEIGHT / dimen.height;
  if (assets == nullptr || !Scene2::init(dimen)) return false;

  // Start up the input handler
  _assets = assets;

  // Acquire the scene built by the asset loader and resize it the scene
  std::shared_ptr<cugl::scene2::SceneNode> scene =
      _assets->get<cugl::scene2::SceneNode>("menu");
  scene->setContentSize(dimen);
  scene->doLayout();  // Repositions the HUD
  _choice = Choice::NONE;
  _hostbutton = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("menu_play_host"));
  _joinbutton = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("menu_play_join"));
  _htpbutton = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("menu_instructions"));

  // handle background and cloud layers
  auto background_layer =
      assets->get<cugl::scene2::SceneNode>("background-menu");
  background_layer->setContentSize(dimen);
  background_layer->setPositionX(getCloudXPosition());
  background_layer->doLayout();

  _cloud_layer = assets->get<cugl::scene2::SceneNode>("clouds-menu");
  _cloud_layer->setContentSize(dimen);
  _cloud_layer->doLayout();

  // Program the buttons
  _hostbutton->addListener([this](const std::string& name, bool down) {
    if (down) _choice = Choice::HOST;
  });
  _joinbutton->addListener([this](const std::string& name, bool down) {
    if (down) _choice = Choice::JOIN;
  });
  _htpbutton->addListener([this](const std::string& name, bool down) {
    if (down) _choice = Choice::HTP;
  });

  addChild(background_layer);
  addChild(_cloud_layer);
  addChild(scene);

  _choice = NONE;
  _hostbutton->activate();
  _joinbutton->activate();
  _htpbutton->activate();

  return true;
}

void MenuScene::update(float timestep) {
  // update cloud background layer
  _cloud_x_pos = _cloud_x_pos + .3;
  if (_cloud_x_pos >= 0) {
    _cloud_x_pos = CLOUD_WRAP;
  }
  _cloud_layer->setPositionX(_cloud_x_pos);
}

void MenuScene::dispose() {
  if (!_active) return;
  _active = false;
  removeAllChildren();
  _hostbutton->deactivate();
  _joinbutton->deactivate();
  _htpbutton->deactivate();
  _cloud_layer->dispose();
  _cloud_layer = nullptr;
  // If any were pressed, reset them.
  _hostbutton->setDown(false);
  _joinbutton->setDown(false);
  _htpbutton->setDown(false);
}

/**
 * Sets whether the scene is currently active
 *
 * This method should be used to toggle all the UI elements.  Buttons
 * should be activated when it is made active and deactivated when
 * it is not.
 *
 * @param value whether the scene is currently active
 */
void MenuScene::setActive(bool value) {
  if (isActive() != value) {
    Scene2::setActive(value);
    if (value) {
      _choice = NONE;
      _hostbutton->activate();
      _joinbutton->activate();
      _cloud_layer->setPositionX(_cloud_x_pos);
      _htpbutton->activate();
    } else {
      _hostbutton->deactivate();
      _joinbutton->deactivate();
      _htpbutton->deactivate();
      // If any were pressed, reset them
      _hostbutton->setDown(false);
      _joinbutton->setDown(false);
      _htpbutton->setDown(false);
    }
  }
}
