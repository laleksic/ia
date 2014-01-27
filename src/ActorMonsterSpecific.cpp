#include "ActorMonster.h"

#include <algorithm>

#include "Engine.h"
#include "ItemFactory.h"
#include "Inventory.h"
#include "Explosion.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "GameTime.h"
#include "ActorFactory.h"
#include "Renderer.h"
#include "CommonData.h"
#include "Map.h"
#include "Blood.h"
#include "FeatureFactory.h"
#include "Knockback.h"
#include "Gods.h"
#include "MapParsing.h"
#include "LineCalc.h"

using namespace std;

string Cultist::getCultistPhrase(Engine& engine) {
  vector<string> phraseCandidates;

  const God* const god = engine.gods->getCurrentGod();
  if(god != NULL && engine.dice.coinToss()) {
    const string name = god->getName();
    const string descr = god->getDescr();
    phraseCandidates.push_back(name + " save us!");
    phraseCandidates.push_back(descr + " will save us!");
    phraseCandidates.push_back(name + ", guide us!");
    phraseCandidates.push_back(descr + " guides us!");
    phraseCandidates.push_back("For " + name + "!");
    phraseCandidates.push_back("For " + descr + "!");
    phraseCandidates.push_back("Blood for " + name + "!");
    phraseCandidates.push_back("Blood for " + descr + "!");
    phraseCandidates.push_back("Perish for " + name + "!");
    phraseCandidates.push_back("Perish for " + descr + "!");
    phraseCandidates.push_back("In the name of " + name + "!");
  } else {
    phraseCandidates.push_back("Apigami!");
    phraseCandidates.push_back("Bhuudesco invisuu!");
    phraseCandidates.push_back("Bhuuesco marana!");
    phraseCandidates.push_back("Crudux cruo!");
    phraseCandidates.push_back("Cruento paashaeximus!");
    phraseCandidates.push_back("Cruento pestis shatruex!");
    phraseCandidates.push_back("Cruo crunatus durbe!");
    phraseCandidates.push_back("Cruo lokemundux!");
    phraseCandidates.push_back("Cruo-stragaraNa!");
    phraseCandidates.push_back("Gero shay cruo!");
    phraseCandidates.push_back("In marana domus-bhaava crunatus!");
    phraseCandidates.push_back("Caecux infirmux!");
    phraseCandidates.push_back("Malax sayti!");
    phraseCandidates.push_back("Marana pallex!");
    phraseCandidates.push_back("Marana malax!");
    phraseCandidates.push_back("Pallex ti!");
    phraseCandidates.push_back("Peroshay bibox malax!");
    phraseCandidates.push_back("Pestis Cruento!");
    phraseCandidates.push_back("Pestis cruento vilomaxus pretiacruento!");
    phraseCandidates.push_back("Pretaanluxis cruonit!");
    phraseCandidates.push_back("Pretiacruento!");
    phraseCandidates.push_back("StragarNaya!");
    phraseCandidates.push_back("Vorox esco marana!");
    phraseCandidates.push_back("Vilomaxus!");
    phraseCandidates.push_back("Prostragaranar malachtose!");
    phraseCandidates.push_back("Apigami!");
  }

  return phraseCandidates.at(
           engine.dice.range(0, phraseCandidates.size() - 1));
}

