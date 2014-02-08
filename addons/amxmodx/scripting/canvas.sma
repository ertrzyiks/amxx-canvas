#include <amxmodx>
#include <amxmisc>
#include <fakemeta>

#define PLUGIN "Canvas"
#define VERSION "0.0.1"
#define AUTHOR "R3X"

new bool:gbTraceHooks[32];
new giTraceHooksActive = 0;
new ghTraceLine;

new gPixel;

new const gszPixelModel[] = "sprites/pixelw.spr";

public plugin_init ()
{
	register_plugin( PLUGIN, VERSION, AUTHOR );
	
	register_clcmd( "amx_canvas", "cmdCanvas", ADMIN_CFG );
}

public plugin_precache ()
{
	gPixel = precache_model( gszPixelModel );
}

public cmdCanvas ( id, level, cid )
{
	if( !cmd_access( id, level, cid, 1 ) )
		return PLUGIN_HANDLED;
		
	createCanvasByAim( id );
	return PLUGIN_HANDLED;
}

public fwTraceLine( const Float:fStart[3], const Float:fEnd[3], conditions, id, tr_handle )
{
	static Float:fOrigin[3], Float:fVec[3];
	
	if ( is_user_connected( id ) && gbTraceHooks[ id ] )
	{
		gbTraceHooks[ id ] = false;
		giTraceHooksActive = giTraceHooksActive - 1;
		
		get_tr2( tr_handle, TR_vecEndPos, fOrigin );
		get_tr2( tr_handle, TR_vecPlaneNormal, fVec );
		
		createCanvas( fOrigin, fVec );
		
		if ( giTraceHooksActive == 0 )
		{
			unregister_forward( FM_TraceLine, ghTraceLine );
		}
	}
	
	return FMRES_IGNORED;
}

createCanvasByAim ( id )
{
	if ( gbTraceHooks[ id ] )
	{
		return;
	}
	
	gbTraceHooks[ id ] = true;
	giTraceHooksActive = giTraceHooksActive + 1;
	
	if ( giTraceHooksActive == 1 )
	{	
		ghTraceLine = register_forward( FM_TraceLine, "fwTraceLine", 1 );
	}
}

createCanvas ( const Float:fOrigin[3], const Float:fVec[3], width = 100, height = 100 )
{
	new Float:fAngle[3];
	fAngle[0] = -fVec[0]
	fAngle[1] = -fVec[1]
	fAngle[2] = fVec[2]
	vector_to_angle( fAngle, fAngle );
	
	new Float:fMyOrigin[3];
	fMyOrigin[0] = fOrigin[0] + fVec[0];
	fMyOrigin[1] = fOrigin[1] + fVec[1];
	fMyOrigin[2] = fOrigin[2] + fVec[2];
	
	new Float:fColor[3];
	fColor[0] = 1.0;
	fColor[1] = 1.0;
	fColor[2] = 1.0;
	
	new ent = engfunc( EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "info_target") );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_classname, "pixel" );
	set_pev( ent, pev_movetype, MOVETYPE_NONE );
	set_pev( ent, pev_solid, SOLID_NOT );
	engfunc( EngFunc_SetModel, ent, gszPixelModel );
	set_pev( ent, pev_origin, fMyOrigin );
	set_pev( ent, pev_angles, fAngle );
	set_pev( ent, pev_scale, 0.125 );
	set_pev( ent, pev_rendercolor, fColor );
	set_pev( ent, pev_renderfx, 0 );
	set_pev( ent, pev_rendermode, kRenderNormal );
	set_pev( ent, pev_renderamt, 255 );
	
	new iEntMax = global_get(glb_maxEntities)
	server_print("entities in world (%d max!)", iEntMax)
}
