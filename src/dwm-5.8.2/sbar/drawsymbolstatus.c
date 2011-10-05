void drawsymbolstatus();
int draw_time(int y, int pos);
int draw_battery(int y, int pos);
int draw_uptime(int y, int pos);
int draw_memory(int y, int pos);
int draw_termal(int y, int pos);
int draw_backlight(int y, int pos);
int draw_audio(int y, int pos);
int draw_net(int y, int pos);

static XGCValues gcv;

int draw_net(int y, int pos)
{

}

int draw_audio(int y, int pos)
{
  int buffer_len;
  char buffer[8];  

  // draw audio
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  // calc symbol
  if(audio.mute)
    sprintf(buffer,"\x0f %d%c", (int)(audio.percent*100),'%');
  else if(audio.headphones)
    sprintf(buffer,"È %d%c", (int)(audio.percent*100),'%');
  else if(audio.percent < 0.25)
    sprintf(buffer,"\x10 %d%c", (int)(audio.percent*100),'%');
  else if(audio.percent < 0.5)
    sprintf(buffer,"\x11 %d%c", (int)(audio.percent*100),'%');
  else if(audio.percent < 0.75)
    sprintf(buffer,"\x12 %d%c", (int)(audio.percent*100),'%');
  else
    sprintf(buffer,"É %d%c", (int)(audio.percent*100),'%');
  
  
    
  buffer_len = strlen(buffer);
  pos -= textnw(buffer, buffer_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);
  
 
  return pos;
}

int draw_backlight(int y, int pos)
{
  int buffer_len;
  char buffer[8];  

  // draw backbacklight.per
  gcv.foreground = sbarcolor.yellow; 
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  // calc symbol
  if(backlight.per < 0.25)
    sprintf(buffer,"\x02 %d%c", (int)(backlight.per*100),'%');
  else if(backlight.per < 0.5)
    sprintf(buffer,"\x1d %d%c", (int)(backlight.per*100),'%');
  else if(backlight.per < 0.75)
    sprintf(buffer,"\x1e %d%c", (int)(backlight.per*100),'%');
  else
    sprintf(buffer,"\x1f %d%c", (int)(backlight.per*100),'%');
  
    
  buffer_len = strlen(buffer);
  pos -= textnw(buffer, buffer_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);
  
  return pos;
}


int draw_termal(int y, int pos)
{
  char buffer[8];
  int buffer_len;

  // draw thermal
  sprintf(buffer,"\x1c %d°C", thermal);
  
  // calculaing color
  if(thermal > 55)
    gcv.foreground = sbarcolor.red;
  else gcv.foreground = dc.norm[ColFG];
  
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    
  buffer_len = strlen(buffer);

  // update pos
  pos -= textnw(buffer, buffer_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);
  
  return pos; 
}

int draw_memory(int y, int pos)
{
  int swap_buffer_len, mem_buffer_len, swapusedper;
  char mem_buffer[5];

  // draw swap if used
  swapusedper = (int)(memory.pswapused*100);
  if(swapusedper > 0){
    char swap_buffer[4];
    
    gcv.foreground = sbarcolor.red;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    
    sprintf(swap_buffer, " %d%c", swapusedper, '%');
    swap_buffer_len = strlen(swap_buffer);
    pos -= textnw(swap_buffer, swap_buffer_len);
    
    if(dc.font.set)
      XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, swap_buffer, swap_buffer_len);
    else
      XDrawString(dpy, dc.drawable, dc.gc, pos, y, swap_buffer, swap_buffer_len);
  }
  
  // draw memory usage
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  
  sprintf(mem_buffer, "\x1a %d%c", (int)(memory.pused*100),'%');
  mem_buffer_len = strlen(mem_buffer);
  pos -= textnw(mem_buffer, mem_buffer_len);

  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, mem_buffer, mem_buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, mem_buffer, mem_buffer_len);
  
  return pos;
}

int draw_uptime(int y, int pos)
{
  // update pos
  pos -= textnw(tbar_uptime.uptime, tbar_uptime.len); 
  
  // draw uptime
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, tbar_uptime.uptime, tbar_uptime.len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y,tbar_uptime.uptime, tbar_uptime.len);
  
  return pos;
}