void Cultist::spawnStartItems() {
  const int DLVL = eng.map->getDlvl();

  const int PISTOL = 6;
  const int PUMP_SHOTGUN = PISTOL + 4;
  const int SAWN_SHOTGUN = PUMP_SHOTGUN + 3;
  const int MG = SAWN_SHOTGUN + (DLVL < 3 ? 0 : 2);

  const int TOT = MG;
  const int RND = DLVL == 0 ? PISTOL : eng.dice.range(1, TOT);

  if(RND <= PISTOL) {
    inventory_->putItemInSlot(
      slot_wielded, eng.itemFactory->spawnItem(item_pistol), true);
    if(eng.dice.percentile() < 40) {
      inventory_->putItemInGeneral(
        eng.itemFactory->spawnItem(item_pistolClip));
    }
  } else if(RND <= PUMP_SHOTGUN) {
    inventory_->putItemInSlot(
      slot_wielded, eng.itemFactory->spawnItem(item_pumpShotgun), true);
    Item* item = eng.itemFactory->spawnItem(item_shotgunShell);
    item->nrItems = eng.dice.range(5, 9);
    inventory_->putItemInGeneral(item);
  } else if(RND <= SAWN_SHOTGUN) {
    inventory_->putItemInSlot(
      slot_wielded, eng.itemFactory->spawnItem(item_sawedOff), true);
    Item* item = eng.itemFactory->spawnItem(item_shotgunShell);
    item->nrItems = eng.dice.range(6, 12);
    inventory_->putItemInGeneral(item);
  } else {
    inventory_->putItemInSlot(
      slot_wielded, eng.itemFactory->spawnItem(item_machineGun), true);
  }

  if(eng.dice.percentile() < 33) {
    inventory_->putItemInGeneral(
      eng.itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(eng.dice.percentile() < 8) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void CultistTeslaCannon::spawnStartItems() {
  Item* item = eng.itemFactory->spawnItem(item_teslaCannon);
  inventory_->putItemInSlot(slot_wielded, item, true);

  inventory_->putItemInGeneral(
    eng.itemFactory->spawnItem(item_teslaCanister));

  if(eng.dice.percentile() < 33) {
    inventory_->putItemInGeneral(
      eng.itemFactory->spawnRandomScrollOrPotion(true, true));
  }

  if(eng.dice.percentile() < 10) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void CultistSpikeGun::spawnStartItems() {
  inventory_->putItemInSlot(
    slot_wielded, eng.itemFactory->spawnItem(item_spikeGun), true);
  Item* item = eng.itemFactory->spawnItem(item_ironSpike);
  item->nrItems = 8 + eng.dice(1, 8);
  inventory_->putItemInGeneral(item);
}

void CultistPriest::spawnStartItems() {
  Item* item = eng.itemFactory->spawnItem(item_dagger);
  dynamic_cast<Weapon*>(item)->meleeDmgPlus = 2;
  inventory_->putItemInSlot(slot_wielded, item, true);

  inventory_->putItemInGeneral(
    eng.itemFactory->spawnRandomScrollOrPotion(true, true));
  inventory_->putItemInGeneral(
    eng.itemFactory->spawnRandomScrollOrPotion(true, true));

  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());

  if(eng.dice.percentile() < 33) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void FireHound::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_fireHoundBreath));
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_fireHoundBite));
}

void FrostHound::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_frostHoundBreath));
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_frostHoundBite));
}

void Zuul::place_() {
  if(eng.actorDataHandler->dataList[actor_zuul].nrLeftAllowedToSpawn > 0) {
    //Note: Do not call die() here, that would have side effects, such as
    //player getting XP.
    deadState = actorDeadState_mangled;
    vector<Monster*> monsters;
    eng.actorFactory->summonMonsters(
      pos, vector<ActorId_t> {actor_cultistPriest}, false, NULL, &monsters);
    if(monsters.empty() == false) {
      Monster* const monster = monsters.at(0);
      PropHandler& propHandler = monster->getPropHandler();
      propHandler.tryApplyProp(
        new PropPossessedByZuul(eng, propTurnsIndefinite), true);
      monster->restoreHp(999, false);
    }
  }
}

void Zuul::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_zuulBite));
}

