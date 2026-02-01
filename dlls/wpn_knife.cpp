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

LINK_ENTITY_TO_CLASS(weapon_knife, CKnife);

void CKnife::Spawn()
{
	pev->classname = MAKE_STRING("weapon_knife");
	Precache();
	m_iId = WEAPON_KNIFE;
	SET_MODEL(ENT(pev), "models/rooster_fortress/wp_group_rf.mdl");
	pev->sequence = 14;
	pev->body = 42;
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}


void CKnife::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/viewmodels/v_knife_spy.mdl");
	PRECACHE_MODEL("models/rooster_fortress/wp_group_rf.mdl");
	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");
	
	m_usKnife = PRECACHE_EVENT(1, "events/knife.sc");
}

bool CKnife::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_KNIFE;
	p->iWeight = 10;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY;
	return true;
}


bool CKnife::Deploy()
{
	return GroupDeploy("models/rooster_fortress/viewmodels/v_knife_spy.mdl", "models/rooster_fortress/wp_group_rf.mdl", KNIFE_DRAW, 42, 0, "crowbar", 0);
}

void CKnife::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
	CBasePlayerWeapon::Holster();
}

void CKnife::PrimaryAttack()
{
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usKnife,
		0.0, g_vecZero, g_vecZero, 0, 0, 0,
		0.0, 0, 0.0);

	switch (((m_iSwing++) % 2) + 1)
	{
	case 0:
		SendWeaponAnim(KNIFE_SWING_A);
		break;
	case 1:
		SendWeaponAnim(KNIFE_SWING_B);
		break;
	case 2:
		SendWeaponAnim(KNIFE_SWING_C);
		break;
	}

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.8);

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

#ifndef CLIENT_DLL

	if (tr.flFraction < 1.0)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();

		float damage = 40.0;

		int damagebits = DMG_CLUB; //

		if (IsBackFace(m_pPlayer->pev->v_angle, pEntity->pev->v_angle))
		{
			damage = pEntity->pev->health * 6.0; // in live tf2 backstabs do 2x + the crit multiplier. this skips all that and just does 6x
			damagebits |= DMG_CRIT;
			// ALERT(at_console, "Backstab!!!");
		}

		pEntity->TraceAttack(m_pPlayer->pev, (int)damage, gpGlobals->v_forward, &tr, damagebits);
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
					return;
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
}


void CKnife::Smack()
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}

bool CKnife::IsBackFace(Vector& anglesAttacker, Vector& anglesVictim)
{
	float flAngles = anglesAttacker.y - anglesVictim.y;
	if (flAngles < -180.0)
		flAngles += 360.0;
	if (flAngles <= 90.0 && flAngles >= -90.0)
		return false;
	return true;
}
// REVOLVER_MAX_CARRY