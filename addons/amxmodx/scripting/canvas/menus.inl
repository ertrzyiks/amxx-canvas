#include <amxmodx>
#include <amxmisc>

new const giSizes[][2] = {
	{ 28, 8 },
	{ 22, 10 },
	{ 16, 12 },
	{ 15, 15 },
	{ 12, 16 },
	{ 10, 22 },
	{ 8, 28 }
}; 

new const giScales[] = {
	1,
	2,
	4,
	8,
	16
};

new gCanvasMenu;
new gCanvasDetailsMenu;
new gProgramMenu;
new gSizeMenu;
new gScaleMenu;

new giCurrentCanvas[32];

/**
 * Canvas menu
 */
createCanvasMenu()
{
	new gCanvasMenu = menu_create("Canvas menu", "handleCanvasMenu");
	menu_additem( gCanvasMenu, "Create canvas (by aim)" );
}

addCanvasMenuItem( const szLabel[], ... )
{	
	new szItemLabel[32];
	vformat( szItemLabel, 31, szLabel, 2 );
	
	menu_additem( gCanvasMenu, szItemLabel );
}

showCanvasMenu( id )
{
	menu_display( id, gCanvasMenu );
}

public handleCanvasMenu( id, menu, item )
{
	if ( item < 0 )
	{
		return PLUGIN_CONTINUE;
	}
	
	if ( item == 0 )
	{
		createCanvasByAim( id );
		return PLUGIN_CONTINUE;
	}
	
	item -= 1;
	
	giCurrentCanvas[id] = item;
	showCanvasDetailsMenu( id );
	return PLUGIN_CONTINUE;
}


/**
 * Canvas Details menu
 */
createCanvasDetailsMenu()
{
	gCanvasDetailsMenu = menu_create("Canvas Details menu", "handleCanvasDetailsMenu");
	menu_additem( gCanvasDetailsMenu, "Change program" );
	menu_additem( gCanvasDetailsMenu, "Change size" );
	menu_additem( gCanvasDetailsMenu, "Change scale" );
}

public handleCanvasDetailsMenu( id, menu, item )
{
	switch ( item )
	{
		case 0: {
			showProgramMenu( id );
		}
		case 1: {
			showSizeMenu( id );
		}
		case 2: {
			showScaleMenu( id );
		}
		
		case MENU_EXIT:{
			showCanvasMenu( id );
		}
	}
	return PLUGIN_CONTINUE;
}

showCanvasDetailsMenu( id )
{
	menu_display( id, gCanvasDetailsMenu );
}


/**
 * Canvas Program menu
 */
new programMenuCb;
 
createProgramMenu()
{
	gProgramMenu = menu_create("Program menu", "handleProgramMenu");
	programMenuCb = menu_makecallback( "checkProgramMenu" );
}

addProgramMenuItem( const szLabel[], ...  )
{
	new szItemLabel[32];
	vformat( szItemLabel, 31, szLabel, 2 );
	
	menu_additem( gProgramMenu, szItemLabel, "", 0, programMenuCb );
}

showProgramMenu( id )
{
	menu_display( id, gProgramMenu );
}

public handleProgramMenu( id, menu, item )
{
	if ( item < 0 )
	{
		return PLUGIN_CONTINUE;
	}
	
	setProgram( giCurrentCanvas[id], item );
	showCanvasDetailsMenu( id );
	return PLUGIN_CONTINUE;
}

public checkProgramMenu( id, menu, item )
{
	if ( getProgram( giCurrentCanvas[id] ) == item )
	{
		return ITEM_DISABLED;
	}
	return ITEM_ENABLED;
}

/**
 * Canvas Size menu
 */
createSizeMenu()
{
	gSizeMenu = menu_create("Size menu", "handleSizeMenu");
	new cb = menu_makecallback( "checkSizeMenuItem" );
	
	new szLabel[32];
	
	for ( new i = 0; i < sizeof( giSizes ); i++ )
	{
		formatex( szLabel, 31, "%dx%d", giSizes[i][0], giSizes[i][1] );
		menu_additem( gSizeMenu, szLabel, "", 0, cb );
	}
}

public handleSizeMenu( id, menu, item )
{
	if ( item >= 0 )
	{
		setSize( giCurrentCanvas[id], giSizes[item][0], giSizes[item][1] );
	}
	
	showCanvasDetailsMenu( id );
	return PLUGIN_CONTINUE;
}

public checkSizeMenuItem( id, menu, item )
{
	new width, height;
	getSize( giCurrentCanvas[id], width, height );
	
	if ( width == giSizes[item][0] && height == giSizes[item][1] )
	{
		return ITEM_DISABLED;
	}
	return ITEM_ENABLED;
}

showSizeMenu( id )
{
	menu_display( id, gSizeMenu );
}

/**
 * Canvas Scale menu
 */
createScaleMenu()
{
	gScaleMenu = menu_create("Scale menu", "handleScaleMenu");
	new cb = menu_makecallback( "checkScaleMenuItem" );
	
	new szLabel[32];
	
	for ( new i = 0; i < sizeof( giScales ); i++ )
	{
		formatex( szLabel, 31, "x%d", giScales[i] );
		menu_additem( gScaleMenu, szLabel, "", 0, cb );
	}
}

public handleScaleMenu( id, menu, item )
{
	if ( item >= 0 )
	{
		setScale( giCurrentCanvas[id], giScales[item] );
	}
	
	showCanvasDetailsMenu( id );
	return PLUGIN_CONTINUE;
}

public checkScaleMenuItem( id, menu, item )
{
	if ( getScale( giCurrentCanvas[id] ) == giScales[item] )
	{
		return ITEM_DISABLED;
	}
	return ITEM_ENABLED;
}

showScaleMenu( id )
{
	menu_display( id, gScaleMenu );
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
