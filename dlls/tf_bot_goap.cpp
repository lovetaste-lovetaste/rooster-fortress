//=============================================================================
// bot_goap.cpp  –  GOAP Bot implementation for halflife-updated SDK
//
// Compile alongside the other dlls/ sources.
// Add bot_goap.cpp + bot_goap.h to your CMakeLists or MSVC project.
//=============================================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "tf_bot_goap.h"

#include <algorithm>
#include <unordered_map>
#include <queue>

// Link the bot class to a Half-Life entity classname so the engine can
// instantiate it.  You may want a separate classname like "bot_player".
LINK_ENTITY_TO_CLASS(bot_player, CHL1Bot);

//=============================================================================
// Constants
//=============================================================================
static constexpr float BOT_LOW_HEALTH_THRESHOLD = 40.0f;
static constexpr float BOT_WEAPON_RANGE_NEAR = 256.0f;
static constexpr float BOT_WEAPON_RANGE_FAR = 1024.0f;
static constexpr float BOT_REPLAN_INTERVAL = 0.35f; // seconds between replans
static constexpr float BOT_SCAN_INTERVAL = 0.5f;	// seconds between env scans
static constexpr float BOT_TURN_RATE = 8.0f;		// degrees-per-frame scale
static constexpr float BOT_MOVE_SPEED = 220.0f;

//=============================================================================
// CHL1Bot  –  Spawn / Precache
//=============================================================================
void CHL1Bot::Spawn()
{
	CBasePlayer::Spawn();

	// Mark as a bot so game-rules / other systems can identify us
	pev->flags |= FL_FAKECLIENT;

	// Build action & goal libraries once
	m_actions.clear();
	m_goals.clear();
	RegisterActions();
	RegisterGoals();

	m_pCurrentGoal = nullptr;
	m_planStep = 0;
	m_flNextPlanTime = 0.0f;
	m_flNextScanTime = 0.0f;
	m_bHasMoveGoal = false;
}

void CHL1Bot::Precache()
{
	CBasePlayer::Precache();
}

//=============================================================================
// Engine-called Think hooks  –  forward to BotThink so your external
// Bot_Think() dispatcher and the engine's own scheduling both work.
//=============================================================================
void CHL1Bot::Think()
{
	BotThink();
	pev->nextthink = gpGlobals->time + 0.05f;
}

void CHL1Bot::PreThink()
{
	// Let the base player handle movement physics, etc.
	CBasePlayer::PreThink();
}

void CHL1Bot::PostThink()
{
	CBasePlayer::PostThink();
}

//=============================================================================
// BotThink  –  primary entry point called by YOUR Bot_Think() dispatcher.
//
// Your dispatcher iterates client slots and calls this; the engine's Think()
// also forwards here, so if both paths fire in the same frame that's fine
// because the throttles inside guard against redundant work.
//=============================================================================
void CHL1Bot::BotThink()
{
	// Don't think while dead / being respawned
	if (pev->deadflag != DEAD_NO)
		return;

	//-- 1. Periodically scan the environment and update memory
	if (gpGlobals->time >= m_flNextScanTime)
	{
		m_flNextScanTime = gpGlobals->time + BOT_SCAN_INTERVAL;

		// Enemy scan
		CBaseEntity* pEnemy = FindNearestEnemy();
		if (pEnemy)
		{
			m_memory.hEnemy = pEnemy;
			m_memory.vecLastEnemyPos = pEnemy->pev->origin;
			m_memory.flLastEnemySeen = gpGlobals->time;
		}
		else if (gpGlobals->time - m_memory.flLastEnemySeen > 10.0f)
		{
			// Forget stale enemy after 10 s of no contact
			m_memory.hEnemy = nullptr;
		}

		// Health-pack scan (only when low)
		if (pev->health < BOT_LOW_HEALTH_THRESHOLD)
			m_memory.hHealthPack = FindNearestHealthPack();
	}

	//-- 2. Build world state from current conditions
	BuildWorldState();

	//-- 3. Re-plan if enough time has passed or the current plan is done
	bool needReplan = (gpGlobals->time >= m_flNextPlanTime) || m_currentPlan.empty() || m_planStep >= (int)m_currentPlan.size();

	if (needReplan)
	{
		m_flNextPlanTime = gpGlobals->time + BOT_REPLAN_INTERVAL;
		PlanGoals();
	}

	//-- 4. Execute the current plan step
	ExecuteCurrentPlan();
}

