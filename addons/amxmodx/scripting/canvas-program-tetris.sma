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

const BOARD_WIDTH = 10;
const BOARD_HEIGHT = 22;


enum BlockType{
	BlockS,
	BlockZ,
	
	BlockL,
	BlockJ,
	
	BlockI,
	BlockO,
	BlockT
};

new giBlockDefs[BlockType][4][4] = {
	{
		{ 1, 5, 6, 10 },
		{ 5, 6, 8, 9 },
		{ 1, 5, 6, 10 },
		{ 5, 6, 8, 9 }
	},
	{
		{ 1, 4, 5, 8 },
		{ 4, 5, 9, 10 },
		{ 1, 4, 5, 8 },
		{ 4, 5, 9, 10 }
	},
	{
		{ 0, 1, 5, 9 },
		{ 2, 4, 5, 6 },
		{ 1, 5, 9, 10 },
		{ 4, 5, 6, 8 }
	},
	{
		{ 1, 2, 5, 9 },
		{ 4, 5, 6, 10 },
		{ 1, 5, 8, 9 },
		{ 0, 4, 5, 6 }
	},
	{
		{ 1, 5, 9, 13 },
		{ 4, 5, 6, 7 },
		{ 1, 5, 9, 13 },
		{ 4, 5, 6, 7 }
	},
	{
		{ 4, 5, 8, 9 },
		{ 4, 5, 8, 9 },
		{ 4, 5, 8, 9 },
		{ 4, 5, 8, 9 }
	},
	{
		{ 4, 5, 6, 9 },
		{ 1, 4, 5, 9 },
		{ 1, 4, 5, 6 },
		{ 1, 5, 6, 9 }
	}
};

new gColors[BlockType];


new tetris;

new giCanvasPlayers[CANVAS_MAX_INSTANCES];

public plugin_init() 
{
	register_plugin(PLUGIN, VERSION, AUTHOR)
	
	tetris = register_canvas_program( "Tetris", "onDraw", BOARD_WIDTH, BOARD_HEIGHT );

	register_program_event( tetris, "interaction:enter", "onEnter" );
	register_program_event( tetris, "interaction:leave", "onLeave" );
	register_program_event( tetris, "interaction:keydown", "onKeyDown" );
	register_program_event( tetris, "interaction:keypress", "onKeyPress" );
	
	register_forward( FM_CmdStart, "fwCmdStart", 1 );
}

public plugin_cfg()
{
	gColors[BlockS] = zipColor( 80, 200, 80 );
	gColors[BlockZ] = zipColor( 12, 200, 12 );
	
	gColors[BlockL] = zipColor( 200, 80, 80 );
	gColors[BlockJ] = zipColor( 200, 12, 12 );
	
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
		
		initGame();
	}
}

public taskShowMessage( id )
{
	set_hudmessage(42, 255, 0, -1.0, 0.2, 0, 6.0, 1.0)
	show_hudmessage(id, "Press E to play Tetris!")
}


//GAMEPLAY
new Float:gfPreGameTime = 0.0;
new Float:gfPreGameRotTime = 0.0;
new giPreGameRotation = 0;
new giPreBrickPos[2] = { 3, -2 };
new BlockType:gPreCurrentType = BlockS;

renderPreGame( canvas, Float:delta )
{
	gfPreGameTime += delta;
	gfPreGameRotTime += delta;
	
	if ( gfPreGameTime > 0.3 )
	{
		gfPreGameTime -= 0.3;
		
		giPreBrickPos[1]++;
		
		if ( giPreBrickPos[1] > BOARD_HEIGHT )
		{
			giPreBrickPos[1] = -1;
			gPreCurrentType = BlockType:random( _:BlockType );
			gfPreGameRotTime = 0.0;
		}
	}
	
	if ( gfPreGameRotTime > 1.0 )
	{
		gfPreGameRotTime -= 1.0;
		giPreGameRotation = (giPreGameRotation + 1) % 4;
	}
	
	for ( new i = 0; i < BOARD_WIDTH ; i++ )
	{		
		for ( new j = 0; j < BOARD_HEIGHT; j++ )
		{
			canvas_set_pixel( canvas, i, j, zipColor( 70, 70, 255  - j * 7) );
		}
	}
	

	drawBlock( canvas, giPreBrickPos[0], giPreBrickPos[1], gPreCurrentType, giPreGameRotation );
}

