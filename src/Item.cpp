#include "Init.h"

#include "Item.h"

#include <assert.h>

#include "Map.h"
#include "GameTime.h"
#include "Utils.h"
#include "MapParsing.h"
#include "Properties.h"
#include "Log.h"
#include "Explosion.h"
#include "Render.h"
#include "Input.h"
#include "Query.h"
#include "ItemFactory.h"
#include "FeatureMob.h"
#include "FeatureRigid.h"
#include "ItemData.h"

using namespace std;

//--------------------------------------------------------- ITEM
Item::Item(ItemDataT* itemData) :
    nrItems_      (1),
    meleeDmgPlus_ (0),
    data_         (itemData) {}

Item::~Item()
{
    for (auto prop   : carrierProps_)   {delete prop;}
    for (auto spell  : carrierSpells_)  {delete spell;}
}

ItemId            Item::getId()     const {return data_->id;}
const ItemDataT&  Item::getData()   const {return *data_;}
Clr               Item::getClr()    const {return data_->clr;}
char              Item::getGlyph()  const {return data_->glyph;}
TileId            Item::getTile()   const {return data_->tile;}

vector<string> Item::getDescr() const
{
    return data_->baseDescr;
}

int Item::getWeight() const
{
    return int(data_->weight) * nrItems_;
}

string Item::getWeightStr() const
{
    const int WEIGHT = getWeight();
    if (WEIGHT <= (int(ItemWeight::extraLight) + int(ItemWeight::light)) / 2)
    {
        return "very light";
    }
    if (WEIGHT <= (int(ItemWeight::light) + int(ItemWeight::medium)) / 2)
    {
        return "light";
    }
    if (WEIGHT <= (int(ItemWeight::medium) + int(ItemWeight::heavy)) / 2)
    {
        return "a bit heavy";
    }
    return "heavy";
}

ConsumeItem Item::activate(Actor* const actor)
{
    (void)actor;
    Log::addMsg("I cannot apply that.");
    return ConsumeItem::no;
}

