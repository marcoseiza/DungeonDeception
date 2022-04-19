#include "SoundController.h"

#include "NetworkController.h"

// Todo: Find a nicer way of getting duration
// The problem is that getting the duration right after playing the sound is not
// always safe. The device can take long to actually register the sound and
// within that wait time, the code can crash.
#define MUSIC_MAIN_DURATION 133.953491

bool SoundController::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;
  _has_sent_music_start = false;
  _all_players_in_game = false;

  _network_listener_key = NetworkController::get()->addListener(
      [=](const Sint32& code, const cugl::NetworkDeserializer::Message& msg) {
        this->processNetworkData(code, msg);
      });

  return true;
}

/** Update the controller state. */
void SoundController::update(float timestep) {
  if (_all_players_in_game && NetworkController::get()->isHost()) {
    if (!_has_sent_music_start) {  // Only send once.
      using namespace std::chrono;
      _has_sent_music_start = true;
      auto info = cugl::JsonValue::allocObject();
      auto now = system_clock::now();
      // This music needs to start a little later due to network lag.
      _music_start = now + milliseconds(200);

      auto start_time =
          cugl::JsonValue::alloc((long)_music_start.time_since_epoch().count());

      info->appendChild(start_time);
      start_time->setKey("start_time");

      NetworkController::get()->send(NC_HOST_MUSIC_START, info);
    }
  }

  // Start the game music if not already started and we have received
  // the start time.

  if (_has_sent_music_start) {
    auto q = cugl::AudioEngine::get()->getMusicQueue();
    if (q->getState() == cugl::AudioEngine::State::INACTIVE) {
      using namespace std::chrono;

      system_clock::time_point now = system_clock::now();
      long now_micros = now.time_since_epoch().count();
      long start_micros = _music_start.time_since_epoch().count();

      if (now_micros > start_micros) {
        q->play(_assets->get<cugl::Sound>("music-main"), true, 1.0f, 1.0f);

        float sec_diff = (float)(now_micros - start_micros) / 1000000.0f;
        sec_diff = std::fmod(sec_diff, MUSIC_MAIN_DURATION);
        CULog("%f, %ld", sec_diff, start_micros);

        q->setTimeElapsed(sec_diff);
      }
    }
  }
}

/** Dispose the controller and all its values. */
void SoundController::dispose() {
  NetworkController::get()->removeListener(_network_listener_key);
  _has_sent_music_start = false;
  _all_players_in_game = false;
  _assets = nullptr;
}

void SoundController::processNetworkData(
    const Sint32& code, const cugl::NetworkDeserializer::Message& msg) {
  switch (code) {
    case NC_HOST_MUSIC_START: {
      auto data = std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      long start_time = data->getLong("start_time");

      _music_start = std::chrono::system_clock::time_point(
          std::chrono::microseconds{start_time});
      _has_sent_music_start = true;
    } break;
    default:
      break;
  }
}
