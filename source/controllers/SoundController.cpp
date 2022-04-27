#include "SoundController.h"

#include "NetworkController.h"

bool SoundController::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  std::random_device rd;
  _generator = std::default_random_engine(rd());

  initMusic();
  initPlayerSFX();

  return true;
}

void SoundController::update(float timestep) {}

void SoundController::dispose() { _assets = nullptr; }

void SoundController::initMusic() {
  _music_state = MusicState::MAIN;
  startMusic();
}

void SoundController::startMusic() {
  auto q = cugl::AudioEngine::get()->getMusicQueue();
  if (q->getState() != cugl::AudioEngine::State::INACTIVE) return;

  _music_mixer = cugl::audio::AudioMixer::alloc(2);

  _music_main = cugl::audio::AudioFader::alloc(
      _assets->get<cugl::Sound>("music-main")->createNode());
  _music_boss = cugl::audio::AudioFader::alloc(
      _assets->get<cugl::Sound>("music-boss")->createNode());

  _music_main->setGain((_music_state == MusicState::MAIN) ? 1 : 0);
  _music_boss->setGain((_music_state == MusicState::BOSS) ? 1 : 0);

  _music_mixer->attach(0, _music_main);
  _music_mixer->attach(1, _music_boss);

  q->play(_music_mixer, true, 1.0f, 1.0f);
}

void SoundController::switchMusic() {
  if (_music_main == nullptr || _music_boss == nullptr) return;

  if (_music_state == MusicState::MAIN) {
    _music_boss->setGain(1);
    _music_boss->setPosition(_music_main->getPosition());
    _music_boss->fadeIn(2 /* seconds */);
    _music_main->fadeOut(4 /* seconds */, true);
    _music_state = MusicState::BOSS;
  } else if (_music_state == MusicState::BOSS) {
    _music_main->setGain(1);
    _music_main->setPosition(_music_boss->getPosition());
    _music_main->fadeIn(2 /* seconds */);
    _music_boss->fadeOut(4 /* seconds */, true);
    _music_state = MusicState::MAIN;
  }
}

void SoundController::pauseMusic(float fade) {
  auto q = cugl::AudioEngine::get()->getMusicQueue();
  q->pause(fade);
}

void SoundController::resumeMusic() {
  auto q = cugl::AudioEngine::get()->getMusicQueue();
  q->resume();
}

void SoundController::stopMusic() {
  auto q = cugl::AudioEngine::get()->getMusicQueue();
  q->clear();
  _music_mixer = nullptr;
  _music_main = nullptr;
  _music_boss = nullptr;
}

void SoundController::initPlayerSFX() {
  _player_swing.push_back(
      SFX("player-attack-swing-1",
          _assets->get<cugl::Sound>("player-attack-swing-1")));

  _player_swing.push_back(
      SFX("player-attack-swing-2",
          _assets->get<cugl::Sound>("player-attack-swing-2")));

  _player_hit.push_back(SFX("player-attack-hit-1",
                            _assets->get<cugl::Sound>("player-attack-hit-1")));
}

void SoundController::playPlayerSwing() {
  auto sfx = pickRandom(_player_swing);
  cugl::AudioEngine::get()->play(sfx.name, sfx.sound, false, 1.0f, true);
}

void SoundController::playPlayerHit() {
  auto sfx = pickRandom(_player_hit);
  cugl::AudioEngine::get()->play(sfx.name, sfx.sound, false, 1.0f, true);
}
