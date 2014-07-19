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

public plugin_natives()
{
	register_library( "Canvas" );
	register_native( "register_canvas_initializer", "nativeRegisterCanvasInitializer" );
	register_native( "register_canvas_program", "nativeRegisterCanvasProgram" );
	register_native( "register_program_event", "nativeRegisterProgramEvent" );
	register_native( "unregister_program_event", "nativeUnregisterProgramEvent" );
	
	register_native( "canvas_lock_user_camera", "nativeCanvasLockUserCamera" );
	register_native( "canvas_unlock_user_camera", "nativeCanvasUnlockUserCamera" );
	
	register_native( "canvas_get_pixels", "nativeCanvasGetPixels" );
	register_native( "canvas_set_pixels", "nativeCanvasSetPixels" );
	
	register_native( "canvas_get_pixel", "nativeCanvasGetPixel" );
	register_native( "canvas_set_pixel", "nativeCanvasSetPixel" );
	
	register_native( "canvas_get_width", "nativeCanvasGetWidth" );
	register_native( "canvas_set_width", "nativeCanvasSetWidth" );
	
	register_native( "canvas_get_height", "nativeCanvasGetHeight" );
	register_native( "canvas_set_height", "nativeCanvasSetHeight" );
	
	register_native( "canvas_get_size", "nativeCanvasGetSize" );
	register_native( "canvas_set_size", "nativeCanvasSetSize" );
}

/**
 * Register canvas elements
 */
 
/**
 * Add new initialization handler. Map iteration number into pixel index to determine appearing order.
 *
 * @param szName String with name of initialize
 * @param szCallback String with name of public function to call
 * @param iTempo Count of pixels per tick
 * @param Initializer id
 */
//native register_canvas_initializer( const szName[], const szCallback[], iTempo = 1 );
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

/**
 * Add new program. Callback is function called on each frame for each canvas which use this program.
 * If forceWidth and forceHeight are greater than 0, program will force given size before render.
 * 
 * @param szName String with name of program
 * @param szCallback String with name of public function to call on each frame  
 *	callback( canvas, Float:detla )
 * @param [forceWidth]
 * @param [forceHeight]
 * @return Program id
 */
//native register_canvas_program( const szName[], const szCallback[], forceWidth = 0, forceHeight = 0 );
public nativeRegisterCanvasProgram( plugin, argc )
{
	if ( argc < 4 )
	{
		log_error( AMX_ERR_PARAMS, "register_canvas_program expects 4 arguments, %d given", argc );
		return -1;
	}
	
	new szName[CANVAS_MAX_PROGRAM_NAME];
	get_string( 1, szName, CANVAS_MAX_PROGRAM_NAME - 1 );
	
	new szCallback[32];
	get_string( 2, szCallback, charsmax(szCallback) );
	
	new forceWidth = get_param( 3 );
	new forceHeight = get_param( 4 );
	
	return createProgram( szName, szCallback, forceWidth, forceHeight, plugin );
}

/**
 * Register event callback for events. Each callback should have following signature.
 * 	callback( canvas, data[], length )
 *
 * Currently supported events:
 * 
 * "init" - on program begin, no data
 * "quit" - on program end, no data
 * "interaction:enter" - when player enters into interaction area, 
 *	data[0]  - player id
 * "interaction:leave" - when player leaves interaction area
 *	data[0] - player id
 * "interaction:quit" - when player lost lock ( like dead, spawn )
 *	data[0] - player id
 * "interaction:hover" - when player look at specific pixel
 *	data[0] - player id
 *	data[1] - col
 *	data[2] - row
 *
 * @param program Program id
 * @param szEvent String with name of event.
 * @param szCallback String with name of public function to call on event
 * @return callback handler
 */
//native register_program_event( program, const szEvent[], const szCallback[] );
public nativeRegisterProgramEvent( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "register_program_event expects 3 arguments, %d given", argc );
		return -1;
	}
	
	new program = get_param( 1 );
	
	new szEvent[32];
	get_string( 2 , szEvent, charsmax(szEvent) );
	
	new szCallback[32];
	get_string( 3, szCallback, charsmax(szCallback) );
	
	new cb = CreateOneForward( plugin, szCallback, FP_CELL, FP_ARRAY, FP_CELL );
	
	new Trie:events = ArrayGetCell( gProgramEvents, program );
	new Array:cbs;
	
	if ( !TrieGetCell( events, szEvent, cbs ) )
	{
		cbs = ArrayCreate();
		TrieSetCell( events, szEvent, cbs );
	}
	
	ArrayPushCell( cbs, cb );
	return ArraySize( cbs ) - 1;
}

/**
 * Remove event callback registered for event.
 *
 * @param program Program id
 * @param szEvent String with name of event.
 * @param handler Callback handler returned by register_program_event
 */
//native unregister_program_event( program, const szEvent[], handler );
public nativeUnregisterProgramEvent( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "unregister_program_event expects 3 arguments, %d given", argc );
		return -1;
	}
	
	new program = get_param( 1 );
	
	new szEvent[32];
	get_string( 2 , szEvent, charsmax(szEvent) );
	
	new handler = get_param( 3 );
	
	new Trie:events = ArrayGetCell( gProgramEvents, program );
	new Array:cbs;
	
	if ( !TrieGetCell( events, szEvent, cbs ) )
	{
		return 0;
	}
	
	if ( handler < 0 && handler >= ArraySize( cbs ) )
	{
		return 0;
	}
	
	ArraySetCell( cbs, handler, -1 );
	return 1;
}
 
 /**
 * Get colors of all pixels of canvas.
 * 
 * @param canvas  Canvas id
 * @param pixels Array to be filled by colors
 * @param size Length of array, for safe copying
 * @return True on success, false otherwise
 */