int draw_battery(int y, int pos)
{
  int buffer_len;
  char buffer[8];

  // calculating battery color;
  gcv.foreground = ((int)(battery.stat * 255)) * statusstyle.gl + 
          (255 - ((int)(battery.stat * 255))) * statusstyle.rl;
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  
  // calculating battery stat
  if(battery.mode == CHARGING){
     sprintf(buffer, "%c %s", (char)23, battery.remain.h, battery.remain.m);
   
  }else if(battery.mode == CHARGED){
    sprintf(buffer, "%c Full", (char)23);
   
  }else{
    if(battery.stat < 0.25)
      sprintf(buffer, "%c %02d:%02d", (char)11, battery.remain.h, battery.remain.m);
    else if(battery.stat < 0.5)
      sprintf(buffer, "%c %02d:%02d", (char)12, battery.remain.h, battery.remain.m);
    else if(battery.stat < 0.75)
      sprintf(buffer, "%c %02d:%02d", (char)13, battery.remain.h, battery.remain.m);
    else
      sprintf(buffer, "%c %02d:%02d", (char)14, battery.remain.h, battery.remain.m);

  }

  // update pos
  buffer_len = strlen(buffer);
  pos -= textnw(buffer, buffer_len);
  
 
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);

  return pos; 
}


int draw_time(int y, int pos)
{
  // update pos  
  pos -= textnw(tbar_date.date,tbar_date.len);

  // draw date and time
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, tbar_date.date,tbar_date.len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, tbar_date.date,tbar_date.len);
  
  return pos;
}

void drawsymbolstatus()
{
  /*************** INITIALIZE ***************/
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  
  // values
  double used, buffer, cached, swapused, wlanstat;
  char thermalstring[6], netsym[13], netspeedstr[11];
  int y, h,pos, memstat_len, temperature, thermal_len, net_len, netspeed;
  Bool netonline, ethonline, walnonline;
  // catching information
  pthread_mutex_lock (&mutex);
  used        = memory.pused;
  buffer      = memory.pbuffer;
  cached      = memory.pcached;
  swapused    = memory.pswapused;
  netonline   = net.connected;
  ethonline   = net.eth0.online;
  walnonline  = net.wlan0.online;
  wlanstat    = net.wlan0.strength;
  netspeed    = net.eth0.between.receive.bytes_per_sec;
  pthread_mutex_unlock (&mutex);
  
  // calculaing font values
  pos = screenWidth - 6;                                        //abstand als variable in config.h auslagern
  h = dc.font.ascent + dc.font.descent;
  y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
  
  /*************** DRAWING ***************/
  

  // draw main symbol
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, 3, y+1, mainsymbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, 3, y+1, mainsymbol, 1);
  
  pos = draw_time(y, pos) - tbar_distancex;  
  pos = draw_battery(y, pos) - tbar_distancex;
  pos = draw_uptime(y, pos) - tbar_distancex; 
  pos = draw_memory(y, pos) - tbar_distancex; 
  pos = draw_termal(y, pos) - tbar_distancex; 
  pos = draw_backlight(y, pos) - tbar_distancex; 
  pos = draw_audio(y, pos) - tbar_distancex; 

  
  
  
 
  
        
  // update pos
  pos -= tbar_distancex;  
  
  // draw net
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  // calc symbol
  // TODO interface off includen via Fn-key
  if(!netonline)
    sprintf(netsym,"Ô offline");
  else if(ethonline){
    human_readable(netspeed, False, netspeedstr);
    sprintf(netsym,"\x19  %s", netspeedstr);
  }else if(walnonline){
    if(wlanstat < 0.25)
      sprintf(netsym,"\x13 %d%c", (int)(wlanstat*100),'%');
    else if(wlanstat < 0.5)
      sprintf(netsym,"\x14 %d%c", (int)(wlanstat*100),'%');
    else if(wlanstat < 0.75)
      sprintf(netsym,"\x15 %d%c", (int)(wlanstat*100),'%');
    else
      sprintf(netsym,"\x16 %d%c", (int)(wlanstat*100),'%');
  }else
    sprintf(netsym,"Error");
    
  net_len = strlen(netsym);
  pos -= textnw(netsym, net_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, netsym, net_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, netsym, net_len);
  
  
}
