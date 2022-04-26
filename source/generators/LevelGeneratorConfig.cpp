#include "LevelGeneratorConfig.h"

namespace level_gen {

LevelGeneratorConfig::LevelGeneratorConfig()
    : _map_radius(55),
      _spawn_radius(35),
      _hallway_radius(1),
      _num_rooms(25),
      _separation_between_layers(12),
      _expansion_factor_rooms(1.2f),
      _max_hallway_length(40),
      _add_edges_back_prob(0.2f),
      _max_num_of_edges(4) {
  addLayer(Layer(1.0f, _map_radius, 5, 0));
}

}  // namespace level_gen