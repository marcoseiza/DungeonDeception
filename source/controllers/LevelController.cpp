#include "LevelController.h"

#include <cugl/cugl.h>
#include <cugl/render/CUTextAlignment.h>

#include "../generators/LevelGenerator.h"
#include "../generators/LevelGeneratorConfig.h"
#include "../models/RoomModel.h"
#include "../models/tiles/Door.h"
#include "../models/tiles/TileHelper.h"
#include "../models/tiles/Wall.h"
#include "Controller.h"

#define TILE_SCALE cugl::Vec2(1, 1)
#define TILE_SIZE cugl::Vec2(48, 48)
#define ACTIVATE_MAP_COLOR cugl::Color4(56, 140, 192, 200)
#define CORRUPT_MAP_COLOR cugl::Color4(172, 50, 50, 200)

bool LevelController::init(
    const std::shared_ptr<cugl::AssetManager> &assets,
    const std::shared_ptr<cugl::scene2::SceneNode> &world_node,
    const std::shared_ptr<cugl::scene2::SceneNode> &debug_node,
    const std::shared_ptr<level_gen::LevelGenerator> &level_gen,
    const std::shared_ptr<cugl::scene2::SceneNode> &map, bool is_betrayer) {
  _assets = assets;
  _world_node = world_node;
  _debug_node = debug_node;
  _map = map;
  _level_gen = level_gen;
  _next_enemy_id = 0;
  populate();
  setupMap(is_betrayer);

  return true;
}

void LevelController::update(float timestep) {
  // Update all the player's ordering in the scene graph.
  for (std::shared_ptr<Player> &player : _player_controller->getPlayerList()) {
    std::shared_ptr<RoomModel> room =
        _level_model->getRoom(player->getRoomId());

    b2Body *body = player->getBody();
    if (body != nullptr) {
      float rel_player_y =
          body->GetPosition().y - room->getNode()->getPosition().y;
      float row = rel_player_y / (TILE_SIZE.y * TILE_SCALE.y);
      player->getPlayerNode()->setPriority(room->getGridSize().height - row);
    }
  }

  // Update the ordering of all the enemies in my player's current room.
  {
    std::shared_ptr<Player> player = _player_controller->getMyPlayer();
    std::shared_ptr<RoomModel> current =
        _level_model->getRoom(player->getRoomId());
    for (std::shared_ptr<EnemyModel> enemy : current->getEnemies()) {
      b2Body *enemy_body = enemy->getBody();
      if (enemy_body != nullptr) {
        float rel_enemy_y =
            enemy_body->GetPosition().y - current->getNode()->getPosition().y;
        float row = rel_enemy_y / (TILE_SIZE.y * TILE_SCALE.y);
        enemy->getNode()->setPriority(current->getGridSize().height - row);

        for (std::shared_ptr<Projectile> projectile : enemy->getProjectiles()) {
          if (projectile->getNode() == nullptr) {  // Not initialized yet
            continue;
          }
          float rel_projectile_y = projectile->getBody()->GetPosition().y -
                                   current->getNode()->getPosition().y;
          row = rel_projectile_y / (TILE_SIZE.y * TILE_SCALE.y);
          player->getPlayerNode()->setPriority(current->getGridSize().height -
                                               row);
        }
      }
    }
  }

  for (auto it : _level_model->getRooms()) {
    std::shared_ptr<RoomModel> room = it.second;
    auto num_players = room->getMapNode()->getChildByName("num_of_players");
    num_players->setVisible(false);

    auto counter = std::dynamic_pointer_cast<cugl::scene2::Label>(
        num_players->getChildByName("counter"));
    counter->setVisible(false);
    counter->setText("0");

    num_players->getChildByName("asterisk")->setVisible(false);

    if (room->getType() == RoomType::TERMINAL) {
      if (room->getEnergy() >= room->getEnergyToActivate()) {
        room->getMapNode()->setColor(ACTIVATE_MAP_COLOR);
      } else if (room->getCorruptedEnergy() >=
                 room->getCorruptedEnergyToActivate()) {
        room->getMapNode()->setColor(CORRUPT_MAP_COLOR);
      }
    }
  }

  if (_player_controller->getMyPlayer()->isBetrayer()) {
    for (auto it : _player_controller->getPlayers()) {
      std::shared_ptr<Player> player = it.second;
      auto room = _level_model->getRoom(player->getRoomId());

      auto num_players = room->getMapNode()->getChildByName("num_of_players");
      num_players->setVisible(true);

      auto counter = std::dynamic_pointer_cast<cugl::scene2::Label>(
          num_players->getChildByName("counter"));
      counter->setVisible(true);
      int num = std::stoi(counter->getText());
      counter->setText(std::to_string(num + 1));

      auto asterisk = num_players->getChildByName("asterisk");
      if (player->isBetrayer()) {
        asterisk->setVisible(true);
      }
    }
  }
}

