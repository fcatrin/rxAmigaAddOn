/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include <jni.h>
#include <android/log.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()

#include "SDL_config.h"

#include "SDL_version.h"
#include "SDL_mutex.h"
#include "SDL_events.h"
#if SDL_VERSION_ATLEAST(1,3,0)
#include "SDL_touch.h"
#include "../../events/SDL_touch_c.h"
#endif

#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"
#include "unicodestuff.h"
#include "atan2i.h"

#ifndef SDL_COMPATIBILITY_HACKS_SLOW_COMPATIBLE_EVENT_QUEUE

#if SDL_VERSION_ATLEAST(1,3,0)

#define SDL_SendKeyboardKey(state, keysym) SDL_SendKeyboardKey(state, (keysym)->sym)
extern SDL_Window * ANDROID_CurrentWindow;

#else

#define SDL_SendMouseMotion(A,B,X,Y) SDL_PrivateMouseMotion(0, 0, X, Y)
#define SDL_SendMouseButton(N, A, B) SDL_PrivateMouseButton( A, B, 0, 0 )
#define SDL_SendKeyboardKey(state, keysym) SDL_PrivateKeyboard(state, keysym)

#endif

static volatile int joystickEventsCount;
static int oldMouseButtons = 0;

extern void SDL_ANDROID_PumpEvents()
{
	joystickEventsCount = 0;

	SDL_ANDROID_processAndroidTrackballDampening();
	SDL_ANDROID_processMoveMouseWithKeyboard();
};

extern void SDL_ANDROID_MainThreadPushMouseMotion(int x, int y)
{
	SDL_ANDROID_currentMouseX = x;
	SDL_ANDROID_currentMouseY = y;

	SDL_SendMouseMotion( ANDROID_CurrentWindow, 0, x, y );
}

extern void SDL_ANDROID_MainThreadPushMouseButton(int pressed, int button)
{
	if( ((oldMouseButtons & SDL_BUTTON(button)) != 0) != pressed )
	{
		oldMouseButtons = (oldMouseButtons & ~SDL_BUTTON(button)) | (pressed ? SDL_BUTTON(button) : 0);
		SDL_SendMouseButton( ANDROID_CurrentWindow, pressed, button );
	}

	if(pressed)
		SDL_ANDROID_currentMouseButtons |= SDL_BUTTON(button);
	else
		SDL_ANDROID_currentMouseButtons &= ~(SDL_BUTTON(button));
}

