#include <amxmodx>
#include <amxmisc>
#include <canvas>
#include <fakemeta>
#include <xs>

new Float:gfViewTransition = 1.0;
new giCameraLocks[33] = { -1, ... };
new bool:gbCameraUnLock[33];
new giCameraEnt[33];
new Float:gfStartOrigin[33][3];
new Float:gfStartAngle[33][3];
new Float:gfStartTime[33];
new Float:gfChangeOrigin[33][3];
new Float:gfChangeAngle[33][3];

new ghAddToFullPack;


/**
 * Check if user has camera locked
 *
 * @param id Player id
 * @return True if camera is locked, false otherwise
 */
bool:hasCameraLock( id )
{
	return giCameraLocks[id] != -1;
}

/**
 * Force user camera to look at canvas.
 *
 * @param id Player id
 * @param [canvas] Canvas id. If -1, then take canvas of current interaction.
 * @return True on successful lock, false on failure.
 */
bool:setCameraLock( id, canvas = -1 )
{	
	if ( canvas == -1 )
	{
		canvas = giInteractionCanvas[ id ];
		
		if ( canvas == -1 )
		{
			return false;
		}
	}
	
	
	if ( giCameraLocks[ id ] == canvas )
	{
		return false;
	}
	
	if ( getLocksCount() == 0 )
	{
		
		ghAddToFullPack = register_forward( FM_AddToFullPack, "fwAddToFullPack" );
	}
	
	giCameraLocks[ id ] = canvas;
	
	new Float:vfViewOffset[3];
	new Float:vfOrigin[3], Float:vfAngle[3];
	new Float:vfEndOrigin[3], Float:vfEndAngle[3];
	pev( id, pev_view_ofs, vfViewOffset);
	pev( id, pev_origin, vfOrigin );
	xs_vec_add( vfOrigin, vfViewOffset, vfOrigin );
	
	pev( id, pev_v_angle, vfAngle );

	
	//Get final origin
	new Float:vfDirection[3];
	vfEndOrigin[0] = Float:gCanvas[canvas][originX];
	vfEndOrigin[1] = Float:gCanvas[canvas][originY];
	vfEndOrigin[2] = Float:gCanvas[canvas][originZ];
	
	vfDirection[0] = Float:gCanvas[canvas][directionX];
	vfDirection[1] = Float:gCanvas[canvas][directionY];
	vfDirection[2] = Float:gCanvas[canvas][directionZ];
	
	angle_vector( vfDirection, ANGLEVECTOR_FORWARD, vfDirection );
	vfDirection[0] = -vfDirection[0];
	vfDirection[1] = -vfDirection[1];
	vfDirection[2] = -vfDirection[2];
	
	xs_vec_mul_scalar( vfDirection, 150.0, vfDirection );
	xs_vec_add( vfEndOrigin, vfDirection, vfEndOrigin );
	
	
	
	//Get final angle
	vfDirection[0] = -vfDirection[0];
	//vfDirection[1] = -vfDirection[1];
	vfDirection[2] = -vfDirection[2];
	
	vector_to_angle( vfDirection, vfDirection );
	
	vfEndAngle[0] = -vfDirection[0];
	vfEndAngle[1] = -vfDirection[1];
	vfEndAngle[2] = -vfDirection[2];

	gbCameraUnLock[ id ] = false;
	tweenCamera( id, vfOrigin, vfAngle, vfEndOrigin, vfEndAngle );
	return true;
}

/**
 * Remove camera lock, switch back FPS camera.
 *
 * @param id Player id 
 */
bool:releaseCameraLock( id )
{
	if ( !giCameraEnt[ id ] )
	{
		return false;
	}
	
	if ( !is_user_alive( id ) )
	{
		_releaseCameraLock( id );
		return true;
	}
	
	new Float:vfViewOffset[3];
	new Float:vfOrigin[3], Float:vfAngle[3];
	pev( id, pev_view_ofs, vfViewOffset);
	pev( id, pev_origin, vfOrigin );
	xs_vec_add( vfOrigin, vfViewOffset, vfOrigin );
	
	pev( id, pev_v_angle, vfAngle );
	
	
	new Float:vfCurrentOrigin[3], Float:vfCurrentAngle[3];
	pev( giCameraEnt[ id ], pev_origin, vfCurrentOrigin );
	pev( giCameraEnt[ id ], pev_angles, vfCurrentAngle );
	
	gbCameraUnLock[ id ] = true;
	tweenCamera( id, vfCurrentOrigin, vfCurrentAngle, vfOrigin, vfAngle );
	return true;
}

_releaseCameraLock( id )
{
	if ( is_user_alive( id ) )
	{
		engfunc( EngFunc_SetView, id, id );
	}
	
	if ( giCameraEnt[id] )
	{
		set_pev( giCameraEnt[id], pev_flags, pev( giCameraEnt[id], pev_flags) | FL_KILLME );
		giCameraEnt[id] = 0;
	}
	
	giCameraLocks[ id ] = -1;
	giInteractionCanvas[ id ] = -1;
	
	set_pev( id, pev_flags, pev(id, pev_flags) & ~FL_FROZEN );
	
	if ( getLocksCount() == 0 )
	{
		unregister_forward( FM_AddToFullPack, ghAddToFullPack );
	}
}