std::shared_ptr<EnemyModel> LevelController::getEnemy(int enemy_id) {
  for (auto it : _level_model->getRooms()) {
    std::shared_ptr<RoomModel> room = it.second;

    for (std::shared_ptr<EnemyModel> enemy : room->getEnemies()) {
      if (enemy->getEnemyId() == enemy_id) {
        return enemy;
      }
    }
  }
  return nullptr;
}

void LevelController::dispose() {
  _level_gen->dispose();
  _next_enemy_id = 0;
}

void LevelController::changeRoom(std::string &door_sensor_name) {
  std::shared_ptr<RoomModel> current = _level_model->getCurrentRoom();

  int destination_room_id =
      current->getRoomIdFromDoorSensorId(door_sensor_name);
  if (destination_room_id == -1) return;

  cugl::Vec2 door_pos = current->getPosOfDestinationDoor(door_sensor_name);

  updateMapCurrentRoom(destination_room_id);

  if (_room_on_chopping_block != nullptr)
    _room_on_chopping_block->setVisible(false);
  _room_on_chopping_block = current;

  _level_model->setCurrentRoom(destination_room_id);

  std::shared_ptr<RoomModel> new_current =
      _level_model->getCurrentRoom();  // New current level.

  new_current->setVisible(true);

  _player_controller->getMyPlayer()->setPosPromise(
      new_current->getNode()->getPosition() +
      door_pos * (TILE_SIZE * TILE_SCALE));
}

void LevelController::moveToCenterOfRoom(int destination_room_id) {
  std::shared_ptr<RoomModel> current = _level_model->getCurrentRoom();

  updateMapCurrentRoom(destination_room_id);

  if (_room_on_chopping_block != nullptr)
    _room_on_chopping_block->setVisible(false);
  _room_on_chopping_block = current;

  _level_model->setCurrentRoom(destination_room_id);
  std::shared_ptr<RoomModel> new_current =
      _level_model->getCurrentRoom();  // New current level.

  new_current->setVisible(true);

  _player_controller->getMyPlayer()->setPosPromise(
      new_current->getNode()->getPosition() +
      new_current->getGridSize().width / 2 * (TILE_SIZE * TILE_SCALE));
}

void LevelController::updateMapCurrentRoom(int room_id) {
  // Update the map SceneNodes.
  for (std::shared_ptr<level_gen::Room> &room : _level_gen->getRooms()) {
    if (room->_key == _level_model->getCurrentRoom()->getKey()) {
      room->_node->getChildByName("border")->setVisible(false);
    }
    if (room->_key == room_id) {
      room->_node->setColor(room->getRoomNodeColor());
      room->_node->setVisible(true);
      room->_node->getChildByName("border")->setVisible(true);
      room->_node->getChildByName("type_decoration")->setVisible(true);

      for (std::shared_ptr<level_gen::Edge> edge : room->_edges) {
        if (edge->_active) {
          edge->_node->setVisible(true);

          auto other_room = edge->getOther(room);
          if (!other_room->_node->isVisible()) {
            cugl::Color4 color = other_room->getRoomNodeColor();
            color.a = 100;
            other_room->_node->setColor(color);
            other_room->_node->setVisible(true);
          }
        }
      }
    }
  }
}

