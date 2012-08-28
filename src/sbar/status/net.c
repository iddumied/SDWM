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
  Bool wireless;
  double strength;
  Bool online, easy_online, exists;
  char name[25];
} Interface;


typedef struct {
  Bool connected;
  int refresh, num_interfaces;
  Interface interfaces[30];
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
  Arg sarg;
  int i;
  
  get_interface_stat();
  
  for (i = 0; i < net.num_interfaces; i++)
    if (strcomp(net.interfaces[i].name, "wlan0"))
      break;

  net.interfaces[i].easy_online = !net.interfaces[i].easy_online;

  if(net.interfaces[i].easy_online)
    sarg.v = (const char*[]){ "/bin/sh", "-c",  "echo 1 > /proc/easy_wifi_kill", NULL };
  else
    sarg.v = (const char*[]){ "/bin/sh", "-c",  "echo 0 > /proc/easy_wifi_kill", NULL }; 

  spawn(&sarg);
}

Interface *interface_by_name(char *name) {
  int i;
  for (i = 0; i < net.num_interfaces; i++) {
    if (strcomp(net.interfaces[i].name, name))
      return &net.interfaces[i];
  }
  
  return NULL;
}

Bool net_lan_online() {
  int i;
  for (i = 0; i < net.num_interfaces; i++) {
    if (!net.interfaces[i].wireless && net.interfaces[i].online)
      return True;
  }
  return False;
}


int net_lan_bytes_per_sec() {
  int i, bytes_per_sec = 0;

  for (i = 0; i < net.num_interfaces; i++) {
    if (!net.interfaces[i].wireless && net.interfaces[i].online)
      bytes_per_sec += net.interfaces[i].between.receive.bytes_per_sec;
  }

  return bytes_per_sec;
}

int net_wlan_bytes_per_sec() {
  int i, bytes_per_sec = 0;

  for (i = 0; i < net.num_interfaces; i++) {
    if (net.interfaces[i].wireless && net.interfaces[i].online)
      bytes_per_sec += net.interfaces[i].between.receive.bytes_per_sec;
  }

  return bytes_per_sec;
}

int net_all_bytes_per_sec() {
  int i, bytes_per_sec = 0;

  for (i = 0; i < net.num_interfaces; i++) {
    if (net.interfaces[i].online)
      bytes_per_sec += net.interfaces[i].between.receive.bytes_per_sec;
  }

  return bytes_per_sec;
}

Bool net_wlan_online() {
  int i;
  for (i = 0; i < net.num_interfaces; i++) {
    if (net.interfaces[i].wireless && net.interfaces[i].online)
      return True;
  }
  return False;
}

double net_wlan_strength() {
  int i;
  double strength = 0.0;
  for (i = 0; i < net.num_interfaces; i++) {
    if (net.interfaces[i].wireless && net.interfaces[i].online && net.interfaces[i].strength > strength)
      strength = net.interfaces[i].strength;
  }
  return strength;
}

