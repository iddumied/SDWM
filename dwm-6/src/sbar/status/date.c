//#include <stdio.h>
//#include <string.h>
#include <time.h>

void update_date();
void setup_date();

typedef struct {
  char date[35];
  int len;
} TBarDate;

static const char weekdays[7][3] = { "So\x00", "Mo\x00", "Di\x00", "Mi\x00", "Do\x00", "Fr\x00", "Sa\x00" };
static TBarDate tbar_date;


void update_date()
{
  time_t t = time(NULL);
  struct tm *ts = localtime(&t);
  char buffer[80];
  
  if(show_year)
    strftime(buffer, 80, "%d.%m. %Y - %H:%M:%S", ts);
  else
    strftime(buffer, 80, "%d.%m. - %H:%M:%S", ts);
  
  sprintf(tbar_date.date, "%s %s", weekdays[ts->tm_wday], buffer);
  tbar_date.len =  strlen(tbar_date.date);
}
