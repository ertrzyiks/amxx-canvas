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
 * @param color {Int} Compressed color
 * @param &r {Int} Value of red channel
 * @param &g {Int} Value of green channel
 * @param &b {Int} Value of blue channel
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
 * @param r {Int} Value of red channel
 * @param g {Int} Value of green channel
 * @param b {Int} Value of blue channel
 * @return {Int} Compressed color
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
 * @param color {Int} Compressed color
 * @param newR {Int} New value of red channel
 * @return {Int} Compressed color with modified red channel
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
 * @param color {Int} Compressed color
 * @param newG {Int} New value of green channel
 * @return {Int} Compressed color with modified green channel
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
 * @param color {Int} Compressed color
 * @param newB {Int} New value of blue channel
 * @return {Int} Compressed color with modified blue channel
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
 * @param szName {String} Name of initialize
 * @param szCallacbk {String} Name of public function to call
 * @param iTempo {Int} Count of pixels per tick
 */
native register_canvas_initializer( const szName[], const szCallback[], iTempo = 1 );


/**
 * Add new program. Callback is function called on each frame for each canvas which use this program.
 * 
 * @param szName {String} Name of program
 * @param szCallback {String} Name of public function to call
 */
native register_canvas_program( const szName[], const szCallback[] );


/**
 * Get colors of all pixels of canvas.
 * 
 * @param canvas {Int} Canvas id
 * @param pixels {Int[]} Array to be filled by colors
 * @param size {Int} Length of array, for safe copying
 * @return {Bool} True on success, false otherwise
 */
native bool:canvas_get_pixels( canvas, pixels[], size = CANVAS_MAX_PIXELS );

/**
 * Set colors of all pixesl of canvas
 *
 * @param canvas {Int} Canvas id
 * @param pixels {Int[]} Array of colors to apply into canvas
 * @param size {Int} Length of array, for safe copying
 * @return {Bool} True on success, false otherwise
 */
native bool:canvas_set_pixels( canvas, pixels[], size = CANVAS_MAX_PIXELS );


/**
 * @param {Int} canvas Canvas id
 * @param {Int} &width Referenced variable will be filled with canvas width in pixels
 * @param {Int} &height Referenced variable will be filled with canvas height in pixels
 */
native canvas_get_size( canvas, &width, &height );

/**
 * @param {Int} canvas Canvas id
 * @param {Int} width Canvas width in pixels
 * @param {Int} height Canvas height in pixels
 */
native canvas_set_size( canvas, width, height );

/**
 * @param {Int} canvas Canvas id
 * @return {Int} Canvas width in pixels
 */
native canvas_get_width( canvas );

/**
 * @param {Int} canvas Canvas id
 * @param {Int} width New width in pixels
 */
native canvas_set_width( canvas, width );

/**
 * @param {Int} canvas Canvas id
 * @return Canvas height in pixels
 */
native canvas_get_height( canvas );

/**
 * @param {Int} canvas Canvas id
 * @param {Int} height New height in pixels
 */
native canvas_set_height( canvas, height );
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
