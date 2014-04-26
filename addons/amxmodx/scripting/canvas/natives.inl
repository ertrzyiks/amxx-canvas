#include <amxmodx>
#include <amxmisc>

public plugin_natives()
{
	register_library( "Canvas" );
	register_native( "register_canvas_initializer", "nativeRegisterCanvasInitializer" );
	register_native( "register_canvas_program", "nativeRegisterCanvasProgram" );
	register_native( "register_program_event", "nativeRegisterProgramEvent" );
	register_native( "unregister_program_event", "nativeUnregisterProgramEvent" );
	
	register_native( "canvas_lock_user_camera", "nativeCanvasLockUserCamera" );
	register_native( "canvas_unlock_user_camera", "nativeCanvasUnlockUserCamera" );
	
	register_native( "canvas_get_pixels", "nativeCanvasGetPixels" );
	register_native( "canvas_set_pixels", "nativeCanvasSetPixels" );
	
	register_native( "canvas_get_pixel", "nativeCanvasGetPixel" );
	register_native( "canvas_set_pixel", "nativeCanvasSetPixel" );
	
	register_native( "canvas_get_width", "nativeCanvasGetWidth" );
	register_native( "canvas_set_width", "nativeCanvasSetWidth" );
	
	register_native( "canvas_get_height", "nativeCanvasGetHeight" );
	register_native( "canvas_set_height", "nativeCanvasSetHeight" );
	
	register_native( "canvas_get_size", "nativeCanvasGetSize" );
	register_native( "canvas_set_size", "nativeCanvasSetSize" );
}


/**
 * Register canvas elements
 */
public nativeRegisterCanvasInitializer( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "register_canvas_initializer expects 3 arguments, %d given", argc );
		return -1;
	}
	
	get_string( 1, gCanvasInitializerNames[giCanvasInitializeIndex], CANVAS_MAX_INIT_NAME - 1 );

	new szCallback[32];
	get_string( 2, szCallback, 31 );
	
	gCanvasInitializers[giCanvasInitializeIndex][callback] = CreateOneForward( plugin, szCallback, FP_CELL, FP_CELL );
	gCanvasInitializers[giCanvasInitializeIndex][tempo] = get_param( 3 );
	
	return giCanvasInitializeIndex++;
}

public nativeRegisterCanvasProgram( plugin, argc )
{
	if ( argc < 4 )
	{
		log_error( AMX_ERR_PARAMS, "register_canvas_program expects 4 arguments, %d given", argc );
		return -1;
	}
	
	new szName[CANVAS_MAX_PROGRAM_NAME];
	get_string( 1, szName, CANVAS_MAX_PROGRAM_NAME - 1 );
	
	new szCallback[32];
	get_string( 2, szCallback, charsmax(szCallback) );
	
	new forceWidth = get_param( 3 );
	new forceHeight = get_param( 4 );
	
	return createProgram( szName, szCallback, forceWidth, forceHeight, plugin );
}

public nativeRegisterProgramEvent( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "register_program_event expects 3 arguments, %d given", argc );
		return -1;
	}
	
	new program = get_param( 1 );
	
	new szEvent[32];
	get_string( 2 , szEvent, charsmax(szEvent) );
	
	new szCallback[32];
	get_string( 3, szCallback, charsmax(szCallback) );
	
	new cb = CreateOneForward( plugin, szCallback, FP_CELL, FP_ARRAY, FP_CELL );
	
	new Trie:events = ArrayGetCell( gProgramEvents, program );
	new Array:cbs;
	
	if ( !TrieGetCell( events, szEvent, cbs ) )
	{
		cbs = ArrayCreate();
		TrieSetCell( events, szEvent, cbs );
	}
	
	ArrayPushCell( cbs, cb );
	return ArraySize( cbs ) - 1;
}

public nativeUnregisterProgramEvent( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "unregister_program_event expects 3 arguments, %d given", argc );
		return -1;
	}
	
	new program = get_param( 1 );
	
	new szEvent[32];
	get_string( 2 , szEvent, charsmax(szEvent) );
	
	new handler = get_param( 3 );
	
	new Trie:events = ArrayGetCell( gProgramEvents, program );
	new Array:cbs;
	
	if ( !TrieGetCell( events, szEvent, cbs ) )
	{
		return 0;
	}
	
	if ( handler < 0 && handler >= ArraySize( cbs ) )
	{
		return 0;
	}
	
	ArraySetCell( cbs, handler, -1 );
	return 1;
}

/**
 * Drawing util
 */
