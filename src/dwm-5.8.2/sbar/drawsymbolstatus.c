void drawsymbolstatus();
int draw_time(int y, int pos);
int draw_battery(int y, int pos);
int draw_uptime(int y, int pos);

static XGCValues gcv;

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
  double used, buffer, cached, swapused, light, audioper, wlanstat;
  char memstat[5], swapstat[4], thermalstring[6], lightsym[7], audiosym[7], netsym[13], netspeedstr[11];
  int upt, upl, battre_h, battre_m, y, h, swapusedper, pos, swapstat_len, memstat_len, temperature, thermal_len, light_len, audio_len, net_len, netspeed;
  Bool audiomute, audiophones, netonline, ethonline, walnonline;
  // catching information
  pthread_mutex_lock (&mutex);
  upt         = tbar_uptime.uptime;
  upl         =  tbar_uptime.len;
  used        = memory.pused;
  buffer      = memory.pbuffer;
  cached      = memory.pcached;
  swapused    = memory.pswapused;
  battre_h    = battery.remain.h;
  battre_m    = battery.remain.m;
  temperature = thermal;
  light       = backlight.per;
  audioper    = audio.percent;
  audiophones = audio.headphones;
  audiomute   = audio.mute;
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
  
    
  // update pos
  pos -= tbar_distancex;  
  
  // draw swap if used
  swapusedper = (int)(swapused*100);
  if(swapusedper > 0){
    gcv.foreground = sbarcolor.red;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    
    sprintf(swapstat, " %d%c", swapusedper, '%');
    swapstat_len = strlen(swapstat);
    pos -= textnw(swapstat, swapstat_len);
    
    if(dc.font.set)
      XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, swapstat, swapstat_len);
    else
      XDrawString(dpy, dc.drawable, dc.gc, pos, y, swapstat, swapstat_len);
  }
  
  // draw memory usage
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  
  sprintf(memstat, "\x1a %d%c", (int)(used*100),'%');
  memstat_len = strlen(memstat);
  pos -= textnw(memstat, memstat_len);

  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, memstat, memstat_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, memstat, memstat_len);
  
  
  // update pos
  pos -= tbar_distancex;  
  
  // draw thermal
  sprintf(thermalstring,"\x1c %d°C",temperature);
  
  // calculaing color
  if(temperature > 55)
    gcv.foreground = sbarcolor.red;
  else gcv.foreground = dc.norm[ColFG];
  
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    
  thermal_len = strlen(thermalstring);
  pos -= textnw(thermalstring, thermal_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, thermalstring, thermal_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, thermalstring, thermal_len);
  
    
  // update pos
  pos -= tbar_distancex;  
  
  // draw backlight
  gcv.foreground = sbarcolor.yellow; 
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  // calc symbol
  if(light < 0.25)
    sprintf(lightsym,"\x02 %d%c", (int)(light*100),'%');
  else if(light < 0.5)
    sprintf(lightsym,"\x1d %d%c", (int)(light*100),'%');
  else if(light < 0.75)
    sprintf(lightsym,"\x1e %d%c", (int)(light*100),'%');
  else
    sprintf(lightsym,"\x1f %d%c", (int)(light*100),'%');
  
    
  light_len = strlen(lightsym);
  pos -= textnw(lightsym, light_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, lightsym, light_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, lightsym, light_len);
  
      
  // update pos
  pos -= tbar_distancex;  
  
  // draw audio
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  // calc symbol
  if(audiomute)
    sprintf(audiosym,"\x0f %d%c", (int)(audioper*100),'%');
  else if(audiophones)
    sprintf(audiosym,"È %d%c", (int)(audioper*100),'%');
  else if(audioper < 0.25)
    sprintf(audiosym,"\x10 %d%c", (int)(audioper*100),'%');
  else if(audioper < 0.5)
    sprintf(audiosym,"\x11 %d%c", (int)(audioper*100),'%');
  else if(audioper < 0.75)
    sprintf(audiosym,"\x12 %d%c", (int)(audioper*100),'%');
  else
    sprintf(audiosym,"É %d%c", (int)(audioper*100),'%');
  
  
    
  audio_len = strlen(audiosym);
  pos -= textnw(audiosym, audio_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, audiosym, audio_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, audiosym, audio_len);
  
  
  
        
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
