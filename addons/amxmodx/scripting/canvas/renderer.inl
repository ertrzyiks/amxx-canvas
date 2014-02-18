#include <amxmodx>
#include <amxmisc>
#include <fakemeta>

enum Canvas
{
	cols,
	rows,
	
	scale,
	ready,
	
	init_tick,
	init_maxtick,
	
	programId,
	
	originX,
	originY,
	originZ,
	
	directionX,
	directionY,
	directionZ,
	
	rightX,
	rightY,
	rightZ,
	
	downX,
	downY,
	downZ
};

enum CanvasInitializer
{
	callback,
	tempo
};

new bool:gbTraceHooks[32];
new giTraceHooksActive = 0;
new ghTraceLine;

new Float:gCanvasStart[CANVAS_MAX_INSTANCES];
new gCanvas[CANVAS_MAX_INSTANCES][Canvas];
new gCanvasPixels[CANVAS_MAX_INSTANCES][CANVAS_MAX_PIXELS];
new giCanvasIndex = 0;

new gCanvasInitializers[CANVAS_MAX_INITIALIZER][CanvasInitializer];
new gCanvasInitializerNames[CANVAS_MAX_INITIALIZER][CANVAS_MAX_INIT_NAME];
new giCanvasInitializeIndex = 0;

new Array:gPrograms;

public fwStartFrame()
{
	for ( new i = 0; i < giCanvasInitializeIndex; i++ )
	{
		new isReady = gCanvas[i][ready];
		
		if ( isReady )
		{
			new cb = ArrayGetCell( gPrograms, gCanvas[i][programId] ), ret;
			
			if ( cb <= 0)
			{
				handleDefaultProgram( i );
			}
			else
			{
				ExecuteForward( cb , ret, i );
			}
		}
		else
		{
			creatingTick( i );
		}
	}
}

onCanvasReady( canvas )
{
	gCanvasStart[canvas] = get_gametime();
}

/**
 * Pixels of canvas can not be created instantly.
 * Each frame we got creation tick and decide which pixels should be created.
 *
 * Controller of this process is called Canvas Initializer.
 */
creatingTick( canvas )
{
	new tick = gCanvas[canvas][init_tick];
	new maxtick = gCanvas[canvas][init_maxtick];
	
	if ( tick >= maxtick )
	{
		gCanvas[canvas][ready] = 1;
		onCanvasReady( canvas );
		return;
	}
	
	new initCallback = gCanvasInitializers[0][callback];
	new initTempo = gCanvasInitializers[0][tempo];
	new pixelIndex;
	new i;
	
	for ( i = 0; i < initTempo; i++ )
	{
		if ( tick + i >= maxtick )
		{
			break;
		}
		
		if ( !ExecuteForward( initCallback, pixelIndex, canvas, tick + i ) )
		{
			pixelIndex = tick + i;
		}
	
		creatingTickByPixel( canvas, pixelIndex );
	}
	
	gCanvas[canvas][init_tick] = tick + i;
}

creatingTickByPixel( canvas, pixelIndex )
{		
	new canvasData[Canvas];
	canvasData = gCanvas[canvas];
	
	new Float:fBaseOrigin[3], Float:fDown[3], Float:fRight[3], Float:fAngle[3], pixelSize;
	fBaseOrigin[0] = Float:canvasData[originX];
	fBaseOrigin[1] = Float:canvasData[originY];
	fBaseOrigin[2] = Float:canvasData[originZ];
	
	fDown[0] = Float:canvasData[downX];
	fDown[1] = Float:canvasData[downY];
	fDown[2] = Float:canvasData[downZ];
	
	fRight[0] = Float:canvasData[rightX];
	fRight[1] = Float:canvasData[rightY];
	fRight[2] = Float:canvasData[rightZ];
	
	fAngle[0] = Float:canvasData[directionX];
	fAngle[1] = Float:canvasData[directionY];
	fAngle[2] = Float:canvasData[directionZ];
	
	pixelSize = canvasData[scale];
	
	new width = canvasData[cols];
	new height= canvasData[rows];
	new row = pixelIndex / width;
	new col = pixelIndex % width;
	
	new Float:fMyOrigin[3], Float:fMyDown[3], Float:fMyRight[3];
	
	xs_vec_mul_scalar( fDown, float( row - height / 2) * pixelSize, fMyDown );
	xs_vec_mul_scalar( fRight, float(col - width / 2) * pixelSize, fMyRight );
	
	xs_vec_copy( fBaseOrigin, fMyOrigin);
	xs_vec_add( fMyOrigin, fMyDown, fMyOrigin );
	xs_vec_add( fMyOrigin, fMyRight, fMyOrigin );
			
	gCanvasPixels[canvas][ pixelIndex ] = createPixel( fMyOrigin, fAngle, pixelSize );
}

