#include "NetworkController.h"

// static
std::shared_ptr<NetworkController> NetworkController::_singleton = nullptr;

bool NetworkController::init(
    const std::shared_ptr<cugl::NetworkConnection> &network) {
  _network = network;
  _next_key = 0;
  return true;
}

void NetworkController::update() {
  if (_network == nullptr) return;

  _network->receive([this](const std::vector<uint8_t> &data) {
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

Uint32 NetworkController::addListener(Listener listener) {
  CUAssertLog(_next_key < (Uint32)-1, "No more available listener slots");
  _listeners[_next_key++] = listener;
  return _next_key;
}

bool NetworkController::removeListener(Uint32 key) {
  if (_listeners.find(key) == _listeners.end()) {
    return false;
  }
  _listeners.erase(key);
  return true;
}

void NetworkController::send(const Sint32 &code,
                             const std::shared_ptr<cugl::JsonValue> &info) {
  if (_network == nullptr) return;
  _serializer.writeSint32(code);
  _serializer.writeJson(info);

  std::vector<uint8_t> msg = _serializer.serialize();

  _serializer.reset();
  _network->send(msg);
}

void NetworkController::send(const Sint32 &code, InfoVector &info) {
  if (_network == nullptr) return;
  _serializer.writeSint32(code);
  _serializer.writeJsonVector(info);

  std::vector<uint8_t> msg = _serializer.serialize();
  if (msg.size() > _network->getMaxPacketSize()) {
    std::vector<InfoVector> split_info = splitVector(info);
    for (InfoVector sub_info : split_info) {
      _serializer.reset();
      _serializer.writeSint32(code);
      _serializer.writeJsonVector(sub_info);
      _network->sendOnlyToHost(msg);
    }
    return;
  }

  _serializer.reset();
  _network->send(msg);
}

void NetworkController::sendOnlyToHost(
    const Sint32 &code, const std::shared_ptr<cugl::JsonValue> &info) {
  if (_network == nullptr) return;
  _serializer.writeSint32(code);
  _serializer.writeJson(info);

  std::vector<uint8_t> msg = _serializer.serialize();
  _serializer.reset();
  _network->sendOnlyToHost(msg);
}

void NetworkController::sendOnlyToHost(const Sint32 &code, InfoVector &info) {
  if (_network == nullptr) return;
  _serializer.writeSint32(code);
  _serializer.writeJsonVector(info);

  std::vector<uint8_t> msg = _serializer.serialize();
  if (msg.size() > _network->getMaxPacketSize()) {
    std::vector<InfoVector> split_info = splitVector(info);
    for (InfoVector sub_info : split_info) {
      _serializer.reset();
      _serializer.writeSint32(code);
      _serializer.writeJsonVector(sub_info);
      _network->sendOnlyToHost(msg);
    }
    return;
  }
  _serializer.reset();
  _network->sendOnlyToHost(msg);
}

std::vector<NetworkController::InfoVector> NetworkController::splitVector(
    InfoVector &info) {
  CUAssertLog(info.size() > 1, "Vector of size %d, cannot be split up.",
              (int)info.size());

  _serializer.reset();

  // First approximate the size of one entry in the vector.
  _serializer.writeJson(info[0]);
  std::size_t size_one = _serializer.serialize().size();
  // Add a small value to approx for the code.
  size_one += sizeof(Sint32) / info.size();
  _serializer.reset();

  std::vector<InfoVector> split_info;
  int ii = 0;                // Current index in given info.
  int jj = 0;                // Current index in split_info.
  std::size_t cur_size = 0;  // The size of the current index.
  while (ii < info.size()) {
    cur_size += size_one;
    if (cur_size > _network->getMaxPacketSize()) {  // Reset to next entry.
      cur_size = size_one;
      jj++;
    }
    split_info[jj].push_back(info[ii++]);
  }

  return split_info;
}