string Item::getName(const ItemRefType      refType,
                     const ItemRefInf       inf,
                     const ItemRefAttInf    attInf) const
{
    ItemRefType refTypeUsed = refType;

    if (refType == ItemRefType::plural && (!data_->isStackable || nrItems_ == 1))
    {
        refTypeUsed = ItemRefType::a;
    }

    string nrStr = "";

    if (refTypeUsed == ItemRefType::plural)
    {
        nrStr = toStr(nrItems_) + " ";
    }

    string attStr = "";

    ItemRefAttInf attInfUsed = attInf;

    if (attInf == ItemRefAttInf::wpnContext)
    {
        switch (data_->mainAttMode)
        {
        case MainAttMode::melee:  attInfUsed = ItemRefAttInf::melee;  break;
        case MainAttMode::ranged: attInfUsed = ItemRefAttInf::ranged; break;
        case MainAttMode::thrown: attInfUsed = ItemRefAttInf::thrown; break;
        case MainAttMode::none:   attInfUsed = ItemRefAttInf::none;   break;
        }
    }

    const auto abilityVals = Map::player->getData().abilityVals;

    if (attInfUsed == ItemRefAttInf::melee)
    {
        const string    rollsStr    = toStr(data_->melee.dmg.first);
        const string    sidesStr    = toStr(data_->melee.dmg.second);
        const int       PLUS        = meleeDmgPlus_;
        const string    plusStr     = PLUS == 0 ? "" :
                                      PLUS  > 0 ?
                                      ("+" + toStr(PLUS)) :
                                      ("-" + toStr(PLUS));
        const int       ITEM_SKILL  = data_->melee.hitChanceMod;
        const int       MELEE_SKILL = abilityVals.getVal(AbilityId::melee, true,
                                      *(Map::player));
        const int       SKILL_TOT   = max(0, min(100, ITEM_SKILL + MELEE_SKILL));
        const string    skillStr    = toStr(SKILL_TOT) + "%";

        attStr = " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr;
    }

    const int RANGED_SKILL = abilityVals.getVal(AbilityId::ranged, true, *(Map::player));

    if (attInfUsed == ItemRefAttInf::ranged)
    {
        string dmgStr = data_->ranged.dmgInfoOverride;

        if (dmgStr.empty())
        {
            const int       MULTIPL     = data_->ranged.isMachineGun ?
                                          NR_MG_PROJECTILES : 1;
            const string    rollsStr    = toStr(data_->ranged.dmg.rolls * MULTIPL);
            const string    sidesStr    = toStr(data_->ranged.dmg.sides);
            const int       PLUS        = data_->ranged.dmg.plus * MULTIPL;
            const string    plusStr     = PLUS ==  0 ? "" :
                                          PLUS  > 0  ?
                                          ("+" + toStr(PLUS)) :
                                          ("-" + toStr(PLUS));
            dmgStr                      = rollsStr + "d" + sidesStr + plusStr;
        }
        const int       ITEM_SKILL  = data_->ranged.hitChanceMod;
        const int       SKILL_TOT   = max(0, min(100, ITEM_SKILL + RANGED_SKILL));
        const string    skillStr    = toStr(SKILL_TOT) + "%";

        attStr = " " + dmgStr + " " + skillStr;
    }

    if (attInfUsed == ItemRefAttInf::thrown)
    {
        const string    rollsStr    = toStr(data_->ranged.throwDmg.rolls);
        const string    sidesStr    = toStr(data_->ranged.throwDmg.sides);
        const int       PLUS        = data_->ranged.throwDmg.plus;
        const string    plusStr     = PLUS ==  0 ? "" :
                                      PLUS  > 0 ? "+" :
                                      ("-" + toStr(PLUS));
        const int       ITEM_SKILL  = data_->ranged.throwHitChanceMod;
        const int       SKILL_TOT   = max(0, min(100, ITEM_SKILL + RANGED_SKILL));
        const string    skillStr    = toStr(SKILL_TOT) + "%";

        attStr = " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr;
    }

    string infStr = "";

    if (inf == ItemRefInf::yes)
    {
        infStr = getNameInf();
        if (!infStr.empty()) {infStr.insert(0, " ");}
    }

    const auto& namesUsed = data_->isIdentified ? data_->baseName : data_->baseNameUnId;

    return nrStr + namesUsed.names[int(refTypeUsed)] + attStr + infStr;
}

bool Item::isInEffectiveRangeLmt(const Pos& p0, const Pos& p1) const
{
    return Utils::kingDist(p0, p1) <= data_->ranged.effectiveRange;
}

//--------------------------------------------------------- ARMOR
Armor::Armor(ItemDataT* const itemData) :
    Item  (itemData),
    dur_  (Rnd::range(80, 100)) {}

void Armor::storeToSaveLines(vector<string>& lines)
{
    lines.push_back(toStr(dur_));
}

void Armor::setupFromSaveLines(vector<string>& lines)
{
    dur_ = toInt(lines.front());
    lines.erase(begin(lines));
}

void Armor::onEquip()
{
    onEquip_();
}

UnequipAllowed Armor::onUnequip()
{
    Render::drawMapAndInterface();

    const UnequipAllowed unequipAllowed = onUnequip_();

    if (unequipAllowed == UnequipAllowed::yes)
    {
        const string name = getName(ItemRefType::plain, ItemRefInf::none);
        Log::addMsg("I take off my " + name + ".", clrWhite, false, true);
    }

    GameTime::tick();

    return unequipAllowed;
}

string Armor::getArmorDataLine(const bool WITH_BRACKETS) const
{
    const int       AP      = getArmorPoints();
    const string    apStr   = toStr(max(0, AP));

    return WITH_BRACKETS ? ("[" + apStr + "]") : apStr;
}