extern void SDL_ANDROID_MainThreadPushKeyboardKey(int pressed, SDL_scancode key, int unicode)
{
	SDL_keysym keysym;

	if( SDL_ANDROID_moveMouseWithArrowKeys && (
		key == SDL_KEY(UP) || key == SDL_KEY(DOWN) ||
		key == SDL_KEY(LEFT) || key == SDL_KEY(RIGHT) ) )
	{
		if( SDL_ANDROID_moveMouseWithKbX < 0 )
		{
			SDL_ANDROID_moveMouseWithKbX = SDL_ANDROID_currentMouseX;
			SDL_ANDROID_moveMouseWithKbY = SDL_ANDROID_currentMouseY;
		}

		if( pressed )
		{
			if( key == SDL_KEY(LEFT) )
			{
				if( SDL_ANDROID_moveMouseWithKbSpeedX > 0 )
					SDL_ANDROID_moveMouseWithKbSpeedX = 0;
				SDL_ANDROID_moveMouseWithKbSpeedX -= SDL_ANDROID_moveMouseWithKbSpeed;
				SDL_ANDROID_moveMouseWithKbAccelX = -SDL_ANDROID_moveMouseWithKbAccel;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded |= 1;
			}
			else if( key == SDL_KEY(RIGHT) )
			{
				if( SDL_ANDROID_moveMouseWithKbSpeedX < 0 )
					SDL_ANDROID_moveMouseWithKbSpeedX = 0;
				SDL_ANDROID_moveMouseWithKbSpeedX += SDL_ANDROID_moveMouseWithKbSpeed;
				SDL_ANDROID_moveMouseWithKbAccelX = SDL_ANDROID_moveMouseWithKbAccel;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded |= 1;
			}

			if( key == SDL_KEY(UP) )
			{
				if( SDL_ANDROID_moveMouseWithKbSpeedY > 0 )
					SDL_ANDROID_moveMouseWithKbSpeedY = 0;
				SDL_ANDROID_moveMouseWithKbSpeedY -= SDL_ANDROID_moveMouseWithKbSpeed;
				SDL_ANDROID_moveMouseWithKbAccelY = -SDL_ANDROID_moveMouseWithKbAccel;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded |= 2;
			}
			else if( key == SDL_KEY(DOWN) )
			{
				if( SDL_ANDROID_moveMouseWithKbSpeedY < 0 )
					SDL_ANDROID_moveMouseWithKbSpeedY = 0;
				SDL_ANDROID_moveMouseWithKbSpeedY += SDL_ANDROID_moveMouseWithKbSpeed;
				SDL_ANDROID_moveMouseWithKbAccelY = SDL_ANDROID_moveMouseWithKbAccel;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded |= 2;
			}
		}
		else
		{
			if( key == SDL_KEY(LEFT) || key == SDL_KEY(RIGHT) )
			{
				SDL_ANDROID_moveMouseWithKbSpeedX = 0;
				SDL_ANDROID_moveMouseWithKbAccelX = 0;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded &= ~1;
			}
			if( key == SDL_KEY(UP) || key == SDL_KEY(DOWN) )
			{
				SDL_ANDROID_moveMouseWithKbSpeedY = 0;
				SDL_ANDROID_moveMouseWithKbAccelY = 0;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded &= ~2;
			}
		}

		SDL_ANDROID_moveMouseWithKbX += SDL_ANDROID_moveMouseWithKbSpeedX;
		SDL_ANDROID_moveMouseWithKbY += SDL_ANDROID_moveMouseWithKbSpeedY;

		SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_moveMouseWithKbX, SDL_ANDROID_moveMouseWithKbY);
		return;
	}

	keysym.scancode = key;
	if ( key < SDLK_LAST )
		keysym.scancode = SDL_android_keysym_to_scancode[key];
	keysym.sym = key;
	keysym.mod = KMOD_NONE;
	keysym.unicode = 0;
#if SDL_VERSION_ATLEAST(1,3,0)
#else
	if ( SDL_TranslateUNICODE )
#endif
		keysym.unicode = unicode;
	if( (keysym.unicode & 0xFF80) != 0 )
		keysym.sym = SDLK_WORLD_0;
	//else if( keysym.sym < 0x80 )
	//	keysym.unicode = keysym.sym;

	if( pressed == SDL_RELEASED )
		keysym.unicode = 0;

	//__android_log_print(ANDROID_LOG_INFO, "libSDL","SDL_SendKeyboardKey sym %d scancode %d unicode %d", keysym.sym, keysym.scancode, keysym.unicode);

	SDL_SendKeyboardKey( pressed, &keysym );
}

extern void SDL_ANDROID_MainThreadPushJoystickAxis(int joy, int axis, int value)
{
	if( ! ( joy < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[joy] ) )
		return;

	if( joystickEventsCount > 64 )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Too many joystick events in the queue - dropping some events");
		return;
	}

	joystickEventsCount++;

	SDL_PrivateJoystickAxis( SDL_ANDROID_CurrentJoysticks[joy], axis, MAX( -32768, MIN( 32767, value ) ) );
}

extern void SDL_ANDROID_MainThreadPushJoystickButton(int joy, int button, int pressed)
{
	if( ! ( joy < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[joy] ) )
		return;

	SDL_PrivateJoystickButton( SDL_ANDROID_CurrentJoysticks[joy], button, pressed );
}

extern void SDL_ANDROID_MainThreadPushJoystickBall(int joy, int ball, int x, int y)
{
	if( ! ( joy < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[joy] ) )
		return;

	SDL_PrivateJoystickBall( SDL_ANDROID_CurrentJoysticks[joy], ball, x, y );
}

extern void SDL_ANDROID_MainThreadPushMultitouchButton(int id, int pressed, int x, int y, int force)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_SendFingerDown(0, id, pressed ? 1 : 0, (float)x / (float)window->w, (float)y / (float)window->h, force);
#endif
}

extern void SDL_ANDROID_MainThreadPushMultitouchMotion(int id, int x, int y, int force)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_SendTouchMotion(0, id, 0, (float)x / (float)window->w, (float)y / (float)window->h, force);
#endif
}

