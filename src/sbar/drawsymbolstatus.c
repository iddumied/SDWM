void drawsymbolstatus();
static XGCValues gcv;

int draw_net(int y, int pos)
{
  int buffer_net_len;
  char *symbol, buffer_net[12], buffer_speed[10];
  
  // draw net
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  // calc symbol
  if(!net.connected) {
    symbol = net_sym_offline;
    if(!(interface_by_name("wlan0")->easy_online))
      sprintf(buffer_net," down");
    else 
      sprintf(buffer_net," of");
  } else if(net_lan_online()){
    symbol = net_sym_lan_online;
    human_readable(net_all_bytes_per_sec(), False, buffer_speed);
    sprintf(buffer_net, "  %s", buffer_speed);
  }else if(net_wlan_online()){
    double strength = net_wlan_strength();
    sprintf(buffer_net," %d%%", (int)(strength*100));
    
    if(strength < 0.25)
      symbol = net_sym_wlan_very_low;
    else if(strength < 0.5)
      symbol = net_sym_wlan_low;
    else if(strength < 0.75)
      symbol = net_sym_wlan_middle;
    else
      symbol = net_sym_wlan_high;
  }else {
    sprintf(buffer_net,"Error");
    symbol = net_sym_offline;
  }
    
  buffer_net_len = strlen(buffer_net);
  pos -= textnw(buffer_net, buffer_net_len);

  if (dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, buffer_net, buffer_net_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer_net, buffer_net_len);

  pos -= sbartextnw(symbol, 1);

  if (sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, symbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, symbol, 1);
  
  return pos;
}

int draw_audio(int y, int pos)
{
  int buffer_len;
  char buffer[7], *symbol;  

  // draw audio
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  sprintf(buffer," %d%%", audio.percent);
    
  // calc symbol
  if(audio.mute)
    symbol = audio_mute;
  else if(audio.percent < 25)
    symbol = audio_low;
  else if(audio.percent < 50)
    symbol = audio_middle;
  else if(audio.percent < 75)
    symbol = audio_high;
  else
    symbol = audio_very_high;
  
  
    
  buffer_len = strlen(buffer);
  pos -= textnw(buffer, buffer_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);

  pos -= sbartextnw(symbol, 1);
  
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, symbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, symbol, 1);

 
  return pos;
}

int draw_backlight(int y, int pos)
{
  int buffer_len;
  char buffer[7], *symbol;  

  // draw backbacklight.per
  gcv.foreground = sbar.colors.yellow; 
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  sprintf(buffer," %d%%", (int)(backlight.per*100));

  // calc symbol
  if(backlight.per < 0.25)
    symbol = backlight_very_low;
  else if(backlight.per < 0.5)
    symbol = backlight_low;
  else if(backlight.per < 0.75)
    symbol = backlight_middle;
  else
    symbol = backlight_high;
  
    
  buffer_len = strlen(buffer);
  pos -= textnw(buffer, buffer_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);

  pos -= sbartextnw(symbol, 1);
  
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, symbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, symbol, 1);

  return pos;
}

int draw_termal(int y, int pos)
{
  char buffer[7];
  int buffer_len;

  // draw thermal
  sprintf(buffer," %d°C", thermal);
  
  // calculaing color
  if(thermal > 55)
    gcv.foreground = sbar.colors.red;
  else gcv.foreground = dc.norm[ColFG];
  
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    
  buffer_len = strlen(buffer);

  // update pos
  pos -= textnw(buffer, buffer_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);

  pos -= sbartextnw(termal_symbol, 1);
  
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, termal_symbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, termal_symbol, 1);
  
  return pos; 
}

int draw_memory(int y, int pos)
{
  int swap_buffer_len, mem_buffer_len, swapusedper;
  char mem_buffer[4];

  // draw swap if used
  swapusedper = (int)(memory.pswapused*100);
  if(swapusedper > 0){
    char swap_buffer[4];
    
    gcv.foreground = sbar.colors.red;
    XChangeGC(dpy, dc.gc, GCForeground, &gcv);
    
    sprintf(swap_buffer, " %d%%", swapusedper);
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
  
  sprintf(mem_buffer, " %d%%", (int)(memory.pused*100));
  mem_buffer_len = strlen(mem_buffer);
  pos -= textnw(mem_buffer, mem_buffer_len);

  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, mem_buffer, mem_buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, mem_buffer, mem_buffer_len);

  pos -= sbartextnw(memory_symbol, 1);
  
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, memory_symbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, memory_symbol, 1);
  
  return pos;
}

int draw_uptime(int y, int pos)
{
  // update pos
  pos -= textnw(uptime.uptime, uptime.len); 
  
  // draw uptime
  gcv.foreground = dc.norm[ColFG];
  XChangeGC(dpy, dc.gc, GCForeground, &gcv);

  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, uptime.uptime, uptime.len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, uptime.uptime, uptime.len);

  pos -= sbartextnw(uptime.symbol, 1); 
  
  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, uptime.symbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, uptime.symbol, 1);
  
  return pos;
}

int draw_battery(int y, int pos)
{
  int buffer_len;
  char buffer[7], *symbol;

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
     sprintf(buffer, " %02d:%02d", battery.remain.h, battery.remain.m);
     symbol = battery_power_on;
   
  }else if(battery.mode == CHARGED){
    sprintf(buffer, " Full");
     symbol = battery_power_on;
   
  }else{
    sprintf(buffer, " %02d:%02d", battery.remain.h, battery.remain.m);

    if(battery.stat < 0.25)
      symbol = battery_very_low;
    else if(battery.stat < 0.5)
      symbol = battery_low;
    else if(battery.stat < 0.75)
      symbol = battery_half_full;
    else
      symbol = battery_full;

  }

  // update pos
  buffer_len = strlen(buffer);
  pos -= textnw(buffer, buffer_len);
  
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, buffer, buffer_len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, buffer, buffer_len);

  pos -= sbartextnw(symbol, 1);

  if(sbar.font.set)
    XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, pos, y, symbol, 1);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, symbol, 1);
 
  return pos; 
}

int draw_time(int y, int pos)
{
  // update pos  
  pos -= textnw(sbar_date.date, sbar_date.len);

  // draw date and time
  if(dc.font.set)
    XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, pos, y, sbar_date.date, sbar_date.len);
  else
    XDrawString(dpy, dc.drawable, dc.gc, pos, y, sbar_date.date, sbar_date.len);
  
  return pos;
}

void drawsymbolstatus()
{
  
  if(!sbar.font.set) 
    log_str("SBar Font not initialized!!!", LOG_ERROR);


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
