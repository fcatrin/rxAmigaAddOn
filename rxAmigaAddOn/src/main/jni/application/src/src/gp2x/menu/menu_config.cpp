#define _MENU_CONFIG_CPP

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "menu.h"
#include "menu_config.h"
#include "autoconf.h"
#include "options.h"
#include "gui.h"
#include "sound.h"
#include "custom.h"
#include "gp2x.h"
#include "cfgfile.h"

extern char kickstarts_dir[300];
extern int kickstart;
extern int sound_rate;
extern int skipintro;
extern int screenWidth;
extern int moveX;
extern int moveY;
#if !( defined(PANDORA) || defined(ANDROIDSDL) )
extern int timeslice_mode;
#endif
extern char launchDir[300];
extern char currentDir[300];
extern int nr_drives;
extern char *config_filename;
int isAmiga1200 = 0;
int isAmiga600  = 0;

extern void extractFileName(char * str,char *buffer);

#ifdef ANDROIDSDL
#include <android/log.h>
#endif

char filename0[256] = "";
char filename1[256] = "";
char filename2[256] = "";
char filename3[256] = "";

int mainMenu_chipMemory = DEFAULT_CHIPMEM_SELECT;
int mainMenu_slowMemory = 0;	/* off */
int mainMenu_fastMemory = 0;	/* off */

int mainMenu_bootHD = DEFAULT_ENABLE_HD;
int mainMenu_filesysUnits = 0;
int hd_dir_unit_nr = -1;
int hd_file_unit_nr = -1;

int mainMenu_drives = DEFAULT_DRIVES;
int mainMenu_floppyspeed = 100;
int mainMenu_CPU_model = DEFAULT_CPU_MODEL;
int mainMenu_chipset = DEFAULT_CHIPSET_SELECT;
int mainMenu_sound = DEFAULT_SOUND;
int mainMenu_soundStereo = 1; // Default is stereo
int mainMenu_CPU_speed = 0;

int mainMenu_cpuSpeed = 600;

int mainMenu_joyConf = 0;
int mainMenu_joyPort = 0; // Both ports
int mainMenu_autofireRate = 8;
int mainMenu_showStatus = DEFAULT_STATUSLN;
int mainMenu_showFPS = false;
int mainMenu_mouseMultiplier = DEFAULT_MOUSEMULTIPLIER;
int mainMenu_stylusOffset = 0;
int mainMenu_tapDelay = 10;
int mainMenu_customControls = 0;
int mainMenu_custom_dpad = 0;
int mainMenu_custom_up = 0;
int mainMenu_custom_down = 0;
int mainMenu_custom_left = 0;
int mainMenu_custom_right = 0;
int mainMenu_custom_A = 0;
int mainMenu_custom_B = 0;
int mainMenu_custom_X = 0;
int mainMenu_custom_Y = 0;
int mainMenu_custom_L = 0;
int mainMenu_custom_R = 0;
int mainMenu_autofire = DEFAULT_AUTOFIRE;

int mainMenu_displayedLines = 240;
int mainMenu_displayHires = 0;
char presetMode[20] = "320x240 upscaled";
int presetModeId = 2;
int mainMenu_cutLeft = 0;
int mainMenu_cutRight = 0;
int mainMenu_ntsc = DEFAULT_NTSC;
int mainMenu_frameskip = 0;
int visibleAreaWidth = 320;

int saveMenu_n_savestate = 0;


// The following params in use, but can't be changed with gui
int mainMenu_autosave = DEFAULT_AUTOSAVE;
int mainMenu_button1 = GP2X_BUTTON_X;
int mainMenu_button2 = GP2X_BUTTON_A;
int mainMenu_autofireButton1 = GP2X_BUTTON_B;
int mainMenu_jump = -1;

// The following params not in use, but stored to write them back to the config file
int gp2xClockSpeed = -1;
int mainMenu_scanlines = 0;
int mainMenu_enableScreenshots = DEFAULT_ENABLESCREENSHOTS;
int mainMenu_enableScripts = DEFAULT_ENABLESCRIPTS;

#ifdef ANDROIDSDL
int mainMenu_onScreen = 1;
int mainMenu_onScreen_textinput = 1;
int mainMenu_onScreen_dpad = 1;
int mainMenu_onScreen_button1 = 1;
int mainMenu_onScreen_button2 = 1;
int mainMenu_onScreen_button3 = 1;
int mainMenu_onScreen_button4 = 1;
int mainMenu_onScreen_button5 = 0;
int mainMenu_onScreen_button6 = 0;
int mainMenu_custom_position = 0;
int mainMenu_pos_x_textinput = 0;
int mainMenu_pos_y_textinput = 0;
int mainMenu_pos_x_dpad = 4;
int mainMenu_pos_y_dpad = 215;
int mainMenu_pos_x_button1 = 430;
int mainMenu_pos_y_button1 = 286;
int mainMenu_pos_x_button2 = 378;
int mainMenu_pos_y_button2 = 286;
int mainMenu_pos_x_button3 = 430;
int mainMenu_pos_y_button3 = 214;
int mainMenu_pos_x_button4 = 378;
int mainMenu_pos_y_button4 = 214;
int mainMenu_pos_x_button5 = 430;
int mainMenu_pos_y_button5 = 142;
int mainMenu_pos_x_button6 = 378;
int mainMenu_pos_y_button6 = 142;
int menuLoad_extfilter=1;
int mainMenu_quickSwitch=0;
#endif

