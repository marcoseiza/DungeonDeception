#include "WinScene.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#pragma mark -
#pragma mark Level Layout

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT 720

#pragma mark -
#pragma mark Constructors

bool WinScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
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
      _assets->get<cugl::scene2::SceneNode>("win-screen");
  scene->setContentSize(dimen);
  scene->doLayout();  // Repositions the HUD
  _choice = Choice::NONE;
  _playagainbutton = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("win-screen_play_play-again"));
  _quitbutton = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("win-screen_play_quit"));

  // Program the buttons
  _playagainbutton->addListener([this](const std::string& name, bool down) {
    if (down) _choice = Choice::PLAYAGAIN;
  });
  _quitbutton->addListener([this](const std::string& name, bool down) {
    if (down) _choice = Choice::QUIT;
  });

  addChild(scene);
  _choice = NONE;
  _playagainbutton->activate();
  _quitbutton->activate();

  return true;
}

void WinScene::dispose() {
  if (!_active) return;
  _active = false;
  removeAllChildren();
  _playagainbutton->deactivate();
  _quitbutton->deactivate();
  // If any were pressed, reset them.
  _playagainbutton->setDown(false);
  _quitbutton->setDown(false);
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
void WinScene::setActive(bool value) {
  if (isActive() != value) {
    Scene2::setActive(value);
    if (value) {
      _choice = NONE;
      _playagainbutton->activate();
      _quitbutton->activate();
    } else {
      _playagainbutton->deactivate();
      _quitbutton->deactivate();
      // If any were pressed, reset them
      _playagainbutton->setDown(false);
      _quitbutton->setDown(false);
    }
  }
}
