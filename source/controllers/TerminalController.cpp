#include "TerminalController.h"

bool TerminalController::init(
    const std::shared_ptr<cugl::AssetManager>& assets) {
  if (_active) return false;

  _assets = assets;
  _scene = _assets->get<cugl::scene2::SceneNode>("terminal-voting-scene");
  _scene->setVisible(false);
  _stage = WAIT_FOR_PLAYERS;

  _wait_for_players_scene = WaitForPlayersScene::alloc(_assets);
  _vote_for_leader_scene = VoteForLeaderScene::alloc(_assets);

  return true;
}

void TerminalController::update(float timestep) {
  if (!_active) return;

  switch (_stage) {
    case Stage::WAIT_FOR_PLAYERS:
      if (!_wait_for_players_scene->isActive()) {
        _wait_for_players_scene->start(_num_players_req);
      }

      _wait_for_players_scene->update();
      _wait_for_players_scene->setCurrentNumPlayers(
          _voting_info[_terminal_room_id]->players.size());

      if (_wait_for_players_scene->isDone()) {
        _wait_for_players_scene->dispose();
        _stage = Stage::VOTE_LEADER;
      }
      break;
    case Stage::VOTE_LEADER:
      if (!_vote_for_leader_scene->isActive()) {
        _vote_for_leader_scene->start(_voting_info[_terminal_room_id]);
      }

      if (_vote_for_leader_scene->isDone()) {
        _vote_for_leader_scene->dispose();
        _stage = Stage::VOTE_TEAM;
      }
      break;
    case Stage::VOTE_TEAM:
      break;
    case Stage::ACTIVATE_TERMINAL:
      break;
    default:
      break;
  }
}

void TerminalController::processNetworkData(
    Sint32 code, cugl::NetworkDeserializer::Message msg) {
  switch (code) {
    case 7:  // Receive one player added to terminal from client.
    {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      int player_id = info->getInt("player_id");

      if (_voting_info.find(terminal_room_id) == _voting_info.end()) {
        auto new_voting_info = std::make_shared<VotingInfo>();
        new_voting_info->terminal_room_id = terminal_room_id;
        _voting_info[terminal_room_id] = new_voting_info;
      }

      auto result =
          std::find(_voting_info[terminal_room_id]->players.begin(),
                    _voting_info[terminal_room_id]->players.end(), player_id);

      if (result == _voting_info[terminal_room_id]->players.end()) {
        _voting_info[terminal_room_id]->players.push_back(player_id);
      }
    } break;
    case 8:  // Receive voting info from host.
    {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      std::vector<int> players = info->get("players")->asIntArray();

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        _voting_info[terminal_room_id]->players = players;
      } else {
        auto new_voting_info = std::make_shared<VotingInfo>();
        new_voting_info->terminal_room_id = terminal_room_id;
        new_voting_info->players = players;
        _voting_info[terminal_room_id] = new_voting_info;
      }
    } break;
  }
}
