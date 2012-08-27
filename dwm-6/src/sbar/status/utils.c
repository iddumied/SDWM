#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
int xtoi(const char* xs, int szlen);
void human_readable(unsigned long long bytes, Bool bits, char* result);
void human_readable_disk(unsigned long long bytes, char* result);
void readadable_seconds(unsigned long long sec, char* result);




void readadable_seconds(unsigned long long sec, char* result)
{
  int d, h, m, s;
  s = sec%60;
  m = (sec/60)%60;
  h = (sec/60/60)%24;
  d = (sec/60/60/24);


  if(d > 0)
    sprintf(result,"%dd %dh %dm %ds",d,h,m,s);
  else if(h > 0)
    sprintf(result,"%dh %dm %ds",h,m,s);
  else if(m > 0)
    sprintf(result,"%dm %ds",m,s);
  else 
    sprintf(result,"%ds",s);

}



void human_readable(unsigned long long bytes, Bool bits, char* result)
{
  const char *byte[] = { "B/s","KB/s","MB/s" };
  const char *bit[]  = { "bit/s","Kbit/s","Mbit/s" };
  
  if(bits) bytes *= 8;
  
  int n, i;
  long double calc;
  
  for(n = 0, calc = (long double)bytes; n < 2 && calc > 1000.0;calc /= 1024, n++);
  
  
  sprintf(result,"%llu %s",(unsigned long long)calc, (bits ? bit[n] : byte[n]));  
}


void human_readable_disk(unsigned long long bytes, char* result)
{
  const char *byte[] = { "B","KB","MB", "GB", "TB" };
  
  int n, i;
  long double calc;
  
  for(n = 0, calc = (long double)bytes; n < 4 && calc > 1000.0;calc /= 1024, n++);
  
  if(calc > 10)  
    sprintf(result,"%llu %s",(unsigned long long)calc, byte[n]);  
  else
    sprintf(result,"%3.1F %s",(double)(calc), byte[n]);  
}


// Converts a hexadecimal string to integer
int xtoi(const char* xs, int szlen)
{
 int i, xv, fact, result;

 if (szlen > 0)
 {
  // Converting more than 32bit hexadecimal value?
  if (szlen>8) return -2; // exit

  // Begin conversion here
  result = 0;
  fact = 1;

  // Run until no more character to convert
  for(i=szlen-1; i>=0 ;i--)
  {
   if (isxdigit(*(xs+i)))
   {
    if (*(xs+i)>=97)
    {
     xv = ( *(xs+i) - 97) + 10;
    }
    else if ( *(xs+i) >= 65)
    {
     xv = (*(xs+i) - 65) + 10;
    }
    else
    {
     xv = *(xs+i) - 48;
    }
    result += (xv * fact);
    fact *= 16;
   }
   else
   {
    // Conversion was abnormally terminated
    // by non hexadecimal digit, hence
    // returning only the converted with
    // an error value 4 (illegal hex character)
    return -4;
   }
  }
  return result;
 }

 // Nothing to convert
 return -1;
}
