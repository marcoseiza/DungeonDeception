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
  _activate_terminal_scene = ActivateTerminalScene::alloc(_assets);

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
        if (_voting_info.find(_terminal_room_id) == _voting_info.end()) {
          _voting_info[_terminal_room_id] = std::make_shared<VotingInfo>();
          _voting_info[_terminal_room_id]->terminal_room_id = _terminal_room_id;
          _voting_info[_terminal_room_id]->num_players_req = _num_players_req;
        }

        _wait_for_players_scene->start(_voting_info[_terminal_room_id],
                                       _num_players_req);

        {
          auto info = cugl::JsonValue::allocObject();
          auto terminal_room_id_info =
              cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
          info->appendChild(terminal_room_id_info);
          terminal_room_id_info->setKey("terminal_room_id");

          auto stage_info = cugl::JsonValue::alloc(static_cast<long>(_stage));
          info->appendChild(stage_info);
          stage_info->setKey("stage");

          if (NetworkController::get()->isHost()) {
            NetworkController::get()->send(NC_HOST_CHANGE_STAGE, info);
          }
        }

      } else {
        if (_voting_info.find(_terminal_room_id) != _voting_info.end()) {
          _wait_for_players_scene->update();
        }
      }

      if (_wait_for_players_scene->didExit()) {
        _wait_for_players_scene->dispose();
        done();
        _terminal_sensor->deactivate();
      }

      if (_wait_for_players_scene->isDone()) {
        _wait_for_players_scene->dispose();
        _stage = Stage::VOTE_LEADER;
      }
      break;
    case Stage::VOTE_LEADER:
      if (!_vote_for_leader_scene->isActive()) {
        _vote_for_leader_scene->start(_voting_info[_terminal_room_id],
                                      _terminal_room_id);

        {
          auto info = cugl::JsonValue::allocObject();
          auto terminal_room_id_info =
              cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
          info->appendChild(terminal_room_id_info);
          terminal_room_id_info->setKey("terminal_room_id");

          auto stage_info = cugl::JsonValue::alloc(static_cast<long>(_stage));
          info->appendChild(stage_info);
          stage_info->setKey("stage");

          if (NetworkController::get()->isHost()) {
            NetworkController::get()->send(NC_HOST_CHANGE_STAGE, info);
          }
        }
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
        _voting_info[_terminal_room_id]->votes.clear();
        _voting_info[_terminal_room_id]->votes[_leader_id] = std::vector<int>();
        _voting_info[_terminal_room_id]->done.clear();

        {
          auto info = cugl::JsonValue::allocObject();
          auto terminal_room_id_info =
              cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
          info->appendChild(terminal_room_id_info);
          terminal_room_id_info->setKey("terminal_room_id");

          NetworkController::get()->sendOnlyToHost(NC_CLIENT_CLEAR_VOTES, info);
          NetworkController::get()->sendOnlyToHost(NC_CLIENT_CLEAR_DONE, info);
        }

        _vote_for_team_scene->start(_voting_info[_terminal_room_id],
                                    _terminal_room_id, _leader_id,
                                    _num_players_req);

        {
          auto info = cugl::JsonValue::allocObject();
          auto terminal_room_id_info =
              cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
          info->appendChild(terminal_room_id_info);
          terminal_room_id_info->setKey("terminal_room_id");

          auto stage_info = cugl::JsonValue::alloc(static_cast<long>(_stage));
          info->appendChild(stage_info);
          stage_info->setKey("stage");

          if (NetworkController::get()->isHost()) {
            NetworkController::get()->send(NC_HOST_CHANGE_STAGE, info);
          }
        }
      }

      _vote_for_team_scene->update();

      if (_vote_for_team_scene->isDone()) {
        _vote_for_team_scene->dispose();

        std::vector<int> &votes =
            _voting_info[_terminal_room_id]->votes[_leader_id];
        votes.push_back(_leader_id);
        auto found =
            std::find(votes.begin(), votes.end(),
                      _player_controller->getMyPlayer()->getPlayerId());

        if (found == votes.end()) {
          done();
        } else {
          _stage = Stage::ACTIVATE_TERMINAL;
        }
      }
      break;
    case Stage::ACTIVATE_TERMINAL:
      if (!_activate_terminal_scene->isActive()) {
        _voting_info[_terminal_room_id]->done.clear();

        {
          auto info = cugl::JsonValue::allocObject();
          auto terminal_room_id_info =
              cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
          info->appendChild(terminal_room_id_info);
          terminal_room_id_info->setKey("terminal_room_id");

          NetworkController::get()->sendOnlyToHost(NC_CLIENT_CLEAR_DONE, info);
        }

        _activate_terminal_scene->start(_voting_info[_terminal_room_id],
                                        _terminal_room_id, _num_players_req);

        {
          auto info = cugl::JsonValue::allocObject();
          auto terminal_room_id_info =
              cugl::JsonValue::alloc(static_cast<long>(_terminal_room_id));
          info->appendChild(terminal_room_id_info);
          terminal_room_id_info->setKey("terminal_room_id");

          auto stage_info = cugl::JsonValue::alloc(static_cast<long>(_stage));
          info->appendChild(stage_info);
          stage_info->setKey("stage");

          if (NetworkController::get()->isHost()) {
            NetworkController::get()->send(NC_HOST_CHANGE_STAGE, info);
          }
        }
      }

      _activate_terminal_scene->update();

      if (_activate_terminal_scene->isDone() &&
          _voting_info[_terminal_room_id]->terminal_done) {
        _activate_terminal_scene->dispose();
        done();
      }
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
        auto terminal_done_info =
            cugl::JsonValue::alloc((it->second)->terminal_done);
        info->appendChild(terminal_done_info);
        terminal_done_info->setKey("terminal_done");
      }

      {
        auto was_activate_info =
            cugl::JsonValue::alloc((it->second)->was_activated);
        info->appendChild(was_activate_info);
        was_activate_info->setKey("was_activated");
      }

      {
        auto terminal_room_id_info = cugl::JsonValue::alloc(
            static_cast<long>((it->second)->terminal_room_id));
        info->appendChild(terminal_room_id_info);
        terminal_room_id_info->setKey("terminal_room_id");
      }

      {
        auto num_players_req_info = cugl::JsonValue::alloc(
            static_cast<long>((it->second)->num_players_req));
        info->appendChild(num_players_req_info);
        num_players_req_info->setKey("num_players_req");
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
        if ((it->second)->players.size() >= (it->second)->num_players_req) {
          (it->second)->buffer_timer++;
          auto buffer_timer_info = cugl::JsonValue::alloc(
              static_cast<long>((it->second)->buffer_timer));
          info->appendChild(buffer_timer_info);
          buffer_timer_info->setKey("buffer_timer");
        }
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

      NetworkController::get()->send(NC_HOST_VOTING_INFO, info);
    }
  }
}

