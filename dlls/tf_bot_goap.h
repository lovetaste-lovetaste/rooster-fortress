//=============================================================================
// bot_goap.h  –  GOAP Bot for halflife-updated SDK
//
// Derives from CBasePlayer (which itself derives from CBaseMonster).
// Your external Bot_Think() dispatcher calls CHL1Bot::BotThink() each frame.
//=============================================================================

#pragma once

#include "player.h"	  // CBasePlayer  (dlls/player.h)
#include "monsters.h" // activity / schedule helpers
#include <vector>
#include <functional>
#include <string>

//-----------------------------------------------------------------------------
// WorldState  –  a flat key/value snapshot of conditions the planner reasons
//               over.  Keep it small; the planner copies it frequently.
//-----------------------------------------------------------------------------
enum EWSKey
{
	WS_ALIVE = 0,		 // bool – bot is alive
	WS_HAS_TARGET,		 // bool – a valid enemy is known
	WS_TARGET_DEAD,		 // bool – current target is dead
	WS_TARGET_IN_RANGE,	 // bool – target within weapon range
	WS_LOW_HEALTH,		 // bool – health below threshold
	WS_HAS_HEALTH_PACK,	 // bool – a health-pack entity is nearby
	WS_AT_COVER,		 // bool – bot is behind cover
	WS_WEAPON_READY,	 // bool – active weapon has ammo
	WS_WEAPON_RELOADING, // bool – currently reloading

	WS_COUNT
};

struct CWorldState
{
	bool values[WS_COUNT];

	CWorldState() { memset(values, 0, sizeof(values)); }

	bool operator==(const CWorldState& o) const
	{
		return memcmp(values, o.values, sizeof(values)) == 0;
	}

	// Convenience helpers
	void Set(EWSKey k, bool v) { values[k] = v; }
	bool Get(EWSKey k) const { return values[k]; }
};

//-----------------------------------------------------------------------------
// GOAPAction  –  one discrete action in the planner's vocabulary.
// Each action has:
//   • preconditions  (what must be true in the world before it can run)
//   • effects        (what it changes in the world after it completes)
//   • cost           (lower = planner prefers it)
//   • IsValid()      (run-time check, e.g. "do I actually have ammo?")
//   • Execute()      (called every frame while the action is active;
//                     returns true when the action has finished)
//-----------------------------------------------------------------------------
class CHL1Bot; // forward

struct CGOAPAction
{
	std::string name;
	CWorldState preconditions; // only Set() keys are checked
	CWorldState effects;
	float cost = 1.0f;

	// Which precondition keys actually matter for this action
	bool preconMask[WS_COUNT] = {};

	// Callbacks – set by the bot
	std::function<bool(CHL1Bot*)> IsValid; // optional extra guard
	std::function<bool(CHL1Bot*)> Execute; // returns true = done

	CGOAPAction() = default;
	CGOAPAction(const char* n, float c) : name(n), cost(c)
	{
		IsValid = [](CHL1Bot*)
		{ return true; };
		Execute = [](CHL1Bot*)
		{ return true; };
	}

	void SetPrecondition(EWSKey k, bool v)
	{
		preconditions.Set(k, v);
		preconMask[k] = true;
	}
	void SetEffect(EWSKey k, bool v)
	{
		effects.Set(k, v);
	}
};

//-----------------------------------------------------------------------------
// GOAPGoal  –  a desired world state the bot wants to reach.
// Goals are prioritised; the planner picks the highest-priority valid goal.
//-----------------------------------------------------------------------------
struct CGOAPGoal
{
	std::string name;
	CWorldState desired;
	bool desiredMask[WS_COUNT] = {}; // which keys this goal cares about
	float priority = 1.0f;

	// Returns current priority (can be dynamic)
	std::function<float(CHL1Bot*)> GetPriority;

	CGOAPGoal()
	{
		GetPriority = [this](CHL1Bot*)
		{ return priority; };
	}
	CGOAPGoal(const char* n, float p) : name(n), priority(p)
	{
		GetPriority = [this](CHL1Bot*)
		{ return priority; };
	}

	void SetDesired(EWSKey k, bool v)
	{
		desired.Set(k, v);
		desiredMask[k] = true;
	}
};

//-----------------------------------------------------------------------------
// BotMemory  –  simple run-time blackboard shared between planner & actions
//-----------------------------------------------------------------------------
struct CBotMemory
{
	EHANDLE hEnemy;			// current combat target
	EHANDLE hHealthPack;	// nearest health-pack we know about
	EHANDLE hCoverSpot;		// cover node entity (can be nullptr)
	Vector vecLastEnemyPos; // last seen position of enemy
	float flLastEnemySeen;	// gpGlobals->time when last seen
	float flNextScanTime;	// when to next run a full scan
	float flNextReloadCheck;

