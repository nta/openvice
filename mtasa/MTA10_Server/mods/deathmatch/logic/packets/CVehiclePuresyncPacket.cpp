/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/packets/CVehiclePuresyncPacket.cpp
*  PURPOSE:     Vehicle pure synchronization packet class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "net/SyncStructures.h"

extern CGame* g_pGame;

bool CVehiclePuresyncPacket::Read ( NetBitStreamInterface& BitStream )
{
    // Got a player to read?
    if ( m_pSourceElement )
    {
        CPlayer * pSourcePlayer = static_cast < CPlayer * > ( m_pSourceElement );

        // Player is in a vehicle?
        CVehicle* pVehicle = pSourcePlayer->GetOccupiedVehicle ();
        if ( pVehicle )
        {
            // Read out the time context
            unsigned char ucTimeContext = 0;
            if ( !BitStream.Read ( ucTimeContext ) )
                return false;

            // Only read this packet if it matches the current time context that
            // player is in.
            if ( !pSourcePlayer->CanUpdateSync ( ucTimeContext ) )
            {
                return false;
            }

            // Read out the keysync data
            CControllerState ControllerState;
            if ( !ReadFullKeysync ( ControllerState, BitStream ) )
                return false;

            // Read out its position
            SPositionSync position ( false );
            if ( !BitStream.Read ( &position ) )
                return false;
            pSourcePlayer->SetPosition ( position.data.vecPosition );

            // Read the camera orientation
            CVector vecCamPosition, vecCamFwd;
            ReadCameraOrientation ( position.data.vecPosition, BitStream, vecCamPosition, vecCamFwd );
            pSourcePlayer->SetCameraOrientation ( vecCamPosition, vecCamFwd );

            // Jax: don't allow any outdated packets through
            SOccupiedSeatSync seat;
            if ( !BitStream.Read ( &seat ) )
                return false;
            if ( seat.data.ucSeat != pSourcePlayer->GetOccupiedVehicleSeat () )
            {
                // Mis-matching seats can happen when we warp into a different one,
                // which will screw up the whole packet
                return false;
            }

            // Read out the vehicle matrix only if he's the driver
            unsigned int uiSeat = pSourcePlayer->GetOccupiedVehicleSeat ();
            if ( uiSeat == 0 )
            {
                // Read out the vehicle rotation in degrees
                SRotationDegreesSync rotation;
                if( !BitStream.Read ( &rotation ) )
                    return false;

                // Set it
                pVehicle->SetPosition ( position.data.vecPosition );
                pVehicle->SetRotationDegrees ( rotation.data.vecRotation );

                // Move speed vector
                SVelocitySync velocity;
                if ( !BitStream.Read ( &velocity ) )
                    return false;

                pVehicle->SetVelocity ( velocity.data.vecVelocity );
                pSourcePlayer->SetVelocity ( velocity.data.vecVelocity );

                // Turn speed vector
                SVelocitySync turnSpeed;
                if ( !BitStream.Read ( &turnSpeed ) )
                    return false;

                pVehicle->SetTurnSpeed ( turnSpeed.data.vecVelocity );

                // Health
                SVehicleHealthSync health;
                if ( !BitStream.Read ( &health ) )
                    return false;

                float fPreviousHealth = pVehicle->GetLastSyncedHealth ( );
                float fHealth = health.data.fValue;

                // Less than last time?
                if ( fHealth < fPreviousHealth )
                {                 
                    // Grab the delta health
                    float fDeltaHealth = fPreviousHealth - fHealth;

                    if ( fDeltaHealth > 0.0f )
                    {
                        // Call the onVehicleDamage event
                        CLuaArguments Arguments;
                        Arguments.PushNumber ( fDeltaHealth );
                        pVehicle->CallEvent ( "onVehicleDamage", Arguments );
                    }
                }
                pVehicle->SetHealth ( fHealth );
                // Stops sync + fixVehicle/setElementHealth conflicts triggering onVehicleDamage by having a seperate stored float keeping track of ONLY what comes in via sync
                // - Caz
                pVehicle->SetLastSyncedHealth( fHealth );

                // Trailer chain
                CVehicle* pTowedByVehicle = pVehicle;
                CVehicle* pTrailer = NULL;
                ElementID TrailerID;
                bool bHasTrailer;
                if ( !BitStream.ReadBit ( bHasTrailer ) )
                    return false;

                while ( bHasTrailer )
                {
                    BitStream.Read ( TrailerID );
                    CElement* pElement = CElementIDs::GetElement ( TrailerID );
                    if ( pElement )
                        pTrailer = dynamic_cast < CVehicle* > ( pElement );  // Uses dynamic_cast as pre r4481 client could send as incorrect element
                    
                    // Read out the trailer position and rotation
                    SPositionSync trailerPosition ( false );
                    if ( !BitStream.Read ( &trailerPosition ) )
                        return false;

                    SRotationDegreesSync trailerRotation;
                    if ( !BitStream.Read ( &trailerRotation ) )
                        return false;

                    // If we found the trailer
                    if ( pTrailer )
                    {
                        // Set its position and rotation
                        pTrailer->SetPosition ( trailerPosition.data.vecPosition );
                        pTrailer->SetRotationDegrees ( trailerRotation.data.vecRotation );
    
                        // Is this a new trailer, attached?
                        CVehicle* pCurrentTrailer = pTowedByVehicle->GetTowedVehicle ();
                        if ( pCurrentTrailer != pTrailer )
                        {
                            // If theres a trailer already attached
                            if ( pCurrentTrailer )
                            {
                                pTowedByVehicle->SetTowedVehicle ( NULL );
                                pCurrentTrailer->SetTowedByVehicle ( NULL );

                                // Tell everyone to detach them
                                CVehicleTrailerPacket AttachPacket ( pTowedByVehicle, pCurrentTrailer, false );
                                g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( AttachPacket );

                                // Execute the attach trailer script function
                                CLuaArguments Arguments;
                                Arguments.PushElement ( pTowedByVehicle );
                                pCurrentTrailer->CallEvent ( "onTrailerDetach", Arguments );
                            }

                            // If something else is towing this trailer
                            CVehicle* pCurrentVehicle = pTrailer->GetTowedByVehicle ();
                            if ( pCurrentVehicle )
                            {
                                pCurrentVehicle->SetTowedVehicle ( NULL );
                                pTrailer->SetTowedByVehicle ( NULL );

                                // Tell everyone to detach them
                                CVehicleTrailerPacket AttachPacket ( pCurrentVehicle, pTrailer, false );
                                g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( AttachPacket );

                                // Execute the attach trailer script function
                                CLuaArguments Arguments;
                                Arguments.PushElement ( pCurrentVehicle );
                                pTrailer->CallEvent ( "onTrailerDetach", Arguments );
                            }

                            pTowedByVehicle->SetTowedVehicle ( pTrailer );
                            pTrailer->SetTowedByVehicle ( pTowedByVehicle );

                            // Execute the attach trailer script function
                            CLuaArguments Arguments;
                            Arguments.PushElement ( pTowedByVehicle );
                            bool bContinue = pTrailer->CallEvent ( "onTrailerAttach", Arguments );

                            // Attach or detach trailers depending on the event outcome
                            CVehicleTrailerPacket TrailerPacket ( pTowedByVehicle, pTrailer, bContinue );
                            g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( TrailerPacket );
                        }
                    }
                    else
                        break;

                    pTowedByVehicle = pTrailer;

                    if ( BitStream.ReadBit ( bHasTrailer ) == false )
                        return false;
                }

                // If there was a trailer before
                CVehicle* pCurrentTrailer = pTowedByVehicle->GetTowedVehicle ();
                if ( pCurrentTrailer )
                {
                    pTowedByVehicle->SetTowedVehicle ( NULL );
                    pCurrentTrailer->SetTowedByVehicle ( NULL );

                    // Tell everyone else to detach them
                    CVehicleTrailerPacket AttachPacket ( pTowedByVehicle, pCurrentTrailer, false );
                    g_pGame->GetPlayerManager ()->BroadcastOnlyJoined ( AttachPacket );

                    // Execute the detach trailer script function
                    CLuaArguments Arguments;
                    Arguments.PushElement ( pTowedByVehicle );
                    pCurrentTrailer->CallEvent ( "onTrailerDetach", Arguments );                    
                }
            }

            // Update Damage info
            if ( BitStream.Version() >= 0x047) 
            {
                if ( BitStream.ReadBit () == true )
                {
                    ElementID DamagerID;
                    if ( !BitStream.Read ( DamagerID ) )
                        return false;

                    SWeaponTypeSync weaponType;
                    if ( !BitStream.Read ( &weaponType ) )
                        return false;

                    SBodypartSync bodyPart;
                    if ( !BitStream.Read ( &bodyPart ) )
                        return false;

                    pSourcePlayer->SetDamageInfo ( DamagerID, weaponType.data.ucWeaponType, bodyPart.data.uiBodypart );
                }
            }

            // Player health
            SPlayerHealthSync health;
            if ( !BitStream.Read ( &health ) )
                return false;
            float fHealth = health.data.fValue;

            float fOldHealth = pSourcePlayer->GetHealth ();
            float fHealthLoss = fOldHealth - fHealth;

            // Less than last packet's frame?
            if ( fHealth < fOldHealth && fHealthLoss > 0 )
            {
                if (BitStream.Version() <= 0x046)
                {
                    // Call the onPlayerDamage event
                    CLuaArguments Arguments;
                    Arguments.PushNil ();
                    Arguments.PushNumber ( false );
                    Arguments.PushNumber ( false );
                    Arguments.PushNumber ( fHealthLoss );
                    pSourcePlayer->CallEvent ( "onPlayerDamage", Arguments );
                }
                else
                {
                    // Call the onPlayerDamage event
                    CLuaArguments Arguments;
                    
                    CElement* pDamageSource = CElementIDs::GetElement ( pSourcePlayer->GetPlayerAttacker () );
                    if ( pDamageSource ) Arguments.PushElement ( pDamageSource );
                    else Arguments.PushNil();
                    Arguments.PushNumber ( pSourcePlayer->GetAttackWeapon () );
                    Arguments.PushNumber ( pSourcePlayer->GetAttackBodyPart () );
                    Arguments.PushNumber ( fHealthLoss );
                    pSourcePlayer->CallEvent ( "onPlayerDamage", Arguments );              
                }
            }
            pSourcePlayer->SetHealth ( fHealth );

            // Armor
            SPlayerArmorSync armor;
            if ( !BitStream.Read ( &armor ) )
                return false;
            float fArmor = armor.data.fValue;

            float fOldArmor = pSourcePlayer->GetArmor ();
            float fArmorLoss = fOldArmor - fArmor;

            // Less than last packet's frame?
            if ( fArmor < fOldArmor && fArmorLoss > 0 )
            {
                if (BitStream.Version() <= 0x046)
                {
                    // Call the onPlayerDamage event
                    CLuaArguments Arguments;
                    Arguments.PushNil ();
                    Arguments.PushNumber ( false );
                    Arguments.PushNumber ( false );
                    Arguments.PushNumber ( fArmorLoss );
                    pSourcePlayer->CallEvent ( "onPlayerDamage", Arguments );
                }
                else
                {
                    // Call the onPlayerDamage event
                    CLuaArguments Arguments;
                    
                    CElement* pDamageSource = CElementIDs::GetElement ( pSourcePlayer->GetPlayerAttacker () );
                    if ( pDamageSource ) Arguments.PushElement ( pDamageSource );
                    else Arguments.PushNil();
                    Arguments.PushNumber ( pSourcePlayer->GetAttackWeapon () );
                    Arguments.PushNumber ( pSourcePlayer->GetAttackBodyPart () );
                    Arguments.PushNumber ( fArmorLoss );
                    pSourcePlayer->CallEvent ( "onPlayerDamage", Arguments );              
                }          
            }
            pSourcePlayer->SetArmor ( fArmor );

            // Flags
            SVehiclePuresyncFlags flags;
            if ( !BitStream.Read ( &flags ) )
                return false;

            pSourcePlayer->SetWearingGoggles ( flags.data.bIsWearingGoggles );
            pSourcePlayer->SetDoingGangDriveby ( flags.data.bIsDoingGangDriveby );            


            // Weapon sync
            if ( flags.data.bHasAWeapon )
            {
                SWeaponSlotSync slot;
                if ( !BitStream.Read ( &slot ) )
                    return false;

                pSourcePlayer->SetWeaponSlot ( slot.data.uiSlot );

                if ( flags.data.bIsDoingGangDriveby && CWeaponNames::DoesSlotHaveAmmo ( slot.data.uiSlot ) )
                {      
                    float fWeaponRange = pSourcePlayer->GetWeaponRangeFromSlot ( slot.data.uiSlot );

                    // Read the ammo states
                    SWeaponAmmoSync ammo ( pSourcePlayer->GetWeaponType (), BitStream.Version () >= 0x44, true );
                    if ( !BitStream.Read ( &ammo ) )
                        return false;
                    pSourcePlayer->SetWeaponAmmoInClip ( ammo.data.usAmmoInClip );
                    if ( BitStream.Version () >= 0x44 )
                        pSourcePlayer->SetWeaponTotalAmmo ( ammo.data.usTotalAmmo );

                    // Read aim data
                    SWeaponAimSync aim ( fWeaponRange, true );
                    if ( !BitStream.Read ( &aim ) )
                        return false;
                    pSourcePlayer->SetAimDirection ( aim.data.fArm );
                    pSourcePlayer->SetSniperSourceVector ( aim.data.vecOrigin );
                    pSourcePlayer->SetTargettingVector ( aim.data.vecTarget );

                    // Read the driveby direction
                    SDrivebyDirectionSync driveby;
                    if ( !BitStream.Read ( &driveby ) )
                        return false;
                    pSourcePlayer->SetDriveByDirection ( driveby.data.ucDirection );
                }
            }
            else
                pSourcePlayer->SetWeaponSlot ( 0 );


            // Vehicle specific data if he's the driver
            if ( uiSeat == 0 )
            {
                ReadVehicleSpecific ( pVehicle, BitStream );

                // Set vehicle specific stuff if he's the driver
                pVehicle->SetSirenActive ( flags.data.bIsSirenOrAlarmActive );
                pVehicle->SetSmokeTrailEnabled ( flags.data.bIsSmokeTrailEnabled );
                pVehicle->SetLandingGearDown ( flags.data.bIsLandingGearDown );
                pVehicle->SetOnGround ( flags.data.bIsOnGround );
                pVehicle->SetInWater ( flags.data.bIsInWater );
                pVehicle->SetDerailed ( flags.data.bIsDerailed );
                pVehicle->SetHeliSearchLightVisible ( flags.data.bIsHeliSearchLightVisible );
            }

            // Read the vehicle_look_left and vehicle_look_right control states
            // if it's an aircraft.
            if ( flags.data.bIsAircraft )
            {
                ControllerState.LeftShoulder2 = BitStream.ReadBit () * 255;
                ControllerState.RightShoulder2 = BitStream.ReadBit () * 255;
            }

            pSourcePlayer->GetPad ()->NewControllerState ( ControllerState );

            // Success
            return true;
        }
    }

    return false;
}


