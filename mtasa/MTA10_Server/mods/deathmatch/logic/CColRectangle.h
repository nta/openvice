/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CColRectangle.h
*  PURPOSE:     Rectangle-shaped collision entity class
*  DEVELOPERS:  Kevin Whiteside <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CCOLRECTANGLE_H
#define __CCOLRECTANGLE_H

#include "CColShape.h"
#include <CVector2D.h>

class CColRectangle : public CColShape
{
public:
                            CColRectangle   ( CColManager* pManager, CElement* pParent, const CVector& vecPosition, const CVector2D& vecSize, CXMLNode* pNode = NULL );

    virtual CSphere         GetWorldBoundingSphere  ( void );

    eColShapeType           GetShapeType    ( void )                        { return COLSHAPE_RECTANGLE; }
    
    bool                    DoHitDetection  ( const CVector& vecNowPosition );

    inline const CVector2D& GetSize         ( void )                        { return m_vecSize; };
    inline void             SetSize         ( const CVector2D& vecSize )    { m_vecSize = vecSize; SizeChanged (); };

protected:
    bool                    ReadSpecialData ( void );

    CVector2D               m_vecSize;
};

#endif
