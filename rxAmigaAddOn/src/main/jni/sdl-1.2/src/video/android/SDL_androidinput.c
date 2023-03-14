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
#include <pthread.h>
#include <semaphore.h>

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
#include "SDL_screenkeyboard.h"
#include "jniwrapperstuff.h"
#include "unicodestuff.h"
#include "atan2i.h"

SDLKey SDL_android_keymap[KEYCODE_LAST+1];

static inline SDL_scancode TranslateKey(int scancode)
{
	if ( scancode >= KEYCODE_LAST + 1 )
		scancode = KEYCODE_UNKNOWN;
	return SDL_android_keymap[scancode];
}

static int isTrackballUsed = 0;
int SDL_ANDROID_isMouseUsed = 0;

#define NORMALIZE_FLOAT_32767(X) (fminf(32767.0f, fmaxf(-32767.0f, (X) * 32767.0f)))

enum { RIGHT_CLICK_NONE = 0, RIGHT_CLICK_WITH_MULTITOUCH = 1, RIGHT_CLICK_WITH_PRESSURE = 2, 
		RIGHT_CLICK_WITH_KEY = 3, RIGHT_CLICK_WITH_TIMEOUT = 4 };
enum { LEFT_CLICK_NORMAL = 0, LEFT_CLICK_NEAR_CURSOR = 1, LEFT_CLICK_WITH_MULTITOUCH = 2, LEFT_CLICK_WITH_PRESSURE = 3,
		LEFT_CLICK_WITH_KEY = 4, LEFT_CLICK_WITH_TIMEOUT = 5, LEFT_CLICK_WITH_TAP = 6, LEFT_CLICK_WITH_TAP_OR_TIMEOUT = 7 };
enum { JOY_TOUCHSCREEN = 0, JOY_ACCELGYRO = 1, JOY_GAMEPAD1 = 2, JOY_GAMEPAD2 = 3, JOY_GAMEPAD3 = 4, JOY_GAMEPAD4 = 5 };
static int leftClickMethod = LEFT_CLICK_NORMAL;
static int rightClickMethod = RIGHT_CLICK_NONE;
static int leftClickKey = KEYCODE_DPAD_CENTER;
static int rightClickKey = KEYCODE_MENU;
int SDL_ANDROID_ShowScreenUnderFinger = ZOOM_NONE;
SDL_Rect SDL_ANDROID_ShowScreenUnderFingerRect = {0, 0, 0, 0}, SDL_ANDROID_ShowScreenUnderFingerRectSrc = {0, 0, 0, 0};
static int clickMouseWithDpadCenter = 0;
int SDL_ANDROID_moveMouseWithArrowKeys = 0;
int SDL_ANDROID_moveMouseWithKbSpeed = 0;
int SDL_ANDROID_moveMouseWithKbAccel = 0;
int SDL_ANDROID_moveMouseWithKbX = -1, SDL_ANDROID_moveMouseWithKbY = -1;
int SDL_ANDROID_moveMouseWithKbSpeedX = 0, SDL_ANDROID_moveMouseWithKbSpeedY = 0;
int SDL_ANDROID_moveMouseWithKbAccelX = 0, SDL_ANDROID_moveMouseWithKbAccelY = 0;
int SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded = 0;
static int maxForce = 0;
static int maxRadius = 0;
int SDL_ANDROID_joysticksAmount = 0;
static int SDL_ANDROID_isAccelerometerUsed = 0;
static int isMultitouchUsed = 0;
SDL_Joystick *SDL_ANDROID_CurrentJoysticks[JOY_GAMEPAD4+1];
static int TrackballDampening = 0; // in milliseconds
static Uint32 lastTrackballAction = 0;
enum { TOUCH_PTR_UP = 0, TOUCH_PTR_MOUSE = 1, TOUCH_PTR_SCREENKB = 2 };
static int touchPointers[MAX_MULTITOUCH_POINTERS] = {0};
static int firstMousePointerId = -1, secondMousePointerId = -1;
enum { MAX_MULTITOUCH_GESTURES = 4 };
static int multitouchGestureKeycode[MAX_MULTITOUCH_GESTURES] = {
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_6)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_7)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_8)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_9))
};
static int multitouchGestureSensitivity = 0;
static int multitouchGestureDist = -1;
static int multitouchGestureAngle = 0;
static int multitouchGestureX = -1;
static int multitouchGestureY = -1;
static int multitouchGestureMiddleX = -1;
static int multitouchGestureMiddleY = -1;
static int multitouchGestureHappened = 0;
enum { MULTITOUCH_MOUSE_WHEEL_DIST = 20 };
int SDL_ANDROID_TouchscreenCalibrationWidth = 480;
int SDL_ANDROID_TouchscreenCalibrationHeight = 320;
int SDL_ANDROID_TouchscreenCalibrationX = 0;
int SDL_ANDROID_TouchscreenCalibrationY = 0;
static int leftClickTimeout = 0;
static int rightClickTimeout = 0;
static int mouseInitialX = -1;
static int mouseInitialY = -1;
static unsigned int mouseInitialTime = 0;
static int deferredMouseTap = 0;
static unsigned int leftButtonDownTime = 0;
static int relativeMovement = 0;
static int relativeMovementSpeed = 2;
static int relativeMovementAccel = 0;
static int relativeMovementX = 0;
static int relativeMovementY = 0;
static unsigned int relativeMovementTime = 0;
int SDL_ANDROID_currentMouseX = 0;
int SDL_ANDROID_currentMouseY = 0;
int SDL_ANDROID_currentMouseButtons = 0;

static int hardwareMouseDetected = 0;
enum { MOUSE_HW_BUTTON_LEFT = 1, MOUSE_HW_BUTTON_RIGHT = 2, MOUSE_HW_BUTTON_MIDDLE = 4, MOUSE_HW_BUTTON_BACK = 8, MOUSE_HW_BUTTON_FORWARD = 16, MOUSE_HW_BUTTON_MAX = MOUSE_HW_BUTTON_FORWARD };
enum { MOUSE_HW_INPUT_FINGER = 0, MOUSE_HW_INPUT_STYLUS = 1, MOUSE_HW_INPUT_MOUSE = 2 };
enum { DEADZONE_HOVER_FINGER = 50, DEADZONE_HOVER_STYLUS = 80, HOVER_FREEZE_TIME = 500, HOVER_DISTANCE_MAX = 1024, HOVER_REDRAW_SCREEN = 1024 * 10 };
static int hoverJitterFilter = 1;
static int hoverX, hoverY, hoverTime = 0, hoverMouseFreeze = 0, hoverDeadzone = 0;
static int rightMouseButtonLongPress = 1;
static int moveMouseWithGyroscope = 0;
static float moveMouseWithGyroscopeSpeed = 5.0f;
static int moveMouseWithGyroscopeX = 0;
static int moveMouseWithGyroscopeY = 0;

static pthread_t mouseClickTimeoutThreadId = 0;
static sem_t mouseClickTimeoutSemaphore;
static int mouseClickTimeout = 100000;
static void *mouseClickTimeoutThread (void *);

static inline int InsideRect( const SDL_Rect * r, int x, int y )
{
	return ( x >= r->x && x <= r->x + r->w ) && ( y >= r->y && y <= r->y + r->h );
}

void UpdateScreenUnderFingerRect( int x, int y )
{
#if SDL_VERSION_ATLEAST(1,3,0)
	return;
#else
	int screenX = SDL_ANDROID_sFakeWindowWidth, screenY = SDL_ANDROID_sFakeWindowHeight;
	if( SDL_ANDROID_ShowScreenUnderFinger == ZOOM_NONE )
		return;

	if( SDL_ANDROID_ShowScreenUnderFinger == ZOOM_MAGNIFIER )
	{
		SDL_ANDROID_ShowScreenUnderFingerRectSrc.w = screenX / 4;
		SDL_ANDROID_ShowScreenUnderFingerRectSrc.h = screenY / 4;
		SDL_ANDROID_ShowScreenUnderFingerRectSrc.x = x - SDL_ANDROID_ShowScreenUnderFingerRectSrc.w/2;
		SDL_ANDROID_ShowScreenUnderFingerRectSrc.y = y - SDL_ANDROID_ShowScreenUnderFingerRectSrc.h/2;
		if( SDL_ANDROID_ShowScreenUnderFingerRectSrc.x < 0 )
			SDL_ANDROID_ShowScreenUnderFingerRectSrc.x = 0;
		if( SDL_ANDROID_ShowScreenUnderFingerRectSrc.y < 0 )
			SDL_ANDROID_ShowScreenUnderFingerRectSrc.y = 0;

		if( SDL_ANDROID_ShowScreenUnderFingerRectSrc.x > screenX - SDL_ANDROID_ShowScreenUnderFingerRectSrc.w )
			SDL_ANDROID_ShowScreenUnderFingerRectSrc.x = screenX - SDL_ANDROID_ShowScreenUnderFingerRectSrc.w;
		if( SDL_ANDROID_ShowScreenUnderFingerRectSrc.y > screenY - SDL_ANDROID_ShowScreenUnderFingerRectSrc.h )
			SDL_ANDROID_ShowScreenUnderFingerRectSrc.y = screenY - SDL_ANDROID_ShowScreenUnderFingerRectSrc.h;

		SDL_ANDROID_ShowScreenUnderFingerRect.w = SDL_ANDROID_ShowScreenUnderFingerRectSrc.w * 3 / 2;
		SDL_ANDROID_ShowScreenUnderFingerRect.h = SDL_ANDROID_ShowScreenUnderFingerRectSrc.h * 3 / 2;
		SDL_ANDROID_ShowScreenUnderFingerRect.x = x + SDL_ANDROID_ShowScreenUnderFingerRect.w/10;
		SDL_ANDROID_ShowScreenUnderFingerRect.y = y - SDL_ANDROID_ShowScreenUnderFingerRect.h*11/10;
		if( SDL_ANDROID_ShowScreenUnderFingerRect.x < 0 )
			SDL_ANDROID_ShowScreenUnderFingerRect.x = 0;
		if( SDL_ANDROID_ShowScreenUnderFingerRect.y < 0 )
			SDL_ANDROID_ShowScreenUnderFingerRect.y = 0;
		if( SDL_ANDROID_ShowScreenUnderFingerRect.x + SDL_ANDROID_ShowScreenUnderFingerRect.w >= screenX )
			SDL_ANDROID_ShowScreenUnderFingerRect.x = screenX - SDL_ANDROID_ShowScreenUnderFingerRect.w - 1;
		if( SDL_ANDROID_ShowScreenUnderFingerRect.y + SDL_ANDROID_ShowScreenUnderFingerRect.h >= screenY )
			SDL_ANDROID_ShowScreenUnderFingerRect.y = screenY - SDL_ANDROID_ShowScreenUnderFingerRect.h - 1;
		if( InsideRect(&SDL_ANDROID_ShowScreenUnderFingerRect, x, y) )
			SDL_ANDROID_ShowScreenUnderFingerRect.x = x - SDL_ANDROID_ShowScreenUnderFingerRect.w*11/10 - 1;
	}
#endif
}

