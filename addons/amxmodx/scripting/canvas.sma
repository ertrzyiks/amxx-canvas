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

enum Canvas
{
	cols,
	rows,
	
	scale,
	ready,
	
	init_tick,
	init_maxtick,
	
	originX,
	originY,
	originZ,
	
	directionX,
	directionY,
	directionZ,
	
	rightX,
	rightY,
	rightZ,
	
	downX,
	downY,
	downZ
};

enum CanvasInitializer
{
	callback,
	tempo
};

#define PLUGIN "Canvas"
#define VERSION "0.0.1"
#define AUTHOR "R3X"

new bool:gbTraceHooks[32];
new giTraceHooksActive = 0;
new ghTraceLine;

new gCanvas[CANVAS_MAX_INSTANCES][Canvas];
new gCanvasPixels[CANVAS_MAX_INSTANCES][CANVAS_MAX_PIXELS];
new giCanvasIndex = 0;

new gCanvasInitializers[CANVAS_MAX_INITIALIZER][CanvasInitializer];
new gCanvasInitializerNames[CANVAS_MAX_INITIALIZER][CANVAS_MAX_INIT_NAME];
new giCanvasInitializeIndex = 0;

new Trie:gPrograms;
new Array:gProgramNames

new const gszPixelModel[] = "sprites/pixel.spr";

public plugin_init ()
{
	register_plugin( PLUGIN, VERSION, AUTHOR );
	
	register_clcmd( "amx_canvas", "cmdCanvas", ADMIN_CFG );
	
	register_forward( FM_StartFrame, "fwStartFrame", 1 );
	
	gPrograms = TrieCreate();
	gProgramNames = ArrayCreate( CANVAS_MAX_PROGRAM_NAME );
	
	createProgram( "Default", "handleDefaultProgram" );
}

public plugin_precache ()
{
	precache_model( gszPixelModel );
}

public plugin_natives()
{
	register_library( "Canvas" );
	register_native( "register_canvas_initializer", "nativeRegisterCanvasInitializer" );
	register_native( "canvas_get_pixels", "nativeCanvasGetPixels" );
	register_native( "canvas_set_pixels", "nativeCanvasSetPixels" );
	register_native( "canvas_get_width", "nativeCanvasGetWidth" );
	register_native( "canvas_get_height", "nativeCanvasGetHeight" );
	register_native( "canvas_get_size", "nativeCanvasGetSize" );
	register_native( "canvas_set_width", "nativeCanvasSetWidth" );
	register_native( "canvas_set_height", "nativeCanvasSetHeight" );
	register_native( "canvas_set_size", "nativeCanvasSetSize" );
}

public cmdCanvas ( id, level, cid )
{
	if( !cmd_access( id, level, cid, 1 ) )
		return PLUGIN_HANDLED;
		
	createCanvasByAim( id );
	return PLUGIN_HANDLED;
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
		
		if ( giTraceHooksActive == 0 )
		{
			unregister_forward( FM_TraceLine, ghTraceLine );
		}
	}
	
	return FMRES_IGNORED;
}

public nativeRegisterCanvasInitializer( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "register_canvas_initializer expects 3 arguments, %d given", argc );
		return -1;
	}
	
	get_string( 1, gCanvasInitializerNames[giCanvasInitializeIndex], CANVAS_MAX_INIT_NAME - 1 );

	new szCallback[32];
	get_string( 2, szCallback, 31 );
	
	gCanvasInitializers[giCanvasInitializeIndex][callback] = CreateOneForward( plugin, szCallback, FP_CELL, FP_CELL );
	gCanvasInitializers[giCanvasInitializeIndex][tempo] = get_param( 3 );
	
	return giCanvasInitializeIndex++;
}

public bool:nativeCanvasGetPixels( plugin, argc )
{
	static iColors[CANVAS_MAX_PIXELS];
	
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_pixels expects 3 arguments, %d given", argc );
		return false;
	}
	
	new canvas = get_param( 1 );
	new size = get_param( 3 );
	new Float:fColor[3];
	
	for ( new i = 0; i < CANVAS_MAX_PIXELS; i++ )
	{
		new ent = gCanvasPixels[canvas][i];
		
		if ( pev_valid( ent ) )
		{
			pev( ent, pev_rendercolor, fColor );
			iColors[i] = zipColor( 
				floatround(fColor[0]),
				floatround(fColor[1]),
				floatround(fColor[2])
			);
		}
		else
		{
			iColors[i] = 0; 
		}
	}
		
	set_array( 2, iColors, size );
	return true;
}

