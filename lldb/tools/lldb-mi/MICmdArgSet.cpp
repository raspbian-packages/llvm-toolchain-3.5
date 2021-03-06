//===-- MICmdArgSet.cpp -----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//++
// File:		MICmdArgSet.cpp
//
// Overview:	CMICmdArgSet implementation.
//
// Environment:	Compilers:	Visual C++ 12.
//							gcc (Ubuntu/Linaro 4.8.1-10ubuntu9) 4.8.1
//				Libraries:	See MIReadmetxt. 
//
// Copyright:	None.
//--

// In-house headers:
#include "MICmdArgSet.h"
#include "MICmdArgValBase.h"
#include "MICmnResources.h"
#include "MICmnLog.h"
#include "MICmnConfig.h"

//++ ------------------------------------------------------------------------------------
// Details:	CMICmdArgSet constructor.
// Type:	Method.
// Args:	None.
// Return:	None.
// Throws:	None.
//--
CMICmdArgSet::CMICmdArgSet( void )
:	m_bIsArgsPresentButNotHandledByCmd( false )
,	m_constStrCommaSpc( ", " )
{
}

//++ ------------------------------------------------------------------------------------
// Details:	CMICmdArgSet destructor.
// Type:	Method.
// Args:	None.
// Return:	None.
// Throws:	None.
//--
CMICmdArgSet::~CMICmdArgSet( void )
{
	// Tidy up
	Destroy();
}

//++ ------------------------------------------------------------------------------------
// Details:	Release resources used by *this container object.
// Type:	Method.
// Args:	None.
// Return:	None.
// Throws:	None.
//--
void CMICmdArgSet::Destroy( void )
{
	// Delete command argument objects
	if( !m_setCmdArgs.empty() )
	{
		SetCmdArgs_t::iterator it = m_setCmdArgs.begin();
		while( it != m_setCmdArgs.end() )
		{
			CMICmdArgValBase * pArg( *it );
			delete pArg;
		
			// Next
			++it;
		}
		m_setCmdArgs.clear();
	}

	m_setCmdArgsThatNotValid.clear();
	m_setCmdArgsThatAreMissing.clear();
	m_setCmdArgsNotHandledByCmd.clear();
	m_setCmdArgsMissingInfo.clear();
	m_bIsArgsPresentButNotHandledByCmd = false;
}

//++ ------------------------------------------------------------------------------------
// Details:	Retrieve the state flag indicating that the command set up ready to parse
//			command arguments or options found that one or more arguments was indeed
//			present but not handled. This is given as a warning in the MI log file.
// Type:	Method.
// Args:	None.
// Return:	bool - True = one or more args not handled, false = all args handled
// Throws:	None.
//--
bool CMICmdArgSet::IsArgsPresentButNotHandledByCmd( void ) const
{
	return m_bIsArgsPresentButNotHandledByCmd;
}
	
//++ ------------------------------------------------------------------------------------
// Details:	Add the list of command's arguments to parse and validate another one.
// Type:	Method.
// Args:	vArg	- (R) A command argument object.
// Return:	MIstatus::success - Functional succeeded.
//			MIstatus::failure - Functional failed.
// Throws:	None.
//--
bool CMICmdArgSet::Add( const CMICmdArgValBase & vArg )
{
	CMICmdArgValBase * pArg = const_cast< CMICmdArgValBase * >( &vArg );
	m_setCmdArgs.push_back( pArg );

	return MIstatus::success;
}

//++ ------------------------------------------------------------------------------------
// Details:	After validating an options line of text (the context) and there is a failure,
//			it is likely a mandatory command argument that is required is missing. This 
//			function returns the argument that should be present.
// Type:	Method.
// Args:	None.
// Return:	SetCmdArgs_t & - Set of argument objects.
// Throws:	None.
//--
const CMICmdArgSet::SetCmdArgs_t &	CMICmdArgSet::GetArgsThatAreMissing( void ) const
{
	return m_setCmdArgsThatAreMissing;
}

//++ ------------------------------------------------------------------------------------
// Details:	After validating an options line of text (the context) and there is a failure,
//			it may be because one or more arguments were unable to extract a value. This 
//			function returns the argument that were found to be invalid.
// Type:	Method.
// Args:	None.
// Return:	SetCmdArgs_t & - Set of argument objects.
// Throws:	None.
//--
const CMICmdArgSet::SetCmdArgs_t &	CMICmdArgSet::GetArgsThatInvalid( void ) const
{
	return m_setCmdArgsThatNotValid;
}	

