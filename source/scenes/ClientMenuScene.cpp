#include "ClientMenuScene.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#pragma mark -
#pragma mark Level Layout

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT 720

#pragma mark -
#pragma mark Client Methods

bool ClientMenuScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  // TODO factor out a lot of this common code to peer scene (if we keep the
  // scene stuff)

  // Initialize the scene to a locked width
  cugl::Size dimen = cugl::Application::get()->getDisplaySize();
  dimen *= SCENE_HEIGHT / dimen.height;
  if (assets == nullptr) {
    return false;
  } else if (!Scene2::init(dimen)) {
    return false;
  }

  // Start up the input handler
  _assets = assets;

  // Acquire the scene built by the asset loader and resize it the scene
  std::shared_ptr<cugl::scene2::SceneNode> scene =
      _assets->get<cugl::scene2::SceneNode>("client");
  scene->setContentSize(dimen);
  scene->doLayout();  // Repositions the HUD

  _startgame = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>(
          "client_center_content_info_start"));
  _backout = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("client_back"));
  _gameid = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _assets->get<cugl::scene2::SceneNode>(
          "client_center_content_info_game_field_text"));

  for (int i = 0; i <= 9; i++) {
    std::string key = "client_center_content_keys_" + to_string(i);
    std::shared_ptr<cugl::scene2::Button> button =
        std::dynamic_pointer_cast<cugl::scene2::Button>(
            _assets->get<cugl::scene2::SceneNode>(key));
    _keypad_buttons.push_back(button);
  }

  _x_button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("client_center_content_keys_x"));

  _status = Status::IDLE;

  _backout->addListener([this](const std::string& name, bool down) {
    if (down) {
      disconnect();
      _status = Status::ABORT;
    }
  });

  _startgame->addListener([=](const std::string& name, bool down) {
    if (down) {
      connect(_gameid->getText());
    }
  });

  // add listener to each keypad button to type in code
  for (int i = 0; i < _keypad_buttons.size(); i++) {
    std::shared_ptr<cugl::scene2::Button> button = _keypad_buttons.at(i);
    button->addListener([=](const std::string& name, bool down) {
      // add text to game id when keypad pressed if code not complete
      if (down && _gameid->getText().size() < 5) {
        _gameid->setText(_gameid->getText() + to_string(i));
      }
    });
  }
  _x_button->addListener([this](const std::string& name, bool down) {
    if (down) {
      std::string curr_text = _gameid->getText();
      _gameid->setText(curr_text.substr(0, curr_text.length() - 1));
    }
  });

  // Create the server configuration
  auto json = _assets->get<cugl::JsonValue>("server");
  _config.punchthroughServerAddr = json->getString("address", "");
  _config.punchthroughServerPort = json->getInt("port", 0);
  _config.maxNumPlayers = json->getInt("maximum", 0);
  _config.apiVersion = json->getInt("version", 0);

  addChild(scene);
  setActive(false);
  return true;
}

void ClientMenuScene::dispose() {
  if (_active) {
    removeAllChildren();
    _active = false;
  }
}

void ClientMenuScene::setActive(bool value) {
  if (isActive() != value) {
    Scene2::setActive(value);
    if (value) {
      _status = IDLE;
      _backout->activate();
      _x_button->activate();
      for (int i = 0; i < _keypad_buttons.size(); i++) {
        std::shared_ptr<cugl::scene2::Button> button = _keypad_buttons[i];
        button->activate();
      }

      _network = nullptr;
      configureStartButton();
      // Don't reset the room id
    } else {
      _startgame->deactivate();
      _backout->deactivate();
      _x_button->deactivate();
      for (int i = 0; i < _keypad_buttons.size(); i++) {
        std::shared_ptr<cugl::scene2::Button> button = _keypad_buttons[i];
        button->deactivate();
      }

      // If any were pressed, reset them
      _startgame->setDown(false);
      _backout->setDown(false);
      _x_button->setDown(false);

      for (int i = 0; i < _keypad_buttons.size(); i++) {
        std::shared_ptr<cugl::scene2::Button> button = _keypad_buttons[i];
        button->setDown(false);
      }
    }
  }
}

void ClientMenuScene::updateText(
    const std::shared_ptr<cugl::scene2::Button>& button,
    const std::string text) {
  auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
      button->getChildByName("up")->getChildByName("label"));
  label->setText(text);
}

void ClientMenuScene::update(float timestep) {
  if (_network) {
    _network->receive(
        [this](const std::vector<uint8_t>& data) { processData(data); });
    checkConnection();
    // Do this last for button safety
    configureStartButton();
  }
}

void ClientMenuScene::processData(const std::vector<uint8_t>& data){};

bool ClientMenuScene::connect(const std::string room) {
  _network = cugl::NetworkConnection::alloc(_config, room);
  return checkConnection();
}

bool ClientMenuScene::checkConnection() {
  switch (_network->getStatus()) {
    case cugl::NetworkConnection::NetStatus::Pending:
      _status = JOIN;
      break;
    case cugl::NetworkConnection::NetStatus::Connected:
      _status = WAIT;
      break;
    case cugl::NetworkConnection::NetStatus::Reconnecting:
      _status = JOIN;
      break;
    case cugl::NetworkConnection::NetStatus::RoomNotFound:
      disconnect();
      _status = IDLE;
      break;
    case cugl::NetworkConnection::NetStatus::ApiMismatch:
      disconnect();
      _status = IDLE;
      break;
    case cugl::NetworkConnection::NetStatus::GenericError:
      disconnect();
      _status = IDLE;
      break;
    case cugl::NetworkConnection::NetStatus::Disconnected:
      disconnect();
      _status = IDLE;
      return false;
  }
  return true;
}

void ClientMenuScene::configureStartButton() {
  if (_status == Status::IDLE) {
    _startgame->activate();
    updateText(_startgame, "JOIN");
  } else if (_status == Status::JOIN) {
    _startgame->deactivate();
    updateText(_startgame, "...");
  } else if (_status == Status::WAIT) {
    updateText(_startgame, "WAIT");
  }
}
