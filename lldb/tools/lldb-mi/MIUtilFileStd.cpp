//===-- MIUtilFileStd.cpp ---------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//++
// File:		MIUtilFileStd.cpp
//
// Overview:	CMIUtilFileStd implementation.
//
// Environment:	Compilers:	Visual C++ 12.
//							gcc (Ubuntu/Linaro 4.8.1-10ubuntu9) 4.8.1
//				Libraries:	See MIReadmetxt. 
//
// Copyright:	None.
//--

// Include compiler configuration
#include "MICmnConfig.h"

// Third party headers
#include <stdio.h>
#include <assert.h>
#include <string.h>		// For strerror()
#include <cerrno>

// In-house headers:
#include "MIUtilFileStd.h"
#include "MICmnResources.h"

//++ ------------------------------------------------------------------------------------
// Details:	CMIUtilFileStd constructor.
// Type:	Method.
// Args:	None.
// Return:	None.
// Throws:	None.
//--
CMIUtilFileStd::CMIUtilFileStd( void )
:	m_fileNamePath( CMIUtilString() )
,	m_pFileHandle( nullptr )
#if defined( _MSC_VER )
,	m_constCharNewLine( "\r\n" )
#else
,	m_constCharNewLine( "\n" )
#endif // #if defined( _MSC_VER )
,	m_bFileError( false )
{
}

//++ ------------------------------------------------------------------------------------
// Details:	CMIUtilFileStd destructor.
// Type:	Method.
// Args:	None.
// Return:	None.
// Throws:	None.
//--
CMIUtilFileStd::~CMIUtilFileStd( void )
{
	Close();
}

//++ ------------------------------------------------------------------------------------
// Details:	Open file for writing. On the first call to this function after *this object
//			is created the file is either created or replace, from then on open only opens
//			an existing file.
// Type:	Method.
// Args:	vFileNamePath	- (R) File name path.
//			vwrbNewCreated	- (W) True - file recreated, false - file appended too.
// Return:	MIstatus::success - Functional succeeded.
//			MIstatus::failure - Functional failed.
// Throws:	None.
//--
bool CMIUtilFileStd::CreateWrite( const CMIUtilString & vFileNamePath, bool & vwrbNewCreated )
{
	// Reset
	m_bFileError = false;
	vwrbNewCreated = false;
		
	if( vFileNamePath.empty() )
	{
		m_bFileError = true;
		SetErrorDescription( MIRSRC( IDS_UTIL_FILE_ERR_INVALID_PATHNAME ) );
		return MIstatus::failure;
	}
	
	// File is already open so exit
	if( m_pFileHandle != nullptr )
		return MIstatus::success;

#if !defined( _MSC_VER )
	// Open with 'write' and 'binary' mode
	m_pFileHandle = ::fopen( vFileNamePath.c_str(), "wb" );
#else
	// Open a file with exclusive write and shared read permissions
	m_pFileHandle = ::_fsopen( vFileNamePath.c_str(), "wb", _SH_DENYWR );
#endif // !defined( _MSC_VER )

	if( m_pFileHandle == nullptr )
	{
		m_bFileError = true;
		SetErrorDescriptionn( MIRSRC( IDS_UTIL_FILE_ERR_OPENING_FILE ), strerror( errno ), vFileNamePath.c_str() );
		return MIstatus::failure;
	}
	
	vwrbNewCreated = true;
	m_fileNamePath = vFileNamePath;

	return MIstatus::success;
}

//++ ------------------------------------------------------------------------------------
// Details:	Write data to existing opened file.
// Type:	Method.
// Args:	vData - (R) Text data.
// Return:	MIstatus::success - Functional succeeded.
//			MIstatus::failure - Functional failed.
// Throws:	None.
//--
bool CMIUtilFileStd::Write( const CMIUtilString & vData )
{
	if( vData.size() == 0 )
		return MIstatus::success;

	if( m_bFileError )
		return MIstatus::failure;

	if( m_pFileHandle == nullptr )
	{
		m_bFileError = true;
		SetErrorDescriptionn( MIRSRC( IDE_UTIL_FILE_ERR_WRITING_NOTOPEN ), m_fileNamePath.c_str() );
		return MIstatus::failure;
	}

	// Get the string size
	MIuint size = vData.size();
	if( ::fwrite( vData.c_str(), 1, size, m_pFileHandle ) == size )
	{
		// Flush the data to the file
		::fflush( m_pFileHandle );
		return MIstatus::success;
	}
	
	// Not all of the data has been transfered
	m_bFileError = true;
	SetErrorDescriptionn( MIRSRC( IDE_UTIL_FILE_ERR_WRITING_FILE ), m_fileNamePath.c_str() );
	return MIstatus::failure;
}

