/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2005 - 2015, ioquake3 contributors
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

// cl_main.c  -- client main loop

#include "client.h"

#include <limits.h>
#include "ghoul2/G2.h"
#include "qcommon/cm_public.h"
#include "qcommon/MiniHeap.h"
#include "qcommon/stringed_ingame.h"
#include "cl_cgameapi.h"
#include "cl_uiapi.h"
#include "cl_lan.h"
#include "snd_local.h"
#include "sys/sys_loadlib.h"
#include <string>

cvar_t *cl_name;

cvar_t	*cl_renderer;

cvar_t	*cl_nodelta;
cvar_t	*cl_debugMove;

cvar_t	*cl_noprint;
cvar_t	*cl_motd;
cvar_t	*cl_motdServer[MAX_MASTER_SERVERS];

cvar_t	*rcon_client_password;
cvar_t	*rconAddress;

cvar_t	*cl_timeout;
cvar_t	*cl_maxpackets;
cvar_t	*cl_packetdup;
#ifndef TOURNAMENT_CLIENT
cvar_t	*cl_timeNudge;
#endif
cvar_t	*cl_showTimeDelta;

cvar_t	*cl_shownet;
cvar_t	*cl_showSend;
cvar_t	*cl_timedemo;
cvar_t	*cl_aviFrameRate;
cvar_t	*cl_aviMotionJpeg;
cvar_t	*cl_avi2GBLimit;

cvar_t	*cl_forceavidemo;
#if JAMME_PIPES
cvar_t	*cl_aviPipe;
cvar_t	*cl_aviPipeCommand;
cvar_t	*cl_aviPipeContainer;
#endif

cvar_t	*cl_freelook;
cvar_t	*cl_sensitivity;

cvar_t	*cl_mouseAccel;
cvar_t	*cl_mouseAccelOffset;
cvar_t	*cl_mouseAccelStyle;
cvar_t	*cl_showMouseRate;

cvar_t	*m_pitchVeh;
cvar_t	*m_pitch;
cvar_t	*m_yaw;
cvar_t	*m_forward;
cvar_t	*m_side;
cvar_t	*m_filter;

cvar_t	*cl_activeAction;

cvar_t	*cl_motdString;

cvar_t	*cl_allowDownload;
cvar_t	*cl_allowAltEnter;
cvar_t	*cl_allowEnterCompletion;
cvar_t	*cl_conXOffset;
cvar_t	*cl_inGameVideo;

cvar_t	*cl_serverStatusResendTime;
cvar_t	*cl_framerate;

// cvar to enable sending a "ja_guid" player identifier in userinfo to servers
// ja_guid is a persistent "cookie" that allows servers to track players across game sessions
cvar_t	*cl_enableGuid;
cvar_t	*cl_guidServerUniq;

cvar_t	*cl_idrive; //JAPRO ENGINE

cvar_t	*protocolswitch;

cvar_t	*cl_autolodscale;

cvar_t	*cl_consoleKeys;
cvar_t	*cl_consoleUseScanCode;

cvar_t  *cl_lanForcePackets;

cvar_t	*cl_drawRecording;

//EternalJK
cvar_t	*cl_ratioFix;

cvar_t	*cl_colorString;
cvar_t	*cl_colorStringCount;
cvar_t	*cl_colorStringRandom;

cvar_t	*cl_chatStylePrefix;
cvar_t	*cl_chatStyleSuffix;

int		cl_unfocusedTime;
cvar_t	*cl_afkPrefix;
cvar_t	*cl_afkTime;
cvar_t	*cl_afkTimeUnfocused;

cvar_t	*cl_logChat;

#if defined(DISCORD) && defined(FINAL_BUILD)
cvar_t	*cl_discordRichPresence;
#endif

cvar_t	*cl_showSpoofAttackMessages;
cvar_t	*cl_antiSpoof;

vec3_t cl_windVec;


clientActive_t		cl;
clientConnection_t	clc;
clientStatic_t		cls;

netadr_t rcon_address;

char cl_reconnectArgs[MAX_OSPATH] = {0};

// Structure containing functions exported from refresh DLL
refexport_t	*re = NULL;
static void	*rendererLib = NULL;

ping_t	cl_pinglist[MAX_PINGREQUESTS];

typedef struct serverStatus_s
{
	char string[BIG_INFO_STRING];
	netadr_t address;
	int time, startTime;
	qboolean pending;
	qboolean print;
	qboolean retrieved;
} serverStatus_t;

serverStatus_t cl_serverStatusList[MAX_SERVERSTATUSREQUESTS] = { 0 };
int serverStatusCount = 0;

IHeapAllocator *G2VertSpaceClient = 0;

extern void SV_BotFrame( int time );
void CL_CheckForResend( void );
void CL_ShowIP_f(void);
void CL_ServerStatus_f(void);
void CL_ServerStatusResponse( netadr_t from, msg_t *msg );
static void CL_ShutdownRef( qboolean restarting );

/*
=======================================================================

CLIENT RELIABLE COMMAND COMMUNICATION

=======================================================================
*/

/*
======================
CL_AddReliableCommand

The given command will be transmitted to the server, and is gauranteed to
not have future usercmd_t executed before it is executed
======================
*/
void CL_AddReliableCommand( const char *cmd, qboolean isDisconnectCmd ) {
	int unacknowledged = clc.reliableSequence - clc.reliableAcknowledge;

	// if we would be losing an old command that hasn't been acknowledged,
	// we must drop the connection
	// also leave one slot open for the disconnect command in this case.

	if ((isDisconnectCmd && unacknowledged > MAX_RELIABLE_COMMANDS) ||
	    (!isDisconnectCmd && unacknowledged >= MAX_RELIABLE_COMMANDS))
	{
		if(com_errorEntered)
			return;
		else
			Com_Error(ERR_DROP, "Client command overflow");
	}

	Q_strncpyz(clc.reliableCommands[++clc.reliableSequence & (MAX_RELIABLE_COMMANDS - 1)],
		   cmd, sizeof(*clc.reliableCommands));
}

/*
=======================================================================

CLIENT SIDE DEMO RECORDING

=======================================================================
*/

/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length
====================
*/
void CL_WriteDemoMessage ( msg_t *msg, int headerBytes ) {
	int		len, swlen;

	// write the packet sequence
	len = clc.serverMessageSequence;
	swlen = LittleLong( len );
	FS_Write (&swlen, 4, clc.demofile);

	// skip the packet sequencing information
	len = msg->cursize - headerBytes;
	swlen = LittleLong(len);
	FS_Write (&swlen, 4, clc.demofile);
	FS_Write ( msg->data + headerBytes, len, clc.demofile );
}


/*
====================
CL_StopRecording_f

stop recording a demo
====================
*/
void CL_StopRecord_f( void ) {
	int		len;

	if ( !clc.demorecording ) {
		Com_Printf ("Not recording a demo.\n");
		return;
	}

	// finish up
	len = -1;
	FS_Write (&len, 4, clc.demofile);
	FS_Write (&len, 4, clc.demofile);
	FS_FCloseFile (clc.demofile);
	clc.demofile = 0;
	clc.demorecording = qfalse;
	clc.spDemoRecording = qfalse;
	Com_Printf ("Stopped demo.\n");
}

/*
==================
CL_DemoFilename
==================
*/
void CL_DemoFilename( char *buf, int bufSize ) {
	time_t rawtime;
	char timeStr[32] = {0}; // should really only reach ~19 chars

	time( &rawtime );
	strftime( timeStr, sizeof( timeStr ), "%Y-%m-%d_%H-%M-%S", localtime( &rawtime ) ); // or gmtime

	Com_sprintf( buf, bufSize, "demo%s", timeStr );
}

/*
====================
CL_Record_f

record <demoname>

Begins recording a demo from the current position
====================
*/
static char		demoName[MAX_QPATH];	// compiler bug workaround
void CL_Record_f( void ) {
	char		name[MAX_OSPATH], extension[32];
	byte		bufData[MAX_MSGLEN];
	msg_t	buf;
	int			i;
	int			len;
	entityState_t	*ent;
	entityState_t	nullstate;
	char		*s;

	if ( Cmd_Argc() > 2 ) {
		Com_Printf ("record <demoname>\n");
		return;
	}

	if ( clc.demorecording ) {
		if (!clc.spDemoRecording) {
			Com_Printf ("Already recording.\n");
		}
		return;
	}

	if ( cls.state != CA_ACTIVE ) {
		Com_Printf ("You must be in a level to record.\n");
		return;
	}

	// sync 0 doesn't prevent recording, so not forcing it off .. everyone does g_sync 1 ; record ; g_sync 0 ..
	//^kill yourself
	/*if ( NET_IsLocalAddress( clc.serverAddress ) && !Cvar_VariableValue( "g_synchronousClients" ) ) {
		Com_Printf (S_COLOR_YELLOW "WARNING: You should set 'g_synchronousClients 1' for smoother demo recording\n");
	}*/

	if (protocolswitch->integer != 2)
		Com_sprintf(extension, sizeof(extension), "dm_%d", PROTOCOL_VERSION);
	else
		Com_sprintf(extension, sizeof(extension), "dm_%d", PROTOCOL_LEGACY);

	if ( Cmd_Argc() == 2 ) {
		s = Cmd_Argv(1);
		Q_strncpyz( demoName, s, sizeof( demoName ) );
		Com_sprintf(name, sizeof(name), "demos/%s.%s", demoName, extension);
	} else {
		// timestamp the file
		CL_DemoFilename( demoName, sizeof( demoName ) );

		Com_sprintf(name, sizeof(name), "demos/%s.%s", demoName, extension);

		if ( FS_FileExists( name ) ) {
			Com_Printf( "Record: Couldn't create a file\n");
			return;
 		}
	}

	// open the demo file

	Com_Printf ("recording to %s.\n", name);
	clc.demofile = FS_FOpenFileWrite( name );
	if ( !clc.demofile ) {
		Com_Printf ("ERROR: couldn't open.\n");
		return;
	}
	clc.demorecording = qtrue;
	if (Cvar_VariableValue("ui_recordSPDemo")) {
	  clc.spDemoRecording = qtrue;
	} else {
	  clc.spDemoRecording = qfalse;
	}

	Q_strncpyz( clc.demoName, demoName, sizeof( clc.demoName ) );

	// don't start saving messages until a non-delta compressed message is received
	clc.demowaiting = qtrue;

	// write out the gamestate message
	MSG_Init (&buf, bufData, sizeof(bufData));
	MSG_Bitstream(&buf);

	// NOTE, MRE: all server->client messages now acknowledge
	MSG_WriteLong( &buf, clc.reliableSequence );

	MSG_WriteByte (&buf, svc_gamestate);
	MSG_WriteLong (&buf, clc.serverCommandSequence );

	// configstrings
	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		if ( !cl.gameState.stringOffsets[i] ) {
			continue;
		}
		s = cl.gameState.stringData + cl.gameState.stringOffsets[i];
		MSG_WriteByte (&buf, svc_configstring);
		MSG_WriteShort (&buf, i);
		MSG_WriteBigString (&buf, s);
	}

	// baselines
	Com_Memset (&nullstate, 0, sizeof(nullstate));
	for ( i = 0; i < MAX_GENTITIES ; i++ ) {
		ent = &cl.entityBaselines[i];
		if ( !ent->number ) {
			continue;
		}
		MSG_WriteByte (&buf, svc_baseline);
		MSG_WriteDeltaEntity (&buf, &nullstate, ent, qtrue );
	}

	MSG_WriteByte( &buf, svc_EOF );

	// finished writing the gamestate stuff

	// write the client num
	MSG_WriteLong(&buf, clc.clientNum);
	// write the checksum feed
	MSG_WriteLong(&buf, clc.checksumFeed);

	// Filler for old RMG system.
	MSG_WriteShort ( &buf, 0 );

	// finished writing the client packet
	MSG_WriteByte( &buf, svc_EOF );

	// write it to the demo file
	len = LittleLong( clc.serverMessageSequence - 1 );
	FS_Write (&len, 4, clc.demofile);

	len = LittleLong (buf.cursize);
	FS_Write (&len, 4, clc.demofile);
	FS_Write (buf.data, buf.cursize, clc.demofile);

	// the rest of the demo file will be copied from net messages
}

/*
=======================================================================

CLIENT SIDE DEMO PLAYBACK

=======================================================================
*/

/*
=================
CL_DemoCompleted
=================
*/
void CL_DemoCompleted( void ) {
	if (cl_timedemo && cl_timedemo->integer) {
		int	time;

		time = Sys_Milliseconds() - clc.timeDemoStart;
		if ( time > 0 ) {
			Com_Printf ("%i frames, %3.1f seconds: %3.1f fps\n", clc.timeDemoFrames,
			time/1000.0, clc.timeDemoFrames*1000.0 / time);
		}
	}

/*	CL_Disconnect( qtrue );
	CL_NextDemo();
	*/

	//rww - The above code seems to just stick you in a no-menu state and you can't do anything there.
	//I'm not sure why it ever worked in TA, but whatever. This code will bring us back to the main menu
	//after a demo is finished playing instead.
	CL_Disconnect_f();
	S_StopAllSounds();
	UIVM_SetActiveMenu( UIMENU_MAIN );

	CL_NextDemo();
}

/*
=================
CL_ReadDemoMessage
=================
*/
void CL_ReadDemoMessage( void ) {
	int			r;
	msg_t		buf;
	byte		bufData[ MAX_MSGLEN ];
	int			s;

	if ( !clc.demofile ) {
		CL_DemoCompleted ();
		return;
	}

	// get the sequence number
	r = FS_Read( &s, 4, clc.demofile);
	if ( r != 4 ) {
		CL_DemoCompleted ();
		return;
	}
	clc.serverMessageSequence = LittleLong( s );

	// init the message
	MSG_Init( &buf, bufData, sizeof( bufData ) );

	// get the length
	r = FS_Read (&buf.cursize, 4, clc.demofile);
	if ( r != 4 ) {
		CL_DemoCompleted ();
		return;
	}
	buf.cursize = LittleLong( buf.cursize );
	if ( buf.cursize == -1 ) {
		CL_DemoCompleted ();
		return;
	}
	if ( buf.cursize > buf.maxsize ) {
		Com_Error (ERR_DROP, "CL_ReadDemoMessage: demoMsglen > MAX_MSGLEN");
	}
	r = FS_Read( buf.data, buf.cursize, clc.demofile );
	if ( r != buf.cursize ) {
		Com_Printf( "Demo file was truncated.\n");
		CL_DemoCompleted ();
		return;
	}

	clc.lastPacketTime = cls.realtime;
	buf.readcount = 0;
	CL_ParseServerMessage( &buf );
}

/*
====================
CL_CompleteDemoName
====================
*/
static void CL_CompleteDemoName( char *args, int argNum )
{
	if( argNum == 2 )
	{
		char demoExtension[8] = {0}, demoExtensionLegacy[8] = {0};

		//this might be weird if someone has a bunch of 1.01 demos and one 1.00 demo, or vice versa
		//need to check for both extensions @ the same time somehow
		Com_sprintf(demoExtensionLegacy, sizeof(demoExtensionLegacy), ".dm_%d", PROTOCOL_LEGACY);
		Field_CompleteFilename("demos", demoExtensionLegacy, qfalse , qtrue);

		Com_sprintf(demoExtension, sizeof(demoExtension), ".dm_%d", PROTOCOL_VERSION);
		Field_CompleteFilename("demos", demoExtension, qfalse, qtrue);
	}
}

/*
====================
CL_PlayDemo_f

demo <demoname>

====================
*/
static void CL_PlayDemo_f( void ) {
	char		name[MAX_STRING_CHARS] = {0};
	char		*arg = NULL, *s = NULL, *demoExtension = NULL, *demoExtensionLegacy = NULL;
	const char	*foundExtension = NULL, *foundSlash = NULL;

	if (Cmd_Argc() < 2) {
		Com_Printf ("demo <demoname>\n");
		return;
	}

	// make sure a local server is killed
	// 2 means don't force disconnect of local client
	Cvar_Set( "sv_killserver", "2" );

	// open the demo file
	arg = Cmd_Args();

	CL_Disconnect( qtrue );

	if (!VALIDSTRING(arg)) {
		Com_Error(ERR_DROP, "couldn't open file");
		return;
	}

	foundSlash = strchr(arg, '/');
	if (Q_stricmpn(arg, "demos/", 6) && (!foundSlash || !VALIDSTRING(foundSlash))) //check for an explicit path
		s = va("demos/%s", arg);

	if (!s || !VALIDSTRING(s))
		s = arg;

	foundExtension = Q_stristr(s, ".dm_");
	demoExtension = va(".dm_%i", PROTOCOL_VERSION);
	demoExtensionLegacy = va(".dm_%i", PROTOCOL_LEGACY);

	if (foundExtension && VALIDSTRING(foundExtension) && (!Q_stricmp(foundExtension, demoExtension) || !Q_stricmp(foundExtension, demoExtensionLegacy)))
	{ //extension was included in the demo's name
		Q_strncpyz(name, s, sizeof(name));
	}
	else
	{ //have to figure it out ourselves
		//look for the old protocol first
		Com_sprintf(name, sizeof(name), "%s%s", s, demoExtensionLegacy);

		if (!FS_FileExists(name)) //try again w/ normal protocol
			Com_sprintf(name, sizeof(name), "%s%s", s, demoExtension);
	}

	FS_FOpenFileRead( name, &clc.demofile, qtrue );
	if (!clc.demofile) {
		if (!Q_stricmp(arg, "(null)"))
		{
			Com_Error( ERR_DROP, SE_GetString("CON_TEXT_NO_DEMO_SELECTED") );
		}
		else
		{
			Com_Error( ERR_DROP, "couldn't open %s", name);
		}
		return;
	}
	Q_strncpyz( clc.demoName, name, sizeof( clc.demoName ) );

	Con_Close();

	cls.state = CA_CONNECTED;
	clc.demoplaying = qtrue;
	Q_strncpyz( cls.servername, Cmd_Argv(1), sizeof( cls.servername ) );

	Cvar_Set("protocolswitch", "1");

	// read demo messages until connected
	while ( cls.state >= CA_CONNECTED && cls.state < CA_PRIMED ) {
		CL_ReadDemoMessage();
	}
	// don't get the first snapshot this frame, to prevent the long
	// time from the gamestate load from messing causing a time skip
	clc.firstDemoFrameSkipped = qfalse;
}

static void CL_DemoRestart_f(void) {
	char *demoname = clc.demoName;
	if (!VALIDSTRING(demoname)) {
		Com_Printf("No demo available to restart.\n");
		return;
	}

	Com_Printf("Restarting demo \"%s\"\n", demoname);
	Cbuf_ExecuteText(EXEC_APPEND, va("demo \"%s\"\n", demoname));
}

static void CL_DelDemo_f(void) {
	char		name[MAX_STRING_CHARS] = {0};
	char		*arg = NULL, *s = NULL, *demoExtension = NULL, *demoExtensionLegacy = NULL;
	const char	*foundExtension = NULL, *foundSlash = NULL;

	if (Cmd_Argc() < 2) {
		Com_Printf("deletedemo <demoname>\n");
		return;
	}

	arg = Cmd_Args();

	if (!VALIDSTRING(arg)) {
		Com_Error(ERR_DROP, "couldn't open file");
		return;
	}

	foundSlash = strchr(arg, '/');
	if (foundSlash && VALIDSTRING(foundSlash) && Q_stricmpn(arg, "demos/", 6)) //only allow deletion of demos inside of demos folder
		return;

	if (!s || !VALIDSTRING(s))
		s = arg;

	foundExtension = Q_stristr(s, ".dm_");
	demoExtension = va(".dm_%i", PROTOCOL_VERSION);
	demoExtensionLegacy = va(".dm_%i", PROTOCOL_LEGACY);

	if (foundExtension && VALIDSTRING(foundExtension) && (!Q_stricmp(foundExtension, demoExtension) || !Q_stricmp(foundExtension, demoExtensionLegacy)))
	{ //extension was included in the demo's name
		Q_strncpyz(name, s, sizeof(name));
	}
	else
	{ //have to figure it out ourselves
		//look for the old protocol first
		Com_sprintf(name, sizeof(name), "%s%s", s, demoExtensionLegacy);

		if (!FS_FileExists(name)) {//try again w/ normal protocol
			Com_sprintf(name, sizeof(name), "%s%s", s, demoExtension);
		}

		if (!FS_FileExists(name)) {
			Com_Printf(S_COLOR_RED "Can't find demofile %s\n", name);
			return;
		}
	}

	FS_HomeRemove(name);
	Com_Printf(S_COLOR_YELLOW "Deleted demofile %s\n", name);
}


