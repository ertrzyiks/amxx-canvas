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

#define PLUGIN "Canvas Program - Tetris"
#define VERSION "0.0.1"
#define AUTHOR "R3X"

new tetris;

public plugin_init() 
{
	register_plugin(PLUGIN, VERSION, AUTHOR)
	
	tetris = register_canvas_program( "Tetris", "onDraw", 10, 22 );
	register_program_event( tetris, "init", "onInit" );
	register_program_event( tetris, "quit", "onQuit" );
	
	register_program_event( tetris, "interaction:enter", "onEnter" );
	register_program_event( tetris, "interaction:leave", "onLeave" );
	register_program_event( tetris, "interaction:keypress", "onKeyPress" );
}

public onDraw( canvas )
{
	
}

public onInit( canvas )
{
	client_print( 0, print_chat, "START TETRIS");
}

public onQuit( canvas )
{
	client_print( 0, print_chat, "END TETRIS");
}

public onEnter( canvas, data[], length )
{
	console_print(0, "LENG %d", length);
	new id = data[0];
	client_print( id, print_chat, "ENTERED");
}

public onLeave( canvas,  data[], length )
{
	new id = data[0];
	client_print( id, print_chat, "LEFT")
}

public onKeyPress( canvas, key )
{
	
}
