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

#define max(a, b) (((a) > (b)) ? (a) : (b))

LINK_ENTITY_TO_CLASS(weapon_grenadelauncher, CGrenadelauncher);

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS(tf2_grenade, CTF2Grenade);

CTF2Grenade::~CTF2Grenade()
{
	if (m_hLauncher)
	{
		// my launcher is still around, tell it I'm dead.
		static_cast<CGrenadelauncher*>(static_cast<CBaseEntity*>(m_hLauncher))->m_cActiveRockets--;
	}
}

//=========================================================
//=========================================================
CTF2Grenade* CTF2Grenade::CreateTF2Grenade(Vector vecOrigin, Vector vecAngles, CBaseEntity* pOwner, CGrenadelauncher* pLauncher)
{
	CTF2Grenade* pRocket = GetClassPtr((CTF2Grenade*)NULL);

	UTIL_SetOrigin(pRocket->pev, vecOrigin);
	pRocket->pev->angles = vecAngles;
	pRocket->pev->v_angle = vecAngles;
	pRocket->Spawn();
	pRocket->SetTouch(&CTF2Grenade::GrenadeTouch);
	pRocket->pev->owner = pOwner->edict();
	pRocket->pev->team = GetClassPtr((CBasePlayer*)(pOwner->pev))->m_iTeam;

	return pRocket;
}

Vector CTF2Grenade::ProjectionProduct(const Vector& a, const Vector& b)
{
	float flCos = DotProduct(a, b) / (a.Length() * b.Length());
	if (!flCos)
		return Vector(0, 0, 0);
	return (b.Normalize() * (a.Length() / flCos) - a);
}


//=========================================================
//=========================================================
void CTF2Grenade::Spawn()
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	pev->avelocity = Vector(RANDOM_FLOAT(0, 720), RANDOM_FLOAT(0, 720), RANDOM_FLOAT(0, 720));
	pev->takedamage = DAMAGE_YES;
	
	SET_MODEL(ENT(pev), "models/rooster_fortress/w_grenade.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin(pev, pev->origin);

	pev->classname = MAKE_STRING("tf2_grenade");

	SetThink(&CTF2Grenade::IgniteThink);
	SetTouch(&CTF2Grenade::GrenadeTouch);

	pev->dmgtime = gpGlobals->time + 2.28;
	pev->health = 999999;

	pev->velocity = gpGlobals->v_forward * 1216.6;
	pev->gravity = 1.0;

	pev->nextthink = gpGlobals->time + 0.1;

	pev->dmg = 100.0;

	m_flIgniteTime = gpGlobals->time;
	m_bFall = false;
	m_bRoll = false;
	m_iJumpTimes = 0;
}

//=========================================================
//=========================================================
void CTF2Grenade::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/w_grenade.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND("weapons/rocket1.wav");
}

void CTF2Grenade::GrenadeTouch(CBaseEntity* pOther)
{
	if (pOther->IsPlayer())
	{
		CBasePlayer* pPlayer = (CBasePlayer*)pOther;
		if (pPlayer->m_iTeam == pev->team)
			return;
		if (m_bFall && pev->velocity.Length() < 80)
			return;
		pev->enemy = pOther->edict();

		TraceResult tr;
		Vector vecSpot = pev->origin - pev->velocity.Normalize() * 32;
		UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr);
		Explode(&tr, DMG_BLAST | DMG_ALWAYSGIB);
	}
	else
	{
		if (m_bRoll)
		{
			pev->velocity = pev->movedir;
			return;
		}
		else if (pev->velocity.Length() <= 10 && (pev->flags & FL_ONGROUND) == 1)
		{
			pev->velocity = Vector(RANDOM_FLOAT(-32, 32), RANDOM_FLOAT(-32, 32), 32);
		}
		pev->velocity = pev->velocity * 0.5;

		TraceResult tr;
		Vector vecSpot = pev->origin;
		if (pev->velocity.Length() > 10)
			UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 128, ignore_monsters, ENT(pev), &tr);
		else
			UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -30), ignore_monsters, ENT(pev), &tr);
		if (tr.flFraction != 1)
		{
			bool bFall = false;
			float flAngle = M_PI / 2;
			if (tr.vecPlaneNormal.z)
			{
				Vector vecSB = Vector(0, 0, 1);
				flAngle = acos(DotProduct(vecSB, tr.vecPlaneNormal) / (tr.vecPlaneNormal.Length()));
				if (flAngle < M_PI / 3)
				{
					m_bFall = bFall = true;
				}
			}

			if (bFall && pev->velocity.Length() < 128)
			{
				UTIL_MakeVectors(pev->angles);
				Vector vecAngles = gpGlobals->v_forward;

				float flAnglesDiff = asin(fabs(DotProduct(tr.vecPlaneNormal, vecAngles)) / (vecAngles.Length()));

				if ((flAnglesDiff < M_PI / 4 && !m_bRoll) || m_iJumpTimes > 3)
				{
					vecAngles = ProjectionProduct(tr.vecPlaneNormal, vecAngles);
					pev->angles = UTIL_VecToAngles(vecAngles);
					pev->velocity = g_vecZero;
					pev->avelocity = g_vecZero;
					float flRollSpeed = 32;
					if (tr.vecPlaneNormal.Length2D() > 0)
					{
						Vector vecMaxRoll = ProjectionProduct(tr.vecPlaneNormal, Vector(0, 0, 1));
						float flCosRollAngles = fabs(DotProduct(vecMaxRoll, vecAngles)) / (vecMaxRoll.Length() * vecAngles.Length());
						flRollSpeed = max(512 * (1 - flCosRollAngles) * flAngle / (M_PI / 2), 32);
					}
					pev->velocity = CrossProduct(tr.vecPlaneNormal, vecAngles).Normalize() * flRollSpeed;
					if (pev->velocity.z > 0)
						pev->velocity = -pev->velocity;

					pev->movedir = pev->velocity;
					m_bRoll = true;
				}
				else
				{
					pev->velocity = pev->velocity * 1.5;
					pev->avelocity = pev->avelocity * 1.5;
					m_iJumpTimes++;
				}
			}
		}
	}
}

