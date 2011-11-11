/* See LICENSE file for copyright and license details. */

/* appearance */
static const char font[]            = /*"-nsb-lokaltog-medium-r-normal--10-100-75-75-c-60-iso10646-1";*/ "-artwiz-cureextra-medium-r-normal--11-110-75-75-p-90-iso8859-1"; /*"-gbdfed-Unknown-Medium-R-Normal--16-120-96-96-P-100-FontSpecific-0";*/// "-*-clean-*-*-*-*-*-*-*-*-*-*-*-3";//"-*-terminus-medium-r-*-*-16-*-*-*-*-*-*-*";
static const char xtermfont[]       = "-nsb-lokaltog-medium-r-normal--10-100-75-75-c-60-iso10646-1";
static const unsigned int borderpx  = 1;          /* border pixel of windows */
static const unsigned int snap      = 32;         /* snap pixel */
static const Bool showbar           = True;       /* False means no bar */
static const int nmaster            = 1;  /* default number of clients in the master area */

#include "nmaster.c"


/* tagging */
static const char *tags[] = { "main", "term", "dev", "img", "misc" };

static const Rule rules[] = {
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            True,        -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       False,       -1 },
	{ "Konsole",  NULL,       NULL,       1 << 8,       True,        -1 },
	{ "xterm",    NULL,       NULL,       1 << 8,       True,        -1 },
	{ "uxterm",   NULL,       NULL,       1 << 8,       True,        -1 },
	{ "feh",      NULL,       NULL,       1 << 8,       True,        -1 },
};

/* layout(s) */
static const float mfact      = 0.55; /* factor of master area size [0.05..0.95] */
static const Bool resizehints = False; /* True means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "\x04",      ntile },    /* first entry is default */
	{ "\x03",      NULL },    /* no layout function means floating behavior */
	{ "\x01",      monocle },
  { "\x05",      nbstack },
  { "\x08",      bstackhoriz },
  { "\x09",      spiral },
  { "\x1b",      dwindle },
  { "\x07",      gaplessgrid },
// TODO remove title and bsack
};