void LevelController::populate() {
  _level_model = LevelModel::alloc();

  instantiateWorld();

  // Initialize every room.
  for (std::shared_ptr<level_gen::Room> room : _level_gen->getRooms()) {
    auto room_node = room->_level_node;

    room_node->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
    cugl::Vec2 pos = room->getRect().origin * (TILE_SIZE * TILE_SCALE);
    room_node->setPosition(pos);
    room_node->setVisible(false);
    room_node->doLayout();

    auto room_model = RoomModel::alloc(room_node, room->_node, room->_key);
    room_model->setType(room->_type);
    if (room->_type == RoomType::TERMINAL) {
      room_model->setNumPlayersRequired(room->_num_players_for_terminal);
    }
    _level_model->addRoom(room->_key, room_model);

    // Make spawn the starting point.
    if (room->_type == RoomType::SPAWN) {
      _level_model->setCurrentRoom(room->_key);
      _level_model->setSpawnRoom(room_model);
      room_node->setVisible(true);
    }

    std::vector<cugl::Vec2> unused_doors = instantiateDoors(room, room_model);

    coverUnusedDoors(room, room_model, unused_doors);

    std::vector<std::shared_ptr<EnemyModel>> enemies;

    instantiateEnemies(room, room_model, enemies);

    room_model->setEnemies(enemies);

    _world_node->addChild(room_node);
  }
}