int Armor::takeDurHitAndGetReducedDmg(const int DMG_BEFORE)
{
    TRACE_FUNC_BEGIN;

    //AP, Armor points:
    //  - Damage soaked up instead of hitting the player
    //DFF, Damage (to) Durability Factor:
    //  - A factor of how much damage the armor durability takes per attack damage point

    const int     AP_BEFORE       = getArmorPoints();
    const double  DDF_BASE        = data_->armor.dmgToDurabilityFactor;
    //TODO: Add check for if wearer is player!
    const double  DDF_WAR_VET_MOD = PlayerBon::getBg() == Bg::warVet ? 0.5 : 1.0;
    const double  DDF_K           = 1.5;
    const double  DMG_BEFORE_DB   = double(DMG_BEFORE);

    dur_ -= int(DMG_BEFORE_DB * DDF_BASE * DDF_WAR_VET_MOD * DDF_K);

    dur_                          = max(0, dur_);
    const int AP_AFTER            = getArmorPoints();

    if (AP_AFTER < AP_BEFORE && AP_AFTER != 0)
    {
        const string armorName = getName(ItemRefType::plain);
        Log::addMsg("My " + armorName + " is damaged!", clrMsgNote);
    }

    TRACE << "Damage before: " + toStr(DMG_BEFORE) << endl;

    const int DMG_AFTER = max(1, DMG_BEFORE - AP_BEFORE);

    TRACE << "Damage after: " + toStr(DMG_AFTER) << endl;

    TRACE_FUNC_END;
    return DMG_AFTER;
}

int Armor::getArmorPoints() const
{
    const int AP_MAX = data_->armor.armorPoints;

    if (dur_ > 60) {return AP_MAX;}
    if (dur_ > 40) {return max(0, AP_MAX - 1);}
    if (dur_ > 25) {return max(0, AP_MAX - 2);}
    if (dur_ > 15) {return max(0, AP_MAX - 3);}

    return 0;
}

void ArmorAsbSuit::onEquip_()
{
    carrierProps_.push_back(new PropRFire   (PropTurns::indefinite));
    carrierProps_.push_back(new PropRAcid   (PropTurns::indefinite));
    carrierProps_.push_back(new PropRElec   (PropTurns::indefinite));
    carrierProps_.push_back(new PropRBreath (PropTurns::indefinite));
}

UnequipAllowed ArmorAsbSuit::onUnequip_()
{
    for (Prop* prop : carrierProps_) {delete prop;}
    carrierProps_.clear();

    return UnequipAllowed::yes;
}

void ArmorHeavyCoat::onEquip_()
{
    carrierProps_.push_back(new PropRCold(PropTurns::indefinite));
}

UnequipAllowed ArmorHeavyCoat::onUnequip_()
{
    for (Prop* prop : carrierProps_) {delete prop;}
    carrierProps_.clear();

    return UnequipAllowed::yes;
}

void ArmorMiGo::onStdTurnInInv(const InvType invType)
{
    (void)invType;

    if (dur_ < 100)
    {
        const int AP_BEFORE = getArmorPoints();

        dur_ = 100;

        const int AP_AFTER  = getArmorPoints();

        if (AP_AFTER > AP_BEFORE)
        {
            const string name = getName(ItemRefType::plain, ItemRefInf::none);
            Log::addMsg("My " + name + " reconstructs itself.", clrMsgNote, false, true);
        }
    }
}

void ArmorMiGo::onEquip_()
{
    Render::drawMapAndInterface();
    Log::addMsg("The armor joins with my skin!", clrWhite, false, true);
    Map::player->incrShock(ShockLvl::heavy, ShockSrc::useStrangeItem);
}