bool Vortex::monsterSpecificOnActorTurn() {
  if(pullCooldown > 0) {
    pullCooldown--;
  }

  if(pullCooldown <= 0) {
    if(playerAwarenessCounter > 0) {
      trace << "Vortex: pullCooldown: " << pullCooldown << endl;
      trace << "Vortex: Is player aware" << endl;
      const Pos& playerPos = eng.player->pos;
      if(eng.basicUtils->isPosAdj(pos, playerPos, true) == false) {

        const int CHANCE_TO_KNOCK = 25;
        if(eng.dice.percentile() < CHANCE_TO_KNOCK) {
          trace << "Vortex: Passed random chance to pull" << endl;

          const Pos playerDelta = playerPos - pos;
          Pos knockBackFromPos = playerPos;
          if(playerDelta.x > 1)   {knockBackFromPos.x++;}
          if(playerDelta.x < -1)  {knockBackFromPos.x--;}
          if(playerDelta.y > 1)   {knockBackFromPos.y++;}
          if(playerDelta.y < -1)  {knockBackFromPos.y--;}

          if(knockBackFromPos != playerPos) {
            trace << "Vortex: Good pos found to knockback player from (";
            trace << knockBackFromPos.x << ",";
            trace << knockBackFromPos.y << ")" << endl;
            trace << "Vortex: Player position: ";
            trace << playerPos.x << "," << playerPos.y << ")" << endl;
            bool visionBlockers[MAP_W][MAP_H];
            MapParse::parse(CellPred::BlocksVision(eng), visionBlockers);
            if(checkIfSeeActor(*(eng.player), visionBlockers)) {
              trace << "Vortex: I am seeing the player" << endl;
              if(eng.player->checkIfSeeActor(*this, NULL)) {
                eng.log->addMsg("The Vortex attempts to pull me in!");
              } else {
                eng.log->addMsg("A powerful wind is pulling me!");
              }
              trace << "Vortex: Attempt pull (knockback)" << endl;
              eng.knockBack->tryKnockBack(
                *(eng.player), knockBackFromPos, false, false);
              pullCooldown = 5;
              eng.gameTime->actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void DustVortex::onMonsterDeath() {
  Explosion::runExplosionAt(
    pos, eng, 0, endOfSfx, false, new PropBlind(eng, propTurnsStd), true,
    clrGray);
}

void DustVortex::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_dustVortexEngulf));
}

void FireVortex::onMonsterDeath() {
  Explosion::runExplosionAt(
    pos, eng, 0, endOfSfx, false, new PropBurning(eng, propTurnsStd), true,
    clrRedLgt);
}

void FireVortex::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_fireVortexEngulf));
}

void FrostVortex::onMonsterDeath() {
  //TODO Add explosion with cold damage
}

void FrostVortex::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_frostVortexEngulf));
}

bool Ghost::monsterSpecificOnActorTurn() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {

      if(eng.basicUtils->isPosAdj(pos, eng.player->pos, false)) {
        if(eng.dice.percentile() < 30) {

          bool blockers[MAP_W][MAP_H];
          MapParse::parse(CellPred::BlocksVision(eng), blockers);
          const bool PLAYER_SEES_ME =
            eng.player->checkIfSeeActor(*this, blockers);
          const string refer = PLAYER_SEES_ME ? getNameThe() : "It";
          eng.log->addMsg(refer + " reaches for me... ");
          const AbilityRollResult_t rollResult = eng.abilityRoll->roll(
              eng.player->getData().abilityVals.getVal(
                ability_dodgeAttack, true, *this));
          const bool PLAYER_DODGES = rollResult >= successSmall;
          if(PLAYER_DODGES) {
            eng.log->addMsg("I dodge!", clrMsgGood);
          } else {
            eng.player->getPropHandler().tryApplyProp(
              new PropSlowed(eng, propTurnsStd));
          }
          eng.gameTime->actorDidAct();
          return true;
        }
      }
    }
  }
  return false;
}

void Ghost::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_ghostClaw));
}

void Phantasm::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_phantasmSickle));
}

void Wraith::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_wraithClaw));
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
}

void MiGo::spawnStartItems() {
  Item* item = eng.itemFactory->spawnItem(item_miGoElectricGun);
  inventory_->putItemInIntrinsics(item);

  spellsKnown.push_back(new SpellTeleport);
  spellsKnown.push_back(new SpellMiGoHypnosis);
  spellsKnown.push_back(new SpellHealSelf);

  if(eng.dice.coinToss()) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void Rat::spawnStartItems() {
  Item* item = NULL;
  if(eng.dice.percentile() < 15) {
    item = eng.itemFactory->spawnItem(item_ratBiteDiseased);
  } else {
    item = eng.itemFactory->spawnItem(item_ratBite);
  }
  inventory_->putItemInIntrinsics(item);
}

void RatThing::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_ratThingBite));
}

void Shadow::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_shadowClaw));
}

void Ghoul::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_ghoulClaw));
}