//++ ------------------------------------------------------------------------------------
// Details:	Write data to existing opened file.
// Type:	Method.
// Args:	vData		- (R) Text data.
//			vCharCnt	- (R) Text data length.
// Return:	MIstatus::success - Functional succeeded.
//			MIstatus::failure - Functional failed.
// Throws:	None.
//--
bool CMIUtilFileStd::Write( const MIchar * vpData, const MIuint vCharCnt )
{
	if( vCharCnt == 0 )
		return MIstatus::success;

	if( m_bFileError )
		return MIstatus::failure;

	if( m_pFileHandle == nullptr )
	{
		m_bFileError = true;
		SetErrorDescriptionn( MIRSRC( IDE_UTIL_FILE_ERR_WRITING_NOTOPEN ), m_fileNamePath.c_str() );
		return MIstatus::failure;
	}

	if( ::fwrite( vpData, 1, vCharCnt, m_pFileHandle ) == vCharCnt )
	{
		// Flush the data to the file
		::fflush( m_pFileHandle );
		return MIstatus::success;
	}

	// Not all of the data has been transfered
	m_bFileError = true;
	SetErrorDescriptionn( MIRSRC( IDE_UTIL_FILE_ERR_WRITING_FILE ), m_fileNamePath.c_str() );
	return MIstatus::failure;
}
	
//++ ------------------------------------------------------------------------------------
// Details:	Close existing opened file. Note Close() must must an open!
// Type:	Method.
// Args:	None.
// Return:	None.
// Throws:	None.
//--
void CMIUtilFileStd::Close( void )
{
	if( m_pFileHandle == nullptr )
		return;
	
	::fclose( m_pFileHandle );
	m_pFileHandle = nullptr;
	//m_bFileError = false; Do not reset as want to remain until next attempt at open or create
}

//++ ------------------------------------------------------------------------------------
// Details:	Retrieve state of whether the file is ok.
// Type:	Method.
// Args:	None.
// Return:	True - file ok.
//			False - file has a problem.
// Throws:	None.
//--
bool CMIUtilFileStd::IsOk( void ) const
{
	return !m_bFileError;
}

//++ ------------------------------------------------------------------------------------
// Details:	Status on a file existing already.
// Type:	Method.
// Args:	vFileNamePath.
// Return:	True - Exists.
//			False - Not found.
// Throws:	None.
//--
bool CMIUtilFileStd::IsFileExist( const CMIUtilString & vFileNamePath ) const
{
	if( vFileNamePath.empty() )
		return false;

	FILE * pTmp = nullptr;
	pTmp = ::fopen( vFileNamePath.c_str(), "wb" );
	if( pTmp != nullptr )
	{
		::fclose( pTmp );
		return true;
	}

	return false;
}

//++ ------------------------------------------------------------------------------------
// Details:	Retrieve the file current carriage line return characters used.
// Type:	Method.
// Args:	None.
// Return:	CMIUtilString & - Text.
// Throws:	None.
//--
const CMIUtilString & CMIUtilFileStd::GetLineReturn( void ) const
{
	return m_constCharNewLine;
}

//++ ------------------------------------------------------------------------------------
// Details:	Given a file name directory path, strip off the filename and return the path.
//			It look for either backslash or forward slash.
// Type:	Method.
// Args:	vDirectoryPath	- (R) Text directory path.
// Return:	CMIUtilString - Directory path.
// Throws:	None.
//--
CMIUtilString CMIUtilFileStd::StripOffFileName( const CMIUtilString & vDirectoryPath ) const
{
	const MIint nPos = vDirectoryPath.rfind( '\\' );
	MIint nPos2 = vDirectoryPath.rfind( '/' );
	if( (nPos == (MIint) std::string::npos) && (nPos2 == (MIint) std::string::npos) )
		return vDirectoryPath;

	if( nPos > nPos2 )
		nPos2 = nPos;

	const CMIUtilString strPath( vDirectoryPath.substr( 0, nPos2 ).c_str() );
	return strPath;
}

//++ ------------------------------------------------------------------------------------
// Details:	Return either backslash or forward slash appropriate to the OS this applilcation
//			is running on.
// Type:	Static method.
// Args:	None.
// Return:	MIchar - '/' or '\' character.
// Throws:	None.
//--
MIchar CMIUtilFileStd::GetSlash( void )
{
#if !defined( _MSC_VER )
	return '/';
#else
	return '\\';
#endif // !defined( _MSC_VER )
}
