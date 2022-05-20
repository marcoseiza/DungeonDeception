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

  _names_in_use = _assets->get<cugl::scene2::SceneNode>(
      "client-lobby-scene_center_menu-status_already-in-use");
  _names_set = _assets->get<cugl::scene2::SceneNode>(
      "client-lobby-scene_center_menu-status_successfully-set");

  _copy = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("client-lobby-scene_game_copy"));
  _copy_tooltip = _assets->get<cugl::scene2::SceneNode>(
      "client-lobby-scene_game_copy_tooltip-wrapper_copied");
  _copy_tooltip_lifetime = 0;
  _copy->addListener([this](const std::string& name, bool down) {
    if (down) {
      int success = SDL_SetClipboardText(this->_gameid->getText().c_str());
      if (success == 0) {
        _copy_tooltip->setVisible(true);
        _copy_tooltip_lifetime = 0;
      }
    }
  });

  _backout->addListener([this](const std::string& name, bool down) {
    if (down) {
      if (_network) {
        auto info = cugl::JsonValue::allocObject();

        auto player_id = cugl::JsonValue::alloc((long)*_network->getPlayerID());
        info->appendChild(player_id);
        player_id->setKey("id");

        _serializer.writeSint32(CLIENT_REMOVE_PLAYER_NAME);
        _serializer.writeJson(info);
        _network->sendOnlyToHost(_serializer.serialize());
        _serializer.reset();
      }
      disconnect();
      _status = Status::ABORT;
    }
  });

  _name->addExitListener(
      [this](const std::string& name, const std::string& current) {
        if (current != "") this->sendPlayerName(current);
      });

  _status = Status::WAIT;

  // handle background and cloud layers
  auto background_layer =
      assets->get<cugl::scene2::SceneNode>("background-client-lobby");
  background_layer->setContentSize(dimen);
  background_layer->setPositionX(getCloudXPosition());
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
      _name->setText("");
      _backout->activate();

      _copy->activate();
      _copy_tooltip->setVisible(false);
      _copy_tooltip_lifetime = 0;

      _names_set->setVisible(false);
      _names_in_use->setVisible(false);
      _cloud_layer->setPositionX(_cloud_x_pos);
    } else {
      // TODO deactivate things as necessary
      _name->deactivate();
      _backout->deactivate();
      _copy->deactivate();
      _copy->setDown(false);
      _backout->setDown(false);
      _names_set->setVisible(false);
      _names_in_use->setVisible(false);
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
  _cloud_x_pos = _cloud_x_pos + .3;
  if (_cloud_x_pos >= 0) {
    _cloud_x_pos = CLOUD_WRAP;
  }
  _cloud_layer->setPositionX(_cloud_x_pos);

  if (_copy_tooltip->isVisible()) {
    _copy_tooltip_lifetime += timestep;
    if (_copy_tooltip_lifetime >= 1.0f /* seconds */) {
      _copy_tooltip->setVisible(false);
    }
  }
}

void ClientLobbyScene::processData(const std::vector<uint8_t>& data) {
  _deserializer.receive(data);
  Sint32 code = std::get<Sint32>(_deserializer.read());

  if (code == 253) {
    cugl::NetworkDeserializer::Message msg = _deserializer.read();
    auto info = std::get<std::shared_ptr<cugl::JsonValue>>(msg);

    for (auto player_info : info->children()) {
      _color_ids[player_info->getInt("id")] = player_info->getInt("clr");
    }
  }

  switch (code) {
    case HOST_ACCEPT_PLAYER_NAME: {
      cugl::NetworkDeserializer::Message msg = _deserializer.read();
      auto info = std::get<std::shared_ptr<cugl::JsonValue>>(msg);
      if (info->getInt("id") == *_network->getPlayerID()) {
        _names_in_use->setVisible(false);
        _names_set->setVisible(true);
      }
    } break;

    case HOST_DENY_PLAYER_NAME: {
      cugl::NetworkDeserializer::Message msg = _deserializer.read();
      auto info = std::get<std::shared_ptr<cugl::JsonValue>>(msg);
      if (info->getInt("id") == *_network->getPlayerID()) {
        _names_in_use->setVisible(true);
        _names_set->setVisible(false);
      }
    } break;

    case HOST_REMOVED_PLAYER_NAME:
    case HOST_NAME_NO_OP:
      break;
  }

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

void ClientLobbyScene::sendPlayerName(const std::string& name) {
  if (!_network) return;
  _names_in_use->setVisible(false);

  auto info = cugl::JsonValue::allocObject();

  auto player_id = cugl::JsonValue::alloc((long)*_network->getPlayerID());
  info->appendChild(player_id);
  player_id->setKey("id");

  auto name_info = cugl::JsonValue::alloc(name);
  info->appendChild(name_info);
  name_info->setKey("name");

  _serializer.writeSint32(CLIENT_SEND_PLAYER_NAME);
  _serializer.writeJson(info);
  _network->sendOnlyToHost(_serializer.serialize());
  _serializer.reset();
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
    case cugl::NetworkConnection::NetStatus::NoInternetError:
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