void LevelController::setupMap(bool is_betrayer) {
  if (_map == nullptr) return;

  for (std::shared_ptr<level_gen::Room> &room : _level_gen->getRooms()) {
    room->_node->setColor(room->getRoomNodeColor());
    // Make all the edges invisible unless you are a betrayer.
    for (std::shared_ptr<level_gen::Edge> &edge : room->_edges) {
      edge->_node->setVisible(is_betrayer);
      edge->_node->setColor(cugl::Color4(255, 255, 255, 200));
      if (!edge->_active) {
        auto parent = edge->_node->getParent();
        if (parent) parent->removeChild(edge->_node);
      }
    }
    // Make all the rooms invisible unless a betrayer.
    room->_node->setVisible(is_betrayer);

    // Create a node for the white border when in the room.
    auto border = cugl::scene2::PathNode::allocWithRect(
        room->_node->getBoundingBox(), 1.5f, cugl::poly2::Joint::MITRE,
        cugl::poly2::EndCap::SQUARE);
    border->setRelativeColor(false);
    border->setColor(cugl::Color4(255, 255, 255, 200));
    border->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
    border->setPosition(cugl::Vec2::ZERO);
    border->setName("border");
    border->setVisible(false);
    room->_node->addChild(border);

    cugl::PolyFactory poly_factory;

    // Create a node to encapsulate the decorations for the type, to hide from
    // players when the room isn't explored.
    auto type_decoration = cugl::scene2::SceneNode::alloc();
    type_decoration->setName("type_decoration");
    type_decoration->setContentSize(room->_node->getContentSize());
    type_decoration->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
    type_decoration->setPosition(cugl::Vec2::ZERO);
    room->_node->addChild(type_decoration);
    type_decoration->setVisible(is_betrayer);

    // This is a scale that is used to make the PolyFactory create circles with
    // a higher definition. The node is then scaled back to 1/scale to return it
    // to normal.
    float higher_def_scale = 20.0f;
    if (room->_type == RoomType::TERMINAL) {
      // Create one large circle in the center.
      float circle_radius = 4.0f;
      auto circle_poly = poly_factory.makeCircle(
          cugl::Vec2::ZERO, circle_radius * higher_def_scale);
      auto circle = cugl::scene2::PolygonNode::allocWithPoly(circle_poly);
      circle->setAnchor(cugl::Vec2::ANCHOR_CENTER);
      circle->setPosition(room->_node->getBoundingBox().size / 2);
      circle->setRelativeColor(false);
      circle->setColor(cugl::Color4(255, 255, 255, 200));
      circle->setScale(1 / higher_def_scale);
      type_decoration->addChild(circle);
    } else if (room->_type == RoomType::SPAWN) {
      // Create four circles spaced evenly in a larger circle.
      int number_of_circles = 4;
      float radius = 3.0f;
      float circle_radius = 1.5f;
      for (int i = 0; i < number_of_circles; i++) {
        cugl::PolyFactory poly_factory;
        auto circle_poly = poly_factory.makeCircle(
            cugl::Vec2::ZERO, circle_radius * higher_def_scale);
        auto circle = cugl::scene2::PolygonNode::allocWithPoly(circle_poly);
        circle->setAnchor(cugl::Vec2::ANCHOR_CENTER);
        float angle = i * (2 * M_PI / number_of_circles);
        cugl::Vec2 pos(radius * cosf(angle), radius * sinf(angle));
        pos += room->_node->getBoundingBox().size / 2;
        circle->setPosition(pos);
        circle->setRelativeColor(false);
        circle->setColor(cugl::Color4(255, 255, 255, 200));
        circle->setScale(1 / higher_def_scale);
        type_decoration->addChild(circle);
      }
    }

    // Create the counter for the number of players.
    auto rounded_rect =
        poly_factory.makeRoundedRect(cugl::Vec2::ZERO, cugl::Vec2(9, 11), 1);
    auto num_of_players =
        cugl::scene2::PolygonNode::allocWithPoly(rounded_rect);
    num_of_players->setName("num_of_players");
    num_of_players->setAnchor(cugl::Vec2::ANCHOR_TOP_RIGHT);
    num_of_players->setPosition((cugl::Vec2)room->_node->getContentSize() +
                                cugl::Vec2(3.0f, 3.0f));
    num_of_players->setRelativeColor(false);
    num_of_players->setColor(cugl::Color4(10, 10, 10, 150));
    auto counter = cugl::scene2::Label::allocWithText(
        "0", _assets->get<cugl::Font>("pixelmix_small_bold"));
    counter->setName("counter");
    counter->setScale(0.3f);
    counter->setAnchor(cugl::Vec2::ANCHOR_CENTER);
    counter->setHorizontalAlignment(HorizontalAlign::CENTER);
    counter->setVerticalAlignment(VerticalAlign::MIDDLE);
    counter->setPosition(num_of_players->getContentWidth() / 2 + 0.4f,
                         num_of_players->getContentHeight() / 2 + 0.5f);
    counter->setRelativeColor(false);
    counter->setForeground(cugl::Color4::WHITE);
    counter->setVisible(false);
    num_of_players->addChild(counter);

    auto asterisk_poly =
        poly_factory.makeCircle(cugl::Vec2::ZERO, 2.5f * 20.0f);
    auto asterisk = cugl::scene2::PolygonNode::allocWithPoly(asterisk_poly);
    asterisk->setName("asterisk");
    asterisk->setAnchor(cugl::Vec2::ANCHOR_CENTER);
    asterisk->setPosition(num_of_players->getContentSize());
    asterisk->setRelativeColor(false);
    asterisk->setColor(cugl::Color4(172, 50, 50));
    asterisk->setScale(1 / 20.0f);
    asterisk->setVisible(false);

    num_of_players->addChild(asterisk);

    room->_node->addChild(num_of_players);
  }

  // Add the map looking background to the node.
  auto map_bkg = cugl::scene2::PolygonNode::allocWithTexture(
      _assets->get<cugl::Texture>("map-background"));

  cugl::Vec2 world_start = _world->getBounds().origin;
  cugl::Vec2 world_end = _world->getBounds().size + world_start;
  world_start /= (TILE_SIZE * TILE_SCALE);
  world_end /= (TILE_SIZE * TILE_SCALE);

  cugl::Size layout_size(
      2 * std::max(std::abs(world_start.x), std::abs(world_end.x)),
      2 * std::max(std::abs(world_start.y), std::abs(world_end.y)));

  // Make it a square
  if (layout_size.width > layout_size.height) {
    layout_size.height = layout_size.width;
  } else {
    layout_size.width = layout_size.height;
  }
  // Add a little padding so it doesn't look weird.
  layout_size += cugl::Vec2(30, 30);

  map_bkg->setScale(layout_size / map_bkg->getContentSize());
  map_bkg->setAnchor(cugl::Vec2::ANCHOR_CENTER);
  map_bkg->setPosition(cugl::Vec2::ZERO);
  map_bkg->setPriority(-1);

  _map->swapChild(_map->getChildByName("background"), map_bkg);

  auto asterisk_description = cugl::scene2::Label::allocWithText(
      "= betrayer in room", _assets->get<cugl::Font>("pixelmix_small_bold"));
  asterisk_description->setName("asterisk_description");
  asterisk_description->setScale(0.4f);
  asterisk_description->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_RIGHT);
  asterisk_description->setHorizontalAlignment(HorizontalAlign::HARD_RIGHT);
  asterisk_description->setVerticalAlignment(VerticalAlign::HARD_BOTTOM);
  asterisk_description->setPosition(map_bkg->getContentWidth() - 10, 10);
  asterisk_description->setRelativeColor(false);
  asterisk_description->setForeground(cugl::Color4::WHITE);
  asterisk_description->setDropShadow(cugl::Vec2(0, -1));
  asterisk_description->setVisible(is_betrayer);
  map_bkg->addChild(asterisk_description);

  cugl::PolyFactory poly_factory;
  auto asterisk_poly = poly_factory.makeCircle(cugl::Vec2::ZERO, 4.0f * 20.0f);
  auto asterisk_icon = cugl::scene2::PolygonNode::allocWithPoly(asterisk_poly);
  asterisk_icon->setName("asterisk_icon");
  asterisk_icon->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_RIGHT);
  cugl::Vec2 asterisk_icon_pos = asterisk_description->getPosition();
  asterisk_icon_pos.x -= asterisk_description->getWidth() + 3;
  asterisk_icon->setPosition(asterisk_icon_pos);
  asterisk_icon->setRelativeColor(false);
  asterisk_icon->setColor(cugl::Color4(172, 50, 50));
  asterisk_icon->setScale(1 / 20.0f);
  asterisk_icon->setVisible(is_betrayer);
  map_bkg->addChild(asterisk_icon);

  // Show the spawn room (first room) as the current room.
  updateMapCurrentRoom(_level_gen->getSpawnRoom()->_key);
}

