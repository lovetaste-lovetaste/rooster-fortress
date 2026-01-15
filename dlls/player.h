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

#pragma once

#include "pm_materials.h"


#define PLAYER_FATAL_FALL_SPEED 1024															  
#define PLAYER_MAX_SAFE_FALL_SPEED 580															 
#define DAMAGE_FOR_FALL_SPEED (float)100 / (PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED) // damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED 200
#define PLAYER_FALL_PUNCH_THRESHHOLD (float)350 // won't punch player's screen/make scrape noise unless player falling at least this fast.

//
// Player PHYSICS FLAGS bits
//
#define PFLAG_ONLADDER (1 << 0)
#define PFLAG_ONSWING (1 << 0)
#define PFLAG_ONTRAIN (1 << 1)
#define PFLAG_ONBARNACLE (1 << 2)
#define PFLAG_DUCKING (1 << 3)	// In the process of ducking, but totally squatted yet
#define PFLAG_USING (1 << 4)	// Using a continuous entity
#define PFLAG_OBSERVER (1 << 5) // player is locked in stationary cam mode. Spectators can move, observers can't.

//
// generic player
//
//-----------------------------------------------------
//This is Half-Life player entity
//-----------------------------------------------------
#define CSUITPLAYLIST 4 // max of 4 suit sentences queued up at any time

#define SUIT_GROUP true
#define SUIT_SENTENCE false

#define SUIT_REPEAT_OK 0
#define SUIT_NEXT_IN_30SEC 30
#define SUIT_NEXT_IN_1MIN 60
#define SUIT_NEXT_IN_5MIN 300
#define SUIT_NEXT_IN_10MIN 600
#define SUIT_NEXT_IN_30MIN 1800
#define SUIT_NEXT_IN_1HOUR 3600

#define CSUITNOREPEAT 32

#define SOUND_FLASHLIGHT_ON "items/flashlight1.wav"
#define SOUND_FLASHLIGHT_OFF "items/flashlight1.wav"

#define TEAM_NAME_LENGTH 16

typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
} PLAYER_ANIM;

#define MAX_ID_RANGE 2048
#define SBAR_STRING_SIZE 128

enum sbar_data
{
	SBAR_ID_TARGETNAME = 1,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETARMOR,
	SBAR_END,
};

// THIS IS DIRECTLY GRABBED FROM THE TF2 SDK

