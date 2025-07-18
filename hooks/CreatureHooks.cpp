/*
 * Copyright (C) 2010 - 2024 Eluna Lua Engine <https://elunaluaengine.github.io/>
 * This program is free software licensed under GPL version 3
 * Please see the included DOCS/LICENSE.md for more information
 */

#include "Hooks.h"
#include "HookHelpers.h"
#include "LuaEngine.h"
#include "BindingMap.h"
#include "ElunaIncludes.h"
#include "ElunaTemplate.h"

using namespace Hooks;

#define START_HOOK(EVENT, CREATURE) \
    auto CreatureEventBindings = GetBinding<EntryKey<CreatureEvents>>(REGTYPE_CREATURE);\
    auto CreatureUniqueBindings = GetBinding<UniqueObjectKey<CreatureEvents>>(REGTYPE_CREATURE_UNIQUE);\
    auto entry_key = EntryKey<CreatureEvents>(EVENT, CREATURE->GetEntry());\
    auto unique_key = UniqueObjectKey<CreatureEvents>(EVENT, CREATURE->GET_GUID(), CREATURE->GetInstanceId());\
    if (!CreatureEventBindings->HasBindingsFor(entry_key))\
        if (!CreatureUniqueBindings->HasBindingsFor(unique_key))\
            return;

#define START_HOOK_WITH_RETVAL(EVENT, CREATURE, RETVAL) \
    auto CreatureEventBindings = GetBinding<EntryKey<CreatureEvents>>(REGTYPE_CREATURE);\
    auto CreatureUniqueBindings = GetBinding<UniqueObjectKey<CreatureEvents>>(REGTYPE_CREATURE_UNIQUE);\
    auto entry_key = EntryKey<CreatureEvents>(EVENT, CREATURE->GetEntry());\
    auto unique_key = UniqueObjectKey<CreatureEvents>(EVENT, CREATURE->GET_GUID(), CREATURE->GetInstanceId());\
    if (!CreatureEventBindings->HasBindingsFor(entry_key))\
        if (!CreatureUniqueBindings->HasBindingsFor(unique_key))\
            return RETVAL;

void Eluna::OnDummyEffect(WorldObject* pCaster, uint32 spellId, SpellEffIndex effIndex, Creature* pTarget)
{
    START_HOOK(CREATURE_EVENT_ON_DUMMY_EFFECT, pTarget);
    HookPush(pCaster);
    HookPush(spellId);
    HookPush(effIndex);
    HookPush(pTarget);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

bool Eluna::OnQuestAccept(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_QUEST_ACCEPT, pCreature, false);
    HookPush(pPlayer);
    HookPush(pCreature);
    HookPush(pQuest);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

bool Eluna::OnQuestReward(Player* pPlayer, Creature* pCreature, Quest const* pQuest, uint32 opt)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_QUEST_REWARD, pCreature, false);
    HookPush(pPlayer);
    HookPush(pCreature);
    HookPush(pQuest);
    HookPush(opt);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

void Eluna::GetDialogStatus(const Player* pPlayer, const Creature* pCreature)
{
    START_HOOK(CREATURE_EVENT_ON_DIALOG_STATUS, pCreature);
    HookPush(pPlayer);
    HookPush(pCreature);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

void Eluna::OnAddToWorld(Creature* pCreature)
{
    START_HOOK(CREATURE_EVENT_ON_ADD, pCreature);
    HookPush(pCreature);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

void Eluna::OnRemoveFromWorld(Creature* pCreature)
{
    START_HOOK(CREATURE_EVENT_ON_REMOVE, pCreature);
    HookPush(pCreature);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

bool Eluna::OnSummoned(Creature* pCreature, Unit* pSummoner)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SUMMONED, pCreature, false);
    HookPush(pCreature);
    HookPush(pSummoner);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

bool Eluna::UpdateAI(Creature* me, const uint32 diff)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_AIUPDATE, me, false);
    HookPush(me);
    HookPush(diff);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

//Called for reaction at enter to combat if not in combat yet (enemy can be NULL)
//Called at creature aggro either by MoveInLOS or Attack Start
bool Eluna::EnterCombat(Creature* me, Unit* target)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_ENTER_COMBAT, me, false);
    HookPush(me);
    HookPush(target);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called at any Damage from any attacker (before damage apply)
bool Eluna::DamageTaken(Creature* me, Unit* attacker, uint32& damage)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_DAMAGE_TAKEN, me, false);
    bool result = false;
    HookPush(me);
    HookPush(attacker);
    HookPush(damage);
    int damageIndex = lua_gettop(L);
    int n = SetupStack(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key, 3);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 3, 2);

        if (lua_isboolean(L, r + 0) && lua_toboolean(L, r + 0))
            result = true;

        if (lua_isnumber(L, r + 1))
        {
            damage = CHECKVAL<uint32>(r + 1);
            // Update the stack for subsequent calls.
            ReplaceArgument(damage, damageIndex);
        }

        lua_pop(L, 2);
    }

    CleanUpStack(3);
    return result;
}

