typedef struct {
  int len;
  char * tab;
} TTab;

typedef struct {
  int w, h, ntabs;
  Pixmap drawable;
  TTab * tabs;
} TBar;


typedef struct {
  int w, h, pos_x, Pos_y;
  Pixmap un_used;
  Pixmap used;
  Pixmap active;
} STab;


typedef struct {
  int w, h, ntabs;
  Pixmap drawable;
  Tab * tabs;  
} SBar;
