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
 * This plugin is library which provide basic API for canvas and pixels manipulation methods.
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
}

enum CanvasInitializer
{
	callback,
	tempo
}

#define PLUGIN "Canvas"
#define VERSION "0.0.1"
#define AUTHOR "R3X"

new bool:gbTraceHooks[32];
new giTraceHooksActive = 0;
new ghTraceLine;

new Array:gCanvas;
new Array:gCanvasReady;
new Array:gCanvasTick;
new Array:gCanvasMaxTick;
new Array:gCanvasPixels;
new Array:gCanvasInitializers;
new Array:gCanvasInitializerNames;

new Trie:gPrograms;
new Array:gProgramNames

new const gszPixelModel[] = "sprites/pixel.spr";

public plugin_init ()
{
	register_plugin( PLUGIN, VERSION, AUTHOR );
	
	register_clcmd( "amx_canvas", "cmdCanvas", ADMIN_CFG );
	
	register_forward( FM_StartFrame, "fwStartFrame", 1 );
	
	gCanvas = ArrayCreate( _:Canvas );
	gCanvasReady = ArrayCreate();
	gCanvasTick = ArrayCreate();
	gCanvasMaxTick = ArrayCreate();
	gCanvasPixels = ArrayCreate( CANVAS_MAX_PIXELS);
	gCanvasInitializers = ArrayCreate( _:CanvasInitializer );
	gCanvasInitializerNames = ArrayCreate( CANVAS_MAX_INIT_NAME );
	
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
	
	
	new szName[CANVAS_MAX_INIT_NAME];
	get_string( 1, szName, CANVAS_MAX_INIT_NAME - 1 );
	
	new canvasInitializer[_:CanvasInitializer];
	new szCallback[32];
	get_string( 2, szCallback, 31 );
	canvasInitializer[_:callback] = CreateOneForward( plugin, szCallback, FP_CELL, FP_CELL );
	canvasInitializer[_:tempo] = get_param( 3 );
	
	ArrayPushArray( gCanvasInitializers, canvasInitializer );
	ArrayPushArray( gCanvasInitializerNames, szName );
	return ArraySize( gCanvasInitializers ) - 1;
}

public bool:nativeCanvasGetPixels( plugin, argc )
{
	static iPixels[CANVAS_MAX_PIXELS];
	static iColors[CANVAS_MAX_PIXELS];
	
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_pixels expects 3 arguments, %d given", argc );
		return false;
	}
	
	new canvas = get_param( 1 );
	new size = get_param( 3 );
	new Float:fColor[3];
	
	ArrayGetArray( gCanvasPixels, canvas, iPixels );
	
	for ( new i = 0; i < CANVAS_MAX_PIXELS; i++ )
	{
		new ent = iPixels[i];
		
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
	static iPixels[CANVAS_MAX_PIXELS];
	static iColors[CANVAS_MAX_PIXELS];
	
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_set_pixels expects 3 arguments, %d given", argc );
		return false;
	}
	
	new canvas = get_param( 1 );
	new size = min( get_param( 3 ), CANVAS_MAX_PIXELS );
	new iColor[3], Float:fColor[3];
	
	ArrayGetArray( gCanvasPixels, canvas, iPixels );
	
	get_array( 2, iColor, size );
	
	for ( new i = 0; i < size; i++ )
	{
		new ent = iPixels[i];
		
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
	
	new canvasData[_:Canvas];
	new canvas = get_param( 1 );
	ArrayGetArray( gCanvas, canvas, canvasData );
	return canvasData[_:cols];
}

public nativeCanvasGetHeight( plugin, argc )
{
	if ( argc < 1 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_height expects 1 arguments, %d given", argc );
		return 0;
	}
	
	new canvasData[_:Canvas];
	new canvas = get_param( 1 );
	ArrayGetArray( gCanvas, canvas, canvasData );
	return canvasData[_:rows];
}

public nativeCanvasGetSize( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_size expects 3 arguments, %d given", argc );
		return;
	}
	
	new canvasData[_:Canvas];
	new canvas = get_param( 1 );
	ArrayGetArray( gCanvas, canvas, canvasData );
	
	set_param_byref( 2, canvasData[_:rows] );
	set_param_byref( 3, canvasData[_:cols] );
}

public nativeCanvasSetWidth( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_set_width expects 2 arguments, %d given", argc );
		return;
	}
	
	new canvasData[_:Canvas];
	new canvas = get_param( 1 );
	ArrayGetArray( gCanvas, canvas, canvasData );
	canvasData[_:cols] = get_param( 2 );
	ArraySetArray( gCanvas, canvas, canvasData );
}

public nativeCanvasSetHeight( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_height expects 2 arguments, %d given", argc );
		return;
	}
	
	new canvasData[_:Canvas];
	new canvas = get_param( 1 );
	ArrayGetArray( gCanvas, canvas, canvasData );
	canvasData[_:rows] = get_param( 2 );
	ArraySetArray( gCanvas, canvas, canvasData );
}

