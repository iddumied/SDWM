#define ANZ_TEXT_BARS 1
#define ANZ_STYL_BARS 1
#define MAX_TABS 9
#include "typedef.c"

#ifdef NF310_A01

// order of SBar Status Symbols. first = right
SBarStatusSymbol sbar_status_symbols[] = {
  //  function      active
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

#else

// order of SBar Status Symbols. first = right
SBarStatusSymbol sbar_status_symbols[] = {
  //  function      active
  { draw_time,      True },
  { draw_battery,   False },
  { draw_uptime,    True },
  { draw_memory,    True },
  { draw_termal,    False },
  { draw_audio,     True },
  { draw_net,       True },
};

// must be in the same order!
enum { DrawTime, DrawBattery, DrawUptime, DrawMemory, DrawTermal, DrawAudio, DrawNet, DrawLast };
#endif

/* tagging */
static const char *tags[] = { "main", "term", "dev", "img", "misc", "more" };

static const Layout layouts[] = {
  /* symbol     arrange function */
  { "\x02",      ntile },    /* first entry is default */
  { "\x03",      NULL },     /* no layout function means floating behavior */
  { "\x04",      monocle },
  { "\x05",      nbstack },
  { "\x06",      nbstackhoriz },
  { "\x07",      spiral },
  { "\x07",      dwindle },
  { "\x08",      gaplessgrid },
};



static const Bool sbartopbar          = True;         // False means bottom bar
static const int cpus_length          = 255;          // length of all cpu timelines
static const Bool cpu_shift_left      = True;         // Shift cpu timeline left (else right)
static const int cpu_distance_x       = 5;            // distance betwen cpu-cores if you have more than one
static const int symbol_distance_y    = 2;            // distance between symbols and top and bottom of bar
static const int symbol_distance_x    = 6;            // distance between two symbols on the x line
static const int timeline_length      = 100;          // length of net transmit and recive and of disk read and write timelines
static Bool show_net_lo_interface     = True;         // shows statistic (Timlie) for the lo interface in the Statuswindow
                                      
static const int status_refresh       = 1;            // refresh zyklus in seconds
static const Bool show_year           = True;

/**
 * Font and symbols for the sbar
 * edit font with gbdfed and change symbol chars
 */
static const char sbarfont[]               = "-lokaltog-symbols-medium-r-normal--11-110-75-75-p-90-iso8859-1"; // font with the sbar status Symbols 
static const char mainsymbol[]             = "\x01"; 
static const char battery_very_low[]       = "\x0a";
static const char battery_low[]            = "\x0b";
static const char battery_half_full[]      = "\x0c";
static const char battery_full[]           = "\x0d";
static const char battery_power_on[]       = "\x0e";
static const char net_sym_offline[]        = "\x0f";
static const char net_sym_lan_online[]     = "\x10";
static const char net_sym_wlan_very_low[]  = "\x11";
static const char net_sym_wlan_low[]       = "\x12";
static const char net_sym_wlan_middle[]    = "\x13";
static const char net_sym_wlan_high[]      = "\x14";
static const char audio_mute[]             = "\x15";
static const char audio_low[]              = "\x16";
static const char audio_middle[]           = "\x17";
static const char audio_high[]             = "\x18";
static const char audio_very_high[]        = "\x19";
static const char uptime_sym_evil[]        = "\x1a";
static const char uptime_sym_berserk[]     = "\x1b";
static const char uptime_sym_angry[]       = "\x1c";
static const char uptime_sym_sad[]         = "\x1d";
static const char uptime_sym_lazy[]        = "\x1e";
static const char uptime_sym_good[]        = "\x1f";
static const char backlight_very_low[]     = "\x20";
static const char backlight_low[]          = "\x21";
static const char backlight_middle[]       = "\x22";
static const char backlight_high[]         = "\x23";
static const char termal_symbol[]          = "\x24";
static const char memory_symbol[]          = "\x25";


                                      
static const Bool utime_seconds       = False;
static const int  sbar_distancex      = 12;
                                      
static const unsigned int gappx       = 6;
static const int max_link_quality     = 70;           // max wlan quality found out by calling iwconfig

static const int uptime_good          = 60 * 60 * 1;  // uptime seconds wen feelings changing
static const int uptime_lazy          = 60 * 60 * 2;
static const int uptime_sad           = 60 * 60 * 4;
static const int uptime_angry         = 60 * 60 * 6;
static const int uptime_berserk       = 60 * 60 * 8;
static const int uptime_evil          = 60 * 60 * 10;


#define MAXPARTITIONS 50                              // max number of posible partitions mounted if you mount more it will propably crash
#define MAX_NET_INTERFACES 30                         // max number of possible net_interfaces if you have more it will propably crash


/******* SECTION INVORMATIONS *******/

// statuswin
static Bool draw_status_win           = True;
static int status_win_width           = 20;

static const double blowstat          = 0.1;          // defines the warning stat of battery
static const double bverylowstat      = 0.12;         // 1 is full 0 is empty
static const double bat_suspend       = 0.075;        // percent when go to suspend
static unsigned int stw_disk_verbose  = 5;            // verbose level of status window disk
static Bool stw_disk_tree_right       = False;        // print tree of stw left or right 
static const unsigned int stw_gappx   = 6;            // border between screen and statuswin
                                                      // (and in block mode the gapp betwen left and righti)





/********* SECTION COLORS **********/

// normal color
static const char normbordercolor[]   = "#005588";  // border of inactiv window
static const char normbgcolor[]       = "#000000";  // color of statsusbar
static const char normfgcolor[]       = "#ffffff";  // font color of statusbar
static const char selbordercolor[]    = "#00aaff";  // border of active window 
static const char selbgcolor[]        = "#000000";  // bgcolor of active text 
static const char selfgcolor[]        = "#00aaff";  // font color of active text
static const char botbordercolor[]    = "#00aaff";  // color of bottom border
static const char cpu_line_color[]    = "#21536a";  // color of cpu timeline line
static const char cpu_point_color[]   = "#00aaff";  // color of cpu timeline point

#include "themes.c"

/********* SECTION VIM LIKE COMMANDS **********/

#include "sdwm_vim.c"

