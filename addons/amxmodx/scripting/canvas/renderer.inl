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
#include <xs>

enum Canvas
{
	cols,
	rows,
	
	scale,
	ready,
	
	init_tick,
	init_maxtick,
	
	programId,
	
	//Origin
	originX,
	originY,
	originZ,
	
	//Origin
	zerozeroX,
	zerozeroY,
	zerozeroZ,
	
	//Angle
	directionX,
	directionY,
	directionZ,
	
	//Vector
	rightX,
	rightY,
	rightZ,
	
	//Vector
	downX,
	downY,
	downZ
};

enum CanvasInitializer
{
	callback,
	tempo
};

/**
 * Holds timestamp of first frame of actual rendering of canvas.
 * Index is canvas id.
 */
new Float:gCanvasStart[CANVAS_MAX_INSTANCES];

/**
 * Index is canvas id.
 */
new gCanvas[CANVAS_MAX_INSTANCES][Canvas];

/**
 * Array of pixels' colors. Linear array
 * First level index is canvas id. 
 * Second level index is index in linear array representing 2D board. 
 * If you convert from row and col notation, use expression: 
 *
 * 	row * WIDTH + col.
 */
new gCanvasPixels[CANVAS_MAX_INSTANCES][CANVAS_MAX_PIXELS];

/**
 * Next free index for new canvas.
 */
new giCanvasIndex = 0;

/**
 * Index and tempo of initializer for canvas.
 * First level index is canvas id. 
 * Second level is key from CanvasInitializer enum.
 */
new gCanvasInitializers[CANVAS_MAX_INITIALIZER][CanvasInitializer];

/**
 * Strings with initializers' names..
 * First level index is canvas id.
 */
new gCanvasInitializerNames[CANVAS_MAX_INITIALIZER][CANVAS_MAX_INIT_NAME];

/**
 * Next free index for new canvas initializer.
 */
new giCanvasInitializeIndex = 0;

/**
 * Called when initializer ends rendering and canvas is ready to draw on it.
 */
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

		if ( !ExecuteForward( initCallback, pixelIndex, canvas, tick + i ) || gCanvasPixels[canvas][ pixelIndex ] != 0 )
		{
			pixelIndex = tick + i;
		}
	
		creatingTickByPixel( canvas, pixelIndex );
	}
	
	gCanvas[canvas][init_tick] = tick + i;
}

/**
 * Create single pixel of canvas on given position.
 * Using plane and top-left corner position calculate world position for pixel sprite.
 *
 * @param cavnas Canvas id
 * @param pixelIndex Index of pixel. Use getPositionIndex( row, col, width ) to retrieve it.
 */
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
	
	xs_vec_mul_scalar( vfDown, float( row - height / 2 ) * pixelSize, vfMyDown );
	xs_vec_mul_scalar( vfRight, float( col - width / 2 ) * pixelSize, vfMyRight );
	
	xs_vec_copy( vfBaseOrigin, vfMyOrigin);
	xs_vec_add( vfMyOrigin, vfMyDown, vfMyOrigin );
	xs_vec_add( vfMyOrigin, vfMyRight, vfMyOrigin );
			
	gCanvasPixels[canvas][ pixelIndex ] = createPixelObject( canvas, vfMyOrigin, vfAngle, col, row, pixelSize );
}

/**
 * Setup new canvas instance.
 *
 * @param vfOrigin Position of center of canvas
 * @param vfVec Vector defining canvas direction
 * @param width Number of columns
 * @param height Number of rows
 * @param pixelsize Scale
 */
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

	//Center of canvas
	gCanvas[giCanvasIndex][originX] = _:vfBaseOrigin[0];
	gCanvas[giCanvasIndex][originY] = _:vfBaseOrigin[1];
	gCanvas[giCanvasIndex][originZ] = _:vfBaseOrigin[2];
	
	gCanvas[giCanvasIndex][directionX] = _:vfAngle[0];
	gCanvas[giCanvasIndex][directionY] = _:vfAngle[1];
	gCanvas[giCanvasIndex][directionZ] = _:vfAngle[2];
	
	//right vector
	gCanvas[giCanvasIndex][rightX] = _:vfRight[0];
	gCanvas[giCanvasIndex][rightY] = _:vfRight[1];
	gCanvas[giCanvasIndex][rightZ] = _:vfRight[2];
	
	//down vector
	gCanvas[giCanvasIndex][downX] = _:vfDown[0];
	gCanvas[giCanvasIndex][downY] = _:vfDown[1];
	gCanvas[giCanvasIndex][downZ] = _:vfDown[2];
	
	updateZeroZeroPoint( giCanvasIndex );
	
	gCanvas[giCanvasIndex][programId] = 0; 
	
	setSize( giCanvasIndex, width, height );
	
	return giCanvasIndex++;
}

