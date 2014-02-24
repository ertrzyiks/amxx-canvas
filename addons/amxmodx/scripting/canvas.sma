/**
 * The MIT License (MIT)
 * 
 * Copyright (c) 2014 Mateusz Derks
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Proof-of-concept code which aim to provide programable displayer inside Half-Life multiplayer gameplay.
 * This plugin is library which provide basic API for canvas and pixels manipulation.
 *
 * @ https://github.com/ertrzyiks/amxx-canvas
 */
#include <amxmodx>
#include <amxmisc>
#include <fakemeta>
#include <canvas>
#include <xs>

#define PLUGIN "Canvas"
#define VERSION "0.1.0"
#define AUTHOR "R3X"

new const gszPixelModel[] = "sprites/pixel.spr";

new giMaxPlayers;

new Array:gPrograms;
new Array:gProgramForceSizes;
new Array:gProgramEvents;

new info_target;


new giInteractionCanvas[33] = { -1, ... };

#include "canvas/menus.inl"
#include "canvas/renderer.inl"
#include "canvas/camera_lock.inl"
#include "canvas/interaction.inl"
#include "canvas/natives.inl"


public plugin_init ()
{
	register_plugin( PLUGIN, VERSION, AUTHOR );
	
	register_clcmd( "amx_canvas", "cmdCanvas", ADMIN_CFG );

	register_forward( FM_StartFrame, "fwStartFrame", 1 );
	register_forward( FM_Think, "fwThinkCamera" );
	register_forward( FM_CmdStart, "fwCmdStart" );
	
	giMaxPlayers = get_maxplayers();
	
	gPrograms = ArrayCreate();
	gProgramForceSizes = ArrayCreate( 2 );
	gProgramEvents = ArrayCreate();
	
	createProgram( "Default", "handleDefaultProgram" );
}
	
public plugin_precache ()
{
	precache_model( gszPixelModel );
	
	info_target = engfunc( EngFunc_AllocString, "info_target" );
	
	createCanvasMenu();
	createCanvasDetailsMenu();
	createProgramMenu();
	createSizeMenu();
	createScaleMenu();
}

public cmdCanvas ( id, level, cid )
{
	if( !cmd_access( id, level, cid, 1 ) )
		return PLUGIN_HANDLED;
		
	showCanvasMenu( id );
	return PLUGIN_HANDLED;
}

public fwStartFrame()
{
	static Float:fTime = 0.0;
	static Float:fNow, Float:fDelta;
	fNow = get_gametime();
	
	fDelta = fNow - fTime;
	fTime = fNow;
	
	for ( new i = 0; i < giCanvasInitializeIndex; i++ )
	{
		new isReady = gCanvas[i][ready];
		
		if ( isReady )
		{
			new cb = ArrayGetCell( gPrograms, gCanvas[i][programId] ), ret;
			
			if ( cb <= 0)
			{
				handleDefaultProgram( i, fDelta );
			}
			else
			{
				ExecuteForward( cb , ret, i, fDelta );
				checkForInteraction( i );
			}
		}
		else
		{
			creatingTick( i );
		}
	}
}



public fwTraceLine( const Float:fStart[3], const Float:fEnd[3], conditions, id, tr_handle )
{
	static Float:fOrigin[3], Float:fVec[3];
	
	if ( is_user_connected( id ) && gbTraceHooks[ id ] )
	{
		gbTraceHooks[ id ] = false;
		giTraceHooksActive = giTraceHooksActive - 1;
		
		get_tr2( tr_handle, TR_vecEndPos, fOrigin );
		get_tr2( tr_handle, TR_vecPlaneNormal, fVec );
		
		createCanvas( fOrigin, fVec );
		showCanvasMenu( id );
		
		if ( giTraceHooksActive == 0 )
		{
			unregister_forward( FM_TraceLine, ghTraceLine );
		}
	}
	
	return FMRES_IGNORED;
}

createCanvasByAim ( id )
{
	if ( gbTraceHooks[ id ] )
	{
		return;
	}
	
	gbTraceHooks[ id ] = true;
	giTraceHooksActive = giTraceHooksActive + 1;
	
	if ( giTraceHooksActive == 1 )
	{	
		ghTraceLine = register_forward( FM_TraceLine, "fwTraceLine", 1 );
	}
}