#if SDL_VERSION_ATLEAST(1,3,0)
#define SDL_ANDROID_sFakeWindowWidth (SDL_GetFocusWindow() ? SDL_GetFocusWindow()->w : 100)
#define SDL_ANDROID_sFakeWindowHeight (SDL_GetFocusWindow() ? SDL_GetFocusWindow()->h : 100)
#endif

static int BumpPointerId( int pointerId )
{
	if(pointerId < 0)
		pointerId = 0;
	if(pointerId >= MAX_MULTITOUCH_POINTERS)
		pointerId = MAX_MULTITOUCH_POINTERS-1;
	return pointerId;
}

static int ProcessTouchscreenKeyboard( int x, int y, int action, int pointerId )
{
	// The touch is passed either to on-screen keyboard or as mouse event for all duration of touch between down and up,
	// even if the finger is not anymore above screen kb button it will not acr as mouse event, and if it's initially
	// touches the screen outside of screen kb it won't trigger button keypress -
	// I think it's more logical this way
	if( SDL_ANDROID_isTouchscreenKeyboardUsed && ( action == MOUSE_DOWN || touchPointers[pointerId] & TOUCH_PTR_SCREENKB ) )
	{
		unsigned processed = SDL_ANDROID_processTouchscreenKeyboard(x, y, action, pointerId);
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_ANDROID_processTouchscreenKeyboard: ptr %d action %d ret 0x%08x", pointerId, action, processed);
		if( processed && action == MOUSE_DOWN )
			touchPointers[pointerId] |= TOUCH_PTR_SCREENKB;
		if( touchPointers[pointerId] & TOUCH_PTR_SCREENKB )
		{
			if( action == MOUSE_UP )
				touchPointers[pointerId] = TOUCH_PTR_UP;
			if( !(processed & TOUCHSCREEN_KEYBOARD_PASS_EVENT_DOWN_TO_SDL) )
				return 1;
		}
	}
	return 0;
}

static void AssignNewTouchPointers( int action, int pointerId )
{
	int i;
	if( action == MOUSE_DOWN )
	{
		touchPointers[pointerId] |= TOUCH_PTR_MOUSE;
		firstMousePointerId = -1;
		secondMousePointerId = -1;
		for( i = 0; i < MAX_MULTITOUCH_POINTERS; i++ )
		{
			if( touchPointers[i] & TOUCH_PTR_MOUSE )
			{
				if( firstMousePointerId == -1 )
					firstMousePointerId = i;
				else if( secondMousePointerId == -1 )
					secondMousePointerId = i;
				else
					break;
			}
		}
	}
}

static void ClearOldTouchPointers( int action, int pointerId )
{
	int i;
	if( action == MOUSE_UP )
	{
		touchPointers[pointerId] = TOUCH_PTR_UP;
		firstMousePointerId = -1;
		secondMousePointerId = -1;
		for( i = 0; i < MAX_MULTITOUCH_POINTERS; i++ )
		{
			if( touchPointers[i] |= TOUCH_PTR_MOUSE )
			{
				if( firstMousePointerId == -1 )
					firstMousePointerId = i;
				else if( secondMousePointerId == -1 )
					secondMousePointerId = i;
				else
					break;
			}
		}
	}
}

static void AdjustTouchScreenCalibration( jint *xx, jint *yy )
{
	int x = *xx, y = *yy;

	x -= SDL_ANDROID_TouchscreenCalibrationX;
	y -= SDL_ANDROID_TouchscreenCalibrationY;
#if SDL_VIDEO_RENDER_RESIZE
	// Translate mouse coordinates

	x -= (SDL_ANDROID_sRealWindowWidth - SDL_ANDROID_sWindowWidth) / 2;
	x = x * SDL_ANDROID_sFakeWindowWidth / SDL_ANDROID_TouchscreenCalibrationWidth;
	y = y * SDL_ANDROID_sFakeWindowHeight / SDL_ANDROID_TouchscreenCalibrationHeight;
	if( x < 0 )
		x = 0;
	if( x > SDL_ANDROID_sFakeWindowWidth )
		x = SDL_ANDROID_sFakeWindowWidth;
	if( y < 0 )
		y = 0;
	if( y > SDL_ANDROID_sFakeWindowHeight )
		y = SDL_ANDROID_sFakeWindowHeight;
	
#else
	x = x * SDL_ANDROID_sRealWindowWidth / SDL_ANDROID_TouchscreenCalibrationWidth;
	y = y * SDL_ANDROID_sRealWindowHeight / SDL_ANDROID_TouchscreenCalibrationHeight;
#endif

	*xx = x;
	*yy = y;
}

static void ProcessMultitouchGesture( int x, int y, int action, int pointerId )
{
	int i;
	if( action == MOUSE_UP )
	{
		multitouchGestureX = -1;
		multitouchGestureY = -1;
		multitouchGestureDist = -1;
		multitouchGestureMiddleX = -1;
		multitouchGestureMiddleY = -1;
	}
	else if( !hardwareMouseDetected && !(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MMASK) )
	{
		if( firstMousePointerId != pointerId )
		{
			multitouchGestureX = x;
			multitouchGestureY = y;
		}
		if( firstMousePointerId == pointerId && multitouchGestureX >= 0 )
		{
			int dist = abs( x - multitouchGestureX ) + abs( y - multitouchGestureY );
			int angle = atan2i( y - multitouchGestureY, x - multitouchGestureX );
			int middleX = (x + multitouchGestureX) / 2;
			int middleY = (y + multitouchGestureY) / 2;
			if( multitouchGestureDist < 0 )
			{
				multitouchGestureDist = dist;
				multitouchGestureAngle = angle;
				multitouchGestureMiddleX = middleX;
				multitouchGestureMiddleY = middleY;
			}
			else
			{
				int distMaxDiff = SDL_ANDROID_sFakeWindowHeight / ( 1 + (1 + multitouchGestureSensitivity) * 2 );
				int angleMaxDiff = atan2i_PI * 2 / 3 / ( 1 + (1 + multitouchGestureSensitivity) * 2 );
				int wheelThreshold = SDL_ANDROID_sFakeWindowHeight / MULTITOUCH_MOUSE_WHEEL_DIST;
				if( dist - multitouchGestureDist > distMaxDiff )
				{
					multitouchGestureHappened = 1;
					multitouchGestureDist += distMaxDiff;
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, multitouchGestureKeycode[0], 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, multitouchGestureKeycode[0], 0 );
				}
				if( multitouchGestureDist - dist > distMaxDiff )
				{
					multitouchGestureHappened = 1;
					multitouchGestureDist -= distMaxDiff;
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, multitouchGestureKeycode[1], 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, multitouchGestureKeycode[1], 0 );
				}

				int angleDiff = angle - multitouchGestureAngle;

				while( angleDiff < atan2i_PI )
					angleDiff += atan2i_PI * 2;
				while( angleDiff > atan2i_PI )
					angleDiff -= atan2i_PI * 2;

				if( angleDiff < -angleMaxDiff )
				{
					multitouchGestureHappened = 1;
					multitouchGestureAngle = angle;
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, multitouchGestureKeycode[2], 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, multitouchGestureKeycode[2], 0 );
				}
				if( angleDiff > angleMaxDiff )
				{
					multitouchGestureHappened = 1;
					multitouchGestureAngle = angle;
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, multitouchGestureKeycode[3], 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, multitouchGestureKeycode[3], 0 );
				}

				if( rightMouseButtonLongPress )
					return;

				//__android_log_print(ANDROID_LOG_INFO, "libSDL", "middleY %d multitouchGestureMiddleY %d threshold %d", middleY, multitouchGestureMiddleY, wheelThreshold);
				if( middleX - multitouchGestureMiddleX > wheelThreshold )
				{
					multitouchGestureHappened = 1;
					multitouchGestureMiddleX += wheelThreshold;
					SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_X1 );
					SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_X1 );
				}
				if( multitouchGestureMiddleX - middleX > wheelThreshold )
				{
					multitouchGestureHappened = 1;
					multitouchGestureMiddleX -= wheelThreshold;
					SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_X2 );
					SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_X2 );
				}
				if( middleY - multitouchGestureMiddleY > wheelThreshold )
				{
					multitouchGestureHappened = 1;
					multitouchGestureMiddleY += wheelThreshold;
					SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_WHEELUP );
					SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_WHEELUP );
				}
				if( multitouchGestureMiddleY - middleY > wheelThreshold )
				{
					multitouchGestureHappened = 1;
					multitouchGestureMiddleY -= wheelThreshold;
					SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_WHEELDOWN );
					SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_WHEELDOWN );
				}
			}
		}
	}
}

