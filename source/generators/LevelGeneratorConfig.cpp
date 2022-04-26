#include "LevelGeneratorConfig.h"

namespace level_gen {

LevelGeneratorConfig::LevelGeneratorConfig()
    : _inner_circle_frac(0.4f),
      _middle_circle_frac(0.75f),
      _hallway_radius(1),
      _num_rooms(25),
      _separation_between_layers(10),
      _max_hallway_length(40),
      _add_edges_back_prob(0.2f),
      _max_num_of_edges(4) {
  setMapRadius(55);
  setNumTerminalRooms(7);

  // TODO: make a setter.
  _circle_fractions = std::vector<float>{1.0f};
  _circle_radius = std::vector<float>{_map_radius};
  _circle_num_terminals = std::vector<int>{5};
  _circle_num_out_edges = std::vector<int>{2};

  _num_circles = 1;

  _spawn_radius = 35;
}

}  // namespace level_gen