#include "Populate_monsters.h"

#include <algorithm>

#include "Init.h"
#include "Room.h"
#include "Feature_trap.h"
#include "Map.h"
#include "Actor_factory.h"
#include "Actor_mon.h"
#include "Actor_player.h"
#include "Map_parsing.h"
#include "Utils.h"
#include "Game_time.h"

using namespace std;

namespace Populate_mon
{

namespace
{

int get_random_out_of_depth()
{
    if (Map::dlvl == 0)                     {return 0;}
    if (Rnd::one_in(40) && Map::dlvl > 1)    {return 5;}
    if (Rnd::one_in(5))                      {return 2;}

    return 0;
}

void mk_list_of_mon_can_auto_spawn(const int NR_LVLS_OUT_OF_DEPTH, vector<Actor_id>& list_ref)
{
    list_ref.clear();

    const int EFFECTIVE_DLVL =
        max(1, min(DLVL_LAST, Map::dlvl + NR_LVLS_OUT_OF_DEPTH));

    //Get list of actors currently on the level (to help avoid spawning multiple uniques,
    //note that this could otherwise happen for example with Zuul - he is allowed to
    //spawn freely after he appears from a possessed Cultist priest)
    bool spawned_ids[size_t(Actor_id::END)];
    for (bool& v : spawned_ids) {v = false;}

    for (const auto* const actor : Game_time::actors_)
    {
        spawned_ids[size_t(actor->get_id())] = true;
    }

    for (const auto& d : Actor_data::data)
    {
        if (
            d.id != Actor_id::player             &&
            d.is_auto_spawn_allowed                &&
            d.nr_left_allowed_to_spawn != 0         &&
            EFFECTIVE_DLVL >= d.spawn_min_dLVL    &&
            EFFECTIVE_DLVL <= d.spawn_max_dLVL    &&
            !(d.is_unique && spawned_ids[size_t(d.id)]))
        {
            list_ref.push_back(d.id);
        }
    }
}

void mk_group_of_random_at(const vector<Pos>&   sorted_free_cells_vector,
                       bool                 blocked[MAP_W][MAP_H],
                       const int            NR_LVLS_OUT_OF_DEPTH_ALLOWED,
                       const bool           IS_ROAMING_ALLOWED)
{
    vector<Actor_id> id_bucket;
    mk_list_of_mon_can_auto_spawn(NR_LVLS_OUT_OF_DEPTH_ALLOWED, id_bucket);

    if (!id_bucket.empty())
    {
        const Actor_id id = id_bucket[Rnd::range(0, id_bucket.size() - 1)];
        mk_group_at(id, sorted_free_cells_vector, blocked, IS_ROAMING_ALLOWED);
    }
}

bool mk_group_of_random_native_to_room_type_at(const Room_type       room_type,
                                       const vector<Pos>&   sorted_free_cells_vector,
                                       bool                 blocked[MAP_W][MAP_H],
                                       const bool           IS_ROAMING_ALLOWED)
{
    TRACE_FUNC_BEGIN_VERBOSE;

    const int NR_LVLS_OUT_OF_DEPTH_ALLOWED = get_random_out_of_depth();
    vector<Actor_id> id_bucket;
    mk_list_of_mon_can_auto_spawn(NR_LVLS_OUT_OF_DEPTH_ALLOWED, id_bucket);

    for (size_t i = 0; i < id_bucket.size(); ++i)
    {
        const Actor_data_t& d = Actor_data::data[int(id_bucket[i])];
        bool is_mon_native_to_room = false;

        for (const auto native_room_type : d.native_rooms)
        {
            if (native_room_type == room_type)
            {
                is_mon_native_to_room = true;
                break;
            }
        }
        if (!is_mon_native_to_room)
        {
            id_bucket.erase(id_bucket.begin() + i);
            --i;
        }
    }

    if (id_bucket.empty())
    {
        TRACE_VERBOSE << "Found no valid monsters to spawn at room type ("
                      << to_str(int(room_type)) + ")" << endl;
        TRACE_FUNC_END_VERBOSE;
        return false;
    }
    else //Found valid monster IDs
    {
        const Actor_id id = id_bucket[Rnd::range(0, id_bucket.size() - 1)];
        mk_group_at(id, sorted_free_cells_vector, blocked, IS_ROAMING_ALLOWED);
        TRACE_FUNC_END_VERBOSE;
        return true;
    }
}

} //namespace

void try_spawn_due_to_time_passed()
{
    TRACE_FUNC_BEGIN;

    if (Game_time::actors_.size() >= MAX_NR_ACTORS_ON_MAP)
    {
        return;
    }

    bool blocked[MAP_W][MAP_H];
    Map_parse::run(Cell_check::Blocks_move_cmn(true), blocked);

    const int MIN_DIST_TO_PLAYER = FOV_STD_RADI_INT + 3;

    const Pos& player_pos = Map::player->pos;

    const int X0 = max(0,           player_pos.x - MIN_DIST_TO_PLAYER);
    const int Y0 = max(0,           player_pos.y - MIN_DIST_TO_PLAYER);
    const int X1 = min(MAP_W - 1,   player_pos.x + MIN_DIST_TO_PLAYER);
    const int Y1 = min(MAP_H - 1,   player_pos.y + MIN_DIST_TO_PLAYER);

    for (int x = X0; x <= X1; ++x)
    {
        for (int y = Y0; y <= Y1; ++y)
        {
            blocked[x][y] = true;
        }
    }

    vector<Pos> free_cells_vector;
    for (int y = 1; y < MAP_H - 1; ++y)
    {
        for (int x = 1; x < MAP_W - 1; ++x)
        {
            if (!blocked[x][y]) {free_cells_vector.push_back(Pos(x, y));}
        }
    }

    if (!free_cells_vector.empty())
    {
        const int ELEMENT = Rnd::range(0, free_cells_vector.size() - 1);
        const Pos& origin = free_cells_vector[ELEMENT];

        mk_sorted_free_cells_vector(origin, blocked, free_cells_vector);

        if (!free_cells_vector.empty())
        {
            if (Map::cells[origin.x][origin.y].is_explored)
            {
                const int NR_OOD = get_random_out_of_depth();
                mk_group_of_random_at(free_cells_vector, blocked, NR_OOD, true);
            }
        }
    }
    TRACE_FUNC_END;
}

void populate_intro_lvl()
{
    const int NR_GROUPS_ALLOWED = 2; //Rnd::range(2, 3);

    bool blocked[MAP_W][MAP_H];

    const int MIN_DIST_FROM_PLAYER = FOV_STD_RADI_INT + 3;
    Map_parse::run(Cell_check::Blocks_move_cmn(true), blocked);

    const Pos& player_pos = Map::player->pos;

    const int X0 = max(0, player_pos.x - MIN_DIST_FROM_PLAYER);
    const int Y0 = max(0, player_pos.y - MIN_DIST_FROM_PLAYER);
    const int X1 = min(MAP_W - 1, player_pos.x + MIN_DIST_FROM_PLAYER) - 1;
    const int Y1 = min(MAP_H - 1, player_pos.y + MIN_DIST_FROM_PLAYER) - 1;
    for (int y = Y0; y <= Y1; ++y)
    {
        for (int x = X0; x <= X1; ++x)
        {
            blocked[x][y] = true;
        }
    }

    for (int i = 0; i < NR_GROUPS_ALLOWED; ++i)
    {
        vector<Pos> origin_bucket;
        for (int y = 1; y < MAP_H - 1; ++y)
        {
            for (int x = 1; x < MAP_W - 1; ++x)
            {
                if (!blocked[x][y]) {origin_bucket.push_back(Pos(x, y));}
            }
        }
        const int ELEMENT = Rnd::range(0, origin_bucket.size() - 1);
        const Pos origin = origin_bucket[ELEMENT];
        vector<Pos> sorted_free_cells_vector;
        mk_sorted_free_cells_vector(origin, blocked, sorted_free_cells_vector);
        if (!sorted_free_cells_vector.empty())
        {
            mk_group_at(Actor_id::wolf, sorted_free_cells_vector, blocked, true);
        }
    }
}

void populate_std_lvl()
{
    TRACE_FUNC_BEGIN;

    const int NR_GROUPS_ALLOWED_ON_MAP = Rnd::range(5, 9);
    int nr_groups_spawned = 0;

    bool blocked[MAP_W][MAP_H];

    const int MIN_DIST_FROM_PLAYER = FOV_STD_RADI_INT - 1;

    Map_parse::run(Cell_check::Blocks_move_cmn(true), blocked);

    const Pos& player_pos = Map::player->pos;

    const int X0 = max(0, player_pos.x - MIN_DIST_FROM_PLAYER);
    const int Y0 = max(0, player_pos.y - MIN_DIST_FROM_PLAYER);
    const int X1 = min(MAP_W - 1, player_pos.x + MIN_DIST_FROM_PLAYER);
    const int Y1 = min(MAP_H - 1, player_pos.y + MIN_DIST_FROM_PLAYER);

    for (int y = Y0; y <= Y1; ++y)
    {
        for (int x = X0; x <= X1; ++x)
        {
            blocked[x][y] = true;
        }
    }

    //First, attempt to populate all non-plain standard rooms
    for (Room* const room : Map::room_list)
    {
        if (
            room->type_ != Room_type::plain &&
            int(room->type_) < int(Room_type::END_OF_STD_ROOMS))
        {
            //TODO: This is not a good method to calculate the number of room cells
            //(the room may be irregularly shaped), parse the room map instead
            const int ROOM_W = room->r_.p1.x - room->r_.p0.x + 1;
            const int ROOM_H = room->r_.p1.y - room->r_.p0.y + 1;
            const int NR_CELLS_IN_ROOM = ROOM_W * ROOM_H;

            const int MAX_NR_GROUPS_IN_ROOM = 2;
            for (int i = 0; i < MAX_NR_GROUPS_IN_ROOM; ++i)
            {
                //Randomly pick a free position inside the room
                vector<Pos> origin_bucket;
                for (int y = room->r_.p0.y; y <= room->r_.p1.y; ++y)
                {
                    for (int x = room->r_.p0.x; x <= room->r_.p1.x; ++x)
                    {
                        if (Map::room_map[x][y] == room && !blocked[x][y])
                        {
                            origin_bucket.push_back(Pos(x, y));
                        }
                    }
                }

                //If room is too full (due to spawned monsters and features),
                //stop spawning in this room
                const int NR_ORIGIN_CANDIDATES = origin_bucket.size();
                if (NR_ORIGIN_CANDIDATES < (NR_CELLS_IN_ROOM / 3)) {break;}

                //Spawn monsters in room
                if (NR_ORIGIN_CANDIDATES > 0)
                {
                    const int ELEMENT = Rnd::range(0, NR_ORIGIN_CANDIDATES - 1);
                    const Pos& origin = origin_bucket[ELEMENT];
                    vector<Pos> sorted_free_cells_vector;
                    mk_sorted_free_cells_vector(origin, blocked, sorted_free_cells_vector);

                    if (mk_group_of_random_native_to_room_type_at(
                                room->type_, sorted_free_cells_vector, blocked, true))
                    {
                        nr_groups_spawned++;
                        if (nr_groups_spawned >= NR_GROUPS_ALLOWED_ON_MAP)
                        {
                            TRACE_FUNC_END;
                            return;
                        }
                    }
                }
            }

            //After attempting to populate a non-plain themed room,
            //mark that area as forbidden
            for (int y = room->r_.p0.y; y <= room->r_.p1.y; ++y)
            {
                for (int x = room->r_.p0.x; x <= room->r_.p1.x; ++x)
                {
                    blocked[x][y] = true;
                }
            }
        }
    }

    //Second, place groups randomly in plain-themed areas until <no more groups to place
    vector<Pos> origin_bucket;
    for (int y = 1; y < MAP_H - 1; ++y)
    {
        for (int x = 1; x < MAP_W - 1; ++x)
        {
            if (Map::room_map[x][y])
            {
                if (!blocked[x][y] && Map::room_map[x][y]->type_ == Room_type::plain)
                {
                    origin_bucket.push_back(Pos(x, y));
                }
            }
        }
    }

    if (!origin_bucket.empty())
    {
        while (nr_groups_spawned < NR_GROUPS_ALLOWED_ON_MAP)
        {
            const int   ELEMENT = Rnd::range(0, origin_bucket.size() - 1);
            const Pos   origin  = origin_bucket[ELEMENT];

            vector<Pos> sorted_free_cells_vector;
            mk_sorted_free_cells_vector(origin, blocked, sorted_free_cells_vector);

            if (mk_group_of_random_native_to_room_type_at(
                        Room_type::plain, sorted_free_cells_vector, blocked, true))
            {
                nr_groups_spawned++;
            }
        }
    }

    TRACE_FUNC_END;
}

void mk_group_at(const Actor_id id, const vector<Pos>& sorted_free_cells_vector,
               bool blocked[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED)
{
    const Actor_data_t& d = Actor_data::data[int(id)];

    int max_nr_in_group = 1;

    switch (d.group_size)
    {
    case Mon_group_size::few:    max_nr_in_group = Rnd::range(1, 2);    break;
    case Mon_group_size::group:  max_nr_in_group = Rnd::range(3, 4);    break;
    case Mon_group_size::horde:  max_nr_in_group = Rnd::range(6, 7);    break;
    case Mon_group_size::swarm:  max_nr_in_group = Rnd::range(10, 12);  break;
    default: {} break;
    }

    Actor* origin_actor = nullptr;

    const int NR_FREE_CELLS     = sorted_free_cells_vector.size();
    const int NR_CAN_BE_SPAWNED = min(NR_FREE_CELLS, max_nr_in_group);

    for (int i = 0; i < NR_CAN_BE_SPAWNED; ++i)
    {
        const Pos&      p       = sorted_free_cells_vector[i];
        Actor* const    actor   = Actor_factory::mk(id, p);
        Mon* const      mon     = static_cast<Mon*>(actor);
        mon->is_roaming_allowed_  = IS_ROAMING_ALLOWED;

        if (i == 0)
        {
            origin_actor = actor;
        }
        else
        {
            mon->leader_ = origin_actor;
        }

        blocked[p.x][p.y] = true;
    }
}

void mk_sorted_free_cells_vector(const Pos& origin,
                             const bool blocked[MAP_W][MAP_H],
                             vector<Pos>& vector_ref)
{
    vector_ref.clear();

    const int RADI = 10;
    const int X0 = get_constr_in_range(1, origin.x - RADI, MAP_W - 2);
    const int Y0 = get_constr_in_range(1, origin.y - RADI, MAP_H - 2);
    const int X1 = get_constr_in_range(1, origin.x + RADI, MAP_W - 2);
    const int Y1 = get_constr_in_range(1, origin.y + RADI, MAP_H - 2);

    for (int y = Y0; y <= Y1; ++y)
    {
        for (int x = X0; x <= X1; ++x)
        {
            if (!blocked[x][y]) {vector_ref.push_back(Pos(x, y));}
        }
    }

    Is_closer_to_pos sorter(origin);
    std::sort(vector_ref.begin(), vector_ref.end(), sorter);
}

} //Populate_mon