public bool:nativeCanvasSetPixels( plugin, argc )
{
	static iColors[CANVAS_MAX_PIXELS];
	
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_set_pixels expects 3 arguments, %d given", argc );
		return false;
	}
	
	new canvas = get_param( 1 );
	new size = min( get_param( 3 ), CANVAS_MAX_PIXELS );
	new iColor[3], Float:fColor[3];
		
	get_array( 2, iColor, size );
	
	for ( new i = 0; i < size; i++ )
	{
		new ent = gCanvasPixels[canvas][i];
		
		if ( pev_valid( ent ) )
		{
			unzipColor( iColors[i], iColor[0], iColor[1], iColor[2] );
			IVecFVec( iColor, fColor );
			set_pev( ent, pev_rendercolor, fColor );
		}
	}
	
	return true;
}

public nativeCanvasGetWidth( plugin, argc )
{
	if ( argc < 1 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_width expects 1 arguments, %d given", argc );
		return 0;
	}
	
	new canvas = get_param( 1 );
	return gCanvas[canvas][cols];
}

public nativeCanvasGetHeight( plugin, argc )
{
	if ( argc < 1 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_height expects 1 arguments, %d given", argc );
		return 0;
	}
	new canvas = get_param( 1 );
	return gCanvas[canvas][rows];
}

public nativeCanvasGetSize( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_size expects 3 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );	
	set_param_byref( 2, gCanvas[canvas][rows] );
	set_param_byref( 3, gCanvas[canvas][cols] );
}

public nativeCanvasSetWidth( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_set_width expects 2 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );
	gCanvas[canvas][cols] = get_param( 2 );
}

public nativeCanvasSetHeight( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_height expects 2 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );
	gCanvas[canvas][rows] = get_param( 2 );
}

public nativeCanvasSetSize( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_size expects 3 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );
	gCanvas[canvas][rows] = get_param( 2 );
	gCanvas[canvas][cols] = get_param( 3 );
}

public fwStartFrame()
{
	for ( new i = 0; i < giCanvasInitializeIndex; i++ )
	{
		new isReady = gCanvas[i][ready];
		
		if ( isReady )
		{
			new cb, ret;
			TrieGetCell( gPrograms, "Default", cb );
			
			if ( cb == -1 )
			{
				handleDefaultProgram( i );
			}
			else
			{
				ExecuteForward( cb , ret, i );
			}
		}
		else
		{
			creatingTick( i );
		}
	}
}

onCanvasReady( canvas )
{
	#pragma unused canvas
}

/**
 * Pixels of canvas can not be created instantly.
 * Each frame we got creation tick and decide which pixels should be created.
 *
 * Controller of this process is called Canvas Initializer.
 */
creatingTick( canvas )
{
	new tick = gCanvas[canvas][init_tick];
	new maxtick = gCanvas[canvas][init_maxtick];
	
	if ( tick >= maxtick )
	{
		gCanvas[canvas][ready] = 1;
		onCanvasReady( canvas );
		return;
	}
	
	new initCallback = gCanvasInitializers[0][callback];
	new initTempo = gCanvasInitializers[0][tempo];
	new pixelIndex;
	new i;
	
	for ( i = 0; i < initTempo; i++ )
	{
		if ( tick + i >= maxtick )
		{
			break;
		}
		
		if ( !ExecuteForward( initCallback, pixelIndex, canvas, tick + i ) )
		{
			pixelIndex = tick + i;
		}
	
		creatingTickByPixel( canvas, pixelIndex );
	}
	
	gCanvas[canvas][init_tick] = tick + i;
}

