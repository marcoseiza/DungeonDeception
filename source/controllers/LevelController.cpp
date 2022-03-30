#include "LevelController.h"

#include <cugl/cugl.h>

#include "../generators/LevelGenerator.h"
#include "../generators/LevelGeneratorConfig.h"
#include "../models/RoomModel.h"
#include "../models/tiles/Door.h"
#include "../models/tiles/Wall.h"
#include "Controller.h"

#define TILE_SCALE cugl::Vec2(0.6, 0.6)
#define TILE_SIZE cugl::Vec2(64, 64)

bool LevelController::init(
    const std::shared_ptr<cugl::AssetManager> &assets,
    const std::shared_ptr<cugl::scene2::SceneNode> &world_node,
    const std::shared_ptr<cugl::scene2::SceneNode> &debug_node,
    const std::shared_ptr<level_gen::LevelGenerator> &level_gen) {
  _assets = assets;
  _world_node = world_node;
  _debug_node = debug_node;
  _level_gen = level_gen;
  populate();
  return true;
}

void LevelController::update() {
  std::shared_ptr<RoomModel> current = _level_model->getCurrentRoom();
  std::shared_ptr<Player> player = _level_model->getPlayer();
  float rel_player_y =
      player->getBody()->GetPosition().y - current->getNode()->getPosition().y;
  float row = rel_player_y / (TILE_SIZE.y * TILE_SCALE.y);
  player->getPlayerNode()->setPriority(current->getGridSize().height - row);

  for (std::shared_ptr<EnemyModel> enemy : current->getEnemies()) {
    b2Body *enemy_body = enemy->getBody();
    if (enemy_body != nullptr) {
      float rel_enemy_y =
          enemy_body->GetPosition().y - current->getNode()->getPosition().y;
      row = rel_enemy_y / (TILE_SIZE.y * TILE_SCALE.y);
      enemy->getNode()->setPriority(current->getGridSize().height - row);

      for (std::shared_ptr<Projectile> projectile : enemy->getProjectiles()) {
        float rel_projectile_y = projectile->getBody()->GetPosition().y -
                                 current->getNode()->getPosition().y;
        row = rel_projectile_y / (TILE_SIZE.y * TILE_SCALE.y);
        player->getPlayerNode()->setPriority(current->getGridSize().height -
                                             row);
      }
    }
  }
}

void LevelController::dispose() { _level_gen->dispose(); }

void LevelController::changeRoom(std::string &door_sensor_name) {
  std::shared_ptr<RoomModel> current = _level_model->getCurrentRoom();

  int destination_room_id =
      current->getRoomIdFromDoorSensorId(door_sensor_name);
  if (destination_room_id == -1) return;

  cugl::Vec2 door_pos = current->getPosOfDestinationDoor(door_sensor_name);

  for (std::shared_ptr<EnemyModel> enemy : current->getEnemies())
    enemy->promiseToChangePhysics(false);

  if (_room_on_chopping_block != nullptr)
    _room_on_chopping_block->setVisible(false);
  _room_on_chopping_block = current;

  _level_model->setCurrentRoom(destination_room_id);

  std::shared_ptr<RoomModel> new_current =
      _level_model->getCurrentRoom();  // New current level.

  new_current->setVisible(true);

  _level_model->getPlayer()->setPosPromise(
      new_current->getNode()->getPosition() +
      door_pos * (TILE_SIZE * TILE_SCALE));

  for (std::shared_ptr<EnemyModel> enemy : new_current->getEnemies())
    enemy->promiseToChangePhysics(true);
}

