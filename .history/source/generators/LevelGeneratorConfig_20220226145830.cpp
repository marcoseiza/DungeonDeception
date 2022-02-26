#include "LevelGeneratorConfig.h"

LevelGeneratorConfig::LevelGeneratorConfig()
    : _inner_circle_frac(0.4f), _middle_circle_frac(0.7f), _hallway_radius(1),
      _num_rooms(50), _num_separation_iterations(50) {
  setMapRadius(80);
  setNumTerminalRooms(7);
}