enum ETFCond
{
	TF_COND_INVALID = -1,
	TF_COND_AIMING = 0, // Sniper aiming, Heavy minigun.
	TF_COND_ZOOMED = 1,
	TF_COND_DISGUISING = 2,
	TF_COND_DISGUISED = 3,
	TF_COND_STEALTHED = 4, // Spy specific
	TF_COND_INVULNERABLE = 5,
	TF_COND_TELEPORTED = 6,
	TF_COND_TAUNTING = 7,
	TF_COND_INVULNERABLE_WEARINGOFF = 8,
	TF_COND_STEALTHED_BLINK = 9,
	TF_COND_SELECTED_TO_TELEPORT = 10,
	TF_COND_CRITBOOSTED = 11, // DO NOT RE-USE THIS -- THIS IS FOR KRITZKRIEG AND REVENGE CRITS ONLY
	TF_COND_TMPDAMAGEBONUS = 12,
	TF_COND_FEIGN_DEATH = 13,
	TF_COND_PHASE = 14,
	TF_COND_STUNNED = 15, // Any type of stun. Check iStunFlags for more info.
	TF_COND_OFFENSEBUFF = 16,
	TF_COND_SHIELD_CHARGE = 17,
	TF_COND_DEMO_BUFF = 18,
	TF_COND_ENERGY_BUFF = 19,
	TF_COND_RADIUSHEAL = 20,
	TF_COND_HEALTH_BUFF = 21,
	TF_COND_BURNING = 22,
	TF_COND_HEALTH_OVERHEALED = 23,
	TF_COND_URINE = 24,
	TF_COND_BLEEDING = 25,
	TF_COND_DEFENSEBUFF = 26, // 35% defense! No crit damage.
	TF_COND_MAD_MILK = 27,
	TF_COND_MEGAHEAL = 28,
	TF_COND_REGENONDAMAGEBUFF = 29,
	TF_COND_MARKEDFORDEATH = 30,
	TF_COND_NOHEALINGDAMAGEBUFF = 31,
	TF_COND_SPEED_BOOST = 32,		  // = 32
	TF_COND_CRITBOOSTED_PUMPKIN = 33, // Brandon hates bits
	TF_COND_CRITBOOSTED_USER_BUFF = 34,
	TF_COND_CRITBOOSTED_DEMO_CHARGE = 35,
	TF_COND_SODAPOPPER_HYPE = 36,
	TF_COND_CRITBOOSTED_FIRST_BLOOD = 37, // arena mode first blood
	TF_COND_CRITBOOSTED_BONUS_TIME = 38,
	TF_COND_CRITBOOSTED_CTF_CAPTURE = 39,
	TF_COND_CRITBOOSTED_ON_KILL = 40, // =40. KGB, etc.
	TF_COND_CANNOT_SWITCH_FROM_MELEE = 41,
	TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK = 42, // 35% defense! Still damaged by crits.
	TF_COND_REPROGRAMMED = 43,				// Bots only
	TF_COND_CRITBOOSTED_RAGE_BUFF = 44,
	TF_COND_DEFENSEBUFF_HIGH = 45,		 // 75% defense! Still damaged by crits.
	TF_COND_SNIPERCHARGE_RAGE_BUFF = 46, // Sniper Rage - Charge time speed up
	TF_COND_DISGUISE_WEARINGOFF = 47,	 // Applied for half-second post-disguise
	TF_COND_MARKEDFORDEATH_SILENT = 48,	 // Sans sound
	TF_COND_DISGUISED_AS_DISPENSER = 49,
	TF_COND_SAPPED = 50, // =50. Bots only
	TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED = 51,
	TF_COND_INVULNERABLE_USER_BUFF = 52,
	TF_COND_HALLOWEEN_BOMB_HEAD = 53,
	TF_COND_HALLOWEEN_THRILLER = 54,
	TF_COND_RADIUSHEAL_ON_DAMAGE = 55,
	TF_COND_CRITBOOSTED_CARD_EFFECT = 56,
	TF_COND_INVULNERABLE_CARD_EFFECT = 57,
	TF_COND_MEDIGUN_UBER_BULLET_RESIST = 58,
	TF_COND_MEDIGUN_UBER_BLAST_RESIST = 59,
	TF_COND_MEDIGUN_UBER_FIRE_RESIST = 60, // =60
	TF_COND_MEDIGUN_SMALL_BULLET_RESIST = 61,
	TF_COND_MEDIGUN_SMALL_BLAST_RESIST = 62,
	TF_COND_MEDIGUN_SMALL_FIRE_RESIST = 63,
	TF_COND_STEALTHED_USER_BUFF = 64, // Any class can have this
	TF_COND_MEDIGUN_DEBUFF = 65,
	TF_COND_STEALTHED_USER_BUFF_FADING = 66,
	TF_COND_BULLET_IMMUNE = 67,
	TF_COND_BLAST_IMMUNE = 68,
	TF_COND_FIRE_IMMUNE = 69,
	TF_COND_PREVENT_DEATH = 70,			 // =70
	TF_COND_MVM_BOT_STUN_RADIOWAVE = 71, // Bots only
	TF_COND_HALLOWEEN_SPEED_BOOST = 72,
	TF_COND_HALLOWEEN_QUICK_HEAL = 73,
	TF_COND_HALLOWEEN_GIANT = 74,
	TF_COND_HALLOWEEN_TINY = 75,
	TF_COND_HALLOWEEN_IN_HELL = 76,
	TF_COND_HALLOWEEN_GHOST_MODE = 77, // =77
	TF_COND_MINICRITBOOSTED_ON_KILL = 78,
	TF_COND_OBSCURED_SMOKE = 79,
	TF_COND_PARACHUTE_ACTIVE = 80, // actively being used (not retracted)
	TF_COND_BLASTJUMPING = 81,
	TF_COND_HALLOWEEN_KART = 82,
	TF_COND_HALLOWEEN_KART_DASH = 83,
	TF_COND_BALLOON_HEAD = 84,		  // =84 larger head, lower-gravity-feeling jumps
	TF_COND_MELEE_ONLY = 85,		  // =85 melee only
	TF_COND_SWIMMING_CURSE = 86,	  // player movement become swimming movement
	TF_COND_FREEZE_INPUT = 87,		  // freezes player input
	TF_COND_HALLOWEEN_KART_CAGE = 88, // attach cage model to player while in kart
	TF_COND_DONOTUSE_0 = 89,
	TF_COND_RUNE_STRENGTH = 90,
	TF_COND_RUNE_HASTE = 91,
	TF_COND_RUNE_REGEN = 92,
	TF_COND_RUNE_RESIST = 93,
	TF_COND_RUNE_VAMPIRE = 94,
	TF_COND_RUNE_REFLECT = 95,
	TF_COND_RUNE_PRECISION = 96,
	TF_COND_RUNE_AGILITY = 97,
	TF_COND_GRAPPLINGHOOK = 98,
	TF_COND_GRAPPLINGHOOK_SAFEFALL = 99,
	TF_COND_GRAPPLINGHOOK_LATCHED = 100,
	TF_COND_GRAPPLINGHOOK_BLEEDING = 101,
	TF_COND_AFTERBURN_IMMUNE = 102,
	TF_COND_RUNE_KNOCKOUT = 103,
	TF_COND_RUNE_IMBALANCE = 104,
	TF_COND_CRITBOOSTED_RUNE_TEMP = 105,
	TF_COND_PASSTIME_INTERCEPTION = 106,
	TF_COND_SWIMMING_NO_EFFECTS = 107, // =107_DNOC_FT
	TF_COND_PURGATORY = 108,
	TF_COND_RUNE_KING = 109,
	TF_COND_RUNE_PLAGUE = 110,
	TF_COND_RUNE_SUPERNOVA = 111,
	TF_COND_PLAGUE = 112,
	TF_COND_KING_BUFFED = 113,
	TF_COND_TEAM_GLOWS = 114, // used to show team glows to living players
	TF_COND_KNOCKED_INTO_AIR = 115,
	TF_COND_COMPETITIVE_WINNER = 116,
	TF_COND_COMPETITIVE_LOSER = 117,
	TF_COND_HEALING_DEBUFF = 118,
	TF_COND_PASSTIME_PENALTY_DEBUFF = 119, // when carrying the ball without any teammates nearby
	TF_COND_GRAPPLED_TO_PLAYER = 120,
	TF_COND_GRAPPLED_BY_PLAYER = 121,
	TF_COND_PARACHUTE_DEPLOYED = 122, // activated at least once while player's been airborne, but not does mean it's active now (see TF_COND_PARACHUTE_ACTIVE)
	TF_COND_GAS = 123,
	TF_COND_BURNING_PYRO = 124,
	TF_COND_ROCKETPACK = 125,
	// Players who lose their footing have lessened friction and don't re-stick to the ground unless they're below a
	// tf_movement_lost_footing_restick speed
	TF_COND_LOST_FOOTING = 126,
	// When in the air, slide up/along surfaces with momentum as if caught up in a... blast of air of some sort.
	// Reduces air control as well.  See tf_movement_aircurrent convars.  Removed upon touching ground.
	TF_COND_AIR_CURRENT = 127,
	TF_COND_HALLOWEEN_HELL_HEAL = 128,
	TF_COND_POWERUPMODE_DOMINANT = 129,
	TF_COND_IMMUNE_TO_PUSHBACK = 130,
	//
	// ADD NEW ITEMS HERE TO AVOID BREAKING DEMOS
	//