static void SendMultitouchEvents( int x, int y, int action, int pointerId, int force, int radius )
{
	if( isMultitouchUsed && (action == MOUSE_DOWN || action == MOUSE_UP || action == MOUSE_MOVE) ) // Ignore hover events
	{
#if SDL_VERSION_ATLEAST(1,3,0)
		// Use nifty SDL 1.3 multitouch API
		if( action == MOUSE_MOVE )
			SDL_ANDROID_MainThreadPushMultitouchMotion(pointerId, x, y, force + radius);
		else
			SDL_ANDROID_MainThreadPushMultitouchButton(pointerId, action == MOUSE_DOWN ? 1 : 0, x, y, force + radius);
#endif

		if( action == MOUSE_DOWN )
			SDL_ANDROID_MainThreadPushJoystickButton(JOY_TOUCHSCREEN, pointerId, SDL_PRESSED);
		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_TOUCHSCREEN, pointerId+4, force + radius); // Radius is more sensitive usually
		SDL_ANDROID_MainThreadPushJoystickBall(JOY_TOUCHSCREEN, pointerId, x, y);
		if( action == MOUSE_UP )
			SDL_ANDROID_MainThreadPushJoystickButton(JOY_TOUCHSCREEN, pointerId, SDL_RELEASED);
	}

	if( !isMultitouchUsed && !SDL_ANDROID_isMouseUsed && !SDL_ANDROID_isTouchscreenKeyboardUsed )
	{
		SDL_keysym keysym;
		if( action == MOUSE_DOWN || action == MOUSE_UP )
			SDL_ANDROID_MainThreadPushKeyboardKey( action == MOUSE_DOWN ? SDL_PRESSED : SDL_RELEASED, SDL_ANDROID_GetScreenKeyboardButtonKey(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0), 0 );
		return;
	}
}

static void ProcessMouseRelativeMovement( jint *xx, jint *yy, int action )
{
	int x = *xx, y = *yy;

	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "process mouse relative x,y = %i, %i", x, y);
	if( !relativeMovement )
		return;

	if( action == MOUSE_DOWN )
	{
		relativeMovementX = SDL_ANDROID_currentMouseX - x;
		relativeMovementY = SDL_ANDROID_currentMouseY - y;
	}
	x += relativeMovementX;
	y += relativeMovementY;
	
	int diffX = x - SDL_ANDROID_currentMouseX;
	int diffY = y - SDL_ANDROID_currentMouseY;
	int coeff = relativeMovementSpeed + 2;
	if( relativeMovementSpeed > 2 )
		coeff += relativeMovementSpeed - 2;
	diffX = diffX * coeff / 4;
	diffY = diffY * coeff / 4;
	if( relativeMovementAccel > 0 )
	{
		unsigned int newTime = SDL_GetTicks();
		if( newTime - relativeMovementTime > 0 )
		{
			diffX += diffX * ( relativeMovementAccel * 30 ) / (int)(newTime - relativeMovementTime);
			diffY += diffY * ( relativeMovementAccel * 30 ) / (int)(newTime - relativeMovementTime);
		}
		relativeMovementTime = newTime;
	}
	diffX -= x - SDL_ANDROID_currentMouseX;
	diffY -= y - SDL_ANDROID_currentMouseY;
	x += diffX;
	y += diffY;
	relativeMovementX += diffX;
	relativeMovementY += diffY;

	diffX = x;
	diffY = y;
	if( x < 0 )
		x = 0;
	if( x > SDL_ANDROID_sFakeWindowWidth )
		x = SDL_ANDROID_sFakeWindowWidth;
	if( y < 0 )
		y = 0;
	if( y > SDL_ANDROID_sFakeWindowHeight )
		y = SDL_ANDROID_sFakeWindowHeight;
	relativeMovementX += x - diffX;
	relativeMovementY += y - diffY;

	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "process mouse relative final x,y = %i, %i", x, y);

	*xx = x;
	*yy = y;
}

static void ProcessMouseUp( int x, int y )
{
	if( rightClickMethod != RIGHT_CLICK_WITH_KEY )
		SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_RIGHT );

	if( mouseInitialX >= 0 && mouseInitialY >= 0 && (
		leftClickMethod == LEFT_CLICK_WITH_TAP || leftClickMethod == LEFT_CLICK_WITH_TAP_OR_TIMEOUT ) &&
		abs(mouseInitialX - x) < SDL_ANDROID_sFakeWindowHeight / 16 &&
		abs(mouseInitialY - y) < SDL_ANDROID_sFakeWindowHeight / 16 &&
		SDL_GetTicks() - mouseInitialTime < 700 )
	{
		SDL_ANDROID_MainThreadPushMouseMotion( mouseInitialX, mouseInitialY );
		SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_LEFT );
		deferredMouseTap = 2;
		mouseInitialX = -1;
		mouseInitialY = -1;
	}
	else
	{
		if( leftClickMethod != LEFT_CLICK_WITH_KEY )
			SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_LEFT );
	}

	SDL_ANDROID_ShowScreenUnderFingerRect.w = SDL_ANDROID_ShowScreenUnderFingerRect.h = 0;
	SDL_ANDROID_ShowScreenUnderFingerRectSrc.w = SDL_ANDROID_ShowScreenUnderFingerRectSrc.h = 0;
	if( SDL_ANDROID_ShowScreenUnderFinger == ZOOM_MAGNIFIER )
	{
		// Move mouse by 1 pixel so it will force screen update and mouse-under-finger window will be removed
		if( SDL_ANDROID_moveMouseWithKbX >= 0 )
			SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_moveMouseWithKbX > 0 ? SDL_ANDROID_moveMouseWithKbX-1 : 0, SDL_ANDROID_moveMouseWithKbY);
		else
			SDL_ANDROID_MainThreadPushMouseMotion(x > 0 ? x-1 : 0, y);
	}
	SDL_ANDROID_moveMouseWithKbX = -1;
	SDL_ANDROID_moveMouseWithKbY = -1;
	SDL_ANDROID_moveMouseWithKbSpeedX = 0;
	SDL_ANDROID_moveMouseWithKbSpeedY = 0;
	if( !deferredMouseTap )
	{
		moveMouseWithGyroscopeX = 0;
		moveMouseWithGyroscopeY = 0;
	}
}

static int ProcessMouseDown( int x, int y )
{
	int action = MOUSE_DOWN;
	if( (SDL_ANDROID_moveMouseWithKbX >= 0 || leftClickMethod == LEFT_CLICK_NEAR_CURSOR) &&
		abs(SDL_ANDROID_currentMouseX - x) < SDL_ANDROID_sFakeWindowWidth / 10 && abs(SDL_ANDROID_currentMouseY - y) < SDL_ANDROID_sFakeWindowHeight / 10 )
	{
		SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_LEFT );
		SDL_ANDROID_moveMouseWithKbX = SDL_ANDROID_currentMouseX;
		SDL_ANDROID_moveMouseWithKbY = SDL_ANDROID_currentMouseY;
		SDL_ANDROID_moveMouseWithKbSpeedX = 0;
		SDL_ANDROID_moveMouseWithKbSpeedY = 0;
		leftButtonDownTime = SDL_GetTicks();
		action = MOUSE_MOVE;
	}
	else
	if( leftClickMethod == LEFT_CLICK_NORMAL )
	{
		SDL_ANDROID_MainThreadPushMouseMotion(x, y);
		if( !hardwareMouseDetected || SDL_ANDROID_currentMouseButtons == 0 )
			SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_LEFT );
	}
	else
	{
		SDL_ANDROID_MainThreadPushMouseMotion(x, y);
		action == MOUSE_MOVE;
		mouseInitialX = x;
		mouseInitialY = y;
		mouseInitialTime = SDL_GetTicks();
	}
	if( SDL_ANDROID_ShowScreenUnderFinger == ZOOM_MAGNIFIER )
		UpdateScreenUnderFingerRect(x, y);
	return action;
}