/*
====================
CL_StartDemoLoop

Closing the main menu will restart the demo loop
====================
*/
void CL_StartDemoLoop( void ) {
	// start the demo loop again
	Cbuf_AddText ("d1\n");
	Key_SetCatcher( 0 );
}

/*
==================
CL_NextDemo

Called when a demo or cinematic finishes
If the "nextdemo" cvar is set, that command will be issued
==================
*/
void CL_NextDemo( void ) {
	char	v[MAX_STRING_CHARS];

	Q_strncpyz( v, Cvar_VariableString ("nextdemo"), sizeof(v) );
	v[MAX_STRING_CHARS-1] = 0;
	Com_DPrintf("CL_NextDemo: %s\n", v );
	if (!v[0]) {
		return;
	}

	Cvar_Set ("nextdemo","");
	Cbuf_AddText (v);
	Cbuf_AddText ("\n");
	Cbuf_Execute();
}

//======================================================================

/*
=====================
CL_ShutdownAll
=====================
*/
void CL_ShutdownAll( qboolean shutdownRef ) {
	if(CL_VideoRecording())
		CL_CloseAVI();

	if(clc.demorecording)
		CL_StopRecord_f();

#if 0 //rwwFIXMEFIXME: Disable this before release!!!!!! I am just trying to find a crash bug.
	//so it doesn't barf on shutdown saying refentities belong to each other
	tr.refdef.num_entities = 0;
#endif

	// clear sounds
	S_DisableSounds();
	// shutdown CGame
	CL_ShutdownCGame();
	// shutdown UI
	CL_ShutdownUI();

	// shutdown the renderer
	if(shutdownRef)
		CL_ShutdownRef( qfalse );
	if ( re && re->Shutdown ) {
		re->Shutdown( qfalse, qfalse );		// don't destroy window or context
	}

	cls.uiStarted = qfalse;
	cls.cgameStarted = qfalse;
	cls.rendererStarted = qfalse;
	cls.soundRegistered = qfalse;
}

/*
=================
CL_FlushMemory

Called by CL_MapLoading, CL_Connect_f, CL_PlayDemo_f, and CL_ParseGamestate the only
ways a client gets into a game
Also called by Com_Error
=================
*/
void CL_FlushMemory( void ) {

	// shutdown all the client stuff
	CL_ShutdownAll( qfalse );

	// if not running a server clear the whole hunk
	if ( !com_sv_running->integer ) {
		// clear collision map data
		CM_ClearMap();
		// clear the whole hunk
		Hunk_Clear();
	}
	else {
		// clear all the client data on the hunk
		Hunk_ClearToMark();
	}

	CL_StartHunkUsers();
}

/*
=====================
CL_MapLoading

A local server is starting to load a map, so update the
screen to let the user know about it, then dump all client
memory on the hunk from cgame, ui, and renderer
=====================
*/
void CL_MapLoading( void ) {
	if ( !com_cl_running->integer ) {
		return;
	}

	// Set this to localhost.
	Cvar_Set( "cl_currentServerAddress", "Localhost");
	Cvar_Set( "cl_currentServerIP", "loopback");

	Con_Close();
	Key_SetCatcher( 0 );

	// if we are already connected to the local host, stay connected
	if ( cls.state >= CA_CONNECTED && !Q_stricmp( cls.servername, "localhost" ) ) {
		cls.state = CA_CONNECTED;		// so the connect screen is drawn
		Com_Memset( cls.updateInfoString, 0, sizeof( cls.updateInfoString ) );
		Com_Memset( clc.serverMessage, 0, sizeof( clc.serverMessage ) );
		Com_Memset( &cl.gameState, 0, sizeof( cl.gameState ) );
		clc.lastPacketSentTime = -9999;
		SCR_UpdateScreen();
	} else {
		// clear nextmap so the cinematic shutdown doesn't execute it
		Cvar_Set( "nextmap", "" );
		CL_Disconnect( qtrue );
		Q_strncpyz( cls.servername, "localhost", sizeof(cls.servername) );
		cls.state = CA_CHALLENGING;		// so the connect screen is drawn
		Key_SetCatcher( 0 );
		SCR_UpdateScreen();
		clc.connectTime = -RETRANSMIT_TIMEOUT;
		NET_StringToAdr( cls.servername, &clc.serverAddress);
		// we don't need a challenge on the localhost

		CL_CheckForResend();
	}
}

/*
=====================
CL_ClearState

Called before parsing a gamestate
=====================
*/
void CL_ClearState (void) {

//	S_StopAllSounds();
	Com_Memset( &cl, 0, sizeof( cl ) );
}

/*
====================
CL_UpdateGUID

update cl_guid using QKEY_FILE and optional prefix
====================
*/
static void CL_UpdateGUID( const char *prefix, int prefix_len )
{
	if (cl_enableGuid->integer) {
		fileHandle_t f;
		int len;

		len = FS_SV_FOpenFileRead( QKEY_FILE, &f );
		FS_FCloseFile( f );

		// initialize the cvar here in case it's unset or was user-created
		// while tracking was disabled (removes CVAR_USER_CREATED)
		Cvar_Get( "ja_guid", "", CVAR_USERINFO | CVAR_ROM, "Client GUID" );

		if( len != QKEY_SIZE ) {
			Cvar_Set( "ja_guid", "" );
		} else {
			Cvar_Set( "ja_guid", Com_MD5File( QKEY_FILE, QKEY_SIZE,
				prefix, prefix_len ) );
		}
	} else {
		// Remove the cvar entirely if tracking is disabled
		uint32_t flags = Cvar_Flags("ja_guid");
		// keep the cvar if it's user-created, but destroy it otherwise
		if (flags != CVAR_NONEXISTENT && !(flags & CVAR_USER_CREATED)) {
			cvar_t *ja_guid = Cvar_Get("ja_guid", "", 0, "Client GUID" );
			Cvar_Unset(ja_guid);
		}
	}
}

/*
=====================
CL_Disconnect

Called when a connection, demo, or cinematic is being terminated.
Goes from a connected state to either a menu state or a console state
Sends a disconnect message to the server
This is also called on Com_Error and Com_Quit, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect( qboolean showMainMenu ) {
	if ( !com_cl_running || !com_cl_running->integer ) {
		return;
	}

	// shutting down the client so enter full screen ui mode
	Cvar_Set("r_uiFullScreen", "1");

	if ( clc.demorecording ) {
		CL_StopRecord_f ();
	}

	if (clc.download) {
		FS_FCloseFile( clc.download );
		clc.download = 0;
	}
	*clc.downloadTempName = *clc.downloadName = 0;
	Cvar_Set( "cl_downloadName", "" );

	if ( clc.demofile ) {
		FS_FCloseFile( clc.demofile );
		clc.demofile = 0;
	}

	if ( cls.uiStarted && showMainMenu ) {
		UIVM_SetActiveMenu( UIMENU_NONE );
	}

	SCR_StopCinematic ();
	S_ClearSoundBuffer();

	// send a disconnect message to the server
	// send it a few times in case one is dropped
	if ( cls.state >= CA_CONNECTED ) {
		CL_AddReliableCommand( "disconnect", qtrue );
		CL_WritePacket();
		CL_WritePacket();
		CL_WritePacket();
	}

	// Remove pure paks
	FS_PureServerSetLoadedPaks("", "");
	FS_PureServerSetReferencedPaks("", "");

	CL_ClearState ();

	// wipe the client connection
	Com_Memset( &clc, 0, sizeof( clc ) );

	cls.state = CA_DISCONNECTED;

	Cvar_Set("protocolswitch", "0");
	cls.JKVersion = VERSION_1_01;

	// allow cheats locally
	Cvar_Set( "sv_cheats", "1" );

	// not connected to a pure server anymore
	cl_connectedToPureServer = qfalse;

	// Stop recording any video
	if( CL_VideoRecording( ) ) {
		// Finish rendering current frame
		SCR_UpdateScreen( );
		CL_CloseAVI( );
	}

	CL_UpdateGUID( NULL, 0 );
}


/*
===================
CL_ForwardCommandToServer

adds the current command line as a clientCommand
things like godmode, noclip, etc, are commands directed to the server,
so when they are typed in at the console, they will need to be forwarded.
===================
*/
void CL_ForwardCommandToServer( const char *string ) {
	char	*cmd;

	cmd = Cmd_Argv(0);

	// ignore key up commands
	if ( cmd[0] == '-' ) {
		return;
	}

	if (clc.demoplaying || cls.state < CA_CONNECTED || cmd[0] == '+' ) {
		Com_Printf ("Unknown command \"%s" S_COLOR_WHITE "\"\n", cmd);
		return;
	}

	if ( Cmd_Argc() > 1 ) {
		CL_AddReliableCommand( string, qfalse );
	} else {
		CL_AddReliableCommand( cmd, qfalse );
	}
}

/*
===================
CL_RequestMotd

===================
*/
void CL_RequestMotd( void ) {
	netadr_t	to;
	int			i;
	char		command[MAX_STRING_CHARS], info[MAX_INFO_STRING];
	char		*motdaddress;

	if ( !cl_motd->integer ) {
		return;
	}

	if ( cl_motd->integer < 1 || cl_motd->integer > MAX_MASTER_SERVERS ) {
		Com_Printf( "CL_RequestMotd: Invalid motd server num. Valid values are 1-%d or 0 to disable\n", MAX_MASTER_SERVERS );
		return;
	}

	Com_sprintf( command, sizeof(command), "cl_motdServer%d", cl_motd->integer );
	motdaddress = Cvar_VariableString( command );

	if ( !*motdaddress )
	{
		Com_Printf( "CL_RequestMotd: Error: No motd server address given.\n" );
		return;
	}

	i = NET_StringToAdr( motdaddress, &to );

	if ( !i )
	{
		Com_Printf( "CL_RequestMotd: Error: could not resolve address of motd server %s\n", motdaddress );
		return;
	}
	to.type = NA_IP;
	to.port = BigShort( PORT_UPDATE );

	Com_Printf( "Requesting motd from update %s (%s)...\n", motdaddress, NET_AdrToString( to ) );

	cls.updateServer = to;

	info[0] = 0;
  // NOTE TTimo xoring against Com_Milliseconds, otherwise we may not have a true randomization
  // only srand I could catch before here is tr_noise.c l:26 srand(1001)
  // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=382
  // NOTE: the Com_Milliseconds xoring only affects the lower 16-bit word,
  //   but I decided it was enough randomization
	Com_sprintf( cls.updateChallenge, sizeof( cls.updateChallenge ), "%i", ((rand() << 16) ^ rand()) ^ Com_Milliseconds());

	Info_SetValueForKey( info, "challenge", cls.updateChallenge );
	Info_SetValueForKey( info, "renderer", cls.glconfig.renderer_string );
	Info_SetValueForKey( info, "rvendor", cls.glconfig.vendor_string );
	Info_SetValueForKey( info, "version", com_version->string );

	//If raven starts filtering for this, add this code back in
#if 0
	Info_SetValueForKey( info, "cputype", "Intel Pentium IV");
	Info_SetValueForKey( info, "mhz", "3000" );
	Info_SetValueForKey( info, "memory", "4096" );
#endif
	Info_SetValueForKey( info, "joystick", Cvar_VariableString("in_joystick") );
	Info_SetValueForKey( info, "colorbits", va("%d",cls.glconfig.colorBits) );

	NET_OutOfBandPrint( NS_CLIENT, cls.updateServer, "getmotd \"%s\"\n", info );
}


/*
======================================================================

CONSOLE COMMANDS

======================================================================
*/

/*
==================
CL_ForwardToServer_f
==================
*/
void CL_ForwardToServer_f( void ) {
	if ( cls.state != CA_ACTIVE || clc.demoplaying ) {
		Com_Printf ("Not connected to a server.\n");
		return;
	}

	// don't forward the first argument
	if ( Cmd_Argc() > 1 ) {
		CL_AddReliableCommand( Cmd_Args(), qfalse );
	}
}


/*
==================
CL_Disconnect_f
==================
*/
void CL_Disconnect_f( void ) {
	SCR_StopCinematic();
	Cvar_Set("ui_singlePlayerActive", "0");
	if ( cls.state != CA_DISCONNECTED && cls.state != CA_CINEMATIC ) {
		Com_Error (ERR_DISCONNECT, "Disconnected from server");
	}
}


/*
================
CL_Reconnect_f

================
*/
void CL_Reconnect_f( void ) {
	if ( !strlen( cl_reconnectArgs ) ) {
		return;
	}
	Cvar_Set("ui_singlePlayerActive", "0");
	Cbuf_AddText( va("connect %s\n", cl_reconnectArgs ) );
}

/*
================
CL_Connect_f

================
*/
void CL_Connect_f( void ) {
	char	*server;
	const char	*serverString;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "usage: connect [server]\n");
		return;
	}

	// save arguments for reconnect
	Q_strncpyz( cl_reconnectArgs, Cmd_Args(), sizeof( cl_reconnectArgs ) );

	Cvar_Set("ui_singlePlayerActive", "0");

	// fire a message off to the motd server
	CL_RequestMotd();

	// clear any previous "server full" type messages
	clc.serverMessage[0] = 0;

	server = Cmd_Argv (1);

	if ( com_sv_running->integer && !strcmp( server, "localhost" ) ) {
		// if running a local server, kill it
		SV_Shutdown( "Server quit\n" );
	}

	// make sure a local server is killed
	Cvar_Set( "sv_killserver", "1" );
	SV_Frame( 0 );

	CL_Disconnect( qtrue );
	Con_Close();

	Q_strncpyz( cls.servername, server, sizeof(cls.servername) );

	if (!NET_StringToAdr( cls.servername, &clc.serverAddress) ) {
		Com_Printf ("Bad server address\n");
		cls.state = CA_DISCONNECTED;
		return;
	}
	if (clc.serverAddress.port == 0) {
		clc.serverAddress.port = BigShort( PORT_SERVER );
	}

	serverString = NET_AdrToString(clc.serverAddress);

	Com_Printf( "%s resolved to %s\n", cls.servername, serverString );

	if( cl_guidServerUniq->integer )
		CL_UpdateGUID( serverString, strlen( serverString ) );
	else
		CL_UpdateGUID( NULL, 0 );

	// if we aren't playing on a lan, we need to authenticate
	if ( NET_IsLocalAddress( clc.serverAddress ) ) {
		cls.state = CA_CHALLENGING;
	} else {
		cls.state = CA_CONNECTING;

		if (cl_antiSpoof->integer)
		{
			//set unique number here
			clc.unique = ((rand() << 14) ^ rand()) ^ Com_Milliseconds();
			clc.uniqueAssigned = false;
		}
		// Set a client challenge number that ideally is mirrored back by the server.
		clc.challenge = ((rand() << 16) ^ rand()) ^ Com_Milliseconds();
	}

	Key_SetCatcher( 0 );
	clc.connectTime = -99999;	// CL_CheckForResend() will fire immediately
	clc.connectPacketCount = 0;

	// server connection string
	Cvar_Set( "cl_currentServerAddress", server );
	Cvar_Set( "cl_currentServerIP", serverString );
}

#define MAX_RCON_MESSAGE 1024

/*
==================
CL_CompleteRcon
==================
*/
static void CL_CompleteRcon( char *args, int argNum )
{
	if( argNum == 2 )
	{
		// Skip "rcon "
		char *p = Com_SkipTokens( args, 1, " " );

		if( p > args )
			Field_CompleteCommand( p, qtrue, qtrue );
	}
}

/*
=====================
CL_Rcon_f

  Send the rest of the command line over as
  an unconnected command.
=====================
*/
void CL_Rcon_f( void ) {
	char	message[MAX_RCON_MESSAGE];

	if ( !rcon_client_password->string[0] ) {
		Com_Printf( "You must set 'rconpassword' before issuing an rcon command.\n" );
		return;
	}

	message[0] = -1;
	message[1] = -1;
	message[2] = -1;
	message[3] = -1;
	message[4] = 0;

	Q_strcat (message, MAX_RCON_MESSAGE, "rcon ");

	Q_strcat (message, MAX_RCON_MESSAGE, rcon_client_password->string);
	Q_strcat (message, MAX_RCON_MESSAGE, " ");

	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=543
	Q_strcat (message, MAX_RCON_MESSAGE, Cmd_Cmd()+5);

	if ( cls.state >= CA_CONNECTED ) {
		rcon_address = clc.netchan.remoteAddress;
	} else {
		if (!strlen(rconAddress->string)) {
			Com_Printf ("You must either be connected,\n"
						"or set the 'rconAddress' cvar\n"
						"to issue rcon commands\n");

			return;
		}
		NET_StringToAdr (rconAddress->string, &rcon_address);
		if (rcon_address.port == 0) {
			rcon_address.port = BigShort (PORT_SERVER);
		}
	}

	NET_SendPacket (NS_CLIENT, strlen(message)+1, message, rcon_address);
}

/*
=================
CL_SendPureChecksums
=================
*/
void CL_SendPureChecksums( void ) {
	char cMsg[MAX_INFO_VALUE];

	// if we are pure we need to send back a command with our referenced pk3 checksums
	Com_sprintf(cMsg, sizeof(cMsg), "cp %s", FS_ReferencedPakPureChecksums());

	CL_AddReliableCommand( cMsg, qfalse );
}

/*
=================
CL_ResetPureClientAtServer
=================
*/
void CL_ResetPureClientAtServer( void ) {
	CL_AddReliableCommand( "vdr", qfalse );
}

void CL_Mod_Restart_f(void) {

	if (Cmd_Argc() != 2) {
		Com_Printf("Usage: loadmod <folder name>\n");
		return;
	}
	Cvar_Set("fs_game", Cmd_Argv(1));
	CL_Vid_Restart_f();
}

/*
=================
CL_Vid_Restart_f

Restart the video subsystem

we also have to reload the UI and CGame because the renderer
doesn't know what graphics to reload
=================
*/
extern bool g_nOverrideChecked;
void CL_Vid_Restart_f( void ) {
	// Settings may have changed so stop recording now
	if( CL_VideoRecording( ) ) {
		CL_CloseAVI( );
	}

	if(clc.demorecording)
		CL_StopRecord_f();

	//rww - sort of nasty, but when a user selects a mod
	//from the menu all it does is a vid_restart, so we
	//have to check for new net overrides for the mod then.
	g_nOverrideChecked = false;

	// don't let them loop during the restart
	S_StopAllSounds();
	// shutdown the UI
	CL_ShutdownUI();
	// shutdown the CGame
	CL_ShutdownCGame();
	// shutdown the renderer and clear the renderer interface
	CL_ShutdownRef( qtrue );
	// client is no longer pure untill new checksums are sent
	CL_ResetPureClientAtServer();
	// clear pak references
	FS_ClearPakReferences( FS_UI_REF | FS_CGAME_REF );
	// reinitialize the filesystem if the game directory or checksum has changed
	FS_ConditionalRestart( clc.checksumFeed );

	cls.rendererStarted = qfalse;
	cls.uiStarted = qfalse;
	cls.cgameStarted = qfalse;
	cls.soundRegistered = qfalse;

	// unpause so the cgame definately gets a snapshot and renders a frame
	Cvar_Set( "cl_paused", "0" );

	// if not running a server clear the whole hunk
	if ( !com_sv_running->integer ) {
		CM_ClearMap();
		// clear the whole hunk
		Hunk_Clear();
	}
	else {
		// clear all the client data on the hunk
		Hunk_ClearToMark();
	}

	// initialize the renderer interface
	CL_InitRef();

	// startup all the client stuff
	CL_StartHunkUsers();

	// start the cgame if connected
	if ( cls.state > CA_CONNECTED && cls.state != CA_CINEMATIC ) {
		cls.cgameStarted = qtrue;
		CL_InitCGame();
		// send pure checksums
		CL_SendPureChecksums();
	}
}

