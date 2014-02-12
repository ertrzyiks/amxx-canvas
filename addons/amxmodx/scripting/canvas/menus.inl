#include <amxmodx>
#include <amxmisc>

new gCanvasMenu;
new gProgramMenu;

new giCurrentCanvas[32];

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
	showProgramMenu( id );
	return PLUGIN_CONTINUE;
}

createProgramMenu()
{
	gProgramMenu = menu_create("Program menu", "handleProgramMenu");
}

addProgramMenuItem( const szLabel[], ...  )
{
	new szItemLabel[32];
	vformat( szItemLabel, 31, szLabel, 2 );
	
	menu_additem( gProgramMenu, szItemLabel );
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
	
	return PLUGIN_CONTINUE;
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
