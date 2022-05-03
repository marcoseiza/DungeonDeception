#ifndef CONTROLLERS_NETWORK_CONTROLLER_H_
#define CONTROLLERS_NETWORK_CONTROLLER_H_

#include <cugl/cugl.h>

#include "CustomNetworkSerializer.h"
#include "NetworkCodes.h"
#include "structs/EnemyStructs.h"
#include "structs/Serializable.h"

class NetworkController {
 public:
  /**
   * Process the network information and update the terminal controller data.
   *
   * @param code The message code
   * @param msg The deserialized message
   */
  typedef std::function<void(
      const Sint32 &code,
      const cugl::CustomNetworkDeserializer::CustomMessage &msg)>
      Listener;

  /** A vector of std::shared_ptr<JsonValue> */
  typedef std::vector<std::shared_ptr<cugl::JsonValue>> InfoVector;

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
  cugl::CustomNetworkSerializer _serializer;

  /** The deserializer used to deserialize complex data sent through the
   * network. */
  cugl::CustomNetworkDeserializer _deserializer;

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
   * @return If the connection (network) is set.
   */
  bool isConnectionSet() { return _network != nullptr; }

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
  Uint32 addListener(Listener listener);

  /**
   * Remove a listener given the listener's key.
   *
   * @param key The key for the listener.
   * @return If the listener was successfully removed.
   */
  bool removeListener(Uint32 key);

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
  void send(const Sint32 &code, const std::shared_ptr<cugl::JsonValue> &info);

  /**
   * Sends json info to all other players
   *
   * Within a few frames, other players should receive this via a call to
   * {@link #receive}.
   *
   * @param code The message code for parsing during receive.
   * @param info The json value info to be sent.
   */
  void send(const Sint32 &code, InfoVector &info);

  /**
   * Sends serializable object info to all other players
   *
   * Within a few frames, other players should receive this via a call to
   * {@link #receive}.
   *
   * @param code The message code for parsing during receive.
   * @param info The serializable info.
   */
  void send(const Sint32 &code,
            const std::shared_ptr<cugl::Serializable> &info);

  /**
   * Sends serializable object info to all other players
   *
   * Within a few frames, other players should receive this via a call to
   * {@link #receive}.
   *
   * @param code The message code for parsing during receive.
   * @param info The serializable info.
   */
  void send(const Sint32 &code,
            std::vector<std::shared_ptr<cugl::Serializable>> &info);

  /**
   * Sends the json info to all other uses. Then calls all the listeners and
   * processes the data directly.
   *
   * @tparam T, info type (eg. Serializable, std::shared_ptr<cugl::JsonValue>)
   */
  template <typename T>
  void sendAndProcess(const Sint32 &code, const T &info) {
    for (auto it : _listeners) {
      (it.second)(code, info);
    }
    send(code, info);
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
   * {@link NetworkSerializer} and {@link NetworkDeserializer} classes to
   * encode more complex data.
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
                      const std::shared_ptr<cugl::JsonValue> &info);

  /**
   * Sends json info to host only
   *
   * Within a few frames, the host should receive this via a call to
   * {@link #receive}.
   *
   * @param code The message code for parsing during receive.
   * @param info The json value info to be sent.
   */
  void sendOnlyToHost(const Sint32 &code, InfoVector &info);

  /**
   * Sends serializable object info to host only
   *
   * Within a few frames, other players should receive this via a call to
   * {@link #receive}.
   *
   * @param code The message code for parsing during receive.
   * @param info The serializable info.
   */
  void sendOnlyToHost(const Sint32 &code,
                      const std::shared_ptr<cugl::Serializable> &info);

  /**
   * Sends serializable object info to host only
   *
   * Within a few frames, other players should receive this via a call to
   * {@link #receive}.
   *
   * @param code The message code for parsing during receive.
   * @param info The serializable info.
   */
  void sendOnlyToHost(const Sint32 &code,
                      std::vector<std::shared_ptr<cugl::Serializable>> &info);

  /**
   * Sends the json info to the host only. If the user is the host, instead it
   * will call all the listeners and process the data directly.
   *
   * @tparam T, info type (eg. Serializable, std::shared_ptr<cugl::JsonValue>)
   */
  template <typename T>
  void sendOnlyToHostOrProcess(const Sint32 &code, const T &info) {
    // If host, this function is a no-op.
    sendOnlyToHost(code, info);

    if (_is_host) {
      for (auto it : _listeners) {
        (it.second)(code, info);
      }
    }
  }

  /** Get the cugl network connection. */
  std::shared_ptr<cugl::NetworkConnection> getConnection() { return _network; }

  /**
   * Vectors are sometimes above the packet limit of 1384 bytes. Therefore, we
   * split it up into sections that are below the limit.
   *
   * @param info The vector json value to be split.
   * @return A vector of vectors of json values.
   */
  std::vector<InfoVector> splitVector(InfoVector &info);

  /**
   * Vectors are sometimes above the packet limit of 1384 bytes. Therefore, we
   * split it up into sections that are below the limit.
   *
   * @param info The vector of serializable objects to be split.
   * @return A vector of vectors of json values.
   */
  std::vector<std::vector<std::shared_ptr<cugl::Serializable>>> splitVector(
      std::vector<std::shared_ptr<cugl::Serializable>> &info);

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