/*
=================
CL_Snd_Restart_f

Restart the sound subsystem
The cgame and game must also be forced to restart because
handles will be invalid
=================
*/
// extern void S_UnCacheDynamicMusic( void );
void CL_Snd_Restart_f( void ) {
	S_Shutdown();
	S_Init();

//	S_FreeAllSFXMem();			// These two removed by BTO (VV)
//	S_UnCacheDynamicMusic();	// S_Shutdown() already does this!

//	CL_Vid_Restart_f();

	extern qboolean	s_soundMuted;
	s_soundMuted = qfalse;		// we can play again

	if (snd_mute_losefocus->integer && (com_unfocused->integer || com_minimized->integer))
		s_soundMuted = qtrue;

	extern void S_RestartMusic( void );
	S_RestartMusic();

//#if defined(DISCORD) && !defined(_DEBUG)
#if defined(DISCORD) && defined(FINAL_BUILD)
	cls.discordNotificationSound = S_RegisterSound("sound/interface/secret_area.mp3");
#endif
}


/*
==================
CL_PK3List_f
==================
*/
void CL_OpenedPK3List_f( void ) {
	Com_Printf("Opened PK3 Names: %s\n", FS_LoadedPakNames());
}

/*
==================
CL_PureList_f
==================
*/
void CL_ReferencedPK3List_f( void ) {
	Com_Printf("Referenced PK3 Names: %s\n", FS_ReferencedPakNames());
}

/*
==================
CL_Configstrings_f
==================
*/
void CL_Configstrings_f( void ) {
	int		i;
	int		ofs;

	if ( cls.state != CA_ACTIVE ) {
		Com_Printf( "Not connected to a server.\n");
		return;
	}

	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		ofs = cl.gameState.stringOffsets[ i ];
		if ( !ofs ) {
			continue;
		}
		Com_Printf( "%4i: %s\n", i, cl.gameState.stringData + ofs );
	}
}

/*
==============
CL_Clientinfo_f
==============
*/
void CL_Clientinfo_f( void ) {
	Com_Printf( "--------- Client Information ---------\n" );
	Com_Printf( "state: %i\n", cls.state );
	Com_Printf( "Server: %s\n", cls.servername );
	Com_Printf ("User info settings:\n");
	Info_Print( Cvar_InfoString( CVAR_USERINFO ) );
	Com_Printf( "--------------------------------------\n" );
}


//====================================================================

/*
=================
CL_DownloadsComplete

Called when all downloading has been completed
=================
*/
void CL_DownloadsComplete( void ) {

	// if we downloaded files we need to restart the file system
	if (clc.downloadRestart) {
		clc.downloadRestart = qfalse;

		FS_Restart(clc.checksumFeed); // We possibly downloaded a pak, restart the file system to load it

		// inform the server so we get new gamestate info
		CL_AddReliableCommand( "donedl", qfalse );

		// by sending the donedl command we request a new gamestate
		// so we don't want to load stuff yet
		return;
	}

	// let the client game init and load data
	cls.state = CA_LOADING;

	// Pump the loop, this may change gamestate!
	Com_EventLoop();

	// if the gamestate was changed by calling Com_EventLoop
	// then we loaded everything already and we don't want to do it again.
	if ( cls.state != CA_LOADING ) {
		return;
	}

	// starting to load a map so we get out of full screen ui mode
	Cvar_Set("r_uiFullScreen", "0");

	// flush client memory and start loading stuff
	// this will also (re)load the UI
	// if this is a local client then only the client part of the hunk
	// will be cleared, note that this is done after the hunk mark has been set
	CL_FlushMemory();

	// initialize the CGame
	cls.cgameStarted = qtrue;
	CL_InitCGame();

	// set pure checksums
	CL_SendPureChecksums();

	CL_WritePacket();
	CL_WritePacket();
	CL_WritePacket();
}

/*
=================
CL_BeginDownload

Requests a file to download from the server.  Stores it in the current
game directory.
=================
*/

void CL_BeginDownload( const char *localName, const char *remoteName ) {

	Com_DPrintf("***** CL_BeginDownload *****\n"
				"Localname: %s\n"
				"Remotename: %s\n"
				"****************************\n", localName, remoteName);

	Q_strncpyz ( clc.downloadName, localName, sizeof(clc.downloadName) );
	Com_sprintf( clc.downloadTempName, sizeof(clc.downloadTempName), "%s.tmp", localName );

	// Set so UI gets access to it
	Cvar_Set( "cl_downloadName", remoteName );
	Cvar_Set( "cl_downloadSize", "0" );
	Cvar_Set( "cl_downloadCount", "0" );
	Cvar_SetValue( "cl_downloadTime", (float) cls.realtime );

	clc.downloadBlock = 0; // Starting new file
	clc.downloadCount = 0;

	CL_AddReliableCommand( va("download %s", remoteName), qfalse );
}

/*
=================
CL_NextDownload

A download completed or failed
=================
*/
void CL_NextDownload(void) {
	char *s;
	char *remoteName, *localName;

	// A download has finished, check whether this matches a referenced checksum
	if(*clc.downloadName)
	{
		char *zippath = FS_BuildOSPath(Cvar_VariableString("fs_homepath"), clc.downloadName, "");
		zippath[strlen(zippath)-1] = '\0';

		if(!FS_CompareZipChecksum(zippath))
			Com_Error(ERR_DROP, "Incorrect checksum for file: %s", clc.downloadName);
	}

	*clc.downloadTempName = *clc.downloadName = 0;
	Cvar_Set("cl_downloadName", "");

	// We are looking to start a download here
	if (*clc.downloadList) {
		s = clc.downloadList;

		// format is:
		//  @remotename@localname@remotename@localname, etc.

		if (*s == '@')
			s++;
		remoteName = s;

		if ( (s = strchr(s, '@')) == NULL ) {
			CL_DownloadsComplete();
			return;
		}

		*s++ = 0;
		localName = s;
		if ( (s = strchr(s, '@')) != NULL )
			*s++ = 0;
		else
			s = localName + strlen(localName); // point at the nul byte

		if (!cl_allowDownload->integer) {
			Com_Error(ERR_DROP, "UDP Downloads are disabled on your client. (cl_allowDownload is %d)", cl_allowDownload->integer);
			return;
		}
		else {
			CL_BeginDownload( localName, remoteName );
		}

		clc.downloadRestart = qtrue;

		// move over the rest
		memmove( clc.downloadList, s, strlen(s) + 1);

		return;
	}

	CL_DownloadsComplete();
}

/*
=================
CL_InitDownloads

After receiving a valid game state, we valid the cgame and local zip files here
and determine if we need to download them
=================
*/
void CL_InitDownloads(void) {
  char missingfiles[1024];

	if ( !cl_allowDownload->integer )
	{
		// autodownload is disabled on the client
		// but it's possible that some referenced files on the server are missing
		if (FS_ComparePaks( missingfiles, sizeof( missingfiles ), qfalse ) )
		{
			// NOTE TTimo I would rather have that printed as a modal message box
			//   but at this point while joining the game we don't know wether we will successfully join or not
			Com_Printf( "\nWARNING: You are missing some files referenced by the server:\n%s"
				"You might not be able to join the game\n"
				"Go to the setting menu to turn on autodownload, or get the file elsewhere\n\n", missingfiles );
		}
	}
	else if ( FS_ComparePaks( clc.downloadList, sizeof( clc.downloadList ) , qtrue ) ) {

		Com_Printf("Need paks: %s\n", clc.downloadList );

		if ( *clc.downloadList ) {
			// if autodownloading is not enabled on the server
			cls.state = CA_CONNECTED;

			*clc.downloadTempName = *clc.downloadName = 0;
			Cvar_Set( "cl_downloadName", "" );

			CL_NextDownload();
			return;
		}

	}
	CL_DownloadsComplete();
}

/*
=================
CL_CheckForResend

Resend a connect message if the last one has timed out
=================
*/
void CL_CheckForResend( void ) {
	int		port;
	char	info[MAX_INFO_STRING];
	char	data[MAX_INFO_STRING+10];
	qboolean localserver = qfalse;

	// don't send anything if playing back a demo
	if ( clc.demoplaying ) {
		return;
	}

	//if (Cvar_VariableIntegerValue("sv_running") != 0) {
	if (Cvar_VariableIntegerValue("sv_running") || clc.serverAddress.type == NA_LOOPBACK) {
		Cvar_Set("protocolswitch", "1");
		cls.JKVersion = VERSION_1_01;
		localserver = qtrue;
	}

	// resend if we haven't gotten a reply yet
	if ( cls.state != CA_CONNECTING && cls.state != CA_CHALLENGING ) {
		return;
	}

	if ( cls.realtime - clc.connectTime < RETRANSMIT_TIMEOUT ) {
		return;
	}

	clc.connectTime = cls.realtime;	// for retransmit requests
	clc.connectPacketCount++;


	switch ( cls.state ) {
	case CA_CONNECTING:
		// requesting a challenge

		if (!localserver) {
			Cvar_Set("protocolswitch", "0"); //reset this here, just to be safe?
			NET_OutOfBandPrint(NS_CLIENT, clc.serverAddress, "getinfo"); //request serverinfo so we know what protocol to use
		}

		if (cl_antiSpoof->integer)
		{
			// The challenge request shall be followed by a client challenge so no malicious server can hijack this connection.
			Com_sprintf(data, sizeof(data), "getUnique %d", clc.unique);

			NET_OutOfBandPrint(NS_CLIENT, clc.serverAddress, data);
		}

		// The challenge request shall be followed by a client challenge so no malicious server can hijack this connection.
		Com_sprintf(data, sizeof(data), "getchallenge %d", clc.challenge);

		NET_OutOfBandPrint(NS_CLIENT, clc.serverAddress, data);
		break;

	case CA_CHALLENGING:
		if (protocolswitch->integer == 0 && !localserver && clc.serverAddress.type != NA_LOOPBACK) {//stall if we somehow got here before the response to our "getinfo" request
			//Com_Printf("^3no protocol set, stalling\n"); //this would make sense but the request isn't always sent in some cases with poor connection, some servers are also configured to refuse getinfo requests for some reason
			//break; //just going to assume servers are 1.01 because it's not like there's enough 1.00 servers for this to really matter
			Com_Printf("^3no response to server getinfo request, assuming protocol 26\n");
			Cvar_Set("protocolswitch", "1");
			cls.JKVersion = VERSION_1_01;
		}

		// sending back the challenge
		port = (int) Cvar_VariableValue ("net_qport");

		Q_strncpyz( info, Cvar_InfoString( CVAR_USERINFO ), sizeof( info ) );

		//if (protocolswitch->integer == 1 || localserver) {
		if (cls.JKVersion == VERSION_UNDEF && (protocolswitch->integer == 1 || localserver)) {
			Info_SetValueForKey(info, "protocol", va("%i", PROTOCOL_VERSION));
			cls.JKVersion = VERSION_1_01;
		}

		if (cls.JKVersion != VERSION_UNDEF)
		{
			{
				char *versionStr = NULL;

				switch (cls.JKVersion)
				{
					case VERSION_1_00:
						versionStr = "JKA v1.00";
						break;
					default:
					case VERSION_1_01:
						versionStr = "JKA v1.01";
						break;
					case VERSION_1_02:
						versionStr = "JK2 v1.02";
						break;
					case VERSION_1_03:
						versionStr = "JK2 v1.03";
						break;
					case VERSION_1_04:
						versionStr = "JK2 v1.04";
						break;
				}

				if (VALIDSTRING(versionStr) && strlen(versionStr))
					Com_DPrintf(S_COLOR_YELLOW "JKVersion = %s\n", versionStr);
			}

			switch (cls.JKVersion)
			{
				case VERSION_1_02: //could be 1.02 or 1.03, assuming 1.02 for now?
				case VERSION_1_03:
					Info_SetValueForKey(info, "protocol", "15");
					break;
				case VERSION_1_04:
					Info_SetValueForKey(info, "protocol", "16");
					break;
				case VERSION_1_00:
					Info_SetValueForKey(info, "protocol", va("%i", PROTOCOL_LEGACY));
					break;
				default:
				case VERSION_1_01:
					Info_SetValueForKey(info, "protocol", va("%i", PROTOCOL_VERSION));
					break;
			}
		}

		Com_DPrintf(S_COLOR_YELLOW "set protocol %s\n", Info_ValueForKey(info, "protocol"));

		Info_SetValueForKey( info, "qport", va("%i", port ) );

		if (clc.uniqueAssigned && cl_antiSpoof->integer)
		{
			Info_SetValueForKey(info, "unique", va("%i", clc.unique));
		}
		Info_SetValueForKey( info, "challenge", va("%i", clc.challenge ) );

		Com_sprintf(data, sizeof(data), "connect \"%s\"", info );
		NET_OutOfBandData( NS_CLIENT, clc.serverAddress, (byte *)data, strlen(data) );

		// the most current userinfo has been sent, so watch for any
		// newer changes to userinfo variables
		cvar_modifiedFlags &= ~CVAR_USERINFO;
		break;

	default:
		Com_Error( ERR_FATAL, "CL_CheckForResend: bad cls.state" );
	}
}


/*
===================
CL_DisconnectPacket

Sometimes the server can drop the client and the netchan based
disconnect can be lost.  If the client continues to send packets
to the server, the server will send out of band disconnect packets
to the client so it doesn't have to wait for the full timeout period.
===================
*/
void CL_DisconnectPacket( netadr_t from ) {
	if ( cls.state < CA_AUTHORIZING ) {
		return;
	}

	// if not from our server, ignore it
	if ( !NET_CompareAdr( from, clc.netchan.remoteAddress ) ) {
		return;
	}

	// if we have received packets within three seconds, ignore it
	// (it might be a malicious spoof)
	if ( cls.realtime - clc.lastPacketTime < 3000 ) {
		return;
	}

	// drop the connection (FIXME: connection dropped dialog)
	Com_Printf( "Server disconnected for unknown reason\n" );

	CL_Disconnect( qtrue );
}


/*
===================
CL_MotdPacket

===================
*/
void CL_MotdPacket( netadr_t from ) {
	char	*challenge;
	char	*info;

	// if not from our server, ignore it
	if ( !NET_CompareAdr( from, cls.updateServer ) ) {
		return;
	}

	info = Cmd_Argv(1);

	// check challenge
	challenge = Info_ValueForKey( info, "challenge" );
	if ( strcmp( challenge, cls.updateChallenge ) ) {
		return;
	}

	challenge = Info_ValueForKey( info, "motd" );

	Q_strncpyz( cls.updateInfoString, info, sizeof( cls.updateInfoString ) );
	Cvar_Set( "cl_motdString", challenge );
}

/*
===================
CL_InitServerInfo
===================
*/
void CL_InitServerInfo( serverInfo_t *server, netadr_t *address ) {
	server->adr = *address;
	server->clients = 0;
	server->filterBots = 0;
	server->hostName[0] = '\0';
	server->mapName[0] = '\0';
	server->maxClients = 0;
	server->maxPing = 0;
	server->minPing = 0;
	server->netType = 0;
	server->needPassword = qfalse;
	server->trueJedi = 0;
	server->weaponDisable = 0;
	server->forceDisable = 0;
	server->ping = -1;
	server->game[0] = '\0';
	server->gameType = 0;
	server->humans = server->bots = 0;
}

#define MAX_SERVERSPERPACKET	256

/*
===================
CL_ServersResponsePacket
===================
*/
void CL_ServersResponsePacket( const netadr_t *from, msg_t *msg ) {
	int				i, j, count, total;
	netadr_t addresses[MAX_SERVERSPERPACKET];
	int				numservers;
	byte*			buffptr;
	byte*			buffend;

	Com_Printf("CL_ServersResponsePacket from %s\n", NET_AdrToString( *from ) );

	if (cls.numglobalservers == -1) {
		// state to detect lack of servers or lack of response
		cls.numglobalservers = 0;
		cls.numGlobalServerAddresses = 0;
	}

	// parse through server response string
	numservers = 0;
	buffptr    = msg->data;
	buffend    = buffptr + msg->cursize;

	// advance to initial token
	do
	{
		if(*buffptr == '\\')
			break;

		buffptr++;
	} while (buffptr < buffend);

	while (buffptr + 1 < buffend)
	{
		// IPv4 address
		if (*buffptr == '\\')
		{
			buffptr++;

			if (buffend - buffptr < (int)(sizeof(addresses[numservers].ip) + sizeof(addresses[numservers].port) + 1))
				break;

			for(size_t i = 0; i < sizeof(addresses[numservers].ip); i++)
				addresses[numservers].ip[i] = *buffptr++;

			addresses[numservers].type = NA_IP;
		}
		else
			// syntax error!
			break;

		// parse out port
		addresses[numservers].port = (*buffptr++) << 8;
		addresses[numservers].port += *buffptr++;
		addresses[numservers].port = BigShort( addresses[numservers].port );

		// syntax check
		if (*buffptr != '\\')
			break;

		numservers++;
		if (numservers >= MAX_SERVERSPERPACKET)
			break;
	}

	count = cls.numglobalservers;

	for (i = 0; i < numservers && count < MAX_GLOBAL_SERVERS; i++) {
		// build net address
		serverInfo_t *server = &cls.globalServers[count];

		// Tequila: It's possible to have sent many master server requests. Then
		// we may receive many times the same addresses from the master server.
		// We just avoid to add a server if it is still in the global servers list.
		for (j = 0; j < count; j++)
		{
			if (NET_CompareAdr(cls.globalServers[j].adr, addresses[i]))
				break;
		}

		if (j < count)
			continue;

		CL_InitServerInfo( server, &addresses[i] );
		// advance to next slot
		count++;
	}

	// if getting the global list
	if ( count >= MAX_GLOBAL_SERVERS && cls.numGlobalServerAddresses < MAX_GLOBAL_SERVERS )
	{
		// if we couldn't store the servers in the main list anymore
		for (; i < numservers && cls.numGlobalServerAddresses < MAX_GLOBAL_SERVERS; i++)
		{
			// just store the addresses in an additional list
			cls.globalServerAddresses[cls.numGlobalServerAddresses++] = addresses[i];
		}
	}

	cls.numglobalservers = count;
	total = count + cls.numGlobalServerAddresses;

	Com_Printf("%d servers parsed (total %d)\n", numservers, total);
}