//++ ------------------------------------------------------------------------------------
// Details:	The list of argument or option (objects) that were specified by the command
//			and so recognised when parsed but were not handled. Ideally the command
//			should handle all arguments and options presented to it. The command sends
//			warning to the MI log file to say that these options were not handled.
//			Used as one way to determine option that maybe should really be implemented
//			and not just ignored.
// Type:	Method.
// Args:	None.
// Return:	SetCmdArgs_t & - Set of argument objects.
// Throws:	None.
//--
const CMICmdArgSet::SetCmdArgs_t &	CMICmdArgSet::GetArgsNotHandledByCmd( void ) const
{
	return m_setCmdArgsNotHandledByCmd;
}

//++ ------------------------------------------------------------------------------------
// Details:	Given a set of command argument objects parse the context option string to
//			find those argument and retrieve their value. If the function fails call
//			GetArgsThatAreMissing() to see which commands that were mandatory were 
//			missing or failed to parse.
// Type:	Method.
// Args:	vStrMiCmd		- (R)  Command's name.
//			vCmdArgsText	- (RW) A command's options or argument.
// Return:	MIstatus::success - Functional succeeded.
//			MIstatus::failure - Functional failed.
// Throws:	None.
//--
bool CMICmdArgSet::Validate( const CMIUtilString & vStrMiCmd, CMICmdArgContext & vwCmdArgsText )
{
	m_cmdArgContext = vwCmdArgsText;

	// Iterate all the arguments or options required by a command
	const MIuint nArgs = vwCmdArgsText.GetNumberArgsPresent();
	MIuint nArgsMandatoryCnt = 0;
	SetCmdArgs_t::const_iterator it = m_setCmdArgs.begin();
	while( it != m_setCmdArgs.end() )
	{
		const CMICmdArgValBase * pArg( *it );
		const CMIUtilString & rArgName( pArg->GetName() ); MIunused( rArgName );
		if( pArg->GetIsMandatory() )
			nArgsMandatoryCnt++;
		if( !const_cast< CMICmdArgValBase * >( pArg )->Validate( vwCmdArgsText ) )
		{
			if( pArg->GetIsMandatory() && !pArg->GetFound() )
				m_setCmdArgsThatAreMissing.push_back( const_cast< CMICmdArgValBase * >( pArg ) );
			else if( pArg->GetFound() ) 
			{
				if( pArg->GetIsMissingOptions() )
					m_setCmdArgsMissingInfo.push_back( const_cast< CMICmdArgValBase * >( pArg ) );
				else if( !pArg->GetValid() )
					m_setCmdArgsThatNotValid.push_back( const_cast< CMICmdArgValBase * >( pArg ) );
			}
		}
		if( pArg->GetFound() && !pArg->GetIsHandledByCmd() )
		{
			m_bIsArgsPresentButNotHandledByCmd = true;
			m_setCmdArgsNotHandledByCmd.push_back( const_cast< CMICmdArgValBase * >( pArg ) );
		}

		// Next
		++it;
	}

	// Check that one or more argument objects have any issues to report...

	if( nArgs < nArgsMandatoryCnt )
	{
		SetErrorDescription( CMIUtilString::Format( MIRSRC( IDS_CMD_ARGS_ERR_N_OPTIONS_REQUIRED ), nArgsMandatoryCnt ) );
		return MIstatus::failure;
	}
	
	if( !vwCmdArgsText.IsEmpty() )
	{
		SetErrorDescription( CMIUtilString::Format( MIRSRC( IDS_CMD_ARGS_ERR_CONTEXT_NOT_ALL_EATTEN ), vwCmdArgsText.GetArgsLeftToParse().c_str() ) );
		return MIstatus::failure;
	}

	if( IsArgsPresentButNotHandledByCmd() )
		WarningArgsNotHandledbyCmdLogFile( vStrMiCmd );

	CMIUtilString strListMissing;
	CMIUtilString strListInvalid;
	CMIUtilString strListMissingInfo;
	const bool bArgsMissing = (m_setCmdArgsThatAreMissing.size() > 0);
	const bool bArgsInvalid = (m_setCmdArgsThatNotValid.size() > 0);
	const bool bArgsMissingInfo = (m_setCmdArgsMissingInfo.size() > 0);
	if( !(bArgsMissing || bArgsInvalid || bArgsMissingInfo) )
		return MIstatus::success;
	if( bArgsMissing )
	{
		MIuint i = 0;
		SetCmdArgs_t::const_iterator it = m_setCmdArgsThatAreMissing.begin();
		while( it != m_setCmdArgsThatAreMissing.end() )
		{
			if( i++ > 0 )
				strListMissing += m_constStrCommaSpc;

			const CMICmdArgValBase * pArg( *it );
			strListMissing += pArg->GetName();

			// Next
			++it;
		}
	}
	if( bArgsInvalid )
	{
		MIuint i = 0;
		SetCmdArgs_t::const_iterator it = m_setCmdArgsThatNotValid.begin();
		while( it != m_setCmdArgsThatNotValid.end() )
		{
			if( i++ > 0 )
				strListMissing += m_constStrCommaSpc;

			const CMICmdArgValBase * pArg( *it );
			strListInvalid += pArg->GetName();

			// Next
			++it;
		}
	}
	if( bArgsMissingInfo )
	{
		MIuint i = 0;
		SetCmdArgs_t::const_iterator it = m_setCmdArgsMissingInfo.begin();
		while( it != m_setCmdArgsMissingInfo.end() )
		{
			if( i++ > 0 )
				strListMissingInfo += m_constStrCommaSpc;

			const CMICmdArgValBase * pArg( *it );
			strListMissingInfo += pArg->GetName();

			// Next
			++it;
		}
	}
	if( bArgsMissing && bArgsInvalid )
	{
		SetErrorDescription( CMIUtilString::Format( MIRSRC( IDS_CMD_ARGS_ERR_VALIDATION_MAN_INVALID ), strListMissing.c_str(), strListInvalid.c_str() ) );
		return MIstatus::failure;
	}
	if( bArgsMissing )
	{
		SetErrorDescription( CMIUtilString::Format( MIRSRC( IDS_CMD_ARGS_ERR_VALIDATION_MANDATORY ), strListMissing.c_str() ) );
		return MIstatus::failure;
	}
	if( bArgsMissingInfo )
	{
		SetErrorDescription( CMIUtilString::Format( MIRSRC( IDS_CMD_ARGS_ERR_VALIDATION_MISSING_INF ), strListMissingInfo.c_str() ) );
		return MIstatus::failure;
	}
	if( bArgsInvalid )
	{
		SetErrorDescription( CMIUtilString::Format( MIRSRC( IDS_CMD_ARGS_ERR_VALIDATION_INVALID ), strListInvalid.c_str() ) );
		return MIstatus::failure;
	}
	
	return MIstatus::success;
}

