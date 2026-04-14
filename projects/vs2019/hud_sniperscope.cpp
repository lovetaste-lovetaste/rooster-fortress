#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE(m_SniperScope, SniperScope)

bool CHudSniperScope::Init()
{
	m_bScoped = false;
	m_iCharge = 0;
	HOOK_MESSAGE(SniperScope);
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem(this);
	return 1;
}

bool CHudSniperScope::VidInit()
{
	m_hScopeSprite = LoadSprite("sprites/rooster_fortress/sniperscope.spr");
	m_pCvarCrosshair = gEngfuncs.pfnGetCvarPointer("crosshair");
	return 1;
}

int CHudSniperScope::MsgFunc_SniperScope(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_bScoped = READ_BYTE() != 0;
	m_iCharge = READ_BYTE(); // 0-100
	if (!m_bScoped && m_pCvarCrosshair)
		m_pCvarCrosshair->value = 1;
	return 1;
}

bool CHudSniperScope::Draw(float flTime)
{
	if (!m_bScoped)
		return 1;

	SCREENINFO si;
	si.iSize = sizeof(si);
	gEngfuncs.pfnGetScreenInfo(&si);
	int scrW = si.iWidth;
	int scrH = si.iHeight;

	if (gEngfuncs.pfnGetScreenInfo)
	{
		gEngfuncs.pfnGetScreenInfo(&si); // Unified SDK style
		scrW = si.iWidth;
		scrH = si.iHeight;
	}
	else
		return 1;

	int cx = scrW / 2;
	int cy = scrH / 2;

	// --- Draw scope vignette (4 black rectangles around a circle) ---
	int radius = (int)(scrH * 0.42f); // scope circle radius

	gEngfuncs.pfnFillRGBA(0, 0, scrW, cy - radius, 0, 0, 0, 255);
	gEngfuncs.pfnFillRGBA(0, cy + radius, scrW, scrH - (cy + radius), 0, 0, 0, 255);
	gEngfuncs.pfnFillRGBA(0, cy - radius, cx - radius, radius * 2, 0, 0, 0, 255);
	gEngfuncs.pfnFillRGBA(cx + radius, cy - radius, scrW - (cx + radius), radius * 2, 0, 0, 0, 255);

//	if (m_hScopeSprite)
//	{
	//	int sprW = radius * 2;
	//	int sprH = radius * 2;
	//	int sprX = cx - radius;
	//	int sprY = cy - radius;
//		int sprW = SPR_Width(m_hScopeSprite, 0);
//		int sprH = SPR_Height(m_hScopeSprite, 0);

//		gEngfuncs.pfnSPR_Set(m_hScopeSprite, 255, 255, 255);
		
//		gEngfuncs.pfnSPR_DrawHoles(0, cx - (sprW / 2), cy - (sprH / 2), nullptr);
//	}

	// --- Crosshair lines ---
	int lineLen = radius - 4;
	int lineThick = 2;
	gEngfuncs.pfnFillRGBA(cx - lineLen, cy - lineThick / 2, lineLen - 4, lineThick, 0, 0, 0, 255); // left
	gEngfuncs.pfnFillRGBA(cx + 4, cy - lineThick / 2, lineLen - 4, lineThick, 0, 0, 0, 255);	   // right
	gEngfuncs.pfnFillRGBA(cx - lineThick / 2, cy - lineLen, lineThick, lineLen - 4, 0, 0, 0, 255); // top
	gEngfuncs.pfnFillRGBA(cx - lineThick / 2, cy + 4, lineThick, lineLen - 4, 0, 0, 0, 255);	   // bottom

	// --- Charge meter (left side of scope, vertical bar) ---
	int meterX = cx - radius + 12;
	int meterH = radius;
	int meterTop = cy - meterH / 2;
	int filled = (int)(meterH * (m_iCharge / 100.0f));

	// Background
	FillRGBA(meterX, meterTop, 6, meterH, 50, 50, 50, 180);
	// Fill — colour shifts red->yellow->green as charge increases
	int r = (int)(255 * (1.0f - m_iCharge / 100.0f));
	int g = (int)(255 * (m_iCharge / 100.0f));
	FillRGBA(meterX, meterTop + meterH - filled, 6, filled, r, g, 0, 220);

	// Hide engine crosshair while scoped
	if (m_pCvarCrosshair)
		m_pCvarCrosshair->value = 0;

	return 1;
}