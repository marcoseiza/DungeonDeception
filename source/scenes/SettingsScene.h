#ifndef SCENES_SETTINGS_SCENE_H_
#define SCENES_SETTINGS_SCENE_H_

#include <cugl/cugl.h>

class SettingsScene {
 public:
  enum Choice {
    NONE,
    LEAVE,
    RESUME,
  };

 protected:
  bool _confirming_leave;

  bool _leave_button_pressed;

  Choice _choice;

  bool _active;

  /** The asset manager for this scene. */
  std::shared_ptr<cugl::AssetManager> _assets;

  /** The root node for this scene. */
  std::shared_ptr<cugl::scene2::SceneNode> _node;

  std::shared_ptr<cugl::scene2::Button> _leave_button;
  std::shared_ptr<cugl::scene2::Label> _leave_button_label;

  std::shared_ptr<cugl::scene2::Button> _leave_button_yes;
  std::shared_ptr<cugl::scene2::Button> _leave_button_no;

  std::shared_ptr<cugl::scene2::Label> _leave_prompt_label;

  std::shared_ptr<cugl::scene2::Button> _resume_button;

 public:
  SettingsScene() {}

  ~SettingsScene() { dispose(); }

  void dispose();

  bool init(const std::shared_ptr<cugl::AssetManager>& assets);

  static std::shared_ptr<SettingsScene> alloc(
      const std::shared_ptr<cugl::AssetManager>& assets) {
    auto res = std::make_shared<SettingsScene>();
    return (res->init(assets)) ? res : nullptr;
  }

  void update();

  std::shared_ptr<cugl::scene2::SceneNode> getNode() const { return _node; }

  void leaveButtonListener(const std::string& name, bool down);

  void leaveButtonConfirmListener(bool leave);

  Choice getChoice() { return _choice; }

  void setActive(bool active);

  bool isActive() { return _active; }
};

#endif  // SCENES_SETTINGS_SCENE_H_