void CTF2Grenade::IgniteThink()
{
	// pev->movetype = MOVETYPE_TOSS;
	pev->effects |= EF_LIGHT;

	// trail
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex()); // entity
	WRITE_SHORT(m_iTrail);	 // model
	WRITE_BYTE(2);			 // life
	WRITE_BYTE(5);			 // width
	WRITE_BYTE(224);		 // r, g, b
	WRITE_BYTE(224);		 // r, g, b
	WRITE_BYTE(255);		 // r, g, b
	WRITE_BYTE(255);		 // brightness

	MESSAGE_END(); // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	SetThink(&CTF2Grenade::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CTF2Grenade::FollowThink()
{
	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
	}

	if ((pev->effects & EF_LIGHT) != 0)
	{
		pev->effects = 0;
	}

	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if ((UTIL_PointContents(pev->origin) == CONTENTS_SKY) || m_flIgniteTime + 2.0 < gpGlobals->time)
	{
		Detonate();
	}

	if (pev->dmgtime - gpGlobals->time > 0.20f)
		pev->nextthink = gpGlobals->time + 0.1;
	else
		pev->nextthink = gpGlobals->time + 0.01;
}
#endif

void CGrenadelauncher::Reload()
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
		SendWeaponAnim(GRENADELAUNCHER_START_RELOAD);
		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.24;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.24;
		m_flNextPrimaryAttack = GetNextAttackDelay(1.24);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.24;
		return;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		SendWeaponAnim(GRENADELAUNCHER_RELOAD);
		
		m_fInSpecialReload = 2;

		if (RANDOM_LONG(0, 1))
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload1.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));
		else
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/reload3.wav", 1, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 0x1f));

		// too long, shorten to match reload anim to make it smoother
		m_flNextPrimaryAttack = GetNextAttackDelay(0.6);
		m_flNextReload = UTIL_WeaponTimeBase() + 0.6;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6;
	}
	else
	{
		// Add them to the clip
		m_iClip += 1;
		m_pPlayer->ammo_rockets -= 1;
		m_fInSpecialReload = 1;
	}
}

void CGrenadelauncher::Spawn()
{
	Precache();
	m_iId = WEAPON_RPG;
	
	SET_MODEL(ENT(pev), "models/rooster_fortress/wp_group_2bone.mdl");
	pev->sequence = 0;
	pev->body = 0;

	m_iDefaultAmmo = GRENADELAUNCHER_MAX_CLIP + GRENADELAUNCHER_MAX_CARRY;

	FallInit(); // get ready to fall down.
}


void CGrenadelauncher::Precache()
{
	PRECACHE_MODEL("models/rooster_fortress/wp_group_2bone.mdl");
	PRECACHE_MODEL("models/rooster_fortress/viewmodels/v_grenadelauncher.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	UTIL_PrecacheOther("tf2_grenade");

	PRECACHE_SOUND("weapons/glauncher.wav");

	m_usGrenadeLauncher = PRECACHE_EVENT(1, "events/grenadelauncher.sc");
}


bool CGrenadelauncher::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "rockets";
	p->iMaxAmmo1 = GRENADELAUNCHER_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GRENADELAUNCHER_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_GRENADELAUNCHER;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHTO;
	p->iWeight = RPG_WEIGHT;

	return true;
}

bool CGrenadelauncher::Deploy()
{
	return DefaultDeploy("models/rooster_fortress/viewmodels/v_grenadelauncher.mdl", "models/rooster_fortress/wp_group_2bone.mdl", GRENADELAUNCHER_DRAW, "rpg");
}


bool CGrenadelauncher::CanHolster()
{
	return true;
}

void CGrenadelauncher::Holster()
{
	m_fInReload = false; // cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(GRENADELAUNCHER_IDLE);
}

void CGrenadelauncher::PrimaryAttack()
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

		CTF2Grenade* pRocket = CTF2Grenade::CreateTF2Grenade(vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this);

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

		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usGrenadeLauncher);

		m_iClip--;

		m_flNextPrimaryAttack = GetNextAttackDelay(0.6);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6;

		m_fInSpecialReload = 0;

		ResetEmptySound();
	}
	else
	{
		PlayEmptySound();
	}
}


void CGrenadelauncher::SecondaryAttack()
{
}


void CGrenadelauncher::WeaponIdle()
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
			SendWeaponAnim(GRENADELAUNCHER_AFTER_RELOAD);

			m_fInSpecialReload = 0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8;
			m_flNextPrimaryAttack = GetNextAttackDelay(0.8);
		}
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (20.0 / 9.0);
		SendWeaponAnim(GRENADELAUNCHER_IDLE);
	}
}

class CGrenadelauncherAmmo : public CBasePlayerAmmo
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
LINK_ENTITY_TO_CLASS(ammo_grenadelauncherclip, CGrenadelauncherAmmo);