static void ProcessMouseMove_Timeouts( int x, int y )
{
	if( mouseInitialX >= 0 && mouseInitialY >= 0 && (
		leftClickMethod == LEFT_CLICK_WITH_TIMEOUT || leftClickMethod == LEFT_CLICK_WITH_TAP ||
		leftClickMethod == LEFT_CLICK_WITH_TAP_OR_TIMEOUT || rightClickMethod == RIGHT_CLICK_WITH_TIMEOUT ) )
	{
		if( abs(mouseInitialX - x) >= SDL_ANDROID_sFakeWindowHeight / 15 || abs(mouseInitialY - y) >= SDL_ANDROID_sFakeWindowHeight / 15 )
		{
			mouseInitialX = -1;
			mouseInitialY = -1;
		}
		else
		{
			if( leftClickMethod == LEFT_CLICK_WITH_TIMEOUT || leftClickMethod == LEFT_CLICK_WITH_TAP_OR_TIMEOUT )
			{
				if( SDL_GetTicks() - mouseInitialTime > leftClickTimeout )
				{
					//SDL_ANDROID_MainThreadPushMouseMotion(mouseInitialX, mouseInitialY);
					SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_LEFT );
					mouseInitialX = -1;
					mouseInitialY = -1;
				}
			}
			if( rightClickMethod == RIGHT_CLICK_WITH_TIMEOUT )
			{
				if( SDL_GetTicks() - mouseInitialTime > rightClickTimeout )
				{
					//SDL_ANDROID_MainThreadPushMouseMotion(mouseInitialX, mouseInitialY);
					SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_RIGHT );
					mouseInitialX = -1;
					mouseInitialY = -1;
				}
			}
		}
	}
	if( SDL_ANDROID_ShowScreenUnderFinger == ZOOM_MAGNIFIER )
		UpdateScreenUnderFingerRect(x, y);
}

static void ProcessMouseMove( int x, int y, int force, int radius )
{
	if( SDL_ANDROID_moveMouseWithKbX >= 0 )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "process mouse move x,y = %i, %i", x, y);

		// Mouse lazily follows magnifying glass, not very intuitive for drag&drop
		/*
		if( abs(moveMouseWithKbX - x) > SDL_ANDROID_sFakeWindowWidth / 12 )
			moveMouseWithKbSpeedX += moveMouseWithKbX > x ? -1 : 1;
		else
			moveMouseWithKbSpeedX = moveMouseWithKbSpeedX * 2 / 3;
		if( abs(moveMouseWithKbY - y) > SDL_ANDROID_sFakeWindowHeight / 12 )
			moveMouseWithKbSpeedY += moveMouseWithKbY > y ? -1 : 1;
		else
			moveMouseWithKbSpeedY = moveMouseWithKbSpeedY * 2 / 3;

		moveMouseWithKbX += moveMouseWithKbSpeedX;
		moveMouseWithKbY += moveMouseWithKbSpeedY;
		*/
		// Mouse follows touch instantly, when it's out of the snapping distance from mouse cursor
		if( abs(SDL_ANDROID_moveMouseWithKbX - x) >= SDL_ANDROID_sFakeWindowWidth / 10 ||
			abs(SDL_ANDROID_moveMouseWithKbY - y) >= SDL_ANDROID_sFakeWindowHeight / 10 ) // || SDL_GetTicks() - leftButtonDownTime > 600
		{
			SDL_ANDROID_moveMouseWithKbX = -1;
			SDL_ANDROID_moveMouseWithKbY = -1;
			SDL_ANDROID_moveMouseWithKbSpeedX = 0;
			SDL_ANDROID_moveMouseWithKbSpeedY = 0;
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "process mouse move PushMouseMotion x,y = %i, %i", x, y);

			SDL_ANDROID_MainThreadPushMouseMotion(x, y);
		}
		else {
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "process mouse move PushMouseMotion with kb x,y = %i, %i", SDL_ANDROID_moveMouseWithKbX, SDL_ANDROID_moveMouseWithKbY);
			SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_moveMouseWithKbX, SDL_ANDROID_moveMouseWithKbY);

		}
	}
	else
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "process mouse move PushMouseMotion no kb x,y = %i, %i", x, y);
		SDL_ANDROID_MainThreadPushMouseMotion(x, y);
	}

	if( rightClickMethod == RIGHT_CLICK_WITH_PRESSURE || leftClickMethod == LEFT_CLICK_WITH_PRESSURE )
	{

		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "process mouse move with pressure");

		int button = (leftClickMethod == LEFT_CLICK_WITH_PRESSURE) ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;
		int buttonState = ( force > maxForce || radius > maxRadius );
		if( button == SDL_BUTTON_RIGHT )
			SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_LEFT );
		SDL_ANDROID_MainThreadPushMouseButton( buttonState ? SDL_PRESSED : SDL_RELEASED, button );
	}

	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "process mouse move timeouts x,y = %i, %i", x, y);

	ProcessMouseMove_Timeouts(x, y);
}

static void ProcessMouseMultitouch( int action, int pointerId )
{
	if( pointerId == secondMousePointerId && (action == MOUSE_DOWN || action == MOUSE_UP) )
	{
		if( leftClickMethod == LEFT_CLICK_WITH_MULTITOUCH )
		{
			SDL_ANDROID_MainThreadPushMouseButton( (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT );
		}
		else if( rightClickMethod == RIGHT_CLICK_WITH_MULTITOUCH )
		{
			SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_LEFT );
			if( rightMouseButtonLongPress )
				SDL_ANDROID_MainThreadPushMouseButton( (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT );
			else
			{
				if( action == MOUSE_UP )
				{
					if( !multitouchGestureHappened )
					{
						SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_RIGHT );
						SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_RIGHT );
					}
				}
				multitouchGestureHappened = 0;
			}
		}
	}
	if( pointerId != firstMousePointerId && pointerId != secondMousePointerId && (action == MOUSE_DOWN || action == MOUSE_UP) )
		SDL_ANDROID_MainThreadPushMouseButton( (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_MIDDLE );
}

static void ProcessMouseHover( jint *xx, jint *yy, int action, int distance )
{
	int x = *xx, y = *yy;

	if( relativeMovement )
		return;

	if( !hoverJitterFilter )
	{
		if( action == MOUSE_HOVER )
			SDL_ANDROID_MainThreadPushMouseMotion(x, y);
	}
	else
	{
		if( action == MOUSE_HOVER )
		{
			if( hoverDeadzone )
			{
				if( abs(x - hoverX) < hoverDeadzone && abs(y - hoverY) < hoverDeadzone )
				{
					if( hoverTime == 0 )
						hoverTime = SDL_GetTicks();
					else if( !hoverMouseFreeze && SDL_GetTicks() > hoverTime + HOVER_FREEZE_TIME )
					{
						hoverMouseFreeze = 1;
						hoverX = x;
						hoverY = y;
					}
				}
				else
				{
					hoverTime = 0;
					hoverMouseFreeze = 0;
					hoverX = x;
					hoverY = y;
				}
			}
			if( !hoverMouseFreeze )
				SDL_ANDROID_MainThreadPushMouseMotion(x, y);
		}
		else if( hoverMouseFreeze && !(abs(x - hoverX) < hoverDeadzone && abs(y - hoverY) < hoverDeadzone) )
		{
			hoverMouseFreeze = 0;
			hoverTime = 0;
		}

		if( hoverMouseFreeze )
		{
			*xx = hoverX;
			*yy = hoverY;
		}
	}

	if( action == MOUSE_HOVER && distance < HOVER_DISTANCE_MAX * 3 / 4 )
		UpdateScreenUnderFingerRect(*xx, *yy);
	else
		SDL_ANDROID_ShowScreenUnderFingerRect.w = SDL_ANDROID_ShowScreenUnderFingerRect.h = 0; // This is reset later by ProcessMouseMove()

	if( distance == HOVER_REDRAW_SCREEN && *xx > 0 ) // Force screen redraw, to remove magnifier window when hovering finger leaves the screen
	{
		(*xx)--;
		SDL_ANDROID_MainThreadPushMouseMotion(*xx, *yy);
	}

#ifdef VIDEO_DEBUG
	SDL_ANDROID_VideoDebugRect.x = hoverX - hoverDeadzone;
	SDL_ANDROID_VideoDebugRect.y = hoverY - hoverDeadzone;
	SDL_ANDROID_VideoDebugRect.w = hoverDeadzone * 2;
	SDL_ANDROID_VideoDebugRect.h = hoverDeadzone * 2;
	memset(&SDL_ANDROID_VideoDebugRectColor, 0, sizeof(SDL_ANDROID_VideoDebugRectColor));
	SDL_ANDROID_VideoDebugRectColor.g = hoverMouseFreeze * 255;
	SDL_ANDROID_VideoDebugRectColor.r = SDL_ANDROID_VideoDebugRectColor.b = (SDL_GetTicks() - hoverTime) * 255 / HOVER_FREEZE_TIME;
#endif
}

