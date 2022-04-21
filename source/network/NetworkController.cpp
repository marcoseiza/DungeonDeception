#include "NetworkController.h"

// static
std::shared_ptr<NetworkController> NetworkController::_singleton = nullptr;

bool NetworkController::init(
    const std::shared_ptr<cugl::NetworkConnection>& network) {
  _network = network;
  _next_key = 0;
  return true;
}

void NetworkController::update() {
  if (_network == nullptr) return;

  _network->receive([this](const std::vector<uint8_t>& data) {
    _deserializer.receive(data);
    Sint32 code = std::get<Sint32>(_deserializer.read());
    cugl::NetworkDeserializer::Message msg = _deserializer.read();
    for (auto it : _listeners) {
      (it.second)(code, msg);
    }
    _deserializer.reset();
  });

  checkConnection();
}

bool NetworkController::dispose() {
  _network = nullptr;
  return true;
}

bool NetworkController::checkConnection() {
  if (_network == nullptr) return false;

  switch (_network->getStatus()) {
    case cugl::NetworkConnection::NetStatus::Pending:
    case cugl::NetworkConnection::NetStatus::Connected:
    case cugl::NetworkConnection::NetStatus::Reconnecting:
      break;
    case cugl::NetworkConnection::NetStatus::RoomNotFound:
    case cugl::NetworkConnection::NetStatus::ApiMismatch:
    case cugl::NetworkConnection::NetStatus::GenericError:
    case cugl::NetworkConnection::NetStatus::Disconnected:
      disconnect();
      return false;
  }
  return true;
}
