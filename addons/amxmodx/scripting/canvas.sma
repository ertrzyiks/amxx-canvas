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
	
	new canvasInitializer[CanvasInitializer];
	new szCallback[32];
	get_string( 2, szCallback, 31 );
	canvasInitializer[callback] = CreateOneForward( plugin, szCallback, FP_CELL, FP_CELL );
	canvasInitializer[tempo] = get_param( 3 );
	
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

public fwStartFrame()
{
	new canvasNum = ArraySize( gCanvas );
	
	for ( new i = 0; i < canvasNum; i++ )
	{
		new ready = ArrayGetCell( gCanvasReady, i );
		
		if ( ! ready )
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
	
	for ( new i = 0; i < initTempo; i++ )
	{
		
		if ( !ExecuteForward( initCallback, pixelIndex, canvas, tick + i ) )
		{
			pixelIndex = tick + i;
		}
	
		creatingTickByPixel( canvas, pixelIndex );
		
	}
	
	ArraySetCell( gCanvasTick, canvas, tick + initTempo );
}

creatingTickByPixel( canvas, pixelIndex )
{
	new pixels[CANVAS_MAX_PIXELS];
	ArrayGetArray(gCanvasPixels, canvas, pixels);
	
	new canvasData[Canvas];
	ArrayGetArray( gCanvas, canvas, canvasData );
	
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
	new row = pixelIndex / width;
	new col = pixelIndex % width;
	
	new Float:fMyOrigin[3], Float:fMyDown[3], Float:fMyRight[3];
	
	xs_vec_mul_scalar( fDown, float(row) * pixelSize, fMyDown );
	xs_vec_mul_scalar( fRight, float(col) * pixelSize, fMyRight );
	
	xs_vec_copy( fBaseOrigin, fMyOrigin);
	xs_vec_add( fMyOrigin, fMyDown, fMyOrigin );
	xs_vec_add( fMyOrigin, fMyRight, fMyOrigin );
			
	pixels[ pixelSize ] = createPixel( fMyOrigin, fAngle, pixelSize );
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
	
	new canvasData[Canvas];
	canvasData[cols] = width;
	canvasData[rows] = height;
	canvasData[scale] = pixelsize;

	canvasData[originX] = _:fBaseOrigin[0];
	canvasData[originY] = _:fBaseOrigin[1];
	canvasData[originZ] = _:fBaseOrigin[2];
	
	canvasData[directionX] = _:fAngle[0];
	canvasData[directionY] = _:fAngle[1];
	canvasData[directionZ] = _:fAngle[2];
	
	canvasData[rightX] = _:fRight[0];
	canvasData[rightY] = _:fRight[1];
	canvasData[rightZ] = _:fRight[2];
	
	canvasData[downX] = _:fDown[0];
	canvasData[downY] = _:fDown[1];
	canvasData[downZ] = _:fDown[2];
	
	new pixels[CANVAS_MAX_PIXELS];
	
	ArrayPushArray( gCanvas, canvasData );
	ArrayPushCell( gCanvasReady, 0 );
	ArrayPushArray( gCanvasPixels, pixels );
	ArrayPushCell( gCanvasTick, 0 );
	ArrayPushCell( gCanvasMaxTick, width * height );
}

createPixel( const Float: fOrigin[3], Float:fAngle[3], pixelSize )
{
	new Float:fColor[3];
	fColor[0] = random_float( 1.0, 255.0 );
	fColor[1] = random_float( 1.0, 255.0 );
	fColor[2] = random_float( 1.0, 255.0 );
	
	new ent = engfunc( EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "info_target") );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_classname, "pixel" );
	set_pev( ent, pev_movetype, MOVETYPE_NONE );
	set_pev( ent, pev_solid, SOLID_NOT );
	engfunc( EngFunc_SetModel, ent, gszPixelModel );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_angles, fAngle );
	set_pev( ent, pev_scale,  0.125 * pixelSize );
	set_pev( ent, pev_rendercolor, fColor );
	set_pev( ent, pev_renderfx, 0 );
	set_pev( ent, pev_rendermode, kRenderNormal );
	set_pev( ent, pev_renderamt, 255 );
	
	return ent;
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