static void AdjustMouseWithGyroscope( jint *xx, jint *yy )
{
	if( !moveMouseWithGyroscope || relativeMovement ||
		SDL_ANDROID_moveMouseWithKbX >= 0 || hardwareMouseDetected == MOUSE_HW_INPUT_MOUSE )
		return;

	static int oldX = 0, oldY = 0, count = 0;
	count += abs(*xx - oldX) + abs(*yy - oldY);
	oldX = *xx;
	oldY = *yy;

	*xx += moveMouseWithGyroscopeX;
	*yy += moveMouseWithGyroscopeY;

	// Decrease the adjustment values slowly, when we move mouse
	int decrease = count / 10;
	count %= 10;

#define SIGN(x) ((x > 0) - (x < 0)) // -1, 0, or 1, depending on the value sign

	int signX = SIGN(moveMouseWithGyroscopeX);
	moveMouseWithGyroscopeX -= signX * decrease;
	if( signX != SIGN(moveMouseWithGyroscopeX) )
		moveMouseWithGyroscopeX = 0;

	int signY = SIGN(moveMouseWithGyroscopeY);
	moveMouseWithGyroscopeY -= signY * decrease;
	if( signY != SIGN(moveMouseWithGyroscopeY) )
		moveMouseWithGyroscopeY = 0;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMouseEvent) ( JNIEnv*  env, jobject  thiz, jint x, jint y, jint action)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	if( !SDL_GetFocusWindow() )
		return;
#else
	if( !SDL_CurrentVideoSurface )
		return;
#endif

	float refX = 640.0;
	float refY = 480.0;

	float warpX = (float)x / refX;
	float warpY = (float)y / refY;

	x = warpX * SDL_ANDROID_sFakeWindowWidth;
	y = warpY * SDL_ANDROID_sFakeWindowHeight;

	ProcessMouseRelativeMovement( &x, &y, action );
	if( action == MOUSE_UP )
		ProcessMouseUp( x, y );
	if( action == MOUSE_DOWN )
		action = ProcessMouseDown( x, y ); // May change action to MOUSE_MOVE
	if( action == MOUSE_MOVE )
		ProcessMouseMove( x, y, 0, 0 );
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMotionEvent) ( JNIEnv*  env, jobject  thiz, jint x, jint y, jint action, jint pointerId, jint force, jint radius )
{
	int i;
#if SDL_VERSION_ATLEAST(1,3,0)
	if( !SDL_GetFocusWindow() )
		return;
#else
	if( !SDL_CurrentVideoSurface )
		return;
#endif

	pointerId = BumpPointerId( pointerId );

	if( ProcessTouchscreenKeyboard( x, y, action, pointerId ) )
		return;
	
	AssignNewTouchPointers( action, pointerId );
	
	AdjustTouchScreenCalibration( &x, &y );

	ProcessMultitouchGesture( x, y, action, pointerId );

	SendMultitouchEvents( x, y, action, pointerId, force, radius );

	if( !SDL_ANDROID_isMouseUsed )
		return;

	AdjustMouseWithGyroscope( &x, &y );

	ProcessMouseHover( &x, &y, action, force );

	if( pointerId == firstMousePointerId )
	{
		ProcessMouseRelativeMovement( &x, &y, action );
		if( action == MOUSE_UP )
			ProcessMouseUp( x, y );
		if( action == MOUSE_DOWN )
			action = ProcessMouseDown( x, y ); // May change action to MOUSE_MOVE
		if( action == MOUSE_MOVE )
			ProcessMouseMove( x, y, force, radius );
	}
	ProcessMouseMultitouch( action, pointerId );

	ClearOldTouchPointers( action, pointerId );
}

static void ProcessDeferredMouseTap()
{
	if( deferredMouseTap > 0 )
	{
		deferredMouseTap--;
		if( deferredMouseTap <= 0 )
		{
#if SDL_VERSION_ATLEAST(1,3,0)
			SDL_Window * window = SDL_GetFocusWindow();
			if( !window )
				return;
#define SDL_ANDROID_sFakeWindowWidth window->w
#define SDL_ANDROID_sFakeWindowHeight window->h
#endif
			if( SDL_ANDROID_currentMouseX + 1 < SDL_ANDROID_sFakeWindowWidth )
				SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_currentMouseX + 1, SDL_ANDROID_currentMouseY);
			SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_LEFT );
			moveMouseWithGyroscopeX = 0;
			moveMouseWithGyroscopeY = 0;
		}
		else if( SDL_ANDROID_currentMouseX > 0 ) // Force application to redraw, and call SDL_Flip()
			SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_currentMouseX - 1, SDL_ANDROID_currentMouseY);
	}
}

static void ProcessMoveMouseWithGyroscope(float gx, float gy, float gz)
{
	if( hardwareMouseDetected == MOUSE_HW_INPUT_MOUSE ) // We don't need all that stuff with a proper precise input device
		return;

	gx += gz; // Ignore Z?
	gx *= -moveMouseWithGyroscopeSpeed;
	gy *= moveMouseWithGyroscopeSpeed; // Screen has Y coordinate inverted

	//gx *= 10; // debug
	//gy *= 10; // debug

	static float subpixelX = 0.0f, subpixelY = 0.0f;

	gx += subpixelX;
	gy += subpixelY;
	float t = truncf(gx);
	subpixelX = gx - t;
	gx = t;
	t = truncf(gy);
	subpixelY = gy - t;
	gy = t;

	// TODO: mutex here?
	// If race condition happens, mouse will jump at random across the screen. Nothing serious.

	if( SDL_ANDROID_moveMouseWithKbX >= 0 )
	{
		SDL_ANDROID_moveMouseWithKbX += gx;
		SDL_ANDROID_moveMouseWithKbY += gy;
		SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_moveMouseWithKbX, SDL_ANDROID_moveMouseWithKbY);
		return;
	}

	if( relativeMovement )
	{
		SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_currentMouseX + gx, SDL_ANDROID_currentMouseY + gy);
		return;
	}

	// Mouse coords before gyroscope was applied
	int actualTouchX = SDL_ANDROID_currentMouseX - moveMouseWithGyroscopeX;
	int actualTouchY = SDL_ANDROID_currentMouseY - moveMouseWithGyroscopeY;

	moveMouseWithGyroscopeX += gx;
	moveMouseWithGyroscopeY += gy;

	SDL_ANDROID_MainThreadPushMouseMotion(actualTouchX + moveMouseWithGyroscopeX, actualTouchY + moveMouseWithGyroscopeY);
}

void SDL_ANDROID_WarpMouse(int x, int y)
{
	if(!relativeMovement)
	{
		//SDL_ANDROID_MainThreadPushMouseMotion(x, y);
	}
	else
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_ANDROID_WarpMouse(): %dx%d rel %dx%d old %dx%d", x, y, relativeMovementX, relativeMovementY, SDL_ANDROID_currentMouseX, SDL_ANDROID_currentMouseY);
		relativeMovementX -= SDL_ANDROID_currentMouseX-x;
		relativeMovementY -= SDL_ANDROID_currentMouseY-y;
		SDL_ANDROID_MainThreadPushMouseMotion(x, y);
	}
};

static int js2_mapped = 0;
static int processAndroidTrackball(int key, int action);

JNIEXPORT jint JNICALL
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeKey) ( JNIEnv*  env, jobject thiz, jint key, jint action, jint unicode )
{
#if SDL_VERSION_ATLEAST(1,3,0)
#else
	if( !SDL_CurrentVideoSurface )
		return 1;
#endif

	if( isTrackballUsed )
		if( processAndroidTrackball(key, action) )
			return 1;
	if( key == rightClickKey && rightClickMethod == RIGHT_CLICK_WITH_KEY )
	{
		SDL_ANDROID_MainThreadPushMouseButton( action ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT );
		return 1;
	}
	if( (key == leftClickKey && leftClickMethod == LEFT_CLICK_WITH_KEY) || (clickMouseWithDpadCenter && key == KEYCODE_DPAD_CENTER) )
	{
		SDL_ANDROID_MainThreadPushMouseButton( action ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT );
		return 1;
	}

	if (!js2_mapped) {
		SDL_android_keymap[KEYCODE_JS2_UP]       = SDL_KEY(JS2_UP);
		SDL_android_keymap[KEYCODE_JS2_DOWN]     = SDL_KEY(JS2_DOWN);
		SDL_android_keymap[KEYCODE_JS2_LEFT]     = SDL_KEY(JS2_LEFT);
		SDL_android_keymap[KEYCODE_JS2_RIGHT]    = SDL_KEY(JS2_RIGHT);
		SDL_android_keymap[KEYCODE_JS2_BUTTON_1] = SDL_KEY(JS2_BUTTON_1);
		SDL_android_keymap[KEYCODE_JS2_BUTTON_2] = SDL_KEY(JS2_BUTTON_2);
		js2_mapped = 1;
	}

	__android_log_print(ANDROID_LOG_INFO, "libSDL","nativeKey %d translated %d unicode %d", key, TranslateKey(key), unicode);

	if( TranslateKey(key) == SDLK_NO_REMAP || (TranslateKey(key) == SDLK_UNKNOWN && (unicode & 0xFF80) == 0) )
		return 0;

	if( TranslateKey(key) != SDLK_UNKNOWN )
		unicode = 0;

	SDL_ANDROID_MainThreadPushKeyboardKey( action ? SDL_PRESSED : SDL_RELEASED, TranslateKey(key), unicode );
	return 1;
}

static char * textInputBuffer = NULL;
int textInputBufferLen = 0;
int textInputBufferPos = 0;