UnequipAllowed ArmorMiGo::onUnequip_()
{
    Render::drawMapAndInterface();
    Log::addMsg("I attempt to tear off the armor, it rips my skin!", clrMsgBad, false,
                true);

    Map::player->hit(Rnd::range(1, 3), DmgType::pure);

    if (Rnd::coinToss())
    {
        //Note: There is no need to print a message here, a message is always printed when
        //taking off armor.
        return UnequipAllowed::yes;
    }
    else
    {
        Log::addMsg("I fail to tear it off.", clrWhite, false, true);
        return UnequipAllowed::no;
    }
}

//--------------------------------------------------------- WEAPON
void Wpn::storeToSaveLines(vector<string>& lines)
{
    lines.push_back(toStr(meleeDmgPlus_));
    lines.push_back(toStr(nrAmmoLoaded));
}

void Wpn::setupFromSaveLines(vector<string>& lines)
{
    meleeDmgPlus_ = toInt(lines.front());
    lines.erase(begin(lines));
    nrAmmoLoaded = toInt(lines.front());
    lines.erase(begin(lines));
}

Wpn::Wpn(ItemDataT* const itemData, ItemDataT* const ammoData, int ammoCap,
         bool isUsingClip) :
    Item                (itemData),
    AMMO_CAP            (ammoCap),
    IS_USING_CLIP       (isUsingClip),
    nrAmmoLoaded        (AMMO_CAP),
    ammoData_           (ammoData) {}

Clr Wpn::getClr() const
{
    if (data_->ranged.isRangedWpn && !data_->ranged.hasInfiniteAmmo)
    {
        if (nrAmmoLoaded == 0)
        {
            Clr ret = data_->clr;
            ret.r /= 2; ret.g /= 2; ret.b /= 2;
            return ret;
        }
    }
    return data_->clr;
}

void Wpn::setRandomMeleePlus()
{
    meleeDmgPlus_ = 0;

    int chance = 45;
    while (Rnd::percent() < chance && meleeDmgPlus_ < 3)
    {
        meleeDmgPlus_++;
        chance -= 5;
    }
}

string Wpn::getNameInf() const
{
    if (data_->ranged.isRangedWpn && !data_->ranged.hasInfiniteAmmo)
    {
        return toStr(nrAmmoLoaded) + "/" + toStr(AMMO_CAP);
    }
    return "";
}

//--------------------------------------------------------- STAFF OF THE PHARAOHS
PharaohStaff::PharaohStaff(ItemDataT* const itemData) : Wpn(itemData, nullptr)
{
    itemData->allowSpawn = false;

    carrierSpells_.push_back(new SpellPharaohStaff);
}

//--------------------------------------------------------- MACHINE GUN
MachineGun::MachineGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData, ammoData->ranged.maxNrAmmoInClip, true) {}

//--------------------------------------------------------- MI-GO ELECTRIC GUN
MiGoGun::MiGoGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData, ammoData->ranged.maxNrAmmoInClip, true) {}

//--------------------------------------------------------- SPIKE GUN
SpikeGun::SpikeGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData, 12, false) {}

//--------------------------------------------------------- INCINERATOR
Incinerator::Incinerator(ItemDataT* const itemData, ItemDataT* const ammoData) :
    Wpn(itemData, ammoData, ammoData->ranged.maxNrAmmoInClip, true) {}

void Incinerator::onProjectileBlocked(
    const Pos& pos, Actor* actorHit)
{
    (void)actorHit;
    Explosion::runExplosionAt(pos, ExplType::expl);
}

//--------------------------------------------------------- AMMO CLIP
AmmoClip::AmmoClip(ItemDataT* const itemData) : Ammo(itemData)
{
    setFullAmmo();
}

void AmmoClip::setFullAmmo()
{
    ammo_ = data_->ranged.maxNrAmmoInClip;
}

//--------------------------------------------------------- MEDICAL BAG
const int NR_TRN_BEFORE_HEAL  = 10;
const int NR_TRN_PER_HP       = 2;

