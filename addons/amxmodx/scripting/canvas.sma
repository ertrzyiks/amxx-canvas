#include <amxmodx>
#include <amxmisc>
#include <fakemeta>
#include <engine>
#include <xs>

#define PLUGIN "Canvas"
#define VERSION "0.0.1"
#define AUTHOR "R3X"

new bool:gbTraceHooks[32];
new giTraceHooksActive = 0;
new ghTraceLine;

new gPixel;

new Array:gCanvas;
new Array:gCanvasPixels;

new const gszPixelModel[] = "sprites/pixel.spr";

public plugin_init ()
{
	register_plugin( PLUGIN, VERSION, AUTHOR );
	
	register_clcmd( "amx_canvas", "cmdCanvas", ADMIN_CFG );
	register_clcmd( "ents", "cmdEntities", ADMIN_CFG );
	
	register_forward( FM_StartFrame, "fwStartFrame", 1 );
	
	gCanvas = ArrayCreate();
	gCanvasPixels = ArrayCreate();
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

public cmdEntities( id )
{
	client_print( id, print_chat, "Entities %d/%d", engfunc(EngFunc_NumberOfEntities), global_get(glb_maxEntities) );
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
		
		createCanvas( fOrigin, fVec, 12, 12 );
		
		if ( giTraceHooksActive == 0 )
		{
			unregister_forward( FM_TraceLine, ghTraceLine );
		}
	}
	
	return FMRES_IGNORED;
}


public fwStartFrame()
{
	creatingTick();
}

new creatingLastIndex = -1;
new creatingIndex = 0;
new Float:gfBaseOrigin[3], Float:gfAngle[3], Float:gfRight[3], Float:gfDown[3];
new giWidth, giHeight;

creatingTick()
{
	if ( creatingLastIndex == -1 )
	{
		return;
	}
	
	if ( creatingIndex >= creatingLastIndex )
	{
		creatingLastIndex = -1;
		return;
	}
	
	new row = creatingIndex / giWidth;
	new col = creatingIndex % giWidth;
	
	new Float:fMyOrigin[3], Float:fMyDown[3], Float:fMyRight[3];
	
	xs_vec_mul_scalar( gfDown, float(row), fMyDown );
	xs_vec_mul_scalar( gfRight, float(col), fMyRight );
	
	xs_vec_copy( gfBaseOrigin, fMyOrigin);
	xs_vec_add( fMyOrigin, fMyDown, fMyOrigin );
	xs_vec_add( fMyOrigin, fMyRight, fMyOrigin );
			
	createPixel( fMyOrigin, gfAngle );
	
	creatingIndex++;
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

createCanvas ( const Float:fOrigin[3], const Float:fVec[3], width = 256, height = 240 )
{
	new Float:fAngle[3];
	fAngle[0] = -fVec[0]
	fAngle[1] = -fVec[1]
	fAngle[2] = fVec[2]
	vector_to_angle( fAngle, fAngle );
	
	new Float:fBaseOrigin[3];
	fBaseOrigin[0] = fOrigin[0] + fVec[0];
	fBaseOrigin[1] = fOrigin[1] + fVec[1];
	fBaseOrigin[2] = fOrigin[2] + fVec[2];
	
	new Float:fUp[3], Float:fDown[3], Float:fRight[3];
	angle_vector( fAngle, ANGLEVECTOR_UP, fUp );
	xs_vec_mul_scalar( fUp, -1.0, fDown );
	
	angle_vector( fAngle, ANGLEVECTOR_RIGHT, fRight );
	
	
	creatingIndex = 0;
	creatingLastIndex = width * height;
	xs_vec_copy( fBaseOrigin , gfBaseOrigin );
	xs_vec_copy( fAngle, gfAngle );
	xs_vec_copy( fRight, gfRight );
	xs_vec_copy( fDown, gfDown );
	
	giWidth = width;
	giHeight = height;
}

createPixel( const Float: fOrigin[3], Float:fAngle[3] )
{
	new ent = engfunc( EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "info_target") );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_classname, "pixel" );
	set_pev( ent, pev_movetype, MOVETYPE_NONE );
	set_pev( ent, pev_solid, SOLID_NOT );
	engfunc( EngFunc_SetModel, ent, gszPixelModel );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_angles, fAngle );
	set_pev( ent, pev_scale, 0.125 );
	set_pev( ent, pev_rendercolor, Float:{ 1.0, 1.0, 1.0 } );
	set_pev( ent, pev_renderfx, 0 );
	set_pev( ent, pev_rendermode, kRenderNormal );
	set_pev( ent, pev_renderamt, 255 );
	
	return ent;
}
