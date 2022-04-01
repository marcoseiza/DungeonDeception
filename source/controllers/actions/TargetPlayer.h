#ifndef CONTROLLERS_ACTIONS_TARGET_PLAYER_H_
#define CONTROLLERS_ACTIONS_TARGET_PLAYER_H_
#include <cugl/cugl.h>

#include "Action.h"

/**
 * This class is an implementation of Action.
 *
 * This class provides betrayer targeting capabilities for the user.
 *
 * As with all Actions attach to InputController by calling allocating using
 * alloc and calling getHook(). This is very similar to Walker White's loader
 * system.
 */
class TargetPlayer : public Action {
 protected:
  /* Reference to map button for registering listeners to press event. */
  std::shared_ptr<cugl::scene2::Button> _button;

  /* Button was previously down on the last tick. */
  bool _prev_down;
  /* Button is currently down on the current tick. */
  bool _curr_down;
  /* Scene2 button is pressed. */
  bool _butt_down;

  /* The duration before the selected player is targeted. */
  int _target_player_hang_frames;

  /* The counter for the target player duration. */
  int _target_player_counter;
  
  /* The player being targeted. -1 if no target. */
  int _target_player_id;
  
  /* Players that have already been targeted */
  std::unordered_set<int> _dirty_players;

  /* Key for all the input listeners, for disposal. */
  Uint32 _listener_key;

 public:
  /**
   * Creates input listeners and sets default variable.
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return If initialized correctly.
   */
  virtual bool init(const std::shared_ptr<cugl::AssetManager> &assets,
                    cugl::Rect bounds) override;

  /**
   * Updates action state.
   * @return If updated correctly.
   */
  virtual bool update() override;

  /**
   * Disposes input listeners and default variables.
   * @return If disposed correctly.
   */
  virtual bool dispose() override;

  /** Reset all the internal input values. */
  virtual void reset() override {
    _prev_down = false;
    _curr_down = false;
    _butt_down = false;
    _target_player_counter = 0;
    _target_player_id = -1;
    _dirty_players.clear();
  }

  /** Pause all input. */
  virtual void pause() override { _button->deactivate(); }

  /** Resume all input. */
  virtual void resume() override { _button->activate(); }

  /**
   * This method allocates TargetPlayer and initializes it.
   *
   * @param assets The loaded assets for this game mode.
   * @param bounds The scene2 game bounds.
   * @return A newly allocated TargetPlayer action.
   */
  static std::shared_ptr<TargetPlayer> alloc(
      const std::shared_ptr<cugl::AssetManager> &assets, cugl::Rect bounds) {
    std::shared_ptr<TargetPlayer> result = std::make_shared<TargetPlayer>();
    return (result->init(assets, bounds) ? result : nullptr);
  }

  /**
   * @return If the player pressed the target button.
   */
  bool didChangeTarget() const { return _prev_down && !_curr_down; }
  
  /**
   * @return If the targeting ability has already seen and skipped the player id.
   */
  bool hasSeenPlayerId(int player_id) { return _dirty_players.find(player_id) != _dirty_players.end(); }
  
  /**
   * Clears all the dirty players so they can be iterated through again
   */
  void clearDirtyPlayers() { _dirty_players.clear(); }
  
  /**
   * Sets the target of the betrayer action to a specific player.
   * @param player_id The player to target.
   */
  void setTarget(int player_id) {
    if (player_id != -1) {
      _dirty_players.insert(player_id);
    }
    _target_player_id = player_id;
    
  }
  
  
  
  /**
   * Returns the target player id of the betrayer action.
   * @return The target player id of the betrayer action
   */
  int getTarget() { return _target_player_id; }

  /**
   * Toggles the activation of the map button. When deactivated, the button
   * cannot be pressed.
   * @param value The activation state.
   */
  void setActive(bool value);
  
#ifdef CU_TOUCH_SCREEN

  /** Touch listener for when the player moves their finger. */
  void touchMoved(const cugl::TouchEvent &event, const cugl::Vec2 &previous,
                  bool focus);

#endif  // CU_TOUCH_SCREEN

  TargetPlayer();
  ~TargetPlayer() {}
};

#endif /* CONTROLLERS_ACTIONS_TARGET_PLAYER_H_ */