bool CVehiclePuresyncPacket::Write ( NetBitStreamInterface& BitStream ) const
{
    // Got a player to send?
    if ( m_pSourceElement )
    {
        CPlayer * pSourcePlayer = static_cast < CPlayer * > ( m_pSourceElement );

        // Player is in a vehicle and is the driver?
        CVehicle* pVehicle = pSourcePlayer->GetOccupiedVehicle ();
        if ( pVehicle )
        {
            // Player ID
            ElementID PlayerID = pSourcePlayer->GetID ();
            BitStream.Write ( PlayerID );

            // Write the time context of that player
            BitStream.Write ( pSourcePlayer->GetSyncTimeContext () );

            // Write his ping divided with 2 plus a small number so the client can find out when this packet was sent
            unsigned short usLatency = pSourcePlayer->GetPing ();
            BitStream.WriteCompressed ( usLatency );

            // Write the keysync data
            const CControllerState& ControllerState = pSourcePlayer->GetPad ()->GetCurrentControllerState ();
            WriteFullKeysync ( ControllerState, BitStream );

            // Write the vehicle matrix only if he's the driver
            CVector vecTemp;
            unsigned int uiSeat = pSourcePlayer->GetOccupiedVehicleSeat ();
            if ( uiSeat == 0 )
            {
                // Vehicle position
                SPositionSync position ( false );
                position.data.vecPosition = pVehicle->GetPosition ();
                BitStream.Write ( &position );

                // Vehicle rotation
                SRotationDegreesSync rotation;
                pVehicle->GetRotationDegrees ( rotation.data.vecRotation );
                BitStream.Write ( &rotation );

                // Move speed vector
                SVelocitySync velocity;
                velocity.data.vecVelocity = pVehicle->GetVelocity ();
                BitStream.Write ( &velocity );

                // Turn speed vector
                SVelocitySync turnSpeed;
                turnSpeed.data.vecVelocity = pVehicle->GetTurnSpeed ();
                BitStream.Write ( &turnSpeed );

                // Health
                SVehicleHealthSync health;
                health.data.fValue = pVehicle->GetHealth ();
                BitStream.Write ( &health );

                // Write the trailer chain
                if ( BitStream.Version () >= 0x42 )
                {
                    CVehicle* pTrailer = pVehicle->GetTowedVehicle ();
                    while ( pTrailer )
                    {
                        BitStream.WriteBit ( true );
                        BitStream.Write ( pTrailer->GetID () );

                        // Write the position and rotation
                        CVector vecTrailerPosition, vecTrailerRotationDegrees;

                        // Write the matrix
                        vecTrailerPosition = pTrailer->GetPosition ();
                        pTrailer->GetRotationDegrees ( vecTrailerRotationDegrees );

                        SPositionSync trailerPosition ( false );
                        trailerPosition.data.vecPosition = vecTrailerPosition;
                        BitStream.Write ( &trailerPosition );

                        SRotationDegreesSync trailerRotation;
                        trailerRotation.data.vecRotation = vecTrailerRotationDegrees;
                        BitStream.Write ( &trailerRotation );

                        // Get the next towed vehicle
                        pTrailer = pTrailer->GetTowedVehicle ();
                    }

                    // End of our trailer chain
                    BitStream.WriteBit ( false );
                }
            }

            // Player health and armor
            SPlayerHealthSync health;
            health.data.fValue = pSourcePlayer->GetHealth ();
            BitStream.Write ( &health );

            SPlayerArmorSync armor;
            armor.data.fValue = pSourcePlayer->GetArmor ();
            BitStream.Write ( &armor );

            // Weapon
            unsigned char ucWeaponType = pSourcePlayer->GetWeaponType ();

            // Flags
            SVehiclePuresyncFlags flags;
            flags.data.bIsWearingGoggles     = pSourcePlayer->IsWearingGoggles ();
            flags.data.bIsDoingGangDriveby   = pSourcePlayer->IsDoingGangDriveby ();
            flags.data.bIsSirenOrAlarmActive = pVehicle->IsSirenActive ();
            flags.data.bIsSmokeTrailEnabled  = pVehicle->IsSmokeTrailEnabled ();
            flags.data.bIsLandingGearDown    = pVehicle->IsLandingGearDown ();
            flags.data.bIsOnGround           = pVehicle->IsOnGround ();
            flags.data.bIsInWater            = pVehicle->IsInWater ();
            flags.data.bIsDerailed           = pVehicle->IsDerailed ();
            flags.data.bIsAircraft           = ( pVehicle->GetVehicleType () == VEHICLE_PLANE ||
                                                 pVehicle->GetVehicleType () == VEHICLE_HELI );
            flags.data.bHasAWeapon           = ( ucWeaponType != 0 );
            flags.data.bIsHeliSearchLightVisible = pVehicle->IsHeliSearchLightVisible ();
            BitStream.Write ( &flags );

            #if MTASA_VERSION_MINOR == 3
                // till r5289 the derailed state reads the landing gear down so since it's unused for trains put the derailed flag in there too so we can fix old clients
                flags.FixDerailedState ( pVehicle->GetModel ( ) );
            #endif

            // Write the weapon stuff
            if ( flags.data.bHasAWeapon )
            {
                // Write the weapon slot
                SWeaponSlotSync slot;
                slot.data.uiSlot = pSourcePlayer->GetWeaponSlot ();
                BitStream.Write ( &slot );

                if ( flags.data.bIsDoingGangDriveby && CWeaponNames::DoesSlotHaveAmmo ( slot.data.uiSlot ) )
                {
                    // Write the ammo states
                    SWeaponAmmoSync ammo ( ucWeaponType, BitStream.Version () >= 0x44, true );
                    ammo.data.usAmmoInClip = pSourcePlayer->GetWeaponAmmoInClip ();
                    ammo.data.usTotalAmmo = pSourcePlayer->GetWeaponTotalAmmo ();
                    BitStream.Write ( &ammo );

                    // Sync aim data
                    SWeaponAimSync aim ( 0.0f, true );
                    aim.data.vecOrigin = pSourcePlayer->GetSniperSourceVector ();
                    pSourcePlayer->GetTargettingVector ( aim.data.vecTarget );
                    aim.data.fArm = pSourcePlayer->GetAimDirection ();
                    BitStream.Write ( &aim );

                    // Sync driveby direction
                    SDrivebyDirectionSync driveby;
                    driveby.data.ucDirection = pSourcePlayer->GetDriveByDirection ();
                    BitStream.Write ( &driveby );
                }
            }

            // Vehicle specific data only if he's the driver
            if ( uiSeat == 0 )
            {
                WriteVehicleSpecific ( pVehicle, BitStream );
            }

            // Write vehicle_look_left and vehicle_look_right control states when
            // it's an aircraft.
            if ( flags.data.bIsAircraft )
            {
                BitStream.WriteBit ( ControllerState.LeftShoulder2 != 0 );
                BitStream.WriteBit ( ControllerState.RightShoulder2 != 0 );
            }

            // Success
            return true;
        }
    }

    return false;
}