//native bool:canvas_get_pixels( canvas, pixels[], size = CANVAS_MAX_PIXELS );
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

/**
 * Set colors of all pixesl of canvas
 *
 * @param canvas Canvas id
 * @param pixels Array of colors to apply into canvas
 * @param size Length of array, for safe copying
 * @return True on success, false otherwise
 */
//native bool:canvas_set_pixels( canvas, pixels[], size = CANVAS_MAX_PIXELS );
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
	
	get_array( 2, iColors, size );
	
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

/**
 * Get color of particular pixel of canvas.
 * 
 * @param canvas  Canvas id
 * @param col Pixel column
 * @param row Pixel row
 * @return Zipped color of pixel, -1 if col/row is invalid
 */
//native canvas_get_pixel( canvas, col, row );
public nativeCanvasGetPixel( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_pixel expects 3 arguments, %d given", argc );
		return -1;
	}
	
	new canvas = get_param( 1 );
	new col = get_param( 2 );
	new row = get_param( 3 );
	new width = gCanvas[canvas][cols];
	new height = gCanvas[canvas][rows];
	new ent = gCanvasPixels[canvas][ row * width + col ];
	
	if ( col < 0 || col >= width || row < 0 || row >= height )
	{
		return -1;
	}
	
	new iColor[3], Float:fColor[3];
	pev( ent, pev_rendercolor, fColor );
	FVecIVec( fColor, iColor );
	return zipColor( iColor[0], iColor[1], iColor[2] );
}

/**
 * Set color of particular pixel of canvas.
 *
 * @param canvas Canvas id
 * @param col Pixel column
 * @param row Pixel row
 * @param color Zipped color of pixel
 */
//native canvas_set_pixel( canvas, col, row, color );
public nativeCanvasSetPixel( plugin, argc )
{
	if ( argc < 4 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_set_pixel expects 4 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );
	new col = get_param( 2 );
	new row = get_param( 3 );
	new color = get_param( 4 );
	new width = gCanvas[canvas][cols];
	new height = gCanvas[canvas][rows];
	
	if ( col < 0 || col >= width || row < 0 || row >= height )
	{
		return;
	}
	
	new ent = gCanvasPixels[canvas][ row * width + col ];
	
	new iColor[3], Float:fColor[3];
	unzipColor( color, iColor[0], iColor[1], iColor[2] );
	IVecFVec( iColor, fColor );
	set_pev( ent, pev_rendercolor, fColor );
}

 /**
 * Lock player camera on given canvas. 
 * If no canvas specified, lock will be set on canvas from current interaction.
 * If no interaction, no action will be taken.
 *
 * @param id Player id
 * @param canvas Canvas id
 */
//native canvas_lock_user_camera( id, canvas = -1 );
public nativeCanvasLockUserCamera( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_lock_user_camera expects 2 arguments, %d given", argc );
		return -1;
	}
	
	new id = get_param( 1 );
	new canvas = get_param( 2 );
	
	if ( setCameraLock( id, canvas ) )
	{
		return giInteractionCanvas[id];
	}
	
	return -1;
}


/**
 * Release lock on player's camera
 * 
 * @param id Player id
 */
//native canvas_unlock_user_camera( id );
public nativeCanvasUnlockUserCamera( plugin, argc )
{
	if ( argc < 1 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_unlock_user_camera expects 1 arguments, %d given", argc );
		return -1;
	}
	
	new id = get_param( 1 );
	
	if ( releaseCameraLock( id ) )
	{
		return giInteractionCanvas[id];
	}
	
	return -1;
}

/**
 * @param canvas Canvas id
 * @return Canvas width in pixels
 */
//native canvas_get_width( canvas );
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

/**
 * @param canvas Canvas id
 * @return Canvas height in pixels
 */
//native canvas_get_height( canvas );
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

/**
 * @param canvas Canvas id
 * @param &width Referenced variable will be filled with canvas width in pixels
 * @param &height Referenced variable will be filled with canvas height in pixels
 */
//native canvas_get_size( canvas, &width, &height );
public nativeCanvasGetSize( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_size expects 3 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );	
	set_param_byref( 2, gCanvas[canvas][cols] );
	set_param_byref( 3, gCanvas[canvas][rows] );
}

/**
 * @param canvas Canvas id
 * @param width New width in pixels
 */
//native canvas_set_width( canvas, width );
public nativeCanvasSetWidth( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_set_width expects 2 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );
	setSize( canvas, get_param( 2 ), gCanvas[canvas][rows] );
}

/**
 * @param canvas Canvas id
 * @param height New height in pixels
 */
//native canvas_set_height( canvas, height );
public nativeCanvasSetHeight( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_height expects 2 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );
	setSize( canvas, gCanvas[canvas][cols], get_param( 2 ) );
}

/**
 * @param canvas Canvas id
 * @param width Canvas width in pixels
 * @param height Canvas height in pixels
 */
//native canvas_set_size( canvas, width, height );
public nativeCanvasSetSize( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_size expects 3 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );	
	setSize( canvas, get_param( 2 ), get_param( 3 ) );
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
