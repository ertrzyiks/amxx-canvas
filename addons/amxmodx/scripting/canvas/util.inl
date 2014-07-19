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

/**
 * Draw a line between two points. Usefull for debugging.
 *
 * @param fStartX Start position, x axis coord
 * @param fStartY Start position, y axis coord
 * @param fStartZ Start position, z axis coord
 * @param fEndX End position, x axis coord
 * @param fEndY End position, y axis coord
 * @param fEndZ End position, z axis coord
 * @param sprite Sprite index
 */
stock make_line
( 
	Float:fStartX, Float:fStartY, Float:fStartZ, 
	Float:fEndX, Float:fEndY, Float:fEndZ,
	sprite
)
{
	message_begin(MSG_BROADCAST ,SVC_TEMPENTITY ); //message begin
	write_byte(0);
	engfunc( EngFunc_WriteCoord, fStartX ); // start position
	engfunc( EngFunc_WriteCoord, fStartY );
	engfunc( EngFunc_WriteCoord, fStartZ );
	engfunc( EngFunc_WriteCoord, fEndX ); // end position
	engfunc( EngFunc_WriteCoord, fEndY );
	engfunc( EngFunc_WriteCoord, fEndZ );
	write_short( sprite ); // sprite index
	write_byte(3); // starting frame
	write_byte(0); // frame rate in 0.1's
	write_byte(100); // life in 0.1's
	write_byte(10); // line width in 0.1's
	write_byte(0); // noise amplitude in 0.01's
	write_byte(255);
	write_byte(0);
	write_byte(0);
	write_byte(55); // brightness
	write_byte(0); // scroll speed in 0.1's
	message_end();
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
