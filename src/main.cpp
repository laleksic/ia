#include <iostream>

#include <SDL.h>

#include "Init.h"

#include "SdlWrapper.h"
#include "Config.h"
#include "Input.h"
#include "Renderer.h"
#include "MainMenu.h"
#include "PlayerBon.h"
#include "Bot.h"
#include "CreateCharacter.h"
#include "ActorPlayer.h"
#include "MapGen.h"
#include "DungeonClimb.h"
#include "DungeonMaster.h"
#include "Popup.h"
#include "Log.h"
#include "Query.h"
#include "Highscore.h"
#include "Postmortem.h"
#include "Map.h"

#ifdef _WIN32
#undef main
#endif

using namespace std;

int main(int argc, char* argv[]) {
  trace << "main()..." << endl;

  (void)argc;
  (void)argv;

  Init::initIO();
  Init::initGame();

  bool quitGame = false;
  while(quitGame == false) {
    Init::initSession();

    int introMusChan = -1;
    const GameEntryMode gameEntryType = MainMenu::run(quitGame, introMusChan);

    if(quitGame == false) {
      Init::quitToMainMenu = false;

      if(gameEntryType == GameEntryMode::newGame) {
        if(Config::isBotPlaying()) {
          PlayerBon::setAllTraitsToPicked();
        }
        CreateCharacter::createCharacter();
        Map::player->spawnStartItems();

        GameTime::insertActorInLoop(Map::player);

        if(Config::isIntroLevelSkipped() == false) {
          //If intro level is used, build forest.
          Renderer::coverPanel(Panel::screen);
          Renderer::updateScreen();
          MapGenIntroForest().run();
        } else {
          //Else build first dungeon level
          DungeonClimb::travelDown();
        }
        DungeonMaster::setTimeStartedToNow();
        const TimeData& t = DungeonMaster::getTimeStarted();
        trace << "Game started on: " << t.getTimeStr(time_minute, true) << endl;
      }

      Audio::fadeOutChannel(introMusChannel);

      Map::player->updateFov();
      Renderer::drawMapAndInterface();

      if(gameEntryType == GameEntryMode::newGame) {
        if(Config::isIntroLevelSkipped() == 0) {
          const string msg =
            "I stand on a cobbled forest path, ahead lies a shunned decrepit "
            "old church building. From years of investigation and discreet "
            "inquiries, I know this to be the access point to the abhorred "
            "\"Cult of Starry Wisdom\". I will enter these sprawling catacombs "
            "and rob them of treasures and knowledge. The ultimate prize is an "
            "artifact of non-human origin called \"The shining Trapezohedron\" "
            "- a window to all secrets of the universe.";
          Popup::showMsg(msg, true, "The story so far...");
        }
      }

      //========== M A I N   L O O P ==========
      while(Init::quitToMainMenu == false) {
        if(Map::player->deadState == ActorDeadState::alive) {

          Actor* const actor = GameTime::getCurrentActor();

          //Properties running on the actor's turn are not immediately applied
          //on the actor, but instead placed in a buffer. This is to ensure
          //that e.g. a property set to last one turn actually covers one turn
          //(and not applied after the actor acts, and ends before the actor's
          //next turn)
          //The contents of the buffer are moved to the applied properties here
          actor->getPropHandler().applyActorTurnPropBuffer();

          actor->updateColor();

          if(
            actor->getPropHandler().allowAct() &&
            actor->deadState != ActorDeadState::destroyed) {
            actor->onActorTurn();
          } else {
            if(actor == Map::player) {
              SdlWrapper::sleep(DELAY_PLAYER_UNABLE_TO_ACT);
            }
            GameTime::actorDidAct();
          }
        } else {
          //Player is dead, run postmortem, then return to main menu
          dynamic_cast<Player*>(Map::player)->waitTurnsLeft = -1;
          Log::addMsg("I am dead... (press space/esc to proceed)",
                      clrMsgBad);
          Audio::play(SfxId::death);
          Renderer::drawMapAndInterface();
          Log::clearLog();
          Query::waitForEscOrSpace();
          HighScore::gameOver(false);
          Postmortem::run(&quitGame);
          Init::quitToMainMenu = true;
        }
      }
    }
    Init::cleanupSession();
  }
  Init::cleanupGame();
  Init::cleanupIO();

  trace << "main() [DONE]" << endl;

  return 0;
}

