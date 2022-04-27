#ifndef CONTROLLERS_SOUND_CONTROLLER_H_
#define CONTROLLERS_SOUND_CONTROLLER_H_

#include <cugl/cugl.h>

#include "Controller.h"

class SoundController : public Controller {
 private:
  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** A generator for random numbers. */
  std::default_random_engine _generator;

  /** A class to represent a sound effect. */
  struct SFX {
    /** A unique name for the sound effect. */
    std::string name;
    /** The sound node for the sound effect. */
    std::shared_ptr<cugl::Sound> sound;
    /** Create a new SFX with the given name and node. */
    SFX(std::string name, const std::shared_ptr<cugl::Sound>& sound)
        : name(name), sound(sound) {}
  };

  //
  //
  //
#pragma mark MusicVariables
  /** The mixer for music layering. */
  std::shared_ptr<cugl::audio::AudioMixer> _music_mixer;
  /** The fader for the main music. */
  std::shared_ptr<cugl::audio::AudioFader> _music_main;
  /** The fader for the boss music. */
  std::shared_ptr<cugl::audio::AudioFader> _music_boss;
  /** An enumeration for the music state. */
  enum MusicState {
    /** The main music is playing. */
    MAIN,
    /** The boss music is playing. */
    BOSS
  };
  /** The music state. */
  MusicState _music_state;
#pragma mark -
  //
  //
  //

  //
  //
  //
#pragma mark PlayerSFXVariables
  /** A list of all the player swing SFX. */
  std::vector<SFX> _player_swing;
  /** A list of all the player hit SFX. */
  std::vector<SFX> _player_hit;
  /** A list of all the possible grass footstep sounds. */
  std::vector<SFX> _player_footsteps_grass;
  /** A list of all the possible stone footstep sounds. */
  std::vector<SFX> _player_footsteps_stone;
  /** An enumeration for the footstep state. */
  enum FootstepState {
    /** The grass footsteps should be playing. */
    GRASS,
    /** The stone footsteps should be playing. */
    STONE
  };
  /** The music state. */
  FootstepState _footstep_state;
#pragma mark -
  //
  //
  //

 public:
  /** Construct a new controller. */
  SoundController() {}
  /** Destroy the controller. */
  ~SoundController() { dispose(); }

  /**
   * Initialize the Sound Controller. Please use alloc().
   *
   * @param assets The game assets.
   * @return If initialized properly.
   */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Allocates and initializes a new SoundController shared_ptr.
   *
   * @param assets The game assets.
   * @return The shared pointer SoundController.
   */
  static std::shared_ptr<SoundController> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto result = std::make_shared<SoundController>();
    return (result->init(assets)) ? result : nullptr;
  }

  /** Update the controller state. */
  void update(float timestep) override;

  /** Dispose the controller and all its values. */
  void dispose() override;

  /**
   * Helper function to pick random entry in vector.
   *
   * @tparam T The value inside the vector.
   * @param vec The vector.
   */
  template <typename T>
  T pickRandom(const std::vector<T>& vec) {
    CUAssertLog(vec.size() > 0, "Vector is empty.");
    std::uniform_int_distribution<int> dist(0, vec.size() - 1);
    int ii = dist(_generator);
    return vec[ii];
  }

  //
  //
  //
#pragma mark MusicMethods
  /** Initialize all the music variables */
  void initMusic();

  /** Start the music in the current music state. */
  void startMusic();

  /** Switch what music is playing to the current music state (_music_state). */
  void switchMusic();

  /**
   * Pause the music.
   * @param fade The amount of seconds the pause should fade out for.
   */
  void pauseMusic(float fade = 0);

  /**
   * Resume playing the music. Will fade in with the same value as was given in
   * pauseMusic.
   */
  void resumeMusic();

  /**
   * Stop the music and reset all the values. startMusic should be called to
   * resume the music again.
   */
  void stopMusic();
#pragma mark -
  //
  //
  //

  //
  //
  //
#pragma mark PlayerSFXMethods
  /** Initialize all the music variables */
  void initPlayerSFX();

  /** Play a swing sound effect. */
  void playPlayerSwing();

  /** Play a swing sound effect. */
  void playPlayerHit();
#pragma mark -
  //
  //
  //
};

#endif  // CONTROLLERS_SOUND_CONTROLLER_H_