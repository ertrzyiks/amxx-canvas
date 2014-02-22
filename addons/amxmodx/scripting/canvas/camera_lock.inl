#include <amxmodx>
#include <amxmisc>
#include <canvas>
#include <fakemeta>
#include <xs>

new gfViewTransition = 2.0;
new giCameraLocks[33] = { -1, ... };
new giCameraEnt[33];
new Float:gfStartOrigin[33][3];
new Float:gfStartAngle[33][3];
new Float:gfStartTime[33];
new Float:gfEndOrigin[33][3];
new Float:gfEndAngle[33][3];

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
	
	giCameraLocks[ id ] = canvas;
	
	new Float:vfViewOffset[3];
	pev( id, pev_view_ofs, vfViewOffset);
	pev( id, pev_origin, gfStartOrigin[id] );
	xs_vec_add( gfStartOrigin[id], vfViewOffset, gfStartOrigin[id] );
	
	pev( id, pev_v_angle, gfStartAngle[id] );
	gfStartTime[id] = get_gametime();

	if ( !giCameraEnt[id] )
	{
		giCameraEnt[id] = createCameraEntity( id );
	}
	
	
	
	new Float:vfOrigin[3], Float:vfDirection[3];
	vfOrigin[0] = Float:gCanvas[canvas][originX];
	vfOrigin[1] = Float:gCanvas[canvas][originY];
	vfOrigin[2] = Float:gCanvas[canvas][originZ];
	
	vfDirection[0] = Float:gCanvas[canvas][directionX];
	vfDirection[1] = Float:gCanvas[canvas][directionY];
	vfDirection[2] = Float:gCanvas[canvas][directionZ];
	
	angle_vector( vfDirection, ANGLEVECTOR_FORWARD, vfDirection );
	vfDirection[0] = -vfDirection[0];
	vfDirection[1] = -vfDirection[1];
	
	xs_vec_mul_scalar( vfDirection, 400.0, vfDirection );
	xs_vec_add( vfOrigin, vfDirection, gfEndOrigin[id] );
	
	vfDirection[0] = -vfDirection[0];
	vfDirection[1] = -vfDirection[1];
	vfDirection[2] = -vfDirection[2];
	
	vector_to_angle( vfDirection, vfDirection );
	
	gfEndAngle[id][0] = -vfDirection[0];
	gfEndAngle[id][1] = -vfDirection[1];
	gfEndAngle[id][2] = -vfDirection[2];
	
	
	return true;
}

bool:releaseCameraLock( id )
{
	if ( giCameraEnt[id] )
	{
		engfunc( EngFunc_SetView, id, id );
		engfunc( EngFunc_RemoveEntity, giCameraEnt[id] );
		giCameraEnt[id] = 0;
		giCameraLocks[id] = -1;
		return true;
	}
	
	return false;
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
	set_pev( ent, pev_nextthink, get_gametime() );
	return ent;
}

public fwThinkCamera( ent )
{
	static szClassname[32];
	pev( ent, pev_classname, szClassname, sizeof szClassname - 1 );
	
	if( !equal( szClassname, "canvas_camera" ) )
	    return FMRES_IGNORED;
	    
	static id;
	id = pev( ent, pev_owner );
	
	if( !is_user_alive( id ) )
	    return FMRES_IGNORED;
	
	engfunc( EngFunc_SetOrigin, ent, gfStartOrigin[id]);
	set_pev( ent, pev_angles, gfStartAngle[id] );
	set_pev( ent, pev_nextthink, get_gametime() );
	
	return FMRES_HANDLED;
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
