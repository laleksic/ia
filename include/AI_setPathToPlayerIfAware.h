#ifndef AI_SET_PATH_TO_PLAYER_IF_AWARE_H
#define AI_SET_PATH_TO_PLAYER_IF_AWARE_H

#include "Engine.h"
#include "MapParsing.h"

class AI_setPathToPlayerIfAware {
public:
  static void learn(Monster& monster, vector<Pos>& path, Engine& engine) {
    if(monster.deadState == actorDeadState_alive) {
      if(monster.playerAwarenessCounter > 0) {

        bool blockers[MAP_W][MAP_H];
        engine.basicUtils->resetArray(blockers, false);
        for(int y = 1; y < MAP_H - 1; y++) {
          for(int x = 1; x < MAP_W - 1; x++) {
            const Feature* const f = engine.map->cells[x][y].featureStatic;
            if(f->canBodyTypePass(monster.getBodyType()) == false) {

              if(f->getId() == feature_door) {

                const Door* const door = dynamic_cast<const Door*>(f);

                const ActorData& d = monster.getData();

                if(
                  (d.canOpenDoors == false && d.canBashDoors == false) ||
                  door->isOpenedAndClosedExternally()) {
                  blockers[x][y] = true;
                }

              } else {
                blockers[x][y] = true;
              }
            }
          }
        }

        //Append living adjacent actors to the blocking array
        MapParse::parse(CellPred::LivingActorsAdjToPos(monster.pos, engine),
                        blockers, mapParseWriteOnlyTrue);

        PathFind::run(monster.pos, engine.player->pos, blockers, path, engine);
      } else {
        path.resize(0);
      }
    } else {
      path.resize(0);
    }
  }

private:
};

#endif

