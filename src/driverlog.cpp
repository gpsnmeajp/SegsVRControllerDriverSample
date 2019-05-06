//============ Copyright (c) Valve Corporation, All rights reserved. ============

/*
https://github.com/ValveSoftware/openvr/blob/master/LICENSE

Copyright (c) 2015, Valve Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//ドライバログ出力

#include "driverlog.h"

#include <stdio.h>
#include <stdarg.h>

static vr::IVRDriverLog * s_pLogFile = NULL;

#if !defined( WIN32)
#define vsnprintf_s vsnprintf
#endif

//ログ出力を初期化
bool InitDriverLog( vr::IVRDriverLog *pDriverLog )
{
	if( s_pLogFile )
		return false;
	s_pLogFile = pDriverLog;
	return s_pLogFile != NULL;
}

//ログ出力をクリーンアップ
void CleanupDriverLog()
{
	s_pLogFile = NULL;
}

//ログへ出力する
static void DriverLogVarArgs( const char *pMsgFormat, va_list args )
{
	char buf[1024];
	vsnprintf_s( buf, sizeof(buf), pMsgFormat, args );

	if( s_pLogFile )
		s_pLogFile->Log( buf );
}

//ログをprintfのように出力する
void DriverLog( const char *pMsgFormat, ... )
{
	va_list args;
	va_start( args, pMsgFormat );

	DriverLogVarArgs( pMsgFormat, args );

	va_end(args);
}


void DebugDriverLog( const char *pMsgFormat, ... )
{
#ifdef _DEBUG
	va_list args;
	va_start( args, pMsgFormat );

	DriverLogVarArgs( pMsgFormat, args );

	va_end(args);
#endif
}