	// ******** Keep this block last! ********
	// Keep experimental conditions below and graduate out of it before shipping

	TF_COND_LAST
};

#define CLASS_SCOUT 1
#define CLASS_HEAVY 2
#define CLASS_SOLDIER 3
#define CLASS_PYRO 4
#define CLASS_SNIPER 5
#define CLASS_MEDIC 6
#define CLASS_ENGINEER 7
#define CLASS_DEMOMAN 8
#define CLASS_SPY 9

#define CHAT_INTERVAL 1.0f

class CBasePlayer : public CBaseMonster
{
public:
	// Spectator camera
	void Observer_FindNextPlayer(bool bReverse);
	void Observer_HandleButtons();
	void Observer_SetMode(int iMode);
	void Observer_CheckTarget();
	void Observer_CheckProperties();
	EHANDLE m_hObserverTarget;
	float m_flNextObserverInput;
	int m_iObserverWeapon;	 // weapon of current tracked target
	int m_iObserverLastMode; // last used observer mode
	bool IsObserver() { return 0 != pev->iuser1; }

	int random_seed; // See that is shared between client & server for shared weapons code

	int m_iTeam;
	int GetTeamNumber();
	void SetTeamNumber(int newTeam);

	int m_iClass;			// tf2 class
	int m_iPlayerSound;		// the index of the sound list slot reserved for this player
	int m_iTargetVolume;	// ideal sound volume.
	int m_iWeaponVolume;	// how loud the player's weapon is right now.
	int m_iExtraSoundTypes; // additional classification for this weapon's sound
	int m_iWeaponFlash;		// brightness of the weapon flash
	float m_flStopExtraSoundTime;

