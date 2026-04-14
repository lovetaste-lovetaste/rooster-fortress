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

#define SNIPER_LASER_SPRITE "sprites/CKF_III/sniperdot.spr"
#define SNIPER_LASER_RED 255
#define SNIPER_LASER_GREEN 50
#define SNIPER_LASER_BLUE 50

extern int gmsgSniperScope;

class CSniperDot : public CBaseEntity
{
public:
	void Spawn() override;
	void Update(Vector vecOrigin, float flCharge);
	static CSniperDot* Create(CBaseEntity* pOwner);

	void SetScale(float flScale)
	{
		pev->scale = flScale;
	}
};

void CSniperDot::Spawn()
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 200;
	pev->scale = 1.0f;

	SET_MODEL(ENT(pev), SNIPER_LASER_SPRITE);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
}

void CSniperDot::Update(Vector vecOrigin, float flCharge)
{
	UTIL_SetOrigin(pev, vecOrigin);

	// Scale 0.05 (min charge) -> 0.20 (max charge)
	float flNorm = (flCharge - TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN) / (float)(TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX - TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN);
	flNorm = flNorm < 0.0f ? 0.0f : flNorm > 1.0f ? 1.0f
												  : flNorm;
	pev->scale = 0.05f + flNorm * 0.15f;

	// Brightness also ramps up with charge
	pev->renderamt = (int)(100 + flNorm * 155.0f); // 100 -> 255
}

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

CSniperDot* CSniperDot::Create(CBaseEntity* pOwner)
{
	CSniperDot* pDot = GetClassPtr<CSniperDot>(nullptr);
	pDot->pev->owner = ENT(pOwner->pev);
	pDot->Spawn();
	return pDot;
}

void CSniperRifle::Spawn()
{
	pev->classname = MAKE_STRING("weapon_sniperrifle"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_SNIPERRIFLE;
	SET_MODEL(ENT(pev), "models/rooster_fortress/wp_group_rf.mdl");
	pev->sequence = 20;
	pev->body = 57;
	m_pSniperDot = nullptr;
	m_iDefaultAmmo = SNIPERRIFLE_MAX_CARRY;

	FallInit(); // get ready to fall down.
}


LINK_ENTITY_TO_CLASS(weapon_sniperrifle, CSniperRifle);

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

	m_iLaserSprite = PRECACHE_MODEL(SNIPER_LASER_SPRITE);
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
	DestroyDot();
	SendScopeMessage(false);
	m_bWasScoped = false;
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

void CSniperRifle::SendScopeMessage(bool bScoped)
{
#ifndef CLIENT_DLL
	// charge as 0-100 percentage
	int iCharge = (int)(((m_flChargedDamage - TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN) / (float)(TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX - TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN)) * 100.0f);
	iCharge = iCharge < 0 ? 0 : iCharge > 100 ? 100 : iCharge;

	MESSAGE_BEGIN(MSG_ONE, gmsgSniperScope, NULL, m_pPlayer->pev);
	WRITE_BYTE(bScoped ? 1 : 0);
	WRITE_BYTE(iCharge);
	MESSAGE_END();
#endif
}

void CSniperRifle::CreateDot()
{
#ifndef CLIENT_DLL
	if (!m_pSniperDot)
		m_pSniperDot = CSniperDot::Create(m_pPlayer);
#endif
}

void CSniperRifle::DestroyDot()
{
#ifndef CLIENT_DLL
	if (m_pSniperDot)
	{
		UTIL_Remove(m_pSniperDot);
		m_pSniperDot = nullptr;
	}
#endif
}

void CSniperRifle::UpdateDot()
{
#ifndef CLIENT_DLL
	if (!m_pSniperDot)
		return;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 8192;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	// Pull the dot slightly off the surface so it doesn't z-fight
	Vector vecDotPos = tr.vecEndPos + tr.vecPlaneNormal * 1.0f;

	m_pSniperDot->Update(vecDotPos, m_flChargedDamage);
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
	// 
	
	float flDamage = (((m_flChargedDamage) > (TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN)) ? (m_flChargedDamage) : (TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN));

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
	vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_0DEGREES, 8192, BULLET_PLAYER_TF2_HEADSHOT, 0, flDamage, m_pPlayer->pev, m_pPlayer->random_seed);
	
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
	m_flUnscopeTime = gpGlobals->time + 0.5;
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
//
void CSniperRifle::ItemPostFrame()
{
// #ifdef CLIENT_DLL
	// ALERT(at_console, "Client: m_flUnscopeTime: %f || time: %f\n", m_flUnscopeTime, gpGlobals->time);
// #endif

#ifndef CLIENT_DLL
	bool bScoped = m_pPlayer->IsInCond(TF_COND_AIMING);

	if (m_pPlayer->pev->button & IN_JUMP && (pev->flags & FL_ONGROUND))
	{
		// this checks if ur on the ground and trying to jump
		// this SPECIFICALLY checks if ur on the ground due to dropshots being a thing in normal tf2
		ALERT(at_console, "Unscope due to jump");
		m_pPlayer->RemoveCondition(TF_COND_AIMING);
		bScoped = false;
	}
	if (m_flUnscopeTime > 0 && gpGlobals->time > m_flUnscopeTime)
	{
		ALERT(at_console, "Unscope due to unscope time");
		m_pPlayer->RemoveCondition(TF_COND_AIMING);
		bScoped = false;
		m_flUnscopeTime = -1.0;	// just in case
	}

	if (bScoped) //	scoped in stuff
	{
		if (m_pPlayer->m_iFOV != 20)
			m_pPlayer->m_iFOV = 20;

		m_flChargedDamage = (((m_flChargedDamage + gpGlobals->frametime * TF_WEAPON_SNIPERRIFLE_CHARGE_PER_SEC) < (TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX)) ? (m_flChargedDamage + gpGlobals->frametime * TF_WEAPON_SNIPERRIFLE_CHARGE_PER_SEC) : (TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX));
		CreateDot(); // no-op if already exists
		UpdateDot(); // reposition + rescale every frame
	}
	else //	un-scoped stuff
	{
		m_flChargedDamage = 0.0;
		m_flUnscopeTime = -1.0;
		DestroyDot();	// destroys the dot if its still active

		if (m_pPlayer->m_iFOV != 0)
		{
			m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
		}
		
	}

	// Send scope state message only when it changes
	if (bScoped != m_bWasScoped)
	{
		SendScopeMessage(bScoped);
		m_bWasScoped = bScoped;
	}
	// While scoped, keep sending charge % so the HUD meter updates
	else if (bScoped)
	{
		SendScopeMessage(true);
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
