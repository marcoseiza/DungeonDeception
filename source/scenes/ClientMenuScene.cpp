#include "ClientMenuScene.h"

#include <cugl/cugl.h>

#include <iostream>
#include <regex>
#include <sstream>

#include "../models/tiles/TileHelper.h"

#pragma mark -
#pragma mark Level Layout

/** Regardless of logo, lock the height to this */
#define SCENE_HEIGHT 720
/** Set cloud wrap x position based on width and scale of cloud layer **/
#define CLOUD_WRAP -1689.6

#pragma mark -
#pragma mark Client Methods

bool ClientMenuScene::init(const std::shared_ptr<cugl::AssetManager>& assets) {
  // TODO factor out a lot of this common code to peer scene (if we keep the
  // scene stuff)

  // Initialize the scene to a locked width
  cugl::Size dimen = cugl::Application::get()->getDisplaySize();
  dimen *= SCENE_HEIGHT / dimen.height;
  if (assets == nullptr) {
    return false;
  } else if (!Scene2::init(dimen)) {
    return false;
  }

  // Start up the input handler
  _assets = assets;

  // Acquire the scene built by the asset loader and resize it the scene
  std::shared_ptr<cugl::scene2::SceneNode> scene =
      _assets->get<cugl::scene2::SceneNode>("client");
  scene->setContentSize(dimen);
  scene->doLayout();  // Repositions the HUD

  _startgame = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>(
          "client_center_content_info_start"));
  _backout = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>("client_back"));
  _gameid = std::dynamic_pointer_cast<cugl::scene2::Label>(
      _assets->get<cugl::scene2::SceneNode>(
          "client_center_content_info_game_field_text"));
  _clipboard = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>(
          "client_center_content_info_game_paste"));
  _paste_tooltip = _assets->get<cugl::scene2::SceneNode>(
      "client_center_content_info_game_paste_tooltip-wrapper_pasted");
  _cannot_paste_tooltip = _assets->get<cugl::scene2::SceneNode>(
      "client_center_content_info_game_paste_tooltip-wrapper_cannot-paste");
  _paste_tooltip->setVisible(false);

  _join_success_tooltip = _assets->get<cugl::scene2::SceneNode>(
      "client_center_content_info_menu-status_success");
  _join_error_tooltip = _assets->get<cugl::scene2::SceneNode>(
      "client_center_content_info_menu-status_error");
  _no_wifi_tooltip = _assets->get<cugl::scene2::SceneNode>(
      "client_center_content_info_menu-status_no-wifi");

  for (int i = 0; i <= 9; i++) {
    std::string key = "client_center_content_keys_" + to_string(i);
    std::shared_ptr<cugl::scene2::Button> button =
        std::dynamic_pointer_cast<cugl::scene2::Button>(
            _assets->get<cugl::scene2::SceneNode>(key));
    _keypad_buttons.push_back(button);
  }

  _x_button = std::dynamic_pointer_cast<cugl::scene2::Button>(
      _assets->get<cugl::scene2::SceneNode>(
          "client_center_content_keys_x_button"));

  _status = Status::IDLE;

  _paste_tooltip_lifetime = 0;
  _clipboard->addListener([this](const std::string& name, bool down) {
    if (down && SDL_HasClipboardText()) {
      const char* clip = SDL_GetClipboardText();
      std::string text(clip);
      const std::regex rgx("\\d{5}");
      if (std::regex_match(text, rgx)) {
        _gameid->setText(text);
        _paste_tooltip->setVisible(true);
        _paste_tooltip_lifetime = 0;
      } else {
        _cannot_paste_tooltip->setVisible(true);
        _paste_tooltip_lifetime = 0;
      }
    }
  });

  _backout->addListener([this](const std::string& name, bool down) {
    if (down) {
      disconnect();
      _status = Status::ABORT;
    }
  });

  _startgame->addListener([=](const std::string& name, bool down) {
    if (down) {
      connect(_gameid->getText());
    }
  });

  // add listener to each keypad button to type in code
  for (int i = 0; i < _keypad_buttons.size(); i++) {
    std::shared_ptr<cugl::scene2::Button> button = _keypad_buttons.at(i);
    button->addListener([=](const std::string& name, bool down) {
      // add text to game id when keypad pressed if code not complete
      if (down && _gameid->getText().size() < 5) {
        _gameid->setText(_gameid->getText() + to_string(i));
      }
    });
  }
  _x_button->addListener([this](const std::string& name, bool down) {
    if (down) {
      std::string curr_text = _gameid->getText();
      _gameid->setText(curr_text.substr(0, curr_text.length() - 1));
    }
  });

  // Create the server configuration
  auto json = _assets->get<cugl::JsonValue>("server");
  _config.punchthroughServerAddr = json->getString("address", "");
  _config.punchthroughServerPort = json->getInt("port", 0);
  _config.maxNumPlayers = json->getInt("maximum", 0);
  _config.apiVersion = json->getInt("version", 0);

  // handle background and cloud layers
  auto background_layer =
      assets->get<cugl::scene2::SceneNode>("background-client-menu");
  background_layer->setContentSize(dimen);
  background_layer->setPositionX(getCloudXPosition());
  background_layer->doLayout();

  _cloud_layer = assets->get<cugl::scene2::SceneNode>("clouds-client-menu");
  _cloud_layer->setContentSize(dimen);
  _cloud_layer->doLayout();

  addChild(background_layer);
  addChild(_cloud_layer);
  addChild(scene);
  setActive(false);
  return true;
}