	float m_flFlashLightTime; // Time until next battery draw/Recharge
	int m_iFlashBattery;	  // Flashlight Battery Draw

	int m_afButtonLast;
	int m_afButtonPressed;
	int m_afButtonReleased;

	EHANDLE m_SndLast;	   // last sound entity to modify player room type
	int m_SndRoomtype = 0; // last roomtype set by sound entity. Defaults to 0 on new maps to disable it by default.
	int m_ClientSndRoomtype;
	float m_flSndRange; // dist from player to sound entity

	float m_flFallVelocity;

	int m_rgItems[MAX_ITEMS];
	bool m_fKnownItem; // True when a new item needs to be added
	int m_fNewAmmo;	   // True when a new item has been added

	unsigned int m_afPhysicsFlags; // physics flags - set when 'normal' physics should be revisited or overriden
	float m_fNextSuicideTime;	   // the time after which the player can next use the suicide command


	// these are time-sensitive things that we keep track of
	float m_flTimeStepSound;  // when the last stepping sound was made
	float m_flTimeWeaponIdle; // when to play another weapon idle animation.
	float m_flSwimTime;		  // how long player has been underwater
	float m_flDuckTime;		  // how long we've been ducking
	float m_flWallJumpTime;	  // how long until next walljump

	float m_flSuitUpdate;						 // when to play next suit update
	int m_rgSuitPlayList[CSUITPLAYLIST];		 // next sentencenum to play for suit update
	int m_iSuitPlayNext;						 // next sentence slot for queue storage;
	int m_rgiSuitNoRepeat[CSUITNOREPEAT];		 // suit sentence no repeat list
	float m_rgflSuitNoRepeatTime[CSUITNOREPEAT]; // how long to wait before allowing repeat
	int m_lastDamageAmount;						 // Last damage taken
	float m_tbdPrev;							 // Time-based damage timer

	float m_flgeigerRange; // range to nearest radiation source
	float m_flgeigerDelay; // delay per update of range msg to client
	int m_igeigerRangePrev;
	int m_iStepLeft;						// alternate left/right foot stepping sound
	char m_szTextureName[CBTEXTURENAMEMAX]; // current texture name we're standing on
	char m_chTextureType;					// current texture type

	int m_idrowndmg;	  // track drowning damage taken
	int m_idrownrestored; // track drowning damage restored

	int m_bitsHUDDamage; // Damage bits for the current fame. These get sent to
						 // the hude via the DAMAGE message
	bool m_fInitHUD;	 // True when deferred HUD restart msg needs to be sent
	bool m_fGameHUDInitialized;
	int m_iTrain;	// Train control position
	bool m_fWeapon; // Set this to false to force a reset of the current weapon HUD info

	EHANDLE m_pTank;		 // the tank which the player is currently controlling,  NULL if no tank
	EHANDLE m_hViewEntity;	 // The view entity being used, or null if the player is using itself as the view entity
	bool m_bResetViewEntity; //True if the player's view needs to be set back to the view entity
	float m_fDeadTime;		 // the time at which the player died  (used in PlayerDeathThink())

	bool m_fNoPlayerSound; // a debugging feature. Player makes no sound if this is true.
	bool m_fLongJump;	   // does this player have the longjump module?