creatingTickByPixel( canvas, pixelIndex )
{		
	new canvasData[Canvas];
	canvasData = gCanvas[canvas];
	
	new Float:fBaseOrigin[3], Float:fDown[3], Float:fRight[3], Float:fAngle[3], pixelSize;
	fBaseOrigin[0] = Float:canvasData[originX];
	fBaseOrigin[1] = Float:canvasData[originY];
	fBaseOrigin[2] = Float:canvasData[originZ];
	
	fDown[0] = Float:canvasData[downX];
	fDown[1] = Float:canvasData[downY];
	fDown[2] = Float:canvasData[downZ];
	
	fRight[0] = Float:canvasData[rightX];
	fRight[1] = Float:canvasData[rightY];
	fRight[2] = Float:canvasData[rightZ];
	
	fAngle[0] = Float:canvasData[directionX];
	fAngle[1] = Float:canvasData[directionY];
	fAngle[2] = Float:canvasData[directionZ];
	
	pixelSize = canvasData[scale];
	
	new width = canvasData[cols];
	new height= canvasData[rows];
	new row = pixelIndex / width;
	new col = pixelIndex % width;
	
	new Float:fMyOrigin[3], Float:fMyDown[3], Float:fMyRight[3];
	
	xs_vec_mul_scalar( fDown, float( row - height / 2) * pixelSize, fMyDown );
	xs_vec_mul_scalar( fRight, float(col - width / 2) * pixelSize, fMyRight );
	
	xs_vec_copy( fBaseOrigin, fMyOrigin);
	xs_vec_add( fMyOrigin, fMyDown, fMyOrigin );
	xs_vec_add( fMyOrigin, fMyRight, fMyOrigin );
			
	gCanvasPixels[canvas][ pixelIndex ] = createPixel( fMyOrigin, fAngle, pixelSize );
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

createCanvas ( const Float:fOrigin[3], const Float:fVec[3], width = 28, height = 8, pixelsize = 8 )
{
	new Float:fAngle[3];
	fAngle[0] = -fVec[0]
	fAngle[1] = -fVec[1]
	fAngle[2] = fVec[2]
	vector_to_angle( fAngle, fAngle );
	
	new Float:fBaseOrigin[3];
	fBaseOrigin[0] = fOrigin[0] + fVec[0];
	fBaseOrigin[1] = fOrigin[1] + fVec[1];
	fBaseOrigin[2] = fOrigin[2] + fVec[2];
	
	new Float:fUp[3], Float:fDown[3], Float:fRight[3];
	angle_vector( fAngle, ANGLEVECTOR_UP, fUp );
	xs_vec_mul_scalar( fUp, -1.0, fDown );
	
	angle_vector( fAngle, ANGLEVECTOR_RIGHT, fRight );
	
	gCanvas[giCanvasIndex][cols] = width;
	gCanvas[giCanvasIndex][rows] = height;
	gCanvas[giCanvasIndex][scale] = pixelsize;

	gCanvas[giCanvasIndex][originX] = _:fBaseOrigin[0];
	gCanvas[giCanvasIndex][originY] = _:fBaseOrigin[1];
	gCanvas[giCanvasIndex][originZ] = _:fBaseOrigin[2];
	
	gCanvas[giCanvasIndex][directionX] = _:fAngle[0];
	gCanvas[giCanvasIndex][directionY] = _:fAngle[1];
	gCanvas[giCanvasIndex][directionZ] = _:fAngle[2];
	
	gCanvas[giCanvasIndex][rightX] = _:fRight[0];
	gCanvas[giCanvasIndex][rightY] = _:fRight[1];
	gCanvas[giCanvasIndex][rightZ] = _:fRight[2];
	
	gCanvas[giCanvasIndex][downX] = _:fDown[0];
	gCanvas[giCanvasIndex][downY] = _:fDown[1];
	gCanvas[giCanvasIndex][downZ] = _:fDown[2];
	
	
	gCanvas[giCanvasIndex][init_tick] = 0; 
	gCanvas[giCanvasIndex][ready] = 0; 
	gCanvas[giCanvasIndex][init_maxtick] = width * height; 
	
	return giCanvasIndex++;
}

createPixel( const Float: fOrigin[3], Float:fAngle[3], pixelSize )
{	
	new ent = engfunc( EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "info_target") );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_classname, "pixel" );
	set_pev( ent, pev_movetype, MOVETYPE_NONE );
	set_pev( ent, pev_solid, SOLID_NOT );
	engfunc( EngFunc_SetModel, ent, gszPixelModel );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_angles, fAngle );
	set_pev( ent, pev_scale,  0.125 * pixelSize );
	set_pev( ent, pev_rendercolor, Float:{ 1.0, 1.0, 1.0 } );
	set_pev( ent, pev_renderfx, 0 );
	set_pev( ent, pev_rendermode, kRenderNormal );
	set_pev( ent, pev_renderamt, 255 );
	
	return ent;
}

createProgram( const szName[], const szFunction[], plugin_id = -1 )
{
	new cb;
	
	if ( plugin_id == -1 )
	{
		cb = -1;
	}
	else
	{
		cb = CreateOneForward( plugin_id, szFunction, FP_CELL );
		
	}
	TrieSetCell( gPrograms, szName, cb );
	ArrayPushString( gProgramNames, szName );
}


handleDefaultProgram( canvas )
{
	for ( new i = 0; i < CANVAS_MAX_PIXELS; i++ )
	{
		new ent = gCanvasPixels[canvas][i];
		
		if ( pev_valid( ent ) )
		{
			new Float:fColor[3];
			fColor[0] = random_float(1.0, 255.0); 
			fColor[1] = fColor[0];
			fColor[2] = fColor[0];	
			
			set_pev( ent, pev_rendercolor, fColor );
		}
	}
}
