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

LINK_ENTITY_TO_CLASS(weapon_rpg, CRpg);

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS(rpg_rocket, CRpgRocket);

CRpgRocket::~CRpgRocket()
{
	if (m_hLauncher)
	{
		// my launcher is still around, tell it I'm dead.
		static_cast<CRpg*>(static_cast<CBaseEntity*>(m_hLauncher))->m_cActiveRockets--;
	}
}

//=========================================================
//=========================================================
CRpgRocket* CRpgRocket::CreateRpgRocket(Vector vecOrigin, Vector vecAngles, CBaseEntity* pOwner, CRpg* pLauncher)
{
	CRpgRocket* pRocket = GetClassPtr((CRpgRocket*)NULL);

	UTIL_SetOrigin(pRocket->pev, vecOrigin);
	pRocket->pev->angles = vecAngles;
	// pRocket->pev->v_angle = vecAngles;
	pRocket->Spawn();
	pRocket->SetTouch(&CRpgRocket::RocketTouch);
	pRocket->m_hLauncher = pLauncher; // remember what RPG fired me.
	pLauncher->m_cActiveRockets++;	  // register this missile as active for the launcher
	pRocket->pev->owner = pOwner->edict();
	pRocket->pev->team = GetClassPtr((CBasePlayer*)(pOwner->pev))->m_iTeam;
	// Setup the initial velocity.
	Vector vecForward, vecRight, vecUp;
	AngleVectors(vecAngles, &vecForward, &vecRight, &vecUp);
	float flLaunchSpeed = 1100.0f;
	pRocket->pev->velocity = gpGlobals->v_forward * flLaunchSpeed;

	return pRocket;
}

//=========================================================
//=========================================================
void CRpgRocket::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_TOSS;
	pev->effects |= EF_LIGHT;
	pev->solid = SOLID_BBOX;
	
	SET_MODEL(ENT(pev), "models/rooster_fortress/w_rocket.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin(pev, pev->origin);

	pev->classname = MAKE_STRING("rpg_rocket");

	// make rocket sound
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5);

	// rocket trail
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex()); // entity
	WRITE_SHORT(m_iTrail);	 // model
	WRITE_BYTE(10);			 // life
	WRITE_BYTE(5);			 // width
	WRITE_BYTE(224);		 // r, g, b
	WRITE_BYTE(224);		 // r, g, b
	WRITE_BYTE(255);		 // r, g, b
	WRITE_BYTE(255);		 // brightness

	MESSAGE_END(); // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	SetThink(&CRpgRocket::FollowThink);
	SetTouch(&CRpgRocket::ExplodeTouch);

	pev->gravity = 0.000001; // stupid chud div by 0 errors

	pev->nextthink = gpGlobals->time + 0.1;

	pev->dmg = 110.0;
}

//=========================================================
//=========================================================
void CRpgRocket::RocketTouch(CBaseEntity* pOther)
{
	if (pOther == nullptr)
		return; // just in case

	CBasePlayer* edictPlayer = GetClassPtr((CBasePlayer*)(pOther->pev));
	
	if (edictPlayer == nullptr)
		return; // just in case

	if ( pev->team != edictPlayer->m_iTeam // only collides & explodes with enemies
		)
	{
		STOP_SOUND(edict(), CHAN_VOICE, "weapons/rocket1.wav");
		ExplodeTouch(pOther);
	}
	// this works, but it kills momentum n shi
}

//=========================================================
//=========================================================
void CRpgRocket::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/w_rocket.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND("weapons/rocket1.wav");
}


void CRpgRocket::IgniteThink()
{
	pev->movetype = MOVETYPE_TOSS;
	pev->effects |= EF_LIGHT;

	// make rocket sound
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5);

	// rocket trail
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex()); // entity
	WRITE_SHORT(m_iTrail);	 // model
	WRITE_BYTE(10);			 // life
	WRITE_BYTE(5);			 // width
	WRITE_BYTE(224);		 // r, g, b
	WRITE_BYTE(224);		 // r, g, b
	WRITE_BYTE(255);		 // r, g, b
	WRITE_BYTE(255);		 // brightness

	MESSAGE_END(); // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	SetThink(&CRpgRocket::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1;
}


CRpg* CRpgRocket::GetLauncher()
{
	if (!m_hLauncher)
		return NULL;

	return (CRpg*)((CBaseEntity*)m_hLauncher);
}

void CRpgRocket::FollowThink()
{
	// float flSpeed = pev->velocity.Length();
	if ((pev->effects & EF_LIGHT) != 0)
	{
		pev->effects = 0;
		STOP_SOUND(ENT(pev), CHAN_VOICE, "weapons/rocket1.wav");
	}

	if (GetLauncher())
	{
		GetLauncher()->m_cActiveRockets--;
		m_hLauncher = NULL;
	}

	if ((UTIL_PointContents(pev->origin) == CONTENTS_SKY))
	{
		Detonate();
	}

	pev->nextthink = gpGlobals->time + 0.1;
}
#endif



