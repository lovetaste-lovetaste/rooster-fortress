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


LINK_ENTITY_TO_CLASS(weapon_shovel, CShovel);

void CShovel::Spawn()
{
	pev->classname = MAKE_STRING("weapon_shovel");
	Precache();
	m_iId = WEAPON_SHOVEL;
	SET_MODEL(ENT(pev), "models/rooster_fortress/wp_group_rf.mdl");
	pev->sequence = 0;
	pev->body = 0;

	m_iClip = -1;

	FallInit(); // get ready to fall down.
}


void CShovel::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/viewmodels/v_shovel_soldier.mdl");
	PRECACHE_MODEL("models/rooster_fortress/wp_group_rf.mdl");
	// PRECACHE_MODEL("models/rooster_fortress/w_shovel.mdl");
	
	// PRECACHE_MODEL("models/p_crowbar.mdl");

	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	m_usShovel = PRECACHE_EVENT(1, "events/sandwich.sc");
}

bool CShovel::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_SHOVEL;
	p->iWeight = 10;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY;
	return true;
}


bool CShovel::Deploy()
{
	return GroupDeploy("models/rooster_fortress/viewmodels/v_shovel_soldier.mdl", "models/rooster_fortress/wp_group_rf.mdl", SHOVEL_DRAW, 0, 0, "crowbar", 0);
	// return DefaultDeploy("models/chicken_fortress_3/v_shovel.mdl", "models/rooster_fortress/w_shovel.mdl", SHOVEL_DRAW, "shovel");
}

void CShovel::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
	CBasePlayerWeapon::Holster();
	// 
	//SendWeaponAnim(SHOVEL_DRAW);
}

void CShovel::PrimaryAttack()
{
	Swing(true);
}


void CShovel::Smack()
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}


void CShovel::SwingAgain()
{
	Swing(false);
}


bool CShovel::Swing(bool fFirst)
{
	if (fFirst)
	{
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usShovel,
			0.0, g_vecZero, g_vecZero, 0, 0, 0,
			0.0, 0, 0.0);

		switch (((m_iSwing++) % 2) + 1)
		{
		case 0:
			SendWeaponAnim(SHOVEL_SWING_A);
			break;
		case 1:
			SendWeaponAnim(SHOVEL_SWING_B);
			break;
		case 2:
			SendWeaponAnim(SHOVEL_SWING_C);
			break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		SetThink(&CShovel::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.2;

		m_flNextPrimaryAttack = GetNextAttackDelay(0.8);

		return false;
	}


	bool fDidHit = false;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos; // This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	// m_flNextPrimaryAttack = GetNextAttackDelay(0.65);

#ifndef CLIENT_DLL

		if (tr.flFraction < 1.0)
		{
			// hit
			fDidHit = true;
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			ClearMultiDamage();

			pEntity->TraceAttack(m_pPlayer->pev, 65.0, gpGlobals->v_forward, &tr, DMG_CLUB);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			// play thwack, smack, or dong sound
			float flVol = 1.0;
			bool fHitWorld = true;

			if (pEntity)
			{
				if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
				{
					// play thwack or smack sound
					switch (RANDOM_LONG(0, 2))
					{
					case 0:
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM);
						break;
					case 1:
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM);
						break;
					case 2:
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM);
						break;
					}
					m_pPlayer->m_iWeaponVolume = SHOVEL_BODYHIT_VOLUME;
					if (!pEntity->IsAlive())
						return true;
					else
						flVol = 0.1;

					fHitWorld = false;
				}
			}

			// play texture hit sound
			// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

			if (fHitWorld)
			{
				float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

				if (g_pGameRules->IsMultiplayer())
				{
					// override the volume here, cause we don't play texture sounds in multiplayer,
					// and fvolbar is going to be 0 from the above call.

					fvolbar = 1;
				}

				// also play crowbar strike
				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				case 1:
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				}

				// delay the decal a bit
				m_trHit = tr;
				DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
			}

			m_pPlayer->m_iWeaponVolume = flVol * SHOVEL_WALLHIT_VOLUME;
		}
#endif
	return fDidHit;
}