void SetDefaultMenuSettings(int general)
{
	mainMenu_chipMemory = DEFAULT_CHIPMEM_SELECT;
	mainMenu_slowMemory = 0;	/* off */
	mainMenu_fastMemory = 0;	/* off */
	UpdateMemorySettings();

	if(general < 2)
	{
		mainMenu_bootHD=DEFAULT_ENABLE_HD;
		if (hd_dir_unit_nr >= 0) 
		{
			kill_filesys_unit(currprefs.mountinfo, 0);
			hd_dir_unit_nr = -1;
		}
		if (hd_file_unit_nr >= 0) 
		{
			kill_filesys_unit(currprefs.mountinfo, 0);
			hd_file_unit_nr = -1;
		}
		mainMenu_filesysUnits = 0;
	}

	if(general > 0)
	{
	//strcpy(uae4all_image_file0, "/sdcard/s1.adf");

		uae4all_image_file0[0] = '\0';
		uae4all_image_file1[0] = '\0';
		uae4all_image_file2[0] = '\0';
		uae4all_image_file3[0] = '\0';
		uae4all_image_file4[0] = '\0';
		uae4all_image_file5[0] = '\0';
		uae4all_image_file6[0] = '\0';
		uae4all_image_file7[0] = '\0';
		mainMenu_drives = DEFAULT_DRIVES;
	}	
	mainMenu_floppyspeed = 100;

	mainMenu_CPU_model = DEFAULT_CPU_MODEL;
	mainMenu_chipset = DEFAULT_CHIPSET_SELECT;
	UpdateChipsetSettings();
	kickstart = DEFAULT_KICKSTART;
	mainMenu_sound = DEFAULT_SOUND;
	sound_rate = DEFAULT_SOUND_FREQ;
	mainMenu_soundStereo = 1; // Default is stereo
	mainMenu_CPU_speed = 0;
	
	mainMenu_cpuSpeed = 600;
	
	mainMenu_joyConf = 0;
	mainMenu_joyPort = 0;
	mainMenu_autofireRate = 8;
	mainMenu_showStatus = DEFAULT_STATUSLN;
	mainMenu_mouseMultiplier = DEFAULT_MOUSEMULTIPLIER;
	mainMenu_stylusOffset = 0;
	mainMenu_tapDelay = 10;
	mainMenu_customControls = 0;
	mainMenu_custom_dpad = 0;
	mainMenu_custom_up = 0;
	mainMenu_custom_down = 0;
	mainMenu_custom_left = 0;
	mainMenu_custom_right = 0;
	mainMenu_custom_A = 0;
	mainMenu_custom_B = 0;
	mainMenu_custom_X = 0;
	mainMenu_custom_Y = 0;
	mainMenu_custom_L = 0;
	mainMenu_custom_R = 0;
	
	SetPresetMode(2);
	moveX = 0;
	moveY = 0;
	mainMenu_cutLeft = 0;
	mainMenu_cutRight = 0;
	mainMenu_ntsc = DEFAULT_NTSC;
	mainMenu_frameskip = 0;
	mainMenu_autofire = DEFAULT_AUTOFIRE;
	
	// The following params can't be changed in gui
	skipintro = DEFAULT_SKIPINTRO;
	mainMenu_autosave = DEFAULT_AUTOSAVE;
	mainMenu_button1 = GP2X_BUTTON_X;
	mainMenu_button2 = GP2X_BUTTON_A;
	mainMenu_autofireButton1 = GP2X_BUTTON_B;
	mainMenu_jump = -1;
	
	gp2xClockSpeed = -1;
	mainMenu_scanlines = 0;
	mainMenu_enableScreenshots = DEFAULT_ENABLESCREENSHOTS;
	mainMenu_enableScripts = DEFAULT_ENABLESCRIPTS;
}


void UpdateCPUModelSettings()
{
	switch (mainMenu_CPU_model)
	{
	    case 1: changed_prefs.cpu_level = M68020; break;
	    default: changed_prefs.cpu_level = M68000; break;
	}
}


void UpdateMemorySettings()
{
	prefs_chipmem_size = 0x000080000 << mainMenu_chipMemory;

	/* >2MB chip memory => 0 fast memory */
	if ((mainMenu_chipMemory > 2) && (mainMenu_fastMemory > 0))
	{
		mainMenu_fastMemory = 0;
		changed_prefs.fastmem_size = 0;
	}

	switch (mainMenu_slowMemory) 
	{
		case 1: case 2:
			prefs_bogomem_size = 0x00080000 << (mainMenu_slowMemory - 1);
			break;
		case 3:
			prefs_bogomem_size = 0x00180000;	/* 1.5M */
			break;
		default:
			prefs_bogomem_size = 0;
	}

	switch (mainMenu_fastMemory) 
	{
		case 0:
			changed_prefs.fastmem_size = 0;
			break;
		default:
			changed_prefs.fastmem_size = 0x00080000 << mainMenu_fastMemory;
	}

}


void UpdateChipsetSettings()
{
	switch (mainMenu_chipset) 
	{
		case 1: changed_prefs.chipset_mask = CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE; break;
		case 2: changed_prefs.chipset_mask = CSMASK_ECS_AGNUS | CSMASK_ECS_DENISE | CSMASK_AGA; break;
		default: changed_prefs.chipset_mask = CSMASK_ECS_AGNUS; break;
	}
}


