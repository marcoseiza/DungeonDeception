#include "TerminalController.h"

#include "NetworkController.h"

bool TerminalController::init(
    const std::shared_ptr<cugl::AssetManager> &assets) {
  if (_active) return false;

  _assets = assets;
  _scene = _assets->get<cugl::scene2::SceneNode>("terminal-voting-scene");
  _scene->setVisible(false);
  _stage = WAIT_FOR_PLAYERS;

  _wait_for_players_scene = WaitForPlayersScene::alloc(_assets);
  _vote_for_leader_scene = VoteForLeaderScene::alloc(_assets);
  _vote_for_team_scene = VoteForTeamScene::alloc(_assets);

  NetworkController::get()->addListener(
      [=](const Sint32 &code, const cugl::NetworkDeserializer::Message &msg) {
        this->processNetworkData(code, msg);
      });

  return true;
}

void TerminalController::update(float timestep) {
  sendNetworkData();

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
        _vote_for_leader_scene->start(_voting_info[_terminal_room_id],
                                      _terminal_room_id);
      }

      _vote_for_leader_scene->update();

      if (_vote_for_leader_scene->isDone()) {
        _leader_id = _vote_for_leader_scene->getLeader();
        _vote_for_leader_scene->dispose();
        _stage = Stage::VOTE_TEAM;
      }
      break;
    case Stage::VOTE_TEAM:
      if (!_vote_for_team_scene->isActive()) {
        _vote_for_team_scene->start(_voting_info[_terminal_room_id],
                                    _terminal_room_id, _leader_id);
      }

      _vote_for_team_scene->update();

      if (_vote_for_team_scene->isDone()) {
        _vote_for_team_scene->dispose();
        _stage = Stage::ACTIVATE_TERMINAL;
      }
      break;
    case Stage::ACTIVATE_TERMINAL:
      break;
    default:
      break;
  }
}

void TerminalController::sendNetworkData() {
  if (NetworkController::get()->isHost()) {
    for (auto it = _voting_info.begin(); it != _voting_info.end(); ++it) {
      auto info = cugl::JsonValue::allocObject();

      {
        auto terminal_room_id_info = cugl::JsonValue::alloc(
            static_cast<long>((it->second)->terminal_room_id));
        info->appendChild(terminal_room_id_info);
        terminal_room_id_info->setKey("terminal_room_id");
      }

      {
        auto players_ids_info = cugl::JsonValue::allocArray();
        for (int player_id : (it->second)->players) {
          players_ids_info->appendChild(
              cugl::JsonValue::alloc(static_cast<long>(player_id)));
        }
        info->appendChild(players_ids_info);
        players_ids_info->setKey("players");
      }

      {
        auto votes_info = cugl::JsonValue::allocArray();

        for (auto vote : (it->second)->votes) {
          auto vote_info = cugl::JsonValue::allocObject();

          int player_id = vote.first;
          auto player_id_info =
              cugl::JsonValue::alloc(static_cast<long>(player_id));
          vote_info->appendChild(player_id_info);
          player_id_info->setKey("player_id");

          auto voted_for_info = cugl::JsonValue::allocArray();
          std::vector<int> &voted_for = vote.second;
          for (int player_voted_for_id : voted_for) {
            voted_for_info->appendChild(
                cugl::JsonValue::alloc(static_cast<long>(player_voted_for_id)));
          }
          vote_info->appendChild(voted_for_info);
          voted_for_info->setKey("voted_for");

          votes_info->appendChild(vote_info);
        }

        info->appendChild(votes_info);
        votes_info->setKey("votes");
      }

      {
        auto done_info = cugl::JsonValue::allocArray();
        for (int player_id : (it->second)->done) {
          done_info->appendChild(
              cugl::JsonValue::alloc(static_cast<long>(player_id)));
        }
        info->appendChild(done_info);
        done_info->setKey("done");
      }

      NetworkController::get()->send(9, info);
    }
  }
}

void TerminalController::processNetworkData(
    const Sint32 &code, const cugl::NetworkDeserializer::Message &msg) {
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
    case 8:  // Receive vote info from client.
    {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      int player_id = info->getInt("player_id");
      std::vector<int> voted_for = info->get("voted_for")->asIntArray();

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        if (_voting_info[terminal_room_id]->votes.find(player_id) !=
            _voting_info[terminal_room_id]->votes.end()) {
          _voting_info[terminal_room_id]->votes[player_id] = voted_for;
        }
      }
    } break;
    case 9:  // Receive voting info from host.
    {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      std::vector<int> players = info->get("players")->asIntArray();
      auto votes = info->get("votes");
      std::vector<int> done = info->get("done")->asIntArray();

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        _voting_info[terminal_room_id]->players = players;
        _voting_info[terminal_room_id]->done = done;

        if (votes->isArray()) {
          for (auto vote : votes->children()) {
            int player_id = vote->getInt("player_id");
            std::vector<int> voted_for = vote->get("voted_for")->asIntArray();
            _voting_info[terminal_room_id]->votes[player_id] = voted_for;
          }
        }
      } else {
        auto new_voting_info = std::make_shared<VotingInfo>();
        new_voting_info->terminal_room_id = terminal_room_id;
        new_voting_info->players = players;
        new_voting_info->done = done;

        if (votes->isArray()) {
          for (auto vote : votes->children()) {
            int player_id = vote->getInt("player_id");
            std::vector<int> voted_for = vote->get("voted_for")->asIntArray();
            new_voting_info->votes[player_id] = voted_for;
          }
        }

        _voting_info[terminal_room_id] = new_voting_info;
      }
    } break;
    case 10:  // Receive done with vote from client.
    {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      int player_id = info->getInt("player_id");
      bool add = info->getBool("add");

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        std::shared_ptr<VotingInfo> v = _voting_info[terminal_room_id];

        auto found = std::find(v->done.begin(), v->done.end(), player_id);

        if (found == v->done.end()) {
          if (add) {
            v->done.push_back(player_id);
          } else {
            v->done.erase(found);
          }
        }
      }
    } break;
  }
}