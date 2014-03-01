#include <amxmodx>
#include <amxmisc>
#include <canvas>
#include <fakemeta>
#include <xs>

new Float:gfViewTransition = 1.0;
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
	
	
		
	if ( getLocksCount() == 0 )
	{
		
		ghAddToFullPack = register_forward( FM_AddToFullPack, "fwAddToFullPack" );
	}
	
	giCameraLocks[ id ] = canvas;
	
	new Float:vfViewOffset[3];
	new Float:vfOrigin[3], Float:vfAngle[3];
	new Float:vfEndOrigin[3], Float:vfEndAngle[3];

	if ( giCameraLocks[ id ] == -1 || !pev_valid( giCameraEnt[ id ] ) )
	{
		pev( id, pev_view_ofs, vfViewOffset);
		pev( id, pev_origin, vfOrigin );
		xs_vec_add( vfOrigin, vfViewOffset, vfOrigin );
		
		pev( id, pev_v_angle, vfAngle );
	}
	else
	{
		pev( giCameraEnt[ id ], pev_origin, vfOrigin );
		pev( giCameraEnt[ id ], pev_angles, vfAngle );
	}
	
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
	
	new canvasScale= getScale( canvas );
	
	xs_vec_mul_scalar( vfDirection, 22.0 * float(canvasScale), vfDirection );
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
	if ( giCameraLocks[ id ] == -1 )
	{
		return;
	}
	
	if ( is_user_alive( id ) )
	{
		_screenFade( id );
		engfunc( EngFunc_SetView, id, id );
	}
	
	if ( giCameraEnt[id] )
	{
		set_pev( giCameraEnt[id], pev_flags, pev( giCameraEnt[id], pev_flags) | FL_KILLME );
		giCameraEnt[id] = 0;
	}
	
	new canvas = giCameraLocks[ id ];
	
	giCameraLocks[ id ] = -1;
	giInteractionCanvas[ id ] = -1;
	
	if ( getLocksCount() == 0 )
	{
		unregister_forward( FM_AddToFullPack, ghAddToFullPack );
	}
	
	new data[1];
	data[0] = id;
	triggerProgramEvent( canvas, gCanvas[ canvas ][ programId ], "interaction:quit", data, 1);
}

_screenFade( id )
{
	static gmsgScreenFade;
	if( !gmsgScreenFade )
	{
		gmsgScreenFade = get_user_msgid("ScreenFade");
	}
	
	message_begin(MSG_ONE_UNRELIABLE, gmsgScreenFade, _, id );
	write_short( 1<<4 );
	write_short( 1<<8 );
	write_short( 0 );
	write_byte (0);
	write_byte (0);
	write_byte (0);
	write_byte ( 255 );
	message_end();
}

