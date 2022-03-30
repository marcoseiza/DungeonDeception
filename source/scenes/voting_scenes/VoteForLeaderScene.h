#ifndef SCENES_VOTING_SCENES_VOTE_FOR_LEADER_SCENE_H_
#define SCENES_VOTING_SCENES_VOTE_FOR_LEADER_SCENE_H_

#include <cugl/cugl.h>

#include "../../controllers/VotingInfo.h"
#include "../../models/Player.h"

class VoteForLeaderScene {
  // The voting info for this terminal.
  std::shared_ptr<VotingInfo> _voting_info;

  /** If the scene has been initialized */
  bool _initialized;

  /** If the scene is currently active. */
  bool _active;

  /** If the scene is done. */
  bool _done;

  /** A reference to the game assets. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** A reference to the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;

 public:
  VoteForLeaderScene() : _active(false), _done(false), _initialized(false) {}
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
    _node->setVisible(false);
  }

  /**
   * Start this VoteForLeaderScene
   */
  void start(std::shared_ptr<VotingInfo> voting_info);

  /** Update the wait for players scene. */
  void update();

  /** Return the node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> getNode() { return _node; }

  /** If the scene is currently active. */
  bool isActive() { return _active; }

  /** If the scene is done. */
  bool isDone() { return _done; }
};

#endif  // SCENES_VOTING_SCENES_VOTE_FOR_LEADER_SCENE_H_