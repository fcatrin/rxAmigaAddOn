
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <jni.h>
#include <android/log.h>
#include "SDL_version.h"
#include "SDL_thread.h"
#include "SDL_main.h"

#if SDL_VERSION_ATLEAST(2,0,0)
#else
#include "SDL_android.h"
#endif

/* JNI-C wrapper stuff */

#ifdef __cplusplus
#define C_LINKAGE "C"
#else
#define C_LINKAGE
#endif


#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

static int argc = 0;
static char ** argv = NULL;

static JNIEnv*  static_env = NULL;
static jobject static_thiz = NULL;

JNIEnv* SDL_ANDROID_JniEnv()
{
	return static_env;
}
jobject SDL_ANDROID_JniVideoObject()
{
	return static_thiz;
}

void redirectStdout()
{
	freopen( "stdout.log", "w", stdout );
	dup2( fileno(stdout), fileno(stderr) );
}

#if SDL_VERSION_ATLEAST(1,3,0)
#else
extern void SDL_ANDROID_MultiThreadedVideoLoopInit();
extern void SDL_ANDROID_MultiThreadedVideoLoop();

static int threadedMain(void * waitForDebugger);

int threadedMain(void * waitForDebugger)
{
	if( waitForDebugger )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "We are being debugged - waiting for debugger for 7 seconds");
		//usleep(7000000);
	}
	SDL_main( argc, argv );
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Application closed, calling exit(0)");
	exit(0);
}
#endif

extern C_LINKAGE void
JAVA_EXPORT_NAME(DemoRenderer_nativeInit) ( JNIEnv*  env, jobject thiz, jstring jcurdir, jstring cmdline, jint multiThreadedVideo, jint waitForDebugger )
{
	int i = 0;
	char curdir[PATH_MAX] = "";
	const jbyte *jstr;
	const char * str = "sdl";

	static_env = env;
	static_thiz = thiz;

	strcpy(curdir, "/sdcard/app-data/");
	strcat(curdir, SDL_CURDIR_PATH);

	jstr = (*env)->GetStringUTFChars(env, jcurdir, NULL);
	if (jstr != NULL && strlen(jstr) > 0)
		strcpy(curdir, jstr);
	(*env)->ReleaseStringUTFChars(env, jcurdir, jstr);

	chdir(curdir);
	setenv("HOME", curdir, 1);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Changing curdir to \"%s\"", curdir);

	if( waitForDebugger )
		redirectStdout();

	jstr = (*env)->GetStringUTFChars(env, cmdline, NULL);

	if (jstr != NULL && strlen(jstr) > 0)
		str = jstr;

	{
		char * str1, * str2;
		str1 = strdup(str);
		str2 = str1;
		while(str2)
		{
			argc++;
			str2 = strchr(str2, ' ');
			if(!str2)
				break;
			str2++;
		}

		argv = (char **)malloc(argc*sizeof(char *));
		str2 = str1;
		while(str2)
		{
			argv[i] = str2;
			i++;
			str2 = strchr(str2, ' ');
			if(str2)
				*str2 = 0;
			else
				break;
			str2++;
		}
	}

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Calling SDL_main(\"%s\")", str);

	(*env)->ReleaseStringUTFChars(env, cmdline, jstr);

	for( i = 0; i < argc; i++ )
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "param %d = \"%s\"", i, argv[i]);

#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_main( argc, argv );
#else
	if( ! multiThreadedVideo )
	{
		if( waitForDebugger )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "We are being debugged - waiting for debugger for 7 seconds");
			//usleep(7000000);
		}
		SDL_main( argc, argv );
	}
	else
	{
		SDL_ANDROID_MultiThreadedVideoLoopInit();
		SDL_CreateThread(threadedMain, (void *)waitForDebugger);
		SDL_ANDROID_MultiThreadedVideoLoop();
	}
#endif
};