//++ ------------------------------------------------------------------------------------
// Details:	Ask if the command's argument options text had any arguments.
// Type:	Method.
// Args:	None.
// Return:	bool	- True = Has one or more arguments present, false = no arguments.
// Throws:	None.
//--
bool CMICmdArgSet::IsArgContextEmpty( void ) const
{
	return m_cmdArgContext.IsEmpty();
}

//++ ------------------------------------------------------------------------------------
// Details:	Retrieve the number of arguments that are being used for the command.
// Type:	Method.
// Args:	None.
// Return:	MIuint - Argument count.
// Throws:	None.
//--
MIuint CMICmdArgSet::GetCount( void ) const
{
	return m_setCmdArgs.size();
}
	
//++ ------------------------------------------------------------------------------------
// Details:	Given a set of command argument objects retrieve the argument with the 
//			specified name.
// Type:	Method.
// Args:	vpArg	- (W) A pointer to a command's argument object.
// Return:	True - Argument found.
//			False - Argument not found.
// Throws:	None.
//--
bool CMICmdArgSet::GetArg( const CMIUtilString & vArgName, CMICmdArgValBase *& vpArg ) const
{
	bool bFound = false;
	SetCmdArgs_t::const_iterator it = m_setCmdArgs.begin();
	while( it != m_setCmdArgs.end() )
	{
		CMICmdArgValBase * pArg( *it );
		if( pArg->GetName() == vArgName )
		{
			bFound = true;
			vpArg = pArg;
			break;
		}
		
		// Next
		++it;
	}

	return bFound;
}
	
//++ ------------------------------------------------------------------------------------
// Details:	Write a warning message to the MI Log file about the command's arguments or 
//			options that were found present but not handled.
// Type:	Method.
// Args:	vrCmdName	- (R) The command's name.
// Return:	None.
// Throws:	None.
//--
void CMICmdArgSet::WarningArgsNotHandledbyCmdLogFile( const CMIUtilString & vrCmdName )
{
#if MICONFIG_GIVE_WARNING_CMD_ARGS_NOT_HANDLED

	CMIUtilString strArgsNotHandled;
	const CMICmdArgSet::SetCmdArgs_t & rSetArgs = GetArgsNotHandledByCmd();
	MIuint nCnt = 0;  
	CMICmdArgSet::SetCmdArgs_t::const_iterator it = rSetArgs.begin();
	while( it != rSetArgs.end() )
	{
		if( nCnt++ > 0 )
			strArgsNotHandled += m_constStrCommaSpc;
		const CMICmdArgValBase * pArg = *it;
		strArgsNotHandled += pArg->GetName();

		// Next
		++it;
	}

	const CMIUtilString strWarningMsg( CMIUtilString::Format( MIRSRC( IDS_CMD_WRN_ARGS_NOT_HANDLED ), vrCmdName.c_str(), strArgsNotHandled.c_str() ) );
	m_pLog->WriteLog( strWarningMsg );

#endif // MICONFIG_GIVE_WARNING_CMD_ARGS_NOT_HANDLED
}
