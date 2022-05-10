#include "SettingsScene.h"

#include "../models/tiles/TileHelper.h"
#include "../network/NetworkController.h"

#define MIN_PLAYERS 4
#define MIN_BETRAYERS 1

bool SettingsScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  _assets = assets;
  _node = assets->get<cugl::scene2::SceneNode>("settings-scene");
  _node->setVisible(false);
  _active = false;

  _confirming_leave = false;
  _leave_button_pressed = false;
  _choice = NONE;

  _node->getChildByName("grey-background")
      ->setColor(cugl::Color4(0, 0, 0, 123));

  auto wrapper = TileHelper::getChildByNameRecursively(
      _node, {"settings-background", "settings-wrapper"});

  _leave_button = TileHelper::getChildByNameRecursively<cugl::scene2::Button>(
      wrapper, {"leave-button-wrapper", "leave-button", "button"});
  _leave_button->setVisible(true);
  _leave_button->activate();

  _leave_button->addListener([=](const std::string& name, bool down) {
    this->leaveButtonListener(name, down);
  });

  _resume_button = TileHelper::getChildByNameRecursively<cugl::scene2::Button>(
      wrapper, {"row-1-wrapper", "resume-button", "button"});
  _resume_button->addListener([=](const std::string& name, bool down) {
    if (down) return;
    this->_choice = RESUME;
  });
  _resume_button->activate();

  _leave_button_yes =
      TileHelper::getChildByNameRecursively<cugl::scene2::Button>(
          wrapper, {"leave-button-wrapper", "leave-button-yes", "button"});
  _leave_button_yes->setVisible(false);
  _leave_button_yes->activate();

  _leave_button_yes->addListener([=](const std::string& name, bool down) {
    if (down) return;
    this->leaveButtonConfirmListener(true);
  });

  _leave_button_no =
      TileHelper::getChildByNameRecursively<cugl::scene2::Button>(
          wrapper, {"leave-button-wrapper", "leave-button-no", "button"});
  _leave_button_no->setVisible(false);
  _leave_button_no->activate();

  _leave_button_no->addListener([=](const std::string& name, bool down) {
    if (down) return;
    this->leaveButtonConfirmListener(false);
  });

  _leave_button_label =
      TileHelper::getChildByNameRecursively<cugl::scene2::Label>(
          _leave_button, {"up", "label"});

  _leave_prompt_label_leave =
      TileHelper::getChildByNameRecursively<cugl::scene2::Label>(
          wrapper, {"row-1-wrapper", "leaving-prompt-leave"});
  _leave_prompt_label_end =
      TileHelper::getChildByNameRecursively<cugl::scene2::Label>(
          wrapper, {"row-1-wrapper", "leaving-prompt-end"});

  _leave_prompt_label_end->setVisible(false);
  _leave_prompt_label_leave->setVisible(false);

  if (NetworkController::get()->isHost()) {
    _leave_button_label->setText("END GAME", true);
  } else {
    _leave_button_label->setText("LEAVE GAME", true);
  }

  return true;
}

void SettingsScene::dispose() {
  _assets = nullptr;
  _choice = NONE;

  _leave_button->clearListeners();
  _leave_button_yes->clearListeners();
  _leave_button_no->clearListeners();
  _resume_button->clearListeners();
}

void SettingsScene::setActive(bool active) {
  _active = active;
  _choice = NONE;
  _confirming_leave = false;
  _leave_button_pressed = false;

  if (_node == nullptr) return;
  _node->setVisible(_active);

  if (_leave_button == nullptr || _leave_button_no == nullptr ||
      _leave_button_yes == nullptr) {
    return;
  }

  if (_active) {
    _leave_button->activate();
    _leave_button_no->deactivate();
    _leave_button_yes->deactivate();
  } else {
    _leave_button->deactivate();
    _leave_button_no->deactivate();
    _leave_button_yes->deactivate();
  }
}

void SettingsScene::update() {
  if (!_active) return;

  bool end_game = (_player_controller->getPlayers().size() <= MIN_PLAYERS);
  end_game |= _player_controller->getNumberBetrayers() <= MIN_BETRAYERS &&
              _player_controller->getMyPlayer()->isBetrayer();
  if (end_game) {
  }

  if (_leave_button_pressed) {
    _leave_button_pressed = false;

    _leave_button->setVisible(!_confirming_leave);
    _resume_button->setVisible(!_confirming_leave);
    _leave_button_no->setVisible(_confirming_leave);
    _leave_button_yes->setVisible(_confirming_leave);

    if (_confirming_leave) {
      _leave_button->deactivate();
      _leave_button_no->activate();
      _leave_button_yes->activate();

      bool end_game = (_player_controller->getPlayers().size() <= MIN_PLAYERS);
      end_game |= _player_controller->getNumberBetrayers() <= MIN_BETRAYERS &&
                  _player_controller->getMyPlayer()->isBetrayer();
      end_game |= NetworkController::get()->isHost();

      _leave_prompt_label_leave->setVisible(!end_game);
      _leave_prompt_label_end->setVisible(end_game);
    } else {
      _leave_button->activate();
      _leave_button_no->deactivate();
      _leave_button_yes->deactivate();

      _leave_prompt_label_leave->setVisible(false);
      _leave_prompt_label_end->setVisible(false);
    }
  }
}

void SettingsScene::leaveButtonListener(const std::string& name, bool down) {
  if (_confirming_leave || down) return;

  _confirming_leave = true;
  _leave_button_pressed = true;
}

void SettingsScene::leaveButtonConfirmListener(bool leave) {
  if (!_confirming_leave) return;

  _confirming_leave = false;
  _leave_button_pressed = true;

  _choice = (leave) ? LEAVE : NONE;
}