static qboolean matchmakingSearchActive = qfalse;
//void CL_MatchmakingResponsePacket(undefined8 param_1,longlong param_2)
static void CL_MatchmakingResponsePacket(const netadr_t *from, msg_t *msg)
{
	/*byte *pbVar1;
	byte bVar2;
	byte bVar3;
	byte bVar4;
	byte bVar5;
	byte bVar6;
	longlong lVar7;
	uint uVar8;
	undefined8 uVar9;
	char *pcVar10;
	ulonglong uVar11;
	byte *pbVar12;
	byte local_668 [4];
	ushort local_664 [770];*/

	Com_Printf("CL_MatchmakingResponsePacket\n");
	/*if (DAT_7100ffd528 == '\0') {
		if (DAT_7100996654 == 1) {
			FUN_710002f1f0();
			if (1 < *(int *)(param_2 + 0x1c)) {
				pbVar1 = *(byte **)(param_2 + 0x10) + *(int *)(param_2 + 0x1c);
				uVar11 = 0;
				pbVar12 = *(byte **)(param_2 + 0x10);
			LAB_71000217fc:
				pbVar12 = pbVar12;
				pbVar12 = pbVar12 + 1;
				if (*pbVar12 != 0x5c) {
					if (pbVar12 < pbVar1) goto LAB_71000217fc;
				}
				if (pbVar12 < pbVar1 + -6) {
					lVar7 = uVar11 * 6;
					bVar2 = *pbVar12;
					local_668[lVar7] = bVar2;
					bVar3 = pbVar12[2];
					local_668[lVar7 + 1] = bVar3;
					bVar4 = pbVar12[3];
					local_668[lVar7 + 2] = bVar4;
					bVar5 = pbVar12[4];
					local_668[lVar7 + 3] = bVar5;
					bVar6 = pbVar12[5];
					local_664[uVar11 * 3] = (ushort)bVar6 << 8;
					uVar8 = FUN_7100059050((ulonglong)CONCAT11(bVar6, pbVar12[6]));
					local_664[uVar11 * 3] = (ushort)uVar8;
					if (pbVar12[7] != 0x5c) goto LAB_71000218dc;
					FUN_71000474c0("server: %d ip: %d.%d.%d.%d:%d\n", uVar11 & 0xffffffff, (ulonglong)bVar2,
						(ulonglong)bVar3, (ulonglong)bVar4, (ulonglong)bVar5,
						(ulonglong)(uVar8 & 0xffff));
					if (0xfe < uVar11) goto LAB_71000218e0;
					uVar11 = uVar11 + 1;
					if ((((pbVar12[8] == 0x45) && (pbVar12[9] == 0x4f)) && (pbVar12[10] == 0x54)) ||
						(pbVar12 = pbVar12 + 7, pbVar1 <= pbVar12 + 8)) goto LAB_71000218e0;
					goto LAB_71000217fc;
				}
			LAB_71000218dc:
				if ((int)uVar11 == 0) goto LAB_710002190c;
			LAB_71000218e0:
				DAT_7100996654 = 0;
				uVar9 = FUN_71000598f0("connect %i.%i.%i.%i:%i\n", (ulonglong)local_668[0],
					(ulonglong)local_668[1], (ulonglong)local_668[2],
					(ulonglong)local_668[3], (ulonglong)local_664[0]);
				goto LAB_71000217b4;
			}
		LAB_710002190c:
			pcVar10 = "matchmaking: no servers, get out!\n";
		}
		else {
			pcVar10 = "matchmaking stopped.\n";
		}
		FUN_71000472a0(pcVar10);
	}
	else {
		uVar9 = FUN_71000598f0("connect %s\n", &DAT_7100ffd529);
	LAB_71000217b4:
		FUN_7100046320(0, uVar9);
	}
	return;*/
}

//void CL_SendMatchmakingRequest(undefined8 param_1,ulonglong param_2,ulonglong param_3) //assumed function name, looks like much was copypasted from CL_GlobalServers_f()
static void CL_StartMatchmaking(char *gametype, int matchid, int minSlots, char *platform)
{
	//undefined2 uVar1;
	//undefined8 local_460;
	//undefined8 uStack1112;
	//undefined4 local_450;
	char acStack1096 [1024];
	netadr_t	master;
	//undefined4 uStack68;
	//undefined8 local_40;
	//undefined4 local_38;

	Com_Printf("Start Matchmaking...\n");
	NET_StringToAdr("gateway.sw-jkja-mp.eks.aspyr.com", &master); //should have checked for bad return here
	matchmakingSearchActive = qtrue;
	master.type = NA_IP;
	master.port = BigShort(30000);
	//local_38 = CONCAT22(uVar1,(undefined2)local_38);
	//sprintf(acStack1096,
	Com_sprintf(acStack1096, sizeof(acStack1096),
		"startmatchmaking{\'gametype\':\'%s\',\'matchid\':\'%d\',\'min_slots\':\'%d\',\'platform\':\'%s\'}",
		gametype, matchid & 0xffffffff, minSlots & 0xffffffff, platform);// "NINTENDO");
	Com_Printf("Start Matchmaking \'%s\'\n",acStack1096);
	/*local_450 = local_38;
	local_460 = CONCAT44(uStack68,to);
	uStack1112 = local_40;*/
	//Net_SendPacket(NS_SERVER,&local_460,acStack1096);
	NET_OutOfBandPrint(NS_SERVER, master, acStack1096);
	return;
}

static void CL_StartMatchmaking_f(void) {
	char *gametype = "GT_FFA", *platform = "NINTENDO";
	int argc = Cmd_Argc(), matchid = 0, minSlots = 0;

	//FUN_71000472a0("usage: startmatchmaking <params>\n");
	if (argc < 1) {
		Com_Printf("usage: startmatchmaking <gametype> <minslots> <platform>\n");
		return;
	}

	gametype = Cmd_Argv(1);
	if (argc >= 2)
		minSlots = atoi(Cmd_Argv(2));
	if (argc >= 3)
		platform = Cmd_Argv(2);
	if (argc >= 4)
		matchid = atoi(Cmd_Argv(3));

	CL_StartMatchmaking(gametype, matchid, minSlots, platform);
}

static void CL_StopMatchmaking_f(void)
{
	/*undefined2 uVar1;
	undefined8 uStack64;
	undefined8 uStack56;
	undefined4 uStack48;
	undefined4 uStack36;
	undefined8 uStack32;
	undefined4 uStack24;*/
	netadr_t	master;

	Com_Printf("Stop Matchmaking...\n");
	NET_StringToAdr("gateway.sw-jkja-mp.eks.aspyr.com", &master);
	matchmakingSearchActive = qfalse;
	master.type = NA_IP;
	master.port = BigShort(30000);
	/*uStack64 = CONCAT44(uStack36,uStack40);
	uStack24 = CONCAT22(uVar1,(undefined2)uStack24);
	uStack56 = uStack32;
	uStack48 = uStack24;*/
	NET_OutOfBandPrint(NS_SERVER, master, "stopmatchmaking");
	return;
}

#ifndef MAX_STRINGED_SV_STRING
#define MAX_STRINGED_SV_STRING 1024
#endif
static void CL_CheckSVStringEdRef(char *buf, const char *str)
{ //I don't really like doing this. But it utilizes the system that was already in place.
	int i = 0;
	int b = 0;
	int strLen = 0;
	qboolean gotStrip = qfalse;

	if (!str || !str[0])
	{
		if (str)
		{
			strcpy(buf, str);
		}
		return;
	}

	strcpy(buf, str);

	strLen = strlen(str);

	if (strLen >= MAX_STRINGED_SV_STRING)
	{
		return;
	}

	while (i < strLen && str[i])
	{
		gotStrip = qfalse;

		if (str[i] == '@' && (i+1) < strLen)
		{
			if (str[i+1] == '@' && (i+2) < strLen)
			{
				if (str[i+2] == '@' && (i+3) < strLen)
				{ //@@@ should mean to insert a stringed reference here, so insert it into buf at the current place
					char stripRef[MAX_STRINGED_SV_STRING];
					int r = 0;

					while (i < strLen && str[i] == '@')
					{
						i++;
					}

					while (i < strLen && str[i] && str[i] != ' ' && str[i] != ':' && str[i] != '.' && str[i] != '\n')
					{
						stripRef[r] = str[i];
						r++;
						i++;
					}
					stripRef[r] = 0;

					buf[b] = 0;
					Q_strcat(buf, MAX_STRINGED_SV_STRING, SE_GetString(va("MP_SVGAME_%s", stripRef)));
					b = strlen(buf);
				}
			}
		}

		if (!gotStrip)
		{
			buf[b] = str[i];
			b++;
		}
		i++;
	}

	buf[b] = 0;
}


/*
=================
CL_ConnectionlessPacket

Responses to broadcasts, etc
=================
*/
void CL_ConnectionlessPacket( netadr_t from, msg_t *msg ) {
	char	*s;
	char	*c;
	int challenge = 0;

	MSG_BeginReadingOOB( msg );
	MSG_ReadLong( msg );	// skip the -1

	s = MSG_ReadStringLine( msg );

	Cmd_TokenizeString( s );

	c = Cmd_Argv( 0 );

	if ( com_developer->integer ) {
		Com_Printf( "CL packet %s: %s\n", NET_AdrToString( from ), c );
	}

	if (!Q_stricmp(c, "uniqueResponse"))
	{
		if (cls.state != CA_CONNECTING)
		{
			Com_Printf("Unwanted unique response received.  Ignored.\n");
			return;
		}

		if (!cl_antiSpoof->integer)
		{
			Com_Printf("unique response received.  but cl_antiSpoof is 0 so ignored.\n");
			return;
		}

		int unique = 0;
		c = Cmd_Argv(2);
		if (*c)
			unique = atoi(c);


		if (!NET_CompareAdr(from, clc.serverAddress))
		{
			// This challenge response is not coming from the expected address.
			// Check whether we have a matching client challenge to prevent
			// connection hi-jacking.

			if (!*c || unique != clc.unique)
			{
				Com_DPrintf("Unique response received from unexpected source. Ignored.\n");
				return;
			}
		}


		 clc.unique = atoi(Cmd_Argv(1));
		 clc.uniqueAssigned = true;

	}

	// challenge from the server we are connecting to
	if ( !Q_stricmp(c, "challengeResponse") )
	{
		if ( cls.state != CA_CONNECTING )
		{
			Com_Printf( "Unwanted challenge response received.  Ignored.\n" );
			return;
		}

		c = Cmd_Argv(2);
		if(*c)
			challenge = atoi(c);

		if(!NET_CompareAdr(from, clc.serverAddress))
		{
			// This challenge response is not coming from the expected address.
			// Check whether we have a matching client challenge to prevent
			// connection hi-jacking.

			if(!*c || challenge != clc.challenge)
			{
				Com_DPrintf("Challenge response received from unexpected source. Ignored.\n");
				return;
			}
		}

		// start sending challenge response instead of challenge request packets
		clc.challenge = atoi(Cmd_Argv(1));
		cls.state = CA_CHALLENGING;
		clc.connectPacketCount = 0;
		clc.connectTime = -99999;

		// take this address as the new server address.  This allows
		// a server proxy to hand off connections to multiple servers
		clc.serverAddress = from;
		Com_DPrintf ("challengeResponse: %d\n", clc.challenge);
		return;
	}

	// server connection
	if ( !Q_stricmp(c, "connectResponse") ) {
		if ( cls.state >= CA_CONNECTED ) {
			Com_Printf ("Dup connect received. Ignored.\n");
			return;
		}
		if ( cls.state != CA_CHALLENGING ) {
			Com_Printf ("connectResponse packet while not connecting. Ignored.\n");
			return;
		}
		if ( !NET_CompareAdr( from, clc.serverAddress ) ) {
			Com_Printf( "connectResponse from wrong address. Ignored.\n" );
			return;
		}
		Netchan_Setup (NS_CLIENT, &clc.netchan, from, Cvar_VariableValue( "net_qport" ) );
		cls.state = CA_CONNECTED;
		clc.lastPacketSentTime = -9999;		// send first packet immediately
		return;
	}

	// server responding to an info broadcast
	if ( !Q_stricmp(c, "infoResponse") ) {
		CL_ServerInfoPacket( from, msg );
		return;
	}

	// server responding to a get playerlist
	if ( !Q_stricmp(c, "statusResponse") ) {
		CL_ServerStatusResponse( from, msg );
		return;
	}

	// a disconnect message from the server, which will happen if the server
	// dropped the connection but it is still getting packets from us
	if (!Q_stricmp(c, "disconnect")) {
		CL_DisconnectPacket( from );
		return;
	}

	// echo request from server
	if ( !Q_stricmp(c, "echo") ) {
		NET_OutOfBandPrint( NS_CLIENT, from, "%s", Cmd_Argv(1) );
		return;
	}

	// cd check
	if ( !Q_stricmp(c, "keyAuthorize") ) {
		// we don't use these now, so dump them on the floor
		return;
	}

	// global MOTD from id
	if ( !Q_stricmp(c, "motd") ) {
		CL_MotdPacket( from );
		return;
	}

	// echo request from server
	if ( !Q_stricmp(c, "print") )
	{
		// NOTE: we may have to add exceptions for auth and update servers
		if (NET_CompareAdr(from, clc.serverAddress) || NET_CompareAdr(from, rcon_address))
		{
			char sTemp[MAX_STRINGED_SV_STRING];

			s = MSG_ReadString( msg );
			CL_CheckSVStringEdRef(sTemp, s);
			Q_strncpyz( clc.serverMessage, sTemp, sizeof( clc.serverMessage ) );
			Com_Printf( "%s", sTemp );
		}
		return;
	}

	// list of servers sent back by a master server (classic)
	if ( !Q_strncmp(c, "getserversResponse", 18) ) {
		CL_ServersResponsePacket( &from, msg );
		return;
	}

	if (!Q_strncmp(c, "startmatchmakingResponse", 24)) {
		CL_MatchmakingResponsePacket(&from, msg);
		return;
	}

	Com_DPrintf ("Unknown connectionless packet command.\n");
}


/*
=================
CL_PacketEvent

A packet has arrived from the main event loop
=================
*/
void CL_PacketEvent( netadr_t from, msg_t *msg ) {
	int		headerBytes;

	clc.lastPacketTime = cls.realtime;

	if ( msg->cursize >= 4 && *(int *)msg->data == -1 ) {
		CL_ConnectionlessPacket( from, msg );
		return;
	}

	if ( cls.state < CA_CONNECTED ) {
		return;		// can't be a valid sequenced packet
	}

	if ( msg->cursize < 4 ) {
		Com_Printf ("%s: Runt packet\n",NET_AdrToString( from ));
		return;
	}

	//
	// packet from server
	//
	if ( !NET_CompareAdr( from, clc.netchan.remoteAddress ) ) {
		if ( com_developer->integer ) {
			Com_Printf( "%s:sequenced packet without connection\n",
				NET_AdrToString( from ) );
		}
		// FIXME: send a client disconnect?
		return;
	}

	if (!CL_Netchan_Process( &clc.netchan, msg) ) {
		return;		// out of order, duplicated, etc
	}

	// the header is different lengths for reliable and unreliable messages
	headerBytes = msg->readcount;

	// track the last message received so it can be returned in
	// client messages, allowing the server to detect a dropped
	// gamestate
	clc.serverMessageSequence = LittleLong( *(int *)msg->data );

	clc.lastPacketTime = cls.realtime;
	CL_ParseServerMessage( msg );

	//
	// we don't know if it is ok to save a demo message until
	// after we have parsed the frame
	//
	if ( clc.demorecording && !clc.demowaiting ) {
		CL_WriteDemoMessage( msg, headerBytes );
	}
}

/*
==================
CL_CheckTimeout

==================
*/
void CL_CheckTimeout( void ) {
	//
	// check timeout
	//
	if ( !clc.demoplaying && ( !CL_CheckPaused() || !sv_paused->integer )
		&& cls.state >= CA_CONNECTED && cls.state != CA_CINEMATIC
	    && cls.realtime - clc.lastPacketTime > cl_timeout->integer*1000) {
		if (++cl.timeoutcount > 5) {	// timeoutcount saves debugger
			const char *psTimedOut = SE_GetString("MP_SVGAME_SERVER_CONNECTION_TIMED_OUT");
			Com_Printf ("\n%s\n",psTimedOut);
			Com_Error(ERR_DROP, psTimedOut);
			//CL_Disconnect( qtrue );
			return;
		}
	} else {
		cl.timeoutcount = 0;
	}
}

/*
==================
CL_CheckPaused
Check whether client has been paused.
==================
*/
qboolean CL_CheckPaused(void)
{
	// if cl_paused->modified is set, the cvar has only been changed in
	// this frame. Keep paused in this frame to ensure the server doesn't
	// lag behind.
	if(cl_paused->integer || cl_paused->modified)
		return qtrue;

	return qfalse;
}

//============================================================================

/*
==================
CL_CheckUserinfo

==================
*/
void CL_CheckUserinfo( void ) {
	// don't add reliable commands when not yet connected
	if ( cls.state < CA_CONNECTED ) {
		return;
	}
	// don't overflow the reliable command buffer when paused
	if ( CL_CheckPaused() ) {
		return;
	}
	// send a reliable userinfo update if needed
	if ( cvar_modifiedFlags & CVAR_USERINFO ) {
		cvar_modifiedFlags &= ~CVAR_USERINFO;
		CL_AddReliableCommand( va("userinfo \"%s\"", Cvar_InfoString( CVAR_USERINFO ) ), qfalse );
	}

}

/*
==================
SE_CheckForLanguageUpdates

 called in CL_Frame, so don't take up any time! (can also be called during dedicated)
 instead of re-loading just the files we've already loaded I'm going to load the whole language (simpler)
 ==================
 */
static void SE_CheckForLanguageUpdates( void )
{
	if ( se_language && se_language->modified ) {
		const char *psErrorMessage = SE_LoadLanguage( se_language->string, SE_TRUE );
		if ( psErrorMessage )
		{
			Com_Error( ERR_DROP, psErrorMessage );
		}

#if 0//ndef _DEBUG
		cls.charSetShader = 0;
		if (strlen(se_language->string) > 0 && !Q_stricmpn(se_language->string, "Rus", 3)) {
			cls.charSetShader = re->RegisterShaderNoMip("gfx/2d/charsgrid_med_cyr");
		}
		if (!cls.charSetShader)
			cls.charSetShader = re->RegisterShaderNoMip("gfx/2d/charsgrid_med");
#endif

		se_language->modified = SE_FALSE;
	}
}

qboolean cl_afkName;
static size_t afkPrefixLen = 0;

static void CL_GetAfk(void) {
	afkPrefixLen = strlen(cl_afkPrefix->string);
	if (!Q_strncmp(cl_name->string, cl_afkPrefix->string, afkPrefixLen)) {
		cl_afkName = qtrue;
	}
	else {
		cl_afkName = qfalse;
	}
}

extern cvar_t	*con_notifywords;
#define			MAX_NOTIFYWORDS 8
char			notifyWords[MAX_NOTIFYWORDS][32];

static void CL_AddNotificationName(char *str) {
	int i;

	//Com_Printf("Adding %s\n", str);
	for (i = 0; i<MAX_NOTIFYWORDS; i++) {
		//Com_Printf("Slot is %s", notifyWords[i]);
		if (!strcmp(notifyWords[i], "")) {
			//Com_Printf("Copying to %i\n", i);
			Q_strncpyz(notifyWords[i], str, sizeof(notifyWords[i]));
			return;
		}
	}
	//Error, max words
}

static void CL_UpdateNotificationWords(void) {
	char * pch;
	char words[MAX_CVAR_VALUE_STRING];

	Q_strncpyz(words, con_notifywords->string, sizeof(words));
	memset(notifyWords, 0, sizeof(notifyWords));
	pch = strtok(words, " ");
	while (pch != NULL) {
		CL_AddNotificationName(pch);
		pch = strtok(NULL, " ");
	}
}
/*
//debug
static void CL_PrintNotificationWords() {
	int i;

	for (i = 0; i<MAX_NOTIFYWORDS; i++) {
		if (strcmp(notifyWords[i], "")) {
			Com_Printf("Notification word: %s\n", notifyWords[i]);
		}
		else break;
	}
}
*/

static void CL_UpdateWidescreen(void) {
	if (cl_ratioFix->integer)
		cls.widthRatioCoef = (float)(SCREEN_WIDTH * cls.glconfig.vidHeight) / (float)(SCREEN_HEIGHT * cls.glconfig.vidWidth);
	else
		cls.widthRatioCoef = 1.0f;
}


int cl_nameModifiedTime = 0;
static void CL_CheckCvarUpdate(void)
{
	if (cl_colorString->modified) {
		// recalculate cl_colorStringCount
		int count;
		cl_colorString->modified = qfalse;
		count = cl_colorString->integer;
		count = count - ((count >> 1) & 0x55555555);
		count = (count & 0x33333333) + ((count >> 2) & 0x33333333);
		count = (((count + (count >> 4)) & 0x0f0f0f0f) * 0x01010101) >> 24;
		Cvar_Set("cl_colorStringCount", va("%i", count));
	}

	if (cl_name->modified) {
		cl_name->modified = qfalse;
		cl_nameModifiedTime = cls.realtime;
		CL_GetAfk();
	}
	if (con_notifywords->modified) {
		con_notifywords->modified = qfalse;
		CL_UpdateNotificationWords();
		//CL_PrintNotificationWords();
	}
	if (cl_ratioFix->modified) {
		cl_ratioFix->modified = qfalse;
		CL_UpdateWidescreen();
	}
}

