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

===== client.cpp ========================================================

  client/server game specific stuff

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

// CODE GRABBED FROM THE TF2SDK

inline bool isTempBot(CBasePlayer* pPlayer)
{
	return (pPlayer && (pPlayer->pev->flags & FL_FAKECLIENT) != 0);
}

//-----------------------------------------------------------------------------
// Purpose: Run this Bot's AI for one frame.
//-----------------------------------------------------------------------------
void Bot_Think(CBasePlayer* pBot)
{
	// Make sure we stay being a bot
	if ((pBot->pev->flags & FL_FAKECLIENT) != 0)
		pBot->pev->flags |= FL_FAKECLIENT;

	pBot->pev->view_ofs;
	Vector vecMove(0, 0, 0);
	byte impulse = 0;
	float frametime = gpGlobals->frametime;
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

	}
	else if (pBot->IsAlive())
	{
		botdata->Bot_AliveThink(&vecViewAngles, &vecMove);
	}
	else
	{
		botdata->Bot_DeadThink(&vecViewAngles, &vecMove);
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
		auto pPlayer = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));

		if (isTempBot(pPlayer))
		{
			Bot_Think(pPlayer);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle the Bot AI for a live bot
//-----------------------------------------------------------------------------
void Bot_AliveThink(CBasePlayer* pBot, Vector vecAngles, Vector vecMove)
{
	trace_t trace;

	// m_bWasDead = false;

	// In item testing mode, we run custom logic
	// from tf2sdk, not implemented YET
	//if (TFGameRules()->IsInItemTestingMode())
	//{
	//	Bot_ItemTestingThink(vecAngles, vecMove);
	//	return;
	//}

	// NOT DONE PORTING

	Bot_AliveMovementThink(pBot, vecAngles, vecMove);
	Bot_AliveWeaponThink(pBot, vecAngles, vecMove);

	// Miscellaneous
	//if (bot_saveme.GetInt() > 0)
	//{
		//m_hBot->SaveMe();
		//bot_saveme.SetValue(bot_saveme.GetInt() - 1);
	//}
}