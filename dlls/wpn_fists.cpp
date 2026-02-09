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
***/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

LINK_ENTITY_TO_CLASS(weapon_fists, CFists);

void CFists::Spawn()
{
	pev->classname = MAKE_STRING("weapon_fists");
	Precache();
	m_iId = WEAPON_FISTS;
	SET_MODEL(ENT(pev), "models/rooster_fortress/null.mdl");
	pev->sequence = 0;
	pev->body = 0;
	m_iClip = -1;
	FallInit();
}

void CFists::Precache()
{
	// make sure to call this when making new melee!
	// like CBasePlayerMelee::Precache() after everything else is precached
	// this is important!!!

	PRECACHE_MODEL("models/rooster_fortress/null.mdl");
	PRECACHE_MODEL("models/rooster_fortress/viewmodels/v_fists.mdl");
	CBasePlayerMelee::Precache();
}

bool CFists::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->iId = WEAPON_FISTS;
	return CBasePlayerMelee::GetItemInfo(p);
}

bool CFists::Deploy()
{
	return DefaultDeploy("models/rooster_fortress/viewmodels/v_fists.mdl", "models/rooster_fortress/null.mdl", MELEE_DRAW, "crowbar", 0);
}

void CFists::PrimaryAttack()
{
	CBasePlayerMelee::PrimaryAttack();

	switch ((m_iSwing++) % 2)
	{
		default:
		case 0:
			SendWeaponAnim(MELEE_SWING_A);
			break;
		case 1:
			SendWeaponAnim(MELEE_SWING_B);
			break;
		case 2:
			SendWeaponAnim(MELEE_SWING_C);
			break;
	}
}

// everything else is handled in CBasePlayerMelee
// go check there if u want to change anything