//=============================================================================
// BuildWorldState  –  snapshot the bot's situation into the world-state struct
//=============================================================================
void CHL1Bot::BuildWorldState()
{
	CBaseEntity* pEnemy = m_memory.hEnemy;

	m_worldState.Set(WS_ALIVE, pev->deadflag == DEAD_NO);
	m_worldState.Set(WS_HAS_TARGET, pEnemy != nullptr);
	m_worldState.Set(WS_LOW_HEALTH, pev->health < BOT_LOW_HEALTH_THRESHOLD);
	m_worldState.Set(WS_HAS_HEALTH_PACK, (CBaseEntity*)m_memory.hHealthPack != nullptr);
	m_worldState.Set(WS_AT_COVER, false); // updated by cover action

	// Target-specific conditions
	bool targetDead = false;
	bool targetInRange = false;
	if (pEnemy)
	{
		targetDead = (pEnemy->pev->deadflag != DEAD_NO || pEnemy->pev->health <= 0);
		float dist = (pEnemy->pev->origin - pev->origin).Length();
		targetInRange = (dist <= BOT_WEAPON_RANGE_FAR);
	}
	m_worldState.Set(WS_TARGET_DEAD, targetDead);
	m_worldState.Set(WS_TARGET_IN_RANGE, targetInRange);

	// Weapon state
	bool weaponReady = true;
	bool weaponReloading = false;
	if (m_pActiveItem)
	{
		CBasePlayerWeapon* pWeapon =
			static_cast<CBasePlayerWeapon*>(m_pActiveItem->GetWeaponPtr());
		if (pWeapon)
		{
			weaponReady = (pWeapon->m_iClip > 0);
			weaponReloading = (pWeapon->m_fInReload != 0);
		}
	}
	m_worldState.Set(WS_WEAPON_READY, weaponReady);
	m_worldState.Set(WS_WEAPON_RELOADING, weaponReloading);
}

//=============================================================================
// RegisterGoals  –  define what the bot wants to achieve
//=============================================================================
void CHL1Bot::RegisterGoals()
{
	// Goal: Kill the target
	{
		CGOAPGoal g("KillTarget", 3.0f);
		g.SetDesired(WS_TARGET_DEAD, true);
		g.GetPriority = [](CHL1Bot* pBot) -> float
		{
			// High priority only when an enemy is known
			return pBot->GetWorldState().Get(WS_HAS_TARGET) ? 3.0f : 0.0f;
		};
		m_goals.push_back(g);
	}

	// Goal: Heal self
	{
		CGOAPGoal g("HealSelf", 2.0f);
		g.SetDesired(WS_LOW_HEALTH, false);
		g.GetPriority = [](CHL1Bot* pBot) -> float
		{
			return pBot->GetWorldState().Get(WS_LOW_HEALTH) ? 2.0f : 0.0f;
		};
		m_goals.push_back(g);
	}

	// Goal: Patrol / idle (always applicable, lowest priority)
	{
		CGOAPGoal g("Patrol", 0.5f);
		g.SetDesired(WS_ALIVE, true); // trivially satisfiable; forces patrol actions
		m_goals.push_back(g);
	}
}

