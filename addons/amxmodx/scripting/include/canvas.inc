#if defined _canvas_included
  #endinput
#endif
#define _canvas_included

#define CANVAS_MAX_PIXELS 256
#define CANVAS_MAX_INIT_NAME 32


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
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