void SetPresetMode(int mode)
{
	presetModeId = mode;
	
	switch(mode)
	{
		case 0:
			mainMenu_displayedLines = 200;
			screenWidth = 768;
			strcpy(presetMode, "320x200 upscaled");
			break;
			
		case 1:
			mainMenu_displayedLines = 216;
			screenWidth = 716;
			strcpy(presetMode, "320x216 upscaled");
			break;
			
		case 2:
			mainMenu_displayedLines = 240;
			screenWidth = 640;
			strcpy(presetMode, "320x240 upscaled");
			break;

		case 3:
			mainMenu_displayedLines = 256;
			screenWidth = 600;
			strcpy(presetMode, "320x256 upscaled");
			break;
						
		case 4:
			mainMenu_displayedLines = 262;
			screenWidth = 588;
			strcpy(presetMode, "320x262 upscaled");
			break;
						
		case 5:
			mainMenu_displayedLines = 270;
			screenWidth = 570;
			strcpy(presetMode, "320x270 upscaled");
			break;
						
		case 6:
			mainMenu_displayedLines = 200;
			screenWidth = 640;
			strcpy(presetMode, "320x200 NTSC");
			break;
						
		case 7:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "320x200 fullscreen");
			break;
						
		case 10:
			mainMenu_displayedLines = 200;
			screenWidth = 768;
			strcpy(presetMode, "640x200 upscaled");
			break;
			
		case 11:
			mainMenu_displayedLines = 216;
			screenWidth = 716;
			strcpy(presetMode, "640x216 upscaled");
			break;
			
		case 12:
			mainMenu_displayedLines = 240;
			screenWidth = 640;
			strcpy(presetMode, "640x240 upscaled");
			break;

		case 13:
			mainMenu_displayedLines = 256;
			screenWidth = 600;
			strcpy(presetMode, "640x256 upscaled");
			break;
						
		case 14:
			mainMenu_displayedLines = 262;
			screenWidth = 588;
			strcpy(presetMode, "640x262 upscaled");
			break;
						
		case 15:
			mainMenu_displayedLines = 270;
			screenWidth = 570;
			strcpy(presetMode, "640x270 upscaled");
			break;
						
		case 16:
			mainMenu_displayedLines = 200;
			screenWidth = 640;
			strcpy(presetMode, "640x200 NTSC");
			break;
						
		case 17:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "640x200 fullscreen");
			break;

		case 20:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "352x200 upscaled");
			break;
			
		case 21:
			mainMenu_displayedLines = 216;
			screenWidth = 784;
			strcpy(presetMode, "352x216 upscaled");
			break;
			
		case 22:
			mainMenu_displayedLines = 240;
			screenWidth = 704;
			strcpy(presetMode, "352x240 upscaled");
			break;

		case 23:
			mainMenu_displayedLines = 256;
			screenWidth = 660;
			strcpy(presetMode, "352x256 upscaled");
			break;
						
		case 24:
			mainMenu_displayedLines = 262;
			screenWidth = 640;
			strcpy(presetMode, "352x262 upscaled");
			break;
						
		case 25:
			mainMenu_displayedLines = 270;
			screenWidth = 624;
			strcpy(presetMode, "352x270 upscaled");
			break;
						
		case 26:
			mainMenu_displayedLines = 200;
			screenWidth = 704;
			strcpy(presetMode, "352x200 NTSC");
			break;
						
		case 27:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "352x200 fullscreen");
			break;
						
		case 30:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "704x200 upscaled");
			break;
			
		case 31:
			mainMenu_displayedLines = 216;
			screenWidth = 784;
			strcpy(presetMode, "704x216 upscaled");
			break;
			
		case 32:
			mainMenu_displayedLines = 240;
			screenWidth = 704;
			strcpy(presetMode, "704x240 upscaled");
			break;

		case 33:
			mainMenu_displayedLines = 256;
			screenWidth = 660;
			strcpy(presetMode, "704x256 upscaled");
			break;
						
		case 34:
			mainMenu_displayedLines = 262;
			screenWidth = 640;
			strcpy(presetMode, "704x262 upscaled");
			break;
						
		case 35:
			mainMenu_displayedLines = 270;
			screenWidth = 624;
			strcpy(presetMode, "704x270 upscaled");
			break;
						
		case 36:
			mainMenu_displayedLines = 200;
			screenWidth = 704;
			strcpy(presetMode, "704x200 NTSC");
			break;
						
		case 37:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "704x200 fullscreen");
			break;
						
		case 40:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "384x200 upscaled");
			break;
			
		case 41:
			mainMenu_displayedLines = 216;
			screenWidth = 800;
			strcpy(presetMode, "384x216 upscaled");
			break;
			
		case 42:
			mainMenu_displayedLines = 240;
			screenWidth = 768;
			strcpy(presetMode, "384x240 upscaled");
			break;

		case 43:
			mainMenu_displayedLines = 256;
			screenWidth = 720;
			strcpy(presetMode, "384x256 upscaled");
			break;
						
		case 44:
			mainMenu_displayedLines = 262;
			screenWidth = 704;
			strcpy(presetMode, "384x262 upscaled");
			break;
						
		case 45:
			mainMenu_displayedLines = 270;
			screenWidth = 684;
			strcpy(presetMode, "384x270 upscaled");
			break;
						
		case 46:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "384x200 NTSC");
			break;
						
		case 47:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "384x200 fullscreen");
			break;

		case 50:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "768x200 upscaled");
			break;
			
		case 51:
			mainMenu_displayedLines = 216;
			screenWidth = 800;
			strcpy(presetMode, "768x216 upscaled");
			break;
			
		case 52:
			mainMenu_displayedLines = 240;
			screenWidth = 768;
			strcpy(presetMode, "768x240 upscaled");
			break;

		case 53:
			mainMenu_displayedLines = 256;
			screenWidth = 720;
			strcpy(presetMode, "768x256 upscaled");
			break;
						
		case 54:
			mainMenu_displayedLines = 262;
			screenWidth = 704;
			strcpy(presetMode, "768x262 upscaled");
			break;
						
		case 55:
			mainMenu_displayedLines = 270;
			screenWidth = 684;
			strcpy(presetMode, "768x270 upscaled");
			break;
						
		case 56:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "768x200 NTSC");
			break;
						
		case 57:
			mainMenu_displayedLines = 200;
			screenWidth = 800;
			strcpy(presetMode, "768x200 fullscreen");
			break;
						
  	default:
			mainMenu_displayedLines = 240;
			screenWidth = 640;
			strcpy(presetMode, "320x240 upscaled");
			presetModeId = 2;
			break;
	}
	
	switch(presetModeId / 10)
	{
		case 0:
			mainMenu_displayHires = 0;
			visibleAreaWidth = 320;
			break;

		case 1:
			mainMenu_displayHires = 1;
			visibleAreaWidth = 640;
			break;

		case 2:
			mainMenu_displayHires = 0;
			visibleAreaWidth = 352;
			break;

		case 3:
			mainMenu_displayHires = 1;
			visibleAreaWidth = 704;
			break;

		case 4:
			mainMenu_displayHires = 0;
			visibleAreaWidth = 384;
			break;

		case 5:
			mainMenu_displayHires = 1;
			visibleAreaWidth = 768;
			break;
	}
}


