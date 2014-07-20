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
#include <hamsandwich>
#include <canvas>
#include <xs>

#define PLUGIN "Canvas"
#define VERSION "0.1.6"
#define AUTHOR "R3X"

new const gszPixelModel[] = "sprites/pixel.spr";

/**
 * Maximum number of player that can play on server in one time.
 */
new giMaxPlayers;


/**
 * Array of registered programs. 
 * Store collection of references to draw function.
 */
new Array:gPrograms;

/**
 * Array of forced size for program. 
 * Store collection of int[2] with width and height. Both can be 0 to turn off limitation.
 * Size synced with gPrograms.
 */
new Array:gProgramForceSizes;

/**
 * Array of registered events for program.
 * Store collection of Tries 
 *
 *	[eventName] = Array:callbacks;
 *
 * Size synced with gPrograms and gProgramForceSizes.
 */
new Array:gProgramEvents;


/**
 * Reference to string "info_target" allocated on game memory space.
 */
new info_target;

/**
 * Reference to precached pixel sprite.
 * Actually we use model name as sprite, reference is required for `make_line` util.
 */
 #pragma unused gSprite
new gSprite;


/**
 * Holds player's camera lock. Map of
 * 
 *	[playerId] = canvasId or -1;
 */
new giCameraLocks[33] = { -1, ... };

/**
 * Holds player's current interaction canvas. Map of
 * 
 *	[playerId] = canvasId or -1;
 */
new giInteractionCanvas[33] = { -1, ... };

/**
 * Flag is set to true for player with active aim position sniffing. Map of
 *
 *	[playerId] = active_sniffing_or_not;
 *
 * Prevent stacking create canvas by aim requests..
 */
new bool:gbTraceHooks[33];

/**
 * Number of active traceline hooks. When reach 0, event handler for traceline should be unregistered.
 */
new giTraceHooksActive = 0;

/**
 * Handler to current traceline hook. Used for unregistering.
 */
new ghTraceLine;

/**
 * Number of active hover event listeners. When reach 0, traceline handler will be unregistered.
 */
new giHoverHooksActive = 0;

/**
 * Handler to current hover traceline hook. Used for unregistering.
 */
new ghHoverTraceLine;

/**
 * Flag for hover input. when set to true and there is no new hover event let sent {-1,-1} coords once.
 */
new bool:gbIsLookingAtCanvas[33][CANVAS_MAX_INSTANCES];

#include "canvas/util.inl"
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
	RegisterHam( Ham_Spawn, "player", "fwPlayerSpawn", 1 );
	
	giMaxPlayers = get_maxplayers();
	
	gPrograms = ArrayCreate();
	gProgramForceSizes = ArrayCreate( 2 );
	gProgramEvents = ArrayCreate();
	
	createProgram( "Default", "handleDefaultProgram" );
}
	
public plugin_precache ()
{
	gSprite = precache_model( gszPixelModel );
	
	info_target = engfunc( EngFunc_AllocString, "info_target" );
	
	createCanvasMenu();
	createCanvasDetailsMenu();
	createProgramMenu();
	createSizeMenu();
	createScaleMenu();
}

/**
 * Display management menu when command used
 *
 * @param id Id of player
 * @param level Access level to command
 * @param cid Command id
 * @return PLUGIN_* callback result
 */
public cmdCanvas ( id, level, cid )
{
	if( !cmd_access( id, level, cid, 1 ) )
		return PLUGIN_HANDLED;
		
	showCanvasMenu( id );
	return PLUGIN_HANDLED;
}

/**
 * Propagate frame from server to canvases
 */
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
				//Create some noise if no custom program used
				handleDefaultProgram( i, fDelta );
			}
			else
			{
				//Give control over rendering to program handler
				ExecuteForward( cb , ret, i, fDelta );
				
				//and check for user input
				checkForInteraction( i );
			}
		}
		else
		{
			//Trigger initialization tick
			creatingTick( i );
		}
	}
}

/**
 * Hooked once per user after request for create canvas by aim to retrieve aim position.
 *
 * @param vfStart Point when trace starts
 * @param vfEnd Point when trace ends
 * @param conditions Engine conditions, like ignore monsters
 * @param id Player id
 * @param tr_handle Trace result handle, used to read tracing data
 */
public fwTraceLine( const Float:vfStart[3], const Float:vfEnd[3], conditions, id, tr_handle )
{
	static Float:vfOrigin[3], Float:vfVec[3];
	
	if ( is_user_connected( id ) && gbTraceHooks[ id ] )
	{
		gbTraceHooks[ id ] = false;
		giTraceHooksActive = giTraceHooksActive - 1;
		
		get_tr2( tr_handle, TR_vecEndPos, vfOrigin );
		get_tr2( tr_handle, TR_vecPlaneNormal, vfVec );
		
		createCanvas( vfOrigin, vfVec );
		showCanvasMenu( id );
		
		if ( giTraceHooksActive == 0 )
		{
			unregister_forward( FM_TraceLine, ghTraceLine );
		}
	}
	
	return FMRES_IGNORED;
}

/**
 * Request canvas creation. Waits for next TraceLine event to read aim position gracefully.
 *
 * @param id Player id
 */
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

/**
 * Used to determine if user is looking at canvas and trigger input event if he is.
 *
 * @param vfStart Point when trace starts
 * @param vfEnd Point when trace ends
 * @param conditions Engine conditions, like ignore monsters
 * @param id Player id
 * @param tr_handle Trace result handle, used to read tracing data
 */
public fwHoverTraceLine( const Float:vfStart[3], const Float:vfEnd[3], conditions, id, tr_handle )
{
	if ( !is_user_connected( id ) )
	{
		return FMRES_IGNORED;
	}
	
	new col, row, data[3];
	
	for ( new i = 0; i < giCanvasIndex; i++ )
	{
		if ( getHoverPoint( i, vfStart, vfEnd, col, row ) )
		{
			gbIsLookingAtCanvas[id][i] = true;
			
			data[0] = id;
			data[1] = col;
			data[2] = row;
			triggerProgramEvent( i, gCanvas[i][programId], "interaction:hover", data, sizeof data );
			break;
		}
		else if ( gbIsLookingAtCanvas[id][i] )
		{
			gbIsLookingAtCanvas[id][i] = false;
			
			data[0] = id;
			data[1] = -1;
			data[2] = -1;
			triggerProgramEvent( i, gCanvas[i][programId], "interaction:hover", data, sizeof data );
		}
	}
	
	return FMRES_IGNORED;
}

/**
 * Called when plugin register event listener. 
 * Used for hooks setup/dismiss.
 *
 * @param szName Event name
 */
onEventListenerAdded( const szName[] )
{
	if ( equal( szName, "interaction:hover" ) )
	{
		giHoverHooksActive++;
	}
	
	if ( giHoverHooksActive == 1 )
	{
		ghHoverTraceLine = register_forward( FM_TraceLine, "fwHoverTraceLine" );
	}
}

/**
 * Called when plugin unregister event listener. 
 * Used for hooks setup/dismiss.
 *
 * @param szName Event name
 */
onEventListenerRemoved( const szName[] )
{
	if ( equal( szName, "interaction:hover" ) )
	{
		giHoverHooksActive--;
	}
	
	if ( giHoverHooksActive == 0 )
	{
		unregister_forward( FM_TraceLine, ghHoverTraceLine );
	}
}