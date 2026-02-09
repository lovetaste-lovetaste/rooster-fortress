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

LINK_ENTITY_TO_CLASS(weapon_minigun, CMinigun);

void CMinigun::Spawn()
{
	pev->classname = MAKE_STRING("weapon_minigun"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_MINIGUN;
	SET_MODEL(ENT(pev), "models/rooster_fortress/wp_group_2bone.mdl");
	pev->sequence = 1;
	pev->body = 3;
	m_iWeaponState = MINIGUN_STATE_IDLE;
	m_iDefaultAmmo = FLAMETHROWER_MAX_CARRY;

	FallInit(); // get ready to fall down.
}
// 706-653-2221
void CMinigun::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/viewmodels/v_minigun.mdl");

	PRECACHE_MODEL("models/rooster_fortress/wp_group_2bone.mdl");

	m_iShell = PRECACHE_MODEL("models/shell.mdl"); // brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND("rooster_fortress/weapons/minigun_empty.wav");
	PRECACHE_SOUND("rooster_fortress/weapons/minigun_wind_up.wav");
	PRECACHE_SOUND("rooster_fortress/weapons/minigun_wind_down.wav");
	PRECACHE_SOUND("rooster_fortress/weapons/minigun_spin.wav");
	PRECACHE_SOUND("rooster_fortress/weapons/minigun_shoot.wav");
	PRECACHE_SOUND("rooster_fortress/weapons/minigun_shoot_crit.wav");

	m_usFireMinigun = PRECACHE_EVENT(1, "events/minigun.sc");
}

bool CMinigun::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = FLAMETHROWER_MAX_CARRY;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MINIGUN;
	p->iWeight = EGON_WEIGHT;

	return true;
}

bool CMinigun::Deploy()
{
	bool success = true;
	IdleNonRevved(); // resets juuuust in case
	success = DefaultDeploy("models/rooster_fortress/viewmodels/v_minigun.mdl", "models/rooster_fortress/wp_group_2bone.mdl", MINIGUN_DRAW, "shotgun", 15);
	m_flTimeWeaponIdle = gpGlobals->time + (31 / 30.0);
	return success;
}

void CMinigun::SecondaryAttack()
{
	// everything is handled in the idle function!!!
	// this just makes sure that nothing happens

	WeaponIdle();
}

void CMinigun::PrimaryAttack()
{
	// everything is handled in the idle function!!!
	// this just makes sure that nothing happens

	WeaponIdle();
}

void CMinigun::Holster()
{
	IdleNonRevved();
}

bool CMinigun::CanHolster()
{
	if (m_iWeaponState != MINIGUN_STATE_IDLE)
		return false;
	return true;
}

void CMinigun::MinigunFire(float flSpread, float flCycleTime)
{
	if (m_iClip <= 0)
	{
		PlayEmptySound();
		IdleRev();
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireMinigun, 0.0, m_pPlayer->GetGunPosition(), g_vecZero, 0.0, 0.0, MINIGUN_STATE_DRYFIRE, 0, 0, 0);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = GetNextAttackDelay(flCycleTime);
		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming;

	vecAiming = gpGlobals->v_forward;

	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer(4, vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192, BULLET_PLAYER_TF2, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed);

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireMinigun, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, (m_iClip == 0) ? 1 : 0, 1);
	// actual firing

	PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireMinigun, 0.0, m_pPlayer->GetGunPosition(), g_vecZero, 0.0, 0.0, MINIGUN_STATE_FIRING, 0, 0, 0);
	// sound
	
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = GetNextAttackDelay(flCycleTime);

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	m_flStartedWindDownAt = gpGlobals->time + 1.3;
}


void CMinigun::Reload()
{
	// NO FREAKING RELOADS!!!
	return;
}

void CMinigun::DebugStater()
{
	int iButton = m_pPlayer->pev->button;

	if (iButton & IN_ATTACK)
	{
		ALERT(at_console, "Firing!!\n");
	}
	else if (iButton & IN_ATTACK2)
	{
		ALERT(at_console, "Revving!!\n");
	}

	switch (m_iWeaponState)
	{
	case MINIGUN_STATE_STOPSPINNING:
	{
		ALERT(at_console, "Minigun is revving DOWN\n");
		break;
	}
	case MINIGUN_STATE_STARTSPINNING:
	{
		ALERT(at_console, "Minigun is revving UP\n");
		break;
	}
	case MINIGUN_STATE_IDLE:
	{
		ALERT(at_console, "Minigun is idle\n");
		break;
	}
	case MINIGUN_STATE_SPINNING:
	{
		ALERT(at_console, "Minigun is revved and idle\n");
		break;
	}
	}
}