void set_joyConf()
{
	if(mainMenu_joyConf==0)
	{
		mainMenu_button1=GP2X_BUTTON_X;
		mainMenu_button2=GP2X_BUTTON_A;
		mainMenu_jump=-1;
		mainMenu_autofireButton1=GP2X_BUTTON_B;
	}
	else if(mainMenu_joyConf==1)
	{
		mainMenu_button1=GP2X_BUTTON_B;
		mainMenu_button2=GP2X_BUTTON_A;
		mainMenu_jump=-1;
		mainMenu_autofireButton1=GP2X_BUTTON_X;
	}	
	else if(mainMenu_joyConf==2)
	{
		mainMenu_button1=GP2X_BUTTON_Y;
		mainMenu_button2=GP2X_BUTTON_A;
		mainMenu_jump=GP2X_BUTTON_X;
		mainMenu_autofireButton1=GP2X_BUTTON_B;
	}
	else if(mainMenu_joyConf==3)
	{
		mainMenu_button1=GP2X_BUTTON_B;
		mainMenu_button2=GP2X_BUTTON_A;
		mainMenu_jump=GP2X_BUTTON_X;
		mainMenu_autofireButton1=GP2X_BUTTON_Y;
	}
}


void reset_hdConf()
{
	/* Reset HD config */
	if (hd_dir_unit_nr >= 0)
	{
		kill_filesys_unit(currprefs.mountinfo, 0);
		hd_dir_unit_nr = -1;
	}
	if (hd_file_unit_nr >= 0)
	{
		kill_filesys_unit(currprefs.mountinfo, 0);
		hd_file_unit_nr = -1;
	}
	mainMenu_filesysUnits = 0;

	switch (mainMenu_bootHD)
	{
		case 0:
			// nothing to do, already killed above
			break;
		case 1:
			if (hd_dir_unit_nr < 0)
			{
				if (uae4all_hard_dir[0] != '\0')
				{
					parse_filesys_spec(0, uae4all_hard_dir);
					hd_dir_unit_nr = mainMenu_filesysUnits++;
				}
			}
			if (hd_file_unit_nr < 0)
			{
				if (uae4all_hard_file[0] != '\0')
				{
					parse_hardfile_spec(uae4all_hard_file);
					hd_file_unit_nr = mainMenu_filesysUnits++;
				}
			}
			break;
		case 2:
			if (hd_file_unit_nr < 0)
			{
				if (uae4all_hard_file[0] != '\0')
				{
					parse_hardfile_spec(uae4all_hard_file);
					hd_file_unit_nr = mainMenu_filesysUnits++;
				}
			}
			if (hd_dir_unit_nr < 0)
			{
				if (uae4all_hard_dir[0] != '\0')
				{
					parse_filesys_spec(0, uae4all_hard_dir);
					hd_dir_unit_nr = mainMenu_filesysUnits++;
				}
			}
			break;
	}
}


static void replace(char * str,char replace, char toreplace)
{
	while(*str)
	{	
		if (*str==toreplace) *str=replace;
		str++;
	}
}


int create_configfilename(char *dest, char *basename, int fromDir)
{
	char *p; 
	p = basename + strlen(basename) - 1;
	while (*p != '/') 
		p--;
	p++;
	if(fromDir == 0)
	{
		int len = strlen(p) + 1;
		char filename[len];
		strcpy(filename, p);
		char *pch = &filename[len];
		while (pch != filename && *pch != '.')
			pch--;
		if (pch)
		{
			*pch='\0';
			snprintf(dest, 300, "%s/conf/%s.conf", launchDir, filename);
			return 1;
		}
	}
	else
	{
		snprintf(dest, 300, "%s/conf/%s.conf", launchDir, p);
		return 1;
	}
			
	return 0;
}


int saveconfig(int general)
{
	char path[300];
	char *p; 
	switch(general)
	{
		case 0:
			if (!uae4all_image_file0[0]) 
				return 0;
			if(!create_configfilename(path, uae4all_image_file0, 0))
				return 0;
			break;
		case 1:
			snprintf(path, 300, "%s/conf/uaeconfig.conf", launchDir);
			break;
		case 2:
			path[0] = '\0';
			if(mainMenu_bootHD == 1 && uae4all_hard_dir[0] != '\0')
				create_configfilename(path, uae4all_hard_dir, 1);
			if(mainMenu_bootHD == 2 && uae4all_hard_file[0] != '\0')
				create_configfilename(path, uae4all_hard_file, 0);
			if(path[0] == '\0')
				return 0;
			break;
		case 3:
			snprintf(path, 300, config_filename, launchDir);
			break;
	}

	FILE *f=fopen(path,"w");
	if (!f) return 0;
	char buffer[255];

	snprintf((char*)buffer, 255, "kickstart=%d\n",kickstart);
	fputs(buffer,f);
#if defined(PANDORA) || defined(ANDROIDSDL)
	snprintf((char*)buffer, 255, "scaling=%d\n",0);
#else
	snprintf((char*)buffer, 255, "scaling=%d\n",mainMenu_enableHWscaling);
#endif
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "showstatus=%d\n",mainMenu_showStatus);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "mousemultiplier=%d\n",mainMenu_mouseMultiplier);
	fputs(buffer,f);
#if defined(PANDORA) || defined(ANDROIDSDL)
	snprintf((char*)buffer, 255, "systemclock=%d\n",5);   // mainMenu_throttle never changes -> removed
#else
	snprintf((char*)buffer, 255, "systemclock=%d\n",mainMenu_throttle);
#endif
	fputs(buffer,f);
#if defined(PANDORA) || defined(ANDROIDSDL)
	snprintf((char*)buffer, 255, "syncthreshold=%d\n",2); // timeslice_mode never changes -> removed
#else
	snprintf((char*)buffer, 255, "syncthreshold=%d\n",timeslice_mode);
#endif
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "frameskip=%d\n",mainMenu_frameskip);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "sound=%d\n",mainMenu_sound + mainMenu_soundStereo * 10);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "soundrate=%d\n",sound_rate);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "autosave=%d\n",mainMenu_autosave);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "gp2xclock=%d\n",gp2xClockSpeed);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "joyconf=%d\n",mainMenu_joyConf + (mainMenu_joyPort << 4));
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "autofireRate=%d\n",mainMenu_autofireRate);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "autofire=%d\n",mainMenu_autofire);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "stylusOffset=%d\n",mainMenu_stylusOffset);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "tapDelay=%d\n",mainMenu_tapDelay);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "scanlines=%d\n",mainMenu_scanlines);
	fputs(buffer,f);
