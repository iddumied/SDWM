void get_thermal();

static int thermal;

void get_thermal()
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i;


    fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fp == NULL){
         printf("\nfailed to read /sys/class/thermal/thermal_zone0/temp\n");
         sbar_status_symbols[DrawTermal].active = False;
         return;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
      line[2] = '.'; // we onley wants tho read the first two numbers
      thermal = atoi(line);
      break;
    }

    if (line) free(line);
    fclose(fp);
}