/*
==================
CL_Frame

==================
*/
static unsigned int frameCount;
static float avgFrametime=0.0;
//extern void SE_CheckForLanguageUpdates(void);
void CL_Frame ( int msec ) {
	qboolean render = qfalse;
	qboolean takeVideoFrame = qfalse;

	if ( !com_cl_running->integer ) {
		return;
	}

	CL_CheckCvarUpdate();

#ifndef TOURNAMENT_CLIENT
	if ((com_renderfps->integer <= 0) || ((cls.realtime >= cls.lastDrawTime + (1000 / com_renderfps->integer))))
#endif
	{
		render = qtrue;
		cls.lastDrawTime = cls.realtime;
	}

	SE_CheckForLanguageUpdates();	// will take zero time to execute unless language changes, then will reload strings.
									//	of course this still doesn't work for menus...

	if ( cls.state == CA_DISCONNECTED && !( Key_GetCatcher( ) & KEYCATCH_UI )
		&& !com_sv_running->integer && cls.uiStarted ) {
		// if disconnected, bring up the menu
		S_StopAllSounds();
		UIVM_SetActiveMenu( UIMENU_MAIN );
	}

	// if recording an avi, lock to a fixed fps
	if ( CL_VideoRecording( ) && cl_aviFrameRate->integer && msec) {
		if ( cls.state == CA_ACTIVE || cl_forceavidemo->integer) {
			float fps = Q_min(cl_aviFrameRate->value * com_timescale->value, 1000.0f);
			float frameDuration = Q_max(1000.0f / fps, 1.0f) + clc.aviVideoFrameRemainder;
			takeVideoFrame = qtrue;

			msec = (int)frameDuration;
			clc.aviVideoFrameRemainder = frameDuration - msec;
		}
	}

	// save the msec before checking pause
	cls.realFrametime = msec;

	// decide the simulation time
	cls.frametime = msec;
	if(cl_framerate->integer)
	{
		avgFrametime+=msec;
	//	char mess[256];
		if(!(frameCount&0x1f))
		{
	//		Com_sprintf(mess,sizeof(mess),"Frame rate=%f\n\n",1000.0f*(1.0/(avgFrametime/32.0f)));
	//		Com_OPrintf("%s", mess);
	//		Com_Printf("%s", mess);
			Com_Printf("Frame rate=%f\n\n",1000.0f*(1.0/(avgFrametime/32.0f)));
			avgFrametime=0.0f;
		}
		frameCount++;
	}

	cls.realtime += cls.frametime;

	if ( cl_timegraph->integer ) {
		SCR_DebugGraph ( cls.realFrametime * 0.25, 0 );
	}

	// see if we need to update any userinfo
	CL_CheckUserinfo();

	// if we haven't gotten a packet in a long time,
	// drop the connection
	if (!clc.demoplaying) {
		CL_CheckTimeout();
	}

	// send intentions now
	CL_SendCmd();

	// resend a connection request if necessary
	CL_CheckForResend();

	// decide on the serverTime to render
	CL_SetCGameTime();

	if (render) {
		// update the screen
		SCR_UpdateScreen();

		// update audio
		S_Update();
	}

	// advance local effects for next frame
	SCR_RunCinematic();

	Con_RunConsole();

	// reset the heap for Ghoul2 vert transform space gameside
	if (G2VertSpaceServer)
	{
		G2VertSpaceServer->ResetHeap();
	}

	cls.framecount++;

	if ( takeVideoFrame ) {
		// save the current screen
		CL_TakeVideoFrame( );
	}

#if defined(DISCORD) && defined(FINAL_BUILD)
	if (cl_discordRichPresence->integer) {
		if ( cls.realtime >= 5000 && !cls.discordInitialized )
		{ //we just turned it on
			CL_DiscordInitialize();
			cls.discordInitialized = qtrue;
		}

		if ( cls.realtime >= cls.discordUpdateTime && cls.discordInitialized )
		{
			CL_DiscordUpdatePresence();
			cls.discordUpdateTime = cls.realtime + 500;
		}
	}
	else if (cls.discordInitialized) { //we just turned it off
		CL_DiscordShutdown();
		cls.discordUpdateTime = 0;
		cls.discordInitialized = qfalse;
	}
#endif
}


//============================================================================

/*
================
CL_RefPrintf

DLL glue
================
*/
void QDECL CL_RefPrintf( int print_level, const char *fmt, ...) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	Q_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	if ( print_level == PRINT_ALL ) {
		Com_Printf ("%s", msg);
	} else if ( print_level == PRINT_WARNING ) {
		Com_Printf (S_COLOR_YELLOW "%s", msg);		// yellow
	} else if ( print_level == PRINT_DEVELOPER ) {
		Com_DPrintf (S_COLOR_RED "%s", msg);		// red
	}
}



/*
============
CL_ShutdownRef
============
*/
static void CL_ShutdownRef( qboolean restarting ) {
	if ( re )
	{
		if ( re->Shutdown )
		{
			re->Shutdown( qtrue, restarting );
		}
	}

	re = NULL;

	if ( rendererLib != NULL ) {
		Sys_UnloadDll (rendererLib);
		rendererLib = NULL;
	}
}

/*
============
CL_InitRenderer
============
*/
void CL_InitRenderer( void ) {
	// this sets up the renderer and calls R_Init
	re->BeginRegistration( &cls.glconfig );

	// load character sets
#if 1
	cls.charSetShader = re->RegisterShaderNoMip("gfx/2d/charsgrid_med");
#else
	char *language = Cvar_VariableString("se_language");
	cls.charSetShader = 0;
	if (VALIDSTRING(language) && strlen(language) > 0 && !Q_stricmpn(language, "Rus", 3)) {
		cls.charSetShader = re->RegisterShaderNoMip("gfx/2d/charsgrid_med_cyr");
	}

	if (!cls.charSetShader)
		cls.charSetShader = re->RegisterShaderNoMip("gfx/2d/charsgrid_med");
#endif

	cls.whiteShader = re->RegisterShader( "white" );
	cls.consoleShader = re->RegisterShader("console");

	CL_UpdateWidescreen();
}

/*
============================
CL_StartHunkUsers

After the server has cleared the hunk, these will need to be restarted
This is the only place that any of these functions are called from
============================
*/
void CL_StartHunkUsers( void ) {
	if ( !com_cl_running->integer ) {
		return;
	}

	if ( !cls.rendererStarted ) {
		cls.rendererStarted = qtrue;
		Com_Memset(&cls.glconfig, 0, sizeof(cls.glconfig));
		CL_InitRenderer();
	}

	if ( !cls.soundStarted ) {
		cls.soundStarted = qtrue;
		S_Init();
	}

	if ( !cls.soundRegistered ) {
		cls.soundRegistered = qtrue;
		S_BeginRegistration();
//#if defined(DISCORD) && !defined(_DEBUG)
#if defined(DISCORD) && defined(FINAL_BUILD)
		cls.discordNotificationSound = S_RegisterSound("sound/interface/secret_area.mp3");
#endif
	}

	if ( !cls.uiStarted ) {
		cls.uiStarted = qtrue;
		CL_InitUI();
	}
}

/*
============
CL_InitRef
============
*/
qboolean Com_TheHunkMarkHasBeenMade(void);

//qcommon/cm_load.cpp
extern void *gpvCachedMapDiskImage;
extern qboolean gbUsingCachedMapDataRightNow;

static char *GetSharedMemory( void ) { return cl.mSharedMemory; }
static vm_t *GetCurrentVM( void ) { return currentVM; }
static qboolean CGVMLoaded( void ) { return (qboolean)cls.cgameStarted; }
static void *CM_GetCachedMapDiskImage( void ) { return gpvCachedMapDiskImage; }
static void CM_SetCachedMapDiskImage( void *ptr ) { gpvCachedMapDiskImage = ptr; }
static void CM_SetUsingCache( qboolean usingCache ) { gbUsingCachedMapDataRightNow = usingCache; }

#define G2_VERT_SPACE_SERVER_SIZE 1024//256
IHeapAllocator *G2VertSpaceServer = NULL;
CMiniHeap IHeapAllocator_singleton(G2_VERT_SPACE_SERVER_SIZE * 1024);

static IHeapAllocator *GetG2VertSpaceServer( void ) {
	return G2VertSpaceServer;
}

#define DEFAULT_RENDER_LIBRARY "rd-eternaljk"

void CL_InitRef( void ) {
	static refimport_t ri;
	refexport_t	*ret;
	GetRefAPI_t	GetRefAPI;
	char		dllName[MAX_OSPATH];

//	Com_Printf( "----- Initializing Renderer ----\n" );
	Com_Printf( "---------- Initializing Renderer ---------\n" );

	cl_renderer = Cvar_Get( "cl_renderer", DEFAULT_RENDER_LIBRARY, CVAR_ARCHIVE|CVAR_LATCH|CVAR_PROTECTED, "Which renderer library to use" );

	Com_sprintf( dllName, sizeof( dllName ), "%s_" ARCH_STRING DLL_EXT, cl_renderer->string );

	if( !(rendererLib = Sys_LoadDll( dllName, qfalse )) && strcmp( cl_renderer->string, cl_renderer->resetString ) )
	{
		Com_Printf( "failed: trying to load fallback renderer\n" );
		Cvar_ForceReset( "cl_renderer" );

		Com_sprintf( dllName, sizeof( dllName ), DEFAULT_RENDER_LIBRARY "_" ARCH_STRING DLL_EXT );
		rendererLib = Sys_LoadDll( dllName, qfalse );
	}

	if ( !rendererLib ) {
		Com_Error( ERR_FATAL, "Failed to load renderer\n" );
	}

	memset( &ri, 0, sizeof( ri ) );

	GetRefAPI = (GetRefAPI_t)Sys_LoadFunction( rendererLib, "GetRefAPI" );
	if ( !GetRefAPI )
		Com_Error( ERR_FATAL, "Can't load symbol GetRefAPI: '%s'", Sys_LibraryError() );

	//set up the import table
	ri.Printf = CL_RefPrintf;
	ri.Error = Com_Error;
	ri.OPrintf = Com_OPrintf;
	ri.Milliseconds = Sys_Milliseconds2; //FIXME: unix+mac need this
	ri.Hunk_AllocateTempMemory = Hunk_AllocateTempMemory;
	ri.Hunk_FreeTempMemory = Hunk_FreeTempMemory;
	ri.Hunk_Alloc = Hunk_Alloc;
	ri.Hunk_MemoryRemaining = Hunk_MemoryRemaining;
	ri.Z_Malloc = Z_Malloc;
	ri.Z_Free = Z_Free;
	ri.Z_MemSize = Z_MemSize;
	ri.Z_MorphMallocTag = Z_MorphMallocTag;
	ri.Cmd_ExecuteString = Cmd_ExecuteString;
	ri.Cmd_Argc = Cmd_Argc;
	ri.Cmd_Argv = Cmd_Argv;
	ri.Cmd_ArgsBuffer = Cmd_ArgsBuffer;
	ri.Cmd_AddCommand = Cmd_AddCommand;
	ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
	ri.Cvar_Set = Cvar_Set;
	ri.Cvar_Get = Cvar_Get;
	ri.Cvar_SetValue = Cvar_SetValue;
	ri.Cvar_CheckRange = Cvar_CheckRange;
	ri.Cvar_VariableStringBuffer = Cvar_VariableStringBuffer;
	ri.Cvar_VariableString = Cvar_VariableString;
	ri.Cvar_VariableValue = Cvar_VariableValue;
	ri.Cvar_VariableIntegerValue = Cvar_VariableIntegerValue;
	ri.Sys_LowPhysicalMemory = Sys_LowPhysicalMemory;
	ri.SE_GetString = SE_GetString;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_FreeFileList = FS_FreeFileList;
	ri.FS_Read = FS_Read;
	ri.FS_ReadFile = FS_ReadFile;
	ri.FS_FCloseFile = FS_FCloseFile;
	ri.FS_FOpenFileRead = FS_FOpenFileRead;
	ri.FS_FOpenFileWrite = FS_FOpenFileWrite;
	ri.FS_FOpenFileByMode = FS_FOpenFileByMode;
	ri.FS_FileExists = FS_FileExists;
	ri.FS_FileIsInPAK = FS_FileIsInPAK;
	ri.FS_ListFiles = FS_ListFiles;
	ri.FS_Write = FS_Write;
	ri.FS_WriteFile = FS_WriteFile;
	ri.CM_BoxTrace = CM_BoxTrace;
	ri.CM_DrawDebugSurface = CM_DrawDebugSurface;
	ri.CM_CullWorldBox = CM_CullWorldBox;
	ri.CM_ClusterPVS = CM_ClusterPVS;
	ri.CM_LeafArea = CM_LeafArea;
	ri.CM_LeafCluster = CM_LeafCluster;
	ri.CM_PointLeafnum = CM_PointLeafnum;
	ri.CM_PointContents = CM_PointContents;
	ri.Com_TheHunkMarkHasBeenMade = Com_TheHunkMarkHasBeenMade;
	ri.S_RestartMusic = S_RestartMusic;
	ri.SND_RegisterAudio_LevelLoadEnd = SND_RegisterAudio_LevelLoadEnd;
	ri.CIN_RunCinematic = CIN_RunCinematic;
	ri.CIN_PlayCinematic = CIN_PlayCinematic;
	ri.CIN_UploadCinematic = CIN_UploadCinematic;
	ri.CL_WriteAVIVideoFrame = CL_WriteAVIVideoFrame;

	// g2 data access
	ri.GetSharedMemory = GetSharedMemory;

	// (c)g vm callbacks
	ri.GetCurrentVM = GetCurrentVM;
	ri.CGVMLoaded = CGVMLoaded;
	ri.CGVM_RagCallback = CGVM_RagCallback;

    ri.WIN_Init = WIN_Init;
	ri.WIN_SetGamma = WIN_SetGamma;
    ri.WIN_Shutdown = WIN_Shutdown;
    ri.WIN_Present = WIN_Present;
	ri.GL_GetProcAddress = WIN_GL_GetProcAddress;
	ri.GL_ExtensionSupported = WIN_GL_ExtensionSupported;

#if WIN32
	//ri.VK_GetInstanceExtensions = WIN_VK_GetInstanceExtensions;
	//ri.VK_CreateWindowSurface = WIN_VK_CreateWindowSurface;
#endif

	ri.CM_GetCachedMapDiskImage = CM_GetCachedMapDiskImage;
	ri.CM_SetCachedMapDiskImage = CM_SetCachedMapDiskImage;
	ri.CM_SetUsingCache = CM_SetUsingCache;

	//FIXME: Might have to do something about this...
	ri.GetG2VertSpaceServer = GetG2VertSpaceServer;
	G2VertSpaceServer = &IHeapAllocator_singleton;

	ri.PD_Store = PD_Store;
	ri.PD_Load = PD_Load;

	ri.VK_IsMinimized = WIN_VK_IsMinimized;
	ri.VK_GetInstanceProcAddress = WIN_VK_GetInstanceProcAddress;
	ri.VK_createSurfaceImpl = WIN_VK_createSurfaceImpl;
	ri.VK_destroyWindow = WIN_VK_destroyWindow;

	ret = GetRefAPI( REF_API_VERSION, &ri );

//	Com_Printf( "-------------------------------\n");
	Com_Printf("---- Renderer Initialization Complete ----\n");

	if ( !ret ) {
		Com_Error (ERR_FATAL, "Couldn't initialize refresh" );
	}

	re = ret;

	// unpause so the cgame definately gets a snapshot and renders a frame
	Cvar_Set( "cl_paused", "0" );
}


//===========================================================================================


void CL_SetGender(char* model)
{
	fileHandle_t f;
	qboolean	isFemale = qfalse;
	int			i = 0;
	int			fLen = 0;
	const char* dir;
	char		soundpath[MAX_QPATH];
	char		soundName[1024];
	const char* s;

	std::string input = dir = model;
	std::string substr = input.substr(0, input.find("/", 0)).c_str();
	if (substr.size())
	{
		dir = substr.c_str();
	}

	fLen = FS_FOpenFileRead(va("models/players/%s/sounds.cfg", dir), &f, qfalse);
	if (!f)
	{//no?  Look for _default sounds.cfg
		fLen = FS_FOpenFileRead(va("models/players/%s/sounds_default.cfg", dir), &f, qfalse);
	}
	if (!f)
	{
		Cvar_Set("sex", "male");
		FS_FCloseFile(f);
		return;
	}


	soundpath[0] = 0;
	FS_Read(soundpath, fLen, f);
	soundpath[fLen] = 0;

	i = fLen;

	while (i >= 0 && soundpath[i] != '\n') {
		if (soundpath[i] == 'f') {
			isFemale = qtrue;
			soundpath[i] = 0;
		}
		i--;
	}
	i = 0;

	FS_FCloseFile(f);

	if (isFemale)
		Cvar_Set("sex", "female");
	else
		Cvar_Set("sex", "male");
}

#define MODEL_CHANGE_DELAY 5000
int gCLModelDelay = 0;

void CL_SetModel_f( void ) {
	char	*arg;
	char	name[256];

	arg = Cmd_Argv( 1 );
	if (arg[0])
	{
		/*
		//If you wanted to be foolproof you would put this on the server I guess. But that
		//tends to put things out of sync regarding cvar status. And I sort of doubt someone
		//is going to write a client and figure out the protocol so that they can annoy people
		//by changing models real fast.
		int curTime = Com_Milliseconds();
		if (gCLModelDelay > curTime)
		{
			Com_Printf("You can only change your model every %i seconds.\n", (MODEL_CHANGE_DELAY/1000));
			return;
		}

		gCLModelDelay = curTime + MODEL_CHANGE_DELAY;
		*/
		//rwwFIXMEFIXME: This is currently broken and doesn't seem to work for connecting clients
		Cvar_Set( "model", arg );
		CL_SetGender(arg);
	}
	else
	{
		Cvar_VariableStringBuffer( "model", name, sizeof(name) );
		Com_Printf("model is set to %s\n", name);
	}
}

void CL_SetForcePowers_f( void ) {
	return;
}

/*
==================
CL_VideoFilename
==================
*/
#if JAMME_PIPES
void CL_VideoFilename( char *buf, int bufSize, char *container ) {
	time_t rawtime;
	char timeStr[32] = {0}; // should really only reach ~19 chars

	time( &rawtime );
	strftime( timeStr, sizeof( timeStr ), "%Y-%m-%d_%H-%M-%S", localtime( &rawtime ) ); // or gmtime

	Com_sprintf( buf, bufSize, "videos/video%s.%s", timeStr, container );
}
#else
void CL_VideoFilename( char *buf, int bufSize ) {
	time_t rawtime;
	char timeStr[32] = {0}; // should really only reach ~19 chars

	time( &rawtime );
	strftime( timeStr, sizeof( timeStr ), "%Y-%m-%d_%H-%M-%S", localtime( &rawtime ) ); // or gmtime

	Com_sprintf( buf, bufSize, "videos/video%s.avi", timeStr );
}
#endif

/*
===============
CL_Video_f

video
video [filename]
===============
*/
#if JAMME_PIPES
void CL_Video_f( void )
{
	char	filename[ MAX_OSPATH ] = { 0 };
	char	*container = "avi", *name = NULL, *extension = NULL;
	int		argc = Cmd_Argc();

	//if ( argc >= 3 )
	//	container = Cmd_Argv( 2 );

	if (cl_aviPipe->integer) {
		if (!VALIDSTRING(cl_aviPipeContainer->string)) {
			Cvar_Set("cl_aviPipeContainer", "mp4");
		}

		container = cl_aviPipeContainer->string;
	}

	if ( argc >= 2 ) { // explicit filename
		name = Cmd_Argv(1);
		Com_sprintf( filename, MAX_OSPATH, "videos/%s.%s", name, container );
	}
	else
	{
		CL_VideoFilename( filename, MAX_OSPATH, container );

		if ( FS_FileExists( filename ) ) {
			Com_Printf( "Video: Couldn't create a file\n");
			return;
 		}
	}

	CL_OpenAVIForWriting( filename );
}
#else
void CL_Video_f( void )
{
	char  filename[ MAX_OSPATH ] = { 0 };

	if( !clc.demoplaying )
	{
		Com_Printf( "The video command can only be used when playing back demos\n" );
		return;
	}

	if( Cmd_Argc( ) == 2 )
	{
		// explicit filename
		Com_sprintf( filename, MAX_OSPATH, "videos/%s.avi", Cmd_Argv( 1 ) );
	}
	else
	{
		CL_VideoFilename( filename, MAX_OSPATH );

		if ( FS_FileExists( filename ) ) {
			Com_Printf( "Video: Couldn't create a file\n");
			return;
 		}
	}

	CL_OpenAVIForWriting( filename );
}
#endif

