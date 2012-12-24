/**
 *  vim like commands for sdwm
 */

typedef struct {
  const char *command;
  void (*func)(const char *);
} VimCommand;

typedef struct {
  char *cmd_bat_verylow, *cmd_bat_low, *cmd;
  char last_input[256];
} VimCommandUtils;

static VimCommandUtils vim_command_utils;



  
// TODO Move to config.h


/**
 * instrcutions:
 * 
 *  first argument is the command you want to use, 
 *  second argument is an function pointer handling your input. 
 *  Your function will get an char pointer to the input
 *
 *  examples:
 * 
 *   comand        function
 *  
 * { ":test ",     do_something },
 * { ":modulo ",   calc_modulo  },
 * { ":echo ",     echo_str     },
 *
 * if the input is ":test 10, ":test lala" or an other string which starts with ":test "
 * do_something is called with the input string
 *
 */
VimCommand vim_commands[] = {
 //   comand                      function
 {  ":stw disk set width ",       stw_disk_set_width },
 {  ":stw disk set width max",    stw_disk_set_width },
 {  ":stw disk set width min",    stw_disk_set_width },
 {  ":stw disk set tree right",   stw_disk_set_tree  },
 {  ":stw disk set tree left",    stw_disk_set_tree  },
 {  ":stw disk set verbose ",     stw_disk_set_verbose },


};

