/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientWaterManager.cpp
*  PURPOSE:     Water entity manager class
*  DEVELOPERS:  arc_
*
*****************************************************************************/

#include "StdInc.h"

using std::list;

CClientWaterManager::CClientWaterManager ( CClientManager* pManager )
{
    m_pManager = pManager;
    m_bDontRemoveFromList = false;
}


CClientWaterManager::~CClientWaterManager ( void )
{
    DeleteAll ();
}


CClientWater* CClientWaterManager::Get ( ElementID ID )
{
    // Grab the element with the given id. Check its type.
    CClientEntity* pEntity = CElementIDs::GetElement ( ID );
    if ( pEntity && pEntity->GetType () == CCLIENTWATER )
    {
        return static_cast < CClientWater* > ( pEntity );
    }

    return NULL;
}


void CClientWaterManager::DeleteAll ( void )
{
    // Delete each water poly
    m_bDontRemoveFromList = true;
    list < CClientWater* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        delete *iter;
    }

    m_bDontRemoveFromList = false;

    // Clear the list
    m_List.clear ();
}


bool CClientWaterManager::Exists ( CClientWater* pWater )
{
    list < CClientWater* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end () ; iter++ )
    {
        if ( *iter == pWater )
        {
            return true;
        }
    }
    return false;
}

void CClientWaterManager::AddToList ( CClientWater* pWater )
{
    m_List.push_back ( pWater );
}

void CClientWaterManager::RemoveFromList ( CClientWater* pWater )
{
    if ( !m_bDontRemoveFromList )
    {
        if ( !m_List.empty() ) m_List.remove ( pWater );
    }
}

bool CClientWaterManager::GetWaterLevel ( CVector& vecPosition, float* pfLevel, bool bCheckWaves, CVector* pvecUnknown )
{
    return g_pGame->GetWaterManager ()->GetWaterLevel ( vecPosition, pfLevel, bCheckWaves, pvecUnknown );
}

bool CClientWaterManager::SetPositionWaterLevel ( const CVector& vecPosition, float fLevel, void* pChangeSource )
{
    return g_pGame->GetWaterManager ()->SetPositionWaterLevel ( vecPosition, fLevel, pChangeSource );
}

bool CClientWaterManager::SetWorldWaterLevel ( float fLevel, void* pChangeSource, bool bIncludeWorldNonSeaLevel )
{
    return g_pGame->GetWaterManager ()->SetWorldWaterLevel ( fLevel, pChangeSource, bIncludeWorldNonSeaLevel );
}

bool CClientWaterManager::SetElementWaterLevel ( CClientWater* pWater, float fLevel, void* pChangeSource )
{
    // Not calling CClientWater::SetPosition as x and y are not changing so the poly zone will not change
    return g_pGame->GetWaterManager ()->SetPolyWaterLevel ( pWater->m_pPoly, fLevel, pChangeSource );
}

bool CClientWaterManager::SetAllElementWaterLevel ( float fLevel, void* pChangeSource )
{
    list < CClientWater* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        SetElementWaterLevel ( *iter, fLevel, pChangeSource );
    }
    return true;
}


void CClientWaterManager::ResetWorldWaterLevel ( void )
{
    g_pGame->GetWaterManager ()->ResetWorldWaterLevel ();
}

float CClientWaterManager::GetWaveLevel ()
{
    return g_pGame->GetWaterManager ()->GetWaveLevel ();
}

void CClientWaterManager::SetWaveLevel ( float fWaveLevel )
{
    g_pGame->GetWaterManager ()->SetWaveLevel ( fWaveLevel );
}