createCanvas ( const Float:fOrigin[3], const Float:fVec[3], width = 28, height = 8, pixelsize = 8 )
{
	
	addCanvasMenuItem( "Canvas #%d", giCanvasIndex + 1 );
	
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
	
	gCanvas[giCanvasIndex][scale] = pixelsize;

	gCanvas[giCanvasIndex][originX] = _:fBaseOrigin[0];
	gCanvas[giCanvasIndex][originY] = _:fBaseOrigin[1];
	gCanvas[giCanvasIndex][originZ] = _:fBaseOrigin[2];
	
	gCanvas[giCanvasIndex][directionX] = _:fAngle[0];
	gCanvas[giCanvasIndex][directionY] = _:fAngle[1];
	gCanvas[giCanvasIndex][directionZ] = _:fAngle[2];
	
	gCanvas[giCanvasIndex][rightX] = _:fRight[0];
	gCanvas[giCanvasIndex][rightY] = _:fRight[1];
	gCanvas[giCanvasIndex][rightZ] = _:fRight[2];
	
	gCanvas[giCanvasIndex][downX] = _:fDown[0];
	gCanvas[giCanvasIndex][downY] = _:fDown[1];
	gCanvas[giCanvasIndex][downZ] = _:fDown[2];
	
	gCanvas[giCanvasIndex][programId] = 0; 
	
	setSize( giCanvasIndex, width, height );
	
	return giCanvasIndex++;
}

createPixel( const Float: fOrigin[3], Float:fAngle[3], pixelSize )
{	
	new ent = engfunc( EngFunc_CreateNamedEntity, engfunc(EngFunc_AllocString, "info_target") );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_classname, "pixel" );
	set_pev( ent, pev_movetype, MOVETYPE_NONE );
	set_pev( ent, pev_solid, SOLID_NOT );
	engfunc( EngFunc_SetModel, ent, gszPixelModel );
	set_pev( ent, pev_origin, fOrigin );
	set_pev( ent, pev_angles, fAngle );
	set_pev( ent, pev_scale,  0.125 * pixelSize );
	set_pev( ent, pev_rendercolor, Float:{ 1.0, 1.0, 1.0 } );
	set_pev( ent, pev_renderfx, 0 );
	set_pev( ent, pev_rendermode, kRenderNormal );
	set_pev( ent, pev_renderamt, 255 );
	
	return ent;
}

createProgram( const szName[], const szFunction[], plugin_id = -1 )
{
	new cb;
	
	if ( plugin_id == -1 )
	{
		cb = -1;
	}
	else
	{
		cb = CreateOneForward( plugin_id, szFunction, FP_CELL );
		
	}
	
	addProgramMenuItem( szName );
	ArrayPushCell( gPrograms, cb );
	return ArraySize( gPrograms ) - 1;
}


handleDefaultProgram( canvas )
{
	for ( new i = 0; i < CANVAS_MAX_PIXELS; i++ )
	{
		new ent = gCanvasPixels[canvas][i];
		
		if ( pev_valid( ent ) )
		{
			new Float:fColor[3];
			fColor[0] = random_float(1.0, 255.0); 
			fColor[1] = fColor[0];
			fColor[2] = fColor[0];	
			
			set_pev( ent, pev_rendercolor, fColor );
		}
	}
}

getProgram( canvas )
{
	return gCanvas[canvas][programId];
}

setProgram( canvas, program )
{
	gCanvas[canvas][programId] = program;
}

getSize( canvas, &width, &height )
{
	width = gCanvas[canvas][cols];
	height = gCanvas[canvas][rows];
}

setSize( canvas, width, height )
{
	gCanvas[canvas][cols] = width;
	gCanvas[canvas][rows] = height;
	
	gCanvas[canvas][init_tick] = 0; 
	gCanvas[canvas][ready] = 0; 
	gCanvas[canvas][init_maxtick] = width * height; 
	
	for ( new i = 0; i < CANVAS_MAX_PIXELS; i++ )
	{
		new ent = gCanvasPixels[canvas][i];
		
		if ( pev_valid( ent ) )
		{
			engfunc( EngFunc_RemoveEntity, ent );
		}
		
		gCanvasPixels[canvas][i] = 0;
	}
}

getScale( canvas )
{
	return gCanvas[canvas][scale];
}

setScale( canvas, newScale )
{
	gCanvas[canvas][scale] = newScale;
	setSize( canvas, gCanvas[canvas][cols], gCanvas[canvas][rows] );
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
