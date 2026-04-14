//=============================================================================
// bot_think.cpp  –  External Bot_Think() dispatcher
//
// Your existing system iterates all client slots and calls Bot_Think() for
// every client.  Drop this file alongside bot_goap.cpp in dlls/.
//
// In your existing iteration loop, replace the call with:
//
//     Bot_Think( pEdict );
//
// or if you already have the edict loop structure, add the call as shown
// in the Bot_RunFrame() example at the bottom.
//=============================================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"
#include "tf_bot_goap.h"
#include "tf_bot_misc.h"

//-----------------------------------------------------------------------------
// Bot_Think
//
// Call this from your client-slot iterator for every occupied slot.
// It is a no-op for real (human) players and for bots that are dead /
// not yet fully spawned.
//
// Parameters:
//   pEdict  – the edict_t* for this client slot (as the engine gives you)
//-----------------------------------------------------------------------------
void Bot_Think(edict_t* pEdict)
{
    if (!pEdict || pEdict->free)
        return;

    // Only process fake-client (bot) slots
    if (!(pEdict->v.flags & FL_FAKECLIENT))
        return;

    CHL1Bot* pBot = BotFromEdict(pEdict);
    if (!pBot)
        return;

    pBot->BotThink();
}

//-----------------------------------------------------------------------------
// Bot_RunFrame
//
// Optional convenience wrapper: call this ONCE per server frame from
// StartFrame() (or wherever you drive your loop) and it will visit every
// connected client slot automatically.
//
// If you already have your own loop, just call Bot_Think(pEdict) inside it
// and ignore this function.
//-----------------------------------------------------------------------------
void Bot_RunFrame()
{
    for (int i = 1; i <= gpGlobals->maxClients; ++i)
    {
        edict_t* pEdict = INDEXENT(i);
        Bot_Think(pEdict);
    }
}

