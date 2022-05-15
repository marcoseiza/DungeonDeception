#ifndef SCENES_LOADING_SCENE_H_
#define SCENES_LOADING_SCENE_H_
#include <cugl/cugl.h>

/**
 * This class is a simple loading screen for asychronous asset loading.
 *
 * The screen will display a very minimal progress bar that displays the
 * status of the asset manager.  Make sure that all asychronous load requests
 * are issued BEFORE calling update for the first time, or else this screen
 * will think that asset loading is complete.
 *
 * Once asset loading is completed, it will display a play button.  Clicking
 * this button will inform the application root to switch to the gameplay mode.
 */
class LoadingScene : public cugl::Scene2 {
 protected:
  /** The asset manager for loading. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** A reference to the loading text for animation. */
  std::shared_ptr<cugl::scene2::Label> _loading_text;

  /** Num of dots currently appended to "loading..." on the screen. */
  int _num_of_dots;
  /** The frame cooldown for loading text animation. */
  int _loading_text_frame_cooldown;

  /** The player sprites that moves accross the screen. */
  std::vector<std::shared_ptr<cugl::scene2::SpriteNode>> _players;

  /** The frame cooldown for player animation. */
  int _frame_cooldown;

  /** The dimensions of the screen. */
  cugl::Size _dim;

  /** The progress displayed on the screen */
  float _progress;
  /** Whether or not the player has pressed play to continue */
  bool _completed;

 public:
  LoadingScene()
      : cugl::Scene2(),
        _progress(0.0f),
        _completed(false),
        _frame_cooldown(0),
        _loading_text_frame_cooldown(0),
        _num_of_dots(1) {}

  /**
   * Disposes of all resources allocated to this mode.
   */
  ~LoadingScene() { dispose(); }

  /**
   * Disposes of all (non-static) resources allocated to this mode.
   */
  void dispose();

  /**
   * Initializes the controller contents, making it ready for loading
   *
   * The constructor does not allocate any objects or memory.  This allows
   * us to have a non-pointer reference to this controller, reducing our
   * memory allocation.  Instead, allocation happens in this method.
   *
   * @param assets    The (loaded) assets for this game mode
   *
   * @return true if the controller is initialized properly, false otherwise.
   */
  bool init(const std::shared_ptr<cugl::AssetManager> &assets);

#pragma mark -
#pragma mark Progress Monitoring
  /**
   * The method called to update the game mode.
   *
   * This method updates the progress bar amount.
   *
   * @param timestep  The amount of time (in seconds) since the last frame
   */
  void update(float timestep);
};

#endif /* SCENES_LOADING_SCENE_H_ */