	float m_tSneaking;
	int m_iUpdateTime;	  // stores the number of frame ticks before sending HUD update messages
	int m_iClientHealth;  // the health currently known by the client.  If this changes, send a new
	int m_iClientBattery; // the Battery currently known by the client.  If this changes, send a new
	int m_iHideHUD;		  // the players hud weapon info is to be hidden
	int m_iClientHideHUD;
	int m_iFOV;		  // field of view
	int m_iClientFOV; // client's known FOV
	// usable player items
	CBasePlayerItem* m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem* m_pActiveItem;
	CBasePlayerItem* m_pClientActiveItem; // client version of the active item
	CBasePlayerItem* m_pLastItem;

	std::uint64_t m_WeaponBits;

	//Not saved, used to update client.
	std::uint64_t m_ClientWeaponBits;

	// shared ammo slots
	int m_rgAmmo[MAX_AMMO_SLOTS];
	int m_rgAmmoLast[MAX_AMMO_SLOTS];

	Vector m_vecAutoAim;
	bool m_fOnTarget;
	int m_iDeaths;
	float m_flRespawnTimer; // used in PlayerDeathThink() to make sure players can always respawn

	int m_lastx, m_lasty; // These are the previous update's crosshair angles, DON"T SAVE/RESTORE

	int m_nCustomSprayFrames = -1; // Custom clan logo frames for this player
	float m_flNextDecalTime;  // next time this player can spray a decal

	char m_szTeamName[TEAM_NAME_LENGTH];

	void SetPlayerModel();

	void Spawn() override;
	void Pain();