/**
 * Remove all pixel entities from world
 *
 * @param canvas Canvas id
 */
hideCanvas( canvas )
{
	new w, h, len;
	getSize( canvas, w, h );
	
	len = w * h;
	
	for ( new i = 0; i < len; i++ )
	{
		hidePixel( canvas, i );	
	}
	
	gCanvas[canvas][ready] = 0;
}

/**
 * Let game loop reinit canvas if required
 *
 * @param canvas Canvas id
 */
showCanvas( canvas )
{
	if ( gCanvas[canvas][ready] == 0 )
	{
		gCanvas[canvas][init_tick] = 0;
	}
}

/**
 * Remove single pixel entity
 *
 * @param canvas Canvas id
 * @param pixelCanvas Pixel index
 */
hidePixel( canvas, pixelIndex )
{
	new ent = gCanvasPixels[canvas][pixelIndex];
	
	if ( pev_valid( ent ) )
	{
		engfunc( EngFunc_RemoveEntity, ent );
	}
	gCanvasPixels[canvas][pixelIndex] = 0;
}

/**
 * Update top-left corner position after resize or reposition.
 *
 * @param canvas Canvas id
 */
updateZeroZeroPoint( canvas )
{
	new Float:vfBaseOrigin[3], Float:vfRight[3], Float:vfDown[3];
	vfBaseOrigin[0] = Float:gCanvas[canvas][originX];
	vfBaseOrigin[1] = Float:gCanvas[canvas][originY];
	vfBaseOrigin[2] = Float:gCanvas[canvas][originZ];
	
	vfRight[0] = Float:gCanvas[canvas][rightX];
	vfRight[1] = Float:gCanvas[canvas][rightY];
	vfRight[2] = Float:gCanvas[canvas][rightZ];
	
	vfDown[0] = Float:gCanvas[canvas][downX];
	vfDown[1] = Float:gCanvas[canvas][downY];
	vfDown[2] = Float:gCanvas[canvas][downZ];
	
	new Float:vfZeroZero[3], Float:vfBuffor[3];
	new pixelsize = gCanvas[canvas][scale];
	new width = gCanvas[canvas][cols];
	new height = gCanvas[canvas][rows];
	
	//0,0 point
	xs_vec_copy( vfBaseOrigin, vfZeroZero );
	
	xs_vec_copy( vfRight, vfBuffor );
	xs_vec_mul_scalar( vfBuffor, pixelsize * width / 2.0 + pixelsize / 2.0, vfBuffor );
	xs_vec_sub( vfZeroZero, vfBuffor, vfZeroZero);
	
	xs_vec_copy( vfDown, vfBuffor );
	xs_vec_mul_scalar( vfBuffor, pixelsize * height / 2.0 + pixelsize / 2.0, vfBuffor);
	xs_vec_sub( vfZeroZero, vfBuffor, vfZeroZero);
	
	gCanvas[canvas][zerozeroX] = _:(vfZeroZero[0]);
	gCanvas[canvas][zerozeroY] = _:(vfZeroZero[1]);
	gCanvas[canvas][zerozeroZ] = _:(vfZeroZero[2]);
}

/**
 * Create pixel entity with square shaped sprite.
 *
 * @param canvas Canvas id
 * @param fOrigin Position of sprite
 * @param fAngle Angle of sprite
 * @param pixelSize Scale multiplier
 */
