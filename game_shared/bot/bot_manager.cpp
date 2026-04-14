//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Author: Michael S. Booth (mike@turtlerockstudios.com), 2003

#pragma warning( disable : 4530 )					// STL uses exceptions, but we are not compiling with them - ignore warning

#define DEFINE_EVENT_NAMES

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "soundent.h"
#include "gamerules.h"
#include "player.h"
#include "client.h"
#include "perf_counter.h"

#include "bot.h"
#include "bot_manager.h"
#include "nav_area.h"
#include "bot_util.h"

const float smokeRadius = 115.0f;		///< for smoke grenades


//#define CHECK_PERFORMANCE
#ifdef CHECK_PERFORMANCE
	// crude performance timing
	static CPerformanceCounter perfCounter;

	struct PerfInfo
	{
		float frameTime;
		float botThinkTime;
	};

	#define MAX_PERF_DATA 50000
	static PerfInfo perfData[ MAX_PERF_DATA ];
	static int perfDataCount = 0;
	static int perfFileIndex = 0;
#endif



/**
 * Convert name to GameEventType
 * @todo Find more appropriate place for this function
 */
GameEventType NameToGameEvent( const char *name )
{
	for( int i=0; GameEventName[i]; ++i )
		if (!stricmp( GameEventName[i], name ))
			return static_cast<GameEventType>( i );

	return EVENT_INVALID;
}


//--------------------------------------------------------------------------------------------------------------
CBotManager::CBotManager()
{
	InitBotTrig();
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked when the round is restarting
 */
void CBotManager::RestartRound( void )
{
#ifdef CHECK_PERFORMANCE
	// dump previous round's performance
	char filename[80];
	sprintf( filename, "perfdata%02X.txt", perfFileIndex++ );
	FILE *fp = fopen( filename, "w" );

	if (fp)
	{
		for( int p=0; p<perfDataCount; ++p )
			fprintf( fp, "%f\t%f\n", perfData[p].frameTime, perfData[p].botThinkTime );

		fclose( fp );
	}
		
	perfDataCount = 0;
#endif
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked at the start of each frame
 */
void CBotManager::StartFrame( void )
{
#ifdef CHECK_PERFORMANCE
	static double lastTime = 0.0f;
	double startTime = perfCounter.GetCurTime();
#endif

	for( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex( i );

		if (!pPlayer)
			continue;

		if (pPlayer->IsFakeClient() && IsEntityValid( pPlayer ))
		{
			CBot *pBot = static_cast<CBot *>( pPlayer );

			pBot->BotThink();
		}
	}

#ifdef CHECK_PERFORMANCE
	if (perfDataCount < MAX_PERF_DATA)
	{
		if (lastTime > 0.0f)
		{
			double endTime = perfCounter.GetCurTime();

			perfData[ perfDataCount ].frameTime = (float)(startTime - lastTime);
			perfData[ perfDataCount ].botThinkTime = (float)(endTime - startTime);
			++perfDataCount;
		}

		lastTime = startTime;
	}
#endif
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Return the filename for this map's "nav map" file
 */
const char *CBotManager::GetNavMapFilename( void ) const
{
	static char filename[256];
	sprintf( filename, "maps\\%s.nav", STRING( gpGlobals->mapname ) );
	return filename;
}

//--------------------------------------------------------------------------------------------------------------
/**
 * Invoked when given player does given event (some events have NULL player).
 * Events are propogated to all bots.
 *
 * @todo This has become the game-wide event dispatcher. We should restructure this.
 */
void CBotManager::OnEvent(GameEventType event, CBaseEntity* entity, CBaseEntity* other)
{
	// propogate event to all bots
	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer* player = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));

		if (player == NULL)
			continue;

		if (FNullEnt(player->pev))
			continue;

		if (FStrEq(STRING(player->pev->netname), ""))
			continue;

		if (!player->IsFakeClient())
			continue;

		// do not send self-generated event
		if (entity == player)
			continue;

		CBot* bot = static_cast<CBot*>(player);
		bot->OnEvent(event, entity, other);
	}
}

bool CBotManager::IsInsideSmokeCloud( const Vector *pos )
{
	return false;
}

bool CBotManager::IsLineBlockedBySmoke( const Vector *from, const Vector *to )
{
	return false;
}
