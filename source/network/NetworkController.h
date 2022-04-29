#ifndef CONTROLLERS_NETWORK_CONTROLLER_H_
#define CONTROLLERS_NETWORK_CONTROLLER_H_

#include <cugl/cugl.h>

#include "NetworkCodes.h"

class NetworkController {
 public:
  /**
   * Process the network information and update the terminal controller data.
   *
   * @param code The message code
   * @param msg The deserialized message
   */
  typedef std::function<void(const Sint32 &code,
                             const cugl::NetworkDeserializer::Message &msg)>
      Listener;

 protected:
  /* Single instance of NetworkController. */
  static std::shared_ptr<NetworkController> _singleton;

  /** A reference to the game's network. */
  std::shared_ptr<cugl::NetworkConnection> _network;

  /** The listener callbacks for receiving information */
  std::unordered_map<Uint32, Listener> _listeners;

  /** The next availiable for the listeners.*/
  Uint32 _next_key;

  /** The serializer used to serialize complex data to send through the network.
   */
  cugl::NetworkSerializer _serializer;

  /** The deserializer used to deserialize complex data sent through the
   * network. */
  cugl::NetworkDeserializer _deserializer;

  bool _is_host;

 public:
  /**
   * @return Singelton instance of NetworkController
   */
  static std::shared_ptr<NetworkController> get() {
    if (_singleton == nullptr) {
      _singleton = std::shared_ptr<NetworkController>{new NetworkController};
    }
    return _singleton;
  }

  /**
   * Initialize the NetworkController and all the actions.
   *
   * @param network A reference to the game network.
   * @return If the controller initializes correctly
   */
  bool init(const std::shared_ptr<cugl::NetworkConnection> &network);

  /**
   * Update the network controller, send information and receive information;
   */
  void update();

  /**
   * Dispose of all internal values.
   *
   * @return If all values disposes correctly;
   */
  bool dispose();

  /**
   * Disconnects this scene from the network controller.
   *
   * Technically, this method does not actually disconnect the network
   * controller. Since the network controller is a smart pointer, it is only
   * fully disconnected when ALL scenes have been disconnected.
   */
  void disconnect() { _network = nullptr; }

  /**
   * Checks that the network connection is still active.
   *
   * Even if you are not sending messages all that often, you need to be calling
   * this method regularly. This method is used to determine the current state
   * of the scene.
   *
   * @return true if the network connection is still active.
   */
  bool checkConnection();

  /**
   * Add a listener to the network receive call.
   *
   * @param listener The listener to add.
   * @return The key for the listener.
   */
  Uint32 addListener(Listener listener) {
    CUAssertLog(_next_key < (Uint32)-1, "No more available listener slots");
    _listeners[_next_key++] = listener;
    return _next_key;
  }

  /**
   * Remove a listener given the listener's key.
   *
   * @param key The key for the listener.
   * @return If the listener was successfully removed.
   */
  bool removeListener(Uint32 key) {
    if (_listeners.find(key) == _listeners.end()) {
      return false;
    }
    _listeners.erase(key);
    return true;
  }

  /**
   * Sends a byte array to all other players.
   *
   * Within a few frames, other players should receive this via a call to
   * {@link #receive}.
   *
   * This requires a connection be established. Otherwise its behavior is
   * undefined.
   *
   * You may choose to either send a byte array directly, or you can use the
   * {@link NetworkSerializer} and {@link NetworkDeserializer} classes to encode
   * more complex data.
   *
   * @param msg The byte array to send.
   */
  void send(const std::vector<uint8_t> &msg) {
    if (_network == nullptr) return;
    _network->send(msg);
  }

  /**
   * Sends json info to all other players
   *
   * Within a few frames, other players should receive this via a call to
   * {@link #receive}.
   *
   * @param code The message code for parsing during receive.
   * @param info The json value info to be sent.
   */
  void send(const Sint32 &code, const std::shared_ptr<cugl::JsonValue> &info) {
    if (_network == nullptr) return;
    _serializer.writeSint32(code);
    _serializer.writeJson(info);

    std::vector<uint8_t> msg = _serializer.serialize();

    _serializer.reset();
    _network->send(msg);
  }

  /**
   * Sends a byte array to the host only.
   *
   * This is only useful when called from a client (player ID != 0). As host,
   * this is method does nothing. Within a few frames, the host should receive
   * this via a call to {@link #receive}
   *
   * This requires a connection be established. Otherwise its behavior is
   * undefined.
   *
   * You may choose to either send a byte array directly, or you can use the
   * {@link NetworkSerializer} and {@link NetworkDeserializer} classes to encode
   * more complex data.
   *
   * @param msg The byte array to send.
   */
  void sendOnlyToHost(const std::vector<uint8_t> &msg) {
    if (_network == nullptr) return;
    _network->sendOnlyToHost(msg);
  }

  /**
   * Sends json info to host only
   *
   * Within a few frames, the host should receive this via a call to
   * {@link #receive}.
   *
   * @param code The message code for parsing during receive.
   * @param info The json value info to be sent.
   */
  void sendOnlyToHost(const Sint32 &code,
                      const std::shared_ptr<cugl::JsonValue> &info) {
    if (_network == nullptr) return;
    _serializer.writeSint32(code);
    _serializer.writeJson(info);

    std::vector<uint8_t> msg = _serializer.serialize();
    _serializer.reset();
    _network->sendOnlyToHost(msg);
  }
  
  /**
   * Sends the json info to the host only. If the user is the host, instead it will call all the listeners and process the data directly.
   */
  void sendOnlyToHostOrProcess(const Sint32 &code,
                               const std::shared_ptr<cugl::JsonValue> &info) {
    if (isHost()) {
      for (auto it : _listeners) {
        (it.second)(code, info);
      }
    } else {
      sendOnlyToHost(code, info);
    }
  }

  /** Get the cugl network connection. */
  std::shared_ptr<cugl::NetworkConnection> getConnection() { return _network; }

  /**
   * Set if this network controller is a host controller.
   * @param val Is host.
   */
  void setIsHost(bool val) { _is_host = val; }

  /**
   * Get if this network controller is a host controller.
   * @return Is host.
   */
  bool isHost() { return _is_host; }

  // Prevent copying.
  NetworkController(NetworkController const &) = delete;
  void operator=(NetworkController const &) = delete;
  ~NetworkController() { dispose(); }

 private:
  NetworkController() : _is_host(false), _next_key(0) {}
};

#endif  // CONTROLLERS_NETWORK_CONTROLLER_H_
