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

bool WinScene::init(const std::shared_ptr<cugl::AssetManager>& assets,
                    bool runnersWin) {
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
      _assets->get<cugl::scene2::SceneNode>("win-scene");
  auto winner_text = scene->getChildByName<cugl::scene2::Label>("title");
  if (runnersWin) {
    winner_text->setText("RUNNERS WIN!", true);
  } else {
    winner_text->setText("BETRAYERS WIN!", true);
  }
  scene->setContentSize(dimen);
  scene->doLayout();  // Repositions the HUD
  _choice = Choice::NONE;
  _menubutton = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("win-scene_play_play-again"));

  // Program the buttons
  _menubutton->addListener([this](const std::string& name, bool down) {
    if (down) _choice = Choice::GOTOMENU;
  });

  addChild(scene);
  _choice = NONE;
  _menubutton->activate();

  return true;
}

void WinScene::dispose() {
  if (!_active) return;
  _active = false;
  removeAllChildren();
  _menubutton->deactivate();
  // If any were pressed, reset them.
  _menubutton->setDown(false);
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
      _menubutton->activate();
    } else {
      _menubutton->deactivate();
      // If any were pressed, reset them
      _menubutton->setDown(false);
    }
  }
}