void MedicalBag::onPickupToBackpack(Inventory& inv)
{
    //Check for existing medical bag in inventory
    for (Item* const other : inv.general_)
    {
        if (other != this && other->getId() == getId())
        {
            //Add my turns left to the other medical bag, then destroy self
            static_cast<MedicalBag*>(other)->nrSupplies_ += nrSupplies_;
            inv.removeItemInBackpackWithPtr(this, true);
            return;
        }
    }
}

ConsumeItem MedicalBag::activate(Actor* const actor)
{
    (void)actor;

    vector<Actor*> seenFoes;
    Map::player->getSeenFoes(seenFoes);
    if (!seenFoes.empty())
    {
        Log::addMsg("Not while an enemy is near.");
        curAction_ = MedBagAction::END;
        return ConsumeItem::no;
    }

    curAction_ = chooseAction();

    Log::clearLog();

    if (curAction_ == MedBagAction::END)
    {
        return ConsumeItem::no;
    }

    //Check if chosen action can be done
    bool props[size_t(PropId::END)];
    Map::player->getPropHandler().getPropIds(props);
    switch (curAction_)
    {
    case MedBagAction::treatWounds:
        if (Map::player->getHp() >= Map::player->getHpMax(true))
        {
            Log::addMsg("I have no wounds to treat.");
            curAction_ = MedBagAction::END;
            return ConsumeItem::no;
        }
        break;

    case MedBagAction::sanitizeInfection:
        if (!props[int(PropId::infected)])
        {
            Log::addMsg("I have no infection to sanitize.");
            curAction_ = MedBagAction::END;
            return ConsumeItem::no;
        }
        break;

    case MedBagAction::END: {}
        break;
    }

    bool isEnoughSuppl = true;

    switch (curAction_)
    {
    case MedBagAction::sanitizeInfection:
        isEnoughSuppl = getTotSupplForSanitize() <= nrSupplies_;
        break;

    case MedBagAction::treatWounds: //Costs one supply per turn
    case MedBagAction::END: {}
        break;
    }

    if (!isEnoughSuppl)
    {
        Log::addMsg("I do not have enough supplies for that.");
        curAction_ = MedBagAction::END;
        return ConsumeItem::no;
    }

    //Action can be done
    Map::player->activeMedicalBag = this;

    switch (curAction_)
    {
    case MedBagAction::treatWounds:
        Log::addMsg("I start treating my wounds...");
        nrTurnsUntilHealWounds_ = NR_TRN_BEFORE_HEAL;
        break;

    case MedBagAction::sanitizeInfection:
        Log::addMsg("I start to sanitize an infection...");
        nrTurnsLeftSanitize_ = getTotTurnsForSanitize();
        break;

    case MedBagAction::END: {} break;
    }

    GameTime::tick();

    return ConsumeItem::no;
}

MedBagAction MedicalBag::chooseAction() const
{
    Log::clearLog();

    bool props[size_t(PropId::END)];
    Map::player->getPropHandler().getPropIds(props);

    //Infections are treated first
    if (props[int(PropId::infected)])
    {
        return MedBagAction::sanitizeInfection;
    }

    return MedBagAction::treatWounds;


//  Log::addMsg("Use Medical Bag how? [h/enter] Treat wounds [s] Sanitize infection",
//              clrWhiteHigh);

//  Render::drawMapAndInterface(true);
//
//  while (true)
//  {
//    const KeyData d = Query::letter(true);
//    if (d.sdlKey == SDLK_ESCAPE || d.sdlKey == SDLK_SPACE)
//    {
//      return MedBagAction::END;
//    }
//    else if (d.sdlKey == SDLK_RETURN || d.key == 'h')
//    {
//      return MedBagAction::treatWounds;
//    }
//    else if (d.key == 's')
//    {
//      return MedBagAction::sanitizeInfection;
//    }
//  }
//
//  return MedBagAction(MedBagAction::END);
}

