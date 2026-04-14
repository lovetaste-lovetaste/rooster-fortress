//=============================================================================
// Team Selection Menu (VGUI1)
// Triggered by: team_selection_menu console command
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

// -----------------------------------------------------------------------
// CTeamSelectionMenu
// Inherits from CMenuPanel, the standard HL1 base for VGUI1 in-game menus.
// -----------------------------------------------------------------------
CTeamSelectionMenu::CTeamSelectionMenu(int iTrans, int iRemoveMe,
	int x, int y, int wide, int tall)
	: CMenuPanel(iTrans, iRemoveMe, x, y, wide, tall)
{
	// ----- Background Panel (centered, semi-transparent) -----
	const int PANEL_W = XRES(200);
	const int PANEL_H = YRES(160);
	const int PANEL_X = (ScreenWidth - PANEL_W) / 2;
	const int PANEL_Y = (ScreenHeight - PANEL_H) / 2;

	CTransparentPanel* pBg = new CTransparentPanel(160, PANEL_X, PANEL_Y, PANEL_W, PANEL_H);
	pBg->setParent(this);
	// setBorder(new LineBorder(Color(255 * 0.7, 170 * 0.7, 0, 0)));

	// ----- Title Label -----
	Label* pTitle = new Label(
		gHUD.m_TextMessage.BufferedLocaliseTextString("Select Team"),
		PANEL_X,
		PANEL_Y + YRES(8),
		PANEL_W,
		YRES(20));
	pTitle->setParent(this);
	pTitle->setContentAlignment(vgui::Label::a_center);

	// ----- Helper: button dimensions -----
	const int BTN_W = XRES(120);
	const int BTN_H = YRES(28);
	const int BTN_X = PANEL_X + (PANEL_W - BTN_W) / 2; // horizontally centred
	const int BTN_GAP = YRES(8);
	int btnY = PANEL_Y + YRES(40);

	// ----- RED button -----
	CommandButton* pRed = new CommandButton(
		gHUD.m_TextMessage.BufferedLocaliseTextString("RED"),
		BTN_X, btnY, BTN_W, BTN_H);
	pRed->setParent(this);
	pRed->addActionSignal(new CMenuHandler_StringCommand("jointeam red"));

	btnY += BTN_H + BTN_GAP;

	// ----- BLUE button -----
	CommandButton* pBlue = new CommandButton(
		gHUD.m_TextMessage.BufferedLocaliseTextString("BLUE"),
		BTN_X, btnY, BTN_W, BTN_H);
	pBlue->setParent(this);
	pBlue->addActionSignal(new CMenuHandler_StringCommand("jointeam blue"));

	btnY += BTN_H + BTN_GAP;

	// ----- SPEC button -----
	CommandButton* pSpec = new CommandButton(
		gHUD.m_TextMessage.BufferedLocaliseTextString("SPECTATOR"),
		BTN_X, btnY, BTN_W, BTN_H);
	pSpec->setParent(this);
	pSpec->addActionSignal(new CMenuHandler_StringCommand("jointeam spectator"));

	btnY += BTN_H + BTN_GAP;

	// ----- Cancel button (closes without sending a command) -----
	CommandButton* pCancel = new CommandButton(
		gHUD.m_TextMessage.BufferedLocaliseTextString("Cancel"),
		BTN_X, btnY, BTN_W, BTN_H);
	pCancel->setParent(this);
	// HIDE_TEXTWINDOW is the standard Unified SDK signal to close the active panel
	pCancel->addActionSignal(new CMenuHandler_TextWindow(HIDE_TEXTWINDOW));
}