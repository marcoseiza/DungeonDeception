#ifndef SCENES_VOTING_SCENES_VOTE_FOR_LEADER_SCENE_H_
#define SCENES_VOTING_SCENES_VOTE_FOR_LEADER_SCENE_H_

#include <cugl/cugl.h>

#include "../../controllers/PlayerController.h"
#include "../../controllers/VotingInfo.h"
#include "../../models/Player.h"

class VoteForLeaderScene {
  // The voting info for this terminal.
  std::shared_ptr<VotingInfo> _voting_info;

  /** The room id for this terminal. */
  int _terminal_room_id;

  /** If the scene has been initialized */
  bool _initialized;

  /** If the scene is currently active. */
  bool _active;

  /** If the players can press the ready button. */
  bool _can_finish;

  /** If the player has clicked ready. */
  bool _has_clicked_ready;

  /** If the scene is done. */
  bool _done;

  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** A reference to the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;

  /** A map from the player id to the button it corresponds to. */
  std::unordered_map<int, std::shared_ptr<cugl::scene2::Button>> _buttons;

  /** A reference to the done button used when finished voting. */
  std::shared_ptr<cugl::scene2::Button> _ready_button;

  /** A reference to the player controller. */
  std::shared_ptr<PlayerController> _player_controller;

  /** The leader. */
  int _winner;

 public:
  VoteForLeaderScene()
      : _active(false),
        _done(false),
        _can_finish(false),
        _has_clicked_ready(false),
        _initialized(false),
        _winner(0) {}
  ~VoteForLeaderScene() { dispose(); }

  /**
   * Initialize a wait for player scene.
   *
   * @param assets The assets for the game.
   * */
  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  /**
   * Allocate a new wait for player scene.
   *
   * @param assets The assets for the game.
   * @return A shared pointer of the initialized wait for players scene.
   */
  static std::shared_ptr<VoteForLeaderScene> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto result = std::make_shared<VoteForLeaderScene>();
    if (result->init(assets)) return result;
    return nullptr;
  }

  /** Dispose of this VoteForLeaderScene. */
  void dispose() {
    _active = false;
    _done = false;
    _has_clicked_ready = false;
    _can_finish = false;
    _winner = 0;
    for (auto& it : _buttons) {
      it.second->clearListeners();
    }
    _ready_button->clearListeners();
    _node->setVisible(false);
  }

  /**
   * Start this VoteForLeaderScene
   */
  void start(std::shared_ptr<VotingInfo> voting_info, int terminal_room_id);

  /** Update the wait for players scene. */
  void update();

  /** Return the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> getNode() { return _node; }

  /** If the scene is currently active. */
  bool isActive() { return _active; }

  /** If the scene is done. */
  bool isDone() { return _done; }

  /** If the scene is done. */
  void setDone(bool val) { _done = val; }

  /** Voting Button listener.  */
  void voteButtonListener(const std::string& name, bool down);

  /** Done Button listener.  */
  void readyButtonListener(const std::string& name, bool down);

  void removeAllPlayersFromDoneList();

  void setPlayerController(
      const std::shared_ptr<PlayerController>& player_controller) {
    _player_controller = player_controller;
  }

  int getLeader() { return (_done) ? _winner : -1; }
};

#endif  // SCENES_VOTING_SCENES_VOTE_FOR_LEADER_SCENE_H_