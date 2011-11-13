
/********* SECTION COLORS **********/

typedef struct {
  const char *normbordercolor;  // border of inactiv window
  const char *normbgcolor;      // color of statsusbar
  const char *normfgcolor;      // font color of statusbar
  const char *selbordercolor;   // border of active window 
  const char *selbgcolor;       // bgcolor of active text 
  const char *selfgcolor;       // font color of active text
  const char *botbordercolor;   // color of bottom border
  const char *timeln_line_color;   // color of timeline line
  const char *timeln_point_color;  // color of timeline point
} ColorTheme;

typedef struct {
  const char *name, *image;
  const ColorTheme stw, normal, low, verylow;
} Theme;

static const int anzThemes = 1;

static const Theme themes[] = {
  // Theme name, Theme Image

{ "BlueAngel", "/home/chief/Angel.jpg",
  // status window
  {"#005588","#000000","#ffffff",
   "#00aaff","#000000","#00aaff",
   "#00aaff","#21536a","#00aaff"},

  // normal
  {"#005588","#000000","#ffffff",
   "#00aaff","#000000","#00aaff",
   "#00aaff","#21536a","#00aaff"},

  // low
  {"#005588","#000000","#ffffff",
   "#00aaff","#000000","#00aaff",
   "#ff0000","#21536a","#00aaff"},

  // very low
  {"#880000","#000000","#ff0000",
   "#ff0000","#000000","#ffaa00",
   "#ff0000","#440000","#ff0000"}},

{ "FireLove", "/home/chief/.dwm/FireLove.jpg",
  // status window
  {"#880000","#000000","#ff0000",
   "#ff0000","#000000","#ffaa00",
   "#ff0000","#440000","#ff0000"},

  // normal
  {"#880000","#000000","#ff0000",
   "#ff0000","#000000","#ffaa00",
   "#ff0000","#440000","#ff0000"},

  // low
  {"#005588","#000000","#ffffff",
   "#00aaff","#000000","#00aaff",
   "#ff0000","#21536a","#00aaff"},

  // very low
  {"#880000","#000000","#ff0000",
   "#ff0000","#000000","#ffaa00",
   "#ff0000","#440000","#ff0000"}}

};
enum { ThemeBlueAngel, ThemeFireLove, ThemeLast };
static int CurTheme = ThemeBlueAngel;
