Uae4all2-sdl 2.3.0.0
based on uae4all openpandora-port sources

Features: AGA/OCS/ECS, 68020 and 68000 emulation, harddisk-support, WHDLoad-support, Chip/Slow/Fast-mem settings, savestates, vsync, most games run fullspeed

Authors:
Chui, john4p, TomB, notaz, Bernd Schneider, Toni Wilen, Pickle, smoku, AnotherGuest, Anonymous engineer, finkel
android build by Lubomyr
android libSDL provided by pelya 

Control  (22.12.2013)

'1'- <Button A> ok/fire [HOME]
'2'- <Button B> [END]
'3'- <Button X> alt.fire/left click [PAGEDOWN]
'4'- <Button Y> [PAGEUP]
additional buttons (can be added via SDL-settings)
'5' <Button L> [SDLK_RSHIFT]
'6' <Button R> [SDLK_RCTRL]
old textUI vkeybd = '2'+'4'

SDL keycode action:
F13 - left mouse button
F14 - right mouse click
F15 textUI vkeybd
LCTRL - emulator menu toggle 
LALT - change input method

on-screen scheme

   4 3
   2 1

Please put kickstarts files in android/data/pandora.uae4all.sdl/kickstarts directory.
Files must be named as kick13.rom kick20.rom kick31.rom

Some recomendation:

for physical mouse use:
please push 'change device configuration' when SDL-logo present on screen, when run uae4all2-sdl
then select 'mouse emulation' -> 'advanced features'
and disable tick 'Relative mouse movement' (laptop mode)

for better touchscreen mouse emulation in workbench:
please push 'change device configuration' when SDl-logo present on screen, when run uae4all2-sdl
then select 'mouse emulation' -> 'Left mouse click'
then select 'Tap or Hold' -> '1.0 sec'