void SDL_ANDROID_TextInputInit(char * buffer, int len)
{
	textInputBuffer = buffer;
	textInputBufferLen = len;
	textInputBufferPos = 0;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(DemoRenderer_nativeTextInput) ( JNIEnv*  env, jobject thiz, jint ascii, jint unicode )
{
	if( ascii == 10 )
		ascii = SDLK_RETURN;

	if( !textInputBuffer )
		SDL_ANDROID_MainThreadPushText(ascii, unicode);
	else
	{
		if( textInputBufferPos < textInputBufferLen + 4 && ascii != SDLK_RETURN && ascii != '\r' && ascii != '\n' )
		{
			textInputBufferPos += UnicodeToUtf8(unicode, textInputBuffer + textInputBufferPos);
		}
	}
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(DemoRenderer_nativeTextInputFinished) ( JNIEnv*  env, jobject thiz )
{
	if (textInputBufferPos == 0 && textInputBuffer) // No text input - clear buffer
		textInputBuffer[0] = 0;
	textInputBuffer = NULL;
	SDL_ANDROID_TextInputFinished = 1;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(AccelerometerReader_nativeAccelerometer) ( JNIEnv*  env, jobject  thiz, jfloat accPosX, jfloat accPosY, jfloat accPosZ )
{
#if SDL_VERSION_ATLEAST(1,3,0)
#else
	if( !SDL_CurrentVideoSurface )
		return;
#endif
	// Calculate two angles from three coordinates
	float normal = sqrt(accPosX*accPosX+accPosY*accPosY+accPosZ*accPosZ);
	if(normal <= 0.0000001f)
		normal = 0.00001f;
	
	SDL_ANDROID_MainThreadPushJoystickAxis(JOY_ACCELGYRO, 0, NORMALIZE_FLOAT_32767(accPosX/normal));
	SDL_ANDROID_MainThreadPushJoystickAxis(JOY_ACCELGYRO, 1, NORMALIZE_FLOAT_32767(-accPosY/normal));

	// Also send raw coordinates
	SDL_ANDROID_MainThreadPushJoystickAxis(JOY_ACCELGYRO, 5, fminf(32767.0f, fmaxf(-32767.0f, accPosX*1000.0f))); // Do not consider wraparound case
	SDL_ANDROID_MainThreadPushJoystickAxis(JOY_ACCELGYRO, 6, fminf(32767.0f, fmaxf(-32767.0f, accPosY*1000.0f)));
	SDL_ANDROID_MainThreadPushJoystickAxis(JOY_ACCELGYRO, 7, fminf(32767.0f, fmaxf(-32767.0f, accPosZ*1000.0f)));
}


JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(AccelerometerReader_nativeGyroscope) ( JNIEnv*  env, jobject thiz, jfloat X, jfloat Y, jfloat Z )
{
#if SDL_VERSION_ATLEAST(1,3,0)
#else
	if( !SDL_CurrentVideoSurface )
		return;
#endif

	if( moveMouseWithGyroscope )
	{
		ProcessMoveMouseWithGyroscope(X, Y, Z);
		return;
	}

	X *= 0.25f;
	Y *= 0.25f;
	Z *= 0.25f;
	while ( X != 0.0f || Y != 0.0f || Z != 0.0f )
	{
		float dx = ( X > 1.0f ? 1.0f : ( X < -1.0f ? -1.0f : X ) );
		float dy = ( Y > 1.0f ? 1.0f : ( Y < -1.0f ? -1.0f : Y ) );
		float dz = ( Z > 1.0f ? 1.0f : ( Z < -1.0f ? -1.0f : Z ) );

		X -= dx;
		Y -= dy;
		Z -= dz;

		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_ACCELGYRO, 2, NORMALIZE_FLOAT_32767(dx));
		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_ACCELGYRO, 3, NORMALIZE_FLOAT_32767(dy));
		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_ACCELGYRO, 4, NORMALIZE_FLOAT_32767(dz));
		//if( fabs(dx) >= 1.0f || fabs(dy) >= 1.0f || fabs(dz) >= 1.0f ) __android_log_print(ANDROID_LOG_INFO, "libSDL", "nativeGyroscope(): sending several events, this eats CPU!");
	}
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetTrackballUsed) ( JNIEnv*  env, jobject thiz)
{
	isTrackballUsed = 1;
}

static int getClickTimeout(int v)
{
	switch(v)
	{
		case 0: return 200;
		case 1: return 300;
		case 2: return 400;
		case 3: return 700;
		case 4: return 1000;
	}
	return 1000;
}

// Mwahaha overkill!
JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetMouseUsed) (JNIEnv* env, jobject thiz,
		jint RightClickMethod, jint ShowScreenUnderFinger, jint LeftClickMethod,
		jint MoveMouseWithJoystick, jint ClickMouseWithDpad,
		jint MaxForce, jint MaxRadius,
		jint MoveMouseWithJoystickSpeed, jint MoveMouseWithJoystickAccel,
		jint LeftClickKeycode, jint RightClickKeycode,
		jint LeftClickTimeout, jint RightClickTimeout,
		jint RelativeMovement, jint RelativeMovementSpeed, jint RelativeMovementAccel,
		jint ShowMouseCursor, jint HoverJitterFilter, jint RightMouseButtonLongPress,
		jint MoveMouseWithGyroscope, jint MoveMouseWithGyroscopeSpeed)
{
	SDL_ANDROID_isMouseUsed = 1;
	rightClickMethod = RightClickMethod;
	SDL_ANDROID_ShowScreenUnderFinger = ShowScreenUnderFinger;
	SDL_ANDROID_moveMouseWithArrowKeys = MoveMouseWithJoystick;
	clickMouseWithDpadCenter = ClickMouseWithDpad;
	leftClickMethod = LeftClickMethod;
	maxForce = MaxForce;
	maxRadius = MaxRadius;
	SDL_ANDROID_moveMouseWithKbSpeed = MoveMouseWithJoystickSpeed + 1;
	SDL_ANDROID_moveMouseWithKbAccel = MoveMouseWithJoystickAccel;
	leftClickKey = LeftClickKeycode;
	rightClickKey = RightClickKeycode;
	leftClickTimeout = getClickTimeout(LeftClickTimeout);
	rightClickTimeout = getClickTimeout(RightClickTimeout);
	relativeMovement = RelativeMovement;
	relativeMovementSpeed = RelativeMovementSpeed;
	relativeMovementAccel = RelativeMovementAccel;
	SDL_ANDROID_ShowMouseCursor = ShowMouseCursor;
	hoverJitterFilter = HoverJitterFilter;
	rightMouseButtonLongPress = RightMouseButtonLongPress;
	moveMouseWithGyroscope = MoveMouseWithGyroscope;
	moveMouseWithGyroscopeSpeed = 0.0625f * MoveMouseWithGyroscopeSpeed * MoveMouseWithGyroscopeSpeed + 0.125f * MoveMouseWithGyroscopeSpeed + 0.5f; // Scale value from 0.5 to 2, with 1 at the middle
	moveMouseWithGyroscopeSpeed *= 5.0f;
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "moveMouseWithGyroscopeSpeed %d = %f", MoveMouseWithGyroscopeSpeed, moveMouseWithGyroscopeSpeed);
	if( mouseClickTimeoutThreadId == 0 && (
		leftClickMethod == LEFT_CLICK_WITH_TIMEOUT ||
		leftClickMethod == LEFT_CLICK_WITH_TAP_OR_TIMEOUT ||
		rightClickMethod == RIGHT_CLICK_WITH_TIMEOUT ) )
	{
		sem_init(&mouseClickTimeoutSemaphore, 0, 0);
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&mouseClickTimeoutThreadId, &attr, mouseClickTimeoutThread, NULL);
		pthread_attr_destroy(&attr);
	}
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeHardwareMouseDetected) (JNIEnv* env, jobject thiz, int detected)
{
	if( !SDL_ANDROID_isMouseUsed )
		return;

	static struct {
		int leftClickMethod;
		int ShowScreenUnderFinger;
		int leftClickTimeout;
		int relativeMovement;
		int ShowMouseCursor;
	} cfg = { 0 };

	if( hardwareMouseDetected != detected )
	{
		hardwareMouseDetected = detected;
		if(detected)
		{
			cfg.leftClickMethod = leftClickMethod;
			cfg.ShowScreenUnderFinger = SDL_ANDROID_ShowScreenUnderFinger;
			cfg.leftClickTimeout = leftClickTimeout;
			cfg.relativeMovement = relativeMovement;
			cfg.ShowMouseCursor = SDL_ANDROID_ShowMouseCursor;
			
			leftClickMethod = LEFT_CLICK_NORMAL;
			SDL_ANDROID_ShowScreenUnderFinger = 0;
			leftClickTimeout = 0;
			relativeMovement = 0;
			SDL_ANDROID_ShowMouseCursor = 0;
		}
		else
		{
			leftClickMethod = cfg.leftClickMethod;
			SDL_ANDROID_ShowScreenUnderFinger = cfg.ShowScreenUnderFinger;
			leftClickTimeout = cfg.leftClickTimeout;
			relativeMovement = cfg.relativeMovement;
			SDL_ANDROID_ShowMouseCursor = cfg.ShowMouseCursor;
		}
	}
	SDL_ANDROID_SetHoverDeadzone();
}

