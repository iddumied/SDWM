#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
int xtoi(const char* xs, int szlen);
void human_readable(int bytes, Bool bits, char *result);


void human_readable(int bytes, Bool bits, char* result)
{
  const char *byte[] = { "B/s","KB/s","MB/s" };
  const char *bit[]  = { "bit/s","Kbit/s","Mbit/s" };
  
  if(bits) bytes *= 8;
  
  
  int n, i;
  double calc;
  
  for(n = 0, calc = (double)bytes; n < 2 && calc > 1000.0;calc /= 1024, n++);
  
  
  sprintf(result,"%d %s",(int)calc, (bits ? bit[n] : byte[n]));  
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