new Float:gfGameTime = 0.0;
new giGameRotation = 0;
new giBrickPos[2] = { 3, -2 };
new giBoard[BOARD_WIDTH][BOARD_HEIGHT];
new BlockType:gCurrentType = BlockS;
new Float:gfFallTime = 0.5;

new Float:gfNextSpeedUp = 0.0;

new bool:gbShouldCollapse = false;
new Float:gfCollapseTime = 0.0;

new gviFullLines[BOARD_HEIGHT];

renderGame( canvas, Float:delta )
{
	collapseEmptyLines( delta );
	checkForSpeedUp( delta );
	
	gfGameTime += delta;
	
	if ( gfGameTime > gfFallTime )
	{
		gfGameTime -= gfFallTime;
		
		if ( !moveDown() )
		{
			leaveBlock();
			sendNewBlock();
		}
	}
	
	for ( new i = 0; i < BOARD_WIDTH ; i++ )
	{		
		for ( new j = 0; j < BOARD_HEIGHT; j++ )
		{
			new piece = giBoard[i][j], color;
			if ( piece == -1 )
			{
				color = zipColor( 255  - j * 2, 255  - j * 2, 255  - j * 2 );
			}
			else
			{
				color = gColors[ BlockType:piece ];
			}
			
			canvas_set_pixel( canvas, i, j, color );
		}
	}
	
	drawBlock( canvas, giBrickPos[0], giBrickPos[1], gCurrentType, giGameRotation );
}

initGame()
{
	gfGameTime = -1.0;
	
	clearBoard();
	sendNewBlock();
	
	giBrickPos[0] = 3;
}

clearBoard()
{
	for ( new i = 0; i < BOARD_WIDTH ; i++ )
	{		
		for ( new j = 0; j < BOARD_HEIGHT; j++ )
		{
			giBoard[i][j] = -1;
		}
	}
}

leaveBlock()
{
	new x = giBrickPos[0];
	new y = giBrickPos[1];
	
	for( new i = 0; i <4; i++ )
	{
		new pos = giBlockDefs[gCurrentType][ giGameRotation % 4 ][i];
		new col = x + pos % 4;
		new row = y + pos / 4;
		
		if ( col < 0 || col >= BOARD_WIDTH || row < 0 || row >= BOARD_HEIGHT )
		{
			onGameOver();
			return;
		}
		
		giBoard[col][row] = _:gCurrentType;
	}
	
	checkFullLines();
}

checkFullLines()
{
	for ( new j = 0; j < BOARD_HEIGHT; j++ )
	{
		gviFullLines[j] = 1;
		for ( new i = 0; i < BOARD_WIDTH; i++ )
		{
			if ( giBoard[i][j] == -1 )
			{
				gviFullLines[j] = 0;
			}
		}
	}
	
	new bool:anyFull = false;
	
	for ( new i = 0; i < BOARD_HEIGHT; i++ )
	{
		if ( gviFullLines[i] )
		{
			removeFullLine( i );
			anyFull = true;
		}
	}
	
	
	if ( anyFull )
	{
		gfGameTime -= gfFallTime * 4;
		gfCollapseTime = 0.1;
		gbShouldCollapse = true;
	}
	
}

removeFullLine( line )
{
	for ( new i = 0; i < BOARD_WIDTH; i++ )
	{
		giBoard[i][line] = -1;
	}
}

collapseEmptyLines( Float:delta )
{
	if ( !gbShouldCollapse )
	{
		return;
	}
	
	gfCollapseTime -= delta;
	
	if ( gfCollapseTime > 0 )
	{
		return;
	}

	for ( new j = 0; j < BOARD_HEIGHT; j++ )
	{
		if ( gviFullLines[j] )
		{
			collapseEmptyLine( j );
		}
	}
	
	gbShouldCollapse = false;
}

collapseEmptyLine( line )
{
	if ( line <= 0)
	{
		return;
	}
	
	for ( new i = 0; i < BOARD_WIDTH; i++ )
	{
		for ( new j = 0; j < line; j++ )
		{
			if ( line - i >= 1 )
			{
				giBoard[i][line - j] = giBoard[i][line - j - 1];
			}
		}
	}
}