extern void SDL_ANDROID_MainThreadPushMouseWheel(int x, int y)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_SendMouseWheel( ANDROID_CurrentWindow, x, y );
#endif
}

extern void SDL_ANDROID_MainThreadPushAppActive(int active)
{
#if SDL_VERSION_ATLEAST(1,3,0)
				//if( ANDROID_CurrentWindow )
				//	SDL_SendWindowEvent(ANDROID_CurrentWindow, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
#else
	SDL_PrivateAppActive(active, SDL_APPACTIVE|SDL_APPINPUTFOCUS|SDL_APPMOUSEFOCUS);
#endif
}

enum { DEFERRED_TEXT_COUNT = 256 };
static struct { int scancode; int unicode; int down; } deferredText[DEFERRED_TEXT_COUNT];
static int deferredTextIdx1 = 0;
static int deferredTextIdx2 = 0;
static SDL_mutex * deferredTextMutex = NULL;

void SDL_ANDROID_DeferredTextInput()
{
	if( !deferredTextMutex )
		deferredTextMutex = SDL_CreateMutex();

	SDL_mutexP(deferredTextMutex);
	
	if( deferredTextIdx1 != deferredTextIdx2 )
	{
		SDL_keysym keysym;

		deferredTextIdx1++;
		if( deferredTextIdx1 >= DEFERRED_TEXT_COUNT )
			deferredTextIdx1 = 0;
		
		keysym = asciiToKeysym( deferredText[deferredTextIdx1].scancode, deferredText[deferredTextIdx1].unicode );
		if( deferredText[deferredTextIdx1].down == SDL_RELEASED )
			keysym.unicode = 0;

		SDL_SendKeyboardKey( deferredText[deferredTextIdx1].down, &keysym );

		if( SDL_ANDROID_isMouseUsed )
			SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_currentMouseX + (SDL_ANDROID_currentMouseX % 2 ? -1 : 1), SDL_ANDROID_currentMouseY); // Force screen redraw
	}
	else
	{
		if( SDL_ANDROID_TextInputFinished )
		{
			SDL_ANDROID_TextInputFinished = 0;
			SDL_ANDROID_IsScreenKeyboardShownFlag = 0;
		}
	}
	
	SDL_mutexV(deferredTextMutex);
}

extern void SDL_ANDROID_MainThreadPushText( int ascii, int unicode )
{
	int shiftRequired;

#if SDL_VERSION_ATLEAST(1,3,0)
	{
		char text[32];
		UnicodeToUtf8(unicode, text);
		SDL_SendKeyboardText(text);
	}
#endif

	if( !deferredTextMutex )
		deferredTextMutex = SDL_CreateMutex();

	SDL_mutexP(deferredTextMutex);

	shiftRequired = checkShiftRequired(&ascii);

	if( shiftRequired )
	{
		deferredTextIdx2++;
		if( deferredTextIdx2 >= DEFERRED_TEXT_COUNT )
			deferredTextIdx2 = 0;
		deferredText[deferredTextIdx2].down = SDL_PRESSED;
		deferredText[deferredTextIdx2].scancode = SDLK_LSHIFT;
		deferredText[deferredTextIdx2].unicode = 0;
	}
	deferredTextIdx2++;
	if( deferredTextIdx2 >= DEFERRED_TEXT_COUNT )
		deferredTextIdx2 = 0;
	deferredText[deferredTextIdx2].down = SDL_PRESSED;
	deferredText[deferredTextIdx2].scancode = ascii;
	deferredText[deferredTextIdx2].unicode = unicode;

	deferredTextIdx2++;
	if( deferredTextIdx2 >= DEFERRED_TEXT_COUNT )
		deferredTextIdx2 = 0;
	deferredText[deferredTextIdx2].down = SDL_RELEASED;
	deferredText[deferredTextIdx2].scancode = ascii;
	deferredText[deferredTextIdx2].unicode = 0;
	if( shiftRequired )
	{
		deferredTextIdx2++;
		if( deferredTextIdx2 >= DEFERRED_TEXT_COUNT )
			deferredTextIdx2 = 0;
		deferredText[deferredTextIdx2].down = SDL_RELEASED;
		deferredText[deferredTextIdx2].scancode = SDLK_LSHIFT;
		deferredText[deferredTextIdx2].unicode = 0;
	}

	SDL_mutexV(deferredTextMutex);
}

#endif
