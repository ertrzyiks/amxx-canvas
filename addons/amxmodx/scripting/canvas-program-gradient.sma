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

#define PLUGIN "Canvas Program - Gradient"
#define VERSION "1.0.0"
#define AUTHOR "R3X"

new giStartColors[CANVAS_MAX_INSTANCES][2];
new giDiffColors[CANVAS_MAX_INSTANCES][2];

public plugin_init() 
{
	register_plugin( PLUGIN, VERSION, AUTHOR );
	
	register_canvas_program( "Gradient", "onDrawGradient" );
}

public onDrawGradient( canvas )
{
	new width, height;
	canvas_get_size( canvas, width, height );
	
	
	if ( giStartColors[canvas][0] == giStartColors[canvas][1])
	{
		giStartColors[canvas][0] = 0;
		giStartColors[canvas][1] = 255;
		giDiffColors[canvas][0] = 1;
		giDiffColors[canvas][1] = -1;
	}
	
	giStartColors[canvas][0] += giDiffColors[canvas][0];
	giStartColors[canvas][1] += giDiffColors[canvas][1];
	
	if ( giStartColors[canvas][0] < 0 )
	{
		giDiffColors[canvas][0] *= -1;
		giStartColors[canvas][0] = 0;
	}
	
	if ( giStartColors[canvas][1] < 0 )
	{
		giDiffColors[canvas][1] *= -1;
		giStartColors[canvas][1] = 0;
	}
	
	
	if ( giStartColors[canvas][0] > 255 )
	{
		giDiffColors[canvas][0] *= -1;
		giStartColors[canvas][0] = 255;
	}
	
	if ( giStartColors[canvas][1] > 255 )
	{
		giDiffColors[canvas][1] *= -1;
		giStartColors[canvas][1] = 255;
	}
	
	new r, b;
	
	static iColors[CANVAS_MAX_PIXELS];
	for ( new i = 0; i < width ; i++ )
	{
		r = floatround(giStartColors[canvas][0] * float(i) / width );
		b = floatround(giStartColors[canvas][1] * float(i) / width );
		
		for ( new j = 0; j < height; j++ )
		{
			iColors[ j * width + i ] = zipColor( r, 125, b );
		}
	}
	
	canvas_set_pixels( canvas, iColors );
}