void CMinigun::WeaponIdle()
{
	ResetEmptySound();

	// m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	if (m_flNextPrimaryAttack > gpGlobals->time)
		return;

	if (m_flNextSecondaryAttack > gpGlobals->time)
		return;
	// UTIL_WeaponTimeBase()
	// the 2 attack things may or may not be needed
	// needs more testing

	int iButton = m_pPlayer->pev->button;
	// int iOldButton = m_pPlayer->pev->oldbuttons;
	// iButton |= iOldButton;
	// DebugStater();

	// since this function is called, this makes sure that the animations + delay for the revs finish and go to their proper states
	// finished the revving up -> switch to idle revved state
	// finished the revving down -> switch to idle non-revved state
	// everything else is handled below

	if (m_iWeaponState == MINIGUN_STATE_STOPSPINNING)
	{
		// special case for unrevving bcuz for some reason unrevving acts stupid as fuuuuck
		if (m_flStartedWindDownAt >= gpGlobals->time)
		{
			// ALERT(at_console, "NOT DONE UNREVVING\n");
			return;
		}
		else
		{
			// ALERT(at_console, "Finishing un-revving into idle state\n");
			IdleNonRevved();
			return;
		}
	}

	if (m_iWeaponState == MINIGUN_STATE_STARTSPINNING)
	{
		// ALERT(at_console, "Finishing revving into spinning state\n");
		IdleRev();
		return;
	}

	if (iButton & IN_ATTACK) // firing
	{
		// ALERT(at_console, "Attack button function\n");
		if (m_iWeaponState == MINIGUN_STATE_IDLE) // not revved at all
		{
			StartRevving(0.75);
			return;
		}
		else if (m_iWeaponState == MINIGUN_STATE_SPINNING)
		{
			// dont need to worry abt going directly from idle to firing bcuz the rev goes directly to spinning
			MinigunFire(0.06, 0.1);
			return;
		}
	}
	else if (iButton & IN_ATTACK2) // manually revving
	{
		// ALERT(at_console, "Attack2 button function\n");
		//  this specifically gets overridden by the attack button due to priority, to prevent weird stuff from happening

		if (m_iWeaponState == MINIGUN_STATE_IDLE) // not revved at all
		{
			StartRevving( 0.75 );
			return;
		}
		else if (m_iWeaponState == MINIGUN_STATE_SPINNING)
		{
			IdleRev(); // any other state other than MINIGUN_STATE_IDLE is a revved up state, so dont worry abt specifying other shit
			return;
		}
	}
	else
	{
		// this handles unrevving

		if (m_iWeaponState == MINIGUN_STATE_IDLE) // not revved at all
		{
			// ALERT(at_console, "No button function IdleNonRevved\n");
			IdleNonRevved();
			return;
		}
		else
		{
			if (m_iWeaponState != MINIGUN_STATE_STOPSPINNING)
			{
				// ALERT(at_console, "No button function StopRevving\n");
				StopRevving();
				return;
			}
			else
			{
				if (m_flStartedWindDownAt >= gpGlobals->time)
				{
					// ALERT(at_console, "WPIDLE NOT DONE UNREVVING\n");
					return;
				}
				else
				{
					// ALERT(at_console, "WPIDLE Finishing un-revving into idle state\n");
					IdleNonRevved();
					return;
				}
			}
		}
	}
}

void CMinigun::ItemPostFrame()
{
#ifndef CLIENT_DLL

	if (!(m_iWeaponState == MINIGUN_STATE_IDLE || m_iWeaponState == MINIGUN_STATE_STOPSPINNING))
	{
		// every other weapon state is a revved one, so it adds the condition
		if (!m_pPlayer->IsInCond(TF_COND_AIMING))
		{
			m_pPlayer->AddCondition(TF_COND_AIMING, PERMANENT_CONDITION, NULL);
		}
	}
	else
	{
		if (m_pPlayer->IsInCond(TF_COND_AIMING))
		{
			m_pPlayer->RemoveCondition(TF_COND_AIMING);
		}
	}

	// ALERT(at_console, "is in cond? %i \n", ((m_pPlayer->IsInCond(TF_COND_AIMING)) ? 1 : 0));

#endif

	CBasePlayerWeapon::ItemPostFrame();
}

void CMinigun::StartRevving(float revtime=0.75)
{
	m_iWeaponState = MINIGUN_STATE_STARTSPINNING;
	SendWeaponAnim(MINIGUN_REV);
	PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireMinigun, 0.0, m_pPlayer->GetGunPosition(), g_vecZero, 0.0, 0.0, MINIGUN_STATE_STARTSPINNING, 0, 0, 0);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = GetNextAttackDelay(revtime);
	m_flStartedWindDownAt = gpGlobals->time + 1.3;
}

void CMinigun::StopRevving()
{
	m_iWeaponState = MINIGUN_STATE_STOPSPINNING;
	SendWeaponAnim(MINIGUN_UNREV);
	PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireMinigun, 0.0, m_pPlayer->GetGunPosition(), g_vecZero, 0.0, 0.0, MINIGUN_STATE_STOPSPINNING, 0, 0, 0);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = gpGlobals->time + 1.3;
	m_flStartedWindDownAt = gpGlobals->time + 1.3;
}

void CMinigun::IdleRev()
{
	m_iWeaponState = MINIGUN_STATE_SPINNING;
	SendWeaponAnim(MINIGUN_IDLE_REVVED);
	PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireMinigun, 0.0, m_pPlayer->GetGunPosition(), g_vecZero, 0.0, 0.0, MINIGUN_STATE_SPINNING, 0, 0, 0);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = GetNextAttackDelay(0.1);
	m_flStartedWindDownAt = gpGlobals->time + 1.3;
}

void CMinigun::IdleNonRevved()
{
	m_iWeaponState = MINIGUN_STATE_IDLE;
	SendWeaponAnim(MINIGUN_IDLE);
	PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFireMinigun, 0.0, m_pPlayer->GetGunPosition(), g_vecZero, 0.0, 0.0, MINIGUN_STATE_IDLE, 0, 0, 0);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = GetNextAttackDelay(0.1);
	m_flStartedWindDownAt = gpGlobals->time + 1.3;
}

class CMinigunAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_9mmclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_GLOCKCLIP_GIVE, "9mm", FLAMETHROWER_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(ammo_minigunclip, CMinigunAmmo);