void CRpg::Reload()
{

	if (m_iClip >= 4)
	{
		// don't bother with any of this if don't need to reload.
		return;
	}

	if (m_pPlayer->ammo_rockets < 1)
		return;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (m_fInSpecialReload == 0)
	{
		SendWeaponAnim(ROCKETLAUNCHER_START_RELOAD);
		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.12;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.12;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.12);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.12;
		return;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		SendWeaponAnim(ROCKETLAUNCHER_RELOAD);
		
		m_fInSpecialReload = 2;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload1.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload3.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));

		// too long, shorten to match reload anim to make it smoother
		m_flNextPrimaryAttack = GetNextAttackDelay(0.80);
		m_flNextReload = UTIL_WeaponTimeBase() + 0.80;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;
	}
	else
	{
		// Add them to the clip
		m_iClip += 1;
		m_pPlayer->ammo_rockets -= 1;
		m_fInSpecialReload = 1;
		// m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
	}
}

void CRpg::Spawn()
{
	Precache();
	m_iId = WEAPON_RPG;

	SET_MODEL(ENT(pev), "models/rooster_fortress/wp_group_rf.mdl");
	pev->sequence = 2;
	pev->body = 6;

#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// more default ammo in multiplay.
		m_iDefaultAmmo = RPG_DEFAULT_GIVE + ROCKERLAUNCHER_MAX_CARRY;
	}
	else
	{
		m_iDefaultAmmo = RPG_DEFAULT_GIVE;
	}

	FallInit(); // get ready to fall down.
}


void CRpg::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/wp_group_rf.mdl");
	PRECACHE_MODEL("models/rooster_fortress/viewmodels/v_rocketlauncher.mdl");
	// PRECACHE_MODEL("models/p_rpg.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	UTIL_PrecacheOther("rpg_rocket");

	PRECACHE_SOUND("chicken_fortress_3/rocketlauncher_shoot.wav");

	m_usRpg = PRECACHE_EVENT(1, "events/rpg.sc");
}


bool CRpg::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "rockets";
	p->iMaxAmmo1 = ROCKET_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = RPG_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_RPG;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHTO;
	p->iWeight = RPG_WEIGHT;

	return true;
}

bool CRpg::Deploy()
{
	//if (m_iClip == 0)
	//{
		//return DefaultDeploy("models/rooster_fortress/viewmodels/v_rocketlauncher.mdl", "models/rooster_fortress/wp_group_rf.mdl", ROCKETLAUNCHER_DRAW, "rpg");
	//}

	return DefaultDeploy("models/rooster_fortress/viewmodels/v_rocketlauncher.mdl", "models/rooster_fortress/wp_group_rf.mdl", ROCKETLAUNCHER_DRAW, "rpg", 6);
}


bool CRpg::CanHolster()
{
	return true;
}

void CRpg::Holster()
{
	m_fInReload = false; // cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(ROCKETLAUNCHER_IDLE);
}

void CRpg::PrimaryAttack()
{
	if (m_iClip >= 1)
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -2;

		CRpgRocket* pRocket = CRpgRocket::CreateRpgRocket(vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle); // RpgRocket::Create stomps on globals, so remake.
		pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct(m_pPlayer->pev->velocity, gpGlobals->v_forward);
#endif

		// firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

		int flags;
#if defined(CLIENT_WEAPONS)
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usRpg);

		m_iClip--;

		m_flNextPrimaryAttack = GetNextAttackDelay(0.8);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8;

		m_fInSpecialReload = 0;

		ResetEmptySound();
	}
	else
	{
		PlayEmptySound();
	}
}


void CRpg::SecondaryAttack()
{
}


void CRpg::WeaponIdle()
{

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->ammo_rockets > 0)
	{
		Reload();
	}
	else if (m_fInSpecialReload != 0)
	{
		if (m_iClip < 4 && m_pPlayer->ammo_rockets > 0)
		{
			Reload();
		}
		else
		{
			// done reloading
			SendWeaponAnim(ROCKETLAUNCHER_AFTER_RELOAD);

			m_fInSpecialReload = 0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8;
			m_flNextPrimaryAttack = GetNextAttackDelay(0.8);
		}
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (20.0 / 9.0);
		SendWeaponAnim(ROCKETLAUNCHER_IDLE);
	}
}

class CRpgAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_rpgammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_rpgammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		int iGive;

#ifdef CLIENT_DLL
		if (bIsMultiplayer())
#else
		if (g_pGameRules->IsMultiplayer())
#endif
		{
			// hand out more ammo per rocket in multiplayer.
			iGive = AMMO_RPGCLIP_GIVE * 4;
		}
		else
		{
			iGive = AMMO_RPGCLIP_GIVE;
		}

		if (pOther->GiveAmmo(iGive, "rockets", ROCKET_MAX_CARRY) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_rpgclip, CRpgAmmo);
