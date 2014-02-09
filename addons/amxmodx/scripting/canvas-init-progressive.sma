#include <amxmodx>
#include <amxmisc>
#include <canvas>

#define PLUGIN "Canvas Init - Progressive"
#define VERSION "1.0"
#define AUTHOR "R3X"


public plugin_init() {
	register_plugin(PLUGIN, VERSION, AUTHOR);
	
	register_canvas_initializer( "Progressive", "cbInitializer" );
}

public cbInitializer( canvas, tick )
{
	return tick;
}