checkForSpeedUp( Float:delta )
{
	gfNextSpeedUp += delta;
	
	if ( gfNextSpeedUp > 12.0 )
	{
		gfNextSpeedUp -= 12.0;
		
		gfFallTime = floatmax( gfFallTime - 0.02, 0.1 );
	}
}

getOffsetByBlock( BlockType:type )
{
	new biggestBlockId = giBlockDefs[type][0][3];
	
	return 3 - biggestBlockId / 4;
}

sendNewBlock()
{
	gCurrentType = BlockType:random( _:BlockType );
	
	giBrickPos[0] = 3;
	giBrickPos[1] = getOffsetByBlock( gCurrentType ) - 4;
}

drawBlock( canvas, x, y, BlockType:type, rotation = 0 )
{
	for( new i = 0; i <4; i++ )
	{
		new pos = giBlockDefs[type][ rotation % 4 ][i];
		new col = pos % 4;
		new row = pos / 4;
	
		canvas_set_pixel( canvas, x + col, y + row, gColors[type] );
	}
}

onGameOver()
{
	for ( new i = 0; i < CANVAS_MAX_INSTANCES; i++ )
	{
		new id = giCanvasPlayers[ i ];
		giCanvasPlayers[ i ] = 0;
		
		if ( is_user_alive(id) )
		{
			canvas_unlock_user_camera( id );
		}
	}
}

new Float:gfLastHorizontalPress = 0.0;
new Float:gfLastVerticalPress = 0.0;
new Float:gfLastRotationPress = 0.0;
public onKeyPress( canvas, const data[], length )
{
	new id = data[0];
	
	if ( giCanvasPlayers[canvas] != id )
	{
		return;
	}
	
	
	new key = data[1];
	
	new Float:fNow = get_gametime();
	

	if ( key == IN_MOVELEFT && canPress( fNow, gfLastHorizontalPress ) )
	{
		if ( moveLeft() )
		{
			gfLastHorizontalPress = fNow;
		}
	}
		
	else if ( key == IN_MOVERIGHT && canPress( fNow, gfLastHorizontalPress ) )
	{
		if ( moveRight() )
		{
			gfLastHorizontalPress = fNow;
		}
	}
	

	if ( key == IN_FORWARD && canPress( fNow, gfLastRotationPress, 0.2 ) )
	{
		if ( rotate( ) )
		{
			gfLastRotationPress = fNow;
		}
	}

	
	if ( key == IN_BACK && canPress( fNow, gfLastVerticalPress, 0.02 ) )
	{
		if ( moveDown() )
		{
			gfLastVerticalPress = fNow;
		}
	}
}

bool:canPress( Float:fNow, Float:fLastUse, Float:fBound = 0.08 ) 
{
	return ( fNow - fLastUse ) > fBound;
}
	

bool:moveLeft( )
{
	giBrickPos[0]--;
	if ( !isValidMove() )
	{
		giBrickPos[0]++;
		return false;
	}
	
	return true;
}

bool:moveRight()
{
	giBrickPos[0]++;
			
	if ( !isValidMove() )
	{
		giBrickPos[0]--;
		return false;
	}
	
	return true;
}

bool:rotate()
{
	giGameRotation++;
			
	if ( !isValidMove() )
	{
		giGameRotation--;
		return false;
	}
	
	return true;
}

bool:moveDown()
{
	giBrickPos[1]++;
			
	if ( !isValidMove() )
	{
		giBrickPos[1]--;
		return false;
	}
	
	return true;
}


bool:isValidMove()
{
	new x = giBrickPos[0];
	new y = giBrickPos[1];
	
	for( new i = 0; i <4; i++ )
	{
		new pos = giBlockDefs[gCurrentType][ giGameRotation % 4 ][i];
		new col = x + pos % 4;
		new row = y + pos / 4;
	
		if ( col < 0 || col >= BOARD_WIDTH )
		{
			return false;
		}
		
		if ( row >= BOARD_HEIGHT )
		{
			return false;
		}
		
		if ( row >= 0 && giBoard[col][row] != -1 )
		{
			return false;
		}
	}
	
	return true;
}