void CVehiclePuresyncPacket::ReadVehicleSpecific ( CVehicle* pVehicle, NetBitStreamInterface& BitStream )
{
    // Turret data
    unsigned short usModel = pVehicle->GetModel ();
    if ( CVehicleManager::HasTurret ( usModel ) ) 
    {
        // Read out the turret position
        SVehicleTurretSync vehicle;
        if ( !BitStream.Read ( &vehicle ) )
            return;

        // Set the data
        pVehicle->SetTurretPosition ( vehicle.data.fTurretX, vehicle.data.fTurretY );
    }

    // Adjustable property value
    if ( CVehicleManager::HasAdjustableProperty ( usModel ) )
    {
        unsigned short usAdjustableProperty;
        if ( BitStream.Read ( usAdjustableProperty ) )
        {
            pVehicle->SetAdjustableProperty ( usAdjustableProperty );
        }
    }

    // Door angles.
    if ( CVehicleManager::HasDoors ( usModel ) )
    {
        SDoorOpenRatioSync door;

        for ( unsigned int i = 2; i < 6; ++i )
        {
            if ( !BitStream.Read ( &door ) )
                return;
            pVehicle->SetDoorOpenRatio ( i, door.data.fRatio );
        }
    }
}


void CVehiclePuresyncPacket::WriteVehicleSpecific ( CVehicle* pVehicle, NetBitStreamInterface& BitStream ) const
{
    // Turret states
    unsigned short usModel = pVehicle->GetModel ();
    if ( CVehicleManager::HasTurret ( usModel ) )
    {
        SVehicleTurretSync vehicle;
        pVehicle->GetTurretPosition ( vehicle.data.fTurretX, vehicle.data.fTurretY );

        BitStream.Write ( &vehicle );
    }

    // Adjustable property value
    if ( CVehicleManager::HasAdjustableProperty ( usModel ) )
    {
        BitStream.Write ( pVehicle->GetAdjustableProperty () );
    }

    // Door angles.
    if ( CVehicleManager::HasDoors ( usModel ) )
    {
        SDoorOpenRatioSync door;
        for ( unsigned int i = 2; i < 6; ++i )
        {
            door.data.fRatio = pVehicle->GetDoorOpenRatio ( i );
            BitStream.Write ( &door );
        }
    }
}
