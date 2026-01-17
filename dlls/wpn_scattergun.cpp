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
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "UserMessages.h"

// special deathmatch shotgun spreads
#define VECTOR_CONE_DM_SHOTGUN Vector(0.08716, 0.04362, 0.00)		// 10 degrees by 5 degrees
#define VECTOR_CONE_DM_DOUBLESHOTGUN Vector(0.17365, 0.04362, 0.00) // 20 degrees by 5 degrees

LINK_ENTITY_TO_CLASS(weapon_scattergun, CScattergun);

void CScattergun::Spawn()
{
	Precache();
	m_iId = WEAPON_SCATTERGUN;
	m_bIsPrimary = true;
	SET_MODEL(ENT(pev), "models/rooster_fortress/wp_group_rf.mdl");
	pev->sequence = 1;
	// pev->body = iBody(); // this is weird for viewmodels
	m_iDefaultAmmo = SCATTERGUN_DEFAULT_GIVE;

	FallInit(); // get ready to fall
}


void CScattergun::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/viewmodels/v_scattergun.mdl");
	PRECACHE_MODEL("models/rooster_fortress/wp_group_rf.mdl");

	m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl"); // shotgun shell

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("chicken_fortress_3/scattergun_reload.wav"); //shotgun
	PRECACHE_SOUND("chicken_fortress_3/scattergun_shoot.wav"); //shotgun

	PRECACHE_SOUND("weapons/reload1.wav"); // shotgun reload
	PRECACHE_SOUND("weapons/reload3.wav"); // shotgun reload

	//	PRECACHE_SOUND ("weapons/sshell1.wav");	// shotgun reload - played on client
	//	PRECACHE_SOUND ("weapons/sshell3.wav");	// shotgun reload - played on client
	
	PRECACHE_SOUND("weapons/357_cock1.wav"); // gun empty sound
	PRECACHE_SOUND("weapons/scock1.wav");	 // cock gun

	m_usScattergun = PRECACHE_EVENT(1, "events/roosterfortress2.sc");
	//m_usDoubleFire = PRECACHE_EVENT(1, "events/shotgun2.sc");
}

bool CScattergun::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = BUCKSHOT_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SCATTERGUN_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_SCATTERGUN;
	p->iWeight = SCATTERGUN_WEIGHT;

	return true;
}

bool CScattergun::Deploy()
{
	//
	//return GroupDeploy("models/rooster_fortress/viewmodels/v_scattergun.mdl", "models/rooster_fortress/wp_group_rf.mdl", SHOTGUN_DRAW, 0, 0, "shotgun", 3);
	return DefaultDeploy("models/rooster_fortress/viewmodels/v_scattergun.mdl", "models/rooster_fortress/wp_group_rf.mdl", SHOTGUN_DRAW, "shotgun", 0);
}

void CScattergun::PrimaryAttack()
{
	if (m_iClip <= 0)
	{
		Reload();
		if (m_iClip == 0)
			PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(10, vecSrc, vecAiming, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0, 6, m_pPlayer->pev, m_pPlayer->random_seed);

	// CKFFireBullets(vecSrc, gpGlobals->v_forward, 0.0, 8192, BULLET_PLAYER_TF2, 6, iCrit, m_pPlayer->pev, m_pPlayer->random_seed, FALSE);

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usScattergun, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	if (m_iClip != 0)
		m_flPumpTime = gpGlobals->time + 0.625;

	m_flNextPrimaryAttack = GetNextAttackDelay(0.625);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.625;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.625;
	m_fInSpecialReload = 0;
}


void CScattergun::SecondaryAttack()
{
}


void CScattergun::Reload()
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == SHOTGUN_MAX_CLIP)
		return;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	// check to see if we're ready to reload
	if (m_fInSpecialReload == 0)
	{
		SendWeaponAnim(SCATTERGUN_RELOAD_START);
		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.6;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.6);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6;
		return;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
		// was waiting for gun to move to side
		m_fInSpecialReload = 2;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload1.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload3.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));

		SendWeaponAnim(SCATTERGUN_RELOAD_LOOP);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
	else
	{
		// Add them to the clip
		m_iClip += 1;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;
		m_fInSpecialReload = 1;
	}
}


void CScattergun::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	//Moved to ItemPostFrame
	/*
	if ( m_flPumpTime && m_flPumpTime < gpGlobals->time )
	{
		// play pumping sound
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0, 95 + RANDOM_LONG(0,0x1f));
		m_flPumpTime = 0;
	}
	*/

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 && m_fInSpecialReload == 0 && 0 != m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_fInSpecialReload != 0)
		{
			if (m_iClip < SCATTERGUN_MAX_CLIP && 0 != m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim(SCATTERGUN_RELOAD_END);

				// play cocking sound
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 0x1f));
				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
		}
		else
		{
			int iAnim;
			iAnim = SCATTERGUN_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (20.0 / 9.0);
			SendWeaponAnim(iAnim);
		}
	}
}

void CScattergun::Holster()
{
	// Delay the next player's attack for about the same time as the holster animation takes
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	CBasePlayerWeapon::Holster();
}

//void CScattergun::ItemPostFrame()
//{
	//if (0 != m_flPumpTime && m_flPumpTime < gpGlobals->time)
	//{
		// play pumping sound
		//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 0x1f));
		//m_flPumpTime = 0;
	//}

	//CBasePlayerWeapon::ItemPostFrame();
//}


class CScattergunAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_shotbox.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_shotbox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_BUCKSHOTBOX_GIVE, "buckshot", BUCKSHOT_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_buckshot_scattergun, CScattergunAmmo);
