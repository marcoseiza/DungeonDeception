#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "HowToPlayScene.h"

#pragma mark -
#pragma mark Level Layout

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT 720

#pragma mark -
#pragma mark Constructors

bool HowToPlayScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  if (_active) return false;
  _active = true;

  _current_slide = 0;
  // Initialize the scene to a locked width
  cugl::Size dimen = cugl::Application::get()->getDisplaySize();
  dimen *= SCENE_HEIGHT / dimen.height;
  if (assets == nullptr || !Scene2::init(dimen)) return false;

  // Start up the input handler
  _assets = assets;

  // Acquire the scene built by the asset loader and resize it the scene
  _scene =_assets->get<cugl::scene2::SceneNode>("how-to-play");
  _scene->setContentSize(dimen);
  _scene->doLayout();  // Repositions the HUD

  auto slide2 = _scene->getChildByName<cugl::scene2::SceneNode>("slide2");
  slide2->setVisible(false);

  auto slide3 = _scene->getChildByName<cugl::scene2::SceneNode>("slide3");
  slide3->setVisible(false);

  auto slide4 = _scene->getChildByName<cugl::scene2::SceneNode>("slide4");
  slide4->setVisible(false);

  _backout = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("how-to-play_back"));
  _left = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("how-to-play_left"));
  _right = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("how-to-play_right"));

  _backout->addListener([this](const std::string& name, bool down) {
    if (down) _choice = Choice::GOTOMENU;
  });
  _left->addListener([this](const std::string& name, bool down) {
    if (down) _change = Change::LEFT;
  });
  _right->addListener([this](const std::string& name, bool down) {
    if (down) _change = Change::RIGHT;
  });

  addChild(_scene);
  _choice = NONE;
  _backout->activate();
  _left->activate();
  _right->activate();
  return true;
}

void HowToPlayScene::update(float timestep) { 
    switch (_change) { 
      case LEFT:
        _change = NO;
        _current_slide--;
        break;
      case RIGHT:
        _change = NO;
        _current_slide++;
        break;
      case NO:
        break;
    }

    auto slide1 = _scene->getChildByName<cugl::scene2::SceneNode>("slide1");
    auto slide2 = _scene->getChildByName<cugl::scene2::SceneNode>("slide2");
    auto slide3 = _scene->getChildByName<cugl::scene2::SceneNode>("slide3");
    auto slide4 = _scene->getChildByName<cugl::scene2::SceneNode>("slide4");

    switch (_current_slide) { 
        case 0: 
          slide1->setVisible(true);
          slide2->setVisible(false);
          slide3->setVisible(false);
          slide4->setVisible(false);
          _left->setVisible(false);
          _left->deactivate();
          break;
        case 1:
          slide2->setVisible(true);
          slide1->setVisible(false);
          slide3->setVisible(false);
          slide4->setVisible(false);
          _left->setVisible(true);
          _left->activate();
          break;
        case 2:
          slide3->setVisible(true);
          slide1->setVisible(false);
          slide2->setVisible(false);
          slide4->setVisible(false);
          _right->setVisible(true);
          _right->activate();
          break;
        case 3:
          slide4->setVisible(true);
          slide1->setVisible(false);
          slide2->setVisible(false);
          slide3->setVisible(false);
          _right->setVisible(false);
          _right->deactivate();
          break;
        default:
          slide1->setVisible(true);
          slide2->setVisible(false);
          slide3->setVisible(false);
          slide4->setVisible(false);
          _left->setVisible(false);
          _left->deactivate();
          break;
    }
    
}

void HowToPlayScene::dispose() {
  if (!_active) return;
  _active = false;
  removeAllChildren();
  _backout->deactivate();
  _left->deactivate();
  _right->deactivate();
  // If any were pressed, reset them.
  _backout->setDown(false);
  _left->setDown(false);
  _right->setDown(false);
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
void HowToPlayScene::setActive(bool value) {
  if (isActive() != value) {
    Scene2::setActive(value);
    if (value) {
      _choice = NONE;
      _backout->activate();
      _left->activate();
      _right->activate();
    } else {
      _backout->deactivate();
      _left->deactivate();
      _right->deactivate();
      _backout->setDown(false);
      _left->setDown(false);
      _right->setDown(false);
    }
  }
}
