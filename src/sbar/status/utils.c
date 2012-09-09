#ifndef STATUS_UTILS
#define STATUS_UTILS
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
int xtoi(const char* xs, int szlen);
void human_readable(unsigned long long bytes, Bool bits, char* result);
void human_readable_disk(unsigned long long bytes, char* result);
void readadable_seconds(unsigned long long sec, char* result);
char *let_str_fitt_to(char *dst, char *src, int dst_len, int src_len);
char *fill_with_spaces_bevor(char *dst, char *src, int dst_len, int src_len);
char *fill_with_spaces_after(char *dst, char *src, int dst_len, int src_len);
void add_char_to_str(char *str, char chr, int len);

void add_char_to_str(char *str, char chr, int len) {
  str[len] = chr;
  str[len + 1] = (char) 0;
}

char *let_str_fitt_to(char *dst, char *src, int dst_len, int src_len) {
  if (src_len <= dst_len) return src;
  if (dst_len <= 3) return NULL;

  int i;
  for (i = 0; i < dst_len; i++) {
    if (i < dst_len - 3)
      dst[i] = src[i];
    else
      dst[i] = '.';
  }
  dst[i] = (char) 0;
  return dst;
}

char *fill_with_spaces_bevor(char *dst, char *src, int dst_len, int src_len) {
  if (dst_len <= src_len) return src;

  int i, distance = dst_len - src_len;
  for (i = 0; i < dst_len; i++) {
    if (i < distance)
      dst[i] = ' ';
    else
      dst[i] = src[i - distance];
  }
  dst[i] = (char) 0;
  return dst;
}

char *fill_with_spaces_after(char *dst, char *src, int dst_len, int src_len) {
  if (dst_len <= src_len) return src;

  int i;
  for (i = 0; i < dst_len; i++) {
    if (i < src_len)
      dst[i] = src[i];
    else
      dst[i] = ' ';
  }
  dst[i] = (char) 0;
  return dst;
}


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

/**
 * Reads an Line from an file handel
 * returns 0 when EOF is retached
 */
int get_line(int fp, char *buffer, int *len) {
  int error;
  char tmp[10];
  
  *len = 0;
  while (1) {
    error = read(fp, tmp, 1);
    if (error == 0 || error == -1) {
      error = *len;
      break;
    }
    
    buffer[*len] = tmp[0];
    if (tmp[0] == '\n') {
      error = *len;
      (*len)++;
      break;
    }
    (*len)++;
  }

  buffer[*len] = '\x00';
  return error;
}
#endif // end of STATUS_UTILS