//=============================================================================
// Bot_AddBot  –  convenience: creates and spawns a new bot in the first
//               free client slot.
//
// Call from a console command handler or game-rules code, e.g.:
//
//     if (FStrEq(pcmd, "addbot"))
//         Bot_AddBot();
//=============================================================================
CHL1Bot* Bot_AddBot()
{
    // Find a free client slot (edict indices 1..maxClients are player slots)
    edict_t* pEdict = nullptr;
    for (int i = 1; i <= gpGlobals->maxClients; ++i)
    {
        edict_t* pSlot = INDEXENT(i);
        if (!pSlot || pSlot->free)
            continue;
        if (!(pSlot->v.flags & (FL_CLIENT | FL_FAKECLIENT)))
        {
            pEdict = pSlot;
            break;
        }
    }

    if (!pEdict)
    {
        ALERT(at_console, "[GOAP Bot] No free client slot available.\n");
        return nullptr;
    }

	const char* FakeNames[150] = {
		"System32",
		"Tom",
		"RogerThat",
		"Spybro",
		"botman",
		"YourGPUdeviceHasLost",
		"TeamForest",
		"Squ33z3",
		"quality",
		"bailoutnow",
		"ph34r",
		"1337sp34k",
		"soupMAN",
		"teke",
		"EngineerGaming",
		"Spyper",
		"benthas",
		"hmmmm",
		"LOLLOL",
		"pjkh",
		"sny",
		"demopan",
		"FREEDOOM!",
		"freeman",
		"alyx",
		"killer",
		"g-man",
		"xcellent",
		"nopera",
		"Moridin",
		"RankedTF2",
		"YoungGirl",
		"System64",
		"Creeper",
		"TheSameKiller",
		"F2PisAMAZING",
		"thinking?",
		"TheBest",
		"unicorn",
		"Pootis",
		"GoodFight",
		"bronze",
		"comeTOhere",
		"dead",
		"XxxBEASTxxX",
		"nope",
		"archive",
		"coach",
		"Steve",
		"notch",
		"forge",
		"herobrine",
		"liteXspy",
		"WEAREGREAT",
		"RickMay",
		"sniper",
		"AllyPlayer",
		"endernight",
		"fabric",
		"encore",
		"pocketPyro",
		"Pybro",
		"Kaboom",
		"PyroMaster",
		"Chucklenuts",
		"CryBaby",
		"WITCH",
		"ThatGuy",
		"Still Alive",
		"Hat-Wearing MAN",
		"Me",
		"Numnutz",
		"H@XX0RZ",
		"The G-Man",
		"Chell",
		"The Combine",
		"Totally Not A Bot",
		"Pow!",
		"Zepheniah Mann",
		"THEM",
		"LOS LOS LOS",
		"10001011101",
		"DeadHead",
		"ZAWMBEEZ",
		"MindlessElectrons",
		"TAAAAANK!",
		"The Freeman",
		"Black Mesa",
		"Soulless",
		"CEDA",
		"BeepBeepBoop",
		"NotMe",
		"CreditToTeam",
		"BoomerBile",
		"Someone Else",
		"Mann Co.",
		"Dog",
		"Kaboom!",
		"AmNot",
		"0xDEADBEEF",
		"HI THERE",
		"SomeDude",
		"GLaDOS",
		"Hostage",
		"Headful of Eyeballs",
		"CrySomeMore",
		"Aperture Science Prototype XR7",
		"Humans Are Weak",
		"AimBot",
		"C++",
		"GutsAndGlory!",
		"Nobody",
		"Saxton Hale",
		"RageQuit",
		"Screamin' Eagles",
		"Ze Ubermensch",
		"Maggot",
		"CRITRAWKETS",
		"Herr Doktor",
		"Gentlemanne of Leisure",
		"Companion Cube",
		"Target Practice",
		"One-Man Cheeseburger Apocalypse",
		"Crowbar",
		"Delicious Cake",
		"IvanTheSpaceBiker",
		"I LIVE!",
		"Cannon Fodder",
		"trigger_hurt",
		"Nom Nom Nom",
		"Divide by Zero",
		"GENTLE MANNE of LEISURE",
		"MoreGun",
		"Tiny Baby Man",
		"Big Mean Muther Hubbard",
		"Force of Nature",
		"Crazed Gunman",
		"Grim Bloody Fable",
		"Poopy Joe",
		"A Professional With Standards",
		"Freakin' Unbelievable",
		"SMELLY UNFORTUNATE",
		"The Administrator",
		"Mentlegen",
		"Archimedes!",
		"Ribs Grow Back",
		"It's Filthy in There!",
		"Mega Baboon",
		"Kill Me",
		"Glorified Toaster with Legs"};

	// The engine will validate the name and change it to "unnamed" if it's not valid.
	// Any duplicates will be disambiguated by prepending a (%d) where %d is a number.
	const char* name = FakeNames[RANDOM_LONG(0, 149)];

	if (!name)
		name = "Nullname";

    // Ask the engine to treat this slot as a fake client
    // (requires engine support; works with the HLDS / ReHLDS engine)
	edict_t* pFakeClient = (*g_engfuncs.pfnCreateFakeClient)(name);
    if (!pFakeClient)
    {
        ALERT(at_console, "[GOAP Bot] pfnCreateFakeClient failed.\n");
        return nullptr;
    }

    char reject[128];
	if (0 == ClientConnect(pFakeClient, STRING(pFakeClient->v.netname), "127.0.0.1", reject))
	{
		// Bot was refused connection, kick it from the server to free the slot.
		SERVER_COMMAND(UTIL_VarArgs("kick %s\n", STRING(pFakeClient->v.netname)));
		return nullptr;
	}

    // Mark and initialise
    pFakeClient->v.flags |= FL_FAKECLIENT;

    // Allocate our bot entity on the edict
    // Taken from ClientPutInServer
	CHL1Bot* pBot;
	entvars_t* pev = &pFakeClient->v;
	pBot = GetClassPtr((CHL1Bot*)pev);
	if (!pBot)
	{
		// Fall back to placement-new on the private data block if needed
		// (this mirrors how LINK_ENTITY_TO_CLASS works internally)
		ALERT(at_console, "[GOAP Bot] GET_PRIVATE failed.\n");
		return nullptr;
	}
	pBot->SetCustomDecalFrames(-1); // Assume none;
    pBot->pev = &pFakeClient->v;
	pBot->m_JoinMenuState = JOINMENUSTATE_YES;
	pBot->SetTeamNumber((RANDOM_LONG(0, 1) ? TEAM_RED : TEAM_BLUE));
	int chosenBotClass = CLASS_UNKNOWN;
	switch (RANDOM_LONG(1, 6))
	{
	case 1:
	{
		chosenBotClass = CLASS_SOLDIER;
		break;
	}
	case 2:
	{
		chosenBotClass = CLASS_ENGINEER;
		break;
	}
	case 3:
	{
		chosenBotClass = CLASS_SPY;
		break;
	}
	case 4:
	{
		chosenBotClass = CLASS_DEMOMAN;
		break;
	}
	case 5:
	{
		chosenBotClass = CLASS_HEAVY;
		break;
	}
	case 6:
	{
		chosenBotClass = CLASS_SOLDIER;
		break;
	}
	}

	chosenBotClass = CLASS_SOLDIER;
	pBot->m_iClass = pBot->m_iNewClass = chosenBotClass;
	pBot->pev->effects |= EF_NOINTERP;
	pBot->pev->iuser1 = 0; // disable any spec modes
	pBot->pev->iuser2 = 0;

	pBot->Spawn();

    ALERT(at_console, "[GOAP Bot] Spawned bot at slot %d.\n",
          ENTINDEX(pFakeClient));

    return pBot;
}
