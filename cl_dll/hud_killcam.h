#pragma once

// Returns true if a killcam is currently active
bool KillCam_IsActive();

// Returns the entity index of the attacker being observed, or 0 if none
int KillCam_GetAttackerIndex();

// Returns a 0..1 fraction of how far through the killcam we are
float KillCam_GetFraction();

void KillCam_Cancel();