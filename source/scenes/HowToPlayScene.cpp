#include "HowToPlayScene.h"

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
  _scene = _assets->get<cugl::scene2::SceneNode>("how-to-play");
  _scene->setContentSize(dimen);
  _scene->doLayout();  // Repositions the HUD

  auto slide1 = _scene->getChildByName<cugl::scene2::SceneNode>("slide1");
  slide1->setVisible(false);

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

  // handle background and cloud layers
  auto background_layer =
      assets->get<cugl::scene2::SceneNode>("background-htp");
  background_layer->setContentSize(dimen);
  background_layer->setPositionX(getCloudXPosition());
  background_layer->doLayout();

  _cloud_layer = assets->get<cugl::scene2::SceneNode>("clouds-htp");
  _cloud_layer->setContentSize(dimen);
  _cloud_layer->doLayout();

  addChild(background_layer);
  addChild(_cloud_layer);
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

  auto slide0 = _scene->getChildByName<cugl::scene2::SceneNode>("slide0");
  auto slide1 = _scene->getChildByName<cugl::scene2::SceneNode>("slide1");
  auto slide2 = _scene->getChildByName<cugl::scene2::SceneNode>("slide2");
  auto slide3 = _scene->getChildByName<cugl::scene2::SceneNode>("slide3");
  auto slide4 = _scene->getChildByName<cugl::scene2::SceneNode>("slide4");

  switch (_current_slide) {
    case 0:
      slide0->setVisible(true);
      slide1->setVisible(false);
      slide2->setVisible(false);
      slide3->setVisible(false);
      slide4->setVisible(false);
      _left->setVisible(false);
      _left->deactivate();
      _right->setVisible(true);
      _right->activate();
      break;
    case 1:
      slide1->setVisible(true);
      slide0->setVisible(false);
      slide2->setVisible(false);
      slide3->setVisible(false);
      slide4->setVisible(false);
      _left->setVisible(true);
      _left->activate();
      _right->setVisible(true);
      _right->activate();
      break;
    case 2:
      slide2->setVisible(true);
      slide0->setVisible(false);
      slide1->setVisible(false);
      slide3->setVisible(false);
      slide4->setVisible(false);
      _left->setVisible(true);
      _left->activate();
      _right->setVisible(true);
      _right->activate();
      break;
    case 3:
      slide3->setVisible(true);
      slide0->setVisible(false);
      slide1->setVisible(false);
      slide2->setVisible(false);
      slide4->setVisible(false);
      _left->setVisible(true);
      _left->activate();
      _right->setVisible(true);
      _right->activate();
      break;
    case 4:
      slide4->setVisible(true);
      slide0->setVisible(false);
      slide1->setVisible(false);
      slide2->setVisible(false);
      slide3->setVisible(false);
      _left->setVisible(true);
      _left->activate();
      _right->setVisible(false);
      _right->deactivate();
      break;
    default:
      slide0->setVisible(true);
      slide1->setVisible(false);
      slide2->setVisible(false);
      slide3->setVisible(false);
      slide4->setVisible(false);
      _left->setVisible(false);
      _left->deactivate();
      _right->setVisible(true);
      _right->activate();
      break;
  }

  // update cloud background layer
  _cloud_x_pos = _cloud_x_pos + .3;
  if (_cloud_x_pos >= 0) {
    _cloud_x_pos = CLOUD_WRAP;
  }
  _cloud_layer->setPositionX(_cloud_x_pos);
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
  _cloud_layer->dispose();
  _cloud_layer = nullptr;
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
      _cloud_layer->setPositionX(_cloud_x_pos);
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
