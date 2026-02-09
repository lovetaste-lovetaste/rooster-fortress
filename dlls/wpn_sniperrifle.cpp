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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "monsters.h"
#include "player.h"
#include "gamerules.h"
#include "UserMessages.h"

#define TF_WEAPON_SNIPERRIFLE_CHARGE_PER_SEC 50.0
#define TF_WEAPON_SNIPERRIFLE_UNCHARGE_PER_SEC 75.0
#define TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN 50
#define TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX 150
#define TF_WEAPON_SNIPERRIFLE_RELOAD_TIME 1.5f
#define TF_WEAPON_SNIPERRIFLE_ZOOM_TIME 0.3f

#define TF_WEAPON_SNIPERRIFLE_NO_CRIT_AFTER_ZOOM_TIME 0.2f

LINK_ENTITY_TO_CLASS(weapon_sniperrifle, CSniperRifle);

bool CSniperRifle::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "357";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SNIPERRIFLE_MAX_CARRY;
	p->iFlags = 0;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_SNIPERRIFLE;
	p->iWeight = PYTHON_WEIGHT;
	return true;
}

void CSniperRifle::Spawn()
{
	pev->classname = MAKE_STRING("weapon_sniperrifle"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_SNIPERRIFLE;
	SET_MODEL(ENT(pev), "models/rooster_fortress/wp_group_rf.mdl");
	pev->sequence = 20;
	pev->body = 57;

	m_iDefaultAmmo = SNIPERRIFLE_MAX_CARRY;

	FallInit(); // get ready to fall down.
}


void CSniperRifle::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/viewmodels/v_sniperrifle.mdl");
	PRECACHE_MODEL("models/rooster_fortress/wp_group_rf.mdl");

	PRECACHE_MODEL("models/w_357ammobox.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/357_reload1.wav");
	PRECACHE_SOUND("weapons/357_cock1.wav");
	PRECACHE_SOUND("weapons/357_shot1.wav");
	PRECACHE_SOUND("weapons/357_shot2.wav");

	m_usSniperRifle = PRECACHE_EVENT(1, "events/sniperrifle.sc");
}

bool CSniperRifle::Deploy()
{
	bool success = DefaultDeploy("models/rooster_fortress/viewmodels/v_sniperrifle.mdl", "models/rooster_fortress/wp_group_rf.mdl", SNIPERRIFLE_DRAW, "python", 0);
	m_flTimeWeaponIdle = (31 / 30.0);
	return success;
}

void CSniperRifle::Holster()
{
#ifndef CLIENT_DLL
	if (m_pPlayer->IsInCond(TF_COND_AIMING))
	{
		m_pPlayer->RemoveCondition(TF_COND_AIMING);
	}
#endif
	CBasePlayerWeapon::Holster();
}

void CSniperRifle::HandleZooms()
{
#ifndef CLIENT_DLL
	if (!m_pPlayer->IsInCond(TF_COND_AIMING))
	{
		m_pPlayer->AddCondition(TF_COND_AIMING, PERMANENT_CONDITION, NULL);
	}
	else if (m_pPlayer->IsInCond(TF_COND_AIMING))
	{
		m_pPlayer->RemoveCondition(TF_COND_AIMING);
	}
#endif
}

void CSniperRifle::SecondaryAttack()
{
	HandleZooms();
	m_flNextSecondaryAttack = 0.5;
}

void CSniperRifle::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_TF2_HEADSHOT, 0, 50, m_pPlayer->pev, m_pPlayer->random_seed);
	
	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usSniperRifle, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

#ifndef CLIENT_DLL
	if (m_pPlayer->IsInCond(TF_COND_AIMING))
	{
		// when shoot remove fov
		m_pPlayer->RemoveCondition(TF_COND_AIMING);
	}
#endif
	m_flNextPrimaryAttack = m_flTimeWeaponIdle = (51 / 30.0);
}


void CSniperRifle::Reload()
{
	// NO FREAKING RELOADS BRAH!!!!!!!!!!!
	return;
}


void CSniperRifle::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_2DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;
	
	SendWeaponAnim(SNIPERRIFLE_IDLE, 0);

	m_flTimeWeaponIdle = (9 / 30.0);
}

void CSniperRifle::ItemPostFrame()
{
#ifndef CLIENT_DLL
	if (m_pPlayer->pev->button & IN_JUMP && (pev->flags & FL_ONGROUND))
	{
		// this checks if ur on the ground and trying to jump
		// this SPECIFICALLY checks if ur on the ground due to dropshots being a thing in normal tf2
		m_pPlayer->RemoveCondition(TF_COND_AIMING);
	}

	if (m_pPlayer->IsInCond(TF_COND_AIMING))
	{
		if (m_pPlayer->m_iFOV != 20)
			m_pPlayer->m_iFOV = 20;
	}
	else if (m_pPlayer->m_iFOV != 0)
	{
		m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
	}

#endif
	CBasePlayerWeapon::ItemPostFrame();
}


class CSniperRifleAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_357ammobox.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_357ammobox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_357BOX_GIVE, "357", _357_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_sniperrifle, CSniperRifleAmmo);