tweenCamera( 
	id, 
	const Float:vfFromOrigin[3], const Float:vfFromAngle[3], 
	const Float:vfToOrigin[3], const Float:vfToAngle[3] 
)
{
	xs_vec_copy( vfFromOrigin, gfStartOrigin[id] );
	xs_vec_copy( vfFromAngle, gfStartAngle[id] );
	
	xs_vec_sub( vfToOrigin, vfFromOrigin, gfChangeOrigin[id] );
	
	xs_vec_sub( vfToAngle, vfFromAngle, gfChangeAngle[id] );
	gfChangeAngle[id][0] = getShortestAngle ( gfChangeAngle[id][0] );
	gfChangeAngle[id][1] = getShortestAngle ( gfChangeAngle[id][1] );
	gfChangeAngle[id][2] = getShortestAngle ( gfChangeAngle[id][2] );
	
	gfStartTime[id] = get_gametime();

	if ( !giCameraEnt[id] )
	{
		giCameraEnt[id] = createCameraEntity( id );
	}
	set_pev( giCameraEnt[id], pev_nextthink, get_gametime() );
	
}

Float:getShortestAngle( Float:fAngle )
{
	if ( fAngle > 180.0 )
	{
		return 360.0 - fAngle;
	}
	
	if ( fAngle < -180.0 )
	{
		return 360.0 + fAngle;
	}
	
	return fAngle;
}


getLocksCount()
{
	new count = 0;
	for ( new id = 1; id <= 32; id++ )
	{
		if ( giCameraLocks[id] != -1 )
		{
			count++;
		}
	}
	
	return count;
}

createCameraEntity( id )
{
	new ent = engfunc( EngFunc_CreateNamedEntity, info_target );
	set_pev( ent, pev_classname, "canvas_camera" );
	engfunc( EngFunc_SetModel, ent, gszPixelModel );
	
	set_pev( ent, pev_solid, SOLID_TRIGGER );
	set_pev( ent, pev_movetype, MOVETYPE_FLY );
	set_pev( ent, pev_owner, id );
	
	set_pev( ent, pev_rendermode, kRenderTransTexture );
	set_pev( ent, pev_renderamt, 0.0 );
	
	engfunc( EngFunc_SetView, id, ent );
	return ent;
}

/**
 *  
 *
 * @param ent Entity id
 */
public fwThinkCamera( ent )
{
	static szClassname[32];
	pev( ent, pev_classname, szClassname, sizeof szClassname - 1 );
	
	if( !equal( szClassname, "canvas_camera" ) )
	    return FMRES_IGNORED;
	    
	static id;
	id = pev( ent, pev_owner );
	
	if( !is_user_alive( id ) )
	{
		releaseCameraLock( id );
		return FMRES_IGNORED;
	}
	    
	
	static Float:timediff;
	timediff = get_gametime() - gfStartTime[id];
	
	static Float:fOrigin[3], Float:fAngle[3];
	fOrigin[ 0 ] = easing( timediff, gfStartOrigin[ id ][ 0 ], gfChangeOrigin[ id ][ 0 ], gfViewTransition );
	fOrigin[ 1 ] = easing( timediff, gfStartOrigin[ id ][ 1 ], gfChangeOrigin[ id ][ 1 ], gfViewTransition );
	fOrigin[ 2 ] = easing( timediff, gfStartOrigin[ id ][ 2 ], gfChangeOrigin[ id ][ 2 ], gfViewTransition );
	
	fAngle[ 0 ] = easing( timediff, gfStartAngle[ id ][ 0 ], gfChangeAngle[ id ][ 0 ], gfViewTransition );
	fAngle[ 1 ] = easing( timediff, gfStartAngle[ id ][ 1 ], gfChangeAngle[ id ][ 1 ], gfViewTransition );
	fAngle[ 2 ] = easing( timediff, gfStartAngle[ id ][ 2 ], gfChangeAngle[ id ][ 2 ], gfViewTransition );
	
	engfunc( EngFunc_SetOrigin, ent, fOrigin );
	set_pev( ent, pev_angles, fAngle );
	
	if ( timediff <= gfViewTransition )
	{
		set_pev( ent, pev_nextthink, get_gametime() );
	}
	else if ( gbCameraUnLock[ id ] )
	{
		_releaseCameraLock( id );
	}
	
	return FMRES_HANDLED;
}

public fwAddToFullPack( es_handle, e, ENT, HOST, hostflags, player, set )
{
	if ( ENT == HOST )
	{
		//set_es( es_handle, ES_RenderMode, kRenderTransAlpha );
		//set_es( es_handle, ES_RenderAmt, 0.0 );
		set_es( es_handle, ES_Effects, get_es( es_handle, ES_Effects)|EF_NODRAW );
		return FMRES_OVERRIDE;
	}
	
	return FMRES_IGNORED;
}

/**
 * @param t Current time
 * @param b Start value
 * @param c Change in value
 * @param d Duration
 */
Float:easing( Float:t, Float:b, Float:c, Float:d )
{
	t = floatmin( t, d );
	t /= d;
	return c*t*t*t*t + b;
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
