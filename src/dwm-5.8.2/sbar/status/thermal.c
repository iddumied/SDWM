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
         return;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
      line[2] = '.'; // we onley wants tho read the first two numbers
      pthread_mutex_lock (&mutex);
      thermal = atoi(line);
      pthread_mutex_unlock (&mutex);
      break;
    }

    if (line) free(line);
    fclose(fp);
}
