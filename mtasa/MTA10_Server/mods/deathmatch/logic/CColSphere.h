/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CColSphere.h
*  PURPOSE:     Sphere-shaped collision entity class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Kevin Whiteside <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CCOLSPHERE_H
#define __CCOLSPHERE_H

#include "CColShape.h"

class CColSphere : public CColShape
{
public:
                    CColSphere      ( CColManager* pManager, CElement* pParent, const CVector& vecPosition, float fRadius, CXMLNode* pNode = NULL, bool bIsPartnered = false );

    virtual CSphere         GetWorldBoundingSphere  ( void );

    eColShapeType   GetShapeType    ( void )            { return COLSHAPE_SPHERE; }

    bool            DoHitDetection  ( const CVector& vecNowPosition );

    float           GetRadius       ( void )            { return m_fRadius; };
    void            SetRadius       ( float fRadius )   { m_fRadius = fRadius; SizeChanged (); };

protected:
    bool            ReadSpecialData ( void );

    float           m_fRadius;
};

#endif
