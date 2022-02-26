#include "LevelGenerator.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomma"
#include "Delaunator.h"
#pragma GCC diagnostic pop

#include <cugl/cugl.h>

// static
const cugl::Size LevelGenerator::TERMINAL(10.0f, 10.0f);

// static
const cugl::Size LevelGenerator::SPAWN(10.0f, 10.0f);

LevelGenerator::LevelGenerator() : _active(false), _generator_step(nullptr) {}

void LevelGenerator::init(LevelGeneratorConfig config,
                          std::shared_ptr<cugl::scene2::SceneNode> map) {
  if (!_active) {
    _config = config;
    _active = true;
    _map = map;
    _generator_step = [this]() { this->generateRooms(); };
  }
}

void LevelGenerator::dispose() {
  if (_active) {
    _rooms.clear();
    _active = false;
    _generator_step = nullptr;
  }
}

void LevelGenerator::generateRooms() {

  std::random_device my_random_device;
  unsigned seed = my_random_device();
  std::default_random_engine gen(seed);

  auto spawn_room = std::make_shared<Room>(LevelGenerator::SPAWN);
  spawn_room->fixed = true;
  spawn_room->node->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
  spawn_room->node->setPosition(spawn_room->node->getContentSize() / -2.0f);
  spawn_room->node->setColor(cugl::Color4(14, 14, 205, 127));
  _rooms.push_back(spawn_room);
  _map->addChild(spawn_room->node);

  // Spawn terminal rooms for inner circle with separation.
  float inner_circle_min_r =
      static_cast<cugl::Vec2>(spawn_room->getRect().size).length() / 2.0f;
  placeTerminalRooms(_config.getNumTerminalRoomsInner(), inner_circle_min_r,
                     _config.getInnerCircleRadius(), gen);

  // Spawn terminal rooms for middle circle with separation.
  placeTerminalRooms(_config.getNumTerminalRoomsMiddle(),
                     _config.getInnerCircleRadius(),
                     _config.getMiddleCircleRadius(), gen);

  // Spawn terminal rooms for outer circle with separation.
  placeTerminalRooms(_config.getNumTerminalRoomsOuter(),
                     _config.getMiddleCircleRadius(), _config.getMapRadius(),
                     gen);

  int num_rooms_inside =
      floor(_config.getNumRooms() * _config.getInnerCircleFrac());
  placeRegularRooms(num_rooms_inside, 0, _config.getInnerCircleRadius(), gen);

  _generator_step = [this]() { this->LevelGenerator::separateRooms(); };
}

void LevelGenerator::placeRegularRooms(int num_rooms, float min_radius,
                                       float max_radius,
                                       std::default_random_engine &generator) {
  //  Distribution to define the size of normal rooms.
  std::normal_distribution<float> size_dis(9.0f, 1.0f);
  std::uniform_real_distribution<float> dis(0.0f, 1.0f);

  for (float i = 0; i < num_rooms; i++) {
    auto room = std::make_shared<Room>(floor(size_dis(generator)),
                                       floor(size_dis(generator)));

    float room_radius =
        static_cast<cugl::Vec2>(room->getRect().size).length() / 2.0f;
    float min_r = min_radius + room_radius;
    float max_r = max_radius - room_radius;

    float r = dis(generator) * (max_r - min_r) + min_r;
    float angle = dis(generator) * 2 * M_PI;
    cugl::Vec2 pos(floor(r * cosf(angle)), floor(r * sinf(angle)));

    room->node->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
    room->node->setPosition(pos - (room->node->getContentSize() / 2.0f));
    room->node->setColor(cugl::Color4(125, 94, 52, 127));

    _rooms.push_back(room);
    _map->addChild(room->node);
  }
}

void LevelGenerator::placeTerminalRooms(int num_rooms, float min_radius,
                                        float max_radius,
                                        std::default_random_engine &generator) {

  std::uniform_real_distribution<float> dis(0.0f, 1.0f);

  // Make sure the room is always inside of the spawn circle;
  float terminal_radius =
      static_cast<cugl::Vec2>(LevelGenerator::TERMINAL).length() / 2.0f;
  min_radius += terminal_radius;
  max_radius -= terminal_radius;

  float r = dis(generator) * (max_radius - min_radius) + min_radius;
  float min_angle = dis(generator) * 2 * M_PI;
  float max_angle = min_angle + M_PI / 2.0f;

  for (float i = 0; i < num_rooms; i++) {
    auto room = std::make_shared<Room>(LevelGenerator::TERMINAL);
    room->fixed = true;
    room->node->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);

    float angle = dis(generator) * (max_angle - min_angle) + min_angle;
    cugl::Vec2 pos(floor(r * cosf(angle)), floor(r * sinf(angle)));

    room->node->setPosition(pos - (room->node->getContentSize() / 2.0f));
    room->node->setColor(cugl::Color4(52, 205, 14, 127));

    min_angle += 2 * M_PI / num_rooms;
    max_angle += 2 * M_PI / num_rooms;

    _rooms.push_back(room);
    _map->addChild(room->node);
  }
}

