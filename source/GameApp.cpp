#include "GameApp.h"

#include "loaders/CustomScene2Loader.h"
#include "models/level_gen/DefaultRooms.h"

void GameApp::onStartup() {
  _assets = cugl::AssetManager::alloc();
  _batch = cugl::SpriteBatch::alloc();
  auto cam = cugl::OrthographicCamera::alloc(getDisplaySize());

#ifdef CU_TOUCH_SCREEN
  cugl::Input::activate<cugl::Touchscreen>();
#else
  cugl::Input::activate<cugl::Mouse>();
  // cugl::Mouse does not track mouse drag or move by default.
  cugl::Input::get<cugl::Mouse>()->setPointerAwareness(
      cugl::Mouse::PointerAwareness::ALWAYS);
#endif
  cugl::Input::activate<cugl::Keyboard>();
  cugl::Input::activate<cugl::TextInput>();

  cugl::AudioEngine::start();

  // Add asset loaders.
  _assets->attach<cugl::Texture>(cugl::TextureLoader::alloc()->getHook());
  _assets->attach<cugl::Font>(cugl::FontLoader::alloc()->getHook());
  _assets->attach<cugl::Sound>(cugl::SoundLoader::alloc()->getHook());
  _assets->attach<cugl::JsonValue>(cugl::JsonLoader::alloc()->getHook());
  _assets->attach<cugl::WidgetValue>(cugl::WidgetLoader::alloc()->getHook());
  _assets->attach<cugl::scene2::SceneNode>(
      cugl::CustomScene2Loader::alloc()->getHook());

  // Create a "loading" screen.
  _loaded = false;
  _loading.init(_assets);

  cugl::Application::onStartup();  // YOU MUST END with call to parent.
}

void GameApp::onShutdown() {
  _loading.dispose();
  _gameplay.dispose();
  _hostgame.dispose();
  _joingame.dispose();
  _joinlobby.dispose();
  _hostlobby.dispose();
  _menu.dispose();
  _win.dispose();
  _level_loading.dispose();
  _assets = nullptr;
  _batch = nullptr;

  cugl::AudioEngine::stop();

#ifdef CU_TOUCH_SCREEN
  cugl::Input::deactivate<cugl::Touchscreen>();
#else
  cugl::Input::deactivate<cugl::Mouse>();
#endif

  cugl::Application::onShutdown();  // YOU MUST END with call to parent.
}

void GameApp::update(float timestep) {
  switch (_scene) {
    case LOAD:
      updateLoadingScene(timestep);
      break;
    case MENU:
      updateMenuScene(timestep);
      break;
    case HOST:
      updateHostMenuScene(timestep);
      break;
    case HOST_LOBBY:
      updateHostLobbyScene(timestep);
      break;
    case CLIENT:
      updateClientMenuScene(timestep);
      break;
    case CLIENT_LOBBY:
      updateClientLobbyScene(timestep);
      break;
    case LEVEL_LOADING:
      updateLevelLoadingScene(timestep);
      break;
    case GAME:
      updateGameScene(timestep);
      break;
    case WIN:
      updateWinScene(timestep);
      break;
  }
}

void GameApp::draw() {
  switch (_scene) {
    case LOAD:
      _loading.render(_batch);
      break;
    case MENU:
      _menu.render(_batch);
      break;
    case HOST:
      _hostgame.render(_batch);
      break;
    case HOST_LOBBY:
      _hostlobby.render(_batch);
      break;
    case CLIENT:
      _joingame.render(_batch);
      break;
    case CLIENT_LOBBY:
      _joinlobby.render(_batch);
      break;
    case LEVEL_LOADING:
      _level_loading.render(_batch);
      break;
    case GAME:
      _gameplay.render(_batch);
      break;
    case WIN:
      _win.render(_batch);
      break;
  }
}

void GameApp::updateLoadingScene(float timestep) {
  if (!_loaded && _loading.isActive()) {
    _loading.update(timestep);
  } else if (!_loaded) {
    // Permanently disables the input listeners in this mode.
    _loading.dispose();
    cugl::scene2::Button::DEFAULT_CLICK_SOUND =
        _assets->get<cugl::Sound>("button-click");
    _menu.init(_assets);
    _hostgame.init(_assets);
    _joingame.init(_assets);
    _joinlobby.init(_assets);
    _hostlobby.init(_assets);
    _menu.setActive(true);
    _hostgame.setActive(false);
    _joingame.setActive(false);
    _hostlobby.setActive(false, nullptr);
    _joinlobby.setActive(false, nullptr);
    _gameplay.setActive(false);
    _win.setActive(false);
    _scene = State::MENU;
    _loaded = true;
  }
}

void GameApp::updateMenuScene(float timestep) {
  _menu.update(timestep);
  switch (_menu.getChoice()) {
    case MenuScene::Choice::HOST:
      _menu.setActive(false);
      _hostgame.setActive(true);
      _scene = State::HOST;
      break;
    case MenuScene::Choice::JOIN:
      _menu.setActive(false);
      _joingame.setActive(true);
      _scene = State::CLIENT;
      break;
    case MenuScene::Choice::JOIN_LOBBY:
      // TODO does menu scene ever make this choice?
      _joingame.setActive(false);
      _joinlobby.setActive(true, _joingame.getConnection());
      _scene = State::CLIENT_LOBBY;
      break;
    case MenuScene::Choice::NONE:
      // DO NOTHING
      break;
  }
}