createPixelObject( canvas, const Float:fOrigin[3], Float:fAngle[3], col, row, pixelSize )
{	
	new ent = engfunc( EngFunc_CreateNamedEntity, info_target );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_classname, gszPixelClassName );
	set_pev( ent, pev_movetype, MOVETYPE_NONE );
	//set_pev( ent, pev_solid, SOLID_NOT );
	set_pev( ent, pev_solid, SOLID_TRIGGER );
	engfunc( EngFunc_SetModel, ent, gszPixelModel );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_angles, fAngle );
	set_pev( ent, pev_scale,  0.125 * pixelSize );
	set_pev( ent, pev_rendercolor, Float:{ 1.0, 1.0, 1.0 } );
	set_pev( ent, pev_renderfx, 0 );
	set_pev( ent, pev_rendermode, kRenderNormal );
	set_pev( ent, pev_renderamt, 255 );
	
	set_pev( ent, PEV_COL, col );
	set_pev( ent, PEV_ROW, row );
	set_pev( ent, pev_owner, canvas );
	
	return ent;
}

/**
 * Register new program
 *
 * @param szName String with program name
 * @param szFunction String with name of callback function called each server frame to draw canvas content
 * @param [forceWidth] Require exact number of columns required by program
 * @param [forceHeight] Require exact number of rows required by program
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
 * Dispatch program event to current canvas program.
 *
 * @param canvas Canvas id
 * @param szEvent String with name of event to trigger
 * @param [data] Array with extra parameters
 * @param [length] Length of array with extra parameters
 */
triggerEvent( canvas, const szEvent[], const data[] = {}, length = 0 )
{
	triggerProgramEvent( canvas, gCanvas[canvas][programId], szEvent, data, length );
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

/**
 * @param canvas Canvas id
 * @return Program id
 */
getProgram( canvas )
{
	return gCanvas[canvas][programId];
}

/**
 * Set all pixels to black
 *
 * @param canvas Canvas id
 */
clearScreen( canvas )
{
	for ( new i = 0; i < CANVAS_MAX_PIXELS; i++ )
	{
		new ent = gCanvasPixels[canvas][i];
		
		if ( pev_valid( ent ) )
		{
			set_pev( ent, pev_rendercolor, Float:{ 1.0, 1.0, 1.0 } );
		}
	}
}

/**
 * Change program. Lock canvas size if forced by program.
 *
 * @param canvas Canvas id
 * @param program Program id
 */
setProgram( canvas, program )
{
	disposeProgram( canvas, gCanvas[canvas][programId] );
	
	clearScreen( canvas );
	
	gCanvas[canvas][programId] = program;
	
	new forceSize[2];
	ArrayGetArray( gProgramForceSizes, program, forceSize );
	if ( forceSize[0] > 0 && forceSize[1] > 0 )
	{
		setSize( canvas, forceSize[0], forceSize[1], true );
	}
	
	setupProgram( canvas, program );
}

/**
 * Retrieve canvas size.
 *
 * @param canvas Canvas id
 * @param width Returned by reference value of width
 * @param height Returned by reference value of height
 */
getSize( canvas, &width, &height )
{
	width = gCanvas[canvas][cols];
	height = gCanvas[canvas][rows];
}

/**
 * Update canvas size. Optionally size can be forced.
 *
 * @param canvas Canvas id
 * @param width New width
 * @param height New height
 * @param [force=false] When true, size will be forced
 */
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
	
	updateZeroZeroPoint( canvas );
	return true;
}

/**
 * @param canvas Canvas id
 * @return Canvas size
 */
getScale( canvas )
{
	return gCanvas[canvas][scale];
}

/**
 * Update scale of canvas.
 *
 * @param canvas Canvas id
 * @param newScale New value of scale
 */
setScale( canvas, newScale )
{
	gCanvas[canvas][scale] = newScale;
	setSize( canvas, gCanvas[canvas][cols], gCanvas[canvas][rows], true );
	
	for ( new id = 1; id < 33; id++ )
	{
		if ( giCameraLocks[id] == canvas )
		{
			setCameraLock( id, canvas );
		}
	}
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
