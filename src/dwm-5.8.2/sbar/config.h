#define ANZ_TEXT_BARS 1
#define ANZ_STYL_BARS 1
#define MAX_TABS 9
#define SATANDART_TEXT True
#define STANADART_BAR 0
#include "typedef.c"

// order of SBar Status Symbols. first = right
SBarStatusSymbol sbar_status_symbols[] = {
//  function        active
  { draw_time,      True },
  { draw_battery,   True },
  { draw_uptime,    True },
  { draw_memory,    True },
  { draw_termal,    True },
  { draw_backlight, True },
  { draw_audio,     True },
  { draw_net,       True },
};
// must be in the same order!
enum { DrawTime, DrawBattery, DrawUptime, DrawMemory, DrawTermal, DrawBacklight, DrawAudio, DrawNet, DrawLast };


static const Bool texttopbar        = True;       // False means bottom bar
static const Bool bottomborder      = True;	      // border of text bar
  // color bottomborder    //"#a2c2cd"; //"#3b85a2"; //"#669cb6"; //"#21536a"; 
static const Bool readFromXroot     = False;      // if set sbar draws a collurfull statusbar in textmode
                                                  // with cpu and memory usage, battery clock and date
                                                  // controled by setings above
static const Bool showbattery       = True;
static const Bool cpu_in_middle     = True;       // if True cpu is placed in middel of bar
static int cpu_posx                 = 600;
static const int cpu_length         = 60;         // length of one cpu consider this if you have more
static const int distance_x         = 5;          // distance betwen mem and swap and between cpus if you have more than one
static const int distance_y         = 2;          // distance between top and bottom of bar

static const char memused_col[]      = "#00aaff";   //"#18B218";
static const char membuff_col[]      = "#ffffff";   //"#0F3FBA";
static const char memcach_col[]      = "#005588";   //"#FFFF00";

static const int status_refresh     = 1;           // refresh zyklus in seconds
static const Bool show_year         = True;

static const char mainsymbol[]      = "\x18";
static const Bool cpumem_timeline   = True;
static const Bool shift_left        = True;


static const Bool utime_seconds     = False;
static const int  tbar_distancex    = 12;

static const unsigned int gappx     = 6;
static const int max_link_quality   = 70; // max wlan quality found out by calling iwconfig

#define MAXPARTITIONS 20                    // max number of posible partitions mounted if you mount more it will crash


// satuswin
static Bool draw_status_win           = True;
static const char swnormbordercolor[] = "#005588";  // border of inactiv window
static const char swnormbgcolor[]     = "#000000";  // color of statsusbar
static const char swnormfgcolor[]     = "#ffffff";  // font color of statusbar
static const char swselbordercolor[]  = "#00aaff";  // border of active window 
static const char swselbgcolor[]      = "#000000";  // bgcolor of active text 
static const char swselfgcolor[]      = "#00aaff";  // font color of active text
static const char swbotbordercolor[]  = "#00aaff";  // color of bottom border
static const char swcpu_line_color[]  = "#21536a";  // color of cpu timeline line
static const char swcpu_point_color[] = "#00aaff";  // color of cpu timeline point
static const int  stwwidth            = 1366;

static const double blowstat        = 0.1;        // defines the warning stat of battery
static const double bverylowstat    = 0.12;      // 1 is full 0 is empty
static const double bat_suspend     = 0.075;       // percent when go to suspend

// normal color
static const char normbordercolor[] = "#005588";  // border of inactiv window
static const char normbgcolor[]     = "#000000";  // color of statsusbar
static const char normfgcolor[]     = "#ffffff";  // font color of statusbar
static const char selbordercolor[]  = "#00aaff";  // border of active window 
static const char selbgcolor[]      = "#000000";  // bgcolor of active text 
static const char selfgcolor[]      = "#00aaff";  // font color of active text
static const char botbordercolor[]  = "#00aaff";  // color of bottom border
static const char cpu_line_color[]  = "#21536a";  // color of cpu timeline line
static const char cpu_point_color[] = "#00aaff";  // color of cpu timeline point

// if battery low
static const char bnormbordercolor[] = "#005588";  // border of inactiv window
static const char bnormbgcolor[]     = "#000000";  // color of statsusbar
static const char bnormfgcolor[]     = "#ffffff";  // font color of statusbar
static const char bselbordercolor[]  = "#00aaff";  // border of active window 
static const char bselbgcolor[]      = "#000000";  // bgcolor of active text 
static const char bselfgcolor[]      = "#00aaff";  // font color of active text
static const char bbotbordercolor[]  = "#ff0000";  // color of bottom border
static const char bcpu_line_color[]  = "#21536a";  // color of cpu timeline line
static const char bcpu_point_color[] = "#00aaff";  // color of cpu timeline point

// if battery very low
static const char bbnormbordercolor[] = "#880000";  // border of inactiv window
static const char bbnormbgcolor[]     = "#000000";  // color of statsusbar
static const char bbnormfgcolor[]     = "#ff0000";  // font color of statusbar
static const char bbselbordercolor[]  = "#ff0000";  // border of active window 
static const char bbselbgcolor[]      = "#000000";  // bgcolor of active text 
static const char bbselfgcolor[]      = "#ffaa00";  // font color of active textmode
static const char bbbotbordercolor[]  = "#ff0000";  // color of bottom border
static const char bbcpu_line_color[]  = "#440000";  // color of cpu timeline line
static const char bbcpu_point_color[] = "#ff0000";  // color of cpu timeline point