void TerminalController::processNetworkData(
    const Sint32 &code, const cugl::NetworkDeserializer::Message &msg) {
  switch (code) {
    case NC_CLIENT_PLAYER_ADDED: {
      if (_stage == Stage::WAIT_FOR_PLAYERS) {
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

        int num_players_req = info->getInt("num_players_req");
        _voting_info[terminal_room_id]->num_players_req = num_players_req;
      }
    } break;
    case NC_CLIENT_PLAYER_REMOVED: {
      if (_stage == Stage::WAIT_FOR_PLAYERS) {
        std::shared_ptr<cugl::JsonValue> info =
            std::get<std::shared_ptr<cugl::JsonValue>>(msg);

        int terminal_room_id = info->getInt("terminal_room_id");
        int player_id = info->getInt("player_id");

        if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
          std::vector<int> &players = _voting_info[terminal_room_id]->players;
          players.erase(std::remove(players.begin(), players.end(), player_id),
                        players.end());
        }
      }
    } break;
    case NC_CLIENT_VOTING_INFO: {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      int player_id = info->getInt("player_id");
      std::vector<int> voted_for = info->get("voted_for")->asIntArray();

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        _voting_info[terminal_room_id]->votes[player_id] = voted_for;
      }
    } break;
    case NC_HOST_VOTING_INFO: {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      int num_players_req = info->getInt("num_players_req");
      int buffer_timer = info->getInt("buffer_timer");
      std::vector<int> players = info->get("players")->asIntArray();
      auto votes = info->get("votes");
      std::vector<int> done = info->get("done")->asIntArray();

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        _voting_info[terminal_room_id]->players = players;
        _voting_info[terminal_room_id]->done = done;
        _voting_info[terminal_room_id]->buffer_timer = buffer_timer;
        _voting_info[terminal_room_id]->was_activated &=
            info->getBool("was_activated");
        _voting_info[terminal_room_id]->terminal_done =
            info->getBool("terminal_done");

        if (votes->isArray()) {
          if (votes->children().size() == 0) {
            _voting_info[terminal_room_id]->votes.clear();
          }

          for (auto vote : votes->children()) {
            int player_id = vote->getInt("player_id");
            std::vector<int> voted_for = vote->get("voted_for")->asIntArray();
            _voting_info[terminal_room_id]->votes[player_id] = voted_for;
          }
        }
      } else {
        auto new_voting_info = std::make_shared<VotingInfo>();
        new_voting_info->terminal_room_id = terminal_room_id;
        new_voting_info->num_players_req = num_players_req;
        new_voting_info->players = players;
        new_voting_info->done = done;

        if (votes->isArray()) {
          if (votes->children().size() == 0) {
            new_voting_info->votes.clear();
          }
          for (auto vote : votes->children()) {
            int player_id = vote->getInt("player_id");
            std::vector<int> voted_for = vote->get("voted_for")->asIntArray();
            new_voting_info->votes[player_id] = voted_for;
          }
        }

        _voting_info[terminal_room_id] = new_voting_info;
      }
    } break;
    case NC_CLIENT_DONE_WITH_VOTE: {
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
    case NC_CLIENT_DONE_ACTIVATE_TERMINAL: {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      int player_id = info->getInt("player_id");

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        std::shared_ptr<VotingInfo> v = _voting_info[terminal_room_id];
        v->was_activated &= info->getBool("did_activate");

        auto found = std::find(v->done.begin(), v->done.end(), player_id);
        if (found == v->done.end()) {
          v->done.push_back(player_id);
        }
      }
    } break;
    case NC_CLIENT_TERMINAL_DONE: {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      bool terminal_done = info->getBool("terminal_done");

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        std::shared_ptr<VotingInfo> v = _voting_info[terminal_room_id];
        v->terminal_done = terminal_done;
      }
    } break;
    case NC_CLIENT_CLEAR_VOTES: {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        _voting_info[terminal_room_id]->votes.clear();
      }
    } break;
    case NC_CLIENT_CLEAR_DONE: {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");

      if (_voting_info.find(terminal_room_id) != _voting_info.end()) {
        _voting_info[terminal_room_id]->done.clear();
      }
    } break;
    case NC_HOST_CHANGE_STAGE: {
      std::shared_ptr<cugl::JsonValue> info =
          std::get<std::shared_ptr<cugl::JsonValue>>(msg);

      int terminal_room_id = info->getInt("terminal_room_id");
      int stage = info->getInt("stage");

      if (stage > Stage::WAIT_FOR_PLAYERS) {
        _wait_for_players_scene->setDone(true);
      }
      if (stage > Stage::VOTE_LEADER) {
        _vote_for_leader_scene->setDone(true);
      }
      if (stage > Stage::VOTE_TEAM) {
        _vote_for_team_scene->setDone(true);
      }

    } break;
  }
}
