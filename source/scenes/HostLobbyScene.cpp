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

void HostLobbyScene::determineAndSendRoles() {
  // determine number of betrayers for number of connected players
  // TODO make this a constant somewhere
  int num_betrayers_per_num_players[8] = {1, 1, 1, 1, 1, 2, 2, 2};
  int num_players = _network->getNumPlayers();
  int num_betrayers = num_betrayers_per_num_players[num_players - 1];

  // assign betrayers randomly
  srand(time(0));

  // pick number between 0 to numPlayers to represent player index
  int first_betrayer_index = rand() % num_players;

  // pick second betrayer as number between 0 to last player index - 1
  // if the same index is picked as first betrayer, assign last player
  int second_betrayer_index = -1;
  if (num_betrayers > 1) {
    second_betrayer_index = rand() % (num_players - 1);
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
