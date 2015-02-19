#include "init.hpp"

#include "item_jewelry.hpp"

#include "map.hpp"
#include "utils.hpp"
#include "log.hpp"
#include "game_time.hpp"
#include "render.hpp"
#include "actor_mon.hpp"
#include "text_format.hpp"
#include "actor_factory.hpp"
#include "feature_rigid.hpp"

using namespace std;

namespace
{

Item_id  effect_list_   [int(Jewelry_effect_id::END)];
bool    effects_known_ [int(Jewelry_effect_id::END)];

Jewelry_effect* mk_effect(const Jewelry_effect_id id, Jewelry* const jewelry)
{
    assert(id != Jewelry_effect_id::END);

    Jewelry_effect* ret = nullptr;

    switch (id)
    {
    case Jewelry_effect_id::r_fire:
        ret = new Jewelry_effect_rFire(jewelry);
        break;

    case Jewelry_effect_id::r_cold:
        ret = new Jewelry_effect_rCold(jewelry);
        break;

    case Jewelry_effect_id::r_elec:
        ret = new Jewelry_effect_rElec(jewelry);
        break;

    case Jewelry_effect_id::r_poison:
        ret = new Jewelry_effect_rPoison(jewelry);
        break;

    case Jewelry_effect_id::r_disease:
        ret = new Jewelry_effect_rDisease(jewelry);
        break;

    case Jewelry_effect_id::tele_ctrl:
        ret = new Jewelry_effect_tele_control(jewelry);
        break;

    case Jewelry_effect_id::summon:
        ret = new Jewelry_effect_summon_mon(jewelry);
        break;

    case Jewelry_effect_id::light:
        ret = new Jewelry_effect_light(jewelry);
        break;

    case Jewelry_effect_id::spell_reflect:
        ret = new Jewelry_effect_spell_reflect(jewelry);
        break;

    case Jewelry_effect_id::hp_bon:
        ret = new Jewelry_effect_hp_bon(jewelry);
        break;

    case Jewelry_effect_id::hp_pen:
        ret = new Jewelry_effect_hp_pen(jewelry);
        break;

    case Jewelry_effect_id::spi_bon:
        ret = new Jewelry_effect_spi_bon(jewelry);
        break;

    case Jewelry_effect_id::spi_pen:
        ret = new Jewelry_effect_spi_pen(jewelry);
        break;

    case Jewelry_effect_id::random_tele:
        ret = new Jewelry_effect_random_tele(jewelry);
        break;

    case Jewelry_effect_id::fire:
        ret = new Jewelry_effect_fire(jewelry);
        break;

    case Jewelry_effect_id::conflict:
        ret = new Jewelry_effect_conflict(jewelry);
        break;

    case Jewelry_effect_id::burden:
        ret = new Jewelry_effect_burden(jewelry);
        break;

    case Jewelry_effect_id::hp_regen_bon:
        ret = new Jewelry_effect_hp_regen_bon(jewelry);
        break;

    case Jewelry_effect_id::hp_regen_pen:
        ret = new Jewelry_effect_hp_regen_pen(jewelry);
        break;

    case Jewelry_effect_id::haste:
        ret = new Jewelry_effect_haste(jewelry);
        break;

    case Jewelry_effect_id::shriek:
        ret = new Jewelry_effect_shriek(jewelry);
        break;

    case Jewelry_effect_id::START_OF_SECONDARY_EFFECTS:
    case Jewelry_effect_id::END: {} break;
    }

    assert(ret);
    return ret;
}

} //namespace

//--------------------------------------------------------- JEWELRY PROPERTY EFFECT
void Jewelry_property_effect::on_equip(const bool IS_SILENT)
{
    Prop* const prop = mk_prop();

    assert(prop);

    jewelry_->carrier_props_.push_back(prop);

    if (!IS_SILENT)
    {
        Game_time::update_light_map();
        Map::player->update_fov();
        Render::draw_map_and_interface();

        const auto&   prop_data  = Prop_data::data[size_t(prop->get_id())];
        const string  msg       = prop_data.msg[prop_msg_on_start_player];

        Log::add_msg(msg);
    }

    jewelry_->effect_noticed(get_id());
}

