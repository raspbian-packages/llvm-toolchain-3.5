//===-- MICmnConfig.h -------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//++
// File:		CMIUtilSystemOsx.h
//
// Overview:	CMIUtilSystemOsx interface.
//
// Environment:	Compilers:	Visual C++ 12.
//							gcc (Ubuntu/Linaro 4.8.1-10ubuntu9) 4.8.1
//				Libraries:	See MIReadmetxt. 
//
// Copyright:	None.
//--

#pragma once

// Include compiler configuration
#include "MICmnConfig.h"

#if defined( __APPLE__ )

// In-house headers:
#include "MIUtilString.h"  

//++ ============================================================================
// Details:	MI common code utility class. Used to set or retrieve information
//			about the current system or user.
//			*** If you change, remove or add functionality it must be replicated
//			*** for the all platforms supported; Windows, OSX, LINUX 
// Gotchas:	None.
// Authors:	Illya Rudkin 29/01/2014.
// Changes:	None.
//--
class CMIUtilSystemOsx
{
// Methods:
public:
	/* ctor */	CMIUtilSystemOsx( void );
	
	bool			GetOSErrorMsg( const MIint vError, CMIUtilString & vrwErrorMsg ) const;
	CMIUtilString	GetOSLastError( void ) const;
	bool			GetExecutablesPath( CMIUtilString & vrwFileNamePath ) const;
	bool			GetLogFilesPath( CMIUtilString & vrwFileNamePath ) const;

// Overrideable:
public:
	// From CMICmnBase
	/* dtor */ virtual ~CMIUtilSystemOsx( void );
};

typedef CMIUtilSystemOsx CMIUtilSystem;

#endif // #if defined( __APPLE__ )
