#include <amxmodx>
#include <amxmisc>
#include <fakemeta>
#include <xs>

#define DEFAULT_DISTANCE 200.0
#define DEFAULT_ANGLE 75.0

new giInteractionCanvas[32] = { -1, ... };


checkForInteraction( canvas )
{
	new program = getProgram( canvas );
	new Trie:events = ArrayGetCell( gProgramEvents, program );
	
	if ( !TrieKeyExists( events, "interaction:enter" ) && !TrieKeyExists( events, "interaction:leave" ) )
	{
		return;
	}
	
	for ( new id = 1; id <= giMaxPlayers; id++ )
	{
		if ( !is_user_alive( id ) )
		{
			continue;
		}
		
		if ( isInInteractionArea( id, canvas ) )
		{
			if ( giInteractionCanvas[id] == -1 )
			{
				giInteractionCanvas[id] = canvas;
				
				new data[1];
				data[0] = id;
				triggerProgramEvent( canvas, program, "interaction:enter", data, 1 );
			}
		}
		else
		{
			if ( giInteractionCanvas[id] == canvas )
			{
				giInteractionCanvas[id] = -1;
				
				new data[1];
				data[0] = id;
				triggerProgramEvent( canvas, program, "interaction:leave", data, 1 );
			}
		}
	}
}

bool:isInInteractionArea( id, canvas, Float:fMaxDistance = DEFAULT_DISTANCE, Float:fMaxAngle = DEFAULT_ANGLE )
{
	new Float:vfCanvasOrigin[3];
	vfCanvasOrigin[0] = Float:gCanvas[canvas][originX];
	vfCanvasOrigin[1] = Float:gCanvas[canvas][originY];
	vfCanvasOrigin[2] = Float:gCanvas[canvas][originZ];
	
	new Float:vfOrigin[3];
	pev( id, pev_origin, vfOrigin );
	
	//Check if player is close enough
	if ( vector_distance( vfCanvasOrigin, vfOrigin ) > fMaxDistance )
	{
		return false;
	}
	
	//Check if player looks at canvas
	new Float:vfAngle[3];
	pev( id, pev_v_angle, vfAngle );
	xs_vec_mul_scalar( vfAngle, -1.0, vfAngle );
	angle_vector( vfAngle, ANGLEVECTOR_RIGHT, vfAngle );
	
	vfCanvasOrigin[0] = Float:gCanvas[canvas][rightX];
	vfCanvasOrigin[1] = Float:gCanvas[canvas][rightY];
	vfCanvasOrigin[2] = Float:gCanvas[canvas][rightZ];
	
	new Float:fAngle = xs_vec_angle( vfAngle, vfCanvasOrigin );
	return fAngle <= fMaxAngle;
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/