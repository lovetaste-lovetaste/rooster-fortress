//=============================================================================
// player_cmodel_patch.h
// Exact changes required in CBasePlayer (player.h / player.cpp) to integrate
// the c_model viewmodel manager.  Apply as a patch or inline directly.
//=============================================================================

//-----------------------------------------------------------------------------
// [player.h]  Add inside the CBasePlayer class declaration:
//-----------------------------------------------------------------------------
/*

#include "sv_viewmodel.h"

class CBasePlayer : public CBaseMonster
{
    // ... existing members ...

public:
    // ---- c_model viewmodel manager ----
    CCModelViewModelManager& GetViewModelManager() { return m_ViewModelManager; }

private:
    CCModelViewModelManager m_ViewModelManager;
};

*/

//-----------------------------------------------------------------------------
// [player.cpp]  CBasePlayer::Spawn  — add AFTER the existing body:
//-----------------------------------------------------------------------------
/*

void CBasePlayer::Spawn()
{
    // ... existing spawn code ...

    // Initialise the viewmodel manager and create both viewmodel entities.
    m_ViewModelManager.Init( this );
}

*/

//-----------------------------------------------------------------------------
// [player.cpp]  CBasePlayer::Killed  — add at the top:
//-----------------------------------------------------------------------------
/*

void CBasePlayer::Killed( entvars_t* pevAttacker, int iGib )
{
    // Clean up viewmodel entities before the player is removed.
    m_ViewModelManager.Shutdown();

    // ... rest of existing code ...
}

*/

//-----------------------------------------------------------------------------
// [player.cpp]  LinkUserMessages()  — add one line:
//-----------------------------------------------------------------------------
/*

void LinkUserMessages()
{
    // ... existing messages ...
    RegisterCModelMessages();      // <-- add this
}

*/

//-----------------------------------------------------------------------------
// [client.cpp / hl_weapons.cpp]  Hook the arms-index message:
//-----------------------------------------------------------------------------
/*

void HUD_Init()
{
    // ... existing hooks ...
    gEngfuncs.pfnHookUserMsg( "CModelArms", MsgFunc_CModelArms );
}

*/

//-----------------------------------------------------------------------------
// [view.cpp]  V_CalcRefdef  — update both viewmodel slots each frame:
//-----------------------------------------------------------------------------
/*

void V_CalcRefdef( ref_params_t* pparams )
{
    // ... existing code ...

    // Update the c_model viewmodel manager so bob/sway are applied.
    g_ViewModelManager.UpdateViewModels( *pparams );
}

*/

//-----------------------------------------------------------------------------
// [StudioModelRenderer.cpp]  CStudioModelRenderer::StudioCalcAttachments
//  or wherever viewmodel rendering is set up — override the entity pointers:
//-----------------------------------------------------------------------------
/*

void CStudioModelRenderer::DrawViewModel()
{
    cl_entity_t* pWeapon = nullptr;
    cl_entity_t* pArms   = nullptr;

    // Let the manager substitute its post-processed copies.
    g_ViewModelManager.OverrideViewModelEntities( &pWeapon, &pArms );

    // Render weapon slot first (behind the arms).
    if ( pWeapon && pWeapon->model )
    {
        m_pCurrentEntity = pWeapon;
        StudioRenderModel();
    }

    // Render arms slot on top.
    if ( pArms && pArms->model )
    {
        m_pCurrentEntity = pArms;
        StudioRenderModel();
    }
}

*/
