#include <stdio.h>
#define BAR_LEN 28

const char * statusbar[BAR_LEN] = { "#000000", "#171717", "#151515", "#141414", "#131313", "#131313", "#121212", "#101010", "#0f0f0f", "#0f0f0f", "#0e0e0e", "#0d0d0d", "#0c0c0c", "#0b0b0b", "#101010", "#1c1c1c", "#1f1f1f", "#232323", "#272727", "#2a2a2a", "#2e2e2e", "#313131", "#363636", "#393939", "#3c3c3c", "#3e3e3e", "#858585", "#000000" };
unsigned long int statusbar_colors[BAR_LEN];

void calculate_colors(Display *dpy, const char ** surce, int surce_length, unsigned long int * target, int target_length);


void calculate_colors(Display *dpy, const char ** surce, int surce_length, unsigned long int * target, int target_length)
{
  Colormap cmap = DefaultColormap(dpy, DefaultScreen(dpy));
  XColor color;
  
  int i = 0;
  for(; i  < surce_length; i++)
  {
    if(!XAllocNamedColor(dpy, cmap, surce[i], &color, &color))
      printf("\n\nShit happens in i: %d\n\n",i);
    
    target[i] = color.pixel;
  }
}

