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

LINK_ENTITY_TO_CLASS(weapon_wrench, CWrench);

void CWrench::Spawn()
{
	pev->classname = MAKE_STRING("weapon_wrench");
	Precache();
	m_iId = WEAPON_WRENCH;
	SET_MODEL(ENT(pev), "models/rooster_fortress/wp_group_rf.mdl");
	pev->sequence = 12;
	pev->body = 36;

	m_iClip = -1;

	FallInit(); // get ready to fall down.
}

void CWrench::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/viewmodels/v_wrench.mdl");
	PRECACHE_MODEL("models/rooster_fortress/wp_group_rf.mdl");

	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	m_usWrench = PRECACHE_EVENT(1, "events/roosterfortress3.sc");
}

bool CWrench::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iId = WEAPON_WRENCH;
	p->iWeight = 10;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY;
	return true;
}


bool CWrench::Deploy()
{
	return GroupDeploy("models/rooster_fortress/viewmodels/v_wrench.mdl", "models/rooster_fortress/wp_group_rf.mdl", WRENCH_DRAW, 36, 0, "crowbar", 0);
}

void CWrench::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
	CBasePlayerWeapon::Holster();
}

void CWrench::PrimaryAttack()
{
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usWrench,
		0.0, g_vecZero, g_vecZero, 0, 0, 0,
		0.0, 0, 0.0);

	switch (((m_iSwing++) % 2) + 1)
	{
	case 0:
		SendWeaponAnim(WRENCH_SWING_A);
		break;
	case 1:
		SendWeaponAnim(WRENCH_SWING_B);
		break;
	case 2:
		SendWeaponAnim(WRENCH_SWING_C);
		break;
	}

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	SetThink(&CShovel::Swing);
	pev->nextthink = gpGlobals->time + 0.2;

	m_flNextPrimaryAttack = GetNextAttackDelay(0.8);

	return;
}