void LevelController::populate() {
  _level_model = LevelModel::alloc();

  instantiateWorld();

  // Initialize every room.
  for (std::shared_ptr<level_gen::Room> room : _level_gen->getRooms()) {
    auto room_node = _assets->get<cugl::scene2::SceneNode>(room->_scene2_key);

    room_node->setAnchor(cugl::Vec2::ANCHOR_BOTTOM_LEFT);
    cugl::Vec2 pos = room->getRect().origin * (TILE_SIZE * TILE_SCALE);
    room_node->setPosition(pos);
    room_node->setVisible(false);

    auto room_model = RoomModel::alloc(room_node, room->_key);
    _level_model->addRoom(room->_key, room_model);

    // Make spawn the starting point.
    if (room->_type == level_gen::Room::RoomType::SPAWN) {
      _level_model->setCurrentRoom(room->_key);
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

void LevelController::instantiateWorld() {
  // Represent the two corners of the world.
  cugl::Vec2 world_start;
  cugl::Vec2 world_end;

  // Get Size of World.
  for (std::shared_ptr<level_gen::Room> room : _level_gen->getRooms()) {
    cugl::Vec2 pos = room->getRect().origin * (TILE_SIZE * TILE_SCALE);
    cugl::Vec2 size =
        ((cugl::Vec2)room->getRect().size) * (TILE_SIZE * TILE_SCALE);
    if (pos.x < world_start.x) world_start.x = pos.x;
    if (pos.y < world_start.y) world_start.y = pos.y;
    if (pos.x + size.x > world_end.x) world_end.x = pos.x;
    if (pos.y + size.y > world_end.y) world_end.y = pos.y;
  }

  _world = cugl::physics2::ObstacleWorld::alloc(
      cugl::Rect(world_start, world_end - world_start));
}

std::vector<cugl::Vec2> LevelController::instantiateDoors(
    const std::shared_ptr<level_gen::Room> &room,
    const std::shared_ptr<RoomModel> &room_model) {
  std::vector<cugl::Vec2> unused_doors = room->_doors;

  // Initialize doors.
  for (auto &it : room->_edge_to_door) {
    std::shared_ptr<level_gen::Edge> edge = it.first;
    cugl::Vec2 door = it.second;
    unused_doors.erase(
        std::remove(unused_doors.begin(), unused_doors.end(), door),
        unused_doors.end());

    int y = room_model->getGridSize().height - static_cast<int>(door.y) - 1;
    int x = static_cast<int>(door.x);
    std::stringstream ss;
    ss << room->_scene2_key << "_tiles_tile-(" << y << "-" << x << ")_tile";
    std::string door_name = ss.str();

    auto door_room_node = std::dynamic_pointer_cast<Door>(
        _assets->get<cugl::scene2::SceneNode>(door_name));

    if (door_room_node) {
      std::string door_sensor_name = door_name + "-door";
      _world->addObstacle(door_room_node->initBox2d(door_sensor_name));

      std::shared_ptr<level_gen::Room> other_room = edge->getOther(room);
      cugl::Vec2 destination = other_room->_edge_to_door[edge];

      if (destination.x == 0) destination.x += 1;
      if (destination.y == 0) destination.y += 3;
      if (destination.x == room_model->getGridSize().width - 1)
        destination.x -= 1;
      if (destination.y == room_model->getGridSize().height - 1)
        destination.y -= 1;

      if (other_room->_key != -1) {
        room_model->addConnection(door_sensor_name, other_room->_key,
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
  auto wall_top = _assets->get<cugl::scene2::SceneNode>("wall-top");
  auto wall_side = _assets->get<cugl::scene2::SceneNode>("wall-side");
  for (cugl::Vec2 door : unused_doors) {
    int y = room_model->getGridSize().height - static_cast<int>(door.y) - 1;
    int x = static_cast<int>(door.x);
    std::stringstream ss;
    ss << room->_scene2_key << "_tiles_tile-(" << y << "-" << x << ")";
    std::string door_name = ss.str();

    auto door_tile_copy = cugl::scene2::SceneNode::alloc();
    auto door_tile = _assets->get<cugl::scene2::SceneNode>(door_name);
    if (door_tile) {
      door_tile->copy(door_tile_copy);
      door_tile_copy->setName(door_name + "-floor");

      auto wall_top_copy = std::dynamic_pointer_cast<Wall>(Wall::alloc());
      auto wall_side_copy = std::dynamic_pointer_cast<Wall>(Wall::alloc());
      wall_top->copy(wall_top_copy);
      wall_side->copy(wall_side_copy);
      // TODO: change depending on wall
      wall_top_copy->setPosition(0.0f, 27.599999999999998f);

      wall_top_copy->setPriority(y);
      wall_side_copy->setPriority(y);

      door_tile->removeAllChildren();
      door_tile->addChild(wall_side_copy);

      door_tile_copy->addChild(wall_top_copy);
      room_model->getNode()->addChild(door_tile_copy);

      _world->addObstacle(wall_top_copy->initBox2d());
      _world->addObstacle(wall_side_copy->initBox2d());
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
    std::string enemy_type = enemy_node->getType();
    auto enemy_texture = _assets->get<cugl::Texture>(enemy_type);
    std::shared_ptr<EnemyModel> enemy = EnemyModel::alloc(
        enemy_node->getWorldPosition(), enemy_node->getName(), enemy_type);
    enemy->setEnemyId(next_enemy_id);
    next_enemy_id = next_enemy_id + 1;
    enemies.push_back(enemy);

    auto e_node = cugl::scene2::SpriteNode::alloc(enemy_texture, 1, 1);
    enemy->setNode(e_node, _debug_node);

    enemy->setRoomPos(room_model->getNode()->getPosition());
    room_model->getNode()->addChild(enemy->getNode());
    _world->addObstacle(enemy);
    if (room->_type != level_gen::Room::RoomType::SPAWN)
      enemy->setEnabled(false);

    enemy->setDebugScene(_debug_node);
    enemy->setDebugColor(cugl::Color4(cugl::Color4::BLACK));
  }
}