void Mummy::spawnStartItems() {
  inventory_->putItemInIntrinsics(eng.itemFactory->spawnItem(item_mummyMaul));

  for(int i = eng.dice.range(1, 3); i > 0; i--) {
    spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  }
}

void MummyUnique::spawnStartItems() {
  inventory_->putItemInIntrinsics(eng.itemFactory->spawnItem(item_mummyMaul));
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
}

bool Khephren::monsterSpecificOnActorTurn() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(hasSummonedLocusts == false) {

        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(eng), blockers);

        if(checkIfSeeActor(*(eng.player), blockers)) {
          MapParse::parse(
            CellPred::BlocksBodyType(bodyType_flying, true, eng), blockers);

          const int SPAWN_AFTER_X =
            eng.player->pos.x + FOV_STD_RADI_INT + 1;
          for(int y = 0; y  < MAP_H; y++) {
            for(int x = 0; x <= SPAWN_AFTER_X; x++) {
              blockers[x][y] = true;
            }
          }

          vector<Pos> freeCells;
          eng.basicUtils->makeVectorFromBoolMap(false, blockers, freeCells);

          sort(freeCells.begin(), freeCells.end(), IsCloserToOrigin(pos, eng));

          const unsigned int NR_OF_SPAWNS = 15;
          if(freeCells.size() >= NR_OF_SPAWNS + 1) {
            eng.log->addMsg("Khephren calls a plague of Locusts!");
            eng.player->incrShock(shockValue_heavy, shockSrc_misc);
            for(unsigned int i = 0; i < NR_OF_SPAWNS; i++) {
              Actor* const actor =
                eng.actorFactory->spawnActor(actor_giantLocust,
                                             freeCells.at(0));
              Monster* const monster = dynamic_cast<Monster*>(actor);
              monster->playerAwarenessCounter = 999;
              monster->leader = this;
              freeCells.erase(freeCells.begin());
            }
            eng.renderer->drawMapAndInterface();
            hasSummonedLocusts = true;
            eng.gameTime->actorDidAct();
            return true;
          }
        }
      }
    }
  }

  return false;
}



void DeepOne::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_deepOneJavelinAttack));
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_deepOneSpearAttack));
}

void GiantBat::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_giantBatBite));
}

void Byakhee::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_byakheeClaw));
}

void GiantMantis::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_giantMantisClaw));
}

void HuntingHorror::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_huntingHorrorBite));
}

bool KeziahMason::monsterSpecificOnActorTurn() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(hasSummonedJenkin == false) {

        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(eng), blockers);

        if(checkIfSeeActor(*(eng.player), blockers)) {

          MapParse::parse(
            CellPred::BlocksBodyType(bodyType_normal, true, eng), blockers);

          vector<Pos> line;
          eng.lineCalc->calcNewLine(pos, eng.player->pos, true, 9999,
                                    false, line);

          const int LINE_SIZE = line.size();
          for(int i = 0; i < LINE_SIZE; i++) {
            const Pos c = line.at(i);
            if(blockers[c.x][c.y] == false) {
              //TODO Make a generalized summoning funtionality
              eng.log->addMsg("Keziah summons Brown Jenkin!");
              Actor* const actor =
                eng.actorFactory->spawnActor(actor_brownJenkin, c);
              Monster* jenkin = dynamic_cast<Monster*>(actor);
              eng.renderer->drawMapAndInterface();
              hasSummonedJenkin = true;
              jenkin->playerAwarenessCounter = 999;
              jenkin->leader = this;
              eng.gameTime->actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }

  return false;
}

void KeziahMason::spawnStartItems() {
  spellsKnown.push_back(new SpellTeleport);
  spellsKnown.push_back(new SpellHealSelf);
  spellsKnown.push_back(new SpellSummonRandom);
  spellsKnown.push_back(new SpellAzathothsWrath);
  spellsKnown.push_back(eng.spellHandler->getRandomSpellForMonster());
}

void Ooze::onStandardTurn_() {
  restoreHp(1, false);
}

void OozeBlack::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_oozeBlackSpewPus));
}

void OozeClear::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_oozeClearSpewPus));
}

