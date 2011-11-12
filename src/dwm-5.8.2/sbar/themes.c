
/********* SECTION COLORS **********/

typedef struct {
  const char *normbordercolor;  // border of inactiv window
  const char *normbgcolor;      // color of statsusbar
  const char *normfgcolor;      // font color of statusbar
  const char *selbordercolor;   // border of active window 
  const char *selbgcolor;       // bgcolor of active text 
  const char *selfgcolor;       // font color of active text
  const char *botbordercolor;   // color of bottom border
  const char *cpu_line_color;   // color of timeline line
  const char *cpu_point_color;  // color of timeline point
} ColorTheme;

typedef struct {
  const char *name, *image;
  const ColorTheme normal, low, verylow;
} Theme;

static const int anzThemes = 1;

static const Theme themes[] = {
{ // Theme name, Theme Image
  "BlueAngel", "/home/john/Angel.jpg",

  // noamle
  {"#005588","#000000","#ffffff",
   "#00aaff","#000000","#00aaff",
   "#00aaff","#21536a","#00aaff"},

  // low
  {"#005588","#000000","#ffffff"
   "#00aaff","#000000","#00aaff"
   "#ff0000","#21536a","#00aaff"},

  // very low
  {"#880000","#000000","#ff0000"
   "#ff0000","#000000","#ffaa00"
   "#ff0000","#440000","#ff0000"}}
};
enum { ThemeBlueAngel, ThemeLast };
static int CurTheme = ThemeBlueAngel;
