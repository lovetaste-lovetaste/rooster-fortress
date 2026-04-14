//=============================================================================
// Class Selection Menu (VGUI1)
// Triggered by: class_selection_menu console command
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

CClassSelectionMenu::CClassSelectionMenu(int iTrans, int iRemoveMe,
	int x, int y, int wide, int tall)
	: CMenuPanel(iTrans, iRemoveMe, x, y, wide, tall)
{
	// ----- Layout constants -----
	const int PANEL_W = XRES(220);
	const int PANEL_H = YRES(330); // tall enough for 10 buttons
	const int PANEL_X = (ScreenWidth - PANEL_W) / 2;
	const int PANEL_Y = (ScreenHeight - PANEL_H) / 2;
	const int BTN_W = XRES(160);
	const int BTN_H = YRES(24);
	const int BTN_X = PANEL_X + (PANEL_W - BTN_W) / 2;
	const int BTN_GAP = YRES(4);

	// ----- Background -----
	CTransparentPanel* pBg = new CTransparentPanel(160, PANEL_X, PANEL_Y, PANEL_W, PANEL_H);
	pBg->setParent(this);

	// ----- Title -----
	Label* pTitle = new Label(
		gHUD.m_TextMessage.BufferedLocaliseTextString("Select Class"),
		PANEL_X,
		PANEL_Y + YRES(8),
		PANEL_W,
		YRES(20));
	pTitle->setParent(this);
	pTitle->setContentAlignment(vgui::Label::a_center);

	// ----- Helper struct to avoid repeating the button setup -----
	struct ClassEntry
	{
		const char* label;
		const char* command; // full string sent to server
	};

	// All 9 classes + Random, in classic TFC order
	static const ClassEntry classes[] =
		{
			{"Scout", "joinclass scout"},
			{"Soldier", "joinclass soldier"},
			{"Pyro", "joinclass pyro"},
			{"Demoman", "joinclass demoman"},
			{"Heavy Weapons Guy", "joinclass heavy"},
			{"Engineer", "joinclass engineer"},
			{"Medic", "joinclass medic"},
			{"Sniper", "joinclass sniper"},
			{"Spy", "joinclass spy"},
			{"Random", "joinclass random"},
		};

	int btnY = PANEL_Y + YRES(36);

	for (const auto& cls : classes)
	{
		CommandButton* pBtn = new CommandButton(
			gHUD.m_TextMessage.BufferedLocaliseTextString(cls.label),
			BTN_X, btnY, BTN_W, BTN_H);
		pBtn->setParent(this);
		pBtn->addActionSignal(new CMenuHandler_StringCommand(cls.command));
		btnY += BTN_H + BTN_GAP;
	}

	// ----- Cancel button -----
	btnY += BTN_GAP; // a little extra space before Cancel
	CommandButton* pCancel = new CommandButton(
		gHUD.m_TextMessage.BufferedLocaliseTextString("Cancel"),
		BTN_X, btnY, BTN_W, BTN_H);
	pCancel->setParent(this);
	pCancel->addActionSignal(new CMenuHandler_TextWindow(HIDE_TEXTWINDOW));
}