void OozePutrid::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_oozePutridSpewPus));
}

void OozePoison::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_oozePoisonSpewPus));
}

void ColourOutOfSpace::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_colourOutOfSpaceTouch));
}

const SDL_Color& ColourOutOfSpace::getColor() {
  return currentColor;
}

void ColourOutOfSpace::onStandardTurn_() {
  currentColor.r = eng.dice.range(40, 255);
  currentColor.g = eng.dice.range(40, 255);
  currentColor.b = eng.dice.range(40, 255);

  restoreHp(1, false);

  if(eng.player->checkIfSeeActor(*this, NULL)) {
    eng.player->getPropHandler().tryApplyProp(
      new PropConfused(eng, propTurnsStd));
  }
}

bool Spider::monsterSpecificOnActorTurn() {
  return false;
}

void GreenSpider::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_greenSpiderBite));
}

void WhiteSpider::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_whiteSpiderBite));
}

void RedSpider::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_redSpiderBite));
}

void ShadowSpider::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_shadowSpiderBite));
}

void LengSpider::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_lengSpiderBite));
}

void Wolf::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_wolfBite));
}

bool WormMass::monsterSpecificOnActorTurn() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(eng.dice.percentile() < chanceToSpawnNew) {

        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksBodyType(getBodyType(), true, eng),
                         blockers);

        Pos spawnPos;
        for(int dx = -1; dx <= 1; dx++) {
          for(int dy = -1; dy <= 1; dy++) {
            spawnPos.set(pos + Pos(dx, dy));
            if(blockers[spawnPos.x][spawnPos.y] == false) {
              Actor* const actor =
                eng.actorFactory->spawnActor(data_->id, spawnPos);
              WormMass* const worm = dynamic_cast<WormMass*>(actor);
              chanceToSpawnNew -= 4;
              worm->chanceToSpawnNew = chanceToSpawnNew;
              worm->playerAwarenessCounter = playerAwarenessCounter;
              eng.gameTime->actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void WormMass::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_wormMassBite));
}

bool GiantLocust::monsterSpecificOnActorTurn() {
  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(eng.dice.percentile() < chanceToSpawnNew) {

        bool blockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksBodyType(getBodyType(), true, eng),
                         blockers);

        Pos spawnPos;
        for(int dx = -1; dx <= 1; dx++) {
          for(int dy = -1; dy <= 1; dy++) {
            spawnPos.set(pos + Pos(dx, dy));
            if(blockers[spawnPos.x][spawnPos.y] == false) {
              Actor* const actor =
                eng.actorFactory->spawnActor(data_->id, spawnPos);
              GiantLocust* const locust = dynamic_cast<GiantLocust*>(actor);
              chanceToSpawnNew -= 2;
              locust->chanceToSpawnNew = chanceToSpawnNew;
              locust->playerAwarenessCounter = playerAwarenessCounter;
              eng.gameTime->actorDidAct();
              return true;
            }
          }
        }
      }
    }
  }
  return false;
}

void GiantLocust::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_giantLocustBite));
}

bool LordOfShadows::monsterSpecificOnActorTurn() {

  return false;
}

void LordOfShadows::spawnStartItems() {

}

bool LordOfSpiders::monsterSpecificOnActorTurn() {
  if(playerAwarenessCounter > 0) {

    if(eng.dice.coinToss()) {

      const Pos playerPos = eng.player->pos;

      if(eng.player->checkIfSeeActor(*this, NULL)) {
        eng.log->addMsg(data_->spellCastMessage);
      }

      for(int dy = -1; dy <= 1; dy++) {
        for(int dx = -1; dx <= 1; dx++) {

          if(eng.dice.percentile() < 75) {

            const Pos c(playerPos + Pos(dx, dy));
            const FeatureStatic* const mimicFeature =
              eng.map->cells[c.x][c.y].featureStatic;

            if(mimicFeature->canHaveStaticFeature()) {

              const FeatureData* const mimicData =
                eng.featureDataHandler->getData(
                  mimicFeature->getId());

              Feature* const f =
                eng.featureFactory->spawnFeatureAt(
                  feature_trap, c,
                  new TrapSpawnData(mimicData, trap_spiderWeb));

              dynamic_cast<Trap*>(f)->reveal(false);
            }
          }
        }
      }
    }
  }
  return false;
}

