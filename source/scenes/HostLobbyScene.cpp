#include "HostLobbyScene.h"

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
  _backout = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("host_back"));

  _copy = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("host-lobby-scene_game_copy"));
  _copy_tooltip = _assets->get<cugl::scene2::SceneNode>(
      "host-lobby-scene_game_copy_tooltip-wrapper_copied");
  _copy_tooltip_lifetime = 0;
  _copy->addListener([this](const std::string& name, bool down) {
    if (down) {
      SDL_SetClipboardText(this->_gameid->getText().c_str());
      _copy_tooltip->setVisible(true);
      _copy_tooltip_lifetime = 0;
    }
  });

  _names_in_use = _assets->get<cugl::scene2::SceneNode>(
      "host-lobby-scene_center_menu-status_already-in-use");
  _names_set = _assets->get<cugl::scene2::SceneNode>(
      "host-lobby-scene_center_menu-status_successfully-set");

  _names_waiting = _assets->get<cugl::scene2::SceneNode>(
      "host-lobby-scene_start-game-status_waiting-for-players");
  _names_success = _assets->get<cugl::scene2::SceneNode>(
      "host-lobby-scene_start-game-status_success");

  // Program the buttons
  _backout->addListener([this](const std::string& name, bool down) {
    if (down) {
      disconnect();
      _status = Status::ABORT;
    }
  });

  _startgame->addListener([this](const std::string& name, bool down) {
    if (down) {
      startGame();
    }
  });

  _name->addExitListener(
      [this](const std::string& name, const std::string& current) {
        this->sendPlayerName(current);
      });
  _status = Status::WAIT;

  // handle background and cloud layers
  auto background_layer =
      assets->get<cugl::scene2::SceneNode>("background-host-lobby");
  background_layer->setContentSize(dimen);
  background_layer->setPositionX(getCloudXPosition());
  background_layer->doLayout();

  _cloud_layer = assets->get<cugl::scene2::SceneNode>("clouds-host-lobby");
  _cloud_layer->setContentSize(dimen);
  _cloud_layer->doLayout();

  addChild(background_layer);
  addChild(_cloud_layer);
  addChild(scene);
  setActive(false, nullptr);
  return true;
}

void HostLobbyScene::dispose() {
  if (_active) {
    removeAllChildren();
    _active = false;
    _cloud_layer->dispose();
    _cloud_layer = nullptr;
  }
}

void HostLobbyScene::setActive(
    bool value, std::shared_ptr<cugl::NetworkConnection> network) {
  if (isActive() != value) {
    Scene2::setActive(value);
    if (value) {
      _status = WAIT;
      _network = network;
      _name->setText("");
      _name->activate();
      _startgame->activate();
      _backout->activate();

      _names_set->setVisible(false);
      _names_in_use->setVisible(false);
      _names_success->setVisible(false);
      _names_waiting->setVisible(false);

      _copy->activate();
      _copy_tooltip->setVisible(false);
      _copy_tooltip_lifetime = 0;

      _player_id_to_name.clear();

      _cloud_layer->setPositionX(_cloud_x_pos);
      setGameId(_network->getRoomID());
    } else {
      // TODO deactivate things as necessary
      _name->deactivate();
      _startgame->deactivate();
      _startgame->setDown(false);
      _backout->deactivate();
      _copy->deactivate();
      _copy->setDown(false);
      _backout->setDown(false);

      _names_set->setVisible(false);
      _names_in_use->setVisible(false);
      _names_success->setVisible(false);
      _names_waiting->setVisible(false);

      _player_id_to_name.clear();
    }
  }
}

