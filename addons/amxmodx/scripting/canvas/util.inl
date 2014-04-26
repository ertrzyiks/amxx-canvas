#include <amxmodx>
#include <amxmisc>
#include <fakemeta>


stock make_line
( 
	Float:fStartX, Float:fStartY, Float:fStartZ, 
	Float:fEndX, Float:fEndY, Float:fEndZ 
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
	write_short( gSprite ); // sprite index
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