//=============================================================================
// RegisterActions  –  define the bot's action vocabulary
//=============================================================================
void CHL1Bot::RegisterActions()
{
	//------------------------------------------------------------------
	// Action: Move to enemy (closes range so we can shoot)
	//------------------------------------------------------------------
	{
		CGOAPAction a("MoveToEnemy", 1.0f);
		a.SetPrecondition(WS_HAS_TARGET, true);
		a.SetPrecondition(WS_TARGET_DEAD, false);
		a.SetEffect(WS_TARGET_IN_RANGE, true);

		a.IsValid = [](CHL1Bot* pBot) -> bool
		{
			return (CBaseEntity*)pBot->GetMemory().hEnemy != nullptr;
		};

		a.Execute = [](CHL1Bot* pBot) -> bool
		{
			CBaseEntity* pEnemy = pBot->GetMemory().hEnemy;
			if (!pEnemy)
				return true; // abort, nothing to chase

			Vector vecTarget = pEnemy->pev->origin;
			float dist = (vecTarget - pBot->pev->origin).Length();

			pBot->FaceTarget(vecTarget);

			if (dist > BOT_WEAPON_RANGE_NEAR)
			{
				pBot->MoveToward(vecTarget, BOT_MOVE_SPEED);
				return false; // not done yet
			}

			pBot->StopMoving();
			return true; // now in range – step complete
		};
		m_actions.push_back(a);
	}

	//------------------------------------------------------------------
	// Action: Shoot enemy
	//------------------------------------------------------------------
	{
		CGOAPAction a("ShootEnemy", 1.0f);
		a.SetPrecondition(WS_HAS_TARGET, true);
		a.SetPrecondition(WS_TARGET_IN_RANGE, true);
		a.SetPrecondition(WS_TARGET_DEAD, false);
		a.SetPrecondition(WS_WEAPON_READY, true);
		a.SetEffect(WS_TARGET_DEAD, true);

		a.Execute = [](CHL1Bot* pBot) -> bool
		{
			CBaseEntity* pEnemy = pBot->GetMemory().hEnemy;
			if (!pEnemy || pEnemy->pev->health <= 0)
				return true;

			pBot->FaceTarget(pEnemy->pev->origin);
			pBot->BotFireWeapon();

			// Re-check target death next world-state build
			return (pEnemy->pev->health <= 0);
		};
		m_actions.push_back(a);
	}

	//------------------------------------------------------------------
	// Action: Reload weapon
	//------------------------------------------------------------------
	{
		CGOAPAction a("Reload", 1.5f);
		a.SetPrecondition(WS_WEAPON_READY, false);
		a.SetEffect(WS_WEAPON_READY, true);

		a.IsValid = [](CHL1Bot* pBot) -> bool
		{
			return pBot->m_pActiveItem != nullptr;
		};

		a.Execute = [](CHL1Bot* pBot) -> bool
		{
			if (!pBot->m_pActiveItem)
				return true;

			CBasePlayerWeapon* pWeapon =
				static_cast<CBasePlayerWeapon*>(pBot->m_pActiveItem->GetWeaponPtr());
			if (!pWeapon)
				return true;

			if (pWeapon->m_fInReload)
				return false; // wait for reload animation

			if (pWeapon->m_iClip <= 0)
			{
				// Simulate pressing the reload key
				pBot->pev->button |= IN_RELOAD;
				return false;
			}
			pBot->pev->button &= ~IN_RELOAD;
			return true;
		};
		m_actions.push_back(a);
	}

	//------------------------------------------------------------------
	// Action: Fetch health pack
	//------------------------------------------------------------------
	{
		CGOAPAction a("FetchHealth", 2.0f);
		a.SetPrecondition(WS_LOW_HEALTH, true);
		a.SetPrecondition(WS_HAS_HEALTH_PACK, true);
		a.SetEffect(WS_LOW_HEALTH, false);

		a.IsValid = [](CHL1Bot* pBot) -> bool
		{
			return (CBaseEntity*)pBot->GetMemory().hHealthPack != nullptr;
		};

		a.Execute = [](CHL1Bot* pBot) -> bool
		{
			CBaseEntity* pPack = pBot->GetMemory().hHealthPack;
			if (!pPack)
				return true; // pack disappeared

			float dist = (pPack->pev->origin - pBot->pev->origin).Length();
			if (dist > 40.0f)
			{
				pBot->FaceTarget(pPack->pev->origin);
				pBot->MoveToward(pPack->pev->origin, BOT_MOVE_SPEED);
				return false;
			}

			// Close enough; USE the pack (touch should auto-trigger it, but
			// also press +USE to be safe with chargers)
			pBot->pev->button |= IN_USE;
			pBot->StopMoving();
			// Consider done once health is no longer low
			return (pBot->pev->health >= BOT_LOW_HEALTH_THRESHOLD);
		};
		m_actions.push_back(a);
	}

	//------------------------------------------------------------------
	// Action: Patrol (wander; trivially satisfies the Patrol goal)
	//------------------------------------------------------------------
	{
		CGOAPAction a("Patrol", 1.0f);
		// No mandatory preconditions  — always applicable as a fallback
		a.SetEffect(WS_ALIVE, true);

		a.Execute = [](CHL1Bot* pBot) -> bool
		{
			// Simple random wander: every few seconds pick a new direction
			static float s_flNextWander = 0.0f;
			if (gpGlobals->time >= s_flNextWander)
			{
				s_flNextWander = gpGlobals->time + RANDOM_FLOAT(2.0f, 5.0f);

				// Pick a random yaw and walk forward for a moment
				float yaw = RANDOM_FLOAT(0.0f, 360.0f);
				pBot->pev->angles.y = yaw;

				// Build a move-goal 256 units ahead in that direction
				Vector fwd;
				UTIL_MakeVectorsPrivate(Vector(0, yaw, 0), fwd, nullptr, nullptr);
				pBot->m_vecMoveGoal = pBot->pev->origin + fwd * 256.0f;
				pBot->m_bHasMoveGoal = true;
			}

			if (pBot->m_bHasMoveGoal)
			{
				float dist = (pBot->m_vecMoveGoal - pBot->pev->origin).Length2D();
				if (dist > 32.0f)
					pBot->MoveToward(pBot->m_vecMoveGoal, BOT_MOVE_SPEED * 0.6f);
				else
					pBot->m_bHasMoveGoal = false;
			}
			return false; // patrol never "completes"; runs until re-plan
		};
		m_actions.push_back(a);
	}
}

