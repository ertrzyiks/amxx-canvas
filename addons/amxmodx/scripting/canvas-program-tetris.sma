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
#include <fakemeta>
#include <canvas>

#define PLUGIN "Canvas Program - Tetris"
#define VERSION "0.0.1"
#define AUTHOR "R3X"

const BLOCKDEF_WIDTH = 4;
const BLOCKDEF_HEIGHT = 4;
const BLOCKDEF_LEN = BLOCKDEF_WIDTH * BLOCKDEF_HEIGHT;


enum BlockType{
	BlockS,
	BlockZ,
	
	BlockL,
	BlockOL,
	
	BlockI,
	BlockO,
	BlockT
};

new giBlockS[][BLOCKDEF_LEN] = {
	{
		0, 1, 0, 0,
		0, 1, 1, 0,
		0, 0, 1, 0,
		0, 0, 0, 0
	},
	{
		0, 0, 0, 0,
		0, 1, 1, 0,
		1, 1, 0, 0,
		0, 0, 0, 0
	}
};

new giBlockZ[][BLOCKDEF_LEN] = {
	{
		0, 1, 0, 0,
		1, 1, 0, 0,
		1, 0, 0, 0,
		0, 0, 0, 0
	},
	{
		0, 0, 0, 0,
		1, 1, 0, 0,
		0, 1, 1, 0,
		0, 0, 0, 0
	}
};

new giBlockL[][BLOCKDEF_LEN] = {
	{
		1, 1, 0, 0,
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 0
	},
	{
		0, 0, 1, 0,
		1, 1, 1, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	},
	{
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 1, 1, 0,
		0, 0, 0, 0
	},
	{
		0, 0, 0, 0,
		1, 1, 1, 0,
		1, 0, 0, 0,
		0, 0, 0, 0
	}
};

new giBlockOL[][BLOCKDEF_LEN] = {
	{
		0, 1, 1, 0,
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 0
	},
	{
		0, 0, 0, 0,
		1, 1, 1, 0,
		0, 0, 1, 0,
		0, 0, 0, 0
	},
	{
		0, 1, 0, 0,
		0, 1, 0, 0,
		1, 1, 0, 0,
		0, 0, 0, 0
	},
	{
		1, 0, 0, 0,
		1, 1, 1, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	}
};

new giBlockI[][BLOCKDEF_LEN] = {
	{
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 1, 0, 0
	},
	{
		0, 0, 0, 0,
		1, 1, 1, 1,
		0, 0, 0, 0,
		0, 0, 0, 0
	}
};

new giBlockO[][BLOCKDEF_LEN] = {
	{
		0, 0, 0, 0,
		1, 1, 0, 0,
		1, 1, 0, 0,
		0, 0, 0, 0
	}
};

new giBlockT[][BLOCKDEF_LEN] = {
	{
		0, 0, 0, 0,
		1, 1, 1, 0,
		0, 1, 0, 0,
		0, 0, 0, 0
	},
	{
		0, 1, 0, 0,
		1, 1, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 0
	},
	{
		0, 1, 0, 0,
		1, 1, 1, 0,
		0, 0, 0, 0,
		0, 0, 0, 0
	},
	{
		0, 1, 0, 0,
		0, 1, 1, 0,
		0, 1, 0, 0,
		0, 0, 0, 0
	}
};

new gColors[BlockType];


new tetris;

new giCanvasPlayers[CANVAS_MAX_INSTANCES];

public plugin_init() 
{
	register_plugin(PLUGIN, VERSION, AUTHOR)
	
	tetris = register_canvas_program( "Tetris", "onDraw", 10, 22 );
	
	register_program_event( tetris, "interaction:enter", "onEnter" );
	register_program_event( tetris, "interaction:leave", "onLeave" );
	register_program_event( tetris, "interaction:keydown", "onKeyDown" );
	
	register_forward( FM_CmdStart, "fwCmdStart", 1 );
}

public plugin_cfg()
{
	gColors[BlockS] = zipColor( 80, 200, 80 );
	gColors[BlockZ] = zipColor( 12, 200, 12 );
	
	gColors[BlockL] = zipColor( 200, 80, 80 );
	gColors[BlockOL] = zipColor( 200, 12, 12 );
	
	gColors[BlockI] = zipColor( 255, 0, 0);
	gColors[BlockO] = zipColor( 12, 12, 200 );
	gColors[BlockT] = zipColor( 200, 200, 12 );
}

new Float:gfLastUse[33];

