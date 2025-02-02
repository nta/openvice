/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CColTube.cpp
*  PURPOSE:     Tube-shaped collision entity class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Kevin Whiteside <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CColTube::CColTube ( CColManager* pManager, CElement* pParent, const CVector& vecPosition, float fRadius, float fHeight, CXMLNode* pNode ) : CColShape ( pManager, pParent, pNode )
{
    m_vecPosition = vecPosition;
    m_fRadius = fRadius;
    m_fHeight = fHeight;
    UpdateSpatialData ();
}


bool CColTube::DoHitDetection ( const CVector& vecNowPosition )
{
    // FIXME: What about radius in height?

    // First see if we're within the circle. Then see if we're within its height
    return ( IsPointNearPoint2D ( vecNowPosition, m_vecPosition, m_fRadius ) &&
             vecNowPosition.fZ >= m_vecPosition.fZ &&
             vecNowPosition.fZ <= m_vecPosition.fZ + m_fHeight );
}


bool CColTube::ReadSpecialData ( void )
{
    int iTemp;
    if ( GetCustomDataInt ( "dimension", iTemp, true ) )
        m_usDimension = static_cast < unsigned short > ( iTemp );

    GetCustomDataFloat ( "radius", m_fRadius, true );
    GetCustomDataFloat ( "height", m_fHeight, true );

    return true;
}


CSphere CColTube::GetWorldBoundingSphere ( void )
{
    CSphere sphere;
    sphere.vecPosition.fX = m_vecPosition.fX;
    sphere.vecPosition.fY = m_vecPosition.fY;
    sphere.vecPosition.fZ = m_vecPosition.fZ + m_fHeight * 0.5f;
    sphere.fRadius = Max ( m_fRadius, m_fHeight * 0.5f );
    return sphere;
}
