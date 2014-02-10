"..\amxxpc.exe" addons\amxmodx\scripting\canvas.sma
copy canvas.amxx addons\amxmodx\plugins\canvas.amxx
copy canvas.amxx ..\..\plugins\canvas.amxx
del canvas.amxx

"..\amxxpc.exe" addons\amxmodx\scripting\canvas-init-progressive.sma
copy canvas-init-progressive.amxx addons\amxmodx\plugins\canvas-init-progressive.amxx
copy canvas-init-progressive.amxx ..\..\plugins\canvas-init-progressive.amxx
del canvas-init-progressive.amxx

"..\amxxpc.exe" addons\amxmodx\scripting\canvas-init-progressive2.sma
copy canvas-init-progressive2.amxx addons\amxmodx\plugins\canvas-init-progressive2.amxx
copy canvas-init-progressive2.amxx ..\..\plugins\canvas-init-progressive2.amxx
del canvas-init-progressive2.amxx

"..\amxxpc.exe" addons\amxmodx\scripting\canvas-program-gradient.sma
copy canvas-program-gradient.amxx addons\amxmodx\plugins\canvas-program-gradient.amxx
copy canvas-program-gradient.amxx ..\..\plugins\canvas-program-gradient.amxx
del canvas-program-gradient.amxx