public fwCmdStart( id, uc_handle )
{
	if ( is_user_alive( id ) )
	{
		if (  get_uc( uc_handle, UC_Buttons ) & IN_USE )
		{
			new Float:now = get_gametime();
			new Float:diff = now - gfLastUse[id];

			if ( diff > 1.0 )
			{
				gfLastUse[id] = now;
				onUse( id );
			}
		}
	}
}


public onDraw( canvas, Float:delta )
{
	if ( giCanvasPlayers[canvas] )
	{
		renderGame( canvas, delta );
	}
	else
	{
		renderPreGame( canvas, delta );
	}
}

public onEnter( canvas, data[], length )
{
	if ( giCanvasPlayers[canvas] )
	{
		return;
	}
	
	
	new id = data[0];
	set_task( 1.0, "taskShowMessage", id, _, _, "b" );
}

public onLeave( canvas, data[], length )
{
	new id = data[0];
	if ( task_exists( id ) )
	{
		remove_task( id );
	}
}

public onKeyDown( canvas, data[], length )
{
	new id = data[0];
	new key = data[1];
	
	if ( key == IN_USE )
	{
		canvas_unlock_user_camera( id );
		giCanvasPlayers[ canvas ] = 0;
	}
}

onUse( id )
{
	if ( task_exists( id ) )
	{
		remove_task( id );
		new canvas = canvas_lock_user_camera( id );
		giCanvasPlayers[ canvas ] = id;
	}
}

public taskShowMessage( id )
{
	set_hudmessage(42, 255, 0, -1.0, 0.2, 0, 6.0, 1.0)
	show_hudmessage(id, "Press E to play Tetris!")
}


//GAMEPLAY
new Float:gfPreGameTime = 0.0;
new giBrickPos[2] = { 5, -2 };
new BlockType:gCurrentType = BlockS;

renderPreGame( canvas, Float:delta )
{
	gfPreGameTime += delta;
	
	new width, height;
	canvas_get_size( canvas, width, height );
	
	if ( gfPreGameTime > 0.3 )
	{
		gfPreGameTime -= 0.3;
		
		giBrickPos[1]++;
		
		if ( giBrickPos[1] > height )
		{
			giBrickPos[1] = -1;
			gCurrentType = BlockType:random( _:BlockType );
		}
	}
	
	for ( new i = 0; i < width ; i++ )
	{		
		for ( new j = 0; j < height; j++ )
		{
			canvas_set_pixel( canvas, i, j, zipColor( 70, 70, 255  - j * 7) );
		}
	}
	

	drawBlock( canvas, giBrickPos[0], giBrickPos[1], gCurrentType );
}

drawBlock( canvas, x, y, BlockType:type, rotation = 0 )
{
	new block[BLOCKDEF_LEN];
	block = getBlockArray( type, rotation );
	
	new refx = 2, refy = 2;
	
	for( new i = 0; i < sizeof(block); i++ )
	{
		new col = i % BLOCKDEF_WIDTH - refx;
		new row = i / BLOCKDEF_WIDTH - refy;
		
		if ( block[i] )
		{
			canvas_set_pixel( canvas, x + col, y + row, gColors[type] );
		}
	}
}

getBlockArray( BlockType:type, rotation )
{
	switch( type )
	{
		case BlockS: {
			return giBlockS[ rotation % sizeof(giBlockS) ];
		}
		
		case BlockZ: {
			return giBlockZ[ rotation % sizeof(giBlockZ) ];
		}
		
		case BlockL: {
			return giBlockL[ rotation % sizeof(giBlockL) ];
		}
		
		case BlockOL: {
			return giBlockOL[ rotation % sizeof(giBlockOL) ];
		}
		
		case BlockI: {
			return giBlockI[ rotation % sizeof(giBlockI) ];
		}
		
		case BlockO: {
			return giBlockO[ rotation % sizeof(giBlockO) ];
		}
		
		case BlockT: {
			return giBlockT[ rotation % sizeof(giBlockT) ];
		}
		
		default:
		{
			log_amx( "Invalid block type %d given", type );
		}
	}
	
	return giBlockS[ rotation % sizeof(giBlockS) ];
}

renderGame( canvas, Float:delta )
{
	gfPreGameTime += delta;
	
	new width, height;
	canvas_get_size( canvas, width, height );
	
	if ( gfPreGameTime > 0.5 )
	{
		gfPreGameTime -= 0.5;
		
		giBrickPos[1]++;
		
		if ( giBrickPos[1] > height )
		{
			giBrickPos[1] = -1;
		}
	}
	
	for ( new i = 0; i < width ; i++ )
	{		
		for ( new j = 0; j < height; j++ )
		{
			canvas_set_pixel( canvas, i, j, zipColor( 255  - j * 7, 70, 70 ) );
		}
	}
}