void HostLobbyScene::startGame() {
  if (_player_id_to_name.size() < _network->getNumPlayers()) {
    _names_waiting->setVisible(true);
    return;
  }

  determineAndSendColors();
  determineAndSendRoles();

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

    if (_player_id_to_name.size() == _network->getNumPlayers()) {
      _names_in_use->setVisible(false);
      _names_waiting->setVisible(false);
      _names_success->setVisible(true);
    }

    // Number of players has changed, resend that lobby is open.
    if (_num_of_players != _network->getNumPlayers()) {
      // This message will make sense to ClientMenuScene.cpp
      _network->send(std::vector<uint8_t>{0});
      _num_of_players = _network->getNumPlayers();
    }
  }

  if (_copy_tooltip->isVisible()) {
    _copy_tooltip_lifetime += timestep;
    if (_copy_tooltip_lifetime >= 1.0f /* seconds */) {
      _copy_tooltip->setVisible(false);
    }
  }

  // update cloud background layer
  _cloud_x_pos = _cloud_x_pos + .3;
  if (_cloud_x_pos >= 0) {
    _cloud_x_pos = CLOUD_WRAP;
  }
  _cloud_layer->setPositionX(_cloud_x_pos);
}

void HostLobbyScene::processData(const std::vector<uint8_t>& data) {
  _deserializer.receive(data);
  Sint32 code = std::get<Sint32>(_deserializer.read());

  if (code == CLIENT_SEND_PLAYER_NAME) {
    cugl::NetworkDeserializer::Message msg = _deserializer.read();
    auto info = std::get<std::shared_ptr<cugl::JsonValue>>(msg);
    HostResponse response =
        processReceivedPlayerName(info->getInt("id"), info->getString("name"));
    _serializer.writeSint32(response);
    _serializer.writeJson(info);
    _network->send(_serializer.serialize());
    _serializer.reset();
  }
};

void HostLobbyScene::sendPlayerName(const std::string& name) {
  if (!_network) return;
  _names_in_use->setVisible(false);
  _names_set->setVisible(false);
  HostResponse reponse =
      processReceivedPlayerName(*_network->getPlayerID(), name);

  switch (reponse) {
    case HOST_ACCEPT_PLAYER_NAME:
      _names_set->setVisible(true);
      break;
    case HOST_DENY_PLAYER_NAME:
      _names_in_use->setVisible(true);
      _names_success->setVisible(false);
      break;
    case HOST_REMOVED_PLAYER_NAME:
      _names_success->setVisible(false);
      break;
    default:
      break;
  }
}

PeerLobbyScene::HostResponse HostLobbyScene::processReceivedPlayerName(
    const int player_id, const std::string& name) {
  if (name == "") {
    if (_player_id_to_name.find(player_id) != _player_id_to_name.end()) {
      _player_id_to_name.erase(player_id);
      return HOST_REMOVED_PLAYER_NAME;
    }
    return HOST_NAME_NO_OP;
  }

  bool deny = false;
  for (auto it : _player_id_to_name) {
    // Name in use by someone else.
    deny |= it.second == name && it.first != player_id;
  }

  if (deny) {
    _player_id_to_name.erase(player_id);
    return HOST_DENY_PLAYER_NAME;
  }

  _player_id_to_name[player_id] = name;
  return HOST_ACCEPT_PLAYER_NAME;
}

