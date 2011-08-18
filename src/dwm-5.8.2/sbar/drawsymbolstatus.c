void drawsymbolstatus();



void drawsymbolstatus()
{
  /*************** INITIALIZE ***************/
  XGCValues gcv;
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  
  // values
  double used, buffer, cached, swapused, battstat, light, audioper, wlanstat;
  char batttime[6], battimesym[8],memstat[5], swapstat[4], thermalstring[6], lightsym[7], audiosym[7], netsym[13], netspeedstr[11];
  int upt, upl, battre_h, battre_m, battime_len, y, h, battmode, swapusedper, pos, swapstat_len, memstat_len, temperature, thermal_len, light_len, audio_len, net_len, netspeed;
  Bool audiomute, audiophones, netonline, ethonline, walnonline;
  // catching information
  pthread_mutex_lock (&mutex);
  upt         = tbar_uptime.uptime;
  upl         =  tbar_uptime.len;
  used        = memory.pused;
  buffer      = memory.pbuffer;
  cached      = memory.pcached;
  swapused    = memory.pswapused;
  battstat    = battery.stat;
  battre_h    = battery.remain.h;
  battre_m    = battery.remain.m;
  temperature = thermal;
  battmode    = battery.mode;
  light       = backlight.per;
  audioper    = audio.percent;
  audiophones = audio.headphones;
  audiomute   = audio.mute;
  netonline   = net.connected;
  ethonline   = net.eth0.online;
  walnonline  = net.wlan0.online;
  wlanstat    = net.wlan0.strength;
  netspeed    = net.eth0.between.receive.bytes_per_sec;
  sprintf(batttime,"%02d:%02d", battery.remain.h, battery.remain.m);
  pthread_mutex_unlock (&mutex);
  
  // calculaing font values
  pos = screenWidth - textnw(tbar_date.date,tbar_date.len) - 6;
  h = dc.font.ascent + dc.font.descent;
  y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
  
  /*************** DRAWING ***************/
  // draw main symbol
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, 3, y+1, mainsymbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, 3, y+1, mainsymbol, 1);
  
  
  // draw date and time
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, tbar_date.date,tbar_date.len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, tbar_date.date,tbar_date.len);
  
  
  // calculating battery color;
  gcv.foreground = ((int)(battstat * 255)) * statusstyle.gl + 
          (255 - ((int)(battstat * 255))) * statusstyle.rl;
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  
  // calculating battery stat
  if(battmode == CHARGING){
     sprintf(battimesym, "%c %s", (char)23, batttime);
   
  }else if(battmode == CHARGED){
    sprintf(battimesym, "%c Full", (char)23);
   
  }else{
    if(battstat < 0.25)
      sprintf(battimesym, "%c %s", (char)11, batttime);
    else if(battstat < 0.5)
      sprintf(battimesym, "%c %s", (char)12, batttime);
    else if(battstat < 0.75)
      sprintf(battimesym, "%c %s", (char)13, batttime);
    else
      sprintf(battimesym, "%c %s", (char)14, batttime);

  }
  
  // update pos
  battime_len = strlen(battimesym);
  pos -= (tbar_distancex + textnw(battimesym, battime_len));
 
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, battimesym, battime_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, battimesym, battime_len);
  
  
  
  // draw uptime
  pos -= (tbar_distancex + textnw(upt, upl));
  
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, upt, upl);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y,  upt, upl);
  
  
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