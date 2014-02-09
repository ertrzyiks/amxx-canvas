"..\amxxpc.exe" addons\amxmodx\scripting\canvas.sma
copy canvas.amxx addons\amxmodx\plugins\canvas.amxx
copy canvas.amxx ..\..\plugins\canvas.amxx
del canvas.amxx

"..\amxxpc.exe" addons\amxmodx\scripting\canvas-init-progressive.sma
copy canvas-init-progressive.amxx addons\amxmodx\plugins\canvas-init-progressive.amxx
copy canvas-init-progressive.amxx ..\..\plugins\canvas-init-progressive.amxx
del canvas-init-progressive.amxx