void ClientMenuScene::dispose() {
  if (_active) {
    removeAllChildren();
    _active = false;
    _cloud_layer->dispose();
    _cloud_layer = nullptr;
  }
}

void ClientMenuScene::setActive(bool value) {
  if (isActive() != value) {
    Scene2::setActive(value);
    if (value) {
      _cloud_layer->setPositionX(_cloud_x_pos);
      _move_to_lobby = false;
      _status = IDLE;
      _backout->activate();
      _x_button->activate();
      _clipboard->activate();
      _join_success_tooltip->setVisible(false);
      _join_error_tooltip->setVisible(false);
      _no_wifi_tooltip->setVisible(false);
      _gameid->setText("");

      _paste_tooltip->setVisible(false);
      _paste_tooltip_lifetime = 0;

      for (int i = 0; i < _keypad_buttons.size(); i++) {
        std::shared_ptr<cugl::scene2::Button> button = _keypad_buttons[i];
        button->activate();
      }

      _network = nullptr;
      configureStartButton();
      // Don't reset the room id
    } else {
      _move_to_lobby = false;
      _startgame->deactivate();
      _backout->deactivate();
      _x_button->deactivate();
      _clipboard->deactivate();
      _join_success_tooltip->setVisible(false);
      _join_error_tooltip->setVisible(false);
      _no_wifi_tooltip->setVisible(false);
      _startgame->setContentSize(300, 90);
      _startgame->getParent()->doLayout();

      for (int i = 0; i < _keypad_buttons.size(); i++) {
        std::shared_ptr<cugl::scene2::Button> button = _keypad_buttons[i];
        button->deactivate();
      }

      // If any were pressed, reset them
      _startgame->setDown(false);
      _backout->setDown(false);
      _x_button->setDown(false);
      _clipboard->setDown(false);

      for (int i = 0; i < _keypad_buttons.size(); i++) {
        std::shared_ptr<cugl::scene2::Button> button = _keypad_buttons[i];
        button->setDown(false);
      }
    }
  }
}

void ClientMenuScene::updateText(
    const std::shared_ptr<cugl::scene2::Button>& button,
    const std::string text) {
  auto label = std::dynamic_pointer_cast<cugl::scene2::Label>(
      button->getChildByName("up")->getChildByName("label"));
  label->setText(text);
}

void ClientMenuScene::update(float timestep) {
  if (_network) {
    _network->receive(
        [this](const std::vector<uint8_t>& data) { processData(data); });
    checkConnection();
    // Do this last for button safety
    configureStartButton();
  }

  // update cloud background layer
  _cloud_x_pos = _cloud_x_pos + .3;
  if (_cloud_x_pos >= 0) {
    _cloud_x_pos = CLOUD_WRAP;
  }
  _cloud_layer->setPositionX(_cloud_x_pos);

  if (_paste_tooltip->isVisible() || _cannot_paste_tooltip->isVisible()) {
    _paste_tooltip_lifetime += timestep;
    if (_paste_tooltip_lifetime >= 1.0f /* seconds */) {
      _paste_tooltip->setVisible(false);
      _cannot_paste_tooltip->setVisible(false);
    }
  }
}

void ClientMenuScene::processData(const std::vector<uint8_t>& data) {
  _deserializer.receive(data);
  Sint32 code = std::get<Sint32>(_deserializer.read());
  if (code == HOST_SEND_THAT_LOBBY_IS_OPEN) _move_to_lobby = true;
};

bool ClientMenuScene::connect(const std::string room) {
  _network = cugl::NetworkConnection::alloc(_config, room);

  _join_success_tooltip->setVisible(false);
  _join_error_tooltip->setVisible(false);
  _no_wifi_tooltip->setVisible(false);

  return checkConnection();
}

bool ClientMenuScene::checkConnection() {
  switch (_network->getStatus()) {
    case cugl::NetworkConnection::NetStatus::Pending:
      _status = JOIN;
      break;
    case cugl::NetworkConnection::NetStatus::Connected:
      _status = WAIT;
      _join_success_tooltip->setVisible(true);
      break;
    case cugl::NetworkConnection::NetStatus::Reconnecting:
      _status = JOIN;
      break;
    case cugl::NetworkConnection::NetStatus::RoomNotFound:
      disconnect();
      _join_error_tooltip->setVisible(true);
      _status = IDLE;
      break;
    case cugl::NetworkConnection::NetStatus::ApiMismatch:
      disconnect();
      _status = IDLE;
      break;
    case cugl::NetworkConnection::NetStatus::GenericError:
      disconnect();
      _join_error_tooltip->setVisible(true);
      _status = IDLE;
      break;
    case cugl::NetworkConnection::NetStatus::NoInternetError:
      disconnect();
      _no_wifi_tooltip->setVisible(true);
      _status = IDLE;
      break;
    case cugl::NetworkConnection::NetStatus::Disconnected:
      disconnect();
      _status = IDLE;
      return false;
  }
  return true;
}

void ClientMenuScene::configureStartButton() {
  if (_status == Status::IDLE) {
    _startgame->activate();
    updateText(_startgame, "JOIN GAME");
  } else if (_status == Status::JOIN) {
    _startgame->deactivate();
    _startgame->setDown(false);
    updateText(_startgame, "...");
  } else if (_status == Status::WAIT) {
    _startgame->deactivate();
    _startgame->setDown(false);
    updateText(_startgame, "WAITING ON LOBBY");
    _startgame->setContentWidth(360);
    _startgame->getParent()->doLayout();
  }
}