void MedicalBag::continueAction()
{
    switch (curAction_)
    {
    case MedBagAction::treatWounds:
    {

        auto& player = *Map::player;

        const bool IS_HEALER = PlayerBon::traits[int(Trait::healer)];

        if (nrTurnsUntilHealWounds_ > 0)
        {
            nrTurnsUntilHealWounds_ -= IS_HEALER ? 2 : 1;
        }
        else
        {
            //If player is healer, double the rate of HP healing.
            const int NR_TRN_PER_HP_W_BON =
                IS_HEALER ? (NR_TRN_PER_HP / 2) : NR_TRN_PER_HP;

            if (GameTime::getTurn() % NR_TRN_PER_HP_W_BON == 0)
            {
                player.restoreHp(1, false);
            }

            //The rate of supply use is consistent (this means that with the healer
            // trait, you spend half the time and supplies, as per the description).
            if (GameTime::getTurn() % NR_TRN_PER_HP == 0)
            {
                --nrSupplies_;
            }
        }

        if (nrSupplies_ <= 0)
        {
            Log::addMsg("No more medical supplies.");
            finishCurAction();
            return;
        }

        if (player.getHp() >= player.getHpMax(true))
        {
            finishCurAction();
            return;
        }

        GameTime::tick();

    } break;

    case MedBagAction::sanitizeInfection:
    {
        --nrTurnsLeftSanitize_;
        if (nrTurnsLeftSanitize_ <= 0)
        {
            finishCurAction();
        }
        else
        {
            GameTime::tick();
        }
    } break;

    case MedBagAction::END:
    {
        assert(false && "Illegal action");
    }
    break;
    }
}

void MedicalBag::finishCurAction()
{
    Map::player->activeMedicalBag = nullptr;

    switch (curAction_)
    {
    case MedBagAction::sanitizeInfection:
    {
        Map::player->getPropHandler().endAppliedProp(PropId::infected);
        nrSupplies_ -= getTotSupplForSanitize();
    } break;

    case MedBagAction::treatWounds:
    {
        Log::addMsg("I finish treating my wounds.");
    } break;

    case MedBagAction::END: {} break;
    }

    curAction_ = MedBagAction::END;

    if (nrSupplies_ <= 0)
    {
        Map::player->getInv().removeItemInBackpackWithPtr(this, true);
    }
}

void MedicalBag::interrupted()
{
    Log::addMsg("My healing is disrupted.", clrWhite, false);

    nrTurnsUntilHealWounds_ = -1;
    nrTurnsLeftSanitize_    = -1;

    Map::player->activeMedicalBag = nullptr;
}

int MedicalBag::getTotTurnsForSanitize() const
{
    return PlayerBon::traits[int(Trait::healer)] ? 10 : 20;
}

int MedicalBag::getTotSupplForSanitize() const
{
    return PlayerBon::traits[int(Trait::healer)] ? 5 : 10;
}

//--------------------------------------------------------- HIDEOUS MASK
HideousMask::HideousMask(ItemDataT* itemData) : Headwear(itemData)
{
    itemData->allowSpawn = false;
}

void HideousMask::onStdTurnInInv(const InvType invType)
{
    if (invType == InvType::slots)
    {
        vector<Actor*> adjActors;
        const Pos p(Map::player->pos);
        for (auto* const actor : GameTime::actors_)
        {
            if (actor->isAlive() && Utils::isPosAdj(p, actor->pos, false))
            {
                adjActors.push_back(actor);
            }
        }
        if (!adjActors.empty())
        {
            bool blockedLos[MAP_W][MAP_H];
            MapParse::run(CellCheck::BlocksLos(), blockedLos);
            for (auto* const actor : adjActors)
            {
                if (Rnd::oneIn(4) && actor->canSeeActor(*Map::player, blockedLos))
                {
                    actor->getPropHandler().tryApplyProp(
                        new PropTerrified(PropTurns::std));
                }
            }
        }
    }
}

