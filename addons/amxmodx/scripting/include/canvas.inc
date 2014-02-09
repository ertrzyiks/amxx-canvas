#if defined _canvas_included
  #endinput
#endif
#define _canvas_included

#define CANVAS_MAX_PIXELS 256
#define CANVAS_MAX_INIT_NAME 32


/**
 * @param szName {String} Name of initialize
 * @param szCallacbk {String} Name of public function to call
 * @param iTempo {Int} Count of pixels per tick
 */
native register_canvas_initializer( const szName[], const szCallback[], iTempo = 1 );


/**
 *
 */
native canvas_get_pixels( canvas, pixels[], size );
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1045\\ f0\\ fs16 \n\\ par }
*/
