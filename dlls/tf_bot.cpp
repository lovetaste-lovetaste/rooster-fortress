/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
// Robin, 4-22-98: Moved set_suicide_frame() here from player.cpp to allow us to
//				   have one without a hardcoded player.mdl in tf_client.cpp

/*

===== tf_bot.cpp ========================================================

  bot stuff :O

*/

#include <algorithm>
#include <string>
#include <vector>

#include "extdll.h"
#include "util.h"
#include "filesystem_utils.h"
#include "cbase.h"
#include "com_model.h"
#include "saverestore.h"
#include "player.h"
#include "spectator.h"
#include "client.h"
#include "soundent.h"
#include "gamerules.h"
#include "game.h"
#include "customentity.h"
#include "weapons.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
#include "pm_shared.h"
#include "pm_defs.h"
#include "UserMessages.h"
#include "tf_bot.h"

// CODE GRABBED FROM THE TF2SDK

static float g_LastBotUpdateTime[34];
static CBaseEntity* enemy[40];

inline bool isTempBot(int client)
{
	auto pPlayer = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(client));

	return (pPlayer && pPlayer->m_bIsConnected && (pPlayer->pev->flags & FL_FAKECLIENT) != 0);
}

//-----------------------------------------------------------------------------
// Purpose: Run this Bot's AI for one frame.
//-----------------------------------------------------------------------------
void Bot_Think(int client)
{
	auto pBot = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(client));

	// Make sure we stay being a bot
	if ((pBot->pev->flags & FL_FAKECLIENT) != 0)
		pBot->pev->flags |= FL_FAKECLIENT;

	float frametime = gpGlobals->time - g_LastBotUpdateTime[client];

	if (frametime > 0.25f || frametime < 0)
	{
		frametime = 0;
	}

	const byte msec = byte(frametime * 1000);

	// pBot->pev->view_ofs;
	Vector vecMove(0, 0, 0);
	byte impulse = 0;
	Vector vecViewAngles = pBot->pev->angles;
	pBot->pev->button = 0;

	// Create some random values
	if (pBot->GetTeamNumber() == TEAM_UNASSIGNED)
	{
		// unassigned team

		pBot->SetTeamNumber((RANDOM_LONG(0, 1) ? TEAM_RED : TEAM_BLUE));
	}
	else if (pBot->GetTeamNumber() != TEAM_UNASSIGNED && pBot->m_iClass == CLASS_UNDEFINED)
	{
		// on team but havent chosen a class
		pBot->m_iClass = CLASS_SOLDIER;
	}
	else if (pBot->IsAlive())
	{
		Bot_AliveThink(client, vecViewAngles, vecMove);
	}
	else
	{
		Bot_DeadThink(client, vecViewAngles, vecMove);
	}
	g_engfuncs.pfnRunPlayerMove(pBot->edict(), vecViewAngles, vecMove[0], vecMove[1], vecMove[2], pBot->pev->button, pBot->pev->impulse, msec);
}

//-----------------------------------------------------------------------------
// Purpose: Run through all the Bots in the game and let them think.
//-----------------------------------------------------------------------------
void Bot_RunAll(void)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		if (isTempBot(i))
		{
			Bot_Think(i);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle the Bot AI for a live bot
//-----------------------------------------------------------------------------
void Bot_AliveThink(int client, Vector vecAngles, Vector vecMove)
{
	auto pBot = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(client));
	// trace_t trace;

	// m_bWasDead = false;

	// In item testing mode, we run custom logic
	// from tf2sdk, not implemented YET
	//if (TFGameRules()->IsInItemTestingMode())
	//{
	//	Bot_ItemTestingThink(vecAngles, vecMove);
	//	return;
	//}

	if (g_LastBotUpdateTime[client] < gpGlobals->time)
	{
		g_LastBotUpdateTime[client] = gpGlobals->time + 0.025;
		enemy[client] = FindNearestEnemy(client, 4096);
	}

	if (!IsTargetVisible(client, enemy[client]))
		enemy[client] = NULL;

	if (enemy[client] != NULL)
	{
		// ALERT(at_console, "Botthink: updated and tried to aim at valid target!\n");
		AimAtTarget(client, enemy[client]);
		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pBot->m_pActiveItem;

		if (pWeapon)
		{
			int clipSize = pWeapon->m_iClip;
			if (clipSize != -1)
			{
				if (clipSize > 0)
				{
					ALERT(at_console, "Botthink: I have ammo / COULD have ammo, fire!\n");
					pBot->pev->button |= IN_ATTACK;
				}
				else
				{
					ALERT(at_console, "Botthink: I have ammo but need to reload!\n");
					pBot->pev->button |= IN_RELOAD;
				}
			}
			else
			{
				ALERT(at_console, "Botthink: No Ammo on this weapon!\n");
			}
		}
	}
	else
	{
		// ALERT(at_console, "Botthink: Couldn't get player!\n");
	}

	Bot_AliveMovementThink(client, vecAngles, vecMove);
	// Bot_AliveWeaponThink(pBot, vecAngles, vecMove);

	// Miscellaneous
	//if (bot_saveme.GetInt() > 0)
	//{
		//m_hBot->SaveMe();
		//bot_saveme.SetValue(bot_saveme.GetInt() - 1);
	//}
}

void Bot_AliveMovementThink(int client, Vector vecAngles, Vector vecMove)
{
	auto pBot = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(client));
	//if (bot_jump.GetBool() && m_hBot->GetFlags() & FL_ONGROUND)
	//{
	//	buttons |= IN_JUMP;
	//}

	//if (bot_crouch.GetBool())
	//{
	//	buttons |= IN_DUCK;
	//}
	// gotta port the commands over from tf2
}

//-----------------------------------------------------------------------------
// Purpose: Handle the Bot AI for a dead bot
//-----------------------------------------------------------------------------
void Bot_DeadThink(int client, Vector vecAngles, Vector vecMove)
{
	auto pBot = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(client));
	// Wait for Reinforcement wave
	if (!pBot->IsAlive())
	{
		// if (m_bWasDead)
		// {
			// Wait for a few seconds before respawning.
			// if (gpGlobals->curtime - m_flDeadTime > 3)
			// {
				// Respawn the bot
				// buttons |= IN_JUMP;
			// }
		// }
		// else
		// {
			// Start a timer to respawn them in a few seconds.
			// m_bWasDead = true;
			// m_flDeadTime = gpGlobals->curtime;
		// }
	}
}