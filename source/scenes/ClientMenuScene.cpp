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
      _assets->get<cugl::scene2::SceneNode>("client_center_start"));
  _backout = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("client_back"));
  _gameid = std::dynamic_pointer_cast<cugl::scene2::TextField>(
      _assets->get<cugl::scene2::SceneNode>("client_center_game_field_text"));
  _player = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _assets->get<cugl::scene2::SceneNode>(
          "client_center_players_field_text"));
  _status = Status::IDLE;

  _backout->addListener([this](const std::string& name, bool down) {
    if (down) {
      disconnect();
      _status = Status::ABORT;
    }
  });

  _startgame->addListener([=](const std::string& name, bool down) {
    if (down) {
      // This will call the _gameid listener
      if (_gameid->hasFocus()) {
        _gameid->releaseFocus();
      } else {
        connect(_gameid->getText());
      }
    }
  });

  _gameid->addExitListener(
      [this](const std::string& name, const std::string& value) {
        connect(value);
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
      _gameid->activate();
      _backout->activate();
      _network = nullptr;
      _player->setText("1");
      configureStartButton();
      // Don't reset the room id
    } else {
      _gameid->deactivate();
      _startgame->deactivate();
      _backout->deactivate();
      // If any were pressed, reset them
      _startgame->setDown(false);
      _backout->setDown(false);
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

bool ClientMenuScene::connect(const std::string room) {
  _network = cugl::NetworkConnection::alloc(_config, room);
  return checkConnection();
}

void ClientMenuScene::processData(const std::vector<uint8_t>& data) {
  _deserializer.receive(data);
  Sint32 code = std::get<Sint32>(_deserializer.read());

  if (code == 254) {
    cugl::NetworkDeserializer::Message msg = _deserializer.read();
    std::shared_ptr<cugl::JsonValue> betrayer_info =
        std::get<std::shared_ptr<cugl::JsonValue>>(msg);
    int num_betrayers = betrayer_info->getInt("num_betrayers");

    _is_betrayer = false;
    std::shared_ptr<cugl::JsonValue> betrayer_ids =
        betrayer_info->get("betrayer_ids");

    for (int i = 0; i < num_betrayers; i++) {
      if (_network->getPlayerID() == betrayer_ids->get(i)->asInt()) {
        _is_betrayer = true;
        break;
      }
    }
  }

  if (code == 255) {
    _seed = std::get<Uint64>(_deserializer.read());
    _status = Status::START;
  }
}

bool ClientMenuScene::checkConnection() {
  switch (_network->getStatus()) {
    case cugl::NetworkConnection::NetStatus::Pending:
      _status = JOIN;
      break;
    case cugl::NetworkConnection::NetStatus::Connected:
      _player->setText(std::to_string(_network->getNumPlayers()));
      if (_status != START) {
        _status = WAIT;
      }
      break;
    case cugl::NetworkConnection::NetStatus::Reconnecting:
      _status = WAIT;
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
    updateText(_startgame, "Start Game");
  } else if (_status == Status::JOIN) {
    _startgame->deactivate();
    updateText(_startgame, "Connecting");
  } else if (_status == Status::WAIT) {
    updateText(_startgame, "Waiting");
  }
}
