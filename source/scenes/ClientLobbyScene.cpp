#include "ClientLobbyScene.h"

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
#pragma mark Client Methods

bool ClientLobbyScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
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
      _assets->get<cugl::scene2::SceneNode>("client-lobby-scene");
  scene->setContentSize(dimen);
  scene->doLayout();  // Repositions the HUD

  _gameid = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _assets->get<cugl::scene2::SceneNode>("client-lobby-scene_game"));
  _player = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _assets->get<cugl::scene2::SceneNode>(
          "client-lobby-scene_players_field"));
  _name = std::dynamic_pointer_cast<cugl::scene2::TextField>(
      _assets->get<cugl::scene2::SceneNode>(
          "client-lobby-scene_center_name_field_text"));
  _backout = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("host_back"));

  _backout->addListener([this](const std::string& name, bool down) {
    if (down) {
      disconnect();
      _status = Status::ABORT;
    }
  });

  _status = Status::WAIT;

  // handle background and cloud layers
  auto background_layer =
      assets->get<cugl::scene2::SceneNode>("background-client-lobby");
  background_layer->setContentSize(dimen);
  background_layer->doLayout();

  _cloud_layer = assets->get<cugl::scene2::SceneNode>("clouds-client-lobby");
  _cloud_layer->setContentSize(dimen);
  _cloud_layer->doLayout();

  addChild(background_layer);
  addChild(_cloud_layer);
  addChild(scene);
  setActive(false, nullptr);
  return true;
}

void ClientLobbyScene::dispose() {
  if (_active) {
    removeAllChildren();
    _active = false;
    _cloud_layer->dispose();
    _cloud_layer = nullptr;
  }
}

void ClientLobbyScene::setActive(
    bool value, std::shared_ptr<cugl::NetworkConnection> network) {
  if (isActive() != value) {
    Scene2::setActive(value);
    if (value) {
      _status = WAIT;
      _network = network;
      _name->activate();
      _backout->activate();

      auto x = *(_network->getPlayerID());
      _name->setText("runner_" + to_string(x));
    } else {
      // TODO deactivate things as necessary
      _name->deactivate();
      _backout->deactivate();
      _backout->setDown(false);
    }
  }
}

void ClientLobbyScene::update(float timestep) {
  if (_network) {
    _network->receive(
        [this](const std::vector<uint8_t>& data) { processData(data); });
    checkConnection();
  }

  // update cloud background layer
  _cloud_layer->setPositionX(_cloud_layer->getPositionX() + .3);
  if (_cloud_layer->getPositionX() >= 0) {
    _cloud_layer->setPositionX(CLOUD_WRAP);
  }
}

void ClientLobbyScene::processData(const std::vector<uint8_t>& data) {
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

bool ClientLobbyScene::checkConnection() {
  // TODO does this need to be updated
  switch (_network->getStatus()) {
    case cugl::NetworkConnection::NetStatus::Pending:
      _status = WAIT;
      break;
    case cugl::NetworkConnection::NetStatus::Connected:
      // Set the text from the network
      _player->setText(": " + std::to_string(_network->getNumPlayers()));
      if (_status != START) {
        _status = WAIT;
      }
      break;
    case cugl::NetworkConnection::NetStatus::Reconnecting:
      _status = WAIT;
      break;
    case cugl::NetworkConnection::NetStatus::RoomNotFound:
      disconnect();
      _status = ABORT;
      break;
    case cugl::NetworkConnection::NetStatus::ApiMismatch:
      disconnect();
      _status = ABORT;
      break;
    case cugl::NetworkConnection::NetStatus::GenericError:
      disconnect();
      _status = ABORT;
      break;
    case cugl::NetworkConnection::NetStatus::Disconnected:
      disconnect();
      _status = ABORT;
      return false;
  }
  return true;
}