//--------------------------------------------------------- GAS MASK
void GasMask::onEquip()
{
    carrierProps_.push_back(new PropRBreath(PropTurns::indefinite));
}

UnequipAllowed GasMask::onUnequip()
{
    for (Prop* prop : carrierProps_) {delete prop;}
    carrierProps_.clear();

    return UnequipAllowed::yes;
}

void GasMask::decrTurnsLeft(Inventory& carrierInv)
{
    --nrTurnsLeft_;

    if (nrTurnsLeft_ <= 0)
    {
        Log::addMsg("My " + getName(ItemRefType::plain, ItemRefInf::none) + " expires.",
                    clrMsgNote, true, true);
        carrierInv.decrItem(this);
    }
}

//--------------------------------------------------------- EXPLOSIVE
ConsumeItem Explosive::activate(Actor* const actor)
{
    (void)actor;
    //Make a copy to use as the held ignited explosive.
    auto* cpy = static_cast<Explosive*>(ItemFactory::mk(getData().id, 1));

    cpy->fuseTurns_               = getStdFuseTurns();
    Map::player->activeExplosive  = cpy;
    Map::player->updateClr();
    cpy->onPlayerIgnite();
    return ConsumeItem::yes;
}

//--------------------------------------------------------- DYNAMITE
void Dynamite::onPlayerIgnite() const
{
    const bool IS_SWIFT   = PlayerBon::traits[int(Trait::demExpert)] && Rnd::coinToss();
    const string swiftStr = IS_SWIFT ? "swiftly " : "";

    Log::addMsg("I " + swiftStr + "light a dynamite stick.");
    Render::drawMapAndInterface();
    GameTime::tick(IS_SWIFT);
}

void Dynamite::onStdTurnPlayerHoldIgnited()
{
    fuseTurns_--;
    if (fuseTurns_ > 0)
    {
        string fuseMsg = "***F";
        for (int i = 0; i < fuseTurns_; ++i) {fuseMsg += "Z";}
        fuseMsg += "***";
        Log::addMsg(fuseMsg, clrYellow);
    }
    else
    {
        Log::addMsg("The dynamite explodes in my hand!");
        Map::player->activeExplosive = nullptr;
        Explosion::runExplosionAt(Map::player->pos, ExplType::expl);
        Map::player->updateClr();
        fuseTurns_ = -1;
        delete this;
    }
}

void Dynamite::onThrownIgnitedLanding(const Pos& p)
{
    GameTime::addMob(new LitDynamite(p, fuseTurns_));
}

void Dynamite::onPlayerParalyzed()
{
    Log::addMsg("The lit Dynamite stick falls from my hand!");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    const Pos& p = Map::player->pos;
    auto* const f = Map::cells[p.x][p.y].rigid;
    if (!f->isBottomless()) {GameTime::addMob(new LitDynamite(p, fuseTurns_));}
    delete this;
}

//--------------------------------------------------------- MOLOTOV
void Molotov::onPlayerIgnite() const
{
    const bool IS_SWIFT   = PlayerBon::traits[int(Trait::demExpert)] &&
                            Rnd::coinToss();
    const string swiftStr = IS_SWIFT ? "swiftly " : "";

    Log::addMsg("I " + swiftStr + "light a Molotov Cocktail.");
    Render::drawMapAndInterface();
    GameTime::tick(IS_SWIFT);
}

void Molotov::onStdTurnPlayerHoldIgnited()
{
    fuseTurns_--;

    if (fuseTurns_ <= 0)
    {
        Log::addMsg("The Molotov Cocktail explodes in my hand!");
        Map::player->activeExplosive = nullptr;
        Map::player->updateClr();
        Explosion::runExplosionAt(Map::player->pos, ExplType::applyProp, ExplSrc::misc, 0,
                                  SfxId::explosionMolotov, new PropBurning(PropTurns::std));
        delete this;
    }
}