//=============================================================================
// PlanGoals  –  choose the highest-priority applicable goal and build a plan
//=============================================================================
void CHL1Bot::PlanGoals()
{
	CGOAPGoal* pBest = nullptr;
	float bestPrio = -1.0f;

	for (auto& goal : m_goals)
	{
		if (SatisfiesGoal(m_worldState, goal))
			continue; // already satisfied

		float prio = goal.GetPriority(this);
		if (prio > bestPrio)
		{
			bestPrio = prio;
			pBest = &goal;
		}
	}

	if (!pBest)
	{
		// All goals satisfied – idle
		m_currentPlan.clear();
		m_planStep = 0;
		return;
	}

	if (pBest == m_pCurrentGoal && !m_currentPlan.empty())
		return; // same goal, keep current plan

	m_pCurrentGoal = pBest;

	auto newPlan = AStarPlan(m_worldState, *pBest);
	if (!newPlan.empty())
	{
		m_currentPlan = newPlan;
		m_planStep = 0;
	}
}

//=============================================================================
// ExecuteCurrentPlan  –  run the current action step
//=============================================================================
void CHL1Bot::ExecuteCurrentPlan()
{
	if (m_currentPlan.empty() || m_planStep >= (int)m_currentPlan.size())
		return;

	CGOAPAction* pAction = m_currentPlan[m_planStep];

	// Extra run-time validity guard
	if (pAction->IsValid && !pAction->IsValid(this))
	{
		// Action is no longer valid; invalidate plan so next tick replans
		m_currentPlan.clear();
		m_planStep = 0;
		return;
	}

	bool done = false;
	if (pAction->Execute)
		done = pAction->Execute(this);

	if (done)
		m_planStep++;
}