Unequip_allowed Jewelry_property_effect::on_unequip()
{
    vector<Prop_id> prop_ids_ended;

    for (Prop* const prop : jewelry_->carrier_props_)
    {
        prop_ids_ended.push_back(prop->get_id());
        delete prop;
    }
    jewelry_->carrier_props_.clear();

    Game_time::update_light_map();
    Map::player->update_fov();
    Render::draw_map_and_interface();

    for (const Prop_id prop_id : prop_ids_ended)
    {
        const auto&     prop_data    = Prop_data::data[size_t(prop_id)];
        const string    msg         = prop_data.msg[prop_msg_on_end_player];
        Log::add_msg(msg);
    }

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- EFFECT: FIRE RESISTANCE
Prop* Jewelry_effect_rFire::mk_prop() const
{
    return new Prop_rFire(Prop_turns::indefinite);
}

//--------------------------------------------------------- EFFECT: COLD RESISTANCE
Prop* Jewelry_effect_rCold::mk_prop() const
{
    return new Prop_rCold(Prop_turns::indefinite);
}

//--------------------------------------------------------- EFFECT: ELEC RESISTANCE
Prop* Jewelry_effect_rElec::mk_prop() const
{
    return new Prop_rElec(Prop_turns::indefinite);
}

//--------------------------------------------------------- EFFECT: POISON RESISTANCE
Prop* Jewelry_effect_rPoison::mk_prop() const
{
    return new Prop_rPoison(Prop_turns::indefinite);
}

//--------------------------------------------------------- EFFECT: DISEASE RESISTANCE
Prop* Jewelry_effect_rDisease::mk_prop() const
{
    return new Prop_rDisease(Prop_turns::indefinite);
}

//--------------------------------------------------------- EFFECT: TELEPORT CONTROL
Prop* Jewelry_effect_tele_control::mk_prop() const
{
    return new Prop_tele_control(Prop_turns::indefinite);
}

//--------------------------------------------------------- EFFECT: LIGHT
Prop* Jewelry_effect_light::mk_prop() const
{
    return new Prop_radiant(Prop_turns::indefinite);
}

//--------------------------------------------------------- EFFECT: SPELL REFLECTION
Prop* Jewelry_effect_spell_reflect::mk_prop() const
{
    return new Prop_spell_reflect(Prop_turns::indefinite);
}

//--------------------------------------------------------- EFFECT: HASTE
Prop* Jewelry_effect_haste::mk_prop() const
{
    return new Prop_hasted(Prop_turns::indefinite);
}

//--------------------------------------------------------- EFFECT: HP BONUS
void Jewelry_effect_hp_bon::on_equip(const bool IS_SILENT)
{
    (void)IS_SILENT;

    Map::player->change_max_hp(4, true);
    jewelry_->effect_noticed(get_id());
}

Unequip_allowed Jewelry_effect_hp_bon::on_unequip()
{
    Map::player->change_max_hp(-4, true);

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- EFFECT: HP PENALTY
void Jewelry_effect_hp_pen::on_equip(const bool IS_SILENT)
{
    (void)IS_SILENT;

    Map::player->change_max_hp(-4, true);
    jewelry_->effect_noticed(get_id());
}

Unequip_allowed Jewelry_effect_hp_pen::on_unequip()
{
    Map::player->change_max_hp(4, true);

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- EFFECT: SPI BONUS
void Jewelry_effect_spi_bon::on_equip(const bool IS_SILENT)
{
    (void)IS_SILENT;

    Map::player->change_max_spi(4, true);
    jewelry_->effect_noticed(get_id());
}

Unequip_allowed Jewelry_effect_spi_bon::on_unequip()
{
    Map::player->change_max_spi(-4, true);

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- EFFECT: SPI PENALTY
void Jewelry_effect_spi_pen::on_equip(const bool IS_SILENT)
{
    (void)IS_SILENT;

    Map::player->change_max_spi(-4, true);
    jewelry_->effect_noticed(get_id());
}

Unequip_allowed Jewelry_effect_spi_pen::on_unequip()
{
    Map::player->change_max_spi(4, true);

    return Unequip_allowed::yes;
}

//--------------------------------------------------------- EFFECT: RANDOM TELEPORTATION
void Jewelry_effect_random_tele::on_std_turn_equiped()
{
    auto& prop_handler = Map::player->get_prop_handler();

    const int TELE_ON_IN_N = 200;

    if (Rnd::one_in(TELE_ON_IN_N) && prop_handler.allow_act())
    {
        Log::add_msg("I am being teleported...", clr_white, true, true);
        Map::player->teleport();
        jewelry_->effect_noticed(get_id());
    }
}

//--------------------------------------------------------- EFFECT: SUMMON MON
void Jewelry_effect_summon_mon::on_std_turn_equiped()
{
    const int SOUND_ONE_IN_N  = 250;

    if (effects_known_[size_t(get_id())] && Rnd::one_in(SOUND_ONE_IN_N))
    {
        Log::add_msg("I hear a faint whistling sound coming nearer...", clr_white, false,
                    true);

        Map::player->incr_shock(Shock_lvl::mild, Shock_src::misc);
    }

    const int SUMMON_ONE_IN_N = 1200;

    if (Rnd::one_in(SUMMON_ONE_IN_N))
    {
        Log::add_msg("There is a loud whistling sound.", clr_white, false, true);

        const Pos origin = Map::player->pos;

        vector<Mon*> summoned_mon;

        Actor_factory::summon(origin, {Actor_id::greater_polyp}, false, nullptr,
                             &summoned_mon);

        const Mon* const mon = summoned_mon[0];

        if (Map::player->can_see_actor(*mon, nullptr))
        {
            Log::add_msg(mon->get_name_a() + " appears!", clr_white, true, true);
        }

        jewelry_->effect_noticed(get_id());
    }
}

//--------------------------------------------------------- EFFECT: FIRE
void Jewelry_effect_fire::on_std_turn_equiped()
{
    const int FIRE_ONE_IN_N = 300;

    if (Rnd::one_in(FIRE_ONE_IN_N))
    {
        const Pos& origin   = Map::player->pos;
        const int D         = FOV_STD_RADI_INT - 2;
        const int X0        = max(1,            origin.x - D);
        const int Y0        = max(1,            origin.y - D);
        const int X1        = min(MAP_W - 2,    origin.x + D);
        const int Y1        = min(MAP_H - 2,    origin.y + D);

        const int FIRE_CELL_ONE_IN_N = 4;

        Log::add_msg("The surrounding area suddenly burst into flames!", clr_white,
                    false, true);

        for (int x = X0; x <= X1; ++x)
        {
            for (int y = Y0; y <= Y1; ++y)
            {
                if (Rnd::one_in(FIRE_CELL_ONE_IN_N) && Pos(x, y) != origin)
                {
                    Map::cells[x][y].rigid->hit(Dmg_type::fire, Dmg_method::elemental);
                }
            }
        }

        jewelry_->effect_noticed(get_id());
    }
}

//--------------------------------------------------------- EFFECT: CONFLICT
void Jewelry_effect_conflict::on_std_turn_equiped()
{
    const int CONFLICT_ONE_IN_N = 50;

    if (Rnd::one_in(CONFLICT_ONE_IN_N))
    {
        vector<Actor*> seen_foes;
        Map::player->get_seen_foes(seen_foes);

        if (!seen_foes.empty())
        {
            random_shuffle(begin(seen_foes), end(seen_foes));

            for (Actor* actor : seen_foes)
            {
                if (!actor->get_data().is_unique)
                {
                    actor->get_prop_handler().try_apply_prop(
                        new Prop_conflict(Prop_turns::std));

                    jewelry_->effect_noticed(get_id());

                    break;
                }
            }
        }
    }
}

//--------------------------------------------------------- EFFECT: SHRIEK
Jewelry_effect_shriek::Jewelry_effect_shriek(Jewelry* const jewelry) :
    Jewelry_effect(jewelry)
{
    string player_name = Map::player->get_name_the();

    Text_format::all_to_upper(player_name);

    words_ =
    {
        player_name,
        player_name,
        player_name,
        "BHUUDESCO",
        "STRAGARANA",
        "INFIRMUX",
        "BHAAVA",
        "CRUENTO",
        "PRETIACRUENTO",
        "VILOMAXUS",
        "DEATH",
        "DYING",
        "TAKE",
        "Bl_oOD",
        "END",
        "SACRIFICE",
        "POSSESSED",
        "PROPHECY",
        "SIGNS",
        "OATH",
        "FATE",
        "SUFFER",
        "BEHOLD",
        "BEWARE",
        "WATCHES",
        "LIGHT",
        "DARK",
        "DISAPPEAR",
        "APPEAR",
        "DECAY",
        "IMMORTAL",
        "BOUNDLESS",
        "ETERNAL",
        "ANCIENT",
        "TIME",
        "NEVER-ENDING",
        "DIMENSIONS",
        "EYES",
        "STARS",
        "GAZE",
        "FORBIDDEN",
        "DOMINIONS",
        "RULER",
        "KING",
        "UNKNOWN",
        "ABYSS",
        "BENEATH",
        "BELOW",
        "PASSAGE",
        "PATH",
        "GATE",
        "SERPENT",
        "NYARLATHOTEP",
        "GOL-GOROTH",
        "ABHOLOS",
        "HASTUR",
        "ISTASHA",
        "ITHAQUA",
        "THOG",
        "TSATHOGGUA",
        "YMNAR",
        "XCTHOL",
        "ZATHOG",
        "ZINDARAK",
        "BASATAN",
        "CTHUGHA"
    };
}

void Jewelry_effect_shriek::on_std_turn_equiped()
{
    const int NOISE_ONE_IN_N = 300;

    if (Rnd::one_in(NOISE_ONE_IN_N))
    {
        const string name = jewelry_->get_name(Item_ref_type::plain, Item_ref_inf::none);

        Log::add_msg("The " + name + " shrieks...", clr_white, false, true);

        const int NR_WORDS = 3;

        string phrase = "";

        for (int i = 0; i < NR_WORDS; ++i)
        {
            const int   IDX     = Rnd::range(0, words_.size() - 1);
            const auto& word    = words_[size_t(IDX)];

            phrase += word;

            if (i < NR_WORDS - 1)
            {
                phrase += " ... ";
            }
        }

        phrase += "!!!";

        Snd_emit::emit_snd(Snd(phrase, Sfx_id::END, Ignore_msg_if_origin_seen::no,
                             Map::player->pos, Map::player, Snd_vol::high, Alerts_mon::yes
                            ));

        Map::player->incr_shock(Shock_lvl::mild, Shock_src::misc);

        Log::more_prompt();

        jewelry_->effect_noticed(get_id());
    }
}

//--------------------------------------------------------- EFFECT: BURDEN
void Jewelry_effect_burden::on_equip(const bool IS_SILENT)
{
    if (!effects_known_[size_t(get_id())])
    {
        if (!IS_SILENT)
        {
            Log::add_msg("I suddenly feel more burdened.");
        }

        jewelry_->effect_noticed(get_id());
    }
}

void Jewelry_effect_burden::change_item_weight(int& weight_ref)
{
    if (effects_known_[size_t(get_id())])
    {
        //If revealed, this item weighs the average of "heavy" and "medium"
        weight_ref = (int(Item_weight::heavy) + int(Item_weight::medium)) / 2;
    }
}

//--------------------------------------------------------- EFFECT: HP REGEN BONUS
void Jewelry_effect_hp_regen_bon::on_equip(const bool IS_SILENT)
{
    if (!IS_SILENT)
    {
        Log::add_msg("I heal faster.");
    }

    jewelry_->effect_noticed(get_id());
}

Unequip_allowed Jewelry_effect_hp_regen_bon::on_unequip()
{
    Log::add_msg("I heal slower.");
    return Unequip_allowed::yes;
}

//--------------------------------------------------------- EFFECT: HP REGEN PENALTY
void Jewelry_effect_hp_regen_pen::on_equip(const bool IS_SILENT)
{
    if (!IS_SILENT)
    {
        Log::add_msg("I heal slower.");
    }

    jewelry_->effect_noticed(get_id());
}

Unequip_allowed Jewelry_effect_hp_regen_pen::on_unequip()
{
    Log::add_msg("I heal faster.");
    return Unequip_allowed::yes;
}

//--------------------------------------------------------- JEWELRY
Jewelry::Jewelry(Item_data_t* const item_data) :
    Item(item_data)
{
    for (size_t i = 0; i < int(Jewelry_effect_id::END); ++i)
    {
        if (effect_list_[i] == item_data->id)
        {
            auto* const effect = mk_effect(Jewelry_effect_id(i), this);
            effects_.push_back(effect);
        }
    }

    //Unique item, do not allow spawning more
    data_->allow_spawn = false;
}

Jewelry::~Jewelry()
{
    for (auto* const effect : effects_)
    {
        delete effect;
    }
}

vector<string> Jewelry::get_descr() const
{
    vector<string> ret;

    for (auto* effect : effects_)
    {
        const size_t EFFECT_IDX = size_t(effect->get_id());

        if (effects_known_[EFFECT_IDX])
        {
            ret.push_back(effect->get_descr());
        }
    }

    if (data_->is_identified)
    {
        const string name = get_name(Item_ref_type::plain, Item_ref_inf::none);

        ret.push_back("All properties of the " + name + " are known to me.");
    }

    ret.push_back(item_carry_shock_descr);

    return ret;
}

string Jewelry::get_name_inf() const
{
    return data_->is_identified ? "{Known}" : "";
}

void Jewelry::on_equip(const bool IS_SILENT)
{
    for (auto* const effect : effects_)
    {
        //This may cause the effect to set up carrier properties (e.g. fire resistance)
        effect->on_equip(IS_SILENT);
    }

    if (!IS_SILENT)
    {
        Log::more_prompt();
    }
}

Unequip_allowed Jewelry::on_unequip()
{
    auto unequip_allowed = Unequip_allowed::yes;

    for (auto* const effect : effects_)
    {
        if (effect->on_unequip() == Unequip_allowed::no)
        {
            unequip_allowed = Unequip_allowed::no;
        }
    }

    Log::more_prompt();

    return unequip_allowed;
}

void Jewelry::on_std_turn_in_inv(const Inv_type inv_type)
{
    if (inv_type == Inv_type::slots)
    {
        for (auto* const effect : effects_)
        {
            effect->on_std_turn_equiped();
        }
    }
}

void Jewelry::on_actor_turn_in_inv(const Inv_type inv_type)
{
    if (inv_type == Inv_type::slots)
    {
        for (auto* const effect : effects_)
        {
            effect->on_actor_turn_equiped();
        }
    }
}

void Jewelry::identify(const bool IS_SILENT_IDENTIFY)
{
    (void)IS_SILENT_IDENTIFY;

    for (auto* effect : effects_)
    {
        const size_t EFFECT_IDX = size_t(effect->get_id());

        effects_known_[EFFECT_IDX] = true;
    }

    data_->is_identified = true;
}

int Jewelry::get_weight() const
{
    int weight = Item::get_weight();

    for (auto* effect : effects_)
    {
        effect->change_item_weight(weight);
    }

    return weight;
}

int Jewelry::get_hp_regen_change(const Inv_type inv_type) const
{
    int change = 0;

    if (inv_type == Inv_type::slots)
    {
        for (auto* effect : effects_)
        {
            change += effect->get_hp_regen_change();
        }
    }

    return change;
}

void Jewelry::effect_noticed(const Jewelry_effect_id effect_id)
{
    const size_t EFFECT_IDX = size_t(effect_id);

    if (!effects_known_[EFFECT_IDX])
    {
        effects_known_[EFFECT_IDX] = true;

        const int   MAX_NR_EFFECTS_ON_ITEM  = 2;
        int         nr_effects_known_this_item  = 0;

        for (auto* effect : effects_)
        {
            const size_t CHECK_EFFECT_IDX = size_t(effect->get_id());

            if (effects_known_[CHECK_EFFECT_IDX])
            {
                ++nr_effects_known_this_item;
            }
        }

        assert(nr_effects_known_this_item <= MAX_NR_EFFECTS_ON_ITEM);

        const string name = get_name(Item_ref_type::plain, Item_ref_inf::none);

        Log::add_msg("I gained new knowledge about the " + name + ".", clr_white, false,
                    true);

        if (nr_effects_known_this_item == MAX_NR_EFFECTS_ON_ITEM)
        {
            Log::add_msg("I feel like all properties of the " + name +
                        " are known to me.", clr_white, false, true);
            data_->is_identified = true;
        }

        Map::player->incr_shock(Shock_lvl::heavy, Shock_src::use_strange_item);
    }
}

//--------------------------------------------------------- JEWELRY HANDLING
namespace
{

bool can_effects_be_combined(const Jewelry_effect_id id1,
                          const Jewelry_effect_id id2)
{
    typedef Jewelry_effect_id Id;

    assert (id1 != Id::END && id2 != Id::END);

    if (id1 == id2)
    {
        return true;
    }

    switch (id1)
    {
    case Id::hp_bon:
        return id2 != Id::hp_pen;

    case Id::hp_pen:
        return id2 != Id::hp_bon && id2 != Id::r_disease;

    case Id::spi_bon:
        return id2 != Id::spi_pen;

    case Id::spi_pen:
        return id2 != Id::spi_bon;

    case Id::r_disease:
        return id2 != Id::hp_pen;

    case Id::tele_ctrl:
        return id2 != Id::random_tele && id2 != Id::spell_reflect;

    case Id::random_tele:
        return id2 != Id::tele_ctrl;

    case Id::spell_reflect:
        return id2 != Id::tele_ctrl;

    case Id::hp_regen_bon:
        return id2 != Id::hp_regen_pen && id2 != Id::hp_bon && id2 != Id::hp_pen;

    case Id::hp_regen_pen:
        return id2 != Id::hp_regen_bon && id2 != Id::hp_bon && id2 != Id::hp_pen;

    case Id::summon:
    case Id::light:
    case Id::conflict:
    case Id::r_fire:
    case Id::r_cold:
    case Id::r_elec:
    case Id::r_poison:
    case Id::burden:
    case Id::shriek:
    case Id::haste:
    case Id::fire:
        return true;

    case Jewelry_effect_id::START_OF_SECONDARY_EFFECTS:
    case Id::END:
        assert(false);
        return false;
    }
    return false;
}

int get_rnd_item_bucket_idx_for_effect(const Jewelry_effect_id  effect_to_assign,
                                 const vector<Item_id>&  item_bucket)
{
    vector<int> item_idx_bucket;

    for (size_t item_bucket_idx = 0; item_bucket_idx < item_bucket.size(); ++item_bucket_idx)
    {
        bool can_be_placed_on_item = true;

        //Verify that the effect can be placed on this item by checking if it can
        //be combined with every effect currently assigned to this item.
        for (int i = 0; i < int(Jewelry_effect_id::END); ++i)
        {
            const Jewelry_effect_id cur_effect = Jewelry_effect_id(i);

            if (
                effect_list_[i] == item_bucket[item_bucket_idx] &&
                !can_effects_be_combined(effect_to_assign, cur_effect))
            {
                //Combination with effect already assigned on item not allowed
                can_be_placed_on_item = false;
                break;
            }
        }

        if (can_be_placed_on_item)
        {
            item_idx_bucket.push_back(item_bucket_idx);
        }
    }

    if (item_idx_bucket.empty())
    {
        return -1;
    }

    const int IDX_BUCKET_IDX = Rnd::range(0, item_idx_bucket.size() - 1);
    return item_idx_bucket[IDX_BUCKET_IDX];
}

} //namespace

namespace jewelry_handling
{

void init()
{
    //Reset the effect list and knowledge status list
    for (size_t i = 0; i < size_t(Jewelry_effect_id::END); ++i)
    {
        effect_list_   [i] = Item_id::END;
        effects_known_ [i] = false;
    }

    vector<Item_id> item_bucket;

    for (auto* data : Item_data::data)
    {
        const auto type = data->type;

        if (type == Item_type::amulet || type == Item_type::ring)
        {
            item_bucket.push_back(data->id);
        }
    }
    random_shuffle(begin(item_bucket), end(item_bucket));

    vector<Jewelry_effect_id> primary_effect_bucket;
    vector<Jewelry_effect_id> secondary_effect_bucket;

    for (size_t i = 0; i < size_t(Jewelry_effect_id::END); ++i)
    {
        const auto id = Jewelry_effect_id(i);

        if (id <  Jewelry_effect_id::START_OF_SECONDARY_EFFECTS)
        {
            primary_effect_bucket.push_back(id);
        }
        else if (id >  Jewelry_effect_id::START_OF_SECONDARY_EFFECTS)
        {
            secondary_effect_bucket.push_back(id);
        }
    }
    random_shuffle(begin(primary_effect_bucket),   end(primary_effect_bucket));
    random_shuffle(begin(secondary_effect_bucket), end(secondary_effect_bucket));

    //Assuming there are more jewelry than primary or secondary effects (if this changes,
    //just add more amulets and rings to the item data)
    assert(item_bucket.size() > primary_effect_bucket.size());
    assert(item_bucket.size() > secondary_effect_bucket.size());

    //Assign primary effects
    for (size_t i = 0; i < item_bucket.size(); ++i)
    {
        const auto item_id = item_bucket[i];

        if (i < primary_effect_bucket.size())
        {
            const auto effect_id = primary_effect_bucket[i];

            effect_list_[size_t(effect_id)] = item_id;
        }
        else //All primary effects are assigned
        {
            Item_data::data[size_t(item_id)]->allow_spawn = false;
        }
    }

    //Remove all items without a primary effect from the item bucket
    item_bucket.resize(primary_effect_bucket.size());

    //Assign secondary effects
    for (const auto secondary_effect_id : secondary_effect_bucket)
    {
        const int ITEM_IDX = get_rnd_item_bucket_idx_for_effect(secondary_effect_id, item_bucket);

        if (ITEM_IDX >= 0)
        {
            effect_list_[size_t(secondary_effect_id)] = item_bucket[size_t(ITEM_IDX)];
            item_bucket.erase(begin(item_bucket) + size_t(ITEM_IDX));
        }
    }
}

void store_to_save_lines(vector<string>& lines)
{
    for (size_t i = 0; i < size_t(Jewelry_effect_id::END); ++i)
    {
        lines.push_back(to_str(int(effect_list_[i])));
        lines.push_back(effects_known_[i] ? "1" : "0");
    }
}

void setup_from_save_lines(vector<string>& lines)
{
    for (size_t i = 0; i < size_t(Jewelry_effect_id::END); ++i)
    {
        effect_list_[i] = Item_id(to_int(lines.front()));
        lines.erase(begin(lines));
        effects_known_[i] = lines.front() == "1";
        lines.erase(begin(lines));
    }
}

} //Jewelry_handling
