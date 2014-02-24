#include <amxmodx>
#include <amxmisc>
#include <fakemeta>

enum Canvas
{
	cols,
	rows,
	
	scale,
	ready,
	
	init_tick,
	init_maxtick,
	
	programId,
	
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

new bool:gbTraceHooks[32];
new giTraceHooksActive = 0;
new ghTraceLine;

new Float:gCanvasStart[CANVAS_MAX_INSTANCES];
new gCanvas[CANVAS_MAX_INSTANCES][Canvas];
new gCanvasPixels[CANVAS_MAX_INSTANCES][CANVAS_MAX_PIXELS];
new giCanvasIndex = 0;

new gCanvasInitializers[CANVAS_MAX_INITIALIZER][CanvasInitializer];
new gCanvasInitializerNames[CANVAS_MAX_INITIALIZER][CANVAS_MAX_INIT_NAME];
new giCanvasInitializeIndex = 0;


onCanvasReady( canvas )
{
	gCanvasStart[canvas] = get_gametime();
}

/**
 * Pixels of canvas can not be created instantly.
 * Each frame we got creation tick and decide which pixels should be created.
 *
 * Controller of this process is called Canvas Initializer.
 * 
 * @param canvas Canvas id
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
	
	new Float:vfBaseOrigin[3], Float:vfDown[3], Float:vfRight[3], Float:vfAngle[3], pixelSize;
	vfBaseOrigin[0] = Float:canvasData[originX];
	vfBaseOrigin[1] = Float:canvasData[originY];
	vfBaseOrigin[2] = Float:canvasData[originZ];
	
	vfDown[0] = Float:canvasData[downX];
	vfDown[1] = Float:canvasData[downY];
	vfDown[2] = Float:canvasData[downZ];
	
	vfRight[0] = Float:canvasData[rightX];
	vfRight[1] = Float:canvasData[rightY];
	vfRight[2] = Float:canvasData[rightZ];
	
	vfAngle[0] = Float:canvasData[directionX];
	vfAngle[1] = Float:canvasData[directionY];
	vfAngle[2] = Float:canvasData[directionZ];
	
	pixelSize = canvasData[scale];
	
	new width = canvasData[cols];
	new height= canvasData[rows];
	new row = pixelIndex / width;
	new col = pixelIndex % width;
	
	new Float:vfMyOrigin[3], Float:vfMyDown[3], Float:vfMyRight[3];
	
	xs_vec_mul_scalar( vfDown, float( row - height / 2) * pixelSize, vfMyDown );
	xs_vec_mul_scalar( vfRight, float(col - width / 2) * pixelSize, vfMyRight );
	
	xs_vec_copy( vfBaseOrigin, vfMyOrigin);
	xs_vec_add( vfMyOrigin, vfMyDown, vfMyOrigin );
	xs_vec_add( vfMyOrigin, vfMyRight, vfMyOrigin );
			
	gCanvasPixels[canvas][ pixelIndex ] = createPixel( vfMyOrigin, vfAngle, pixelSize );
}

createCanvas ( const Float:vfOrigin[3], const Float:vfVec[3], width = 28, height = 8, pixelsize = 8 )
{
	
	addCanvasMenuItem( "Canvas #%d", giCanvasIndex + 1 );
	
	new Float:vfAngle[3];
	vfAngle[0] = -vfVec[0]
	vfAngle[1] = -vfVec[1]
	vfAngle[2] = vfVec[2]
	vector_to_angle( vfAngle, vfAngle );
	
	new Float:vfBaseOrigin[3];
	vfBaseOrigin[0] = vfOrigin[0] + vfVec[0];
	vfBaseOrigin[1] = vfOrigin[1] + vfVec[1];
	vfBaseOrigin[2] = vfOrigin[2] + vfVec[2];
	
	new Float:vfUp[3], Float:vfDown[3], Float:vfRight[3];
	angle_vector( vfAngle, ANGLEVECTOR_UP, vfUp );
	xs_vec_mul_scalar( vfUp, -1.0, vfDown );
	
	angle_vector( vfAngle, ANGLEVECTOR_RIGHT, vfRight );
	
	gCanvas[giCanvasIndex][scale] = pixelsize;

	gCanvas[giCanvasIndex][originX] = _:vfBaseOrigin[0];
	gCanvas[giCanvasIndex][originY] = _:vfBaseOrigin[1];
	gCanvas[giCanvasIndex][originZ] = _:vfBaseOrigin[2];
	
	gCanvas[giCanvasIndex][directionX] = _:vfAngle[0];
	gCanvas[giCanvasIndex][directionY] = _:vfAngle[1];
	gCanvas[giCanvasIndex][directionZ] = _:vfAngle[2];
	
	gCanvas[giCanvasIndex][rightX] = _:vfRight[0];
	gCanvas[giCanvasIndex][rightY] = _:vfRight[1];
	gCanvas[giCanvasIndex][rightZ] = _:vfRight[2];
	
	gCanvas[giCanvasIndex][downX] = _:vfDown[0];
	gCanvas[giCanvasIndex][downY] = _:vfDown[1];
	gCanvas[giCanvasIndex][downZ] = _:vfDown[2];
	
	gCanvas[giCanvasIndex][programId] = 0; 
	
	setSize( giCanvasIndex, width, height );
	
	return giCanvasIndex++;
}

createPixel( const Float: fOrigin[3], Float:fAngle[3], pixelSize )
{	
	new ent = engfunc( EngFunc_CreateNamedEntity, info_target );
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

/**
 * 
 *
 * @param szName String with program name
 * @param szFunction String with name of callback function called each server frame to draw canvas content
 * @param [forceWidth]
 * @param [forceHeight]
 * @param [plugin_id] Id of plugin to search for function. When -1, look for function it this plugin.
 */
createProgram( const szName[], const szFunction[], forceWidth = 0, forceHeight = 0, plugin_id = -1 )
{
	new cb;
	
	if ( plugin_id == -1 )
	{
		cb = -1;
	}
	else
	{
		cb = CreateOneForward( plugin_id, szFunction, FP_CELL, FP_FLOAT );
		
	}
	
	addProgramMenuItem( szName );
	ArrayPushCell( gPrograms, cb );
	
	new forceSize[2];
	forceSize[0] = forceWidth;
	forceSize[1] = forceHeight;
	ArrayPushArray( gProgramForceSizes, forceSize );
	
	new Trie:events = TrieCreate();
	ArrayPushCell( gProgramEvents, events );
	
	return ArraySize( gPrograms ) - 1;
}

/**
 * Update handler of default program. Render greyscaled noise.
 *
 * @param canvas Canvas id
 */
handleDefaultProgram( canvas, Float:fDelta )
{
	#pragma unused fDelta
	
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

/**
 * Dispatch program event. Currently are events have to take 
 *	callback( canvas, data[], length )
 *
 * @param canvas Canvas id
 * @param program Program id
 * @param szEvent String with name of event to trigger
 * @param [data] Array with extra parameters
 * @param [length] Length of array with extra parameters
 */
triggerProgramEvent( canvas, program, const szEvent[], const data[] = {}, length = 0 )
{
	new Trie:events = ArrayGetCell( gProgramEvents, program );
	new Array:callbacks;
	
	if ( !TrieGetCell( events, szEvent, callbacks ) )
	{
		return;
	}
	
	for ( new i = 0; i < ArraySize( callbacks ); i++ )
	{
		new fw = ArrayGetCell( callbacks, i );
		new ret;
		ExecuteForward( fw, ret, canvas, PrepareArray( data ,length ), length );
	}
}

/**
 * Dispatch quit event for not default programs.
 *
 * @param canvas Canvas id.
 * @param program Program id
 */
disposeProgram( canvas, program )
{
	if ( program )
	{
		triggerProgramEvent( canvas, program, "quit" );
	}
}

/**
 * Dispatch init event for not default programs.
 * 
 * @param canvas Canvas id
 * @param program Program id
 */
setupProgram( canvas, program )
{
	if ( program )
	{
		triggerProgramEvent( canvas, program, "init" );
	}
}



getProgram( canvas )
{
	return gCanvas[canvas][programId];
}

setProgram( canvas, program )
{
	disposeProgram( canvas, gCanvas[canvas][programId] );
	
	gCanvas[canvas][programId] = program;
	
	new forceSize[2];
	ArrayGetArray( gProgramForceSizes, program, forceSize );
	if ( forceSize[0] > 0 && forceSize[1] > 0 )
	{
		setSize( canvas, forceSize[0], forceSize[1], true );
	}
	
	setupProgram( canvas, program );
}

getSize( canvas, &width, &height )
{
	width = gCanvas[canvas][cols];
	height = gCanvas[canvas][rows];
}

bool:setSize( canvas, width, height, bool:force = false )
{
	new program = getProgram( canvas );
	new forceSize[2];
	ArrayGetArray( gProgramForceSizes, program, forceSize );
	
	//Ignore when size is forced
	if ( !force && forceSize[0] > 0 && forceSize[1] > 0 )
	{
		return false;
	}
	
	gCanvas[canvas][cols] = width;
	gCanvas[canvas][rows] = height;
	
	gCanvas[canvas][init_tick] = 0; 
	gCanvas[canvas][ready] = 0; 
	gCanvas[canvas][init_maxtick] = width * height; 
	
	for ( new i = 0; i < CANVAS_MAX_PIXELS; i++ )
	{
		new ent = gCanvasPixels[canvas][i];
		
		if ( pev_valid( ent ) )
		{
			engfunc( EngFunc_RemoveEntity, ent );
		}
		
		gCanvasPixels[canvas][i] = 0;
	}
	return true;
}

getScale( canvas )
{
	return gCanvas[canvas][scale];
}

setScale( canvas, newScale )
{
	gCanvas[canvas][scale] = newScale;
	setSize( canvas, gCanvas[canvas][cols], gCanvas[canvas][rows], true );
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