void SDL_ANDROID_SetHoverDeadzone()
{
	hoverDeadzone = (hardwareMouseDetected == MOUSE_HW_INPUT_STYLUS) ?
					SDL_ANDROID_sFakeWindowHeight * (70 / (atoi(getenv("DISPLAY_HEIGHT_MM")) > 0 ? atoi(getenv("DISPLAY_HEIGHT_MM")) : 70)) / DEADZONE_HOVER_STYLUS :
					(hardwareMouseDetected == MOUSE_HW_INPUT_FINGER) ?
					SDL_ANDROID_sFakeWindowHeight * (70 / (atoi(getenv("DISPLAY_HEIGHT_MM")) > 0 ? atoi(getenv("DISPLAY_HEIGHT_MM")) : 70)) / DEADZONE_HOVER_FINGER : 0;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMouseButtonsPressed) (JNIEnv* env, jobject thiz, jint buttonId, jint pressedState)
{
	int btn = SDL_BUTTON_LEFT;
	if( !SDL_ANDROID_isMouseUsed )
		return;

	switch(buttonId)
	{
		case MOUSE_HW_BUTTON_LEFT:
			btn = SDL_BUTTON_LEFT;
			break;
		case MOUSE_HW_BUTTON_RIGHT:
			btn = SDL_BUTTON_RIGHT;
			break;
		case MOUSE_HW_BUTTON_MIDDLE:
			btn = SDL_BUTTON_MIDDLE;
			break;
		case MOUSE_HW_BUTTON_BACK:
			btn = SDL_BUTTON_X1;
			break;
		case MOUSE_HW_BUTTON_FORWARD:
			btn = SDL_BUTTON_X2;
			break;
	}
	SDL_ANDROID_MainThreadPushMouseButton( pressedState ? SDL_PRESSED : SDL_RELEASED, btn );
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMouseWheel) (JNIEnv* env, jobject thiz, jint scrollX, jint scrollY)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_ANDROID_MainThreadPushMouseWheel( scrollX, scrollY );
#else
	// TODO: direction might get inverted
	for( ; scrollX > 0; scrollX-- )
	{
		if( !SDL_ANDROID_isMouseUsed )
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_RIGHT), 0 );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_RIGHT), 0 );
		}
		else
		{
			SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_X2 );
			SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_X2 );
		}
	}
	for( ; scrollX < 0; scrollX++ )
	{
		if( !SDL_ANDROID_isMouseUsed )
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_LEFT), 0 );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_LEFT), 0 );
		}
		else
		{
			SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_X1 );
			SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_X1 );
		}
	}
	for( ; scrollY > 0; scrollY-- )
	{
		if( !SDL_ANDROID_isMouseUsed )
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_UP), 0 );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_UP), 0 );
		}
		else
		{
			SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_WHEELUP );
			SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_WHEELUP );
		}
	}
	for( ; scrollY < 0; scrollY++ )
	{
		if( !SDL_ANDROID_isMouseUsed )
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_DOWN), 0 );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_DOWN), 0 );
		}
		else
		{
			SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_WHEELDOWN );
			SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_WHEELDOWN );
		}
	}
#endif
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetJoystickUsed) (JNIEnv* env, jobject thiz, jint amount)
{
	SDL_ANDROID_joysticksAmount = amount;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetAccelerometerUsed) (JNIEnv* env, jobject thiz)
{
	SDL_ANDROID_isAccelerometerUsed = 1;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetMultitouchUsed) ( JNIEnv*  env, jobject thiz)
{
	isMultitouchUsed = 1;
}


static float dx = 0.04, dy = 0.1, dz = 0.1, joystickSensitivity = 400.0f; // For accelerometer
enum { ACCELEROMETER_CENTER_FLOATING, ACCELEROMETER_CENTER_FIXED_START, ACCELEROMETER_CENTER_FIXED_HORIZ };
static int accelerometerCenterPos = ACCELEROMETER_CENTER_FLOATING;

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetAccelerometerSettings) ( JNIEnv*  env, jobject thiz, jint sensitivity, jint centerPos)
{
	dx = 0.04; dy = 0.08; dz = 0.08; joystickSensitivity = 32767.0f * 3.0f; // Fast sensitivity
	if( sensitivity == 1 )
	{
		dx = 0.1; dy = 0.15; dz = 0.15; joystickSensitivity = 32767.0f * 2.0f; // Medium sensitivity
	}
	if( sensitivity == 2 )
	{
		dx = 0.2; dy = 0.25; dz = 0.25; joystickSensitivity = 32767.0f; // Slow sensitivity
	}
	accelerometerCenterPos = centerPos;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetTrackballDampening) ( JNIEnv*  env, jobject thiz, jint value)
{
	TrackballDampening = (value * 200);
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeGamepadAnalogJoystickInput) (JNIEnv* env, jobject thiz,
	jfloat stick1x, jfloat stick1y, jfloat stick2x, jfloat stick2y, jfloat rtrigger, jfloat ltrigger, jint port)
{
	if( SDL_ANDROID_CurrentJoysticks[JOY_GAMEPAD1] )
	{
		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_GAMEPAD1, 0, NORMALIZE_FLOAT_32767(stick1x));
		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_GAMEPAD1, 1, NORMALIZE_FLOAT_32767(stick1y));
		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_GAMEPAD1, 2, NORMALIZE_FLOAT_32767(stick2x));
		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_GAMEPAD1, 3, NORMALIZE_FLOAT_32767(stick2y));
		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_GAMEPAD1, 4, NORMALIZE_FLOAT_32767(ltrigger));
		SDL_ANDROID_MainThreadPushJoystickAxis(JOY_GAMEPAD1, 5, NORMALIZE_FLOAT_32767(rtrigger));
	}
	else
	{

		int sdl_left  = port == 0 ? SDL_KEY(LEFT)  : SDL_KEY(JS2_LEFT);
		int sdl_right = port == 0 ? SDL_KEY(RIGHT) : SDL_KEY(JS2_RIGHT);
		int sdl_up    = port == 0 ? SDL_KEY(UP)    : SDL_KEY(JS2_UP);
		int sdl_down  = port == 0 ? SDL_KEY(DOWN)  : SDL_KEY(JS2_DOWN);

		// Translate to up/down/left/right
		if( stick1x < -0.5f )
		{
			if( !SDL_GetKeyboardState(NULL)[sdl_left] )
				SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, sdl_left, 0 );
		}
		else
		{
			if( SDL_GetKeyboardState(NULL)[sdl_left] )
				SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, sdl_left, 0 );
		}
		if( stick1x > 0.5f )
		{
			if( !SDL_GetKeyboardState(NULL)[sdl_right] )
				SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, sdl_right, 0 );
		}
		else
		{
			if( SDL_GetKeyboardState(NULL)[sdl_right] )
				SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, sdl_right, 0 );
		}
		if( stick1y < -0.5f )
		{
			if( !SDL_GetKeyboardState(NULL)[sdl_up] )
				SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, sdl_up, 0 );
		}
		else
		{
			if( SDL_GetKeyboardState(NULL)[sdl_up] )
				SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, sdl_up, 0 );
		}
		if( stick1y > 0.5f )
		{
			if( !SDL_GetKeyboardState(NULL)[sdl_down] )
				SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, sdl_down, 0 );
		}
		else
		{
			if( SDL_GetKeyboardState(NULL)[sdl_down] )
				SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, sdl_down, 0 );
		}
	}
}

static int leftPressed = 0, rightPressed = 0, upPressed = 0, downPressed = 0;

int processAndroidTrackball(int key, int action)
{
	SDL_keysym keysym;
	
	if( ! action && (
		key == KEYCODE_DPAD_UP ||
		key == KEYCODE_DPAD_DOWN ||
		key == KEYCODE_DPAD_LEFT ||
		key == KEYCODE_DPAD_RIGHT ) )
		return 1;
	lastTrackballAction = SDL_GetTicks();

	if( key == KEYCODE_DPAD_UP )
	{
		if( downPressed )
		{
			downPressed = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_DOWN), 0 );
			return 1;
		}
		if( !upPressed )
		{
			upPressed = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key), 0 );
		}
		else
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(key), 0 );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key), 0 );
		}
		return 1;
	}

	if( key == KEYCODE_DPAD_DOWN )
	{
		if( upPressed )
		{
			upPressed = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_UP), 0 );
			return 1;
		}
		if( !upPressed )
		{
			downPressed = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key), 0 );
		}
		else
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(key), 0 );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key), 0 );
		}
		return 1;
	}

	if( key == KEYCODE_DPAD_LEFT )
	{
		if( rightPressed )
		{
			rightPressed = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_RIGHT), 0 );
			return 1;
		}
		if( !leftPressed )
		{
			leftPressed = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key), 0 );
		}
		else
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(key), 0 );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key), 0 );
		}
		return 1;
	}

	if( key == KEYCODE_DPAD_RIGHT )
	{
		if( leftPressed )
		{
			leftPressed = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_LEFT), 0 );
			return 1;
		}
		if( !rightPressed )
		{
			rightPressed = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key), 0 );
		}
		else
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(key), 0 );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key), 0 );
		}
		return 1;
	}

	return 0;
}

void SDL_ANDROID_processAndroidTrackballDampening()
{
	if( !TrackballDampening )
		return;
	if( SDL_GetTicks() > TrackballDampening + lastTrackballAction  )
	{
		if( upPressed )
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_UP), 0 );
		if( downPressed )
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_DOWN), 0 );
		if( leftPressed )
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_LEFT), 0 );
		if( rightPressed )
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_RIGHT), 0 );
		upPressed = 0;
		downPressed = 0;
		leftPressed = 0;
		rightPressed = 0;
	}
}

int SDL_SYS_JoystickInit(void)
{
	SDL_numjoysticks = JOY_GAMEPAD4 + 1;

	return(SDL_numjoysticks);
}