void LevelController::instantiateWorld() {
  // Represent the two corners of the world.
  cugl::Vec2 world_start;
  cugl::Vec2 world_end;

  // Get Size of World.
  for (std::shared_ptr<level_gen::Room> room : _level_gen->getRooms()) {
    cugl::Vec2 pos = room->getRect().origin;
    cugl::Vec2 size = ((cugl::Vec2)room->getRect().size);
    if (pos.x < world_start.x) world_start.x = pos.x;
    if (pos.y < world_start.y) world_start.y = pos.y;
    if (pos.x + size.x > world_end.x) world_end.x = pos.x + size.x;
    if (pos.y + size.y > world_end.y) world_end.y = pos.y + size.y;
  }

  world_start *= (TILE_SIZE * TILE_SCALE);
  world_end *= (TILE_SIZE * TILE_SCALE);

  _world = cugl::physics2::ObstacleWorld::alloc(
      cugl::Rect(world_start, world_end - world_start));
}

std::vector<cugl::Vec2> LevelController::instantiateDoors(
    const std::shared_ptr<level_gen::Room> &room,
    const std::shared_ptr<RoomModel> &room_model) {
  std::vector<cugl::Vec2> unused_doors = room->_doors;

  // Initialize box2d in only the used doors.
  for (auto &it : room->_edge_to_door) {
    std::shared_ptr<level_gen::Edge> edge = it.first;
    cugl::Vec2 door = it.second;
    unused_doors.erase(
        std::remove(unused_doors.begin(), unused_doors.end(), door),
        unused_doors.end());

    int y = (int)door.y, x = (int)door.x;
    std::stringstream ss;
    ss << "tile-(" << x << "-" << y << ")";

    auto door_room_node = TileHelper::getChildByNameRecursively<Door>(
        room_model->getNode(), {"tiles", ss.str(), "tile"});

    if (door_room_node) {
      door_room_node->initDelegates();

      if (x == 0 || y == 0) {
        door_room_node->setNegative();
      } else if (x == room_model->getGridSize().width - 1 ||
                 y == room_model->getGridSize().height - 1) {
        door_room_node->setPositive();
      }
      std::stringstream door_sensor_name;
      door_sensor_name << "room-" << room->_key << "-door-" << ss.str();
      _world->addObstacle(door_room_node->initBox2d(door_sensor_name.str()));

      std::shared_ptr<level_gen::Room> other_room = edge->getOther(room);
      cugl::Vec2 destination = other_room->_edge_to_door[edge];

      if (destination.x == 0) destination.x += 2;
      if (destination.y == 0) destination.y += 2;
      if (destination.x == room_model->getGridSize().width - 1)
        destination.x -= 2;
      if (destination.y == room_model->getGridSize().height - 1)
        destination.y -= 2;

      if (other_room->_key != -1) {
        room_model->addConnection(door_sensor_name.str(), other_room->_key,
                                  destination);
      }
    }
  }

  return unused_doors;
}