void Molotov::onThrownIgnitedLanding(const Pos& p)
{
    const int D = PlayerBon::traits[int(Trait::demExpert)] ? 1 : 0;
    Explosion::runExplosionAt(p, ExplType::applyProp, ExplSrc::playerUseMoltvIntended, D,
                              SfxId::explosionMolotov, new PropBurning(PropTurns::std));
}


void Molotov::onPlayerParalyzed()
{
    Log::addMsg("The lit Molotov Cocktail falls from my hand!");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    Explosion::runExplosionAt(Map::player->pos, ExplType::applyProp, ExplSrc::misc, 0,
                              SfxId::explosionMolotov, new PropBurning(PropTurns::std));
    delete this;
}

//--------------------------------------------------------- FLARE
void Flare::onPlayerIgnite() const
{
    const bool IS_SWIFT   = PlayerBon::traits[int(Trait::demExpert)] &&
                            Rnd::coinToss();
    const string swiftStr = IS_SWIFT ? "swiftly " : "";

    Log::addMsg("I " + swiftStr + "light a Flare.");
    GameTime::updateLightMap();
    Map::player->updateFov();
    Render::drawMapAndInterface();
    GameTime::tick(IS_SWIFT);
}

void Flare::onStdTurnPlayerHoldIgnited()
{
    fuseTurns_--;
    if (fuseTurns_ <= 0)
    {
        Log::addMsg("The flare is extinguished.");
        Map::player->activeExplosive = nullptr;
        Map::player->updateClr();
        delete this;
    }
}

void Flare::onThrownIgnitedLanding(const Pos& p)
{
    GameTime::addMob(new LitFlare(p, fuseTurns_));
    GameTime::updateLightMap();
    Map::player->updateFov();
    Render::drawMapAndInterface();
}

void Flare::onPlayerParalyzed()
{
    Log::addMsg("The lit Flare falls from my hand!");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    const Pos&  p = Map::player->pos;
    auto* const f = Map::cells[p.x][p.y].rigid;
    if (!f->isBottomless()) {GameTime::addMob(new LitFlare(p, fuseTurns_));}
    GameTime::updateLightMap();
    Map::player->updateFov();
    Render::drawMapAndInterface();
    delete this;
}

//--------------------------------------------------------- SMOKE GRENADE
void SmokeGrenade::onPlayerIgnite() const
{
    const bool IS_SWIFT   = PlayerBon::traits[int(Trait::demExpert)] &&
                            Rnd::coinToss();
    const string swiftStr = IS_SWIFT ? "swiftly " : "";

    Log::addMsg("I " + swiftStr + "ignite a smoke grenade.");
    Render::drawMapAndInterface();
    GameTime::tick(IS_SWIFT);
}

void SmokeGrenade::onStdTurnPlayerHoldIgnited()
{
    if (fuseTurns_ < getStdFuseTurns() && Rnd::coinToss())
    {
        Explosion::runSmokeExplosionAt(Map::player->pos);
    }
    fuseTurns_--;
    if (fuseTurns_ <= 0)
    {
        Log::addMsg("The smoke grenade is extinguished.");
        Map::player->activeExplosive = nullptr;
        Map::player->updateClr();
        delete this;
    }
}

void SmokeGrenade::onThrownIgnitedLanding(const Pos& p)
{
    Explosion::runSmokeExplosionAt(p);
    Map::player->updateFov();
    Render::drawMapAndInterface();
}

void SmokeGrenade::onPlayerParalyzed()
{
    Log::addMsg("The ignited smoke grenade falls from my hand!");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    const Pos&  p = Map::player->pos;
    auto* const f = Map::cells[p.x][p.y].rigid;
    if (!f->isBottomless()) {Explosion::runSmokeExplosionAt(Map::player->pos);}
    Map::player->updateFov();
    Render::drawMapAndInterface();
    delete this;
}

Clr SmokeGrenade::getIgnitedProjectileClr() const
{
    return getData().clr;
}

