#ifndef SCENES_PAUSE_SCENES_PAUSE_SCENE_H_
#define SCENES_PAUSE_SCENES_PAUSE_SCENE_H_

#include <cugl/cugl.h>

class PauseScene {
 protected:
  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** The root node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;

  std::shared_ptr<cugl::scene2::Button> _leave_button;

  bool _confirming_leave;
  bool _leave_button_pressed;

  bool _should_leave;

  std::shared_ptr<cugl::scene2::Button> _leave_button_yes;
  std::shared_ptr<cugl::scene2::Button> _leave_button_no;

  std::shared_ptr<cugl::scene2::Label> _leave_button_label;

  std::shared_ptr<cugl::scene2::Label> _leave_prompt_label;

 public:
  PauseScene() {}

  ~PauseScene() { dispose(); }

  void dispose();

  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  static std::shared_ptr<PauseScene> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto res = std::make_shared<PauseScene>();
    return (res->init(assets)) ? res : nullptr;
  }

  void update();

  std::shared_ptr<cugl::scene2::SceneNode> getNode() const { return _node; }

  void leaveButtonListener(const std::string& name, bool down);

  void leaveButtonConfirmListener(bool leave);

  bool shouldLeave() { return _should_leave; }
};

#endif  // SCENES_PAUSE_SCENES_PAUSE_SCENE_H_