void GameApp::updateHostMenuScene(float timestep) {
  _hostgame.update(timestep);
  switch (_hostgame.getStatus()) {
    case HostMenuScene::Status::ABORT:
      _hostgame.setActive(false);
      _menu.setActive(true);
      _scene = State::MENU;
      break;
    case HostMenuScene::Status::JOIN:
      _hostgame.setActive(false);
      _hostlobby.setActive(true, _hostgame.getConnection());
      _scene = State::HOST_LOBBY;
      break;
    case HostMenuScene::Status::WAIT:
    case HostMenuScene::Status::IDLE:
      // DO NOTHING
      break;
  }
}

void GameApp::updateClientMenuScene(float timestep) {
  // TODO client menu scene should not have start status, etc.
  _joingame.update(timestep);
  switch (_joingame.getStatus()) {
    case ClientMenuScene::Status::ABORT:
      _joingame.setActive(false);
      _menu.setActive(true);
      _scene = State::MENU;
      break;
    case ClientMenuScene::Status::WAIT:
      _joingame.setActive(false);
      _joinlobby.setGameId(_joingame.getGameId());
      _joinlobby.setActive(true, _joingame.getConnection());
      _scene = State::CLIENT_LOBBY;
      break;
    case ClientMenuScene::Status::JOIN:
    case ClientMenuScene::Status::IDLE:
      // DO NOTHING
      break;
  }
}

void GameApp::updateHostLobbyScene(float timestep) {
  _hostlobby.update(timestep);
  switch (_hostlobby.getStatus()) {
    case ClientLobbyScene::Status::ABORT:
      _hostlobby.setActive(false, nullptr);
      _hostgame.setActive(true);
      _scene = State::HOST;
      break;
    case ClientLobbyScene::Status::START:
      _hostlobby.setActive(false, nullptr);
      _level_loading.init(_assets, _hostlobby.getSeed());
      _scene = State::LEVEL_LOADING;
      // Transfer connection ownership
      _level_loading.setConnection(_hostgame.getConnection());
      _hostgame.disconnect();
      _level_loading.setHost(true);
      break;
    case ClientLobbyScene::Status::WAIT:
      // DO NOTHING
      break;
  }
}

void GameApp::updateClientLobbyScene(float timestep) {
  _joinlobby.update(timestep);
  switch (_joinlobby.getStatus()) {
    case ClientLobbyScene::Status::ABORT:
      _joinlobby.setActive(false, nullptr);
      _joingame.setActive(true);
      _scene = State::CLIENT;
      break;
    case ClientLobbyScene::Status::START:
      _joinlobby.setActive(false, nullptr);
      _level_loading.init(_assets, _joinlobby.getSeed());
      _scene = State::LEVEL_LOADING;
      // Transfer connection ownership
      _level_loading.setConnection(_joingame.getConnection());
      _hostgame.disconnect();
      _level_loading.setHost(false);
      break;
    case ClientLobbyScene::Status::WAIT:
      // DO NOTHING
      break;
  }
}

/**
 * Individualized update method for the level loading scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the loading scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameApp::updateLevelLoadingScene(float timestep) {
  if (_level_loading.isActive()) {
    _level_loading.update(timestep);
    return;
  }

  // Transfer connection ownership
  _gameplay.setConnection(_level_loading.getConnection());
  _level_loading.disconnect();
  _gameplay.setHost(_level_loading.getIsHost());

  bool betrayer = (_level_loading.getIsHost()) ? _hostlobby.isBetrayer()
                                               : _joinlobby.isBetrayer();
  std::string name = (_level_loading.getIsHost()) ? _hostlobby.getPlayerName()
                                                  : _joinlobby.getPlayerName();

  _level_loading.removeChild(_level_loading.getMap());
  _gameplay.init(_assets, _level_loading.getLevelGenerator(),
                 _level_loading.getMap(), betrayer, name);
  _level_loading.dispose();

  _scene = State::GAME;
}

/**
 * Individualized update method for the game scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the game scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameApp::updateGameScene(float timestep) {
  switch (_gameplay.getState()) {
    case GameScene::State::RUN:
      _gameplay.update(timestep);
      return;
    case GameScene::State::DONE:
      _gameplay.dispose();
      _win.init(_assets, _gameplay.checkCooperatorWin());
      _scene = State::WIN;
      return;
    case GameScene::State::LEAVE:
      _gameplay.dispose();
      _menu.setActive(true);
      _hostgame.setActive(false);
      _joingame.setActive(false);
      _hostlobby.setActive(false, nullptr);
      _joinlobby.setActive(false, nullptr);

      _scene = State::MENU;
      return;
    default:
      return;
  }
}

/**
 * Individualized update method for the win scene.
 *
 * This method keeps the primary {@link #update} from being a mess of switch
 * statements. It also handles the transition logic from the game scene.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameApp::updateWinScene(float timestep) {
  _win.update(timestep);
  switch (_win.getChoice()) {
    case WinScene::Choice::GOTOMENU:
      _win.dispose();
      _menu.setActive(true);
      _hostgame.setActive(false);
      _joingame.setActive(false);
      _hostlobby.setActive(false, nullptr);
      _joinlobby.setActive(false, nullptr);

      _scene = State::MENU;
      _loaded = true;
      break;
    case WinScene::Choice::NONE:
      // DO NOTHING
      break;
  }
}
