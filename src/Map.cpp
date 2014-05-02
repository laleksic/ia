#include "Map.h"

#include "Feature.h"
#include "FeatureFactory.h"
#include "ActorFactory.h"
#include "ItemFactory.h"
#include "GameTime.h"
#include "Renderer.h"
#include "MapGen.h"
#include "Item.h"
#include "Utils.h"

using namespace std;

inline void Cell::clear() {
  isExplored = isSeenByPlayer = isLight = isDark = false;

  if(featureStatic != NULL) {
    delete featureStatic;
    featureStatic = NULL;
  }

  if(item != NULL) {
    delete item;
    item = NULL;
  }

  playerVisualMemory.clear();
}

namespace Map {

Player*       player  = NULL;
int           dlvl    = 0;
Cell          cells[MAP_W][MAP_H];
vector<Room*> rooms;

namespace {

void Map::resetCells(const bool MAKE_STONE_WALLS) {
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {

      cells[x][y].clear();

      cells[x][y].pos = Pos(x, y);

      Renderer::renderArray[x][y].clear();
      Renderer::renderArrayNoActors[x][y].clear();

      if(MAKE_STONE_WALLS) {
        FeatureFactory::spawnFeatureAt(FeatureId::stoneWall, Pos(x, y));
      }
    }
  }
}

} //Namespace

void init() {
  const Pos playerPos(PLAYER_START_X, PLAYER_START_Y);
  player = ActorFactory::spawnActor(actor_player, playerPos);

  dlvl = 0;

  rooms.resize(0);

  ActorFactory::deleteAllMonsters();

  resetCells(false);
  GameTime::eraseAllFeatureMobs();
  GameTime::resetTurnTypeAndActorCounters();
}

void cleanup() {
  player = NULL; //Note: GameTime will delete player

  resetMap();
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      delete cells[x][y].featureStatic;
    }
  }
}

void storeToSaveLines(vector<string>& lines) const {
  lines.push_back(toStr(dlvl_));
}

void setupFromSaveLines(vector<string>& lines) {
  dlvl_ = toInt(lines.front());
  lines.erase(lines.begin());
}

//TODO This should probably go in a virtual method in Feature instead
void Map::switchToDestroyedFeatAt(const Pos& pos) {
  if(Utils::isPosInsideMap(pos)) {

    const FeatureId OLD_FEATURE_ID =
      Map::cells[pos.x][pos.y].featureStatic->getId();

    const vector<FeatureId> convertionCandidates =
      FeatureData::getData(OLD_FEATURE_ID)->featuresOnDestroyed;

    const int SIZE = convertionCandidates.size();
    if(SIZE > 0) {
      const FeatureId NEW_ID =
        convertionCandidates.at(Rnd::dice(1, SIZE) - 1);

      FeatureFactory::spawnFeatureAt(NEW_ID, pos);

      //Destroy adjacent doors?
      if(
        (NEW_ID == FeatureId::rubbleHigh || NEW_ID == FeatureId::rubbleLow) &&
        NEW_ID != OLD_FEATURE_ID) {
        for(int x = pos.x - 1; x <= pos.x + 1; x++) {
          for(int y = pos.y - 1; y <= pos.y + 1; y++) {
            if(x == 0 || y == 0) {
              const FeatureStatic* const f =
                Map::cells[x][y].featureStatic;
              if(f->getId() == FeatureId::door) {
                FeatureFactory::spawnFeatureAt(
                  FeatureId::rubbleLow, Pos(x, y));
              }
            }
          }
        }
      }

      if(NEW_ID == FeatureId::rubbleLow && NEW_ID != OLD_FEATURE_ID) {
        if(Rnd::percentile() < 50) {
          ItemFactory::spawnItemOnMap(ItemId::rock, pos);
        }
      }
    }
  }
}

void Map::resetMap() {
  ActorFactory::deleteAllMonsters();

  for(Room * room : rooms) {delete room;}

  rooms.resize(0);

  resetCells(true);
  GameTime::eraseAllFeatureMobs();
  GameTime::resetTurnTypeAndActorCounters();
}

void Map::updateVisualMemory() {
  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAP_H; y++) {
      Map::cells[x][y].playerVisualMemory =
        Renderer::renderArrayNoActors[x][y];
    }
  }
}

void makeBlood(const Pos& origin) {
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      const Pos c = origin + Pos(dx, dy);
      FeatureStatic* const f  = Map::cells[c.x][c.y].featureStatic;
      if(f->canHaveBlood()) {
        if(Rnd::percentile() > 66) {
          f->setHasBlood(true);
        }
      }
    }
  }
}

void makeGore(const Pos& origin) {
  for(int dx = -1; dx <= 1; dx++) {
    for(int dy = -1; dy <= 1; dy++) {
      const Pos c = origin + Pos(dx, dy);
      if(Rnd::percentile() > 66) {
        Map::cells[c.x][c.y].featureStatic->setGoreIfPossible();
      }
    }
  }
}

} //Map
