#include <cugl/cugl.h>

#include "GameApp.h"

// Window size if not on mobile device.
#define GAME_WIDTH 1024
#define GAME_HEIGHT 576

int main(int argc, char *argv[]) {
  GameApp app;

  app.setName("Dungeon Deception");
  app.setOrganization("(Oops) Games");

  // Set the window properties (Only applies to OS X/Windows Desktop).
  app.setDisplaySize(GAME_WIDTH, GAME_HEIGHT);
  app.setFPS(60.0f);
  app.setHighDPI(true);

  /// DO NOT MODIFY ANYTHING BELOW THIS LINE.
  if (!app.init()) {
    return 1;
  }

  // Run the application until completion.
  app.onStartup();

  while (app.step())
    ;
  app.onShutdown();

  exit(0);  // Necessary to quit on mobile devices.
  return 0;
}
