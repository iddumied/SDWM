#include "typedef.c"

#define ANZ_TEXT_BARS 1
#define ANZ_STYL_BARS 1

typedef struct {
  const int x, y;  
} Tes;

static const Test [] = {
  {1,3},
  {2,1},
  {7,4}, 
};

/*
// name of each text bar tabs
static const char** tbars_tab_names[ANZ_TEXT_BARS] {
  { "main", "term", "dev", "stuff", "else", },
}


// TODO replayde config in config.h
static const char* tbars_colors[ANZ_TEXT_BARS] {
 "#000000"; 
  
}
*/

/* includes heigth, ntabs, gb-img, Tabs(w,h,x,y,used, active) of each Bar
 * 
 * imgs must be in the xpm format.
 * default to place in :
 * 
 * ~/.dwm-sbar/bg 
 * ~/.dwm-sbar/used 
 * ~/.dwm-sbar/active
 * * * * * * * * * * * * * * * * * * * */
static ConfBar * confbar[] = {
  { 40, 5, "~/.dwm-sbar/bg/win7.xpm",
    { 
      { 60, 40, 59, 0,
        "~/.dwm-sbar/used/ie.xpm",
        "~/.dwm-sbar/active/ie_active.xpm" },
      { 60, 40, 121, 0,
        "~/.dwm-sbar/used/term.xpm",
        "~/.dwm-sbar/active/tem_active.xpm" },
      { 60, 40, 183, 0,	
	"~/.dwm-sbar/used/dev.xpm",
	"~/.dwm-sbar/active/dev_active.xpm" }
      { 60, 40, 245, 0, 
	"~/.dwm-sbar/used/play.xpm",
	"~/.dwm-sbar/active/play_active.xpm" }
      { 60, 40, 307, 0,
        "~/.dwm-sbar/used/dir.xpm",
	"~/.dwm-sbar/active/dir_active.xpm" }
     }
  }
};


  
  