/* key definitions */ //Mod4Mask == Winkey
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static const char *dmenucmd[] = { "dmenu_run", "-fn", font, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbgcolor, "-sf", selfgcolor, NULL };
static const char *termcmd[]  = { "uxterm", "-bc", "-bg", selbgcolor, "-cr" ,selfgcolor, "-fg", selfgcolor, "-fn", xtermfont, NULL };

 
static Key keys[] = {
	/* modifier                     key        function        argument */
  { 0,                            XFnToggleMute,    toggle_mute,      {} },
  { 0,                            XFnAudioUp,       set_audio,        {.i = +10 } },
  { 0,                            XFnAudioDown,     set_audio,        {.i = -10 } },
  { ControlMask,                  XFnAudioUp,       set_audio,        {.i = +1 } },
  { ControlMask,                  XFnAudioDown,     set_audio,        {.i = -1 } },
  { 0,                            XfnToggleDisplay, toggle_backlight, {} },
  { 0,                            XfnBacklightUp,   set_backlight,    {.i = +1 } },
  { 0,                            XfnBacklightDown, set_backlight,    {.i = -1 } }, 
  { 0,                            XfnWlan,          toogle_wlan,      {} },
  { MODKEY,                       XK_y,             incnmaster,       {.i = +1 } },
  { MODKEY,                       XK_x,             incnmaster,       {.i = -1 } },
  { MODKEY,                       XK_z,             setnmaster,       {.i = 2 } },
  { MODKEY,                       XK_Down,          moveresize,       {.v = (int []){ 0, 25, 0, 0 }}},
  { MODKEY,                       XK_Up,            moveresize,       {.v = (int []){ 0, -25, 0, 0 }}},
  { MODKEY,                       XK_Right,         moveresize,       {.v = (int []){ 25, 0, 0, 0 }}},
  { MODKEY,                       XK_Left,          moveresize,       {.v = (int []){ -25, 0, 0, 0 }}},
  { MODKEY|ShiftMask,             XK_Down,          moveresize,       {.v = (int []){ 0, 0, 0, 25 }}},
  { MODKEY|ShiftMask,             XK_Up,            moveresize,       {.v = (int []){ 0, 0, 0, -25 }}},
  { MODKEY|ShiftMask,             XK_Right,         moveresize,       {.v = (int []){ 0, 0, 25, 0 }}},
  { MODKEY|ShiftMask,             XK_Left,          moveresize,       {.v = (int []){ 0, 0, -25, 0 }}},
  { MODKEY|ControlMask,           XK_Down,          moveresize,       {.v = (int []){ 0, 1, 0, 0 }}},
  { MODKEY|ControlMask,           XK_Up,            moveresize,       {.v = (int []){ 0, -1, 0, 0 }}},
  { MODKEY|ControlMask,           XK_Right,         moveresize,       {.v = (int []){ 1, 0, 0, 0 }}},
  { MODKEY|ControlMask,           XK_Left,          moveresize,       {.v = (int []){ -1, 0, 0, 0 }}},
  { MODKEY|ControlMask|ShiftMask, XK_Down,          moveresize,       {.v = (int []){ 0, 0, 0, 1 }}},
  { MODKEY|ControlMask|ShiftMask, XK_Up,            moveresize,       {.v = (int []){ 0, 0, 0, -1 }}},
  { MODKEY|ControlMask|ShiftMask, XK_Right,         moveresize,       {.v = (int []){ 0, 0, 1, 0 }}},
  { MODKEY|ControlMask|ShiftMask, XK_Left,          moveresize,       {.v = (int []){ 0, 0, -1, 0 }}},
  { MODKEY|ControlMask|ShiftMask, XK_p,             spawn,            SHCMD("sudo shutdown -h now")},
  { MODKEY,                       XK_u,             spawn,            SHCMD("xmodmap ~/.xmodmap")},
  { MODKEY|ControlMask|ShiftMask, XK_r,             spawn,            SHCMD("sudo reboot")},
  { MODKEY|ControlMask|ShiftMask, XK_s,             spawn,            SHCMD("slock & sudo pm-suspend")},
  { MODKEY,                       XK_l,             spawn,            SHCMD("slock")},
  { MODKEY|ShiftMask,             XK_i,             spawn,            SHCMD("tabbed")},
	{ MODKEY,                       XK_p,             spawn,            {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return,        spawn,            {.v = termcmd } },
  { MODKEY|ShiftMask,             XK_z,             spawn,            SHCMD("transset-df -a") },
	{ MODKEY,                       XK_b,             togglebar,        {0} },
	{ MODKEY|ShiftMask,             XK_b,             togglestw,        {} },
	{ MODKEY,                       XK_s,             focusstack,       {.i = +1 } },
	{ MODKEY,                       XK_a,             focusstack,       {.i = -1 } },
	{ MODKEY,                       XK_minus,         setmfact,         {.f = +0.05} },
	{ MODKEY,                       XK_plus,          setmfact,         {.f = -0.05} },
  { MODKEY|ShiftMask,             XK_minus,         movestack,        {.i = +1} },
  { MODKEY|ShiftMask,             XK_plus,          movestack,        {.i = -1} },
	{ MODKEY,                       XK_Return,        zoom,             {0} },
	{ MODKEY,                       XK_Tab,           view,             {0} },
	{ MODKEY|ShiftMask,             XK_c,             killclient,       {0} },
	{ MODKEY,                       XK_t,             setlayout,        {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,             setlayout,        {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,             setlayout,        {.v = &layouts[2]} },
  { MODKEY,                       XK_h,             setlayout,        {.v = &layouts[3]} },
  { MODKEY|ControlMask,           XK_h,             setlayout,        {.v = &layouts[4]} },
  { MODKEY,                       XK_g,             setlayout,        {.v = &layouts[5]} },
  { MODKEY|ControlMask,           XK_g,             setlayout,        {.v = &layouts[6]} },
  { MODKEY,                       XK_c,             setlayout,        {.v = &layouts[7]} },
	{ MODKEY,                       XK_space,         setlayout,        {0} },
	{ MODKEY|ShiftMask,             XK_space,         togglefloating,   {0} },
	{ MODKEY,                       XK_0,             view,             {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,             tag,              {.ui = ~0 } },
	{ MODKEY,                       XK_comma,         focusmon,         {.i = -1 } },
	{ MODKEY,                       XK_period,        focusmon,         {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,         tagmon,           {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,        tagmon,           {.i = +1 } },
	{ MODKEY,                       XK_F12,           mytest,           {} },
	TAGKEYS(                        XK_1,                               0)
	TAGKEYS(                        XK_2,                               1)
	TAGKEYS(                        XK_3,                               2)
	TAGKEYS(                        XK_4,                               3)
	TAGKEYS(                        XK_5,                               4)
	TAGKEYS(                        XK_6,                               5)
	TAGKEYS(                        XK_7,                               6)
	TAGKEYS(                        XK_8,                               7)
	TAGKEYS(                        XK_9,                               8)
	{ MODKEY|ShiftMask,             XK_q,             quit,             {0} },
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};


