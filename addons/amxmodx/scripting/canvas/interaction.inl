#include <amxmodx>
#include <amxmisc>
#include <fakemeta>
#include <xs>

#define DEFAULT_DISTANCE 200.0
#define DEFAULT_ANGLE 75.0


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
	vfAngle[2] = -vfAngle[2];
	angle_vector( vfAngle, ANGLEVECTOR_RIGHT, vfAngle );
	
	vfCanvasOrigin[0] = Float:gCanvas[canvas][rightX];
	vfCanvasOrigin[1] = Float:gCanvas[canvas][rightY];
	vfCanvasOrigin[2] = Float:gCanvas[canvas][rightZ];
	
	new Float:fAngle = xs_vec_angle( vfAngle, vfCanvasOrigin );
	return fAngle <= fMaxAngle;
}


/**
 * 
 */
public fwCmdStart( id, uc_handle )
{
	static iDetectKeys[] = {
		IN_FORWARD,
		IN_BACK,
		IN_MOVELEFT,
		IN_MOVERIGHT,
		
		IN_ATTACK,
		IN_ATTACK2,
		
		IN_USE
	};
	
	if ( !hasCameraLock( id ) || gbCameraUnLock[ id ] )
	{
		return FMRES_IGNORED;
	}
	
	if ( !is_user_alive( id ) )
	{
		releaseCameraLock( id );
		return FMRES_HANDLED;
	}
	
	new button = get_uc( uc_handle, UC_Buttons );
	new oldbutton = pev( id, pev_oldbuttons );
	for ( new i = 0; i < sizeof( iDetectKeys ); i++ )
	{
		new key = iDetectKeys[i];
		
		if ( button & key )
		{
			if ( (oldbutton & key) == 0 )
			{
				onKeyDown( id, key );
			}
			
			onKeyPress( id, key );
		}
		else if ( oldbutton & key )
		{
			onKeyUp( id, key );
		}
	}

	return FMRES_HANDLED;
}

onKeyDown( id, key )
{
	new canvas = giCameraLocks[ id ];
	new program = gCanvas[ canvas ][ programId ];
	
	new data[2];
	data[0] = id;
	data[1] = key;
	triggerProgramEvent( canvas, program, "interaction:keydown", data, sizeof( data ) );
}

onKeyUp( id, key )
{
	new canvas = giCameraLocks[ id ];
	new program = gCanvas[ canvas ][ programId ];
	
	new data[2];
	data[0] = id;
	data[1] = key;
	triggerProgramEvent( canvas, program, "interaction:keyup", data, sizeof( data ) );
}

onKeyPress( id, key )
{
	new canvas = giCameraLocks[ id ];
	new program = gCanvas[ canvas ][ programId ];
	
	new data[2];
	data[0] = id;
	data[1] = key;
	triggerProgramEvent( canvas, program, "interaction:keypress", data, sizeof( data ) );
}


bool:getHoverPoint( canvas, const Float:vfStart[3], const Float:vfEnd[3], &col, &row )
{	
	/**
	 * Calculate canvas plane
	 */
	 //s = ( x1, x2, x3 )
	new Float:x1 = Float:gCanvas[canvas][zerozeroX], 
		Float:y1 = Float:gCanvas[canvas][zerozeroY], 
		Float:z1 = Float:gCanvas[canvas][zerozeroZ];
		
	new Float:s[3];
	s[0] = x1;
	s[1] = y1;
	s[2] = z1;
	
	//N = ( A, B, C )
	new Float:N[3];
	N[0] = Float:gCanvas[canvas][directionX];
	N[1] = Float:gCanvas[canvas][directionY];
	N[2] = - Float:gCanvas[canvas][directionZ];
	
	angle_vector( N, ANGLEVECTOR_FORWARD, N );
	
	new Float:A = N[0], Float:B = N[1], Float:C = N[2];
	
	/**
	 * Ax + By + Cz + D = 0
	 * D = -Ax - By - Cz
	 */
	new Float:D = -A * x1 - B * y1 - C * z1;
	
	new Float:vfCrossPoint[3];
	_calculateCrossPoint( vfStart, vfEnd, A, B, C, D, vfCrossPoint );
	
	new Float:fDistance = get_distance_f( vfCrossPoint, s );
	
	/**
	 * Calulate axis and radius vectors
	 */
	//r
	new Float:r[3];
	xs_vec_sub( vfCrossPoint, s, r );
	xs_vec_normalize( r, r );
	
	//axis
	new Float:d[3];
	d[0] = Float:gCanvas[canvas][downX];
	d[1] = Float:gCanvas[canvas][downY];
	d[2] = Float:gCanvas[canvas][downZ];
	
	/**
	 * Calulate canvas x,y
	 */
	new Float:dot = xs_vec_dot( r, d );
	new Float:cross[3];
	xs_vec_cross( r, d, cross )
	new Float:det = xs_vec_dot( N, cross );
	
	new Float:fAngle = floatatan2( det, dot, degrees);
	
	new Float:x = fDistance * floatsin( fAngle, degrees);
	new Float:y = fDistance * floatcos( fAngle, degrees );
	
	return _getPixelByCoords( canvas, x, y, col, row );
}

/**
 * Calculate cross point of plane Ax + By + Cz + D = 0 and line create from point vfStart to vfEnd.
 */
_calculateCrossPoint
( 
	const Float:vfStart[3], const Float:vfEnd[3], 
	Float:A, Float:B, Float:C, Float:D, 
	Float:vfCrossPoint[3]
)
{
	//p1 = ( xp1, yp1, zp1 )
	new Float:xp1 = vfStart[0], Float:yp1 = vfStart[1], Float:zp1 = vfStart[2];
	
	//p2 = ( xp2, yp2, zp2 )
	new Float:xp2 = vfEnd[0], Float:yp2 = vfEnd[1], Float:zp2 = vfEnd[2];
	
	//x = ax * t + bx
	//y = ay * t + by
	//z = az * t + bz
	new Float:ax = xp2 - xp1;
	new Float:bx = xp1;
	
	new Float:ay = yp2 - yp1;
	new Float:by = yp1;
	
	new Float:az = zp2 - zp1;
	new Float:bz = zp1;
	
	/*
	 * Calulate cross point
	 */
	//t = ...
	new Float:t = ( -D - A * bx - B * by - C * bz ) / ( A * ax + B * ay + C * az );
	
	vfCrossPoint[0] = ax * t + bx;
	vfCrossPoint[1] = ay * t + by;
	vfCrossPoint[2] = az * t + bz;
}

bool:_getPixelByCoords( canvas, Float:x, Float:y, &col, &row )
{
	new pixelsize = gCanvas[canvas][scale];
	col = floatround( x / pixelsize, floatround_floor);
	row = floatround( y / pixelsize, floatround_floor);
	return true;
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
