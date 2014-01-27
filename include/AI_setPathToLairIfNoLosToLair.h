#ifndef AI_SET_PATH_TO_LAIR_IF_NO_LOS_TO_LAIR_H
#define AI_SET_PATH_TO_LAIR_IF_NO_LOS_TO_LAIR_H

#include "Engine.h"

class AI_setPathToLairIfNoLosToLair {
public:
  static void learn(Monster& monster, vector<Pos>& path, const Pos& lairCell,
                    Engine& eng) {

    if(monster.deadState == actorDeadState_alive) {

      bool blockers[MAP_W][MAP_H];
      MapParse::parse(CellPred::BlocksVision(eng), blockers);

      if(eng.fov->checkCell(blockers, lairCell, monster.pos, true)) {
        path.resize(0);
        return;
      }

      MapParse::parse(
        CellPred::BlocksBodyType(monster.getBodyType(), false, eng), blockers);

      MapParse::parse(CellPred::LivingActorsAdjToPos(monster.pos, eng),
                      blockers, mapParseWriteOnlyTrue);

      PathFind::run(monster.pos, lairCell, blockers, path, eng);
      return;
    }

    path.resize(0);

  }
private:
};

#endif