void LevelGenerator::separateRooms() {
  if (!anyRoomsOverlapping()) {
    _generator_step = [this]() { this->markAndFillHallways(); };
    return;
  }
  for (int i = 0; i < _rooms.size(); i++) {
    auto room = _rooms[i];
    cugl::Rect room_rect = room->getRect();

    if (!room->fixed) {
      for (int j = i + 1; j < _rooms.size(); j++) {
        auto n_room = _rooms[j];
        cugl::Rect n_room_rect = n_room->getRect();

        if (room_rect.doesIntersect(n_room_rect)) {

          cugl::Vec2 direction = room->getMid() - n_room->getMid();

          direction.normalize();
          room->move(direction);
          n_room->move(direction * -1.0f);
        }
      }
    }
  }
}

bool LevelGenerator::anyRoomsOverlapping() {
  for (int i = 0; i < _rooms.size(); i++) {
    cugl::Rect room_rect = _rooms[i]->getRect();
    for (int j = i + 1; j < _rooms.size(); j++) {
      cugl::Rect n_room_rect = _rooms[j]->getRect();

      if (room_rect.doesIntersect(n_room_rect)) {
        return true;
      }
    }
  }
  return false;
}

void LevelGenerator::markAndFillHallways() {
  calculateDelaunayTriangles();
  calculateMinimumSpanningTree();
  addEdgesBack();
  fillHallways();

  // _generator_step = [this]() { this->buildCompositeAreas(); };
  _generator_step = nullptr;
}

void LevelGenerator::buildCompositeAreas() {
  _generator_step = [this]() { this->establishGates(); };
}

void LevelGenerator::establishGates() { _generator_step = nullptr; }

void LevelGenerator::calculateDelaunayTriangles() {
  std::vector<double> coords;

  for (std::shared_ptr<Room> room : _rooms) {
    cugl::Vec2 mid_point = room->getMid();

    coords.push_back(static_cast<double>(mid_point.x));
    coords.push_back(static_cast<double>(mid_point.y));
  }

  delaunator::Delaunator d(coords);

  for (std::size_t i = 0; i < d.triangles.size(); i += 3) {
    std::shared_ptr<Room> node_0 = _rooms[d.triangles[i]];
    std::shared_ptr<Room> node_1 = _rooms[d.triangles[i + 1]];
    std::shared_ptr<Room> node_2 = _rooms[d.triangles[i + 2]];

    auto edge_0_1 = std::make_shared<Edge>(node_0, node_1);
    auto edge_0_2 = std::make_shared<Edge>(node_0, node_2);
    auto edge_1_2 = std::make_shared<Edge>(node_1, node_2);

    _map->addChild(edge_0_1->path);
    _map->addChild(edge_0_2->path);
    _map->addChild(edge_1_2->path);

    node_0->edges.push_back(edge_0_1);
    node_0->edges.push_back(edge_0_2);

    node_1->edges.push_back(edge_0_1);
    node_1->edges.push_back(edge_1_2);

    node_2->edges.push_back(edge_0_2);
    node_2->edges.push_back(edge_1_2);
  }

  _map->doLayout();
}

void LevelGenerator::calculateMinimumSpanningTree() {
  int num_edges = 0;
  std::vector<std::shared_ptr<Edge>> result;
  // Reset tree to start Prim's algorithm.
  for (std::shared_ptr<Room> room : _rooms) {
    room->visited = false;
  }
  _rooms[0]->visited = true;

  while (num_edges < _rooms.size() - 1) {
    std::shared_ptr<Edge> min_edge;

    for (std::shared_ptr<Room> room : _rooms) {
      if (room->visited) {
        for (std::shared_ptr<Edge> edge : room->edges) {
          if (!edge->neighbor->visited) {
            min_edge = edge;
          }
        }
      }
    }

    num_edges++;

    if (min_edge) {
      min_edge->neighbor->visited = true;
      result.push_back(min_edge);
    }
  }

  for (std::shared_ptr<Room> room : _rooms) {
    for (std::shared_ptr<Edge> edge : room->edges) {
      edge->path->setVisible(false);
    }
  }

  for (std::shared_ptr<Edge> edge : result) {
    edge->path->setColor(cugl::Color4(255, 14, 14, 124));
    edge->path->setVisible(true);
  }
}

void LevelGenerator::addEdgesBack() {}

void LevelGenerator::fillHallways() {}