void LordOfSpiders::spawnStartItems() {

}

bool LordOfSpirits::monsterSpecificOnActorTurn() {

  return false;
}

void LordOfSpirits::spawnStartItems() {

}

bool LordOfPestilence::monsterSpecificOnActorTurn() {

  return false;
}

void LordOfPestilence::spawnStartItems() {

}

bool Zombie::monsterSpecificOnActorTurn() {
  return tryResurrect();
}

bool MajorClaphamLee::monsterSpecificOnActorTurn() {
  if(tryResurrect()) {
    return true;
  }

  if(deadState == actorDeadState_alive) {
    if(playerAwarenessCounter > 0) {
      if(hasSummonedTombLegions == false) {

        bool visionBlockers[MAP_W][MAP_H];
        MapParse::parse(CellPred::BlocksVision(eng), visionBlockers);

        if(checkIfSeeActor(*(eng.player), visionBlockers)) {
          eng.log->addMsg("Major Clapham Lee calls forth his Tomb-Legions!");
          vector<ActorId_t> monsterIds;
          monsterIds.resize(0);

          monsterIds.push_back(actor_deanHalsey);

          const int NR_OF_EXTRA_SPAWNS = 4;

          for(int i = 0; i < NR_OF_EXTRA_SPAWNS; i++) {
            const int ZOMBIE_TYPE = eng.dice.range(1, 3);
            ActorId_t id = actor_zombie;
            switch(ZOMBIE_TYPE) {
              case 1: id = actor_zombie;        break;
              case 2: id = actor_zombieAxe;     break;
              case 3: id = actor_bloatedZombie; break;
            }
            monsterIds.push_back(id);
          }
          eng.actorFactory->summonMonsters(pos, monsterIds, true, this);
          eng.renderer->drawMapAndInterface();
          hasSummonedTombLegions = true;
          eng.player->incrShock(shockValue_heavy, shockSrc_misc);
          eng.gameTime->actorDidAct();
          return true;
        }
      }
    }
  }

  return false;
}

bool Zombie::tryResurrect() {
  if(deadState == actorDeadState_corpse) {
    if(hasResurrected == false) {
      deadTurnCounter += 1;
      if(deadTurnCounter > 5) {
        if(pos != eng.player->pos && eng.dice.percentile() < 7) {
          deadState = actorDeadState_alive;
          hp_ = (getHpMax(true) * 3) / 4;
          glyph_ = data_->glyph;
          tile_ = data_->tile;
          clr_ = data_->color;
          hasResurrected = true;
          data_->nrOfKills--;
          if(eng.map->cells[pos.x][pos.y].isSeenByPlayer) {
            eng.log->addMsg(
              getNameThe() + " rises again!!", clrWhite, true);
            eng.player->incrShock(shockValue_some, shockSrc_misc);
          }

          playerAwarenessCounter = data_->nrTurnsAwarePlayer * 2;
          eng.gameTime->actorDidAct();
          return true;
        }
      }
    }
  }
  return false;
}

void Zombie::die_() {
  //If resurrected once and has corpse, blow up the corpse
  if(hasResurrected && deadState == actorDeadState_corpse) {
    deadState = actorDeadState_mangled;
    eng.gore->makeBlood(pos);
    eng.gore->makeGore(pos);
  }
}

void ZombieClaw::spawnStartItems() {
  Item* item = NULL;
  if(eng.dice.percentile() < 20) {
    item = eng.itemFactory->spawnItem(item_zombieClawDiseased);
  } else {
    item = eng.itemFactory->spawnItem(item_zombieClaw);
  }
  inventory_->putItemInIntrinsics(item);
}

void ZombieAxe::spawnStartItems() {
  inventory_->putItemInIntrinsics(eng.itemFactory->spawnItem(item_zombieAxe));
}

void BloatedZombie::spawnStartItems() {
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_bloatedZombiePunch));
  inventory_->putItemInIntrinsics(
    eng.itemFactory->spawnItem(item_bloatedZombieSpit));
}

