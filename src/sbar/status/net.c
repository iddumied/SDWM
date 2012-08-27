void setup_net();
void get_interface_stat();
void get_wireless_sterngth();
void get_net_statistic();
void toogle_wlan();
void update_net();

typedef struct {
  int bytes, total, error, drop, fifo, frame, colls, compressed, carrier, multicast;
  int bytes_per_sec;
} Packgages;

typedef struct {
  Packgages receive, transmit;
} Statistic;


typedef struct {
  Statistic last, current, between;
  int linkcur, linkmax, bytespersec;
  double strength;
  Bool online, easy_online, exists;
} Wlan;

typedef struct {
  int bytespersec;
  Statistic last, current, between;
  Bool online, exists;  
} LAN;


typedef struct {
  Bool connected;
  int refresh;
  Wlan wlan0;
  LAN eth0, lo;
} Net;

Net net;


void update_net()
{
  get_interface_stat();
  get_wireless_sterngth();
  get_net_statistic();
  

}

void toogle_wlan()
{
  Bool easy_on;
  Arg sarg;
  
  get_interface_stat();
  
  easy_on = !net.wlan0.easy_online;
  net.wlan0.easy_online = easy_on;

  if(easy_on)
    sarg.v = (const char*[]){ "/bin/sh", "-c",  "echo 1 > /proc/easy_wifi_kill", NULL };
  else
    sarg.v = (const char*[]){ "/bin/sh", "-c",  "echo 0 > /proc/easy_wifi_kill", NULL }; 

  spawn(&sarg);
}


void get_net_statistic()
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  Statistic *save, *last, *cur, *between;
  int i, j, x;
  i = j = x = 0;

  fp = fopen("/proc/net/dev", "r");
  if (fp == NULL){
        printf("\nfailed to read /proc/net/dev\n");
        sbar_status_symbols[DrawNet].active = False;
        return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if(i < 2){
      i++;
      continue;
    }else if(i == 2)
      save = &net.lo.current;
    else if(i == 3)
      save = &net.eth0.current;
    else if(i == 4)
      save = &net.wlan0.current;
    
    x = 0;
    for(j = 7;j < len;j++){
      if(line[j] != ' '){
        switch(x){
          
          case 0:
            save->receive.bytes = atoi(line+j);
            break;
            
          case 1:
            save->receive.total = atoi(line+j);
            break;
            
          case 2:
            save->receive.error = atoi(line+j);
            break;
            
          case 3:
            save->receive.drop = atoi(line+j);
            break;    
            
          case 4:
            save->receive.fifo = atoi(line+j);
            break;
                          
          case 5:
            save->receive.frame = atoi(line+j);
            break;
                                                      
          case 6:
            save->receive.compressed = atoi(line+j);
            break;

          case 7:
            save->receive.multicast = atoi(line+j);
            break;
                          
          case 8:
            save->transmit.bytes = atoi(line+j);
            break;
            
          case 9:
            save->transmit.total = atoi(line+j);
            break;
            
          case 10:
            save->transmit.error = atoi(line+j);
            break;
            
          case 11:
            save->transmit.drop = atoi(line+j);
            break;    
            
          case 12:
            save->transmit.fifo = atoi(line+j);
            break;
                          
          case 13:
            save->receive.colls = atoi(line+j);
            break;
                          
          case 14:
            save->transmit.carrier = atoi(line+j);
            break;
                          
          case 15:
            save->transmit.compressed = atoi(line+j);
            break;
                    
        }
        if(x == 15) break;
        for(;j < len;j++){ // jump to next space
          if(line[j] == ' ') break;
        }
        x++;
      }
      
    }
    if(i == 4) break;
    i++;
  }
  
  if (line) free(line);
  fclose(fp);
  
  
  // claculate new and bytes per second
  for(i = 0; i < 3;i++){
    if(i == 0){ // eth0
      cur     = &net.lo.current;
      last    = &net.lo.last;
      between = &net.lo.between;
    }else if(i == 1){
      cur     = &net.eth0.current;
      last    = &net.eth0.last;
      between = &net.eth0.between;
    }else if(i == 2){
      cur     = &net.wlan0.current;
      last    = &net.wlan0.last;
      between = &net.wlan0.between;
    }
    
         
    between->receive.bytes       = cur->receive.bytes       - last->receive.bytes;
    between->receive.total       = cur->receive.total       - last->receive.total;
    between->receive.error       = cur->receive.error       - last->receive.error;
    between->receive.drop        = cur->receive.drop        - last->receive.drop;
    between->receive.fifo        = cur->receive.fifo        - last->receive.fifo;
    between->receive.frame       = cur->receive.frame       - last->receive.frame;
    between->receive.compressed  = cur->receive.compressed  - last->receive.compressed;
    between->receive.multicast   = cur->receive.multicast   - last->receive.multicast;
    between->transmit.bytes      = cur->transmit.bytes      - last->transmit.bytes;
    between->transmit.total      = cur->transmit.total      - last->transmit.total;
    between->transmit.error      = cur->transmit.error      - last->transmit.error;
    between->transmit.drop       = cur->transmit.drop       - last->transmit.drop;
    between->transmit.fifo       = cur->transmit.fifo       - last->transmit.fifo;
    between->transmit.colls      = cur->transmit.colls      - last->transmit.colls;
    between->transmit.carrier    = cur->transmit.carrier    - last->transmit.carrier;
    between->transmit.compressed = cur->transmit.compressed - last->transmit.compressed;
    
    last->receive.bytes       = cur->receive.bytes;
    last->receive.total       = cur->receive.total;
    last->receive.error       = cur->receive.error;
    last->receive.drop        = cur->receive.drop;
    last->receive.fifo        = cur->receive.fifo;
    last->receive.frame       = cur->receive.frame;
    last->receive.compressed  = cur->receive.compressed;
    last->receive.multicast   = cur->receive.multicast;
    last->transmit.bytes      = cur->transmit.bytes;
    last->transmit.total      = cur->transmit.total;
    last->transmit.error      = cur->transmit.error;
    last->transmit.drop       = cur->transmit.drop;
    last->transmit.fifo       = cur->transmit.fifo;
    last->transmit.colls      = cur->transmit.colls;
    last->transmit.carrier    = cur->transmit.carrier;
    last->transmit.compressed = cur->transmit.compressed;
    
    between->receive.bytes_per_sec  = between->receive.bytes  / net.refresh;
    between->transmit.bytes_per_sec = between->transmit.bytes / net.refresh;
  }
}