#if defined(PANDORA) || defined(ANDROIDSDL)
	snprintf((char*)buffer, 255, "ham=%d\n",1);
#else
	snprintf((char*)buffer, 255, "ham=%d\n",mainMenu_ham);
#endif
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "enableScreenshots=%d\n",mainMenu_enableScreenshots);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "floppyspeed=%d\n",mainMenu_floppyspeed);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "drives=%d\n",nr_drives);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "videomode=%d\n",mainMenu_ntsc);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "mainMenu_cpuSpeed=%d\n",mainMenu_cpuSpeed);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "presetModeId=%d\n",presetModeId);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "moveX=%d\n",moveX);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "moveY=%d\n",moveY);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "displayedLines=%d\n",mainMenu_displayedLines);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "screenWidth=%d\n",screenWidth);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "cutLeft=%d\n",mainMenu_cutLeft);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "cutRight=%d\n",mainMenu_cutRight);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "customControls=%d\n",mainMenu_customControls);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_dpad=%d\n",mainMenu_custom_dpad);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_up=%d\n",mainMenu_custom_up);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_down=%d\n",mainMenu_custom_down);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_left=%d\n",mainMenu_custom_left);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_right=%d\n",mainMenu_custom_right);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_A=%d\n",mainMenu_custom_A);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_B=%d\n",mainMenu_custom_B);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_X=%d\n",mainMenu_custom_X);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_Y=%d\n",mainMenu_custom_Y);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_L=%d\n",mainMenu_custom_L);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_R=%d\n",mainMenu_custom_R);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "cpu=%d\n",mainMenu_CPU_model);
  fputs(buffer,f);
	snprintf((char*)buffer, 255, "chipset=%d\n",mainMenu_chipset);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "cpu=%d\n",-mainMenu_CPU_speed);
 	fputs(buffer,f);

	if(general == 0)
	{
		char namebuffer[256];
		strcpy(namebuffer,uae4all_image_file0);
		replace (namebuffer,'|',' ');
		snprintf((char*)buffer, 255, "df0=%s\n",namebuffer);
		fputs(buffer,f);
		if (uae4all_image_file1[0])
		{
			strcpy(namebuffer,uae4all_image_file1);
			replace (namebuffer,'|',' ');
			snprintf((char*)buffer, 255, "df1=%s\n",namebuffer);
			fputs(buffer,f);
		}
		if (uae4all_image_file2[0])
		{
			strcpy(namebuffer,uae4all_image_file2);
			replace (namebuffer,'|',' ');	
			snprintf((char*)buffer, 255, "df2=%s\n",namebuffer);
			fputs(buffer,f);
		}
		if (uae4all_image_file3[0])
		{
			strcpy(namebuffer,uae4all_image_file3);
			replace (namebuffer,'|',' ');
			snprintf((char*)buffer, 255, "df3=%s\n",namebuffer);
			fputs(buffer,f);
		}
	}
	else
	{
		snprintf((char*)buffer, 255, "script=%d\n",mainMenu_enableScripts);
		fputs(buffer,f);
		snprintf((char*)buffer, 255, "screenshot=%d\n",mainMenu_enableScreenshots);
		fputs(buffer,f);
		snprintf((char*)buffer, 255, "skipintro=%d\n",skipintro);
		fputs(buffer,f);
		snprintf((char*)buffer, 255, "boot_hd=%d\n",mainMenu_bootHD);
		fputs(buffer,f);
		if (uae4all_hard_dir[0] == '\0')
			snprintf((char*)buffer, 255, "hard_disk_dir=%s\n","*");
		else
			snprintf((char*)buffer, 255, "hard_disk_dir=%s\n",uae4all_hard_dir);
		fputs(buffer,f);
		if (uae4all_hard_file[0] == '\0')
			snprintf((char*)buffer, 255, "hard_disk_file=%s\n","*");
		else
			snprintf((char*)buffer, 255, "hard_disk_file=%s\n",uae4all_hard_file);
		fputs(buffer,f);
	}
	snprintf((char*)buffer, 255, "chipmemory=%d\n",mainMenu_chipMemory);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "slowmemory=%d\n",mainMenu_slowMemory);
	fputs(buffer,f);
	snprintf((char*)buffer, 255, "fastmemory=%d\n",mainMenu_fastMemory);
	fputs(buffer,f);
#ifdef ANDROIDSDL
	snprintf((char*)buffer, 255, "onscreen=%d\n",mainMenu_onScreen);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "onScreen_textinput=%d\n",mainMenu_onScreen_textinput);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "onScreen_dpad=%d\n",mainMenu_onScreen_dpad);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "onScreen_button1=%d\n",mainMenu_onScreen_button1);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "onScreen_button2=%d\n",mainMenu_onScreen_button2);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "onScreen_button3=%d\n",mainMenu_onScreen_button3);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "onScreen_button4=%d\n",mainMenu_onScreen_button4);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "onScreen_button5=%d\n",mainMenu_onScreen_button5);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "onScreen_button6=%d\n",mainMenu_onScreen_button6);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "custom_position=%d\n",mainMenu_custom_position);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_x_textinput=%d\n",mainMenu_pos_x_textinput);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_y_textinput=%d\n",mainMenu_pos_y_textinput);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_x_dpad=%d\n",mainMenu_pos_x_dpad);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_y_dpad=%d\n",mainMenu_pos_y_dpad);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_x_button1=%d\n",mainMenu_pos_x_button1);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_y_button1=%d\n",mainMenu_pos_y_button1);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_x_button2=%d\n",mainMenu_pos_x_button2);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_y_button2=%d\n",mainMenu_pos_y_button2);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_x_button3=%d\n",mainMenu_pos_x_button3);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_y_button3=%d\n",mainMenu_pos_y_button3);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_x_button4=%d\n",mainMenu_pos_x_button4);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_y_button4=%d\n",mainMenu_pos_y_button4);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_x_button5=%d\n",mainMenu_pos_x_button5);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_y_button5=%d\n",mainMenu_pos_y_button5);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_x_button6=%d\n",mainMenu_pos_x_button6);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "pos_y_button6=%d\n",mainMenu_pos_y_button6);
 	fputs(buffer,f);
	snprintf((char*)buffer, 255, "quick_switch=%d\n",mainMenu_quickSwitch);
 	fputs(buffer,f);