bool HostLobbyScene::checkConnection() {
  // TODO does this need to be updated
  switch (_network->getStatus()) {
    case cugl::NetworkConnection::NetStatus::Pending:
      _status = WAIT;
      break;
    case cugl::NetworkConnection::NetStatus::Connected:
      // Set the text from the network
      _player->setText(": " + std::to_string(_network->getNumPlayers()), true);
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

void HostLobbyScene::determineAndSendRoles() {
  // determine number of betrayers for number of connected players
  // TODO make this a constant somewhere
  int num_betrayers_per_num_players[8] = {1, 1, 1, 1, 2, 2, 2, 2};
  int num_players = _network->getNumPlayers();
  int num_betrayers = num_betrayers_per_num_players[num_players - 1];

  // assign betrayers randomly
  std::uniform_int_distribution dist(0, num_players - 1);
  std::random_device rd;
  std::default_random_engine generator(rd());

  // pick number between 0 to numPlayers to represent player index
  int first_betrayer_index = dist(generator);

  // pick second betrayer as number between 0 to last player index - 1
  // if the same index is picked as first betrayer, assign last player
  int second_betrayer_index = -1;
  if (num_betrayers > 1) {
    std::uniform_int_distribution dist_2(0, num_players - 2);
    second_betrayer_index = dist_2(generator);
    if (second_betrayer_index == first_betrayer_index) {
      second_betrayer_index = num_players - 1;
    }
  }

  // find the IDs matched with player indices.
  // this is necessary if any players disconnect in the lobby, as a player might
  // have an id that is >= numPlayers.
  int num_player_ids_found = 0;
  int current_id = 0;
  int player_ids[8] = {};  // 8 is max number of players
  while (num_player_ids_found < num_players) {
    if (_network->isPlayerActive(current_id)) {
      player_ids[num_player_ids_found] = current_id;
      num_player_ids_found += 1;
    }
    current_id += 1;
  }

  // convert index representing player to player ID
  int first_betrayer_id = player_ids[first_betrayer_index];
  int second_betrayer_id = -1;
  if (num_betrayers > 1) {
    second_betrayer_id = player_ids[second_betrayer_index];
  }

  // assign host as betrayer if either id is 0
  if (first_betrayer_id == 0 || second_betrayer_id == 0) {
    _is_betrayer = true;
  } else {
    _is_betrayer = false;
  }

  // send number of betrayers and list of ids to all accounts
  std::shared_ptr<cugl::JsonValue> betrayer_info =
      cugl::JsonValue::allocObject();

  std::shared_ptr<cugl::JsonValue> json_num_betrayers =
      cugl::JsonValue::alloc(static_cast<long>(num_betrayers));
  betrayer_info->appendChild(json_num_betrayers);
  json_num_betrayers->setKey("num_betrayers");

  std::shared_ptr<cugl::JsonValue> betrayer_ids = cugl::JsonValue::allocArray();
  std::shared_ptr<cugl::JsonValue> betrayer_1 =
      cugl::JsonValue::alloc(static_cast<long>(first_betrayer_id));

  betrayer_ids->appendChild(betrayer_1);

  if (num_betrayers > 1) {
    std::shared_ptr<cugl::JsonValue> betrayer_2 =
        cugl::JsonValue::alloc(static_cast<long>(second_betrayer_id));
    betrayer_ids->appendChild(betrayer_2);
  }

  betrayer_info->appendChild(betrayer_ids);
  betrayer_ids->setKey("betrayer_ids");

  _serializer.writeSint32(254);
  _serializer.writeJson(betrayer_info);
  std::vector<uint8_t> msg = _serializer.serialize();
  _serializer.reset();
  _network->send(msg);
}

void HostLobbyScene::determineAndSendColors() {
  int num_players = _network->getNumPlayers();

  auto info = cugl::JsonValue::allocArray();

  std::vector<int> colors{1, 2, 3, 4, 5, 6, 7, 8};

  std::random_device rd;
  std::default_random_engine generator(rd());
  std::uniform_real_distribution<float> dist(0.f, 1.f);

  for (int i = 0; i < num_players; i++) {
    auto player_info = cugl::JsonValue::allocObject();

    int color_ii = (int)(dist(generator) * colors.size() - 1.f);
    _color_ids[i] = colors[color_ii];

    auto player_id = cugl::JsonValue::alloc((long)i);
    player_info->appendChild(player_id);
    player_id->setKey("id");

    auto color_ii_info = cugl::JsonValue::alloc((long)colors[color_ii]);
    player_info->appendChild(color_ii_info);
    color_ii_info->setKey("clr");

    info->appendChild(player_info);

    colors.erase(colors.begin() + color_ii);
  }

  _serializer.writeSint32(253);
  _serializer.writeJson(info);
  std::vector<uint8_t> msg = _serializer.serialize();
  _serializer.reset();
  _network->send(msg);
}