//=============================================================================
// Planner helpers
//=============================================================================
bool CHL1Bot::SatisfiesGoal(const CWorldState& state, const CGOAPGoal& goal) const
{
	for (int i = 0; i < WS_COUNT; ++i)
	{
		if (goal.desiredMask[i] && state.values[i] != goal.desired.values[i])
			return false;
	}
	return true;
}

bool CHL1Bot::MeetsPreconditions(const CWorldState& state, const CGOAPAction& action) const
{
	for (int i = 0; i < WS_COUNT; ++i)
	{
		if (action.preconMask[i] && state.values[i] != action.preconditions.values[i])
			return false;
	}
	return true;
}

void CHL1Bot::ApplyEffects(CWorldState& state, const CGOAPAction& action) const
{
	for (int i = 0; i < WS_COUNT; ++i)
	{
		// Effects always apply, whether or not precondition mask covers that key
		// But we only apply keys that have an explicit effect defined:
		// We use the same preconMask trick: use a separate effectMask.
		// For simplicity we apply ALL effect values — the action builder only
		// calls SetEffect() for keys it intentionally changes.
		state.values[i] = action.effects.values[i];
	}
}

//=============================================================================
// AStarPlan  –  forward-search over the action graph
//
//  We represent a search node as (worldState, costSoFar, path).
//  Because the state space is tiny (WS_COUNT booleans) we use a simple
//  priority queue; the heuristic is the number of unsatisfied goal keys.
//=============================================================================
struct PlanNode
{
	CWorldState state;
	float g; // cost so far
	float f; // g + h
	std::vector<CGOAPAction*> path;

	bool operator>(const PlanNode& o) const { return f > o.f; }
};

static float Heuristic(const CWorldState& state, const CGOAPGoal& goal)
{
	float h = 0.0f;
	for (int i = 0; i < WS_COUNT; ++i)
		if (goal.desiredMask[i] && state.values[i] != goal.desired.values[i])
			h += 1.0f;
	return h;
}

std::vector<CGOAPAction*> CHL1Bot::AStarPlan(const CWorldState& start,
	const CGOAPGoal& goal)
{
	std::priority_queue<PlanNode, std::vector<PlanNode>, std::greater<PlanNode>> open;

	PlanNode root;
	root.state = start;
	root.g = 0.0f;
	root.f = Heuristic(start, goal);
	open.push(root);

	constexpr int MAX_ITERATIONS = 128;
	int iters = 0;

	while (!open.empty() && iters < MAX_ITERATIONS)
	{
		++iters;
		PlanNode current = open.top();
		open.pop();

		if (SatisfiesGoal(current.state, goal))
			return current.path;

		for (auto& action : m_actions)
		{
			if (!MeetsPreconditions(current.state, action))
				continue;

			// Run-time validity (e.g. do we have ammo?)
			if (action.IsValid && !action.IsValid(this))
				continue;

			CWorldState newState = current.state;
			ApplyEffects(newState, action);

			// Avoid infinite loops: skip if this state was already better
			float newG = current.g + action.cost;
			PlanNode next;
			next.state = newState;
			next.g = newG;
			next.f = newG + Heuristic(newState, goal);
			next.path = current.path;
			next.path.push_back(const_cast<CGOAPAction*>(&action));

			open.push(next);
		}
	}

	return {}; // no plan found
}

