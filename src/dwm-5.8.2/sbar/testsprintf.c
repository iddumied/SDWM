#include <stdio.h>

int main(){
  
  FILE *fp;
  fp = popen("sleep 2;dmenu;feh --bg-fill /home/chief/Angel.jpg","r");
}
