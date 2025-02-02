/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CCustomData.cpp
*  PURPOSE:     Custom entity data class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Ed Lyons <>
*               Jax <>
*               Cecill Etheredge <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

void CCustomData::Copy ( CCustomData* pCustomData )
{
    map < std::string, SCustomData > :: const_iterator iter = pCustomData->IterBegin ();
    for ( ; iter != pCustomData->IterEnd (); iter++ )
    {
        Set ( iter->first.c_str (), iter->second.Variable, iter->second.pLuaMain );
    }
}

SCustomData* CCustomData::Get ( const char* szName )
{
    assert ( szName );

    std::map < std::string, SCustomData > :: const_iterator it = m_Data.find ( szName );
    if ( it != m_Data.end () )
        return (SCustomData *)&it->second;

    return NULL;
}

SCustomData* CCustomData::GetSynced ( const char* szName )
{
    assert ( szName );

    std::map < std::string, SCustomData > :: const_iterator it = m_SyncedData.find ( szName );
    if ( it != m_SyncedData.end () )
        return (SCustomData *)&it->second;

    return NULL;
}


bool CCustomData::DeleteSynced ( const char* szName )
{
    // Find the item and delete it
    std::map < std::string, SCustomData > :: iterator iter = m_SyncedData.find ( szName );
    if ( iter != m_SyncedData.end () )
    {
        m_SyncedData.erase ( iter );
        return true;
    }

    // Didn't exist
    return false;
}

void CCustomData::UpdateSynced ( const char* szName, const CLuaArgument& Variable, class CLuaMain* pLuaMain, bool bSynchronized )
{
    if ( bSynchronized )
    {
        SCustomData* pDataSynced = GetSynced ( szName );
        if ( pDataSynced )
        {
            pDataSynced->Variable = Variable;
            pDataSynced->pLuaMain = pLuaMain;
            pDataSynced->bSynchronized = bSynchronized;
        }
        else
        {
            SCustomData newData;
            newData.Variable = Variable;
            newData.pLuaMain = pLuaMain;
            newData.bSynchronized = bSynchronized;
            m_SyncedData [ szName ] = newData;
        }
    }
    else
    {
        DeleteSynced ( szName );
    }
}
// User-defined warnings
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : warning C0000 *MTA Developers*: "

void CCustomData::Set ( const char* szName, const CLuaArgument& Variable, class CLuaMain* pLuaMain, bool bSynchronized )
{
    assert ( szName );

    // Grab the item with the given name
    SCustomData* pData = Get ( szName );
    if ( pData )
    {
        // Set the variable and eventually its new owner
        pData->Variable = Variable;
        pData->pLuaMain = pLuaMain;
        pData->bSynchronized = bSynchronized;
        UpdateSynced ( szName, Variable, pLuaMain, bSynchronized );
    }
    else
    {
        // Set the stuff and add it
        SCustomData newData;
        newData.Variable = Variable;
        newData.pLuaMain = pLuaMain;
        newData.bSynchronized = bSynchronized;
        m_Data [ szName ] = newData;
        UpdateSynced ( szName, Variable, pLuaMain, bSynchronized );
    }
}


bool CCustomData::Delete ( const char* szName )
{
    // Find the item and delete it
    std::map < std::string, SCustomData > :: iterator it = m_Data.find ( szName );
    if ( it != m_Data.end () )
    {
        DeleteSynced ( szName );
        m_Data.erase ( it );
        return true;
    }

    // Didn't exist
    return false;
}

void CCustomData::DeleteAll ( class CLuaMain* pLuaMain )
{
    // Delete any items with matching VM's
    std::map < std::string, SCustomData > :: iterator iter = m_Data.begin ();
    while ( iter != m_Data.end () )
    {
        // Delete it if they match
        if ( iter->second.pLuaMain == pLuaMain )
            m_Data.erase ( iter++ );
        else
            iter++;
    }
    iter = m_SyncedData.begin ();
    while ( iter != m_SyncedData.end () )
    {
        // Delete it if they match
        if ( iter->second.pLuaMain == pLuaMain )
            m_SyncedData.erase ( iter++ );
        else
            iter++;
    }
}


void CCustomData::DeleteAll ( void )
{
    // Delete all the items
    m_Data.clear ( );
    m_SyncedData.clear ( );
}

CXMLNode * CCustomData::OutputToXML ( CXMLNode * pNode )
{
    std::map < std::string, SCustomData > :: const_iterator iter = m_Data.begin ();
    for ( ; iter != m_Data.end (); iter++ )
    {
        CLuaArgument* arg = (CLuaArgument *)&iter->second.Variable;
        
        switch ( arg->GetType() )
        {
        case LUA_TSTRING:
            {
                CXMLAttribute* attr = pNode->GetAttributes().Create( iter->first.c_str () );
                attr->SetValue ( arg->GetString ().c_str () );
                break;
            }
        case LUA_TNUMBER:
            {
                CXMLAttribute* attr = pNode->GetAttributes().Create( iter->first.c_str () );
                attr->SetValue ( (float)arg->GetNumber () );
                break;
            }
        case LUA_TBOOLEAN:
            {
                CXMLAttribute* attr = pNode->GetAttributes().Create( iter->first.c_str () );
                attr->SetValue ( arg->GetBoolean () );
                break;
            }
        }
    }
    return pNode;   
}

unsigned short CCustomData::CountOnlySynchronized ( void )
{
    return m_SyncedData.size ( ); 
}