tweenCamera( 
	id, 
	const Float:vfFromOrigin[3], const Float:vfFromAngle[3], 
	const Float:vfToOrigin[3], const Float:vfToAngle[3],
	bool:update = false
)
{
	xs_vec_copy( vfFromOrigin, gfStartOrigin[id] );
	xs_vec_copy( vfFromAngle, gfStartAngle[id] );
	
	xs_vec_sub( vfToOrigin, vfFromOrigin, gfChangeOrigin[id] );
	
	xs_vec_sub( vfToAngle, vfFromAngle, gfChangeAngle[id] );
	gfChangeAngle[id][0] = getShortestAngle ( gfChangeAngle[id][0] );
	gfChangeAngle[id][1] = getShortestAngle ( gfChangeAngle[id][1] );
	gfChangeAngle[id][2] = getShortestAngle ( gfChangeAngle[id][2] );
	
	if ( !update )
	{
		gfStartTime[id] = get_gametime();
	}

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
		_releaseCameraLock( id );
		return FMRES_IGNORED;
	}
	
	//Update end position on camera unlock
	if ( gbCameraUnLock[ id ] )
	{
		new Float:vfViewOffset[3];
		new Float:vfEndOrigin[3], Float:vfEndAngle[3];
		pev( id, pev_view_ofs, vfViewOffset);
		pev( id, pev_origin, vfEndOrigin );
		xs_vec_add( vfEndOrigin, vfViewOffset, vfEndOrigin );
		
		pev( id, pev_v_angle, vfEndAngle );
		tweenCamera( id, gfStartOrigin[ id ], gfStartAngle[ id ], vfEndOrigin, vfEndAngle, true );
	}
	    
	
	static Float:timediff;
	timediff = floatmin( get_gametime() - gfStartTime[id], gfViewTransition );
	
	static Float:vfOrigin[3], Float:vfAngle[3];
	
	
	if ( timediff < gfViewTransition )
	{
		easingArrayByName( 
			"easingInQuartic", 
			timediff, gfStartOrigin[ id ], gfChangeOrigin[ id ], gfViewTransition, 
			vfOrigin 
		);
		easingArrayByName( 
			gbCameraUnLock[ id ] ? "easingInQuartic" : "easingInOutQuartic", 
			timediff, gfStartAngle[ id ], gfChangeAngle[ id ], gfViewTransition, 
			vfAngle 
		);
	
		engfunc( EngFunc_SetOrigin, ent, vfOrigin );
		set_pev( ent, pev_angles, vfAngle );
		
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
	if ( ENT == HOST && giCameraLocks[HOST] != -1 )
	{
		set_es( es_handle, ES_Effects, get_es( es_handle, ES_Effects)|EF_NODRAW );
		return FMRES_OVERRIDE;
	}
	
	return FMRES_IGNORED;
}

public fwPlayerSpawn( id )
{
	if ( giCameraLocks[id] != -1 )
	{
		_releaseCameraLock( id );
	}
}
stock Float:easingArrayByFuncId( funcId, Float:t, const Float:b[3], const Float:c[3], Float:d, Float:output[3] )
{
	output[0] = easingByFuncId( funcId, t, b[0], c[0], d );
	output[1] = easingByFuncId( funcId, t, b[1], c[1], d );
	output[2] = easingByFuncId( funcId, t, b[2], c[2], d );
}

stock Float:easingArrayByName( const szFunction[], Float:t, const Float:b[3], const Float:c[3], Float:d, Float:output[3] )
{
	new funcId = get_func_id( szFunction );
	output[0] = easingByFuncId( funcId, t, b[0], c[0], d );
	output[1] = easingByFuncId( funcId, t, b[1], c[1], d );
	output[2] = easingByFuncId( funcId, t, b[2], c[2], d );
}

stock Float:easingByName( const szFunction[], Float:t, Float:b, Float:c, Float:d )
{
	new funcId = get_func_id( szFunction );
	return easingByFuncId( t, b, c, d );
}

stock Float:easingByFuncId( funcId, Float:t, Float:b, Float:c, Float:d )
{
	callfunc_begin_i( funcId );
	callfunc_push_float( t );
	callfunc_push_float( b );
	callfunc_push_float( c );
	callfunc_push_float( d );
	return Float:callfunc_end();
}


/**
 * @param t Current time
 * @param b Start value
 * @param c Change in value
 * @param d Duration
 */
public Float:easingLinear( Float:t, Float:b, Float:c, Float:d )
{
	return c*t/d + b;
}


/**
 * @param t Current time
 * @param b Start value
 * @param c Change in value
 * @param d Duration
 */
public Float:easingInQuartic( Float:t, Float:b, Float:c, Float:d )
{
	t = floatmin( t, d );
	t /= d;
	return c*t*t*t*t + b;
}

/**
 * @param t Current time
 * @param b Start value
 * @param c Change in value
 * @param d Duration
 */
public Float:easingOutQuartic( Float:t, Float:b, Float:c, Float:d )
{
	t /= d;
	t = t - 1.0;
	return -c * (t*t*t*t - 1) + b;
}

/**
 * @param t Current time
 * @param b Start value
 * @param c Change in value
 * @param d Duration
 */
public Float:easingInOutQuartic( Float:t, Float:b, Float:c, Float:d )
{
	t /= d/2;
	if (t < 1) return c/2*t*t*t*t + b;
	t -= 2;
	return -c/2 * (t*t*t*t - 2) + b;
}

/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