public nativeCanvasSetSize( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_size expects 3 arguments, %d given", argc );
		return;
	}
	
	new canvasData[_:Canvas];
	new canvas = get_param( 1 );
	ArrayGetArray( gCanvas, canvas, canvasData );
	canvasData[_:rows] = get_param( 2 );
	canvasData[_:cols] = get_param( 3 );
	ArraySetArray( gCanvas, canvas, canvasData );
}
public fwStartFrame()
{
	new canvasNum = ArraySize( gCanvas );
	
	for ( new i = 0; i < canvasNum; i++ )
	{
		new ready = ArrayGetCell( gCanvasReady, i );
		
		if ( ready )
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
	new tick = ArrayGetCell( gCanvasTick, canvas );
	new maxtick = ArrayGetCell( gCanvasMaxTick, canvas );
	
	if ( tick >= maxtick )
	{
		ArraySetCell( gCanvasReady, canvas, 1 );
		onCanvasReady( canvas );
		return;
	}
	
	new init[2];
	ArrayGetArray( gCanvasInitializers, 0, init );
	
	new initCallback = init[0];
	new initTempo = init[1];
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
	
	ArraySetCell( gCanvasTick, canvas, tick + i );
}

creatingTickByPixel( canvas, pixelIndex )
{
	new pixels[CANVAS_MAX_PIXELS];
	ArrayGetArray(gCanvasPixels, canvas, pixels);
	
	new canvasData[_:Canvas];
	ArrayGetArray( gCanvas, canvas, canvasData );
	
	new Float:fBaseOrigin[3], Float:fDown[3], Float:fRight[3], Float:fAngle[3], pixelSize;
	fBaseOrigin[0] = Float:canvasData[_:originX];
	fBaseOrigin[1] = Float:canvasData[_:originY];
	fBaseOrigin[2] = Float:canvasData[_:originZ];
	
	fDown[0] = Float:canvasData[_:downX];
	fDown[1] = Float:canvasData[_:downY];
	fDown[2] = Float:canvasData[_:downZ];
	
	fRight[0] = Float:canvasData[_:rightX];
	fRight[1] = Float:canvasData[_:rightY];
	fRight[2] = Float:canvasData[_:rightZ];
	
	fAngle[0] = Float:canvasData[_:directionX];
	fAngle[1] = Float:canvasData[_:directionY];
	fAngle[2] = Float:canvasData[_:directionZ];
	
	pixelSize = canvasData[_:scale];
	
	new width = canvasData[_:cols];
	new height= canvasData[_:rows];
	new row = pixelIndex / width;
	new col = pixelIndex % width;
	
	new Float:fMyOrigin[3], Float:fMyDown[3], Float:fMyRight[3];
	
	xs_vec_mul_scalar( fDown, float( row - height / 2) * pixelSize, fMyDown );
	xs_vec_mul_scalar( fRight, float(col - width / 2) * pixelSize, fMyRight );
	
	xs_vec_copy( fBaseOrigin, fMyOrigin);
	xs_vec_add( fMyOrigin, fMyDown, fMyOrigin );
	xs_vec_add( fMyOrigin, fMyRight, fMyOrigin );
			
	pixels[ pixelIndex ] = createPixel( fMyOrigin, fAngle, pixelSize );
	ArraySetArray( gCanvasPixels, canvas, pixels );
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
	
	new canvasData[_:Canvas];
	canvasData[_:cols] = width;
	canvasData[_:rows] = height;
	canvasData[_:scale] = pixelsize;

	canvasData[_:originX] = _:fBaseOrigin[0];
	canvasData[_:originY] = _:fBaseOrigin[1];
	canvasData[_:originZ] = _:fBaseOrigin[2];
	
	canvasData[_:directionX] = _:fAngle[0];
	canvasData[_:directionY] = _:fAngle[1];
	canvasData[_:directionZ] = _:fAngle[2];
	
	canvasData[_:rightX] = _:fRight[0];
	canvasData[_:rightY] = _:fRight[1];
	canvasData[_:rightZ] = _:fRight[2];
	
	canvasData[_:downX] = _:fDown[0];
	canvasData[_:downY] = _:fDown[1];
	canvasData[_:downZ] = _:fDown[2];
	
	new pixels[CANVAS_MAX_PIXELS];
	
	ArrayPushArray( gCanvas, canvasData );
	ArrayPushCell( gCanvasReady, 0 );
	ArrayPushArray( gCanvasPixels, pixels );
	ArrayPushCell( gCanvasTick, 0 );
	ArrayPushCell( gCanvasMaxTick, width * height );
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
	static pixels[CANVAS_MAX_PIXELS];
	ArrayGetArray( gCanvasPixels, canvas, pixels );
	for ( new i = 0; i < CANVAS_MAX_PIXELS; i++ )
	{
		new ent = pixels[i];
		
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
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
