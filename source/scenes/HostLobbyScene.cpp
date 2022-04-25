#include "HostLobbyScene.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#pragma mark -
#pragma mark Level Layout

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT 720

#pragma mark -
#pragma mark Host Methods

bool HostLobbyScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
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
      _assets->get<cugl::scene2::SceneNode>("host-lobby-scene");
  scene->setContentSize(dimen);
  scene->doLayout();  // Repositions the HUD

  _gameid = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _assets->get<cugl::scene2::SceneNode>("host-lobby-scene_game"));
  _player = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _assets->get<cugl::scene2::SceneNode>("host-lobby-scene_players_field"));
  _name = std::dynamic_pointer_cast<cugl::scene2::TextField>(
      _assets->get<cugl::scene2::SceneNode>(
          "host-lobby-scene_center_name_field_text"));
  _startgame = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("host-lobby-scene_start"));

  _startgame->addListener([this](const std::string& name, bool down) {
    if (down) {
      startGame();
    }
  });

  _status = Status::WAIT;

  addChild(scene);
  setActive(false, nullptr);
  return true;
}

void HostLobbyScene::dispose() {
  if (_active) {
    removeAllChildren();
    _active = false;
  }
}

void HostLobbyScene::setActive(
    bool value, std::shared_ptr<cugl::NetworkConnection> network) {
  if (isActive() != value) {
    Scene2::setActive(value);
    if (value) {
      _status = WAIT;
      _network = network;
      _name->activate();
      _startgame->activate();

      setGameId(_network->getRoomID());

      auto x = *(_network->getPlayerID());
      _name->setText("runner_" + to_string(x));
    } else {
      // TODO deactivate things as necessary
      _name->deactivate();
      _startgame->deactivate();
      _startgame->setDown(false);
    }
  }
}

void HostLobbyScene::startGame() {
  // determineAndSendRoles();

  std::random_device my_random_device;
  _seed = my_random_device();

  // Send individual player information.
  _serializer.writeSint32(255);
  _serializer.writeUint64(_seed);
  std::vector<uint8_t> msg = _serializer.serialize();
  _serializer.reset();

  _network->send(msg);

  _status = START;
}

void HostLobbyScene::update(float timestep) {
  if (_network) {
    _network->receive(
        [this](const std::vector<uint8_t>& data) { processData(data); });
    checkConnection();
  }
}

void HostLobbyScene::processData(const std::vector<uint8_t>& data) {
  _deserializer.receive(data);
  Sint32 code = std::get<Sint32>(_deserializer.read());

  // TODO process data with new codes for starting game, updating players, etc.

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

bool HostLobbyScene::checkConnection() {
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
