#include "TargetPlayer.h"

#define BLOCK_ANIM_LIMIT 25
#define BLOCK_ACCEPT_START 25
#define BLOCK_ACCEPT_LIMIT 46
#define BLOCK_ANIM_LENGTH 60000 /* ms (i.e. one min cooldown) */

TargetPlayer::TargetPlayer()
    : _curr_down(false),
      _prev_down(false),
      _button(nullptr),
      _is_activating_action(false),
      _target_player_hang_frames(150),
      _target_player_counter(-1),
      _target_player_id(-1),
      _prev_target_player_id(-1),
      _target_cooldown_counter(-1),
      _cooldown_finished(false) {}

bool TargetPlayer::init(const std::shared_ptr<cugl::AssetManager> &assets,
                        cugl::Rect bounds) {
  Action::init(assets, bounds);
  _right_screen_bounds = Action::_display_coord_bounds;
  _right_screen_bounds.size.width *= 0.5;
  _right_screen_bounds.origin.x += _right_screen_bounds.size.width;

  _button_node = cugl::scene2::SpriteNode::alloc(
      assets->get<cugl::Texture>("block-player"), 8, 6, BLOCK_ACCEPT_LIMIT);
  _button_node->setFrame(0);
  _button = cugl::scene2::Button::alloc(_button_node);
  _button->setPlayClickSound(false);

  _tooltip =
      assets->get<cugl::scene2::SceneNode>("ui-scene_target-player-tooltip");

  assets->get<cugl::scene2::SceneNode>("ui-scene_block-player")
      ->addChild(_button);
  _button->setAnchor(cugl::Vec2::ANCHOR_CENTER);
  _button->setPosition(_button->getParent()->getContentSize() / 2);
  _button->setScale(_button->getParent()->getScale());
  _button->getParent()->doLayout();

  _start_cooldown = false;
  _cooldown_finished = false;
  _anim_buffer = 0;

  _butt_down = false;
  _button->addListener(
      [=](const std::string &name, bool down) { _butt_down = down; });

  _button->activate();

#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
  _listener_key = touch->acquireKey();

  touch->addBeginListener(_listener_key,
                          [=](const cugl::TouchEvent &event, bool focus) {
                            this->touchBegan(event, focus);
                          });
#else
  cugl::Mouse *mouse = cugl::Input::get<cugl::Mouse>();
  _listener_key = mouse->acquireKey();

  mouse->addPressListener(_listener_key, [=](const cugl::MouseEvent &event,
                                             Uint8 clicks, bool focus) {
    this->mouseBegan(event, clicks, focus);
  });

#endif  // CU_TOUCH_SCREEN

  return true;
}

bool TargetPlayer::update() {
  _prev_down = _curr_down;
  _curr_down = _butt_down;

  if (!_prev_down && _curr_down) _tooltip->setVisible(true);

  if (_curr_down && _target_player_counter >= 0) _target_player_counter++;

  if (_start_cooldown) {
    _target_cooldown_counter--;
    cugl::Timestamp time;
    Uint64 millis = time.ellapsedMillis(_time_cooldown_start);
    int frame = _button_node->getFrame();

    if (frame < BLOCK_ANIM_LIMIT - 1) {
      float diff = std::min(1.0f, millis * 1.0f / BLOCK_ANIM_LENGTH);
      _button_node->setFrame((BLOCK_ANIM_LIMIT - 1) * diff);
    } else if (frame == BLOCK_ANIM_LIMIT - 1) {
      _start_cooldown = false;
      _cooldown_finished = true;
      _button->resetDownColor();
      _button->setDown(false, false);
    }
    _anim_buffer++;
  } else {
    _button->activate();
    _button_node->setFrame(0);

    if (isActivatingTargetAction()) {
      _is_activating_action = false;
      _target_player_counter = -1;
      _prev_target_player_id = _target_player_id;
      _target_player_id = -1;
      _start_cooldown = true;
      _tooltip->setVisible(false);
      _button->setDownColor(cugl::Color4::WHITE);
      _time_cooldown_start.mark();
      _button->deactivate();
    }

    if (didChangeTarget()) {
      _target_player_counter = 0;
      _button_node->setFrame(BLOCK_ACCEPT_START);
    }

    if (_target_player_counter > _target_player_hang_frames) {
      if (_target_player_id != -1) {
        _is_activating_action = true;
      } else {
        _target_player_counter = 0;
        _target_player_id = -1;
      }
    } else if (_target_player_counter > 0) {
      float frac = (float)_target_player_counter / _target_player_hang_frames;
      float rel_frame = (BLOCK_ACCEPT_LIMIT - 1 - BLOCK_ACCEPT_START) * frac;
      _button_node->setFrame((int)rel_frame + BLOCK_ACCEPT_START);
    }
  }
  return true;
}

bool TargetPlayer::dispose() {
  _button = nullptr;

#ifdef CU_TOUCH_SCREEN
  cugl::Touchscreen *touch = cugl::Input::get<cugl::Touchscreen>();
  touch->removeBeginListener(_listener_key);
#else
  cugl::Mouse *mouse = cugl::Input::get<cugl::Mouse>();
  mouse->removePressListener(_listener_key);

#endif  // CU_TOUCH_SCREEN
  return true;
}

void TargetPlayer::setActive(bool value) {
  if (value) {
    _button->activate();
    _button->setColor(cugl::Color4::WHITE);
  } else {
    _button->deactivate();
    _button->setColor(cugl::Color4::GRAY);
  }
}

#ifdef CU_TOUCH_SCREEN
void TargetPlayer::touchBegan(const cugl::TouchEvent &event, bool focus) {
  if (_button->containsScreen(event.position)) return;
  if (!_right_screen_bounds.contains(event.position)) return;
  _button_node->setFrame(0);
  _is_activating_action = false;
  _target_player_counter = -1;
  _target_player_id = -1;
  _tooltip->setVisible(false);
  _button->setDown(false, false);
}

#else
void TargetPlayer::mouseBegan(const cugl::MouseEvent &event, Uint8 clicks,
                              bool focus) {
  if (_button->containsScreen(event.position)) return;
  if (!_right_screen_bounds.contains(event.position)) return;
  _button_node->setFrame(0);
  _is_activating_action = false;
  _target_player_counter = -1;
  _target_player_id = -1;
  _tooltip->setVisible(false);
  _button->setDown(false, false);
}
#endif  // CU_TOUCH_SCREEN
