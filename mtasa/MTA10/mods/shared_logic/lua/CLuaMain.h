/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaMain.h
*  PURPOSE:     Lua virtual machine container class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CLuaMain;

#ifndef __CLUAMAIN_H
#define __CLUAMAIN_H

#include "CLuaTimerManager.h"

#include "CLuaFunctionDefs.h"

#include <xml/CXMLFile.h>

#define MAX_SCRIPTNAME_LENGTH 64

#include <list>

struct CRefInfo
{
    unsigned long int ulUseCount;
    int iFunction;
};

class CLuaMain //: public CClient
{
public:
    ZERO_ON_NEW
                                    CLuaMain                ( class CLuaManager* pLuaManager, CResource* pResourceOwner );
                                    ~CLuaMain               ( void );

    bool                            LoadScriptFromBuffer    ( const char* cpBuffer, unsigned int uiSize, const char* szFileName );
    bool                            LoadScript              ( const char* szLUAScript );
    void                            UnloadScript            ( void );

    void                            Start                   ( void );

    void                            DoPulse                 ( void );

    const char*                     GetScriptName           ( void ) const                  { return m_strScriptName; }
    void                            SetScriptName           ( const char* szName )          { m_strScriptName.AssignLeft ( szName, MAX_SCRIPTNAME_LENGTH ); }

    void                            RegisterFunction        ( const char* szFunction, lua_CFunction function );

    inline lua_State*               GetVM                   ( void )                        { return m_luaVM; };
    inline CLuaTimerManager*        GetTimerManager         ( void ) const                  { return m_pLuaTimerManager; };

    bool                            BeingDeleted            ( void );
    inline lua_State *              GetVirtualMachine       ( void ) const                  { return m_luaVM; };

    void                            ResetInstructionCount   ( void );

    inline class CResource*         GetResource             ( void )                        { return m_pResource; }

    CXMLFile *                      CreateXML               ( const char* szFilename );
    void                            DestroyXML              ( CXMLFile* pFile );
    void                            DestroyXML              ( CXMLNode* pRootNode );
    void                            SaveXML                 ( CXMLNode * pRootNode );
    bool                            XMLExists               ( CXMLFile* pFile );
    unsigned long                   GetXMLFileCount         ( void ) const                  { return m_XMLFiles.size (); };
    unsigned long                   GetTimerCount           ( void ) const                  { return m_pLuaTimerManager ? m_pLuaTimerManager->GetTimerCount () : 0; };
    unsigned long                   GetElementCount         ( void ) const;

    void                            InitVM                  ( void );
    const SString&                  GetFunctionTag          ( int iLuaFunction );
    int                             PCall                   ( lua_State *L, int nargs, int nresults, int errfunc );

private:
    void                            InitSecurity            ( void );

    static void                     InstructionCountHook    ( lua_State* luaVM, lua_Debug* pDebug );

    SString                         m_strScriptName;

    lua_State*                      m_luaVM;
    CLuaTimerManager*               m_pLuaTimerManager;

    bool                            m_bBeingDeleted; // prevent it being deleted twice

    CElapsedTime                    m_FunctionEnterTimer;

    class CResource*                m_pResource;

    std::list < CXMLFile* >         m_XMLFiles;
public:
    std::map < const void*, CRefInfo >      m_CallbackTable;
    std::map < int, SString >               m_FunctionTagMap;
};

#endif
