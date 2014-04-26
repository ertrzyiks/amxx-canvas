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
 */
#if defined _canvas_included
  #endinput
#endif
#define _canvas_included

#define CANVAS_MAX_INSTANCES 12
#define CANVAS_MAX_INITIALIZER 8

#define CANVAS_MAX_PIXELS 256
#define CANVAS_MAX_INIT_NAME 32
#define CANVAS_MAX_PROGRAM_NAME 32


/**
 * Extracts channel values from compressed color.
 * Returns values by reference.
 * 
 * @param color Compressed color
 * @param &r Value of red channel
 * @param &g Value of green channel
 * @param &b Value of blue channel
 */
stock unzipColor( color, &r, &g, &b )
{
	r = (color & 0x00FF0000) >> 16;
	g = (color & 0x0000FF00) >> 8;
	b = (color & 0x000000FF) >> 0;
}

/**
 * Compress color channels into 4 byte length variable
 *
 * @param r Value of red channel
 * @param g Value of green channel
 * @param b Value of blue channel
 * @return Compressed color
 */
stock zipColor( r, g, b )
{
	//0,0,0 render as white, so we need 1,1,1 as black
	if ( r == 0 && g == 0 && b == 0)
	{
		r = g = b = 1;
	}
	
	r = r & 0xFF;
	g = g & 0xFF;
	b = b & 0xFF;
	
	return r << 16 | g << 8 | b << 0;
}

/**
 * Takes compressed color and return value with modified red channel 
 *
 * @param color Compressed color
 * @param newR New value of red channel
 * @return Compressed color with modified red channel
 */
stock setR( color, newR )
{
	new r, g, b;
	unzipColor( color, r, g, b );
	return zipColor( newR, g, b );
}

/**
 * Takes compressed color and return value with modified green channel 
 *
 * @param color Compressed color
 * @param newG New value of green channel
 * @return Compressed color with modified green channel
 */
stock setG( color, newG )
{
	new r, g, b;
	unzipColor( color, r, g, b );
	return zipColor( r, newG, b );
}

/**
 * Takes compressed color and return value with modified blue channel 
 *
 * @param color Compressed color
 * @param newB New value of blue channel
 * @return Compressed color with modified blue channel
 */
stock setB( color, newB )
{
	new r, g, b;
	unzipColor( color, r, g, b );
	return zipColor( r, g, newB );
}


/**
 * Add new initialization handler. Map iteration number into pixel index to determine appearing order.
 *
 * @param szName String with name of initialize
 * @param szCallacbk String with name of public function to call
 * @param iTempo Count of pixels per tick
 * @param Initializer id
 */
native register_canvas_initializer( const szName[], const szCallback[], iTempo = 1 );


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
native register_canvas_program( const szName[], const szCallback[], forceWidth = 0, forceHeight = 0 );

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
native register_program_event( program, const szEvent[], const szCallback[] );

/**
 * Remove event callback registered for event.
 *
 * @param program Program id
 * @param szEvent String with name of event.
 * @param handler Callback handler returned by register_program_event
 */
native unregister_program_event( program, const szEvent[], handler );


/**
 * Lock player camera on given canvas. 
 * If no canvas specified, lock will be set on canvas from current interaction.
 * If no interaction, no action will be taken.
 *
 * @param id Player id
 * @param canvas Canvas id
 */
native canvas_lock_user_camera( id, canvas = -1 );

/**
 * Release lock on player's camera
 * 
 * @param id Player id
 */
native canvas_unlock_user_camera( id );




/**
 * Get colors of all pixels of canvas.
 * 
 * @param canvas  Canvas id
 * @param pixels Array to be filled by colors
 * @param size Length of array, for safe copying
 * @return True on success, false otherwise
 */
native bool:canvas_get_pixels( canvas, pixels[], size = CANVAS_MAX_PIXELS );

/**
 * Set colors of all pixesl of canvas
 *
 * @param canvas Canvas id
 * @param pixels Array of colors to apply into canvas
 * @param size Length of array, for safe copying
 * @return True on success, false otherwise
 */
native bool:canvas_set_pixels( canvas, pixels[], size = CANVAS_MAX_PIXELS );


/**
 * Get color of particular pixel of canvas.
 * 
 * @param canvas  Canvas id
 * @param col Pixel column
 * @param row Pixel row
 * @return Zipped color of pixel, -1 if col/row is invalid
 */
native canvas_get_pixel( canvas, col, row );

/**
 * Set color of particular pixel of canvas.
 *
 * @param canvas Canvas id
 * @param col Pixel column
 * @param row Pixel row
 * @param color Zipped color of pixel
 */
native canvas_set_pixel( canvas, col, row, color );


/**
 * @param canvas Canvas id
 * @param &width Referenced variable will be filled with canvas width in pixels
 * @param &height Referenced variable will be filled with canvas height in pixels
 */
native canvas_get_size( canvas, &width, &height );

/**
 * @param canvas Canvas id
 * @param width Canvas width in pixels
 * @param height Canvas height in pixels
 */
native canvas_set_size( canvas, width, height );

/**
 * @param canvas Canvas id
 * @return Canvas width in pixels
 */
native canvas_get_width( canvas );

/**
 * @param canvas Canvas id
 * @param width New width in pixels
 */
native canvas_set_width( canvas, width );

/**
 * @param canvas Canvas id
 * @return Canvas height in pixels
 */
native canvas_get_height( canvas );

/**
 * @param canvas Canvas id
 * @param height New height in pixels
 */
native canvas_set_height( canvas, height );
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