void LevelController::coverUnusedDoors(
    const std::shared_ptr<level_gen::Room> &room,
    const std::shared_ptr<RoomModel> &room_model,
    std::vector<cugl::Vec2> &unused_doors) {
  for (cugl::Vec2 door : unused_doors) {
    int y = (int)door.y, x = (int)door.x;
    std::stringstream ss;
    ss << "tile-(" << x << "-" << y << ")";

    auto door_room_node = TileHelper::getChildByNameRecursively<Door>(
        room_model->getNode(), {"tiles", ss.str(), "tile"});

    if (door_room_node) {
      door_room_node->initDelegates();
      door_room_node->setState(Door::State::UNUSED);

      if (x == 0 || y == 0) {
        door_room_node->setNegative();
      } else if (x == room_model->getGridSize().width - 1 ||
                 y == room_model->getGridSize().height - 1) {
        door_room_node->setPositive();
      }
    }
  }
}

void LevelController::instantiateEnemies(
    const std::shared_ptr<level_gen::Room> &room,
    const std::shared_ptr<RoomModel> &room_model,
    std::vector<std::shared_ptr<EnemyModel>> &enemies) {
  // Initialize enemies in room.
  for (std::shared_ptr<cugl::scene2::SceneNode> enemy_node :
       room_model->getNode()->getChildByName("enemies")->getChildren()) {
    enemy_node = enemy_node->getChildByName("enemy");

    std::string enemy_type = enemy_node->getType();
    auto enemy_texture = _assets->get<cugl::Texture>(enemy_type);
    auto enemy = EnemyModel::alloc(enemy_node->getWorldPosition(),
                                   enemy_node->getName(), enemy_type);

    enemy->setEnemyId(_next_enemy_id++);
    enemy->setRoomId(room_model->getKey());
    enemies.push_back(enemy);

    enemy->setNode(enemy_texture, _debug_node);

    enemy->setRoomPos(room_model->getNode()->getPosition());
    room_model->getNode()->addChild(enemy->getNode());
    _world->addObstacle(enemy);

    enemy->setDebugScene(_debug_node);
    enemy->setDebugColor(cugl::Color4(cugl::Color4::BLACK));
  }
}