void get_wireless_sterngth()
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i = 0;

  fp = fopen("/proc/net/wireless", "r");
  if (fp == NULL){
        printf("\nfailed to read /proc/net/wireless\n");
        sbar_status_symbols[DrawNet].active = False;
        return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if(i == 2){ //up
      net.wlan0.linkcur = atoi(line+15);
      break;
    }
    i++;
  }
  
  net.wlan0.strength = (double)net.wlan0.linkcur / (double)net.wlan0.linkmax;
  
  if (line) free(line);
  fclose(fp);
}


void get_interface_stat()
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen("/sys/class/net/wlan0/operstate", "r");
  if (fp == NULL){
        printf("\nfailed to read /sys/class/net/wlan0/operstate\n");
        sbar_status_symbols[DrawNet].active = False;
        return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if(line[0] == 'u'){ //up
      net.wlan0.online = True;
    }else{ //down
      net.wlan0.online = False;
    }
    break;
  }

  fclose(fp);
  
  fp = fopen("/proc/easy_wifi_kill", "r");
  if (fp == NULL){
        printf("\nfailed to read /proc/easy_wifi_kill\n");
        sbar_status_symbols[DrawNet].active = False;
        return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if(line[0] == '1'){ //up
      net.wlan0.easy_online = True;
    }else{ //down
      net.wlan0.easy_online = False;
    }
    break;
  }

  fclose(fp);
  
  fp = fopen("/sys/class/net/eth0/operstate", "r");
  if (fp == NULL){
        printf("\nfailed to read /sys/class/net/eth0/operstate\n");
        sbar_status_symbols[DrawNet].active = False;
        return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if(line[0] == 'u'){ //up
      net.eth0.online = True;
    }else{ //down
      net.eth0.online = False;
    }
    break;
  }
  
  net.connected = ((net.eth0.online || net.wlan0.online) ? True : False);
  
  if (line) free(line);
  fclose(fp);
}


void setup_net()
{
  net.wlan0.linkmax = max_link_quality;
  net.refresh = status_refresh;
  
  
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  Statistic *save, *last, *cur, *between;
  int i, j, x;
  i = j = x = 0;

  fp = fopen("/proc/net/dev", "r");
  if (fp == NULL){
        printf("\nfailed to read /proc/net/dev\n");
        sbar_status_symbols[DrawNet].active = False;
        return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if(i == 0) // eth0
      save = &net.lo.last;
    else if(i == 1)
      save = &net.eth0.last;
    else if(i == 2)
      save = &net.wlan0.last;
      
    for(i = 7;i < len;i++){
      if(line[i] != ' '){
        switch(x){
          
          case 0:
            save->receive.bytes = atoi(line+i);
            break;
            
          case 1:
            save->receive.total = atoi(line+i);
            break;
            
          case 2:
            save->receive.error = atoi(line+i);
            break;
            
          case 3:
            save->receive.drop = atoi(line+i);
            break;    
            
          case 4:
            save->receive.fifo = atoi(line+i);
            break;
                          
          case 5:
            save->receive.frame = atoi(line+i);
            break;
                                                      
          case 6:
            save->receive.compressed = atoi(line+i);
            break;

          case 7:
            save->receive.multicast = atoi(line+i);
            break;
                          
          case 8:
            save->transmit.bytes = atoi(line+i);
            break;
            
          case 9:
            save->transmit.total = atoi(line+i);
            break;
            
          case 10:
            save->transmit.error = atoi(line+i);
            break;
            
          case 11:
            save->transmit.drop = atoi(line+i);
            break;    
            
          case 12:
            save->transmit.fifo = atoi(line+i);
            break;
                          
          case 13:
            save->receive.colls = atoi(line+i);
            break;
                          
          case 14:
            save->transmit.carrier = atoi(line+i);
            break;
                          
          case 15:
            save->transmit.compressed = atoi(line+i);
            break;
                    
        }
        for(;i < len;i++){ // jump to next space
          if(line[i] == ' ') break;
        }
        x++;
      }
      
    }
    i++;
  }
  
  if (line) free(line);
  fclose(fp);
  
  update_net();
}