/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
	if( index == JOY_TOUCHSCREEN )
		return "Multitouch and on-screen joystick";
	if( index == JOY_ACCELGYRO )
		return "Accelerometer/gyroscope";
	if( index == JOY_GAMEPAD1 )
		return "Gamepad 1";
	if( index == JOY_GAMEPAD2 )
		return "Gamepad 2";
	if( index == JOY_GAMEPAD3 )
		return "Gamepad 3";
	if( index == JOY_GAMEPAD4 )
		return "Gamepad 4";
	return "This joystick does not exist, check your code";
}

int SDL_SYS_JoystickOpen(SDL_Joystick *joystick)
{
	joystick->naxes = 0;
	joystick->nbuttons = 0;
	joystick->nhats = 0;
	joystick->nballs = 0;
	if( joystick->index == JOY_TOUCHSCREEN )
	{
		joystick->naxes = 6 + MAX_MULTITOUCH_POINTERS; // Three joysticks plus touch pressure/size
		joystick->nbuttons = MAX_MULTITOUCH_POINTERS;
		joystick->nballs = MAX_MULTITOUCH_POINTERS;
	}
	if( joystick->index == JOY_ACCELGYRO )
	{
		joystick->naxes = 8; // Normalized accelerometer = axes 0-1, gyroscope = axes 2-4, raw accelerometer = axes 5-7
		if( !moveMouseWithGyroscope )
			SDL_ANDROID_CallJavaStartAccelerometerGyroscope(1);
	}
	if( joystick->index >= JOY_GAMEPAD1 && joystick->index <= JOY_GAMEPAD4 )
	{
		joystick->naxes = 8; // Two analog stick + two trigger buttons + Ouya touchpad
	}
	SDL_ANDROID_CurrentJoysticks[joystick->index] = joystick;
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Opened joystick %d axes %d buttons %d balls %d", joystick->index, joystick->naxes, joystick->nbuttons, joystick->nballs);
	return(0);
}

void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{
	return;
}

/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
	SDL_ANDROID_CurrentJoysticks[joystick->index] = NULL;
	if( joystick->index == JOY_ACCELGYRO && !moveMouseWithGyroscope )
		SDL_ANDROID_CallJavaStartAccelerometerGyroscope(0);
	return;
}

/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void)
{
	int i;
	SDL_ANDROID_CurrentJoysticks[0] = NULL;
	return;
}

Uint32 lastMoveMouseWithKeyboardUpdate = 0;

void SDL_ANDROID_processMoveMouseWithKeyboard()
{
	if( ! SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded )
		return;

	Uint32 ticks = SDL_GetTicks();

	if( ticks - lastMoveMouseWithKeyboardUpdate < 20 ) // Update at 50 FPS max, or it will not work properlty on very fast devices
		return;

	lastMoveMouseWithKeyboardUpdate = ticks;

	SDL_ANDROID_moveMouseWithKbSpeedX += SDL_ANDROID_moveMouseWithKbAccelX;
	SDL_ANDROID_moveMouseWithKbSpeedY += SDL_ANDROID_moveMouseWithKbAccelY;

	SDL_ANDROID_moveMouseWithKbX += SDL_ANDROID_moveMouseWithKbSpeedX;
	SDL_ANDROID_moveMouseWithKbY += SDL_ANDROID_moveMouseWithKbSpeedY;
	SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_moveMouseWithKbX, SDL_ANDROID_moveMouseWithKbY);
};

extern void SDL_ANDROID_ProcessDeferredEvents()
{
	SDL_ANDROID_DeferredTextInput();
	ProcessDeferredMouseTap();
};

void ANDROID_InitOSKeymap()
{
#if (SDL_VERSION_ATLEAST(1,3,0))
  SDLKey defaultKeymap[SDL_NUM_SCANCODES];
  SDL_GetDefaultKeymap(defaultKeymap);
  SDL_SetKeymap(0, defaultKeymap, SDL_NUM_SCANCODES);

  SDL_Touch touch;
  memset( &touch, 0, sizeof(touch) );
  touch.x_min = touch.y_min = touch.pressure_min = 0.0f;
  touch.pressure_max = 1000000;
  touch.x_max = SDL_ANDROID_sWindowWidth;
  touch.y_max = SDL_ANDROID_sWindowHeight;

  // These constants are hardcoded inside SDL_touch.c, which makes no sense for me.
  touch.xres = touch.yres = 32768;
  touch.native_xres = touch.native_yres = 32768.0f;

  touch.pressureres = 1;
  touch.native_pressureres = 1.0f;
  touch.id = 0;

  SDL_AddTouch(&touch, "Android touch screen");
#endif
}

JNIEXPORT jint JNICALL 
JAVA_EXPORT_NAME(Settings_nativeGetKeymapKey) (JNIEnv* env, jobject thiz, jint code)
{
	if( code < 0 || code > KEYCODE_LAST )
		return SDL_KEY(UNKNOWN);
	return SDL_android_keymap[code];
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetKeymapKey) (JNIEnv* env, jobject thiz, jint javakey, jint key)
{
	if( javakey < 0 || javakey > KEYCODE_LAST )
		return;
	SDL_android_keymap[javakey] = key;
}

JNIEXPORT jint JNICALL
JAVA_EXPORT_NAME(Settings_nativeGetKeymapKeyMultitouchGesture) (JNIEnv* env, jobject thiz, jint keynum)
{
	if( keynum < 0 || keynum >= MAX_MULTITOUCH_GESTURES )
		return SDL_KEY(UNKNOWN);
	return multitouchGestureKeycode[keynum];
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetKeymapKeyMultitouchGesture) (JNIEnv* env, jobject thiz, jint keynum, jint keycode)
{
	if( keynum < 0 || keynum >= MAX_MULTITOUCH_GESTURES )
		return;
	multitouchGestureKeycode[keynum] = keycode;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetMultitouchGestureSensitivity) (JNIEnv* env, jobject thiz, jint sensitivity)
{
	multitouchGestureSensitivity = sensitivity;
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetTouchscreenCalibration) (JNIEnv* env, jobject thiz, jint x1, jint y1, jint x2, jint y2)
{
	SDL_ANDROID_TouchscreenCalibrationX = x1;
	SDL_ANDROID_TouchscreenCalibrationY = y1;
	SDL_ANDROID_TouchscreenCalibrationWidth = x2 - x1;
	SDL_ANDROID_TouchscreenCalibrationHeight = y2 - y1;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeInitKeymap) ( JNIEnv*  env, jobject thiz )
{
	SDL_android_init_keymap(SDL_android_keymap);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_android_keymap button 1 is %d [%d]", SDL_android_keymap[KEYCODE_JS2_BUTTON_1], KEYCODE_JS2_BUTTON_1);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_android_keymap button 2 is %d [%d]", SDL_android_keymap[KEYCODE_JS2_BUTTON_2], KEYCODE_JS2_BUTTON_2);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Translated button 1 is %d", TranslateKey(KEYCODE_JS2_BUTTON_1));
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Translated button 2 is %d", TranslateKey(KEYCODE_JS2_BUTTON_2));

}

void SDL_ANDROID_SetGamepadKeymap(int A, int B, int X, int Y, int L1, int R1, int L2, int R2, int LThumb, int RThumb)
{
        /*
        Arguments are SDL keycodes. Use the SDLK_ constants.
        Pass zero to leave a button mapping untouched.

        On OUYA: O = A
                 U = X
                 Y = Y
                 A = B
                 C and Z do not exist, they also do not exist on any gamepad I've seen (PS3/XBox/SHIELD)
        */
	if (A) SDL_android_keymap[KEYCODE_BUTTON_A] = A;
	if (B) SDL_android_keymap[KEYCODE_BUTTON_B] = B;
	//if (C) SDL_android_keymap[KEYCODE_BUTTON_C] = C;
	if (X) SDL_android_keymap[KEYCODE_BUTTON_X] = X;
	if (Y) SDL_android_keymap[KEYCODE_BUTTON_Y] = Y;
	//if (Z) SDL_android_keymap[KEYCODE_BUTTON_Z] = Z;
	if (L1) SDL_android_keymap[KEYCODE_BUTTON_L1] = L1;
	if (R1) SDL_android_keymap[KEYCODE_BUTTON_R1] = R1;
	if (L2) SDL_android_keymap[KEYCODE_BUTTON_L2] = L2;
	if (R2) SDL_android_keymap[KEYCODE_BUTTON_R2] = R2;
	if (LThumb) SDL_android_keymap[KEYCODE_BUTTON_THUMBL] = LThumb;
	if (RThumb) SDL_android_keymap[KEYCODE_BUTTON_THUMBR] = RThumb;
}

void *mouseClickTimeoutThread (void * unused)
{
	struct timespec ts;
	while(1)
	{
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += mouseClickTimeout / 1000;
		ts.tv_nsec += (mouseClickTimeout % 1000) * 1000000;
		if( ts.tv_nsec >= 1000000000 )
		{
			ts.tv_sec++;
			ts.tv_nsec %= 1000000000;
		}
		sem_timedwait(&mouseClickTimeoutSemaphore, &ts);
		ProcessMouseMove_Timeouts(SDL_ANDROID_currentMouseX, SDL_ANDROID_currentMouseY);
	}
	return NULL;
}