/*
===============
CL_StopVideo_f
===============
*/
void CL_StopVideo_f( void )
{
	CL_CloseAVI( );
}

static void CL_AddFavorite_f( void ) {
	const bool connected = (cls.state == CA_ACTIVE) && !clc.demoplaying;
	const int argc = Cmd_Argc();
	if ( !connected && argc != 2 ) {
		Com_Printf( "syntax: addFavorite <ip or hostname>\n" );
		return;
	}

	const char *server = (argc == 2) ? Cmd_Argv( 1 ) : NET_AdrToString( clc.serverAddress );
	const int status = LAN_AddFavAddr( server );
	switch ( status ) {
	case -1:
		Com_Printf( "error adding favorite server: too many favorite servers\n" );
		break;
	case 0:
		Com_Printf( "error adding favorite server: server already exists\n" );
		break;
	case 1:
		Com_Printf( "successfully added favorite server \"%s\"\n", server );
		break;
	default:
		Com_Printf( "unknown error (%i) adding favorite server\n", status );
		break;
	}
}

static void CL_DeferNameChange(void)
{
	char msg[MAX_STRINGED_SV_STRING] = { 0 };

	//Com_Printf("You must wait 5 seconds before changing your name again.\n");
	CL_CheckSVStringEdRef(msg, "@@@NONAMECHANGE");
	Com_Printf("%s\n", msg);
	/*if (cls.state == CA_ACTIVE) { //may jus want to use wait instead idk
		Cvar_Get("deferNameChange", va("set name %s ; unset deferNameChange", cl_name->string), CVAR_TEMP|CVAR_USER_CREATED);
		Cvar_Reset("name");
		Cbuf_ExecuteText(EXEC_APPEND, "do deferredNameChange 5000");
	}*/
}
void CL_Afk_f(void) {
	char name[MAX_INFO_STRING] = { 0 }, afkPrefix[MAX_INFO_STRING] = { 0 };

	if (cls.realtime - cl_nameModifiedTime <= 5000) {
		CL_DeferNameChange();
		return;
	}

	Cvar_VariableStringBuffer("name", name, sizeof(name));
	Cvar_VariableStringBuffer("cl_afkPrefix", afkPrefix, sizeof(afkPrefix));

	if (cl_afkName) {
		Cvar_Set("name", name + afkPrefixLen);
	}
	else {
		Cvar_Set("name", va("%s%s", afkPrefix, name));
	}
}

typedef struct bitInfo_S {
	const char	*string;
} bitInfo_t;

static bitInfo_t colors[] = {
	{ "^0BLACK" },
	{ "^1RED" },
	{ "^2GREEN" },
	{ "^3YELLOW" },
	{ "^4BLUE" },
	{ "^5CYAN" },
	{ "^6MAGENTA" },
	{ "^7WHITE" },
	{ "^8ORANGE" },
	{ "^9GRAY" }
};

static void CL_ColorString_f(void) {
	const int argc = Cmd_Argc();
	if (argc == 1) {
		int i = 0;
		for (i = 0; i < 10; i++) {
			if ((cl_colorString->integer & (1 << i))) {
				Com_Printf("%2d [X] %s\n", i, colors[i].string);
			}
			else {
				Com_Printf("%2d [ ] %s\n", i, colors[i].string);
			}
		}
		return;
	}
	else if (argc == 2) {
		char arg[8] = { 0 };
		int index;
		const uint32_t mask = (1 << 10) - 1;

		Cmd_ArgvBuffer(1, arg, sizeof(arg));
		index = atoi(arg);

		if (index == -1) {
			Cvar_Set("cl_colorString", "0");
			Com_Printf("colorString: All colors are now ^1disabled\n");
			return;
		}

		if (index == 10) {
			Cvar_Set("cl_colorString", "1023");
			Com_Printf("colorString: All colors are now ^2enabled\n");
			return;
		}

		if (index < 0 || index >= 10) {
			Com_Printf("colorString: Invalid range: %i [0, %i]\n", index, 9);
			return;
		}

		Cvar_Set("cl_colorString", va("%i", (1 << index) ^ (cl_colorString->integer & mask)));

		Com_Printf("%s %s^7\n", colors[index].string, ((cl_colorString->integer & (1 << index))
			? "^2Enabled" : "^1Disabled"));
	}
	else {
		char arg[8] = { 0 };
		int i, argc, index[10], bits = 0;
		const uint32_t mask = (1 << 10) - 1;

		if ((argc = Cmd_Argc() - 1) > 10) {
			Com_Printf("colorString: More than 10 arguments were entered");
			return;
		}

		for (i = 0; i < argc; i++) {
			Cmd_ArgvBuffer(i+1, arg, sizeof(arg));
			index[i] = atoi(arg);

			if (index[i] < 0 || index[i] >= 10) {
				Com_Printf("colorString: Invalid range: %i [0, %i]\n", index[i], 9);
				return;
			}

			if ((bits & mask) & (1 << index[i])) {
				Com_Printf("colorString: %s ^7was entered more than once\n", colors[index[i]].string);
				return;
			}

			bits = (1 << index[i]) ^ (bits & mask);
		}

		Cvar_Set("cl_colorString", va("%i", bits));

		for (i = 0; i < 10; i++) {
			if ((cl_colorString->integer & (1 << i))) {
				Com_Printf("%2d [X] %s\n", i, colors[i].string);
			}
			else {
				Com_Printf("%2d [ ] %s\n", i, colors[i].string);
			}
		}
	}
}

void CL_RandomizeColors(const char *in, char *out) {
	int count = cl_colorStringCount->integer;
	int i, random, j = 0, store = 0;
	const char *p = in;
	char *s = out, c;

	while ((c = *p++)) {
		if (c == '^' && *p != '\0' && *p >= '0' && *p <= '9') {
			*s++ = c;
			*s++ = *p++;
			c = *p++;
			store = 0;
		}
		else if (count == 1) {
			if (store == 0) {
				for (i = 0; i < 10; i++) {
					if ((cl_colorString->integer & (1 << i))) {
						store = i;
						*s++ = '^';
						*s++ = i + '0';
						break;
					}
				}
			}
		}
		else if (store != (random = irand(1, count*(store == 0
			? 1 : cl_colorStringRandom->integer))) && random <= count) {
			for (i = 0; i < 10; i++) {
				if ((cl_colorString->integer & (1 << i)) && (random - 1) == j++) {
					store = random;
					*s++ = '^';
					*s++ = i + '0';
				}
			}
			j = 0;
		}
		*s++ = c;
	}
	*s = '\0';
}

static void CL_ColorName_f(void) {
	char name[MAX_TOKEN_CHARS];
	char coloredName[MAX_TOKEN_CHARS];
	int storebits = cl_colorString->integer;
	int storebitcount = cl_colorStringCount->integer;

	if (cls.realtime - cl_nameModifiedTime <= 5000) {
		CL_DeferNameChange();
		return;
	}

	Cvar_VariableStringBuffer("name", name, sizeof(name));
	Q_StripColor(name);
	if (Cmd_Argc() == 1) {
		CL_RandomizeColors(name, coloredName);
		Cvar_Set("name", va("%s", coloredName));
		return;
	}
	else if (Cmd_Argc() == 2) {
		char arg[8] = { 0 };
		int index;
		const uint32_t mask = (1 << 10) - 1;

		Cmd_ArgvBuffer(1, arg, sizeof(arg));
		index = atoi(arg);

		if (index == -1) {
			Cvar_Set("name", va("%s", name));
			return;
		}

		if (index == 10) {
			cl_colorString->integer = 1023;
			cl_colorStringCount->integer = 10;
			CL_RandomizeColors(name, coloredName);
			Cvar_Set("name", va("%s", coloredName));
			cl_colorString->integer = storebits;
			cl_colorStringCount->integer = storebitcount;
			return;
		}

		if (index < 0 || index >= 10) {
			Com_Printf("colorName: Invalid range: %i [0, %i]\n", index, 9);
			return;
		}

		cl_colorStringCount->integer = 1;
		cl_colorString->integer = 0;
		cl_colorString->integer = (1 << index) ^ (cl_colorString->integer & mask);
	}
	else {
		char arg[8] = { 0 };
		int i, argc, index[10], bits = 0;
		const uint32_t mask = (1 << 10) - 1;

		if ((argc = Cmd_Argc() - 1) > 10) {
			Com_Printf("colorName: More than 10 arguments were entered");
			return;
		}

		for (i = 0; i < argc; i++) {
			Cmd_ArgvBuffer(i + 1, arg, sizeof(arg));
			index[i] = atoi(arg);

			if (index[i] < 0 || index[i] >= 10) {
				Com_Printf("colorName: Invalid range: %i [0, %i]\n", index[i], 9);
				return;
			}

			if ((bits & mask) & (1 << index[i])) {
				Com_Printf("colorName: %s ^7was entered more than once\n", colors[index[i]].string);
				return;
			}

			bits = (1 << index[i]) ^ (bits & mask);
		}

		cl_colorStringCount->integer = argc;
		cl_colorString->integer = bits;
	}

	CL_RandomizeColors(name, coloredName);
	Cvar_Set("name", va("%s", coloredName));
	cl_colorString->integer = storebits;
	cl_colorStringCount->integer = storebitcount;
}

static void CL_MaxFPS_f(void) {
	if (Cmd_Argc() < 2) {
		Cbuf_ExecuteText(EXEC_NOW, "print com_maxFPS\n");
		return;
	}

	Cbuf_ExecuteText(EXEC_NOW, va("set com_maxFPS %s\n", Cmd_ArgsFrom(1)));
}

#define G2_VERT_SPACE_CLIENT_SIZE 256

/*
===============
CL_GenerateQKey

test to see if a valid QKEY_FILE exists.  If one does not, try to generate
it by filling it with 2048 bytes of random data.
===============
*/

static void CL_GenerateQKey(void)
{
	if (cl_enableGuid->integer) {
		int len = 0;
		unsigned char buff[ QKEY_SIZE ];
		fileHandle_t f;

		len = FS_SV_FOpenFileRead( QKEY_FILE, &f );
		FS_FCloseFile( f );
		if( len == QKEY_SIZE ) {
			Com_Printf( "QKEY found.\n" );
			return;
		}
		else {
			if( len > 0 ) {
				Com_Printf( "QKEY file size != %d, regenerating\n",
					QKEY_SIZE );
			}

			Com_Printf( "QKEY building random string\n" );
			Com_RandomBytes( buff, sizeof(buff) );

			f = FS_SV_FOpenFileWrite( QKEY_FILE );
			if( !f ) {
				Com_Printf( "QKEY could not open %s for write\n",
					QKEY_FILE );
				return;
			}
			FS_Write( buff, sizeof(buff), f );
			FS_FCloseFile( f );
			Com_Printf( "QKEY generated\n" );
		}
	}
}

