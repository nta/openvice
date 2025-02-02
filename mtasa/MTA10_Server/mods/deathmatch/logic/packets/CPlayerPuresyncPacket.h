/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CPlayerPuresyncPacket.h
*  PURPOSE:     Player pure synchronization packet class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <>
*               Alberto Alonso <rydencillo@gmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CPLAYERPURESYNCPACKET_H
#define __CPLAYERPURESYNCPACKET_H

#include "CPacket.h"
#include "CPlayer.h"

class CPlayerPuresyncPacket : public CPacket
{
public:
    inline                  CPlayerPuresyncPacket           ( void )                        {};
    inline explicit         CPlayerPuresyncPacket           ( CPlayer* pPlayer )            { m_pSourceElement = pPlayer; };

    bool                            HasSimHandler           ( void ) const                  { return true; }
    inline ePacketID                GetPacketID             ( void ) const                  { return PACKET_ID_PLAYER_PURESYNC; };
    inline unsigned long            GetFlags                ( void ) const                  { return PACKET_MEDIUM_PRIORITY | PACKET_SEQUENCED; };

    bool                    Read                            ( NetBitStreamInterface& BitStream );
    bool                    Write                           ( NetBitStreamInterface& BitStream ) const;
};

#endif
