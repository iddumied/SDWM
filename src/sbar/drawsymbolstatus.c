void drawsymbolstatus();
static XGCValues gcv;

int draw_net(int y, int pos)
{
  int buffer_net_len;
  char buffer_net[13], buffer_speed[11];
  
  // draw net
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  // calc symbol
  if(!net.connected)
    if(!(interface_by_name("wlan0")->easy_online))
      sprintf(buffer_net,"Ô down");
    else
      sprintf(buffer_net,"Ô of");
  else if(net_lan_online()){
    human_readable(net_all_bytes_per_sec(), False, buffer_speed);
    sprintf(buffer_net,"\x19  %s", buffer_speed);
  }else if(net_wlan_online()){
    double strength = net_wlan_strength();
    if(strength < 0.25)
      sprintf(buffer_net,"Ö %d%c", (int)(strength*100),'%');
    else if(strength < 0.5)
      sprintf(buffer_net,"\x14 %d%c", (int)(strength*100),'%');
    else if(strength < 0.75)
      sprintf(buffer_net,"\x15 %d%c", (int)(strength*100),'%');
    else
      sprintf(buffer_net,"\x16 %d%c", (int)(strength*100),'%');
  }else
    sprintf(buffer_net,"Error");
    
  buffer_net_len = strlen(buffer_net);
  pos -= textnw(buffer_net, buffer_net_len);
  
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, buffer_net, buffer_net_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer_net, buffer_net_len);
  
  return pos;
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
    sprintf(buffer,"\x0f %d%c", audio.percent,'%');
  else if(audio.percent < 25)
    sprintf(buffer,"\x10 %d%c", audio.percent,'%');
  else if(audio.percent < 50)
    sprintf(buffer,"\x11 %d%c", audio.percent,'%');
  else if(audio.percent < 75)
    sprintf(buffer,"\x12 %d%c", audio.percent,'%');
  else
    sprintf(buffer,"É %d%c", audio.percent,'%');
  
  
    
  buffer_len = strlen(buffer);
  pos -= textnw(buffer, buffer_len);
  
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);
  
 
  return pos;
}

int draw_backlight(int y, int pos)
{
  int buffer_len;
  char buffer[8];  

  // draw backbacklight.per
  gcv.foreground = sbar.colors.yellow; 
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
  
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, buffer, buffer_len);
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
    gcv.foreground = sbar.colors.red;
  else gcv.foreground = dc.norm[ColFG];
  
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    
  buffer_len = strlen(buffer);

  // update pos
  pos -= textnw(buffer, buffer_len);
  
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, buffer, buffer_len);
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
    
    gcv.foreground = sbar.colors.red;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    
    sprintf(swap_buffer, " %d%c", swapusedper, '%');
    swap_buffer_len = strlen(swap_buffer);
    pos -= textnw(swap_buffer, swap_buffer_len);
    
    if(sbar.font.set)
      XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, swap_buffer, swap_buffer_len);
    else
      XDrawString(dpy, dc.drawable, dc.gc, pos, y, swap_buffer, swap_buffer_len);
  }
  
  // draw memory usage
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  
  sprintf(mem_buffer, "\x1a %d%c", (int)(memory.pused*100),'%');
  mem_buffer_len = strlen(mem_buffer);
  pos -= textnw(mem_buffer, mem_buffer_len);

  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, mem_buffer, mem_buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, mem_buffer, mem_buffer_len);
  
  return pos;
}

int draw_uptime(int y, int pos)
{
  // update pos
  pos -= textnw(uptime.uptime, uptime.len); 
  
  // draw uptime
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, uptime.uptime, uptime.len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, uptime.uptime, uptime.len);
  
  return pos;
}

int draw_battery(int y, int pos)
{
  int buffer_len;
  char buffer[8];

  // calculating battery color;
  gcv.foreground = ((int)(battery.stat * 255)) * sbar.colors.greenlow + 
          (255 - ((int)(battery.stat * 255))) * sbar.colors.redlow;
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  #ifdef DEBUG
  char log[256];
  sprintf(log, "Draw Battery Color: %d", gcv.foreground);
  log_str(log, LOG_DEBUG);
  sprintf(log, "battery.stat %f", battery.stat);
  log_str(log, LOG_DEBUG);
  #endif
  
  // calculating battery stat
  if(battery.mode == CHARGING){
     sprintf(buffer, "%c %02d:%02d", (char)23, battery.remain.h, battery.remain.m);
   
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
  
 
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);

  return pos; 
}

int draw_time(int y, int pos)
{
  // update pos  
  pos -= textnw(tbar_date.date,tbar_date.len);

  // draw date and time
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, tbar_date.date,tbar_date.len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, tbar_date.date,tbar_date.len);
  
  return pos;
}

void drawsymbolstatus()
{
  /*************** INITIALIZE ***************/
  int i, y, h, pos;
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
  
  // calculaing font values
  pos = screenWidth - symbol_distance_x;
  h = sbar.font.ascent + sbar.font.descent;
  y = dc.y + (dc.h / 2) - (h / 2) + sbar.font.ascent;
  
  /*************** DRAWING ***************/
  
  // draw main symbol
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, 3, y+1, mainsymbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, 3, y+1, mainsymbol, 1);
  

  // drawing sbar symbols. The order is configured in config.h
  for(i = 0; i < DrawLast;i++){
    if(sbar_status_symbols[i].active) {
      pos = sbar_status_symbols[i].func(y, pos) - sbar_distancex;
    } 
    #ifdef DEBUG
      char buffer[256];
      sprintf(buffer, "drawsymbolstatus %d %s", i, sbar_status_symbols[i].active ? "yes" : "no");
      log_str(buffer, LOG_DEBUG);
    #endif
  }

}