/*
====================
CL_Init
====================
*/
void CL_Init( void ) {
	Com_Printf( "--------- Client Initialization ----------\n" );

	Con_Init ();

	CL_ClearState ();

	cls.state = CA_DISCONNECTED;	// no longer CA_UNINITIALIZED

	cls.realtime = 0;

	CL_InitInput ();

	//
	// register our variables
	//
	cl_noprint = Cvar_Get( "cl_noprint", "0", 0 );
	cl_motd = Cvar_Get ("cl_motd", "1", CVAR_ARCHIVE_ND, "Display welcome message from master server on the bottom of connection screen" );
	cl_motdServer[0] = Cvar_Get( "cl_motdServer1", UPDATE_SERVER_NAME, 0 );
	cl_motdServer[1] = Cvar_Get( "cl_motdServer2", JKHUB_UPDATE_SERVER_NAME, 0 );
	for ( int index = 2; index < MAX_MASTER_SERVERS; index++ )
		cl_motdServer[index] = Cvar_Get( va( "cl_motdServer%d", index + 1 ), "", CVAR_ARCHIVE_ND );

	cl_timeout = Cvar_Get ("cl_timeout", "200", 0);

	cl_shownet = Cvar_Get ("cl_shownet", "0", CVAR_TEMP );
	cl_showSend = Cvar_Get ("cl_showSend", "0", CVAR_TEMP );
	cl_showTimeDelta = Cvar_Get ("cl_showTimeDelta", "0", CVAR_TEMP );
	rcon_client_password = Cvar_Get ("rconPassword", "", CVAR_TEMP, "Password for remote console access" );
	cl_activeAction = Cvar_Get( "activeAction", "", CVAR_TEMP );

	cl_showSpoofAttackMessages = Cvar_Get("cl_showSpoofAttackMessages", "1", CVAR_ARCHIVE);
	cl_antiSpoof = Cvar_Get("cl_antiSpoof", "1", CVAR_LATCH);


	cl_timedemo = Cvar_Get ("timedemo", "0", 0);
	cl_aviFrameRate = Cvar_Get ("cl_aviFrameRate", "25", CVAR_ARCHIVE);
	cl_aviMotionJpeg = Cvar_Get ("cl_aviMotionJpeg", "1", CVAR_ARCHIVE);
	cl_avi2GBLimit = Cvar_Get ("cl_avi2GBLimit", "1", CVAR_ARCHIVE );
	cl_forceavidemo = Cvar_Get ("cl_forceavidemo", "0", 0);

#if JAMME_PIPES
	cl_aviPipe = Cvar_Get("cl_aviPipe", "0", CVAR_ARCHIVE_ND, "use ffmpeg pipe for avi recording");
	cl_aviPipeCommand = Cvar_Get("cl_aviPipeCommand", PIPE_COMMAND_DEFAULT, CVAR_TEMP, "");
	cl_aviPipeContainer = Cvar_Get("cl_aviPipeContainer", "mp4", CVAR_TEMP, "");
#endif

	rconAddress = Cvar_Get ("rconAddress", "", 0, "Alternate server address to remotely access via rcon protocol");

	protocolswitch = Cvar_Get("protocolswitch", "0", CVAR_ROM|CVAR_INTERNAL|CVAR_PROTECTED|CVAR_NORESTART, "Sets protocol based on server info response");

	//Static cvars for UI
	Cvar_Get("com_protocol", va("%i", PROTOCOL_VERSION), CVAR_ROM, "1.01 protocol");
	Cvar_Get("com_legacyprotocol", va("%i", PROTOCOL_LEGACY), CVAR_ROM, "1.00 protocol");

	cl_yawspeed = Cvar_Get ("cl_yawspeed", "140", CVAR_ARCHIVE );
	cl_pitchspeed = Cvar_Get ("cl_pitchspeed", "140", CVAR_ARCHIVE );
	cl_anglespeedkey = Cvar_Get ("cl_anglespeedkey", "1.5", CVAR_ARCHIVE );

	cl_maxpackets = Cvar_Get ("cl_maxpackets", "125", CVAR_ARCHIVE );
	cl_packetdup = Cvar_Get ("cl_packetdup", "1", CVAR_ARCHIVE_ND );

#ifndef TOURNAMENT_CLIENT
	cl_timeNudge = Cvar_Get ("cl_timeNudge", "0", CVAR_TEMP );
#endif

	cl_run = Cvar_Get ("cl_run", "1", CVAR_ARCHIVE, "Always run");
	cl_sensitivity = Cvar_Get ("sensitivity", "5", CVAR_ARCHIVE, "Mouse sensitivity value");
	cl_mouseAccel = Cvar_Get ("cl_mouseAccel", "0", CVAR_ARCHIVE, "Mouse acceleration value");
	cl_freelook = Cvar_Get( "cl_freelook", "1", CVAR_ARCHIVE_ND, "Mouse look" );

	// 0: legacy mouse acceleration
	// 1: new implementation
	cl_mouseAccelStyle = Cvar_Get( "cl_mouseAccelStyle", "0", CVAR_ARCHIVE, "Mouse accelration style (0:legacy, 1:QuakeLive)" );
	// offset for the power function (for style 1, ignored otherwise)
	// this should be set to the max rate value
	cl_mouseAccelOffset = Cvar_Get( "cl_mouseAccelOffset", "5", CVAR_ARCHIVE, "Mouse acceleration offset for style 1" );

	cl_showMouseRate = Cvar_Get ("cl_showmouserate", "0", 0);
	cl_framerate	= Cvar_Get ("cl_framerate", "0", CVAR_TEMP);
	cl_allowDownload = Cvar_Get ("cl_allowDownload", "0", CVAR_ARCHIVE_ND, "Allow downloading custom paks from server");
	cl_allowAltEnter = Cvar_Get ("cl_allowAltEnter", "1", CVAR_ARCHIVE_ND, "Enables use of ALT+ENTER keyboard combo to toggle fullscreen" );
	cl_allowEnterCompletion = Cvar_Get("cl_allowEnterCompletion", "1", CVAR_ARCHIVE, "Enables autocomplete when pressing enter");

	cl_autolodscale = Cvar_Get( "cl_autolodscale", "1", CVAR_ARCHIVE_ND );

	cl_conXOffset = Cvar_Get ("cl_conXOffset", "0", 0);
	cl_inGameVideo = Cvar_Get ("r_inGameVideo", "1", CVAR_ARCHIVE_ND );

	cl_serverStatusResendTime = Cvar_Get ("cl_serverStatusResendTime", "750", 0);

	// init autoswitch so the ui will have it correctly even
	// if the cgame hasn't been started
	Cvar_Get ("cg_autoswitch", "1", CVAR_ARCHIVE);

	m_pitchVeh = Cvar_Get ("m_pitchVeh", "0.022", CVAR_ARCHIVE);
	m_pitch = Cvar_Get ("m_pitch", "0.022", CVAR_ARCHIVE);
	m_yaw = Cvar_Get ("m_yaw", "0.022", CVAR_ARCHIVE);
	m_forward = Cvar_Get ("m_forward", "0.25", CVAR_ARCHIVE);
	m_side = Cvar_Get ("m_side", "0.25", CVAR_ARCHIVE);
#ifdef MACOS_X
        // Input is jittery on OS X w/o this
	m_filter = Cvar_Get ("m_filter", "1", CVAR_ARCHIVE_ND);
#else
	m_filter = Cvar_Get ("m_filter", "0", CVAR_ARCHIVE_ND);
#endif

	cl_motdString = Cvar_Get( "cl_motdString", "", CVAR_ROM );

	Cvar_Get( "cl_maxPing", "800", CVAR_ARCHIVE_ND, "Max. ping for servers when searching the serverlist" );

	cl_lanForcePackets = Cvar_Get ("cl_lanForcePackets", "1", CVAR_ARCHIVE_ND);

	cl_drawRecording = Cvar_Get("cl_drawRecording", "1", CVAR_ARCHIVE);

	// enable the ja_guid player identifier in userinfo by default in OpenJK
	cl_enableGuid = Cvar_Get("cl_enableGuid", "1", CVAR_ARCHIVE_ND, "Enable GUID userinfo identifier" );
	cl_guidServerUniq = Cvar_Get ("cl_guidServerUniq", "1", CVAR_ARCHIVE_ND, "Use a unique guid value per server" );

	// ~ and `, as keys and characters
	cl_consoleKeys = Cvar_Get( "cl_consoleKeys", "~ ` 0x7e 0x60 0xb2", CVAR_ARCHIVE, "Which keys are used to toggle the console");
	cl_consoleUseScanCode = Cvar_Get( "cl_consoleUseScanCode", "1", CVAR_ARCHIVE, "Use native console key detection" );

	// userinfo
	cl_name = Cvar_Get ("name", "Padawan", CVAR_USERINFO | CVAR_ARCHIVE_ND, "Player name" );
	Cvar_Get ("rate", "25000", CVAR_USERINFO | CVAR_ARCHIVE, "Data rate" );
	Cvar_Get ("snaps", "40", CVAR_USERINFO | CVAR_ARCHIVE, "Client snapshots per second" );
	Cvar_Get ("model", DEFAULT_MODEL"/default", CVAR_USERINFO | CVAR_ARCHIVE, "Player model" );
	Cvar_Get ("forcepowers", "7-1-032330000000001333", CVAR_USERINFO | CVAR_ARCHIVE, "Player forcepowers" );
//	Cvar_Get ("g_redTeam", DEFAULT_REDTEAM_NAME, CVAR_SERVERINFO | CVAR_ARCHIVE);
//	Cvar_Get ("g_blueTeam", DEFAULT_BLUETEAM_NAME, CVAR_SERVERINFO | CVAR_ARCHIVE);
	Cvar_Get ("color1",  "4", CVAR_USERINFO | CVAR_ARCHIVE, "Player saber1 color" );
	Cvar_Get ("color2", "4", CVAR_USERINFO | CVAR_ARCHIVE, "Player saber2 color" );
	Cvar_Get ("handicap", "100", CVAR_USERINFO | CVAR_ARCHIVE, "Player handicap" );
	Cvar_Get ("sex", "male", CVAR_USERINFO | CVAR_ARCHIVE, "Player sex" );
	Cvar_Get ("password", "", CVAR_USERINFO|CVAR_NORESTART, "Password to join server" );
	Cvar_Get ("cg_predictItems", "1", CVAR_USERINFO | CVAR_ARCHIVE );

	//default sabers
	Cvar_Get ("saber1",  DEFAULT_SABER, CVAR_USERINFO | CVAR_ARCHIVE, "Player default right hand saber" );
	Cvar_Get ("saber2",  "none", CVAR_USERINFO | CVAR_ARCHIVE, "Player left hand saber" );

	//skin color
	Cvar_Get ("char_color_red",  "255", CVAR_USERINFO | CVAR_ARCHIVE, "Player tint (Red)" );
	Cvar_Get ("char_color_green",  "255", CVAR_USERINFO | CVAR_ARCHIVE, "Player tint (Green)" );
	Cvar_Get ("char_color_blue",  "255", CVAR_USERINFO | CVAR_ARCHIVE, "Player tint (Blue)" );

	// cgame might not be initialized before menu is used
	Cvar_Get ("cg_viewsize", "100", CVAR_ARCHIVE_ND );

	cl_ratioFix = Cvar_Get("cl_ratioFix", "1", CVAR_ARCHIVE, "Widescreen aspect ratio correction");
	cl_ratioFix->modified = qfalse;

	cl_chatStylePrefix = Cvar_Get("cl_chatStylePrefix", "", CVAR_ARCHIVE, "String inserted before sent chat messages");
	cl_chatStyleSuffix = Cvar_Get("cl_chatStyleSuffix", "", CVAR_ARCHIVE, "String appended to send chat messages");

	cl_colorString = Cvar_Get("cl_colorString", "0", CVAR_ARCHIVE, "Bit value of selected colors in colorString, configure chat colors with /colorstring");
	cl_colorStringCount = Cvar_Get("cl_colorStringCount", "0", CVAR_INTERNAL | CVAR_ROM | CVAR_ARCHIVE);
	cl_colorStringRandom = Cvar_Get("cl_colorStringRandom", "2", CVAR_ARCHIVE, "Randomness of the colors changing, higher numbers are less random");
	cl_colorString->modified = cl_colorStringCount->modified = cl_colorStringRandom->modified = qfalse;

	cl_afkTime = Cvar_Get("cl_afkTime", "10", CVAR_ARCHIVE, "Minutes to autorename to afk, 0 to disable");
	cl_afkTimeUnfocused = Cvar_Get("cl_afkTimeUnfocused", "5", CVAR_ARCHIVE, "Minutes to autorename to afk while unfocused/minimized");
	cl_afkPrefix = Cvar_Get("cl_afkPrefix", "[AFK]", CVAR_ARCHIVE, "Prefix to add to player name when AFK");
	cl_unfocusedTime = 0;
	cl_afkPrefix->modified = qfalse;

	cl_logChat = Cvar_Get("cl_logChat", "0", CVAR_ARCHIVE, "Toggle engine chat logs");

#if defined(DISCORD) && defined(FINAL_BUILD)
	cl_discordRichPresence = Cvar_Get("cl_discordRichPresence", "1", CVAR_ARCHIVE, "Allow/disallow sharing current game information on Discord profile status");
#endif

	cls.JKVersion = VERSION_1_01;

	//
	// register our commands
	//
	Cmd_AddCommand ("cmd", CL_ForwardToServer_f, "Forward command to server" );
	Cmd_AddCommand ("globalservers", CL_GlobalServers_f, "Query the masterserver for serverlist" );
	Cmd_AddCommand( "addFavorite", CL_AddFavorite_f, "Add server to favorites" );
	Cmd_AddCommand ("record", CL_Record_f, "Record a demo" );
	Cmd_AddCommand ("demo", CL_PlayDemo_f, "Playback a demo" );
	Cmd_SetCommandCompletionFunc( "demo", CL_CompleteDemoName );
	Cmd_AddCommand("playdemo", CL_PlayDemo_f, "Playback a demo"); //source engine inspired alias
	Cmd_SetCommandCompletionFunc("playdemo", CL_CompleteDemoName);
	Cmd_AddCommand("deletedemo", CL_DelDemo_f, "Delete a demo");
	Cmd_SetCommandCompletionFunc("deletedemo", CL_CompleteDemoName);
	Cmd_AddCommand ("demo_restart", CL_DemoRestart_f, "Restarts the current or last-played demo" );
	Cmd_AddCommand ("stoprecord", CL_StopRecord_f, "Stop recording a demo" );
	Cmd_AddCommand ("configstrings", CL_Configstrings_f, "Prints the configstrings list" );
	Cmd_AddCommand ("clientinfo", CL_Clientinfo_f, "Prints the userinfo variables" );
	Cmd_AddCommand ("snd_restart", CL_Snd_Restart_f, "Restart sound" );
	Cmd_AddCommand ("vid_restart", CL_Vid_Restart_f, "Restart the renderer - or change the resolution" );
	Cmd_AddCommand ("loadmod", CL_Mod_Restart_f, "Restart the renderer (with specified mod folder) - or change the resolution");
	Cmd_AddCommand ("disconnect", CL_Disconnect_f, "Disconnect from current server" );
	Cmd_AddCommand ("cinematic", CL_PlayCinematic_f, "Play a cinematic video" );
	Cmd_AddCommand ("connect", CL_Connect_f, "Connect to a server" );
	Cmd_AddCommand ("reconnect", CL_Reconnect_f, "Reconnect to current server" );
	Cmd_AddCommand ("localservers", CL_LocalServers_f, "Query LAN for local servers" );
	Cmd_AddCommand ("rcon", CL_Rcon_f, "Execute commands remotely to a server" );
	Cmd_SetCommandCompletionFunc( "rcon", CL_CompleteRcon );
	Cmd_AddCommand ("ping", CL_Ping_f, "Ping a server for info response" );
	Cmd_AddCommand ("serverstatus", CL_ServerStatus_f, "Retrieve current or specified server's status" );
	Cmd_AddCommand ("showip", CL_ShowIP_f, "Shows local IP" );
	Cmd_AddCommand ("fs_openedList", CL_OpenedPK3List_f, "Lists open pak files" );
	Cmd_AddCommand ("fs_referencedList", CL_ReferencedPK3List_f, "Lists referenced pak files" );
	Cmd_AddCommand ("model", CL_SetModel_f, "Set the player model" );
	Cmd_AddCommand ("forcepowers", CL_SetForcePowers_f );
	Cmd_AddCommand ("userinfo", CL_Clientinfo_f);
	Cmd_AddCommand ("video", CL_Video_f, "Record demo to avi" );
	Cmd_AddCommand ("stopvideo", CL_StopVideo_f, "Stop avi recording" );

	Cmd_AddCommand("afk", CL_Afk_f, "Rename to or from afk");
	Cmd_AddCommand("colorstring", CL_ColorString_f, "Color say text");
	Cmd_AddCommand("colorname", CL_ColorName_f, "Color name");

	Cmd_AddCommand("fps_max", CL_MaxFPS_f, "");

	CL_InitRef();

	SCR_Init ();

	Cbuf_Execute ();

	Cvar_Set( "cl_running", "1" );

	G2VertSpaceClient = new CMiniHeap (G2_VERT_SPACE_CLIENT_SIZE * 1024);

	CL_GenerateQKey(); //loda fixme, malware warning!
	CL_UpdateGUID( NULL, 0 );

#if defined(DISCORD) && defined(FINAL_BUILD)
	if (cl_discordRichPresence->integer) {
		CL_DiscordInitialize();
		cls.discordInitialized = qtrue;
	}
#endif

	Com_Printf( "----- Client Initialization Complete -----\n" );
}


/*
===============
CL_Shutdown

===============
*/
void CL_Shutdown( void ) {
	static qboolean recursive = qfalse;

	Com_Printf( "----- CL_Shutdown -----\n" );

	if ( recursive ) {
		Com_Printf ("WARNING: Recursive CL_Shutdown called!\n");
		return;
	}
	recursive = qtrue;

	if (G2VertSpaceClient)
	{
		delete G2VertSpaceClient;
		G2VertSpaceClient = 0;
	}

	CL_Disconnect( qtrue );

	// RJ: added the shutdown all to close down the cgame (to free up some memory, such as in the fx system)
	CL_ShutdownAll( qtrue );

	S_Shutdown();
	//CL_ShutdownUI();

	Cmd_RemoveCommand ("cmd");
	Cmd_RemoveCommand ("configstrings");
	Cmd_RemoveCommand ("clientinfo");
	Cmd_RemoveCommand ("snd_restart");
	Cmd_RemoveCommand ("vid_restart");
	Cmd_RemoveCommand ("disconnect");
	Cmd_RemoveCommand ("record");
	Cmd_RemoveCommand ("demo");
	Cmd_RemoveCommand ("playdemo");
	Cmd_RemoveCommand ("deletedemo");
	Cmd_RemoveCommand ("demo_restart");
	Cmd_RemoveCommand ("cinematic");
	Cmd_RemoveCommand ("stoprecord");
	Cmd_RemoveCommand ("connect");
	Cmd_RemoveCommand ("reconnect");
	Cmd_RemoveCommand ("localservers");
	Cmd_RemoveCommand ("globalservers");
	Cmd_RemoveCommand ( "addFavorite" );
	Cmd_RemoveCommand ("rcon");
	Cmd_RemoveCommand ("ping");
	Cmd_RemoveCommand ("serverstatus");
	Cmd_RemoveCommand ("showip");
	Cmd_RemoveCommand ("fs_openedList");
	Cmd_RemoveCommand ("fs_referencedList");
	Cmd_RemoveCommand ("model");
	Cmd_RemoveCommand ("forcepowers");
	Cmd_RemoveCommand ("userinfo");
	Cmd_RemoveCommand ("video");
	Cmd_RemoveCommand ("stopvideo");

	Cmd_RemoveCommand("afk");
	Cmd_RemoveCommand("colorstring");
	Cmd_RemoveCommand("colorname");

	Cmd_RemoveCommand("fps_max");

#if defined(DISCORD) && defined(FINAL_BUILD)
	if (cl_discordRichPresence->integer || cls.discordInitialized)
		CL_DiscordShutdown();
#endif

	CL_ShutdownInput();
	Con_Shutdown();

	Cvar_Set( "cl_running", "0" );

	recursive = qfalse;

	Com_Memset( &cls, 0, sizeof( cls ) );
	Key_SetCatcher( 0 );

	Com_Printf( "-----------------------\n" );
}

qboolean CL_ConnectedToRemoteServer( void ) {
	return (qboolean)( com_sv_running && !com_sv_running->integer && cls.state >= CA_CONNECTED && !clc.demoplaying );
}

static void CL_SetServerInfo(serverInfo_t *server, const char *info, int ping) {
	if (server) {
		if (info) {
			char *filteredHostName = Info_ValueForKey(info, "hostname");
			if (VALIDSTRING(filteredHostName)) {
				//Q_strstrip(filteredHostName, "\xac\x82\xe2\xa2\x80", NULL);
				Q_strstrip(filteredHostName, "\xac\x82\xe2\xa2\x80\u2026", NULL);
				//while (*filteredHostName == '\x20' || *filteredHostName == '\x2e' || *filteredHostName == '\xac' || *filteredHostName == '\u2026')
				while (*filteredHostName == '\x20' || *filteredHostName == '\x2e')
					*filteredHostName++;
			}
			//server->clients = atoi(Info_ValueForKey(info, "clients"));
			Q_strncpyz(server->hostName, filteredHostName, MAX_NAME_LENGTH);
			Q_strncpyz(server->mapName, Info_ValueForKey(info, "mapname"), MAX_NAME_LENGTH);
			server->maxClients = atoi(Info_ValueForKey(info, "sv_maxclients"));
			Q_strncpyz(server->game,Info_ValueForKey(info, "game"), MAX_NAME_LENGTH);
			server->gameType = atoi(Info_ValueForKey(info, "gametype"));
			server->netType = atoi(Info_ValueForKey(info, "nettype"));
			server->minPing = atoi(Info_ValueForKey(info, "minping"));
			server->maxPing = atoi(Info_ValueForKey(info, "maxping"));
//			server->allowAnonymous = atoi(Info_ValueForKey(info, "sv_allowAnonymous"));
			server->needPassword = (qboolean)atoi(Info_ValueForKey(info, "needpass" ));
			server->trueJedi = atoi(Info_ValueForKey(info, "truejedi" ));
			server->weaponDisable = atoi(Info_ValueForKey(info, "wdisable" ));
			server->forceDisable = atoi(Info_ValueForKey(info, "fdisable" ));
			server->humans = atoi( Info_ValueForKey( info, "g_humanplayers" ) );
			server->bots = atoi( Info_ValueForKey( info, "bots" ) );
//			server->pure = (qboolean)atoi(Info_ValueForKey(info, "pure" ));
		}
		server->ping = ping;
	}
}

static void CL_SetServerInfoByAddress(netadr_t from, const char *info, int ping) {
	int i;

	for (i = 0; i < MAX_OTHER_SERVERS; i++) {
		if (NET_CompareAdr(from, cls.localServers[i].adr)) {
			CL_SetServerInfo(&cls.localServers[i], info, ping);
		}
	}

	for (i = 0; i < MAX_GLOBAL_SERVERS; i++) {
		if (NET_CompareAdr(from, cls.globalServers[i].adr)) {
			CL_SetServerInfo(&cls.globalServers[i], info, ping);
		}
	}

	for (i = 0; i < MAX_OTHER_SERVERS; i++) {
		if (NET_CompareAdr(from, cls.favoriteServers[i].adr)) {
			CL_SetServerInfo(&cls.favoriteServers[i], info, ping);
		}
	}
}

void CL_SetServerFakeInfoByAddress(netadr_t from, int clients, int bots) {
	int i;

	for (i = 0; i < MAX_OTHER_SERVERS; i++) {
		if (NET_CompareAdr(from, cls.localServers[i].adr)) {
			if (clients != -1) {
				cls.localServers[i].clients = clients;
				cls.localServers[i].filterBots = bots;
			}
		}
	}

	for (i = 0; i < MAX_GLOBAL_SERVERS; i++) {
		if (NET_CompareAdr(from, cls.globalServers[i].adr)) {
			if (clients != -1) {
				cls.globalServers[i].clients = clients;
				cls.globalServers[i].filterBots = bots;
			}
		}
	}

	for (i = 0; i < MAX_OTHER_SERVERS; i++) {
		if (NET_CompareAdr(from, cls.favoriteServers[i].adr)) {
			if (clients != -1) {
				cls.favoriteServers[i].clients = clients;
				cls.favoriteServers[i].filterBots = bots;
			}
		}
	}
}

/*
===================
CL_ServerInfoPacket
===================
*/
void CL_ServerInfoPacket( netadr_t from, msg_t *msg ) {
	int		i, type;
	char	info[MAX_INFO_STRING];
	char	*infoString;
	int		prot;

	infoString = MSG_ReadString( msg );

	// if this isn't the correct protocol version, ignore it
	prot = atoi( Info_ValueForKey( infoString, "protocol" ) );

	//multiprotocol "support"
	if (prot != PROTOCOL_VERSION && prot != PROTOCOL_LEGACY && prot != 15 && prot != 16)
	{ //so we don't just ignore servers with different protocols
		Com_Printf("Different protocol info packet: %s\n", infoString);
		return;
	}

	if ((cls.state == CA_CONNECTING || cls.state == CA_CHALLENGING) && NET_CompareAdr(from, clc.serverAddress) && !Cvar_VariableIntegerValue("sv_running"))
	{
		char *versionStr = NULL;

		switch (prot)
		{
			default:
				Com_Printf("Different protocol info packet: %s\n", infoString);
				return;
			case PROTOCOL_VERSION:
				Cvar_Set("protocolswitch", "1");
				cls.JKVersion = VERSION_1_01;
				break;
			case PROTOCOL_LEGACY:
				Cvar_Set("protocolswitch", "2");
				cls.JKVersion = VERSION_1_00;
				break;
			case 15: //could be 1.03 or 1.02, going to assume 1.02 for now..
				Cvar_Set("protocolswitch", "2");
				versionStr = Info_ValueForKey(infoString, "version");
				if (VALIDSTRING(versionStr) && strlen(versionStr)) {
					if (strstr(versionStr, "v1.03")) {
						cls.JKVersion = VERSION_1_03;
						break;
					}
				} //else is 1.02
				cls.JKVersion = VERSION_1_02;
				break;
			case 16:
				Cvar_Set("protocolswitch", "2");
				cls.JKVersion = VERSION_1_04;
				break;
		}

		//it seems Aspyr removed trap_SnapVector from their new ports, i guess because it's the only code that was written in raw assembly and it was just easier to remove lol
		Cvar_Set("pmove_float", "0");
	}

	// if this is an MB2 server, ignore it
	if (!Q_stricmp(Info_ValueForKey(infoString, "game"), "mbii") && Q_stricmp(Cvar_VariableString("fs_game"), "mbii")) {
		return;
	}

	// iterate servers waiting for ping response
	for (i=0; i<MAX_PINGREQUESTS; i++)
	{
		if ( cl_pinglist[i].adr.port && !cl_pinglist[i].time && NET_CompareAdr( from, cl_pinglist[i].adr ) )
		{
			// calc ping time
			cl_pinglist[i].time = Sys_Milliseconds() - cl_pinglist[i].start;
			if ( com_developer->integer ) {
				Com_Printf( "ping time %dms from %s\n", cl_pinglist[i].time, NET_AdrToString( from ) );
			}

			// save of info
			Q_strncpyz( cl_pinglist[i].info, infoString, sizeof( cl_pinglist[i].info ) );

			// tack on the net type
			// NOTE: make sure these types are in sync with the netnames strings in the UI
			switch (from.type)
			{
				case NA_BROADCAST:
				case NA_IP:
					type = 1;
					break;

				default:
					type = 0;
					break;
			}
			Info_SetValueForKey( cl_pinglist[i].info, "nettype", va("%d", type) );
			CL_SetServerInfoByAddress(from, infoString, cl_pinglist[i].time);

			return;
		}
	}

	// if not just sent a local broadcast or pinging local servers
	if (cls.pingUpdateSource != AS_LOCAL) {
		return;
	}

	for ( i = 0 ; i < MAX_OTHER_SERVERS ; i++ ) {
		// empty slot
		if ( cls.localServers[i].adr.port == 0 ) {
			break;
		}

		// avoid duplicate
		if ( NET_CompareAdr( from, cls.localServers[i].adr ) ) {
			return;
		}
	}

	if ( i == MAX_OTHER_SERVERS ) {
		Com_DPrintf( "MAX_OTHER_SERVERS hit, dropping infoResponse\n" );
		return;
	}

	// add this to the list
	cls.numlocalservers = i+1;
	CL_InitServerInfo( &cls.localServers[i], &from );

	Q_strncpyz( info, MSG_ReadString( msg ), MAX_INFO_STRING );
	if (strlen(info)) {
		if (info[strlen(info)-1] != '\n') {
			Q_strcat(info, sizeof(info), "\n");
		}
		Com_Printf( "%s: %s", NET_AdrToString( from ), info );
	}
}

