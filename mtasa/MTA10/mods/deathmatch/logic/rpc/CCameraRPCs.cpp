/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/rpc/CCameraRPCs.cpp
*  PURPOSE:     Camera remote procedure calls
*  DEVELOPERS:  Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "CCameraRPCs.h"

void CCameraRPCs::LoadFunctions ( void )
{
    AddHandler ( SET_CAMERA_MATRIX, SetCameraMatrix, "SetCameraMatrix" );
    AddHandler ( SET_CAMERA_TARGET, SetCameraTarget, "SetCameraTarget" );
    AddHandler ( SET_CAMERA_INTERIOR, SetCameraInterior, "SetCameraInterior" );
    AddHandler ( FADE_CAMERA, FadeCamera, "FadeCamera" );
}


void CCameraRPCs::SetCameraMatrix ( NetBitStreamInterface& bitStream )
{
    CVector vecPosition, vecLookAt;
    float fRoll = 0.0f;
    float fFOV = 70.0f;
    if ( bitStream.Read ( vecPosition.fX ) &&
         bitStream.Read ( vecPosition.fY ) &&
         bitStream.Read ( vecPosition.fZ ) &&
         bitStream.Read ( vecLookAt.fX ) &&
         bitStream.Read ( vecLookAt.fY ) &&
         bitStream.Read ( vecLookAt.fZ ) )
    {
        bitStream.Read ( fRoll );
        bitStream.Read ( fFOV );

        if ( !m_pCamera->IsInFixedMode () )
            m_pCamera->ToggleCameraFixedMode ( true );

        // Put the camera there
        m_pCamera->SetPosition ( vecPosition );
        m_pCamera->SetFixedTarget ( vecLookAt );
        m_pCamera->SetRoll ( fRoll );
        m_pCamera->SetFOV ( fFOV );
    }
}

void CCameraRPCs::SetCameraTarget ( NetBitStreamInterface& bitStream )
{
    ElementID targetID;
    if ( bitStream.Read ( targetID ) )
    {
        CClientEntity * pEntity = CElementIDs::GetElement ( targetID );
        if ( pEntity )
        {
            // Save our current target for later
            CClientEntity * pPreviousTarget = m_pCamera->GetTargetEntity ();

            switch ( pEntity->GetType () )
            {
                case CCLIENTPLAYER:
                {
                    CClientPlayer* pPlayer = static_cast < CClientPlayer* > ( pEntity );
                    if ( pPlayer->IsLocalPlayer () )
                    {
                        // Return the focus to the local player
                        m_pCamera->SetFocusToLocalPlayer ();
                    }
                    else
                    {
                        // Put the focus on that player
                        m_pCamera->SetFocus ( pPlayer, MODE_BEHINDCAR, false );
                    }
                    break;
                }
                default:
                    return;
            }
        }
    }
}


void CCameraRPCs::SetCameraInterior ( NetBitStreamInterface& bitStream )
{
    // Read out the camera mode
    unsigned char ucInterior;
    if ( bitStream.Read ( ucInterior ) )
    {
        g_pGame->GetWorld ()->SetCurrentArea ( ucInterior );
    }
}


void CCameraRPCs::FadeCamera ( NetBitStreamInterface& bitStream )
{
    unsigned char ucFadeIn;
    float fFadeTime = 1.0f;
    if ( bitStream.Read ( ucFadeIn ) &&
         bitStream.Read ( fFadeTime ) )
    {
        g_pClientGame->SetInitiallyFadedOut ( false );

        if ( ucFadeIn )
        {
            m_pCamera->FadeIn ( fFadeTime );
            g_pGame->GetHud ()->SetComponentVisible ( HUD_AREA_NAME, !g_pClientGame->GetHudAreaNameDisabled () );
        }
        else
        {
            unsigned char ucRed = 0;
            unsigned char ucGreen = 0;
            unsigned char ucBlue = 0;
            
            if ( bitStream.Read ( ucRed ) &&
                 bitStream.Read ( ucGreen ) &&
                 bitStream.Read ( ucBlue ) )
            {
                m_pCamera->FadeOut ( fFadeTime, ucRed, ucGreen, ucBlue );
                g_pGame->GetHud ()->SetComponentVisible ( HUD_AREA_NAME, false );
            }
        }
    }
}