	CBotMemory()
		: vecLastEnemyPos(vec3_origin), flLastEnemySeen(0), flNextScanTime(0), flNextReloadCheck(0)
	{
	}
};

//-----------------------------------------------------------------------------
// CHL1Bot  –  the actual bot class, derived from CBasePlayer
//-----------------------------------------------------------------------------
class CHL1Bot : public CBasePlayer
{
public:
	//-------------------------------------------------------------------------
	// Required CBaseEntity / CBasePlayer overrides
	//-------------------------------------------------------------------------
	void Spawn() override;
	void Precache() override;
	void Think() override; // Called by the engine; delegates to BotThink
	void PreThink() override;
	void PostThink() override;
	void UpdateClientData() override {} // bots don't need HUD updates

	int Classify() override { return CLASS_PLAYER_ALLY; }
	bool IsBot() const { return true; }

	//-------------------------------------------------------------------------
	// Your external dispatcher calls this every frame for each bot slot
	//-------------------------------------------------------------------------
	void BotThink();

	//-------------------------------------------------------------------------
	// GOAP planner interface
	//-------------------------------------------------------------------------
	void BuildWorldState();
	void PlanGoals();
	CGOAPGoal* SelectBestGoal();
	std::vector<CGOAPAction*> PlanActions(const CGOAPGoal& goal);
	void ExecuteCurrentPlan();

	//-------------------------------------------------------------------------
	// World-state accessors (used by actions)
	//-------------------------------------------------------------------------
	CWorldState& GetWorldState() { return m_worldState; }
	CBotMemory& GetMemory() { return m_memory; }

	//-------------------------------------------------------------------------
	// High-level helper utilities (used by actions)
	//-------------------------------------------------------------------------
	CBaseEntity* FindNearestEnemy();
	CBaseEntity* FindNearestHealthPack();
	bool LineOfSightTo(CBaseEntity* pTarget);
	void FaceTarget(const Vector& vecTarget, float flRate = 10.0f);
	void MoveToward(const Vector& vecDest, float flSpeed = 200.0f);
	void StopMoving();
	void BotFireWeapon();

private:
	//-------------------------------------------------------------------------
	// Internal planner helpers
	//-------------------------------------------------------------------------
	bool SatisfiesGoal(const CWorldState& state, const CGOAPGoal& goal) const;
	bool MeetsPreconditions(const CWorldState& state, const CGOAPAction& action) const;
	void ApplyEffects(CWorldState& state, const CGOAPAction& action) const;
	// A* over action graph
	std::vector<CGOAPAction*> AStarPlan(const CWorldState& start,
		const CGOAPGoal& goal);

	//-------------------------------------------------------------------------
	// Action & goal tables (built once in Spawn)
	//-------------------------------------------------------------------------
	void RegisterActions();
	void RegisterGoals();

	std::vector<CGOAPAction> m_actions;
	std::vector<CGOAPGoal> m_goals;

	//-------------------------------------------------------------------------
	// Runtime planner state
	//-------------------------------------------------------------------------
	CWorldState m_worldState;
	CBotMemory m_memory;

	CGOAPGoal* m_pCurrentGoal = nullptr;
	std::vector<CGOAPAction*> m_currentPlan; // ordered list of actions
	int m_planStep = 0;

	float m_flNextPlanTime = 0.0f; // re-plan throttle
	float m_flNextScanTime = 0.0f;

	//-------------------------------------------------------------------------
	// Navigation / movement cache
	//-------------------------------------------------------------------------
	Vector m_vecMoveGoal;
	bool m_bHasMoveGoal = false;
};

//-----------------------------------------------------------------------------
// Inline: retrieve a CHL1Bot* from a client slot edict
// Usage: CHL1Bot* pBot = BotFromEdict( pEdict );
//        if (pBot) pBot->BotThink();
//-----------------------------------------------------------------------------
inline CHL1Bot* BotFromEdict(edict_t* pEdict)
{
	if (!pEdict || pEdict->free)
		return nullptr;
	CBaseEntity* pEnt = CBaseEntity::Instance(pEdict);
	if (!pEnt)
		return nullptr;
	CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(pEnt);
	if (!pPlayer)
		return nullptr;
	return dynamic_cast<CHL1Bot*>(pPlayer);
}
