/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/common/CBitStream.h
*  PURPOSE:     Network bitstream class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CBITSTREAM_H
#define __CBITSTREAM_H

#include "../CCommon.h"

class CBitStream
{
public:
    inline                          CBitStream  ( void )    { pBitStream = g_pNetServer->AllocateNetServerBitStream ( 0 ); };
    inline                          ~CBitStream ( void )    { g_pNetServer->DeallocateNetServerBitStream ( (NetBitStreamInterface*)pBitStream ); };

    NetBitStreamInterfaceNoVersion* pBitStream;
};

#endif
