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

typedef struct
{
	bool Enabled;

	CBaseEntity* Targets[2];
	
	float timeSpentUnderwater;
	float BaseReactionTime;
	float LastTimeReacted;
	float AwarenessOfClient[TF_MAXPLAYERS];
	float lastTargetCoords[3];
} BotData;