//Called at creature death
bool Eluna::JustDied(Creature* me, Unit* killer)
{
    On_Reset(me);
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_DIED, me, false);
    HookPush(me);
    HookPush(killer);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

//Called at creature killing another unit
bool Eluna::KilledUnit(Creature* me, Unit* victim)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_TARGET_DIED, me, false);
    HookPush(me);
    HookPush(victim);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when the creature summon successfully other creature
bool Eluna::JustSummoned(Creature* me, Creature* summon)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_JUST_SUMMONED_CREATURE, me, false);
    HookPush(me);
    HookPush(summon);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when a summoned creature is despawned
bool Eluna::SummonedCreatureDespawn(Creature* me, Creature* summon)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SUMMONED_CREATURE_DESPAWN, me, false);
    HookPush(me);
    HookPush(summon);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

//Called at waypoint reached or PointMovement end
bool Eluna::MovementInform(Creature* me, uint32 type, uint32 id)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_REACH_WP, me, false);
    HookPush(me);
    HookPush(type);
    HookPush(id);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called before EnterCombat even before the creature is in combat.
bool Eluna::AttackStart(Creature* me, Unit* target)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_PRE_COMBAT, me, false);
    HookPush(me);
    HookPush(target);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called for reaction at stopping attack at no attackers or targets
bool Eluna::EnterEvadeMode(Creature* me)
{
    On_Reset(me);
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_LEAVE_COMBAT, me, false);
    HookPush(me);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when creature is spawned or respawned (for reseting variables)
bool Eluna::JustRespawned(Creature* me)
{
    On_Reset(me);
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SPAWN, me, false);
    HookPush(me);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called at reaching home after evade
bool Eluna::JustReachedHome(Creature* me)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_REACH_HOME, me, false);
    HookPush(me);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called at text emote receive from player
bool Eluna::ReceiveEmote(Creature* me, Player* player, uint32 emoteId)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_RECEIVE_EMOTE, me, false);
    HookPush(me);
    HookPush(player);
    HookPush(emoteId);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// called when the corpse of this creature gets removed
bool Eluna::CorpseRemoved(Creature* me, uint32& respawnDelay)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_CORPSE_REMOVED, me, false);
    bool result = false;
    HookPush(me);
    HookPush(respawnDelay);
    int respawnDelayIndex = lua_gettop(L);
    int n = SetupStack(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key, 2);

    while (n > 0)
    {
        int r = CallOneFunction(n--, 2, 2);

        if (lua_isboolean(L, r + 0) && lua_toboolean(L, r + 0))
            result = true;

        if (lua_isnumber(L, r + 1))
        {
            respawnDelay = CHECKVAL<uint32>(r + 1);
            // Update the stack for subsequent calls.
            ReplaceArgument(respawnDelay, respawnDelayIndex);
        }

        lua_pop(L, 2);
    }

    CleanUpStack(2);
    return result;
}

bool Eluna::MoveInLineOfSight(Creature* me, Unit* who)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_MOVE_IN_LOS, me, false);
    HookPush(me);
    HookPush(who);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called on creature initial spawn, respawn, death, evade (leave combat)
void Eluna::On_Reset(Creature* me) // Not an override, custom
{
    START_HOOK(CREATURE_EVENT_ON_RESET, me);
    HookPush(me);
    CallAllFunctions(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when hit by a spell
bool Eluna::SpellHit(Creature* me, WorldObject* caster, SpellInfo const* spell)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_HIT_BY_SPELL, me, false);
    HookPush(me);
    HookPush(caster);
    HookPush(spell->Id); // Pass spell object?
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when spell hits a target
bool Eluna::SpellHitTarget(Creature* me, WorldObject* target, SpellInfo const* spell)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SPELL_HIT_TARGET, me, false);
    HookPush(me);
    HookPush(target);
    HookPush(spell->Id); // Pass spell object?
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

#if defined ELUNA_TRINITY

bool Eluna::SummonedCreatureDies(Creature* me, Creature* summon, Unit* killer)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_SUMMONED_CREATURE_DIED, me, false);
    HookPush(me);
    HookPush(summon);
    HookPush(killer);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when owner takes damage
bool Eluna::OwnerAttackedBy(Creature* me, Unit* attacker)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_OWNER_ATTACKED_AT, me, false);
    HookPush(me);
    HookPush(attacker);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

// Called when owner attacks something
bool Eluna::OwnerAttacked(Creature* me, Unit* target)
{
    START_HOOK_WITH_RETVAL(CREATURE_EVENT_ON_OWNER_ATTACKED, me, false);
    HookPush(me);
    HookPush(target);
    return CallAllFunctionsBool(CreatureEventBindings, CreatureUniqueBindings, entry_key, unique_key);
}

#endif // ELUNA_TRINITY
