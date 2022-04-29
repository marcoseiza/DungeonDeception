#include "SoundController.h"

bool SoundController::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;

  std::random_device rd;
  _generator = std::default_random_engine(rd());

  initMusic();
  initPlayerSFX();
  initEnemySFX();

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

  auto music_main_sound = _assets->get<cugl::Sound>("music-main");
  auto music_boss_sound = _assets->get<cugl::Sound>("music-boss");

  _music_main_vol = music_main_sound->getVolume();
  _music_boss_vol = music_boss_sound->getVolume();

  _music_main = cugl::audio::AudioFader::alloc(music_main_sound->createNode());
  _music_boss = cugl::audio::AudioFader::alloc(music_boss_sound->createNode());

  if (_music_state == MusicState::MAIN) {
    _music_main->setGain(_music_main_vol);
    _music_boss->setGain(0);
  } else if (_music_state == MusicState::BOSS) {
    _music_boss->setGain(_music_boss_vol);
    _music_main->setGain(0);
  }

  _music_mixer->attach(0, _music_main);
  _music_mixer->attach(1, _music_boss);

  q->play(_music_mixer, true, 1.0f, 1.0f);
}

void SoundController::switchMusic() {
  if (_music_main == nullptr || _music_boss == nullptr) return;

  if (_music_state == MusicState::MAIN) {
    _music_boss->setGain(_music_boss_vol);
    _music_boss->setPosition((Uint32)_music_main->getPosition());
    _music_boss->fadeIn(2 /* seconds */);
    _music_main->fadeOut(4 /* seconds */, true);
    _music_state = MusicState::BOSS;
  } else if (_music_state == MusicState::BOSS) {
    _music_main->setGain(_music_main_vol);
    _music_main->setPosition((Uint32)_music_boss->getPosition());
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
  for (int i = 1; i <= 2; i++) {
    std::string name = "player-attack-swing-" + std::to_string(i);
    _player_swing.push_back(SFX(name, _assets->get<cugl::Sound>(name)));
  }

  _player_hit =
      SFX("player-attack-hit", _assets->get<cugl::Sound>("player-attack-hit"));

  _player_energy_wave =
      SFX("player-attack-energy-wave",
          _assets->get<cugl::Sound>("player-attack-energy-wave"));

  for (int i = 1; i <= 4; i++) {  // 4 grass footsteps
    std::string name = "player-footsteps-grass-" + std::to_string(i);
    _player_footsteps_grass.push_back(
        SFX(name, _assets->get<cugl::Sound>(name)));
  }

  for (int i = 1; i <= 4; i++) {  // 4 stone footsteps
    std::string name = "player-footsteps-stone-" + std::to_string(i);
    _player_footsteps_stone.push_back(
        SFX(name, _assets->get<cugl::Sound>(name)));
  }
}

void SoundController::playPlayerSwing() {
  auto sfx = pickRandom(_player_swing);
  cugl::AudioEngine::get()->play(sfx.name, sfx.sound, false, sfx.volume, true);
}

void SoundController::playPlayerHit() {
  cugl::AudioEngine::get()->play(_player_hit.name, _player_hit.sound, false,
                                 _player_hit.volume, true);
}

void SoundController::playPlayerEnergyWave() {
  cugl::AudioEngine::get()->play(_player_energy_wave.name,
                                 _player_energy_wave.sound, false,
                                 _player_energy_wave.volume, true);
}

void SoundController::playPlayerFootstep(const FootstepType& type) {
  SFX sfx;
  if (type == FootstepType::GRASS) {
    sfx = pickRandom(_player_footsteps_grass);
  } else if (type == FootstepType::STONE) {
    sfx = pickRandom(_player_footsteps_stone);
  }
  cugl::AudioEngine::get()->play(sfx.name, sfx.sound, false, sfx.volume, true);
}

void SoundController::initEnemySFX() {
  _enemy_small_gunshot =
      SFX("enemy-shotgunner-small-gunshot",
          _assets->get<cugl::Sound>("enemy-shotgunner-small-gunshot"));

  _enemy_large_gunshot =
      SFX("enemy-shotgunner-large-gunshot",
          _assets->get<cugl::Sound>("enemy-shotgunner-large-gunshot"));

  _enemy_hit = SFX("enemy-hit", _assets->get<cugl::Sound>("enemy-hit"));

  for (int i = 1; i <= 2; i++) {  // 2 enemy swing effects
    std::string name = "enemy-swing-" + std::to_string(i);
    _enemy_swing.push_back(SFX(name, _assets->get<cugl::Sound>(name)));
  }
}

void SoundController::playEnemySmallGunshot() {
  cugl::AudioEngine::get()->play(_enemy_small_gunshot.name,
                                 _enemy_small_gunshot.sound, false,
                                 _enemy_small_gunshot.volume, true);
}

void SoundController::playEnemyLargeGunshot() {
  cugl::AudioEngine::get()->play(_enemy_large_gunshot.name,
                                 _enemy_large_gunshot.sound, false,
                                 _enemy_large_gunshot.volume, true);
}

void SoundController::playEnemySwing() {
  auto sfx = pickRandom(_enemy_swing);
  cugl::AudioEngine::get()->play(sfx.name, sfx.sound, false, sfx.volume, true);
}

void SoundController::playEnemyHit() {
  cugl::AudioEngine::get()->play(_enemy_hit.name, _enemy_hit.sound, false,
                                 _enemy_hit.volume, true);
}