#endif
	fclose(f);
	return 1;
}


void loadconfig(int general) {
	loadconfigcustom(general, NULL);
}

void loadconfigcustom(int general, char *configfile)
{
	char path[300];

	__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "loadconfigcustom %i %s", general, configfile==NULL?"null":configfile);

	if(general == 1)
	{
		snprintf(path, 300, "%s/conf/adfdir.conf", launchDir);
		FILE *f1=fopen(path,"rt");
		if(!f1)
		{
			printf ("No config file %s!\n",path);
			strcpy(currentDir, launchDir);
#ifdef ANDROIDSDL
		char *dirname="../../com.cloanto.amigaforever.essentials/files/adf/";
		struct stat statbuf;
		stat (dirname, &statbuf);
		if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
			  strcat(currentDir, "/../../com.cloanto.amigaforever.essentials/files/adf");
			else
#endif
		  strcat(currentDir, "/roms/");
		}
		else
		{
			fscanf(f1,"path=%s\n",&currentDir);
			fclose(f1);
			printf("adfdir loaded. currentDir=%s\n", currentDir);
		}
	}

	if (configfile!=NULL)
		snprintf(path, 300, "%s", configfile);
	else if (general == 1)
		snprintf(path, 300, "%s/conf/uaeconfig.conf", launchDir);
	else if (general == 3)
		snprintf(path, 300, config_filename, launchDir);
	else if(general == 0)
		create_configfilename(path, uae4all_image_file0, 0);
	else
	{
		path[0] = '\0';
		if(mainMenu_bootHD == 1 && uae4all_hard_dir[0] != '\0')
			create_configfilename(path, uae4all_hard_dir, 1);
		if(mainMenu_bootHD == 2 && uae4all_hard_file[0] != '\0')
			create_configfilename(path, uae4all_hard_file, 0);
		if(path[0] == '\0')
			return;
	}
		
	FILE *f=fopen(path,"rt");
	if (!f){
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "No config file %s!",path);
	}
	else
	{
		// Set everthing to default and clear HD settings
		SetDefaultMenuSettings(general);
	
		char filebuffer[256];
#if defined(PANDORA) || defined(ANDROIDSDL)
		int dummy;
#endif
		char line[1024];
		while (fgets(line, sizeof(line), f)) {

			sscanf(line, "df0=%[^\n]s", uae4all_image_file0);
			sscanf(line, "df1=%[^\n]s", uae4all_image_file1);
			sscanf(line, "df2=%[^\n]s", uae4all_image_file2);
			sscanf(line, "df3=%[^\n]s", uae4all_image_file3);
			sscanf(line, "df4=%[^\n]s", uae4all_image_file4);
			sscanf(line, "df5=%[^\n]s", uae4all_image_file5);
			sscanf(line, "df6=%[^\n]s", uae4all_image_file6);
			sscanf(line, "df7=%[^\n]s", uae4all_image_file7);

			sscanf(line, "hdf=%[^\n]s", uae4all_hard_file);

			sscanf(line, "kickstart=%d\n",&kickstart);
			sscanf(line, "kickstarts_dir=%s\n",kickstarts_dir);
			sscanf(line, "frameskip=%d\n",&mainMenu_frameskip);
			sscanf(line, "presetModeId=%d\n",&presetModeId); // resolution to render on
			sscanf(line, "showstatus=%d\n",&mainMenu_showStatus); // 1 = show leds
			sscanf(line, "showfps=%d\n",&mainMenu_showFPS); // 1 = show leds
			sscanf(line, "soundrate=%d\n",&sound_rate); // default is 44100. 22050 is more close to the Amiga500
			sscanf(line, "floppyspeed=%d\n",&mainMenu_floppyspeed); // floppy speed in percent (100 = 100% Amiga)
			sscanf(line, "drives=%d\n",&mainMenu_drives); // restrict number of drives
			sscanf(line, "a1200=%d\n", &isAmiga1200);
			sscanf(line, "a600=%d\n", &isAmiga600);
			sscanf(line,"moveX=%d\n",&moveX);
			sscanf(line,"moveY=%d\n",&moveY);


			// joystick & keyboaard overlay (1 = show, 0 = hide)
			sscanf(line, "onscreen=%d\n",&mainMenu_onScreen);
			sscanf(line, "onScreen_textinput=%d\n",&mainMenu_onScreen_textinput);
			sscanf(line, "onScreen_dpad=%d\n",&mainMenu_onScreen_dpad);
			sscanf(line, "onScreen_button1=%d\n",&mainMenu_onScreen_button1);
			sscanf(line, "onScreen_button2=%d\n",&mainMenu_onScreen_button2);
			sscanf(line, "onScreen_button3=%d\n",&mainMenu_onScreen_button3);
			sscanf(line, "onScreen_button4=%d\n",&mainMenu_onScreen_button4);
			sscanf(line, "onScreen_button5=%d\n",&mainMenu_onScreen_button5);
			sscanf(line, "onScreen_button6=%d\n",&mainMenu_onScreen_button6);
		}


		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "df0 %s", uae4all_image_file0);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "df1 %s", uae4all_image_file1);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "df2 %s", uae4all_image_file2);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "df3 %s", uae4all_image_file3);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "df4 %s", uae4all_image_file4);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "df5 %s", uae4all_image_file5);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "df6 %s", uae4all_image_file6);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "df7 %s", uae4all_image_file7);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "hdf %s", uae4all_hard_file);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "kickstart %i", kickstart);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "kickstarts_dir %s", kickstarts_dir);
		snprintf(romfile, 256, "%s/%s",kickstarts_dir,kickstarts_rom_names[kickstart]);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "romfile %s", kickstarts_dir);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "frameskip %d", mainMenu_frameskip);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "soundrate %d", sound_rate);
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "system %s", isAmiga1200?"a1200": (isAmiga600 ? "a600": "a500"));

