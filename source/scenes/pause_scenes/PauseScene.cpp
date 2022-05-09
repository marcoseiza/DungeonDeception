#include "PauseScene.h"

#include "../../models/tiles/TileHelper.h"
#include "../../network/NetworkController.h"

bool PauseScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;
  _node = assets->get<cugl::scene2::SceneNode>("pause-scene");

  _confirming_leave = false;
  _leave_button_pressed = false;
  _should_leave = false;

  auto wrapper = TileHelper::getChildByNameRecursively(
      _node, {"pause-background", "pause-wrapper"});

  _leave_button = TileHelper::getChildByNameRecursively<cugl::scene2::Button>(
      wrapper, {"leave-button", "button"});
  _leave_button->setVisible(true);
  _leave_button->activate();

  _leave_button->addListener([=](const std::string& name, bool down) {
    this->leaveButtonListener(name, down);
  });

  _leave_button_yes =
      TileHelper::getChildByNameRecursively<cugl::scene2::Button>(
          wrapper, {"leave-button-yes", "button"});
  _leave_button_yes->setVisible(false);
  _leave_button_yes->activate();

  _leave_button_yes->addListener([=](const std::string& name, bool down) {
    if (down) return;
    this->leaveButtonConfirmListener(true);
  });

  _leave_button_no =
      TileHelper::getChildByNameRecursively<cugl::scene2::Button>(
          wrapper, {"leave-button-no", "button"});
  _leave_button_no->setVisible(false);
  _leave_button_no->activate();

  _leave_button_no->addListener([=](const std::string& name, bool down) {
    if (down) return;
    this->leaveButtonConfirmListener(false);
  });

  _leave_button_label =
      TileHelper::getChildByNameRecursively<cugl::scene2::Label>(
          _leave_button, {"up", "label"});

  if (NetworkController::get()->isHost()) {
    _leave_button_label->setText("END GAME", true);

    wrapper->getChildByName("leaving-prompt-client")->setVisible(false);
    _leave_prompt_label = std::dynamic_pointer_cast<cugl::scene2::Label>(
        wrapper->getChildByName("leaving-prompt-host"));
  } else {
    _leave_button_label->setText("LEAVE GAME", true);

    wrapper->getChildByName("leaving-prompt-host")->setVisible(false);
    _leave_prompt_label = std::dynamic_pointer_cast<cugl::scene2::Label>(
        wrapper->getChildByName("leaving-prompt-client"));
  }
  _leave_prompt_label->setVisible(false);

  return true;
}

void PauseScene::dispose() {
  _assets = nullptr;
  _should_leave = false;

  _leave_button->clearListeners();
  _leave_button_yes->clearListeners();
  _leave_button_no->clearListeners();
}

void PauseScene::update() {
  if (_leave_button_pressed) {
    _leave_button_pressed = false;

    _leave_button->setVisible(!_confirming_leave);
    _leave_button_no->setVisible(_confirming_leave);
    _leave_button_yes->setVisible(_confirming_leave);
    _leave_prompt_label->setVisible(_confirming_leave);

    if (_confirming_leave) {
      _leave_button->deactivate();
      _leave_button_no->activate();
      _leave_button_yes->activate();
    } else {
      _leave_button->activate();
      _leave_button_no->deactivate();
      _leave_button_yes->deactivate();
    }
  }
}

void PauseScene::leaveButtonListener(const std::string& name, bool down) {
  if (_confirming_leave || down) return;

  _confirming_leave = true;
  _leave_button_pressed = true;

  _leave_button->setVisible(false);
  _leave_button_no->setVisible(true);
  _leave_button_yes->setVisible(true);

  _leave_prompt_label->setVisible(true);
}

void PauseScene::leaveButtonConfirmListener(bool leave) {
  if (!_confirming_leave) return;

  _confirming_leave = false;
  _leave_button_pressed = true;

  if (!leave) {
    _should_leave = false;
    _leave_button->setVisible(true);
    _leave_button_no->setVisible(false);
    _leave_button_yes->setVisible(false);

    _leave_prompt_label->setVisible(false);
  } else {
    _should_leave = true;
  }
}