/*
===================
CL_GetServerStatus
===================
*/
serverStatus_t *CL_GetServerStatus( netadr_t from ) {
	int i, oldest, oldestTime;

	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if ( NET_CompareAdr( from, cl_serverStatusList[i].address ) ) {
			return &cl_serverStatusList[i];
		}
	}
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if ( cl_serverStatusList[i].retrieved ) {
			return &cl_serverStatusList[i];
		}
	}
	oldest = -1;
	oldestTime = 0;
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if (oldest == -1 || cl_serverStatusList[i].startTime < oldestTime) {
			oldest = i;
			oldestTime = cl_serverStatusList[i].startTime;
		}
	}
	if (oldest != -1) {
		return &cl_serverStatusList[oldest];
	}
	serverStatusCount++;
	return &cl_serverStatusList[serverStatusCount & (MAX_SERVERSTATUSREQUESTS-1)];
}

/*
===================
CL_ServerStatus
===================
*/
int CL_ServerStatus( const char *serverAddress, char *serverStatusString, int maxLen ) {
	int i;
	netadr_t	to;
	serverStatus_t *serverStatus;

	// if no server address then reset all server status requests
	if ( !serverAddress ) {
		for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
			cl_serverStatusList[i].address.port = 0;
			cl_serverStatusList[i].retrieved = qtrue;
		}
		return qfalse;
	}
	// get the address
	if ( !NET_StringToAdr( serverAddress, &to ) ) {
		return qfalse;
	}
	serverStatus = CL_GetServerStatus( to );
	// if no server status string then reset the server status request for this address
	if ( !serverStatusString ) {
		serverStatus->retrieved = qtrue;
		return qfalse;
	}

	// if this server status request has the same address
	if ( NET_CompareAdr( to, serverStatus->address) ) {
		// if we received a response for this server status request
		if (!serverStatus->pending) {
			Q_strncpyz(serverStatusString, serverStatus->string, maxLen);
			serverStatus->retrieved = qtrue;
			serverStatus->startTime = 0;
			return qtrue;
		}
		// resend the request regularly
		else if ( serverStatus->startTime < Com_Milliseconds() - cl_serverStatusResendTime->integer ) {
			serverStatus->print = qfalse;
			serverStatus->pending = qtrue;
			serverStatus->retrieved = qfalse;
			serverStatus->time = 0;
			serverStatus->startTime = Com_Milliseconds();
			NET_OutOfBandPrint(NS_CLIENT, to, "getinfo");
			NET_OutOfBandPrint( NS_CLIENT, to, "getstatus" );
			return qfalse;
		}
	}
	// if retrieved
	else if ( serverStatus->retrieved ) {
		serverStatus->address = to;
		serverStatus->print = qfalse;
		serverStatus->pending = qtrue;
		serverStatus->retrieved = qfalse;
		serverStatus->startTime = Com_Milliseconds();
		serverStatus->time = 0;
		NET_OutOfBandPrint(NS_CLIENT, to, "getinfo");
		NET_OutOfBandPrint( NS_CLIENT, to, "getstatus" );
		return qfalse;
	}
	return qfalse;
}

/*
===================
CL_ServerStatusResponse
===================
*/
void CL_ServerStatusResponse( netadr_t from, msg_t *msg ) {
	char	*s;
	char	info[MAX_INFO_STRING];
	int		i, l, score, ping;
	int		len;
	int		bots;
	serverStatus_t *serverStatus;

	CL_SetServerFakeInfoByAddress(from, -1, -1);

	serverStatus = NULL;
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if ( NET_CompareAdr( from, cl_serverStatusList[i].address ) ) {
			serverStatus = &cl_serverStatusList[i];
			break;
		}
	}
	// if we didn't request this server status
	if (!serverStatus) {
		return;
	}

	s = MSG_ReadStringLine( msg );

	len = 0;
	Com_sprintf(&serverStatus->string[len], sizeof(serverStatus->string)-len, "%s", s);

	if (serverStatus->print) {
		Com_Printf( "Server (%s)\n",
			NET_AdrToString( serverStatus->address ) );
		Com_Printf("Server settings:\n");
		// print cvars
		while (*s) {
			for (i = 0; i < 2 && *s; i++) {
				if (*s == '\\')
					s++;
				l = 0;
				while (*s) {
					info[l++] = *s;
					if (l >= MAX_INFO_STRING-1)
						break;
					s++;
					if (*s == '\\') {
						break;
					}
				}
				info[l] = '\0';
				if (i) {
					Com_Printf("%s\n", info);
				}
				else {
					Com_Printf("%-24s", info);
				}
			}
		}
	}

	len = strlen(serverStatus->string);
	Com_sprintf(&serverStatus->string[len], sizeof(serverStatus->string)-len, "\\");

	if (serverStatus->print) {
		Com_Printf("\nPlayers:\n");
		Com_Printf("num: score: ping: name:\n");
	}

	bots = 0;
	for (i = 0, s = MSG_ReadStringLine( msg ); *s; s = MSG_ReadStringLine( msg ), i++) {

		len = strlen(serverStatus->string);
		Com_sprintf(&serverStatus->string[len], sizeof(serverStatus->string)-len, "\\%s", s);

		score = ping = 0;
		sscanf(s, "%d %d", &score, &ping);
		s = strchr(s, ' ');
		if (s)
			s = strchr(s+1, ' ');
		if (s)
			s++;
		else
			s = "unknown";

		if (ping == 0) {
			bots++;
		}

		if (serverStatus->print) {
			Com_Printf("%-2d   %-3d    %-3d   %s\n", i, score, ping, s );
		}
	}

	CL_SetServerFakeInfoByAddress(from, i, bots);

	len = strlen(serverStatus->string);
	Com_sprintf(&serverStatus->string[len], sizeof(serverStatus->string)-len, "\\");

	serverStatus->time = Com_Milliseconds();
	serverStatus->address = from;
	serverStatus->pending = qfalse;
	if (serverStatus->print) {
		serverStatus->retrieved = qtrue;
	}
}

/*
==================
CL_LocalServers_f
==================
*/
void CL_LocalServers_f( void ) {
	char		*message;
	int			i, j;
	netadr_t	to;

	Com_Printf( "Scanning for servers on the local network...\n");

	// reset the list, waiting for response
	cls.numlocalservers = 0;
	cls.pingUpdateSource = AS_LOCAL;

	for (i = 0; i < MAX_OTHER_SERVERS; i++) {
		qboolean b = cls.localServers[i].visible;
		Com_Memset(&cls.localServers[i], 0, sizeof(cls.localServers[i]));
		cls.localServers[i].visible = b;
	}
	Com_Memset( &to, 0, sizeof( to ) );

	// The 'xxx' in the message is a challenge that will be echoed back
	// by the server.  We don't care about that here, but master servers
	// can use that to prevent spoofed server responses from invalid ip
	message = "\377\377\377\377getinfo xxx";

	// send each message twice in case one is dropped
	for ( i = 0 ; i < 2 ; i++ ) {
		// send a broadcast packet on each server port
		// we support multiple server ports so a single machine
		// can nicely run multiple servers
		for ( j = 0 ; j < NUM_SERVER_PORTS ; j++ ) {
			to.port = BigShort( (short)(PORT_SERVER + j) );

			to.type = NA_BROADCAST;
			NET_SendPacket( NS_CLIENT, strlen( message ), message, to );
		}
	}
}

/*
==================
CL_GlobalServers_f

Originally master 0 was Internet and master 1 was MPlayer.
ioquake3 2008; added support for requesting five separate master servers using 0-4.
ioquake3 2017; made master 0 fetch all master servers and 1-5 request a single master server.
OpenJK 2013; added support for requesting five separate master servers using 0-4.
OpenJK July 2017; made master 0 fetch all master servers and 1-5 request a single master server.

==================
*/
void CL_GlobalServers_f( void ) {
	netadr_t	to;
	int			count, i, masterNum, protocol;
	char		command[1024], *masteraddress;

	if ((count = Cmd_Argc()) < 3 || (masterNum = atoi(Cmd_Argv(1))) < 0 || masterNum > MAX_MASTER_SERVERS)
	{
		Com_Printf("usage: globalservers <master# 0-%d> <protocol> [keywords]\n", MAX_MASTER_SERVERS);
		return;
	}

	protocol = atoi(Cmd_Argv(2));
	// request from all master servers
	if ( !masterNum ) {
		int numAddress = 0;
		char *cmdargs = Cmd_ArgsFrom(3);

		for ( i = 1; i <= MAX_MASTER_SERVERS; i++ ) {
			Com_sprintf( command, sizeof(command), "sv_master%d", i );
			masteraddress = Cvar_VariableString(command);

			if(!*masteraddress)
				continue;

			//ok yet ANOTHER hacky edit for new console protocols
			if (protocol != PROTOCOL_VERSION) { //we're already forcing their protocol below
				if (Q_stristr(masteraddress, "aspyr") || !Q_stricmpn(masteraddress, "gateway.sw-jkja-mp.eks.aspyr.com", 32)) {
					continue;
				}
			} //hopefully this keeps us from sending redundant packets that would trigger some anti ddos system they have

			numAddress++;

			Com_sprintf(command, sizeof(command), "globalservers %i %i %s\n", i, protocol, cmdargs);
			//Com_sprintf(command, sizeof(command), "wait %i ; globalservers %i %i %s\n", com_maxfps->integer, i, protocol, cmdargs);
			Cbuf_AddText(command);
		}

		if ( !numAddress ) {
			Com_Printf( "CL_GlobalServers_f: Error: No master server addresses.\n");
		}
		return;
	}

	Com_sprintf( command, sizeof(command), "sv_master%d", masterNum );
	masteraddress = Cvar_VariableString( command );

	if ( !*masteraddress )
	{
		Com_Printf( "CL_GlobalServers_f: Error: No master server address given for %s.\n", command );
		return;
	}

	// reset the list, waiting for response
	// -1 is used to distinguish a "no response"

	i = NET_StringToAdr( masteraddress, &to );

	if (!i)
	{
		Com_Printf( "CL_GlobalServers_f: Error: could not resolve address of master %s\n", masteraddress );
		return;
	}
	to.type = NA_IP;
	//to.port = BigShort(PORT_MASTER);
	if (!strchr(masteraddress, ':'))
	{
		to.port = BigShort(PORT_MASTER);
	}
	Com_Printf( "Requesting servers from the master %s (%s)...\n", masteraddress, NET_AdrToString( to ) );

	cls.numglobalservers = -1;
	cls.pingUpdateSource = AS_GLOBAL;

	//Com_sprintf(command, sizeof(command), "getservers %s", Cmd_Argv(2));
	Com_sprintf(command, sizeof(command), "getservers %i", protocol);

	// tack on keywords
	for (i = 3; i < count; i++)
	{
		Q_strcat(command, sizeof(command), " ");
		Q_strcat(command, sizeof(command), Cmd_Argv(i));
	}

	NET_OutOfBandPrint( NS_SERVER, to, "%s", command );
}

/*
==================
CL_GetPing
==================
*/
void CL_GetPing( int n, char *buf, int buflen, int *pingtime )
{
	const char	*str;
	int		time;
	int		maxPing;

	if (n < 0 || n >= MAX_PINGREQUESTS || !cl_pinglist[n].adr.port)
	{
		// empty or invalid slot
		buf[0]    = '\0';
		*pingtime = 0;
		return;
	}

	str = NET_AdrToString( cl_pinglist[n].adr );
	Q_strncpyz( buf, str, buflen );

	time = cl_pinglist[n].time;
	if (!time)
	{
		// check for timeout
		time = Sys_Milliseconds() - cl_pinglist[n].start;
		maxPing = Cvar_VariableIntegerValue( "cl_maxPing" );
		if( maxPing < 100 ) {
			maxPing = 100;
		}
		if (time < maxPing)
		{
			// not timed out yet
			time = 0;
		}
	}

	CL_SetServerInfoByAddress(cl_pinglist[n].adr, cl_pinglist[n].info, cl_pinglist[n].time);

	*pingtime = time;
}

/*
==================
CL_GetPingInfo
==================
*/
void CL_GetPingInfo( int n, char *buf, int buflen )
{
	if (n < 0 || n >= MAX_PINGREQUESTS || !cl_pinglist[n].adr.port)
	{
		// empty or invalid slot
		if (buflen)
			buf[0] = '\0';
		return;
	}

	Q_strncpyz( buf, cl_pinglist[n].info, buflen );
}

/*
==================
CL_ClearPing
==================
*/
void CL_ClearPing( int n )
{
	if (n < 0 || n >= MAX_PINGREQUESTS)
		return;

	cl_pinglist[n].adr.port = 0;
}

/*
==================
CL_GetPingQueueCount
==================
*/
int CL_GetPingQueueCount( void )
{
	int		i;
	int		count;
	ping_t*	pingptr;

	count   = 0;
	pingptr = cl_pinglist;

	for (i=0; i<MAX_PINGREQUESTS; i++, pingptr++ ) {
		if (pingptr->adr.port) {
			count++;
		}
	}

	return (count);
}

/*
==================
CL_GetFreePing
==================
*/
ping_t* CL_GetFreePing( void )
{
	ping_t*	pingptr;
	ping_t*	best;
	int		oldest;
	int		i;
	int		time;

	pingptr = cl_pinglist;
	for (i=0; i<MAX_PINGREQUESTS; i++, pingptr++ )
	{
		// find free ping slot
		if (pingptr->adr.port)
		{
			if (!pingptr->time)
			{
				if (Sys_Milliseconds() - pingptr->start < 500)
				{
					// still waiting for response
					continue;
				}
			}
			else if (pingptr->time < 500)
			{
				// results have not been queried
				continue;
			}
		}

		// clear it
		pingptr->adr.port = 0;
		return (pingptr);
	}

	// use oldest entry
	pingptr = cl_pinglist;
	best    = cl_pinglist;
	oldest  = INT_MIN;
	for (i=0; i<MAX_PINGREQUESTS; i++, pingptr++ )
	{
		// scan for oldest
		time = Sys_Milliseconds() - pingptr->start;
		if (time > oldest)
		{
			oldest = time;
			best   = pingptr;
		}
	}

	return (best);
}

/*
==================
CL_Ping_f
==================
*/
void CL_Ping_f( void ) {
	netadr_t	to;
	ping_t*		pingptr;
	char*		server;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "usage: ping [server]\n");
		return;
	}

	Com_Memset( &to, 0, sizeof(netadr_t) );

	server = Cmd_Argv(1);

	if ( !NET_StringToAdr( server, &to ) ) {
		return;
	}

	pingptr = CL_GetFreePing();

	memcpy( &pingptr->adr, &to, sizeof (netadr_t) );
	pingptr->start = Sys_Milliseconds();
	pingptr->time  = 0;

	CL_SetServerInfoByAddress(pingptr->adr, NULL, 0);

	NET_OutOfBandPrint( NS_CLIENT, to, "getinfo xxx" );
}

/*
==================
CL_UpdateVisiblePings_f
==================
*/
qboolean CL_UpdateVisiblePings_f(int source) {
	int			slots, i;
	char		buff[MAX_STRING_CHARS];
	int			pingTime;
	int			max;
	qboolean status = qfalse;

	if (source < 0 || source > AS_FAVORITES) {
		return qfalse;
	}

	cls.pingUpdateSource = source;

	slots = CL_GetPingQueueCount();
	if (slots < MAX_PINGREQUESTS) {
		serverInfo_t *server = NULL;

		switch (source) {
			case AS_LOCAL :
				server = &cls.localServers[0];
				max = cls.numlocalservers;
			break;
			case AS_GLOBAL :
				server = &cls.globalServers[0];
				max = cls.numglobalservers;
			break;
			case AS_FAVORITES :
				server = &cls.favoriteServers[0];
				max = cls.numfavoriteservers;
			break;
			default:
				return qfalse;
		}
		for (i = 0; i < max; i++) {
			if (server[i].visible) {
				if (server[i].ping == -1) {
					int j;

					if (slots >= MAX_PINGREQUESTS) {
						break;
					}
					for (j = 0; j < MAX_PINGREQUESTS; j++) {
						if (!cl_pinglist[j].adr.port) {
							continue;
						}
						if (NET_CompareAdr( cl_pinglist[j].adr, server[i].adr)) {
							// already on the list
							break;
						}
					}
					if (j >= MAX_PINGREQUESTS) {
						status = qtrue;
						for (j = 0; j < MAX_PINGREQUESTS; j++) {
							if (!cl_pinglist[j].adr.port) {
								break;
							}
						}
						memcpy(&cl_pinglist[j].adr, &server[i].adr, sizeof(netadr_t));
						cl_pinglist[j].start = Sys_Milliseconds();
						cl_pinglist[j].time = 0;
						NET_OutOfBandPrint( NS_CLIENT, cl_pinglist[j].adr, "getinfo xxx" );

						serverStatus_t *serverStatus = CL_GetServerStatus(cl_pinglist[j].adr);
						serverStatus->address = cl_pinglist[j].adr;
						serverStatus->print = qfalse;
						serverStatus->pending = qtrue;
						serverStatus->retrieved = qfalse;
						serverStatus->startTime = Com_Milliseconds();
						serverStatus->time = 0;
						NET_OutOfBandPrint(NS_CLIENT, cl_pinglist[j].adr, "getstatus");

						slots++;
					}
				}
				// if the server has a ping higher than cl_maxPing or
				// the ping packet got lost
				else if (server[i].ping == 0) {
					// if we are updating global servers
					if (source == AS_GLOBAL) {
						//
						if ( cls.numGlobalServerAddresses > 0 ) {
							// overwrite this server with one from the additional global servers
							cls.numGlobalServerAddresses--;
							CL_InitServerInfo(&server[i], &cls.globalServerAddresses[cls.numGlobalServerAddresses]);
							// NOTE: the server[i].visible flag stays untouched
						}
					}
				}
			}
		}
	}

	if (slots) {
		status = qtrue;
	}
	for (i = 0; i < MAX_PINGREQUESTS; i++) {
		if (!cl_pinglist[i].adr.port) {
			continue;
		}
		CL_GetPing( i, buff, MAX_STRING_CHARS, &pingTime );
		if (pingTime != 0) {
			CL_ClearPing(i);
			status = qtrue;
		}
	}

	return status;
}

/*
==================
CL_ServerStatus_f
==================
*/
void CL_ServerStatus_f(void) {
	netadr_t	to, *toptr = NULL;
	char		*server;
	serverStatus_t *serverStatus;

	if ( Cmd_Argc() != 2 ) {
		if ( cls.state != CA_ACTIVE || clc.demoplaying ) {
			Com_Printf ("Not connected to a server.\n");
			Com_Printf( "Usage: serverstatus [server]\n");
			return;
		}

		toptr = &clc.serverAddress;
	}

	if(!toptr)
	{
		Com_Memset( &to, 0, sizeof(netadr_t) );

		server = Cmd_Argv(1);

		toptr = &to;
		if ( !NET_StringToAdr( server, toptr ) )
			return;
	}

	NET_OutOfBandPrint( NS_CLIENT, *toptr, "getstatus" );

	serverStatus = CL_GetServerStatus( *toptr );
	serverStatus->address = *toptr;
	serverStatus->print = qtrue;
	serverStatus->pending = qtrue;
}

/*
==================
CL_ShowIP_f
==================
*/
void CL_ShowIP_f(void) {
	Sys_ShowIP();
}
