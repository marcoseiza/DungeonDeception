#include "LevelGenerator.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomma"
#include "Delaunator.h"
#pragma GCC diagnostic pop

#include <cugl/cugl.h>

#include "../models/level_gen/DefaultRooms.h"
#include "../models/level_gen/RoomTypes.h"

namespace level_gen {

LevelGenerator::LevelGenerator() : _active(false), _generator_step(nullptr) {}

void LevelGenerator::init(LevelGeneratorConfig &config,
                          const std::shared_ptr<cugl::scene2::SceneNode> &map) {
  if (_active) return;
  _active = true;

  _config = config;
  _map = map;
  _generator_step = [this]() { this->generateRooms(); };
  std::random_device my_random_device;
  unsigned seed = my_random_device();
  _generator = std::default_random_engine(seed);
}

void LevelGenerator::init(LevelGeneratorConfig &config,
                          const std::shared_ptr<cugl::scene2::SceneNode> &map,
                          Uint64 seed) {
  if (_active) return;
  _active = true;

  _config = config;
  _map = map;

  cugl::PolyFactory poly_factory;
  auto circle_poly =
      poly_factory.makeCircle(cugl::Vec2::ZERO, _config.getMapRadius());
  auto circle = cugl::scene2::PolygonNode::allocWithPoly(circle_poly);
  circle->setAnchor(cugl::Vec2::ANCHOR_CENTER);
  circle->setPosition(cugl::Vec2::ZERO);
  circle->setColor(cugl::Color4(0, 0, 0, 20));
  _map->addChild(circle);
  _generator_step = [this]() { this->generateRooms(); };
  _generator = std::default_random_engine(seed);
}

void LevelGenerator::dispose() {
  if (!_active) return;
  _active = false;

  _rooms.clear();

  _circle_rooms.clear();

  _spawn_room = nullptr;
  _map = nullptr;
  _generator_step = nullptr;
}

bool LevelGenerator::update() {
  if (_generator_step != nullptr) {
    _generator_step();
    return true;  // Not done.
  }
  return false;  // Done!
}

void LevelGenerator::generateRooms() {
  _spawn_room = std::make_shared<Room>(default_rooms::kSpawn);
  _spawn_room->_type = RoomType::SPAWN;
  _spawn_room->_fixed = true;
  _spawn_room->_node->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
  _spawn_room->_node->setPosition(_spawn_room->_node->getContentSize() / -2.0f);
  _spawn_room->_node->setColor(_spawn_room->getRoomNodeColor());
  _rooms.push_back(_spawn_room);

  for (int i = 0; i < _config._num_circles; i++) {
    _circle_rooms.push_back(std::vector<std::shared_ptr<Room>>{});
  }

  _circle_rooms[0].push_back(_spawn_room);

  _map->addChild(_spawn_room->_node);

  float min_radius = _spawn_room->getRadius();

  placeRegularRooms(_config.getNumRooms(), min_radius, _config._spawn_radius);

  _generator_step = [this]() {
    this->separateRooms([this]() { this->placeTerminals(); });
  };
}

void LevelGenerator::placeRegularRooms(int num_rooms, float min_radius,
                                       float max_radius) {
  //  Distribution to define the positioning of rooms.
  std::uniform_real_distribution<float> dis(0.0f, 1.0f);

  for (float i = 0; i < num_rooms; i++) {
    int max_room_ii = static_cast<int>(default_rooms::kRegularRooms.size() - 1);
    std::uniform_int_distribution<> room_iid(0, max_room_ii);

    default_rooms::RoomConfig chosen_room =
        default_rooms::kRegularRooms[room_iid(_generator)];
    std::shared_ptr<Room> room = std::make_shared<Room>(chosen_room);

    float room_radius = room->getRadius();
    float min_r = min_radius + room_radius;
    float max_r = max_radius - room_radius;

    float r = dis(_generator) * (max_r - min_r) + min_r;
    float angle = dis(_generator) * 2 * M_PI;
    cugl::Vec2 pos(r * cosf(angle), r * sinf(angle));
    pos -= room->_node->getContentSize() / 2.0f;
    pos.x = floorf(pos.x);
    pos.y = floorf(pos.y);

    room->_node->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
    room->_node->setPosition(pos);
    room->_node->setColor(room->getRoomNodeColor());

    _rooms.push_back(room);
    _map->addChild(room->_node);
  }
}

void LevelGenerator::separateRooms(
    std::function<void(void)> next_generator_step) {
  if (!anyRoomsOverlapping()) {
    _generator_step = next_generator_step;
    return;
  }
  for (int i = 0; i < _rooms.size(); i++) {
    std::shared_ptr<Room> &room = _rooms[i];
    cugl::Rect room_rect = room->getRect();

    for (int j = i + 1; j < _rooms.size(); j++) {
      std::shared_ptr<Room> &n_room = _rooms[j];
      cugl::Rect n_room_rect = n_room->getRect();

      if (room_rect.doesIntersect(n_room_rect)) {
        cugl::Vec2 direction = room->getMid() - n_room->getMid();
        if (direction == cugl::Vec2::ZERO) {
          direction += cugl::Vec2::ONE;
        }
        direction.normalize();
        room->move(direction);
        n_room->move(direction * -1.0f);
      }
    }
  }
}

bool LevelGenerator::anyRoomsOverlapping() {
  for (int i = 0; i < _rooms.size(); i++) {
    cugl::Rect room_rect = _rooms[i]->getRect();
    for (int j = i + 1; j < _rooms.size(); j++) {
      if (_rooms[i] != _rooms[j]) {
        cugl::Rect n_room_rect = _rooms[j]->getRect();

        if (room_rect.doesIntersect(n_room_rect)) {
          return true;
        }
      }
    }
  }
  return false;
}

std::shared_ptr<Room> LevelGenerator::roomMostOverlappingWith(
    const std::shared_ptr<Room> &room) {
  cugl::Rect room_rect = room->getRect();

  auto it = std::max_element(
      _rooms.begin(), _rooms.end(),
      [room, room_rect](const std::shared_ptr<Room> &l,
                        const std::shared_ptr<Room> &r) {
        cugl::Size l_size = l->getRect().intersect(room_rect).size;
        cugl::Size r_size = r->getRect().intersect(room_rect).size;
        if (l == room || r == room) return false;
        return l_size.width * l_size.height < r_size.width * r_size.height;
      });

  return (it != _rooms.end()) ? (*it) : room;
}

void LevelGenerator::placeTerminals() {
  float min_radius = _config._circle_radius[0] * 0.4f;

  for (int i = 0; i < _config._num_circles; i++) {
    if (i > 0) min_radius = _config._circle_radius[i - 1];
    placeTerminalRooms(_circle_rooms[i], _config._circle_num_terminals[i],
                       min_radius, _config._circle_radius[i]);
  }

  _generator_step = [this]() {
    this->separateRooms([this]() { this->segregateLayers(); });
  };
}

void LevelGenerator::placeTerminalRooms(
    std::vector<std::shared_ptr<Room>> &circle, int num_rooms, float min_radius,
    float max_radius) {
  std::uniform_real_distribution<float> dis(0.0f, 1.0f);

  // Make sure the room is always inside of the spawn circle;
  float terminal_radius =
      ((cugl::Vec2)default_rooms::kTerminal.size).length() / 2.0f;
  min_radius += terminal_radius;
  max_radius -= terminal_radius;

  float r = dis(_generator) * (max_radius - min_radius) + min_radius;
  float min_angle = dis(_generator) * 2 * M_PI;
  float max_angle = min_angle + 2 * M_3_PI_4 / num_rooms;

  for (float i = 0; i < num_rooms; i++) {
    std::shared_ptr<Room> room =
        std::make_shared<Room>(default_rooms::kTerminal);

    room->_type = RoomType::TERMINAL;

    float angle = dis(_generator) * (max_angle - min_angle) + min_angle;
    cugl::Vec2 pos(r * cosf(angle), r * sinf(angle));
    pos -= room->_node->getContentSize() / 2.0f;
    pos.x = floorf(pos.x);
    pos.y = floorf(pos.y);

    room->_node->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
    room->_node->setPosition(pos);
    room->_node->setColor(room->getRoomNodeColor());

    min_angle += 2 * M_PI / num_rooms;
    max_angle += 2 * M_PI / num_rooms;

    _rooms.push_back(room);
    circle.push_back(room);
    _map->addChild(room->_node);

    std::shared_ptr<Room> overlapping = roomMostOverlappingWith(room);
    if (overlapping != room) {
      auto it = std::find(_rooms.begin(), _rooms.end(), overlapping);
      if (it != _rooms.end() && (*it)->_type != RoomType::SPAWN) {
        _map->removeChild(overlapping->_node);
        _rooms.erase(it);
      }
    }
  }
}

void LevelGenerator::segregateLayers() {
  float min_radius = 0;
  float max_radius = 0;

  for (int i = 0; i < _config._num_circles; i++) {
    if (i > 0) {
      min_radius = _config._circle_radius[i - 1];
    }
    max_radius = _config._circle_radius[i];

    for (std::shared_ptr<Room> room : _rooms) {
      bool add = (room->_type == RoomType::STANDARD);
      add &= room->getMid().length() > min_radius;
      if (i < _config._num_circles - 1) {
        add &= room->getMid().length() <= max_radius;
      }

      if (add) _circle_rooms[i].push_back(room);
    }
  }

  // TODO: make this a config val.
  float factor = 1.2f;

  for (int i = 0; i < _config._num_circles; i++) {
    for (std::shared_ptr<Room> room : _circle_rooms[i]) {
      cugl::Vec2 pos = room->getMid() * factor;

      if (i > 0) {
        pos += room->getMid().getNormalization() *
               _config.getSeparationBetweenLayers() * factor;
      }

      pos -= room->_node->getSize() / 2.0f;

      room->_node->setPosition(roundf(pos.x), roundf(pos.y));
    }
  }

  _generator_step = [this]() {
    this->separateRooms([this]() { this->markAndFillHallways(); });
  };
}

void LevelGenerator::markAndFillHallways() {
  float min_radius = 0.0f;
  for (int i = 0; i < _config._num_circles; i++) {
    if (i > 0) {
      min_radius = _config._circle_radius[i - 1];
    }

    calculateDelaunayTriangles(_circle_rooms[i], min_radius);
    calculateMinimumSpanningTree(_circle_rooms[i]);
    addEdgesBackAndRemoveUnecessary(_circle_rooms[i]);
  }

  for (int i = 0; i < _config._num_circles; i++) {
    if (i < _config._num_circles - 1) {
      connectLayers(_circle_rooms[i], _circle_rooms[i + 1],
                    _config._circle_num_out_edges[i]);
    }
  }

  fillHallways();

  _generator_step = [this]() { this->establishGates(); };
}

void LevelGenerator::establishGates() { _generator_step = nullptr; }

void LevelGenerator::calculateDelaunayTriangles(
    std::vector<std::shared_ptr<Room>> &rooms, float min_r) {
  if (rooms.size() == 0) return;

  std::vector<double> coords;

  for (std::shared_ptr<Room> &room : rooms) {
    cugl::Vec2 mid_point = room->getMid();

    coords.push_back(static_cast<double>(mid_point.x));
    coords.push_back(static_cast<double>(mid_point.y));
  }

  delaunator::Delaunator d(coords);

  for (std::size_t i = 0; i < d.triangles.size(); i++) {
    if (i < d.halfedges[i]) {
      std::shared_ptr<Room> &node_0 = rooms[d.triangles[i]];
      std::size_t next_node_ii = (i % 3 == 2) ? i - 2 : i + 1;
      std::shared_ptr<Room> &node_1 = rooms[d.triangles[next_node_ii]];

      std::shared_ptr<Edge> edge_0_1 = std::make_shared<Edge>(node_0, node_1);

      if (min_r == 0.0f || !edge_0_1->doesIntersect(cugl::Vec2::ZERO, min_r)) {
        _map->addChild(edge_0_1->_node);
        node_0->addEdge(edge_0_1);
        node_1->addEdge(edge_0_1);
      }
    }
  }

  _map->doLayout();
}

void LevelGenerator::calculateMinimumSpanningTree(
    std::vector<std::shared_ptr<Room>> &rooms) {
  int num_edges = 0;
  std::vector<std::shared_ptr<Edge>> result;
  // Reset tree to start Prim's algorithm.
  for (std::shared_ptr<Room> &room : rooms) {
    room->_visited = false;
    for (std::shared_ptr<Edge> edge : room->_edges) {
      edge->_node->setVisible(false);
      edge->_active = false;
    }
  }
  rooms[0]->_visited = true;

  while (num_edges < rooms.size() - 1) {
    std::shared_ptr<Edge> min_edge = nullptr;
    std::shared_ptr<Room> min_neighbor = nullptr;
    float min_weight = FLT_MAX;

    for (std::shared_ptr<Room> &room : rooms) {
      if (room->_visited) {
        for (std::shared_ptr<Edge> edge : room->_edges) {
          if (!edge->getOther(room)->_visited && edge->_weight < min_weight) {
            min_edge = edge;
            min_neighbor = edge->getOther(room);
            min_weight = edge->_weight;
          }
        }
      }
    }

    num_edges++;

    if (min_edge) {
      min_neighbor->_visited = true;
      result.push_back(std::move(min_edge));
    }
  }

  for (std::shared_ptr<Edge> &edge : result) {
    edge->_node->setColor(cugl::Color4(255, 14, 14, 124));
    edge->_node->setVisible(true);
    edge->_active = true;
  }
}

void LevelGenerator::addEdgesBackAndRemoveUnecessary(
    std::vector<std::shared_ptr<Room>> &rooms) {
  std::uniform_real_distribution<float> rand(0.0f, 1.0f);
  for (std::shared_ptr<Room> &room : rooms) {
    for (std::shared_ptr<Edge> edge : room->_edges) {
      bool add_back = edge->_weight < _config.getMaxHallwayLength();

      long num_edges_source = std::count_if(
          edge->_source->_edges.begin(), edge->_source->_edges.end(),
          [](const std::shared_ptr<Edge> &edge) { return edge->_active; });
      long num_edges_neighbor = std::count_if(
          edge->_neighbor->_edges.begin(), edge->_neighbor->_edges.end(),
          [](const std::shared_ptr<Edge> &edge) { return edge->_active; });

      add_back &= num_edges_source < _config.getMaxNumEdges();
      add_back &= num_edges_neighbor < _config.getMaxNumEdges();

      add_back &= rand(_generator) <= _config.getAddEdgesBackProb();
      if (!edge->_active && add_back) {
        edge->_node->setVisible(true);
        edge->_active = true;
        edge->_node->setColor(cugl::Color4(255, 14, 14, 124));
      }
    }
  }
}

void LevelGenerator::connectLayers(std::vector<std::shared_ptr<Room>> &layer_a,
                                   std::vector<std::shared_ptr<Room>> &layer_b,
                                   int num_connections) {
  std::vector<std::shared_ptr<Edge>> connections;

  std::uniform_real_distribution<float> dis(0.0f, 1.0f);
  float min_angle = dis(_generator) * 2 * M_PI;
  float max_angle = fmod(min_angle + M_PI / num_connections, 2 * M_PI);

  for (int i = 0; i < num_connections; i++) {
    std::shared_ptr<Edge> winner;
    float winner_dist = FLT_MAX;

    for (std::shared_ptr<Room> &a_room : layer_a) {
      float angle = a_room->getMid().getAngle();
      angle += (angle < 0.0f) ? (2 * M_PI) : 0.0f;

      bool between_angles = (max_angle > min_angle)
                                ? (angle >= min_angle && angle <= max_angle)
                                : (angle >= min_angle || angle <= max_angle);

      if (between_angles && a_room->_type == RoomType::STANDARD) {
        for (std::shared_ptr<Room> &b_room : layer_b) {
          if (b_room->_type == RoomType::STANDARD) {
            // Find if this edge already been chosen.
            auto curr = std::make_shared<Edge>(a_room, b_room);
            auto res =
                std::find_if(connections.begin(), connections.end(),
                             [curr](const std::shared_ptr<Edge> &edge) {
                               return *edge == *curr || edge->shareRoom(curr);
                             });

            long a_num_edges =
                std::count_if(a_room->_edges.begin(), a_room->_edges.end(),
                              [](const std::shared_ptr<Edge> &edge) {
                                return edge->_active;
                              });
            long b_num_edges =
                std::count_if(b_room->_edges.begin(), b_room->_edges.end(),
                              [](const std::shared_ptr<Edge> &edge) {
                                return edge->_active;
                              });

            bool under_limit = a_num_edges < _config.getMaxNumEdges();
            under_limit &= b_num_edges < _config.getMaxNumEdges();

            if (curr->_weight < winner_dist && under_limit &&
                res == connections.end()) {
              winner_dist = curr->_weight;
              winner = std::move(curr);
            }
          }
        }
      }
    }

    if (winner) {
      connections.push_back(winner);
      min_angle = fmod(min_angle + 2 * M_PI / num_connections, 2 * M_PI);
      max_angle = fmod(min_angle + M_PI / num_connections, 2 * M_PI);
    }
  }

  for (std::shared_ptr<Edge> &connection : connections) {
    connection->_active = true;
    connection->_source->addEdge(connection);
    connection->_neighbor->addEdge(connection);
    connection->_node->setColor(cugl::Color4(15, 15, 230, 147));
    _map->addChild(connection->_node);
  }
  _map->doLayout();
}

void LevelGenerator::fillHallways() {
  // Resset state of all edges.
  for (std::shared_ptr<Room> &room : _rooms) {
    for (std::shared_ptr<Edge> &edge : room->_edges) {
      edge->_calculated = false;
    }
  }

  for (std::shared_ptr<Room> &room : _rooms) {
    for (int i = 0; i < room->_edges.size(); i++) {
      std::shared_ptr<Edge> &edge = room->_edges[i];

      if (!edge->_calculated && edge->_active) {
        edge->_calculated = true;

        std::shared_ptr<Room> &source = edge->_source;
        std::shared_ptr<Room> &neighbor = edge->_neighbor;

        cugl::Vec2 start =
            source->_node->getPosition() + source->getDoorForEdge(edge);
        cugl::Vec2 end =
            neighbor->_node->getPosition() + neighbor->getDoorForEdge(edge);
        cugl::Vec2 start_pos(std::min(start.x, end.x),
                             std::min(start.y, end.y));
        cugl::Vec2 end_pos(std::max(start.x, end.x) + 1,
                           std::max(start.y, end.y) + 1);

        std::vector<cugl::Vec2> path{start, end};
        auto bounds = cugl::scene2::PathNode::allocWithVertices(path, 0.4f);

        bounds->setColor(cugl::Color4::BLACK);
        bounds->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
        bounds->setPosition(start_pos);

        //        _map->addChild(bounds);
      }
    }
  }

  _map->doLayout();
}

}  // namespace level_gen