void new_interface(Interface *interface, char *line, int len) {
  int i, x;
  Bool start = False, started = False;
  FILE * fp;
  char * line2 = NULL;
  size_t len2 = 0;
  ssize_t read;
  char cmd[100];

  for (i = 0; i < len && i < 10; i++) {

    if (line[i] == ' ' && started) break;

    if (line[i] != ' ') {
      if (!started) x = i;
      started = True;
      interface->name[i-x] = line[i];
    }
  }
  interface->name[i-x] = '\x00';

  sprintf(cmd, "iwconfig %s", interface->name);

  fp = popen(cmd, "r");
  if (fp == NULL){
        printf("\nfailed to read iwconfig output\n");
        sbar_status_symbols[DrawNet].active = False;
        return;
  }

  if ((read = getline(&line2, &len2, fp)) != -1)
    interface->wireless = (strstr(line2, "no wireless extensions")) ? False : True;
  else {
    printf("\nfailed to read iwconfig output\n");
    sbar_status_symbols[DrawNet].active = False;
    return;
  }

  if (line2) free(line2);
  close(fp);

  if (interface->wireless) {

    sprintf(cmd, "iwconfig %s | grep \"Link Quality\"", interface->name);
    fp = popen(cmd, "r");
    if (fp == NULL){
          printf("\nfailed to read iwconfig output\n");
          sbar_status_symbols[DrawNet].active = False;
          return;
    }
 
    if ((read = getline(&line2, &len2, fp)) != -1) {
      for (i = 0; i < len2; i++) {
        if (line2[i] == '/')
          interface->linkmax = atoi(line + i + 1);

      }
    } else {
      printf("\nfailed to read iwconfig output\n");
      sbar_status_symbols[DrawNet].active = False;
      return;
    }
  }

  for(i = 0;i < len;i++){
    if (!start) {
      if (line[i] == ':')
        start = True;

      continue;
    }

    if(line[i] != ' '){
      switch(x){
        
        case 0:
          interface->last.receive.bytes = atoi(line+i);
          break;
          
        case 1:
          interface->last.receive.total = atoi(line+i);
          break;
          
        case 2:
          interface->last.receive.error = atoi(line+i);
          break;
          
        case 3:
          interface->last.receive.drop = atoi(line+i);
          break;    
          
        case 4:
          interface->last.receive.fifo = atoi(line+i);
          break;
                        
        case 5:
          interface->last.receive.frame = atoi(line+i);
          break;
                                                    
        case 6:
          interface->last.receive.compressed = atoi(line+i);
          break;

        case 7:
          interface->last.receive.multicast = atoi(line+i);
          break;
                        
        case 8:
          interface->last.transmit.bytes = atoi(line+i);
          break;
          
        case 9:
          interface->last.transmit.total = atoi(line+i);
          break;
          
        case 10:
          interface->last.transmit.error = atoi(line+i);
          break;
          
        case 11:
          interface->last.transmit.drop = atoi(line+i);
          break;    
          
        case 12:
          interface->last.transmit.fifo = atoi(line+i);
          break;
                        
        case 13:
          interface->last.receive.colls = atoi(line+i);
          break;
                        
        case 14:
          interface->last.transmit.carrier = atoi(line+i);
          break;
                        
        case 15:
          interface->last.transmit.compressed = atoi(line+i);
          break;
                  
      }
      for(;i < len;i++){ // jump to next space
        if(line[i] == ' ') break;
      }
      x++;
    }
    
  }
  
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
  net.num_interfaces = 0;
  Bool start = False;

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
    }
    net.num_interfaces++;
    if (!strstr(line, net.interfaces[i-2].name))
       new_interface(&net.interfaces[i-2], line, len);

    save = &net.interfaces[i-2].current;
    
    x = 0;
    for(j = 0;j < len;j++){
      if (!start) {
        if (line[i] == ':')
          start = True;
     
        continue;
      }

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
  for(i = 0; i < net.num_interfaces;i++){
    cur     = &net.interfaces[i].current;
    last    = &net.interfaces[i].last;
    between = &net.interfaces[i].between;
    
         
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
  int l, k, j, i = 0;
  Bool notspace = False;

  fp = fopen("/proc/net/wireless", "r");
  if (fp == NULL){
        printf("\nfailed to read /proc/net/wireless\n");
        sbar_status_symbols[DrawNet].active = False;
        return;
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    if (i < 2) {
      i++;
      continue;
    }
    for (j = 0; j < net.num_interfaces; j++) {
      if (strstr(line, net.interfaces[j].name)) {
        l = 0;
        for (k = 0; k < len; k++) {
          if (line[k] != ' ' && !notspace) {
            l++;
            notspace = True;
          } else
            notspace = False;

          if (l == 3) {
            net.interfaces[j].linkcur = atoi(line + k);
            net.interfaces[j].strength = (double)net.interfaces[j].linkcur / (double)net.interfaces[j].linkmax;
            break;
          }
        }
        break;
      }
    }
  }
  
  
  if (line) free(line);
  fclose(fp);
}


void get_interface_stat()
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  int i;
  char cmd[100];

  for (i = 0; i < net.num_interfaces; i++) {
    sprintf(cmd, "/sys/class/net/%s/operstate", net.interfaces[i].name);

    fp = fopen(cmd, "r");
    if (fp == NULL){
          printf("\nfailed to read %s\n", cmd);
          sbar_status_symbols[DrawNet].active = False;
          return;
    }
 
    while ((read = getline(&line, &len, fp)) != -1) {
      if(line[0] == 'u'){ //up
        net.interfaces[i].online = True;
      }else{ //down
        net.interfaces[i].online = False;
      }
      break;
    }
 
    if (line) free(line);
    fclose(fp);
  }
  
  fp = fopen("/proc/easy_wifi_kill", "r");
  if (fp == NULL){
        printf("\nfailed to read /proc/easy_wifi_kill\n");
        sbar_status_symbols[DrawNet].active = False;
        return;
  }

  if ((read = getline(&line, &len, fp)) != -1) {
    for (i = 0; i < net.num_interfaces; i++) {
      if (strcomp(net.interfaces[i].name, "wlan0")) {

        if(line[0] == '1'){ //up
          net.interfaces[i].easy_online = True;
        }else{ //down
          net.interfaces[i].easy_online = False;
        }
        break;
      }

    }
  }
  
  for (i = 0; i < net.num_interfaces; i++) {
    if (net.interfaces[i].online) {
      net.connected = True;
      break;
    }
  }
  
  if (line) free(line);
  fclose(fp);
}


void setup_net()
{
  net.refresh = status_refresh;
  update_net();
}