	//	void Think() override;
	virtual void Jump();
	virtual void Duck();
	virtual void PreThink();
	virtual void PostThink();
	Vector GetGunPosition() override;
	bool TakeHealth(float flHealth, int bitsDamageType) override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void Killed(entvars_t* pevAttacker, int iGib) override;
	Vector BodyTarget(const Vector& posSrc) override { return Center() + pev->view_ofs * RANDOM_FLOAT(0.5, 1.1); } // position to shoot at
	void StartSneaking() override { m_tSneaking = gpGlobals->time - 1; }
	void StopSneaking() override { m_tSneaking = gpGlobals->time + 30; }
	bool IsSneaking() override { return m_tSneaking <= gpGlobals->time; }
	bool IsAlive() override { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	bool ShouldFadeOnDeath() override { return false; }
	bool IsPlayer() override { return true; } // Spectators should return false for this, they aren't "players" as far as game logic is concerned

	bool IsNetClient() override { return (pev->flags & FL_FAKECLIENT) == 0; }
	// Bots should return false for this, they can't receive NET messages. Spectators should return true for this

	const char* TeamID() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	void RenewItems();
	void SpawnClassWeapons();
	void PackDeadPlayerItems();
	void RemoveAllItems(bool removeSuit);
	bool SwitchWeapon(CBasePlayerItem* pWeapon);

	/**
	*	@brief Equips an appropriate weapon for the player if they don't have one equipped already.
	*/
	void EquipWeapon();

	void SetWeaponBit(int id);
	void ClearWeaponBit(int id);

	bool HasSuit() const;
	void SetHasSuit(bool hasSuit);

	// JOHN:  sends custom messages if player HUD data has changed  (eg health, ammo)
	virtual void UpdateClientData();

	static TYPEDESCRIPTION m_playerSaveData[];

	// Player is moved across the transition by other means
	int ObjectCaps() override { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void Precache() override;
	bool IsOnLadder();
	bool FlashlightIsOn();
	void FlashlightTurnOn();
	void FlashlightTurnOff();

	void UpdatePlayerSound();
	void DeathSound() override;

	int Classify() override;
	void SetAnimation(PLAYER_ANIM playerAnim);
	void SetWeaponAnimType(const char* szExtention);
	char m_szAnimExtention[32];

	// custom player functions
	virtual void ImpulseCommands();
	void CheatImpulseCommands(int iImpulse);

	void StartDeathCam();
	void StartObserver(Vector vecPosition, Vector vecViewAngle);

	void AddPoints(int score, bool bAllowNegativeScore) override;
	void AddPointsToTeam(int score, bool bAllowNegativeScore) override;
	bool AddPlayerItem(CBasePlayerItem* pItem) override;
	bool RemovePlayerItem(CBasePlayerItem* pItem) override;
	void DropPlayerItem(char* pszItemName);
	bool HasPlayerItem(CBasePlayerItem* pCheckItem);
	bool HasNamedPlayerItem(const char* pszItemName);
	bool HasPlayerItemFromID(int nID);
	bool HasWeapons(); // do I have ANY weapons?
	void SelectPrevItem(int iItem);
	void SelectNextItem(int iItem);
	void SelectLastItem();
	void SelectItem(const char* pstr);
	void ItemPreFrame();
	void ItemPostFrame();
	void GiveNamedItem(const char* szName);
	void GiveNamedItem(const char* szName, int defaultAmmo);
	void EnableControl(bool fControl);

	int GiveAmmo(int iAmount, const char* szName, int iMax) override;
	void SendAmmoUpdate();
	void SendSingleAmmoUpdate(int ammoIndex);

private:
	void InternalSendSingleAmmoUpdate(int ammoIndex);

public:
	void WaterMove();
	void EXPORT PlayerDeathThink();
	void PlayerUse();

	void CheckSuitUpdate();
	void SetSuitUpdate(const char* name, bool fgroup, int iNoRepeat);
	void UpdateGeigerCounter();
	void CheckTimeBasedDamage();

	bool FBecomeProne() override;
	void BarnacleVictimBitten(entvars_t* pevBarnacle) override;
	void BarnacleVictimReleased() override;
	static int GetAmmoIndex(const char* psz);
	int AmmoInventory(int iAmmoIndex);
	int Illumination() override;

	void ResetAutoaim();
	Vector GetAutoaimVector(float flDelta);
	Vector AutoaimDeflection(Vector& vecSrc, float flDist, float flDelta);

	void ForceClientDllUpdate(); // Forces all client .dll specific data to be resent to client.

	void DeathMessage(entvars_t* pevKiller);

	void SetCustomDecalFrames(int nFrames);
	int GetCustomDecalFrames();

	void TabulateAmmo();

	float m_flStartCharge;
	float m_flAmmoStartCharge;
	float m_flPlayAftershock;
	float m_flNextAmmoBurn; // while charging, when to absorb another unit of player's ammo?

	//Player ID
	void InitStatusBar();
	void UpdateStatusBar();
	int m_izSBarState[SBAR_END];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[SBAR_STRING_SIZE];
	char m_SbarString1[SBAR_STRING_SIZE];

	float m_flNextChatTime;

	void SetPrefsFromUserinfo(char* infobuffer);

	bool IsFakeClient(void);

	int m_iAutoWepSwitch;

	bool m_bRestored;

	//True if the player is currently spawning.
	bool m_bIsSpawning = false;

	/**
	 *    @brief True if the player is currently connected to the server.
	 *    Should only be false in multiplayer games, for players that have disconnected.
	 */
	bool m_bIsConnected = true;

};


inline void CBasePlayer::SetWeaponBit(int id)
{
	m_WeaponBits |= 1ULL << id;
}

inline void CBasePlayer::ClearWeaponBit(int id)
{
	m_WeaponBits &= ~(1ULL << id);
}

inline bool CBasePlayer::HasSuit() const
{
	return (m_WeaponBits & (1ULL << WEAPON_SUIT)) != 0;
}

inline void CBasePlayer::SetHasSuit(bool hasSuit)
{
	if (hasSuit)
	{
		SetWeaponBit(WEAPON_SUIT);
	}
	else
	{
		ClearWeaponBit(WEAPON_SUIT);
	}
}

#define AUTOAIM_2DEGREES 0.0348994967025
#define AUTOAIM_5DEGREES 0.08715574274766
#define AUTOAIM_8DEGREES 0.1391731009601
#define AUTOAIM_10DEGREES 0.1736481776669

inline bool gInitHUD = true;
inline bool gEvilImpulse101 = false;
inline bool giPrecacheGrunt = false;

/**
*	@brief Display the game title if this key is set
*/
inline DLL_GLOBAL bool gDisplayTitle = false;
inline DLL_GLOBAL unsigned int g_ulModelIndexPlayer = 0;
inline DLL_GLOBAL CBaseEntity* g_pLastSpawn = nullptr;