//=============================================================================
// Environment queries
//=============================================================================
CBaseEntity* CHL1Bot::FindNearestEnemy()
{
	CBaseEntity* pNearest = nullptr;
	float flNearest = BOT_WEAPON_RANGE_FAR * 2.0f;

	CBaseEntity* pEnt = nullptr;
	while ((pEnt = UTIL_FindEntityInSphere(pEnt, pev->origin, BOT_WEAPON_RANGE_FAR * 2.0f)) != nullptr)
	{
		if (pEnt == this)
			continue;

		// Target: players on opposing team, or any entity that hates us
		if (!pEnt->IsPlayer())
			continue;

		CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEnt);
		if (pPlayer->pev->deadflag != DEAD_NO)
			continue;

		// In deathmatch everyone is an enemy
		if (!g_pGameRules->IsDeathmatch())
			continue;

		if (!LineOfSightTo(pEnt))
			continue;

		float dist = (pEnt->pev->origin - pev->origin).Length();
		if (dist < flNearest)
		{
			flNearest = dist;
			pNearest = pEnt;
		}
	}
	return pNearest;
}

CBaseEntity* CHL1Bot::FindNearestHealthPack()
{
	CBaseEntity* pNearest = nullptr;
	float flNearest = 1024.0f;

	CBaseEntity* pEnt = nullptr;
	while ((pEnt = UTIL_FindEntityByClassname(pEnt, "item_healthkit")) != nullptr)
	{
		float dist = (pEnt->pev->origin - pev->origin).Length();
		if (dist < flNearest)
		{
			flNearest = dist;
			pNearest = pEnt;
		}
	}
	// Also check wall chargers
	pEnt = nullptr;
	while ((pEnt = UTIL_FindEntityByClassname(pEnt, "func_healthcharger")) != nullptr)
	{
		float dist = (pEnt->pev->origin - pev->origin).Length();
		if (dist < flNearest)
		{
			flNearest = dist;
			pNearest = pEnt;
		}
	}
	return pNearest;
}

bool CHL1Bot::LineOfSightTo(CBaseEntity* pTarget)
{
	if (!pTarget)
		return false;

	TraceResult tr;
	Vector vecEye = pev->origin + pev->view_ofs;
	Vector vecTarget = pTarget->pev->origin + pTarget->pev->view_ofs;

	UTIL_TraceLine(vecEye, vecTarget, dont_ignore_monsters, edict(), &tr);
	return (tr.flFraction >= 0.99f || tr.pHit == pTarget->edict());
}

//=============================================================================
// Movement & combat helpers
//=============================================================================
void CHL1Bot::FaceTarget(const Vector& vecTarget, float flRate)
{
	Vector vecDir = (vecTarget - pev->origin).Normalize();
	Vector vecAngles = UTIL_VecToAngles(vecDir);

	// Smooth yaw rotation
	float yawDiff = vecAngles.y - pev->v_angle.y;
	// Normalise to [-180, 180]
	while (yawDiff > 180.0f)
		yawDiff -= 360.0f;
	while (yawDiff < -180.0f)
		yawDiff += 360.0f;

	pev->v_angle.y += yawDiff * (flRate * gpGlobals->frametime);
	pev->v_angle.x = vecAngles.x; // pitch directly
	pev->angles.y = pev->v_angle.y;
	pev->ideal_yaw = pev->v_angle.y;
}

void CHL1Bot::MoveToward(const Vector& vecDest, float flSpeed)
{
	Vector vecDir = (vecDest - pev->origin).Normalize();
	pev->velocity = vecDir * flSpeed;

	// Hold forward button so animation plays correctly
	pev->button |= IN_FORWARD;
}

void CHL1Bot::StopMoving()
{
	pev->velocity = vec3_origin;
	pev->button &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
}

void CHL1Bot::BotFireWeapon()
{
	if (!m_pActiveItem)
		return;

	CBasePlayerWeapon* pWeapon =
		static_cast<CBasePlayerWeapon*>(m_pActiveItem->GetWeaponPtr());
	if (!pWeapon || pWeapon->m_iClip <= 0)
		return;

	// Simulate holding the attack key
	pev->button |= IN_ATTACK;
}
