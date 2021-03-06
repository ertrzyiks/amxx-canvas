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
#include <amxmodx>
#include <amxmisc>
#include <canvas>

#define PLUGIN "Canvas Program - Touch demo"
#define VERSION "1.0.0"
#define AUTHOR "R3X"

new game;

public plugin_init() 
{
	register_plugin( PLUGIN, VERSION, AUTHOR );
	
	game = register_canvas_program( "Touch demo", "onDraw" );
	register_program_event( game, "interaction:touch", "onTouch" );
}

public onDraw( canvas )
{
	new width, height;
	canvas_get_size( canvas, width, height );
	
	static iColors[CANVAS_MAX_PIXELS];
	canvas_get_pixels( canvas, iColors, sizeof iColors );
	
	new index, r, g, b;
	
	new Float:t = 0.99;
	for ( new i = 0; i < width ; i++ )
	{
		for ( new j = 0; j < height; j++ )
		{
			index = j * width + i;

			unzipColor( iColors[ index ], r, g, b );
			
			if ( r != 0 || g != 0 || b != 0 )
			{
				r = max( floatround(r * t, floatround_floor), 0 );
				g = max( floatround(g * t, floatround_floor), 0 );
				b = max( floatround(b * t, floatround_floor), 0 );
				iColors[ index ] = zipColor( r, g, b );
			}
		}
	}
	
	canvas_set_pixels( canvas, iColors );
}

public onTouch( canvas, const data[], len )
{
	new col = data[1], row = data[2];
	new hoverColor = zipColor( 255, 125, 0 );
	
	canvas_set_pixel( canvas, col, row, hoverColor );
}
