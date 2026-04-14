/***
 *
 *   hud_killcam.cpp
 *
 *   TF2-style killcam HUD element.
 *
 *   NOTE on message wiring:
 *     __MsgFunc_KillCam is already manually defined in hud.cpp (pointing to
 *     gHUD.m_Killcam) and HOOK_MESSAGE(KillCam) is already called from
 *     CHud::Init() in hud.cpp.
 *
 *     Therefore this file must NOT contain DECLARE_MESSAGE or HOOK_MESSAGE —
 *     doing so creates a second definition of __MsgFunc_KillCam, which is the
 *     LNK2005 "already defined" linker error.
 *
 *     CHudKillCam::Init() only sets the active flag and registers itself in
 *     the HUD draw list. Nothing else.
 *
 ****/

#include "hud.h"
#include "cl_util.h"  // ConsoleStringLen, FillRGBA, ScreenWidth, ScreenHeight
#include "parsemsg.h" // BEGIN_READ, READ_BYTE, READ_FLOAT
#include "hud_killcam.h"

// -----------------------------------------------------------------------
//  Internal state
// -----------------------------------------------------------------------
namespace
{
struct KillCamState
{
	bool active = false;
	int attackerEntIdx = 0;
	float startTime = 0.f;
	float endTime = 0.f;
	float duration = 0.f;
};

KillCamState g_KillCam;
}

// -----------------------------------------------------------------------
//  Public accessors used by view.cpp, hud_redraw.cpp, hud_msg.cpp
// -----------------------------------------------------------------------
bool KillCam_IsActive()
{
	return g_KillCam.active && gHUD.m_flTime < g_KillCam.endTime;
}

int KillCam_GetAttackerIndex()
{
	return g_KillCam.active ? g_KillCam.attackerEntIdx : 0;
}

float KillCam_GetFraction()
{
	if (!g_KillCam.active || g_KillCam.duration <= 0.f)
		return 0.f;
	return (gHUD.m_flTime - g_KillCam.startTime) / g_KillCam.duration;
}

void KillCam_Cancel()
{
	g_KillCam.active = false;
}

// -----------------------------------------------------------------------
//  CHudKillCam
// -----------------------------------------------------------------------

// DO NOT add DECLARE_MESSAGE here — __MsgFunc_KillCam is already defined
// manually in hud.cpp and HOOK_MESSAGE(KillCam) is already in CHud::Init().
// A second DECLARE_MESSAGE produces LNK2005.

bool CHudKillCam::Init()
{
	// DO NOT call HOOK_MESSAGE(KillCam) here — already done in CHud::Init().
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);
	return true;
}

bool CHudKillCam::VidInit()
{
	return true;
}

// Called via __MsgFunc_KillCam defined in hud.cpp -> gHUD.m_Killcam.MsgFunc_KillCam
int CHudKillCam::MsgFunc_KillCam(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int attackerIdx = READ_BYTE();
	float dur = READ_FLOAT();

	if (attackerIdx <= 0 || attackerIdx > gEngfuncs.GetMaxClients())
		return 1;

	g_KillCam.active = true;
	g_KillCam.attackerEntIdx = attackerIdx;
	g_KillCam.startTime = gHUD.m_flTime;
	g_KillCam.endTime = gHUD.m_flTime + dur;
	g_KillCam.duration = dur;

	return 1;
}

// -----------------------------------------------------------------------
//  Draw
// -----------------------------------------------------------------------
bool CHudKillCam::Draw(float flTime)
{
	if (g_KillCam.active && flTime >= g_KillCam.endTime)
	{
		g_KillCam.active = false;
		return true;
	}

	if (!g_KillCam.active)
		return true;

	const int SW = ScreenWidth;
	const int SH = ScreenHeight;
	const int BAR_H = 56;

	FillRGBA(0, 0, SW, BAR_H, 0, 0, 0, 210);
	FillRGBA(0, SH - BAR_H, SW, BAR_H, 0, 0, 0, 210);

	// ConsoleStringLen() is in cl_util.h — correct width query for this SDK.
	// gHUD.GetHudStringWidth does not exist in halflife-updated.
	const char* pLabel = "KILLED BY";
	int labelW = ConsoleStringLen(pLabel);
	int labelX = (SW - labelW) / 2;

	// gHUD.DrawHudString is a method on CHud, not a free function.
	gHUD.DrawHudString(labelX, 14, SW, pLabel, 220, 60, 60);

	hud_player_info_t info;
	memset(&info, 0, sizeof(info));
	gEngfuncs.pfnGetPlayerInfo(g_KillCam.attackerEntIdx, &info);

	if (info.name && info.name[0] != '\0')
	{
		int nameW = ConsoleStringLen(info.name);
		int nameX = (SW - nameW) / 2;
		gHUD.DrawHudString(nameX, 32, SW, info.name, 255, 200, 80);
	}

	float elapsed = flTime - g_KillCam.startTime;
	float fraction = 1.0f - (elapsed / g_KillCam.duration);
	if (fraction < 0.f)
		fraction = 0.f;
	if (fraction > 1.f)
		fraction = 1.f;

	int barY = SH - BAR_H + 8;
	int barMaxW = SW - 80;
	int barW = (int)(fraction * (float)barMaxW);

	FillRGBA(40, barY, barMaxW, 6, 60, 60, 60, 180);
	FillRGBA(40, barY, barW, 6, 200, 50, 50, 220);

	int secsLeft = (int)((g_KillCam.endTime - flTime) + 0.999f);
	char secsBuf[8];
	snprintf(secsBuf, sizeof(secsBuf), "%ds", secsLeft);
	gHUD.DrawHudString(SW / 2 - 10, SH - BAR_H + 20, SW, secsBuf, 180, 180, 180);

	return true;
}
