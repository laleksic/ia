#ifndef ACTOR_FACTORY
#define ACTOR_FACTORY

#include <vector>

#include "Actor_data.h"

class Actor;
class Mon;

namespace Actor_factory
{

void delete_all_mon();

Actor* mk(const Actor_id id, const Pos& pos);

void summon(const Pos&                  origin,
            const std::vector<Actor_id>& monster_ids,
            const bool                  MAKE_MONSTERS_AWARE,
            Actor* const                actor_to_set_as_leader = nullptr,
            std::vector<Mon*>*          monsters_ret = nullptr);

} //Actor_factory

#endif