public bool:nativeCanvasGetPixels( plugin, argc )
{
	static iColors[CANVAS_MAX_PIXELS];
	
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_pixels expects 3 arguments, %d given", argc );
		return false;
	}
	
	new canvas = get_param( 1 );
	new size = get_param( 3 );
	new Float:fColor[3];
	
	for ( new i = 0; i < CANVAS_MAX_PIXELS; i++ )
	{
		new ent = gCanvasPixels[canvas][i];
		
		if ( pev_valid( ent ) )
		{
			pev( ent, pev_rendercolor, fColor );
			iColors[i] = zipColor( 
				floatround(fColor[0]),
				floatround(fColor[1]),
				floatround(fColor[2])
			);
		}
		else
		{
			iColors[i] = 0; 
		}
	}
		
	set_array( 2, iColors, size );
	return true;
}

public bool:nativeCanvasSetPixels( plugin, argc )
{
	static iColors[CANVAS_MAX_PIXELS];
	
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_set_pixels expects 3 arguments, %d given", argc );
		return false;
	}
	
	new canvas = get_param( 1 );
	new size = min( get_param( 3 ), CANVAS_MAX_PIXELS );
	new iColor[3], Float:fColor[3];
	
	get_array( 2, iColors, size );
	
	for ( new i = 0; i < size; i++ )
	{
		new ent = gCanvasPixels[canvas][i];
		
		if ( pev_valid( ent ) )
		{
			unzipColor( iColors[i], iColor[0], iColor[1], iColor[2] );
			IVecFVec( iColor, fColor );
			set_pev( ent, pev_rendercolor, fColor );
		}
	}
	
	return true;
}

public nativeCanvasGetPixel( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_pixel expects 3 arguments, %d given", argc );
		return -1;
	}
	
	new canvas = get_param( 1 );
	new col = get_param( 2 );
	new row = get_param( 3 );
	new width = gCanvas[canvas][cols];
	new height = gCanvas[canvas][rows];
	new ent = gCanvasPixels[canvas][ row * width + col ];
	
	if ( col < 0 || col >= width || row < 0 || row >= height )
	{
		return -1;
	}
	
	new iColor[3], Float:fColor[3];
	pev( ent, pev_rendercolor, fColor );
	FVecIVec( fColor, iColor );
	return zipColor( iColor[0], iColor[1], iColor[2] );
}

public nativeCanvasSetPixel( plugin, argc )
{
	if ( argc < 4 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_set_pixel expects 4 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );
	new col = get_param( 2 );
	new row = get_param( 3 );
	new color = get_param( 4 );
	new width = gCanvas[canvas][cols];
	new height = gCanvas[canvas][rows];
	
	if ( col < 0 || col >= width || row < 0 || row >= height )
	{
		return;
	}
	
	new ent = gCanvasPixels[canvas][ row * width + col ];
	
	new iColor[3], Float:fColor[3];
	unzipColor( color, iColor[0], iColor[1], iColor[2] );
	IVecFVec( iColor, fColor );
	set_pev( ent, pev_rendercolor, fColor );
}

/**
 * Game util
 */
public nativeCanvasLockUserCamera( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_lock_user_camera expects 2 arguments, %d given", argc );
		return -1;
	}
	
	new id = get_param( 1 );
	new canvas = get_param( 2 );
	
	if ( setCameraLock( id, canvas ) )
	{
		return giInteractionCanvas[id];
	}
	
	return -1;
}

public nativeCanvasUnlockUserCamera( plugin, argc )
{
	if ( argc < 1 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_unlock_user_camera expects 1 arguments, %d given", argc );
		return -1;
	}
	
	new id = get_param( 1 );
	
	if ( releaseCameraLock( id ) )
	{
		return giInteractionCanvas[id];
	}
	
	return -1;
}



/**
 * Getters/setters
 */
public nativeCanvasGetWidth( plugin, argc )
{
	if ( argc < 1 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_width expects 1 arguments, %d given", argc );
		return 0;
	}
	
	new canvas = get_param( 1 );
	return gCanvas[canvas][cols];
}

public nativeCanvasGetHeight( plugin, argc )
{
	if ( argc < 1 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_height expects 1 arguments, %d given", argc );
		return 0;
	}
	new canvas = get_param( 1 );
	return gCanvas[canvas][rows];
}

public nativeCanvasGetSize( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_size expects 3 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );	
	set_param_byref( 2, gCanvas[canvas][cols] );
	set_param_byref( 3, gCanvas[canvas][rows] );
}

public nativeCanvasSetWidth( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_set_width expects 2 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );
	setSize( canvas, get_param( 2 ), gCanvas[canvas][rows] );
}

public nativeCanvasSetHeight( plugin, argc )
{
	if ( argc < 2 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_height expects 2 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );
	setSize( canvas, gCanvas[canvas][cols], get_param( 2 ) );
}

public nativeCanvasSetSize( plugin, argc )
{
	if ( argc < 3 )
	{
		log_error( AMX_ERR_PARAMS, "canvas_get_size expects 3 arguments, %d given", argc );
		return;
	}
	
	new canvas = get_param( 1 );	
	setSize( canvas, get_param( 2 ), get_param( 3 ) );
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
