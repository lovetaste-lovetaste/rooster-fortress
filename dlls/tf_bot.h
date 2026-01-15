void Bot_Think(int client);
void Bot_RunAll();
void Bot_AliveThink(int client, Vector* vecAngles, Vector* vecMove);
void Bot_AliveMovementThink(int client, Vector* vecAngles, Vector* vecMove);
void Bot_AliveWeaponThink(int client, Vector* vecAngles, Vector* vecMove);
void Bot_DeadThink(int client, Vector* vecAngles, Vector* vecMove);
CBaseEntity* FindNearestEnemy(int client, float MAXRANGE);
void TF2_LookAtPos(int client, Vector* vecAngles, float flGoal[3], float flAimSpeed);
void AimAtTarget(int client, CBaseEntity* pTarget);
float clamp(float main, float min, float max);
float AngleNormalize(float angle);
bool IsTargetVisible(int client, CBaseEntity* pTarget);

void BotMatchFacing(CBasePlayer* pBot, const Vector& v_source, Vector v_focus);
void BotFixIdealPitch(edict_t* pEdict);
void BotFixIdealYaw(edict_t* pEdict);