#include "RoomModel.h"

#include "tiles/Terminal.h"
#include "tiles/TileHelper.h"

#define SPACE_BETWEEN_BAR_AND_TERMINAL 10
#define SPACE_BETWEEN_BARS 5

bool RoomModel::init(const std::shared_ptr<cugl::scene2::SceneNode>& node,
                     const std::shared_ptr<cugl::scene2::SceneNode>& map_node,
                     int key) {
  _node = node;
  _map_node = map_node;
  auto tiles = node->getChildByName("tiles");
  auto grid_layout =
      std::dynamic_pointer_cast<cugl::scene2::GridLayout>(tiles->getLayout());
  _grid_size = grid_layout->getGridSize();
  _key = key;

  return true;
}

void RoomModel::setEnergyBars(
    const std::shared_ptr<cugl::scene2::ProgressBar>& reg_bar,
    const std::shared_ptr<cugl::scene2::ProgressBar>& cor_bar) {
  if (reg_bar == nullptr || cor_bar == nullptr) return;

  _reg_bar = reg_bar;
  _cor_bar = cor_bar;

  _reg_bar->setAnchor(cugl::Vec2::ANCHOR_CENTER);
  _cor_bar->setAnchor(cugl::Vec2::ANCHOR_CENTER);

  auto terminals = TileHelper::getTile<Terminal>(_node);
  if (terminals.size() != 1) return;
  auto terminal = terminals[0];

  float offset_top_bar = _cor_bar->getContentHeight() + SPACE_BETWEEN_BARS;
  _reg_bar->setPositionY(terminal->getContentHeight() * 0.75f +
                         SPACE_BETWEEN_BAR_AND_TERMINAL + offset_top_bar);
  _cor_bar->setPositionY(terminal->getContentHeight() * 0.75f +
                         SPACE_BETWEEN_BAR_AND_TERMINAL);

  _reg_bar->setPositionX(terminal->getContentWidth() / 2);
  _cor_bar->setPositionX(terminal->getContentWidth() / 2);

  _reg_bar->setPriority(std::numeric_limits<float>::max());
  for (auto child : _reg_bar->getChildren()) {
    child->setPriority(std::numeric_limits<float>::max());
  }
  _cor_bar->setPriority(std::numeric_limits<float>::max());
  for (auto child : _cor_bar->getChildren()) {
    child->setPriority(std::numeric_limits<float>::max());
  }

  terminal->addChild(_reg_bar);
  terminal->addChild(_cor_bar);
}

void RoomModel::dispose() { _node = nullptr; }

void RoomModel::setEnergy(int energy) {
  _energy = energy;
  if (_reg_bar != nullptr) _reg_bar->setProgress(_energy / 100.0f);

  if (_energy >= _energy_to_activate) {
    auto terminals = TileHelper::getTile<Terminal>(_node);
    if (terminals.size() != 1) return;
    auto terminal = terminals[0];
    terminal->setActivated();
  }
}

void RoomModel::setCorruptedEnergy(int energy) {
  _corrupted_energy = energy;
  if (_cor_bar != nullptr) _cor_bar->setProgress(_corrupted_energy / 100.0f);

  if (_corrupted_energy >= _corrupted_energy_to_activate) {
    auto terminals = TileHelper::getTile<Terminal>(_node);
    if (terminals.size() != 1) return;
    auto terminal = terminals[0];
    terminal->setCorrupted();
  }
}