/*		fscanf(f,"kickstart=%d\n",&kickstart);
#if defined(PANDORA) || defined(ANDROIDSDL)
		fscanf(f,"scaling=%d\n",&dummy);
#else
		fscanf(f,"scaling=%d\n",&mainMenu_enableHWscaling);
#endif
		fscanf(f,"showstatus=%d\n",&mainMenu_showStatus);
		fscanf(f,"mousemultiplier=%d\n",&mainMenu_mouseMultiplier );
#if defined(PANDORA) || defined(ANDROIDSDL)
		fscanf(f,"systemclock=%d\n",&dummy);    // mainMenu_throttle never changes -> removed
		fscanf(f,"syncthreshold=%d\n", &dummy); // timeslice_mode never changes -> removed
#else
		fscanf(f,"systemclock=%d\n",&mainMenu_throttle);
		fscanf(f,"syncthreshold=%d\n", &timeslice_mode);
#endif
		fscanf(f,"frameskip=%d\n",&mainMenu_frameskip);
		fscanf(f,"sound=%d\n",&mainMenu_sound );
		if(mainMenu_sound >= 10)
	  {
	    mainMenu_soundStereo = 1;
	    mainMenu_sound -= 10;
	  }
	  else
	    mainMenu_soundStereo = 0;
		fscanf(f,"soundrate=%d\n",&sound_rate);
		fscanf(f,"autosave=%d\n",&mainMenu_autosave);
		fscanf(f,"gp2xclock=%d\n", &gp2xClockSpeed);
		int joybuffer = 0;
		fscanf(f,"joyconf=%d\n",&joybuffer);
		mainMenu_joyConf = (joybuffer & 0x0f);
		mainMenu_joyPort = ((joybuffer >> 4) & 0x0f);
		fscanf(f,"autofireRate=%d\n",&mainMenu_autofireRate);
		fscanf(f,"autofire=%d\n",&mainMenu_autofire);
		fscanf(f,"stylusOffset=%d\n",&mainMenu_stylusOffset);
		fscanf(f,"tapDelay=%d\n",&mainMenu_tapDelay);
		fscanf(f,"scanlines=%d\n",&mainMenu_scanlines);
#if defined(PANDORA) || defined(ANDROIDSDL)
		fscanf(f,"ham=%d\n",&dummy);
#else
		fscanf(f,"ham=%d\n",&mainMenu_ham);
#endif
		fscanf(f,"enableScreenshots=%d\n",&mainMenu_enableScreenshots);
		fscanf(f,"floppyspeed=%d\n",&mainMenu_floppyspeed);
		fscanf(f,"drives=%d\n",&nr_drives);
		fscanf(f,"videomode=%d\n",&mainMenu_ntsc);
		fscanf(f,"mainMenu_cpuSpeed=%d\n",&mainMenu_cpuSpeed);
		fscanf(f,"moveX=%d\n",&moveX);
		fscanf(f,"moveY=%d\n",&moveY);
		fscanf(f,"displayedLines=%d\n",&mainMenu_displayedLines);
		fscanf(f,"screenWidth=%d\n",&screenWidth);
		fscanf(f,"cutLeft=%d\n",&mainMenu_cutLeft);
		fscanf(f,"cutRight=%d\n",&mainMenu_cutRight);
		fscanf(f,"customControls=%d\n",&mainMenu_customControls);
		fscanf(f,"custom_dpad=%d\n",&mainMenu_custom_dpad);
		fscanf(f,"custom_up=%d\n",&mainMenu_custom_up);
		fscanf(f,"custom_down=%d\n",&mainMenu_custom_down);
		fscanf(f,"custom_left=%d\n",&mainMenu_custom_left);
		fscanf(f,"custom_right=%d\n",&mainMenu_custom_right);
		fscanf(f,"custom_A=%d\n",&mainMenu_custom_A);
		fscanf(f,"custom_B=%d\n",&mainMenu_custom_B);
		fscanf(f,"custom_X=%d\n",&mainMenu_custom_X);
		fscanf(f,"custom_Y=%d\n",&mainMenu_custom_Y);
		fscanf(f,"custom_L=%d\n",&mainMenu_custom_L);
		fscanf(f,"custom_R=%d\n",&mainMenu_custom_R);
		fscanf(f,"cpu=%d\n",&mainMenu_CPU_model);
		fscanf(f,"chipset=%d\n",&mainMenu_chipset);
		fscanf(f,"cpu=%d\n",&mainMenu_CPU_speed);
	  if(mainMenu_CPU_speed < 0)
    {
      // New version of this option
      mainMenu_CPU_speed = -mainMenu_CPU_speed;
    }
    else
    {
      // Old version (500 5T 1200 12T 12T2)
	    if(mainMenu_CPU_speed >= 2)
      {
        // 1200: set to 68020 with 14 MHz
        mainMenu_CPU_model = 1;
        mainMenu_CPU_speed--;
        if(mainMenu_CPU_speed > 2)
          mainMenu_CPU_speed = 2;
      }
	  }
		if(general == 0)
		{
			fscanf(f,"df0=%s\n",&filebuffer);
			replace(filebuffer,' ','|');
			strcpy(uae4all_image_file0,filebuffer);	
			if(nr_drives > 1)
			{
				memset(filebuffer, 0, 256);
				fscanf(f,"df1=%s\n",&filebuffer);
				replace(filebuffer,' ','|');
				strcpy(uae4all_image_file1,filebuffer);
				extractFileName(uae4all_image_file1,filename1);
			}
			if(nr_drives > 2)
			{
				memset(filebuffer, 0, 256);
				fscanf(f,"df2=%s\n",&filebuffer);
				replace(filebuffer,' ','|');
				strcpy(uae4all_image_file2,filebuffer);
				extractFileName(uae4all_image_file2,filename2);
			}
			if(nr_drives > 3)
			{
				memset(filebuffer, 0, 256);
				fscanf(f,"df3=%s\n",&filebuffer);
				replace(filebuffer,' ','|');
				strcpy(uae4all_image_file3,filebuffer);
				extractFileName(uae4all_image_file3,filename3);
			}
		}
		else
		{
			fscanf(f,"script=%d\n",&mainMenu_enableScripts);
			fscanf(f,"screenshot=%d\n", &mainMenu_enableScreenshots);
			fscanf(f,"skipintro=%d\n", &skipintro);
			fscanf(f,"boot_hd=%d\n",&mainMenu_bootHD);
			
			fscanf(f,"hard_disk_dir=",uae4all_hard_dir);
			uae4all_hard_dir[0] = '\0';
			{
				char c[2] = {0, 0};
				*c = fgetc(f);
				while (*c && (*c != '\n')) {
					strcat(uae4all_hard_dir, c);
					*c = fgetc(f);
				}
			}
			if (uae4all_hard_dir[0] == '*')
				uae4all_hard_dir[0] = '\0';
			
			fscanf(f,"hard_disk_file=",uae4all_hard_file);
			uae4all_hard_file[0] = '\0';
			{
				char c[2] = {0, 0};
				*c = fgetc(f);
				while (*c && (*c != '\n')) {
					strcat(uae4all_hard_file, c);
					*c = fgetc(f);
				}
			}
			if (uae4all_hard_file[0] == '*')
				uae4all_hard_file[0] = '\0';
		}
		mainMenu_drives=nr_drives;
	
		fscanf(f,"chipmemory=%d\n",&mainMenu_chipMemory);
		fscanf(f,"slowmemory=%d\n",&mainMenu_slowMemory);
		fscanf(f,"fastmemory=%d\n",&mainMenu_fastMemory);
#ifdef ANDROIDSDL		
		fscanf(f,"onscreen=%d\n",&mainMenu_onScreen);
		fscanf(f,"onScreen_textinput=%d\n",&mainMenu_onScreen_textinput);
		fscanf(f,"onScreen_dpad=%d\n",&mainMenu_onScreen_dpad);
		fscanf(f,"onScreen_button1=%d\n",&mainMenu_onScreen_button1);
		fscanf(f,"onScreen_button2=%d\n",&mainMenu_onScreen_button2);
		fscanf(f,"onScreen_button3=%d\n",&mainMenu_onScreen_button3);
		fscanf(f,"onScreen_button4=%d\n",&mainMenu_onScreen_button4);
		fscanf(f,"onScreen_button5=%d\n",&mainMenu_onScreen_button5);
		fscanf(f,"onScreen_button6=%d\n",&mainMenu_onScreen_button6);
		fscanf(f,"custom_position=%d\n",&mainMenu_custom_position);
		fscanf(f,"pos_x_textinput=%d\n",&mainMenu_pos_x_textinput);
		fscanf(f,"pos_y_textinput=%d\n",&mainMenu_pos_y_textinput);
		fscanf(f,"pos_x_dpad=%d\n",&mainMenu_pos_x_dpad);
		fscanf(f,"pos_y_dpad=%d\n",&mainMenu_pos_y_dpad);
		fscanf(f,"pos_x_button1=%d\n",&mainMenu_pos_x_button1);
		fscanf(f,"pos_y_button1=%d\n",&mainMenu_pos_y_button1);
		fscanf(f,"pos_x_button2=%d\n",&mainMenu_pos_x_button2);
		fscanf(f,"pos_y_button2=%d\n",&mainMenu_pos_y_button2);
		fscanf(f,"pos_x_button3=%d\n",&mainMenu_pos_x_button3);
		fscanf(f,"pos_y_button3=%d\n",&mainMenu_pos_y_button3);
		fscanf(f,"pos_x_button4=%d\n",&mainMenu_pos_x_button4);
		fscanf(f,"pos_y_button4=%d\n",&mainMenu_pos_y_button4);
		fscanf(f,"pos_x_button5=%d\n",&mainMenu_pos_x_button5);
		fscanf(f,"pos_y_button5=%d\n",&mainMenu_pos_y_button5);
		fscanf(f,"pos_x_button6=%d\n",&mainMenu_pos_x_button6);
		fscanf(f,"pos_y_button6=%d\n",&mainMenu_pos_y_button6);
		fscanf(f,"quick_switch=%d\n",&mainMenu_quickSwitch);
#endif
	*/
		fclose(f);
	}

	if (isAmiga1200) { // A1200
		mainMenu_chipMemory=2;
		mainMenu_slowMemory=0;
		mainMenu_fastMemory=4;
		kickstart=3;
		mainMenu_CPU_model=1;
		mainMenu_chipset=2;
		mainMenu_CPU_speed=1;
	} else if (isAmiga600) { // A600
		mainMenu_chipMemory = 2;
		mainMenu_slowMemory = 0;
		mainMenu_fastMemory = 0;
		kickstart = 2;
		mainMenu_CPU_model = 0;
		mainMenu_chipset = 1;
		mainMenu_CPU_speed = 0;
	} else { // A500
		mainMenu_chipMemory = 1;
		mainMenu_slowMemory = 0;
		mainMenu_fastMemory = 0;
		kickstart = 1;
		mainMenu_CPU_model = 0;
		mainMenu_chipset = 0;
		mainMenu_CPU_speed = 0;
	}

	if (uae4all_hard_file[0] != '\0') {
		make_hard_file_cfg_line(uae4all_hard_file);
		mainMenu_bootHD = 2;
		__android_log_print(ANDROID_LOG_INFO, "UAE4ALL2", "boot from hard drive %s", uae4all_hard_file);
	}

	SetPresetMode(presetModeId);
	UpdateCPUModelSettings();
	UpdateChipsetSettings();
#ifdef USE_GUICHAN
  if (running==false)
#endif
  {
  	update_display();
  }
	UpdateMemorySettings();
	set_joyConf();
	reset_hdConf();
}
