void Bot_Think(int client);
void Bot_RunAll();
void Bot_AliveThink(int client, Vector* vecAngles, Vector* vecMove);
void Bot_AliveMovementThink(int client, Vector* vecAngles, Vector* vecMove);
void Bot_AliveWeaponThink(int client, Vector* vecAngles, Vector* vecMove);
void Bot_DeadThink(int client, Vector* vecAngles, Vector* vecMove);
void Bot_Start(int client);

CBaseEntity* FindNearestEnemy(int client, int mode, float MAXRANGE);
float clamp(float main, float min, float max);
float AngleNormalize(float angle);
bool IsTargetVisible(int client, CBaseEntity* pTarget);

void BotMatchFacing(CBasePlayer* pBot, const Vector& v_source, Vector v_focus);
void BotFixIdealPitch(edict_t* pEdict);
void BotFixIdealYaw(edict_t* pEdict);

#define TF_MAXPLAYERS 32 
// change this if you need to. unless you somehow have more bots than your max players, it should be fine

// bullet types
typedef enum
{
	LEVEL_NONE = 0,
	LEVEL_1,
	LEVEL_2,
	LEVEL_3,
	LEVEL_3_1,
	LEVEL_3_4,
	LEVEL_4,
	LEVEL_4_1,
	LEVEL_4_2,
	LEVEL_4_3
} MovementLevel;

typedef struct
{
	bool Enabled;

	int pyroSpycheckTimer;
	int spyBackstabTimer;
	CBaseEntity* Targets[2];
	
	int Difficulty[10];
	// the first dimension is the base difficulty
	// the rest are class difficulties
	// this represents how people have both base skill and class skill; players can be decent overall but better on specifically their main

	float ConfidenceAgainst[TF_MAXPLAYERS];

	float altTabbedTimer;
	// if non-zero, the bot will not do anything. stimulates people alt tabbed
	float noMouseTimer;
	// same as above, but just for mouse

	float timeSpentUnderwater;
	float optimalDistance;
	float BaseReactionTime;
	float LastTimeReacted;
	float combatPercent;
	float AwarenessOfClient[TF_MAXPLAYERS];
	float SpySuspisionOfClient[TF_MAXPLAYERS];
	float SpycheckingAwarenessOfClient[TF_MAXPLAYERS];
	float DisregardTimer[TF_MAXPLAYERS];
	/*
	bot disregard timer
		bots have a timer that ticks down for each client
		while sorting for targets, if they are disregarding, they will ignore
			this timer has special values
				-1 is disregard until either dies
				-2 is disregard until either damages the other. if either bot goes off screen, the timer is set to a positive number to represent people still being friends for a SHORT time but eventually going back to normal
				-3 is disregard until either kills the other ( only the victim stop disregarding, the attacker doesnt stop. this is meant for accidental kills that cause alliances to break )
				-4 is disregard forever
		mainly used for scenarios where normal players fully ignore other players
		THIS IS NOT FOR FRIENDLIES, FRIENDLIES AND OTHER THINGS THAT REQUIRE FRIENDLIES ( players witnessing a cool thing another player did, friendlies, funny moments that are only temporary ) HAVE THE SAME SYSTEM BUT SEPEREATE. NOT IMPLEMENTED YET
	*/
	float RecentDamageTaken[2];
	// first dimension is for the amount of gathered damage
	// second is the time
	// if the time is more than 2 seconds, then the damage will be reset and only take into account the recent dmg
	// otherwise, it will add onto the dmg

	MovementLevel personalLevel[6];

	// PERSONALITY STUFF
	int Ego;
	int Paranoia; // handles spy stuff

	int MechanicalSkill; // handles skillbased stuff that varies depending on how good someone is mechanically, disregarding the difficulty. there are some mechanically cracked players that are ASS ( cs players that just moved to tf2 lolz ), and there are some players that are high skill but SHIT mechanics ( medic players )

	float lastTargetCoords[3];
} BotData;