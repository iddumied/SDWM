//#define DEBUG
//#define INFO
#define NF310_A01

/* See LICENSE file for copyright and license details.
 *
 * stylish dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance.  Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of sdwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag.  Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define TAGMASK                 ((1 << LENGTH(tags)) - 1)
#define TEXTW(X)                (textnw(X, strlen(X)) + dc.font.height)
#define SBARTEXTW(X)            (sbartextnw(X, strlen(X)) + sbar.font.height)

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast };              /* cursor */
enum { ColBorder, ColFG, ColBG, ColLast };                    /* color */
enum { SBarCpuPoint, SBarLine, SBarBorder, SBarLast };     /* colorss */
enum { NetSupported, NetWMName, NetWMState,
       NetWMFullscreen, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetLast };                      /* EWMH atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkMainSymbol, ClkLast };    /* clicks */
enum {MaxMon = 9};

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client {
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	unsigned int tags;
	Bool isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	Client *next;
	Client *snext;
	Monitor *mon;
	Window win;
};

typedef struct {
	int x, y, w, h;
	unsigned long norm[ColLast];
	unsigned long sel[ColLast];
  unsigned long sbar[SBarLast];
	Drawable drawable;
	GC gc;
	struct {
		int ascent;
		int descent;
		int height;
		XFontSet set;
		XFontStruct *xfont;
	} font;
} DC; /* draw context */

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

struct Monitor {
	char ltsymbol[16];
	float mfact;
	int num;
	int by;               /* bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	Bool showbar;
	Bool topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Window barwin;
  Window statuswin;
	const Layout *lt[2];
  int curtag;
  int prevtag;
  const Layout *lts[10];
  double mfacts[10];
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	Bool isfloating;
	int monitor;
} Rule;

/* function declarations */
// from patches and me
static void moveresize(const Arg *arg);
static void attachaside(Client *c);        // next window not in master area
static void nbstackhoriz(Monitor *m);       // ===
void spiral(Monitor *mon);                 // [@]
void dwindle(Monitor *mon);                // [//]
void fibonacci(Monitor *mon, int s);       // calculationg of [@] and [//]
void gaplessgrid(Monitor *m);              // [#]
void movestack(const Arg *arg);            // move window in stack
int get_stackposition(Client *c, Client *stack);
int get_next_stackposition(Client* sel, Client* stack);
static void incnmaster(const Arg *arg);
static void setnmaster(const Arg *arg);
static void ntile(Monitor *m);
static void nbstack(Monitor *m);
static void black_floading();
static void black_floadquit();
// SBar Staus Symbol functions
int draw_time(int y, int pos);
int draw_battery(int y, int pos);
int draw_uptime(int y, int pos);
int draw_memory(int y, int pos);
int draw_termal(int y, int pos);
int draw_backlight(int y, int pos);
int draw_audio(int y, int pos);
int draw_net(int y, int pos);
void dmenucmd();
void termcmd();
void custom_suspend();
void custom_shutdown();
void custom_reboot();
int sbartextnw(const char *text, unsigned int len);
void sbardrawtext(const char *text, unsigned long col[ColLast], Bool invert);
void setup_vim();
void vimcmd();
void stw_disk_set_width(const char *input);
void stw_disk_set_tree(const char *input);
void stw_disk_set_verbose(const char *input);
void run_vim_command(const char *input);
void log_str(const char *str, unsigned int importance);
#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_WARNING 2
#define LOG_ERROR   3
#ifdef DEBUG
#ifndef INFO
#define INFO
#endif
#endif



// dwm functions
static void applyrules(Client *c);
static Bool applysizehints(Client *c, int *x, int *y, int *w, int *h, Bool interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clearurgent(Client *c);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(void);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
static void die(const char *errstr, ...);
static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m);
static void drawbars(void);
static void drawsquare(Bool filled, Bool empty, Bool invert, unsigned long col[ColLast]);
static void drawtext(const char *text, unsigned long col[ColLast], Bool invert);
static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void focus(Client *c);
static void focusin(XEvent *e);
static void focusmon(const Arg *arg);
static void focusstack(const Arg *arg);
static unsigned long getcolor(const char *colstr);
static Bool getrootptr(int *x, int *y);
static long getstate(Window w);
static Bool gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, Bool focused);
static void grabkeys(void);
static void initfont(const char *fontstr);
static void keypress(XEvent *e);
static void killclient(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(Monitor *m);
static void motionnotify(XEvent *e);
static void movemouse(const Arg *arg);
static Client *nexttiled(Client *c);
static void pop(Client *);
static void propertynotify(XEvent *e);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void resize(Client *c, int x, int y, int w, int h, Bool interact);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void restack(Monitor *m);
static void run(void);
static void scan(void);
static Bool sendevent(Client *c, Atom proto);
static void sendmon(Client *c, Monitor *m);
static void setclientstate(Client *c, long state);
static void setfocus(Client *c);
static void setfullscreen(Client *c, Bool fullscreen);
static void setlayout(const Arg *arg);
static void setmfact(const Arg *arg);
static void setup(void);
static void showhide(Client *c);
static void sigchld(int unused);
static void spawn(Arg *arg);
static void tag(const Arg *arg);
static void tagmon(const Arg *arg);
static int textnw(const char *text, unsigned int len);
static void togglebar(const Arg *arg);
static void togglefloating(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void unfocus(Client *c, Bool setfocus);
static void unmanage(Client *c, Bool destroyed);
static void unmapnotify(XEvent *e);
static Bool updategeom(void);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updatenumlockmask(void);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatewindowtype(Client *c);
static void updatetitle(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void zoom(const Arg *arg);

/* variables */
static const char broken[] = "broken";
static char stext[256];
static int screen;
static int sw, sh;           /* X display screen geometry width, height */
static int bh, blw = 0;      /* bar geometry */
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = clientmessage,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast];
static Bool running = True;
static Cursor cursor[CurLast];
static Display *dpy;
static DC dc;
static Monitor *mons = NULL, *selmon = NULL;
static Window root;
static int globalborder;
static int nmasters[MaxMon];
static int initnm = 0;

/* configuration, allows nested code to access above variables */
#include "./sbar/sbar.c"
#include "config.h"
#include "./sbar/drawstatus.c"

// PATHCH FUNCTIONS

static void
initnmaster(void) {
	int i;

	if(initnm)
		return;
	for(i = 0; i < MaxMon; i++)
		nmasters[i] = nmaster;
	initnm = 1;
}

static void
incnmaster(const Arg *arg) {
	if(!arg || !selmon->lt[selmon->sellt]->arrange || selmon->curtag > MaxMon)
		return;
	nmasters[selmon->curtag-1] += arg->i;
	if(nmasters[selmon->curtag-1] < 0)
		nmasters[selmon->curtag-1] = 0;
	arrange(NULL);
}

static void
setnmaster(const Arg *arg) {
	if(!arg || !selmon->lt[selmon->sellt]->arrange || selmon->curtag > MaxMon)
		return;
	nmasters[selmon->curtag-1] = arg->i > 0 ? arg->i : 0;
	arrange(NULL);
}

static void
ntile(Monitor *m) {
	int x, y, h, w, mw, nm;
	unsigned int i, n;
	Client *c;

	initnmaster();
	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	c = nexttiled(m->clients);
	nm = m->curtag <= MaxMon ? nmasters[m->curtag-1] : nmaster;
	if(nm > n)
		nm = n;
	/* master */
	if(nm > 0) {
		mw = m->mfact * m->ww;
		h = m->wh / nm;
		if(h < bh)
			h = m->wh;
		y = m->wy;
		for(i = 0; i < nm; i++, c = nexttiled(c->next)) {
			resize(c, m->wx, y, (n == nm ? m->ww : mw) - 2 * c->bw,
			       ((i + 1 == nm) ? m->wy + m->wh - y : h) - 2 * c->bw, False);
			if(h != m->wh)
				y = c->y + HEIGHT(c);
		}
		n -= nm;
	} else
		mw = 0;
	if(n == 0)
		return;
	/* tile stack */
	x = m->wx + mw;
	y = m->wy;
	w = m->ww - mw;
	h = m->wh / n;
	if(h < bh)
		h = m->wh;
	for(i = 0; c; c = nexttiled(c->next), i++) {
		resize(c, x, y, w - 2 * c->bw,
		       ((i + 1 == n) ? m->wy + m->wh - y : h) - 2 * c->bw, False);
		if(h != m->wh)
			y = c->y + HEIGHT(c);
	}
}

static void
nbstack(Monitor *m) {
	int x, y, h, w, mh, nm;
	unsigned int i, n;
	Client *c;

	initnmaster();
	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	c = nexttiled(m->clients);
	nm = m->curtag <= MaxMon ? nmasters[m->curtag-1] : nmaster;
	if(nm > n)
		nm = n;
	/* master */
	if(nm > 0) {
		mh = m->mfact * m->wh;
		w = m->ww / nm;
		if(w < bh)
			w = m->ww;
		x = m->wx;
		for(i = 0; i < nm; i++, c = nexttiled(c->next)) {
			resize(c, x, m->wy, ((i + 1 == nm) ? m->wx + m->ww - x : w) - 2 * c->bw,
			       (n == nm ? m->wh : mh) - 2 * c->bw, False);
			if(w != m->ww)
				x = c->x + WIDTH(c);
		}
		n -= nm;
	} else
		mh = 0;
	if(n == 0)
		return;
	/* tile stack */
	x = m->wx;
	y = m->wy + mh;
	w = m->ww / n;
	h = m->wh - mh;
	if(w < bh)
		w = m->ww;
	for(i = 0; c; c = nexttiled(c->next), i++) {
		resize(c, x, y, ((i + 1 == n) ? m->wx + m->ww - x : w) - 2 * c->bw,
		       h - 2 * c->bw, False);
		if(w != m->ww)
			x = c->x + WIDTH(c);
	}
}

static void
nbstackhoriz(Monitor *m) {
  int x, y, h, w, mh, nm;
  unsigned int i, n;
  Client *c;

  initnmaster();
	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	c = nexttiled(m->clients);
	nm = m->curtag <= MaxMon ? nmasters[m->curtag-1] : nmaster;
	if(nm > n)
		nm = n;
	/* master */
	if(nm > 0) {
		mh = m->mfact * m->wh;
		w = m->ww / nm;
		if(w < bh)
			w = m->ww;
		x = m->wx;
		for(i = 0; i < nm; i++, c = nexttiled(c->next)) {
			resize(c, x, m->wy, ((i + 1 == nm) ? m->wx + m->ww - x : w) - 2 * c->bw,
			       (n == nm ? m->wh : mh) - 2 * c->bw, False);
			if(w != m->ww)
				x = c->x + WIDTH(c);
		}
		n -= nm;
	} else
		mh = 0;
	if(n == 0)
		return;
  /* tile stack */
	x = m->wx;
	y = m->wy + mh;
	w = m->ww;
	h = m->wh - mh;
  h /= n;
  if(h < bh)
    h = m->wh;
  for(i = 0; c;  c = nexttiled(c->next), i++) {
     resize(c, x, y, w - 2 * c->bw, /* remainder */ ((i + 1 == n)
            ? m->wy + m->wh - y - 2 * c->bw : h - 2 * c->bw), False);
     if(h != m->wh)
      y = c->y + HEIGHT(c);
  }
}

void
movestack(const Arg *arg) {
 Client *c = NULL, *p = NULL, *pc = NULL, *i;

 if(arg->i > 0) {
   /* find the client after selmon->sel */
   for(c = selmon->sel->next; c && (!ISVISIBLE(c) || c->isfloating); c = c->next);
   if(!c)
     for(c = selmon->clients; c && (!ISVISIBLE(c) || c->isfloating); c = c->next);

 }
 else {
   /* find the client before selmon->sel */
   for(i = selmon->clients; i != selmon->sel; i = i->next)
     if(ISVISIBLE(i) && !i->isfloating)
       c = i;
   if(!c)
     for(; i; i = i->next)
       if(ISVISIBLE(i) && !i->isfloating)
         c = i;
 }
 /* find the client before selmon->sel and c */
 for(i = selmon->clients; i && (!p || !pc); i = i->next) {
   if(i->next == selmon->sel)
     p = i;
   if(i->next == c)
     pc = i;
 }

 /* swap c and selmon->sel selmon->clients in the selmon->clients list */
 if(c && c != selmon->sel) {
   Client *temp = selmon->sel->next==c?selmon->sel:selmon->sel->next;
   selmon->sel->next = c->next==selmon->sel?c:c->next;
   c->next = temp;

   if(p && p != c)
     p->next = c;
   if(pc && pc != selmon->sel)
     pc->next = selmon->sel;

   if(selmon->sel == selmon->clients)
     selmon->clients = c;
   else if(c == selmon->clients)
     selmon->clients = selmon->sel;

   arrange(selmon);
 }
}


void
gaplessgrid(Monitor *m) {
  unsigned int n, cols, rows, cn, rn, i, cx, cy, cw, ch;
  Client *c;

  for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next))
    n++;
  if(n == 0)
    return;

  /* grid dimensions */
  for(cols = 0; cols <= n/2; cols++)
    if(cols*cols >= n)
      break;
  if(n == 5) /* set layout against the general calculation: not 1:2:2, but 2:3 */
    cols = 2;
  rows = n/cols;

  /* window geometries */
  cw = cols ? m->ww / cols : m->ww;
  cn = 0; /* current column number */
  rn = 0; /* current row number */
  for(i = 0, c = nexttiled(m->clients); c; i++, c = nexttiled(c->next)) {
    if(i/rows + 1 > cols - n%cols)
      rows = n/cols + 1;
    ch = rows ? m->wh / rows : m->wh;
    cx = m->wx + cn*cw;
    cy = m->wy + rn*ch;
    resize(c, cx, cy, cw - 2 * c->bw, ch - 2 * c->bw, False);
    rn++;
    if(rn >= rows) {
      rn = 0;
      cn++;
    }
  }
}

void
fibonacci(Monitor *mon, int s) {
 unsigned int i, n, nx, ny, nw, nh;
 Client *c;

 for(n = 0, c = nexttiled(mon->clients); c; c = nexttiled(c->next), n++);
 if(n == 0)
   return;
 
 nx = mon->wx;
 ny = 0;
 nw = mon->ww;
 nh = mon->wh;
 
 for(i = 0, c = nexttiled(mon->clients); c; c = nexttiled(c->next)) {
   if((i % 2 && nh / 2 > 2 * c->bw)
      || (!(i % 2) && nw / 2 > 2 * c->bw)) {
     if(i < n - 1) {
       if(i % 2)
         nh /= 2;
       else
         nw /= 2;
       if((i % 4) == 2 && !s)
         nx += nw;
       else if((i % 4) == 3 && !s)
         ny += nh;
     }
     if((i % 4) == 0) {
       if(s)
         ny += nh;
       else
         ny -= nh;
     }
     else if((i % 4) == 1)
       nx += nw;
     else if((i % 4) == 2)
       ny += nh;
     else if((i % 4) == 3) {
       if(s)
         nx += nw;
       else
         nx -= nw;
     }
     if(i == 0)
     {
       if(n != 1)
         nw = mon->ww * mon->mfact;
       ny = mon->wy;
     }
     else if(i == 1)
       nw = mon->ww - nw;
     i++;
   }
   resize(c, nx, ny, nw - 2 * c->bw, nh - 2 * c->bw, False);
 }
}

void
dwindle(Monitor *mon) {
 fibonacci(mon, 1);
}

void
spiral(Monitor *mon) {
 fibonacci(mon, 0);
}

void
attachaside(Client *c) {
 Client *at = nexttiled(c->mon->clients);;
 if(c->mon->sel == NULL || c->mon->sel->isfloating || !at) {
   attach(c);
   return;
 }
 c->next = at->next;
 at->next = c;
}

static void moveresize(const Arg *arg) {
XEvent ev;
Monitor *m = selmon;

  if(!(m->sel && arg && arg->v))
    return;
  if(m->lt[m->sellt]->arrange && !m->sel->isfloating){
    
    Arg newarg;

    // if not floating use Keys to navigate in stack
    switch(((int *)arg->v)[4]) {
      case 1: // MODKEY + DOWN
        newarg.i = +1;
        focusstack(&newarg);
        break;
      case 2: // MODKEY + UP
        newarg.i = -1;
        focusstack(&newarg);
        break;
      case 3: // MODKEY + RIGHT
        newarg.i = -1;
        incnmaster(&newarg);
        break;
      case 4: // MODKEY + LEFT
        newarg.i = +1;
        incnmaster(&newarg);
        break;
      case 10: // MODKEY + SHIFT + DOWN
        newarg.i = +1;
        movestack(&newarg);
        break;
      case 20: // MODKEY + SHIFT + UP
        newarg.i = -1;
        movestack(&newarg);
        break;
    }
     
  }else{
  resize(m->sel, m->sel->x + ((int *)arg->v)[0],
    m->sel->y + ((int *)arg->v)[1],
    m->sel->w + ((int *)arg->v)[2],
    m->sel->h + ((int *)arg->v)[3],
    True);
  }
  while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));

}

void black_floading()
{
  srand(time(NULL));
  int *pointx;
  int *pointy;
  int *ary;
  int i, j, p1, p2, ra1, ra2;
  GC gc = XCreateGC(dpy, DefaultRootWindow(dpy), 0, NULL);
  XGCValues gcv;
  gcv.foreground = sbar.colors.red;
  XChangeGC(dpy, gc, GCForeground, &gcv);

  pointx = (int*)malloc(sizeof(int)*screenWidth);
  pointy = (int*)malloc(sizeof(int)*screenWidth);
  ary = (int*)malloc(sizeof(int)*screenWidth);
 
  for(i = 0; i < screenWidth;i++){
     pointx[i] = 0;
     pointy[i] = bh;
     ary[i] = i;
  }
 
  for (i = 0; i < screenWidth; i++) {
    XDrawPoint(dpy, root, gc, i, bh-1);
    usleep(500);
    XFlush(dpy);
  }

  gcv.foreground = sbar.colors.black;
  XChangeGC(dpy, gc, GCForeground, &gcv);

  for(i = 0; i < screenWidth*2;i++){
 
    if(i < screenWidth){  
      p1 = (int)(rand()%(screenWidth-i));
      ra1 = ary[p1];
 
      for(j = p1; j < (screenWidth-i-1);j++)
        ary[j] = ary[j+1];
      
      pointx[i] = ra1;
    }
 
    int x = (i+1 < screenWidth) ? (i+1) : (screenWidth);
 
    for(j = 0; j < x;j++){
      XDrawPoint(dpy, root, gc, pointx[j], pointy[j]);
      
      pointy[j]++;
    }
    
  }
  usleep(1000);
}

void black_floadquit()
{
  Client *c;

  // destroying all Clients
	for(c = selmon->stack; c; c = c->snext)
    XDestroyWindow(dpy,c->win);

  black_floading();
  quit(NULL);
}

void custom_suspend()
{
  popen("sudo pm-suspend", "w");

  int i;
  GC gc = XCreateGC(dpy, DefaultRootWindow(dpy), 0, NULL);
  XGCValues gcv;
  gcv.foreground = sbar.colors.red;
  XChangeGC(dpy, gc, GCForeground, &gcv);
 
  for (i = 0; i < screenWidth; i++) {
    XDrawPoint(dpy, root, gc, i, bh-1);
    usleep(2000);
    XFlush(dpy);
  }
  sleep(3);
}

void custom_shutdown()
{
  Client *c;

  // destroying all Clients
	for(c = selmon->stack; c; c = c->snext)
    XDestroyWindow(dpy,c->win);

  black_floading();
  popen("sudo shutdown -h now", "w");
  sleep(5);
}

void custom_reboot()
{
  Client *c;

  // destroying all Clients
	for(c = selmon->stack; c; c = c->snext)
    XDestroyWindow(dpy,c->win);

  black_floading();
  popen("sudo reboot", "w");
  sleep(5);
}

void log_str(const char *str, unsigned int importance)
{
  time_t rawtime;
  struct tm * timeinfo;
  char curtime[80];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(curtime, 80, "[%d/%b/%Y::%H:%M:%S] ", timeinfo);

  int fd = open("/var/log/sdwm", O_APPEND|O_WRONLY);
  if (fd == -1) {
    printf("[ERROR] couldn't open logfile /var/log/sdwm\n");
    if (importance == LOG_DEBUG)
      printf("[DD]%s %s", curtime, str);
    else if (importance == LOG_INFO)
      printf("[II]%s %s", curtime, str);
    else if (importance == LOG_WARNING)
      printf("[WW]%s %s", curtime, str);
    else
      printf("[EE]%s %s", curtime, str);

    return;
  }

  if (importance == LOG_DEBUG)
    write(fd, "[DD]", 4);
  else if (importance == LOG_INFO)
    write(fd, "[II]", 4);
  else if (importance == LOG_WARNING)
    write(fd, "[WW]", 4);
  else
    write(fd, "[EE]", 4);

  write(fd, curtime, strlen(curtime));
  write(fd, str, strlen(str));
  write(fd, "\n", 1);
  close(fd);
}

/**
 * Vim Commands functions
 */

void setup_vim() {

  int i, commands_len = 64;
  for (i = 0; i < LENGTH(vim_commands); i++)
    commands_len += strlen(vim_commands[i].command) + 1;

  char *commands = (char *) malloc(sizeof(char) * commands_len);
  sprintf(commands, "%s", "xsetroot -name \"VIMCMD: $(echo '");

  for (i = 0; i < LENGTH(vim_commands); i++) {
    strcat(commands, "\n");
    strcat(commands, vim_commands[i].command);
  }
  strcat(commands, "'");

  int cmd_len = (sizeof(char) * (commands_len + strlen(font) + strlen(themes[CurTheme].verylow.normbgcolor) * 4 + 64));
  vim_command_utils.cmd_bat_verylow = (char *) malloc(cmd_len);
  vim_command_utils.cmd_bat_low = (char *) malloc(cmd_len);
  vim_command_utils.cmd = (char *) malloc(cmd_len);

  sprintf(vim_command_utils.cmd_bat_verylow, "%s | dmenu -fn '%s' -nb '%s' -nf '%s' -sb '%s' -sf '%s')\"", commands,
          font, themes[CurTheme].verylow.normbgcolor,
          themes[CurTheme].verylow.normfgcolor, 
          themes[CurTheme].verylow.selbgcolor, 
          themes[CurTheme].verylow.selfgcolor);

  sprintf(vim_command_utils.cmd_bat_low, "%s | dmenu -fn '%s' -nb '%s' -nf '%s' -sb '%s' -sf '%s')\"", commands,
          font, themes[CurTheme].low.normbgcolor,
          themes[CurTheme].low.normfgcolor, 
          themes[CurTheme].low.selbgcolor, 
          themes[CurTheme].low.selfgcolor);

  sprintf(vim_command_utils.cmd, "%s | dmenu -fn '%s' -nb '%s' -nf '%s' -sb '%s' -sf '%s')\"", commands,
          font, themes[CurTheme].normal.normbgcolor,
          themes[CurTheme].normal.normfgcolor, 
          themes[CurTheme].normal.selbgcolor, 
          themes[CurTheme].normal.selfgcolor);

}

void vimcmd() {
  if(sbar_status_symbols[DrawBattery].active && battery.stat <= bverylowstat && !battery.adapter){
    popen(vim_command_utils.cmd_bat_verylow, "w");

  }else if(sbar_status_symbols[DrawBattery].active && battery.stat <= blowstat && !battery.adapter){
    popen(vim_command_utils.cmd_bat_low, "w");

  }else{
    popen(vim_command_utils.cmd, "w");

  }
}

void run_vim_command(const char *input) {

  int i;
  for (i = 0; i < LENGTH(vim_commands); i++) {
    if (!strncmp(input, vim_commands[i].command, strlen(vim_commands[i].command))) {
      vim_commands[i].func(input);
      return;
    }
  }

}

/**
 * vim command functions 
 */

void stw_disk_set_width(const char *input) {

  #ifdef DEBUG
    char log_buff[256];
    sprintf(log_buff, "entered %s [DSW]", __func__);
    log_str(log_buff, LOG_DEBUG);
  #endif

  int i, j;
 

  if (stw_disk_verbose == 5) {
    status_win_width = 266 - stw_gappx; 
  } else {
  
    if (!strcmp(input, ":stw disk set width max")) 
      status_win_width = diskstat_utils.max_status_win_width;
    else if (!strcmp(input, ":stw disk set width min")) 
      status_win_width = diskstat_utils.min_status_win_width;
    else
      sscanf(input, ":stw disk set width %d", &status_win_width);
  
    #ifdef DEBUG
      sprintf(log_buff, "[DSW] new width %d", status_win_width);
      log_str(log_buff, LOG_DEBUG);
      sprintf(log_buff, "[DSW] min width %d max width %d", diskstat_utils.min_status_win_width, 
                                                        diskstat_utils.max_status_win_width);
      log_str(log_buff, LOG_DEBUG);
    #endif
 
 
    if (status_win_width < diskstat_utils.min_status_win_width) {
      char logbuff[64];
      sprintf(logbuff, "status_win_width (%d) to smal => set to %d", status_win_width, diskstat_utils.min_status_win_width);
      log_str(logbuff, LOG_WARNING);
 
      status_win_width = diskstat_utils.min_status_win_width;
    } 
 
    // init prefixes
    sprintf(diskstat_utils.read_prefix, "%s", "read: ");
    sprintf(diskstat_utils.write_prefix,"%s", "write: ");
 
    int max_prefix = textnw(diskstat_utils.write_prefix, strlen(diskstat_utils.write_prefix))
                      + textnw(diskstat_utils.read_prefix, strlen(diskstat_utils.read_prefix));
 
    #ifdef DEBUG
      sprintf(log_buff, "[DSW] max_prefix %d", max_prefix);
      log_str(log_buff, LOG_DEBUG);
    #endif

    if (status_win_width > diskstat_utils.max_status_win_width) {
      char logbuff[64];
      sprintf(logbuff, "status_win_width (%d) to big => set to %d", status_win_width, diskstat_utils.min_status_win_width);
      log_str(logbuff, LOG_WARNING);
 
      status_win_width = diskstat_utils.max_status_win_width;
    } 
 
    if ((max_prefix + diskstat_utils.min_status_win_width) > status_win_width) {
      sprintf(diskstat_utils.read_prefix, "%s", "r: ");
      sprintf(diskstat_utils.write_prefix,"%s", "w: ");
 
      max_prefix = textnw(diskstat_utils.write_prefix, strlen(diskstat_utils.write_prefix))
                    + textnw(diskstat_utils.read_prefix, strlen(diskstat_utils.read_prefix));
      #ifdef DEBUG
        sprintf(log_buff, "[DSW] max_prefix %d", max_prefix);
        log_str(log_buff, LOG_DEBUG);
      #endif
 
      if ((max_prefix + diskstat_utils.min_status_win_width) > status_win_width) {
        diskstat_utils.read_prefix[0]  = (char) 0;
        diskstat_utils.write_prefix[0] = (char) 0;
      }
    }
 
    sprintf(diskstat_utils.readed_prefix, "%s", "readed: ");
    sprintf(diskstat_utils.written_prefix,"%s", "written: ");
 
    int max2_prefix = textnw(diskstat_utils.written_prefix, strlen(diskstat_utils.written_prefix))
                       + textnw(diskstat_utils.readed_prefix, strlen(diskstat_utils.readed_prefix));

    #ifdef DEBUG
      sprintf(log_buff, "[DSW] max2_prefix %d", max2_prefix);
      log_str(log_buff, LOG_DEBUG);
    #endif
 
    if ((max2_prefix + diskstat_utils.readed_width) > status_win_width) {
      sprintf(diskstat_utils.readed_prefix, "%s", "r: ");
      sprintf(diskstat_utils.written_prefix,"%s", "w: ");
    }
 
    sprintf(diskstat_utils.free_prefix, "%s", "free: ");
    int free_prefix_len = textnw(diskstat_utils.free_prefix, strlen(diskstat_utils.free_prefix));

    #ifdef DEBUG
      sprintf(log_buff, "[DSW] free_prefix_len %d", free_prefix_len);
      log_str(log_buff, LOG_DEBUG);
    #endif
 
    if ((free_prefix_len + diskstat_utils.free_width) * 2 > status_win_width) {
      diskstat_utils.free_prefix[0]  = (char) 0;
    }
  }

  #ifdef DEBUG
    sprintf(log_buff, "[DSW] status_win_width / 2 - stw_gappx %d", status_win_width / 2 - stw_gappx);
    log_str(log_buff, LOG_DEBUG);
  #endif

  // init diskstat (Distk Timelines and stuff)
  for(i = 0; i < MAXPARTITIONS;i++){
    if (stw_disk_verbose == 5) {
      diskstat[i].length      = 100;
    } else {
      diskstat[i].length      = status_win_width / 2 - stw_gappx;
    }

    for(j = 0;j < diskstat[i].length;j++){
      diskstat[i].read.bytes[i]  = 0;
      diskstat[i].write.bytes[i] = 0;
    }
    diskstat[i].read.max  = 0;
    diskstat[i].write.max = 0;
    diskstat[i].writeges  = 0;
    diskstat[i].readges   = 0;
  }



  diskstat_utils.line_seperator[0] = '-';
  diskstat_utils.line_seperator[1] = (char) 0;
  int line_seperator_len = textnw(diskstat_utils.line_seperator, 1);

  #ifdef DEBUG
    sprintf(log_buff, "[DSW] line_seperator_len %d", line_seperator_len);
    log_str(log_buff, LOG_DEBUG);
  #endif
  
  for (i = 1; i < status_win_width / line_seperator_len; i++)
    add_char_to_str(diskstat_utils.line_seperator, '-', i);

  diskstat_utils.line_seperator[i - 1] = (char) 0;

  diskstat_utils.max_chrs_per_line = status_win_width / diskstat_utils.max_char_len;
  diskstat_utils.max_chrs_per_halfln = (status_win_width / 2) / diskstat_utils.max_char_len;

  #ifdef DEBUG
    sprintf(log_buff, "[DSW] max_char_len %d status_win_width %d max_chrs_per_line %d max_chrs_per_halfln %d", 
              diskstat_utils.max_char_len, status_win_width, diskstat_utils.max_chrs_per_line, 
              diskstat_utils.max_chrs_per_halfln );
    log_str(log_buff, LOG_DEBUG);
  #endif

}

void stw_disk_set_tree(const char *input) {
  if (!strcmp(input, ":stw disk set tree right")) 
    stw_disk_tree_right = True;
  else if (!strcmp(input, ":stw disk set tree left"))
    stw_disk_tree_right = False;
}

void stw_disk_set_verbose(const char *input) {
  
  int verbose = -1;
  sscanf(input, ":stw disk set verbose %d", &verbose);

  if (verbose > 5) verbose = 5;
  if (verbose < 0) verbose = 0;

  stw_disk_verbose = verbose;

  stw_disk_set_width("stw disk set width max");
}



// DWM FUNCTIONS

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

/* function implementations */
void
applyrules(Client *c) {
	const char *class, *instance;
	unsigned int i;
	const Rule *r;
	Monitor *m;
	XClassHint ch = { NULL, NULL };

	/* rule matching */
	c->isfloating = c->tags = 0;
	XGetClassHint(dpy, c->win, &ch);
	class    = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name  ? ch.res_name  : broken;

	for(i = 0; i < LENGTH(rules); i++) {
		r = &rules[i];
		if((!r->title || strstr(c->name, r->title))
		&& (!r->class || strstr(class, r->class))
		&& (!r->instance || strstr(instance, r->instance)))
		{
			c->isfloating = r->isfloating;
			c->tags |= r->tags;
			for(m = mons; m && m->num != r->monitor; m = m->next);
			if(m)
				c->mon = m;
		}
	}
	if(ch.res_class)
		XFree(ch.res_class);
	if(ch.res_name)
		XFree(ch.res_name);
	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

Bool
applysizehints(Client *c, int *x, int *y, int *w, int *h, Bool interact) {
	Bool baseismin;
	Monitor *m = c->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if(interact) {
		if(*x > sw)
			*x = sw - WIDTH(c);
		if(*y > sh)
			*y = sh - HEIGHT(c);
		if(*x + *w + 2 * c->bw < 0)
			*x = 0;
		if(*y + *h + 2 * c->bw < 0)
			*y = 0;
	}
	else {
		if(*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);
		if(*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);
		if(*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if(*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}
	if(*h < bh)
		*h = bh;
	if(*w < bh)
		*w = bh;
	if(resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if(!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect limits */
		if(c->mina > 0 && c->maxa > 0) {
			if(c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if(c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}
		if(baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for increment value */
		if(c->incw)
			*w -= *w % c->incw;
		if(c->inch)
			*h -= *h % c->inch;
		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if(c->maxw)
			*w = MIN(*w, c->maxw);
		if(c->maxh)
			*h = MIN(*h, c->maxh);
	}
	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void
arrange(Monitor *m) {
	if(m)
		showhide(m->stack);
	else for(m = mons; m; m = m->next)
		showhide(m->stack);
	if(m)
		arrangemon(m);
	else for(m = mons; m; m = m->next)
		arrangemon(m);
}

void
arrangemon(Monitor *m) {
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
	if(m->lt[m->sellt]->arrange)
		m->lt[m->sellt]->arrange(m);
	restack(m);
}

void
attach(Client *c) {
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void
attachstack(Client *c) {
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void
buttonpress(XEvent *e) {
	unsigned int i, x, click;
	Arg arg = {0};
	Client *c;
	Monitor *m;
	XButtonPressedEvent *ev = &e->xbutton;

	click = ClkRootWin;
	/* focus monitor if necessary */
	if((m = wintomon(ev->window)) && m != selmon) {
		unfocus(selmon->sel, True);
		selmon = m;
		focus(NULL);
	}
  if(ev->window == selmon->barwin || ev->window == selmon->statuswin) {
      
    i = 0;
    x = SBARTEXTW(mainsymbol);
      
    do { 
       // calculates on witch tab mous was pressed
       x = x + TEXTW(tags[i]);

		} while(ev->x >= x && ++i < LENGTH(tags));
    if(ev->x < TEXTW(mainsymbol))
      click = ClkMainSymbol;
    else if(i < (LENGTH(tags))){
		  click = ClkTagBar;
		  arg.ui = 1 << i;
		}
		else if(ev->x < x + blw)
			click = ClkLtSymbol;
		else if(ev->x > selmon->ww - TEXTW(stext))
			click = ClkStatusText;
		else
			click = ClkWinTitle;
	}
	else if((c = wintoclient(ev->window))) {
		focus(c);
		click = ClkClientWin;
	}
	for(i = 0; i < LENGTH(buttons); i++)
		if(click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
}

void
checkotherwm(void) {
	xerrorxlib = XSetErrorHandler(xerrorstart);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void) {
	Arg a = {.ui = ~0};
	Layout foo = { "", NULL };
	Monitor *m;

	view(&a);
	selmon->lt[selmon->sellt] = &foo;
	for(m = mons; m; m = m->next)
		while(m->stack)
			unmanage(m->stack, False);
	if(dc.font.set)
		XFreeFontSet(dpy, dc.font.set);
	else
		XFreeFont(dpy, dc.font.xfont);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	XFreePixmap(dpy, dc.drawable);
	XFreeGC(dpy, dc.gc);
	XFreeCursor(dpy, cursor[CurNormal]);
	XFreeCursor(dpy, cursor[CurResize]);
	XFreeCursor(dpy, cursor[CurMove]);
	while(mons)
		cleanupmon(mons);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
}

void
cleanupmon(Monitor *mon) {
	Monitor *m;

	if(mon == mons)
		mons = mons->next;
	else {
		for(m = mons; m && m->next != mon; m = m->next);
		m->next = mon->next;
	}
	XUnmapWindow(dpy, mon->barwin);
  XUnmapWindow(dpy, mon->statuswin);
  XDestroyWindow(dpy, mon->barwin);
  XDestroyWindow(dpy, mon->statuswin);
	free(mon);
}

void
clearurgent(Client *c) {
	XWMHints *wmh;

	c->isurgent = False;
	if(!(wmh = XGetWMHints(dpy, c->win)))
		return;
	wmh->flags &= ~XUrgencyHint;
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void
clientmessage(XEvent *e) {
	XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);

	if(!c)
		return;
	if(cme->message_type == netatom[NetWMState]) {
		if(cme->data.l[1] == netatom[NetWMFullscreen] || cme->data.l[2] == netatom[NetWMFullscreen])
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */
			              || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
	}
	else if(cme->message_type == netatom[NetActiveWindow]) {
		if(!ISVISIBLE(c)) {
			c->mon->seltags ^= 1;
			c->mon->tagset[c->mon->seltags] = c->tags;
		}
		pop(c);
	}
}

void
configure(Client *c) {
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
configurenotify(XEvent *e) {
	Monitor *m;
	XConfigureEvent *ev = &e->xconfigure;
	Bool dirty;

	if(ev->window == root) {
		dirty = (sw != ev->width);
		sw = ev->width;
		sh = ev->height;
		if(updategeom() || dirty) {
			if(dc.drawable != 0)
				XFreePixmap(dpy, dc.drawable);
			dc.drawable = XCreatePixmap(dpy, root, sw, bh, DefaultDepth(dpy, screen));
			updatebars();
			for(m = mons; m; m = m->next)
				XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, m->ww, bh);
			focus(NULL);
			arrange(NULL);
		}
	}
}

void
configurerequest(XEvent *e) {
	Client *c;
	Monitor *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if((c = wintoclient(ev->window))) {
		if(ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if(c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
			m = c->mon;
			if(ev->value_mask & CWX) {
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}
			if(ev->value_mask & CWY) {
				c->oldy = c->y;
				c->y = m->my + ev->y;
			}
			if(ev->value_mask & CWWidth) {
				c->oldw = c->w;
				c->w = ev->width;
			}
			if(ev->value_mask & CWHeight) {
				c->oldh = c->h;
				c->h = ev->height;
			}
			if((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
			if((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
			if((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c);
			if(ISVISIBLE(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		}
		else
			configure(c);
	}
	else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

Monitor *
createmon(void) {
	Monitor *m;
  unsigned int i;

	if(!(m = (Monitor *)calloc(1, sizeof(Monitor))))
		die("fatal: could not malloc() %u bytes\n", sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = mfact;
	m->showbar = showbar;
	m->topbar = sbar.topbar;
	m->lt[0] = &layouts[0];
	m->lt[1] = &layouts[1 % LENGTH(layouts)];
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
  /* pertag init */
  m->curtag = m->prevtag = 1;
  for(i=0; i < LENGTH(tags) + 1 ; i++) {
    m->mfacts[i] = mfact;
    m->lts[i] = &layouts[0];
  }
	return m;
}

void
destroynotify(XEvent *e) {
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if((c = wintoclient(ev->window)))
		unmanage(c, True);
}

void
detach(Client *c) {
	Client **tc;

	for(tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void
detachstack(Client *c) {
	Client **tc, *t;

	for(tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if(c == c->mon->sel) {
		for(t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
		c->mon->sel = t;
	}
}

void
die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
  log_str(errstr, LOG_ERROR);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

Monitor *
dirtomon(int dir) {
	Monitor *m = NULL;

	if(dir > 0) {
		if(!(m = selmon->next))
			m = mons;
	}
	else if(selmon == mons)
		for(m = mons; m->next; m = m->next);
	else
		for(m = mons; m->next != selmon; m = m->next);
	return m;
}

void
drawbar(Monitor *m) {

 //calc if Battery is used
 unsigned int i;
 
 if(sbar_status_symbols[DrawBattery].active){
   // if battery low, change style
 
   if(battery.adapter){ //normal colors
     dc.norm[ColBorder]    = sbar.colors.normbordercolor;
     dc.norm[ColBG]        = sbar.colors.normbgcolor;
     dc.norm[ColFG]        = sbar.colors.normfgcolor;
     dc.sel[ColBorder]     = sbar.colors.selbordercolor;
     dc.sel[ColBG]         = sbar.colors.selbgcolor;
     dc.sel[ColFG]         = sbar.colors.selfgcolor;
     dc.sbar[SBarBorder]   = sbar.colors.botbordercolor;
     dc.sbar[SBarLine]     = sbar.colors.cpu_line; 
     dc.sbar[SBarCpuPoint] = sbar.colors.cpu_point;
     
   }else if(battery.stat <= bat_suspend){
     Arg arg = SHCMD("sudo pm-suspend");
     spawn(&arg);
 
       
   }else if(battery.stat <= bverylowstat){
     dc.norm[ColBorder]    = sbar.colors.bbnormbordercolor;
     dc.norm[ColBG]        = sbar.colors.bbnormbgcolor;
     dc.norm[ColFG]        = sbar.colors.bbnormfgcolor;
     dc.sel[ColBorder]     = sbar.colors.bbselbordercolor;
     dc.sel[ColBG]         = sbar.colors.bbselbgcolor;
     dc.sel[ColFG]         = sbar.colors.bbselfgcolor;
     dc.sbar[SBarBorder]   = sbar.colors.bbbotbordercolor;
     dc.sbar[SBarLine]     = sbar.colors.bbcpu_line; 
     dc.sbar[SBarCpuPoint] = sbar.colors.bbcpu_point;
     
   }else if(battery.stat <= blowstat){
     dc.norm[ColBorder]    = sbar.colors.bnormbordercolor;
     dc.norm[ColBG]        = sbar.colors.bnormbgcolor;
     dc.norm[ColFG]        = sbar.colors.bnormfgcolor;
     dc.sel[ColBorder]     = sbar.colors.bselbordercolor;
     dc.sel[ColBG]         = sbar.colors.bselbgcolor;
     dc.sel[ColFG]         = sbar.colors.bselfgcolor;
     dc.sbar[SBarBorder]   = sbar.colors.bbotbordercolor;
     dc.sbar[SBarLine]     = sbar.colors.bcpu_line; 
     dc.sbar[SBarCpuPoint] = sbar.colors.bcpu_point;
   }
 }
 
 /**
  * Dwm Stuff (tags drawing)
  */

	int x;
	unsigned int occ = 0, urg = 0;
	unsigned long *col;
	Client *c;

  for(c = m->clients; c; c = c->next) { // calculating used monitors
    occ |= c->tags;
    if(c->isurgent)
      urg |= c->tags;
  }
   
  dc.x = SBARTEXTW(mainsymbol);
  for(i = 0; i < LENGTH(tags); i++) {
    dc.w = TEXTW(tags[i]);
    col = m->tagset[m->seltags] & 1 << i ? dc.sel : dc.norm;
    drawtext(tags[i], col, urg & 1 << i);
    drawsquare(m == selmon && selmon->sel && selmon->sel->tags & 1 << i,
      occ & 1 << i, urg & 1 << i, col);
    dc.x += dc.w;
  }

  // draw layout symbol
  dc.w = blw = SBARTEXTW(m->ltsymbol);
  sbardrawtext(m->ltsymbol, dc.norm, False);
  dc.x += dc.w;
  x = dc.x;
  if(m != selmon)  
    dc.x = m->ww;

	XCopyArea(dpy, dc.drawable, m->barwin, dc.gc, 0, 0, m->ww, bh, 0, 0);
	XSync(dpy, False);
}

void
drawbars(void) {
	Monitor *m;

	for(m = mons; m; m = m->next)
		drawbar(m);
}

void
drawsquare(Bool filled, Bool empty, Bool invert, unsigned long col[ColLast]) {
	int x;

	XSetForeground(dpy, dc.gc, col[invert ? ColBG : ColFG]);
	x = (dc.font.ascent + dc.font.descent + 2) / 4;
	if(filled)
		XFillRectangle(dpy, dc.drawable, dc.gc, dc.x+1, dc.y+1, x+1, x+1);
	else if(empty)
		XDrawRectangle(dpy, dc.drawable, dc.gc, dc.x+1, dc.y+1, x, x);
}

void
sbardrawtext(const char *text, unsigned long col[ColLast], Bool invert) {
	char buf[256];
	int i, x, y, h, len, olen;

	XSetForeground(dpy, dc.gc, col[invert ? ColFG : ColBG]);
	XFillRectangle(dpy, dc.drawable, dc.gc, dc.x, dc.y, dc.w, dc.h);
	if(!text)
		return;
	olen = strlen(text);
	h = sbar.font.ascent + sbar.font.descent;
	y = dc.y + (dc.h / 2) - (h / 2) + sbar.font.ascent;
	x = dc.x + (h / 2);
	/* shorten text if necessary */
	for(len = MIN(olen, sizeof buf); len && sbartextnw(text, len) > dc.w - h; len--);
	if(!len)
		return;
	memcpy(buf, text, len);
	if(len < olen)
		for(i = len; i && i > len - 3; buf[--i] = '.');
	XSetForeground(dpy, dc.gc, col[invert ? ColBG : ColFG]);
	if(sbar.font.set)
		XmbDrawString(dpy, dc.drawable, sbar.font.set, dc.gc, x, y, buf, len);
	else
		XDrawString(dpy, dc.drawable, dc.gc, x, y, buf, len);
}

void
drawtext(const char *text, unsigned long col[ColLast], Bool invert) {
	char buf[256];
	int i, x, y, h, len, olen;

	XSetForeground(dpy, dc.gc, col[invert ? ColFG : ColBG]);
	XFillRectangle(dpy, dc.drawable, dc.gc, dc.x, dc.y, dc.w, dc.h);
	if(!text)
		return;
	olen = strlen(text);
	h = dc.font.ascent + dc.font.descent;
	y = dc.y + (dc.h / 2) - (h / 2) + dc.font.ascent;
	x = dc.x + (h / 2);
	/* shorten text if necessary */
	for(len = MIN(olen, sizeof buf); len && textnw(text, len) > dc.w - h; len--);
	if(!len)
		return;
	memcpy(buf, text, len);
	if(len < olen)
		for(i = len; i && i > len - 3; buf[--i] = '.');
	XSetForeground(dpy, dc.gc, col[invert ? ColBG : ColFG]);
	if(dc.font.set)
		XmbDrawString(dpy, dc.drawable, dc.font.set, dc.gc, x, y, buf, len);
	else
		XDrawString(dpy, dc.drawable, dc.gc, x, y, buf, len);
}

void
enternotify(XEvent *e) {
	Client *c;
	Monitor *m;
	XCrossingEvent *ev = &e->xcrossing;

	if((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window);
	if(m != selmon) {
		unfocus(selmon->sel, True);
		selmon = m;
	}
	else if(!c || c == selmon->sel)
		return;
	focus(c);
}

void
expose(XEvent *e) {
	Monitor *m;
	XExposeEvent *ev = &e->xexpose;

	if(ev->count == 0 && (m = wintomon(ev->window)))
		drawbar(m);
}

void
focus(Client *c) {
	if(!c || !ISVISIBLE(c))
		for(c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
	/* was if(selmon->sel) */
	if(selmon->sel && selmon->sel != c)
		unfocus(selmon->sel, False);
	if(c) {
		if(c->mon != selmon)
			selmon = c->mon;
		if(c->isurgent)
			clearurgent(c);
		detachstack(c);
		attachstack(c);
		grabbuttons(c, True);
		XSetWindowBorder(dpy, c->win, dc.sel[ColBorder]);
		setfocus(c);
	}
	else
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
	selmon->sel = c;
	drawbars();
}

void
focusin(XEvent *e) { /* there are some broken focus acquiring clients */
	XFocusChangeEvent *ev = &e->xfocus;

	if(selmon->sel && ev->window != selmon->sel->win)
		setfocus(selmon->sel);
}

void
focusmon(const Arg *arg) {
	Monitor *m;

	if(!mons->next)
		return;
	if((m = dirtomon(arg->i)) == selmon)
		return;
	unfocus(selmon->sel, True);
	selmon = m;
	focus(NULL);
}

void
focusstack(const Arg *arg) {
	Client *c = NULL, *i;

	if(!selmon->sel)
		return;
	if(arg->i > 0) {
		for(c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
		if(!c)
			for(c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
	}
	else {
		for(i = selmon->clients; i != selmon->sel; i = i->next)
			if(ISVISIBLE(i))
				c = i;
		if(!c)
			for(; i; i = i->next)
				if(ISVISIBLE(i))
					c = i;
	}
	if(c) {
		focus(c);
		restack(selmon);
	}
}

Atom
getatomprop(Client *c, Atom prop) {
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;

	if(XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM,
	                      &da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom *)p;
		XFree(p);
	}
	return atom;
}

unsigned long
getcolor(const char *colstr) {
	Colormap cmap = DefaultColormap(dpy, screen);
	XColor color;

	if(!XAllocNamedColor(dpy, cmap, colstr, &color, &color))
		die("error, cannot allocate color '%s'\n", colstr);
	return color.pixel;
}

Bool
getrootptr(int *x, int *y) {
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

long
getstate(Window w) {
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if(XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
	                      &real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;
	if(n != 0)
		result = *p;
	XFree(p);
	return result;
}

Bool
gettextprop(Window w, Atom atom, char *text, unsigned int size) {
	char **list = NULL;
	int n;
	XTextProperty name;

	if(!text || size == 0)
		return False;
	text[0] = '\0';
	XGetTextProperty(dpy, w, &name, atom);
	if(!name.nitems)
		return False;
	if(name.encoding == XA_STRING)
		strncpy(text, (char *)name.value, size - 1);
	else {
		if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
			strncpy(text, *list, size - 1);
			XFreeStringList(list);
		}
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return True;
}

void
grabbuttons(Client *c, Bool focused) {
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if(focused) {
			for(i = 0; i < LENGTH(buttons); i++)
				if(buttons[i].click == ClkClientWin)
					for(j = 0; j < LENGTH(modifiers); j++)
						XGrabButton(dpy, buttons[i].button,
						            buttons[i].mask | modifiers[j],
						            c->win, False, BUTTONMASK,
						            GrabModeAsync, GrabModeSync, None, None);
		}
		else
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
			            BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
	}
}

void
grabkeys(void) {
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		KeyCode code;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for(i = 0; i < LENGTH(keys); i++)
			if((code = XKeysymToKeycode(dpy, keys[i].keysym)))
				for(j = 0; j < LENGTH(modifiers); j++)
					XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
						 True, GrabModeAsync, GrabModeAsync);
	}
}

void
initfont(const char *fontstr) {
	char *def, **missing, buffer[200];
	int n;

	dc.font.set = XCreateFontSet(dpy, fontstr, &missing, &n, &def);
	if(missing) {
		while(n--) {
			sprintf(buffer, "sdwm: missing fontset: %s", missing[n]);
      log_str(buffer, LOG_WARNING);
    }
		XFreeStringList(missing);
	}
	if(dc.font.set) {
		XFontStruct **xfonts;
		char **font_names;

		dc.font.ascent = dc.font.descent = 0;
		XExtentsOfFontSet(dc.font.set);
		n = XFontsOfFontSet(dc.font.set, &xfonts, &font_names);
		while(n--) {
			dc.font.ascent = MAX(dc.font.ascent, (*xfonts)->ascent);
			dc.font.descent = MAX(dc.font.descent,(*xfonts)->descent);
			xfonts++;
		}
	}
	else {
		if(!(dc.font.xfont = XLoadQueryFont(dpy, fontstr))
		&& !(dc.font.xfont = XLoadQueryFont(dpy, "fixed")))
			die("error, cannot load font: '%s'\n", fontstr);
		dc.font.ascent = dc.font.xfont->ascent;
		dc.font.descent = dc.font.xfont->descent;
	}
	dc.font.height = dc.font.ascent + dc.font.descent;
}

#ifdef XINERAMA
static Bool
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info) {
	while(n--)
		if(unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
		&& unique[n].width == info->width && unique[n].height == info->height)
			return False;
	return True;
}
#endif /* XINERAMA */

void
keypress(XEvent *e) {
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for(i = 0; i < LENGTH(keys); i++)
		if(keysym == keys[i].keysym
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
		&& keys[i].func)
			keys[i].func(&(keys[i].arg));
}

void
killclient(const Arg *arg) {
	if(!selmon->sel)
		return;
	if(!sendevent(selmon->sel, wmatom[WMDelete])) {
		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, selmon->sel->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}


  Client *c;
  Monitor *m;
  m = selmon;
  int win_nf = 0; // number of windows not floating
  
  for(c = m->stack; c; c = c->snext)
    if(!c->isfloating && ISVISIBLE(c)) 
       win_nf++;        
     
  // if there is no more not floating window turn statuswin back on
  if(!draw_status_win && win_nf == 1 && !selmon->sel->isfloating) togglestw();
  
  XSync(dpy, False);
}

void
manage(Window w, XWindowAttributes *wa) {
	Client *c, *t = NULL;
	Window trans = None;
	XWindowChanges wc;

	if(!(c = calloc(1, sizeof(Client))))
		die("fatal: could not malloc() %u bytes\n", sizeof(Client));
	c->win = w;
	updatetitle(c);
	if(XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	}
	else {
		c->mon = selmon;
		applyrules(c);
	}
	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;

	if(c->x + WIDTH(c) > c->mon->mx + c->mon->mw)
		c->x = c->mon->mx + c->mon->mw - WIDTH(c);
	if(c->y + HEIGHT(c) > c->mon->my + c->mon->mh)
		c->y = c->mon->my + c->mon->mh - HEIGHT(c);
	c->x = MAX(c->x, c->mon->mx);
	/* only fix client y-offset, if the client center might cover the bar */
	c->y = MAX(c->y, ((c->mon->by == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
	           && (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? bh : c->mon->my);
	c->bw = borderpx;

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, dc.norm[ColBorder]);
	configure(c); /* propagates border_width, if size doesn't change */
	updatewindowtype(c);
	updatesizehints(c);
	updatewmhints(c);
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, False);
	if(!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;
	if(c->isfloating)
		XRaiseWindow(dpy, c->win);
  attachaside(c);
	attachstack(c);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState);
	if (c->mon == selmon)
		unfocus(selmon->sel, False);
	c->mon->sel = c;
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	focus(NULL);
}

void
mappingnotify(XEvent *e) {
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if(ev->request == MappingKeyboard)
		grabkeys();
}

void
maprequest(XEvent *e) {
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;

	if(!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if(wa.override_redirect)
		return;
	if(!wintoclient(ev->window))
		manage(ev->window, &wa);
}

void
monocle(Monitor *m) {
	unsigned int n = 0;
	Client *c;

	for(c = m->clients; c; c = c->next)
		if(ISVISIBLE(c))
			n++;
	if(n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for(c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, False);
}

void
motionnotify(XEvent *e) {
	static Monitor *mon = NULL;
	Monitor *m;
	XMotionEvent *ev = &e->xmotion;

	if(ev->window != root)
		return;
	if((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		selmon = m;
		focus(NULL);
	}
	mon = m;
}

void
movemouse(const Arg *arg) {
	int x, y, ocx, ocy, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;

	if(!(c = selmon->sel))
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
	None, cursor[CurMove], CurrentTime) != GrabSuccess)
		return;
	if(!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if(nx >= selmon->wx && nx <= selmon->wx + selmon->ww
			&& ny >= selmon->wy && ny <= selmon->wy + selmon->wh) {
				if(abs(selmon->wx - nx) < snap)
					nx = selmon->wx;
				else if(abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
					nx = selmon->wx + selmon->ww - WIDTH(c);
				if(abs(selmon->wy - ny) < snap)
					ny = selmon->wy;
				else if(abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
					ny = selmon->wy + selmon->wh - HEIGHT(c);
				if(!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
					togglefloating(NULL);
			}
			if(!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, True);
			break;
		}
	} while(ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

Client *
nexttiled(Client *c) {
	for(; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void
pop(Client *c) {
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

void
propertynotify(XEvent *e) {
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if((ev->window == root) && (ev->atom == XA_WM_NAME))
		updatestatus();
	else if(ev->state == PropertyDelete)
		return; /* ignore */
	else if((c = wintoclient(ev->window))) {
		switch(ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			if(!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
			   (c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon);
			break;
		case XA_WM_NORMAL_HINTS:
			updatesizehints(c);
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			drawbars();
			break;
		}
		if(ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if(c == c->mon->sel)
				drawbar(c->mon);
		}
		if(ev->atom == netatom[NetWMWindowType])
			updatewindowtype(c);
	}
}

void
quit(const Arg *arg) {
	running = False;
}

Monitor *
recttomon(int x, int y, int w, int h) {
	Monitor *m, *r = selmon;
	int a, area = 0;

	for(m = mons; m; m = m->next)
		if((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

void
resize(Client *c, int x, int y, int w, int h, Bool interact) {
	if(applysizehints(c, &x, &y, &w, &h, interact))
		resizeclient(c, x, y, w, h);
}

void
resizeclient(Client *c, int x, int y, int w, int h) {
	XWindowChanges wc;

	if(c->isfloating || selmon->lt[selmon->sellt]->arrange == NULL) { globalborder = 0 ; }
	else {
		if (selmon->lt[selmon->sellt]->arrange == monocle) { globalborder = 0 - borderpx ; }
		else { globalborder =  gappx ; }
	}

  c->oldx = c->x; c->x = wc.x = x + globalborder ;
  c->oldy = c->y; c->y = wc.y = y + globalborder ;
  c->oldw = c->w; c->w = wc.width = w - 2 * globalborder ;
  c->oldh = c->h; c->h = wc.height = h - 2 * globalborder ;
	wc.border_width = c->bw;
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void
resizemouse(const Arg *arg) {
	int ocx, ocy;
	int nw, nh;
	Client *c;
	Monitor *m;
	XEvent ev;

	if(!(c = selmon->sel))
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if(XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
	                None, cursor[CurResize], CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if(c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if(!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if(!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, True);
			break;
		}
	} while(ev.type != ButtonRelease);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void
restack(Monitor *m) {
	Client *c;
	XEvent ev;
	XWindowChanges wc;

	drawbar(m);
  if(!m->sel){
     // there is no window turn statuswin back on (if of)
     if(!draw_status_win) togglestw();
		return;
  }
	if(m->sel->isfloating || !m->lt[m->sellt]->arrange)
		XRaiseWindow(dpy, m->sel->win);
	if(m->lt[m->sellt]->arrange) {
    int win_nf = 0; // number of windows not floating
		wc.stack_mode = Below;
		wc.sibling = m->barwin;
		for(c = m->stack; c; c = c->snext) {
			if(!c->isfloating && ISVISIBLE(c)) {
        win_nf++;        
      
        // set statuswin invisible if nedded
        if(draw_status_win) togglestw();

				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
    }
    // if there is no more not floating window turn statuswin back on
    if(!draw_status_win && win_nf == 0) togglestw();
	}
	XSync(dpy, False);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
run(void) {
	XEvent ev;
	/* main event loop */
	XSync(dpy, False);
	while(running && !XNextEvent(dpy, &ev))
		if(handler[ev.type])
			handler[ev.type](&ev); /* call handler */
}

void
scan(void) {
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if(XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for(i = 0; i < num; i++) {
			if(!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if(wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
				manage(wins[i], &wa);
		}
		for(i = 0; i < num; i++) { /* now the transients */
			if(!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if(XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
		if(wins)
			XFree(wins);
	}
}

void
sendmon(Client *c, Monitor *m) {
	if(c->mon == m)
		return;
	unfocus(c, True);
	detach(c);
	detachstack(c);
	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
	attach(c);
	attachstack(c);
	focus(NULL);
	arrange(NULL);
}

void
setclientstate(Client *c, long state) {
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
			PropModeReplace, (unsigned char *)data, 2);
}

Bool
sendevent(Client *c, Atom proto) {
	int n;
	Atom *protocols;
	Bool exists = False;
	XEvent ev;

	if(XGetWMProtocols(dpy, c->win, &protocols, &n)) {
		while(!exists && n--)
			exists = protocols[n] == proto;
		XFree(protocols);
	}
	if(exists) {
		ev.type = ClientMessage;
		ev.xclient.window = c->win;
		ev.xclient.message_type = wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, c->win, False, NoEventMask, &ev);
	}
	return exists;
}

void
setfocus(Client *c) {
	if(!c->neverfocus)
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
	sendevent(c, wmatom[WMTakeFocus]);
}

void
setfullscreen(Client *c, Bool fullscreen) {
	if(fullscreen) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
		                PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
		c->isfullscreen = True;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = True;
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dpy, c->win);
	}
	else {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
		                PropModeReplace, (unsigned char*)0, 0);
		c->isfullscreen = False;
		c->isfloating = c->oldstate;
		c->bw = c->oldbw;
		c->x = c->oldx;
		c->y = c->oldy;
		c->w = c->oldw;
		c->h = c->oldh;
		resizeclient(c, c->x, c->y, c->w, c->h);
		arrange(c->mon);
	}
}

void
setlayout(const Arg *arg) {
	if(!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt ^= 1;
	if(arg && arg->v)
    selmon->lt[selmon->sellt] = selmon->lts[selmon->curtag] = (Layout *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if(selmon->sel)
		arrange(selmon);
	else
		drawbar(selmon);

  // map statuswin if Layout is Floating 
  if(((Layout *)arg->v)->arrange == NULL && !draw_status_win)
    togglestw();
}

/* arg > 1.0 will set mfact absolutly */
void
setmfact(const Arg *arg) {
	float f;

	if(!arg || !selmon->lt[selmon->sellt]->arrange)
		return;
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if(f < 0.1 || f > 0.9)
		return;
  selmon->mfact = selmon->mfacts[selmon->curtag] = f;
	arrange(selmon);
}

void
setup(void) {
	XSetWindowAttributes wa;

	/* clean up any zombies immediately */
	sigchld(0);

	/* init screen */
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);
	initfont(font);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);

  //init bar heigth  
  bh = dc.h = dc.font.height + 2;

  // load sbar
  setup_sbar();
  
	updategeom();
	/* init atoms */
	wmatom[WMProtocols]            = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete]               = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState]                = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus]            = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	netatom[NetActiveWindow]       = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported]          = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetWMName]             = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState]            = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMFullscreen]       = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMWindowType]       = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	/* init cursors */
	cursor[CurNormal] = XCreateFontCursor(dpy, XC_left_ptr);
	cursor[CurResize] = XCreateFontCursor(dpy, XC_sizing);
	cursor[CurMove]   = XCreateFontCursor(dpy, XC_fleur);

	/* init appearance */
  // this creates the pixelmap for the statusbar
	dc.drawable = XCreatePixmap(dpy, root, DisplayWidth(dpy, screen), bh, DefaultDepth(dpy, screen));

	dc.gc = XCreateGC(dpy, root, 0, NULL);
	XSetLineAttributes(dpy, dc.gc, 1, LineSolid, CapButt, JoinMiter);
	if(!dc.font.set)
		XSetFont(dpy, dc.gc, dc.font.xfont->fid);
	/* init bars */
	updatebars();
	updatestatus();
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
			PropModeReplace, (unsigned char *) netatom, NetLast);
	/* select for events */
	wa.cursor = cursor[CurNormal];
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask|PointerMotionMask
	                |EnterWindowMask|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	grabkeys();
}

void
showhide(Client *c) {
	if(!c)
		return;
	if(ISVISIBLE(c)) { /* show clients top down */
		XMoveWindow(dpy, c->win, c->x, c->y);
		if((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, False);
		showhide(c->snext);
	}
	else { /* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	}
}

void
sigchld(int unused) {
	if(signal(SIGCHLD, sigchld) == SIG_ERR)
		die("Can't install SIGCHLD handler");
	while(0 < waitpid(-1, NULL, WNOHANG));
}

void
spawn(Arg *arg) {
	if(fork() == 0) {
		if(dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **)arg->v)[0], (char **)arg->v);
		fprintf(stderr, "sdwm: execvp %s", ((char **)arg->v)[0]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
}
void dmenucmd(){
  char *cmd;
  cmd = (char *)malloc(sizeof(char) * (strlen(font) + strlen(themes[CurTheme].verylow.normbgcolor) +
                                       strlen(themes[CurTheme].verylow.normfgcolor) +
                                       strlen(themes[CurTheme].verylow.selbgcolor) +
                                       strlen(themes[CurTheme].verylow.selfgcolor) + 50));

  if(sbar_status_symbols[DrawBattery].active && battery.stat <= bverylowstat && !battery.adapter){
    sprintf(cmd, "dmenu_run -fn '%s' -nb '%s' -nf '%s' -sb '%s' -sf '%s'", 
            font, themes[CurTheme].verylow.normbgcolor,
            themes[CurTheme].verylow.normfgcolor, 
            themes[CurTheme].verylow.selbgcolor, 
            themes[CurTheme].verylow.selfgcolor);

  }else if(sbar_status_symbols[DrawBattery].active && battery.stat <= blowstat && !battery.adapter){
    sprintf(cmd, "dmenu_run -fn '%s' -nb '%s' -nf '%s' -sb '%s' -sf '%s'", 
            font, themes[CurTheme].low.normbgcolor,
            themes[CurTheme].low.normfgcolor, 
            themes[CurTheme].low.selbgcolor, 
            themes[CurTheme].low.selfgcolor);

  }else{
    sprintf(cmd, "dmenu_run -fn '%s' -nb '%s' -nf '%s' -sb '%s' -sf '%s'", 
            font, themes[CurTheme].normal.normbgcolor,
            themes[CurTheme].normal.normfgcolor, 
            themes[CurTheme].normal.selbgcolor, 
            themes[CurTheme].normal.selfgcolor);
  }
  
  popen(cmd,"w");
}

void termcmd(){
  char *cmd;
  cmd = (char *)malloc(sizeof(char) * (strlen(xtermfont) + strlen(themes[CurTheme].verylow.selbgcolor) +
                                       strlen(themes[CurTheme].verylow.selfgcolor)*2 + 50));

  if(sbar_status_symbols[DrawBattery].active && battery.stat <= bverylowstat && !battery.adapter){
    sprintf(cmd, "uxterm -bc -bg '%s' -cr '%s' -fg '%s' -fn '%s'", 
            themes[CurTheme].verylow.selbgcolor, 
            themes[CurTheme].verylow.selfgcolor,
            themes[CurTheme].verylow.selfgcolor, xtermfont);

  }else if(sbar_status_symbols[DrawBattery].active && battery.stat <= blowstat && !battery.adapter){
    sprintf(cmd, "uxterm -bc -bg '%s' -cr '%s' -fg '%s' -fn '%s'",
            themes[CurTheme].low.selbgcolor, 
            themes[CurTheme].low.selfgcolor,
            themes[CurTheme].low.selfgcolor, xtermfont);

  }else{
    sprintf(cmd, "uxterm -bc -bg '%s' -cr '%s' -fg '%s' -fn '%s'",
            themes[CurTheme].normal.selbgcolor, 
            themes[CurTheme].normal.selfgcolor,
            themes[CurTheme].normal.selfgcolor, xtermfont);
  }
  
  popen(cmd,"w");
}


void
tag(const Arg *arg) {
	if(selmon->sel && arg->ui & TAGMASK) {
		selmon->sel->tags = arg->ui & TAGMASK;
		focus(NULL);
		arrange(selmon);
	}
}

void
tagmon(const Arg *arg) {
	if(!selmon->sel || !mons->next)
		return;
	sendmon(selmon->sel, dirtomon(arg->i));
}

int
textnw(const char *text, unsigned int len) {
	XRectangle r;

	if(dc.font.set) {
		XmbTextExtents(dc.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(dc.font.xfont, text, len);
}

int
sbartextnw(const char *text, unsigned int len) {
	XRectangle r;

	if(sbar.font.set) {
		XmbTextExtents(sbar.font.set, text, len, NULL, &r);
		return r.width;
	}
	return XTextWidth(sbar.font.xfont, text, len);
}

void
togglebar(const Arg *arg) {
	selmon->showbar = !selmon->showbar;
	updatebarpos(selmon);
	XMoveResizeWindow(dpy, selmon->barwin, selmon->wx, selmon->by, selmon->ww, bh);
	arrange(selmon);
}

void
togglefloating(const Arg *arg) {
	if(!selmon->sel)
		return;
	selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
	if(selmon->sel->isfloating)
		resize(selmon->sel, selmon->sel->x, selmon->sel->y,
		       selmon->sel->w, selmon->sel->h, False);
	arrange(selmon);
}

void
toggletag(const Arg *arg) {
	unsigned int newtags, i;

	if(!selmon->sel)
		return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if(newtags) {
		selmon->sel->tags = newtags;
    if(newtags == ~0) {
      selmon->prevtag = selmon->curtag;
      selmon->curtag = 0;
    }
    if(!(newtags & 1 << (selmon->curtag - 1))) {
      selmon->prevtag = selmon->curtag;
      for (i=0; !(newtags & 1 << i); i++);
        selmon->curtag = i + 1;
    }
    selmon->sel->tags = newtags;
    selmon->lt[selmon->sellt] = selmon->lts[selmon->curtag];
    selmon->mfact = selmon->mfacts[selmon->curtag];
		focus(NULL);
		arrange(selmon);
	}
}

void
toggleview(const Arg *arg) {
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);

	if(newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		focus(NULL);
		arrange(selmon);
	}
}

void
unfocus(Client *c, Bool setfocus) {
	if(!c)
		return;
	grabbuttons(c, False);
	XSetWindowBorder(dpy, c->win, dc.norm[ColBorder]);
	if(setfocus)
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
}

void
unmanage(Client *c, Bool destroyed) {
	Monitor *m = c->mon;
	XWindowChanges wc;

	/* The server grab construct avoids race conditions. */
	detach(c);
	detachstack(c);
	if(!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
	free(c);
	focus(NULL);
	arrange(m);
}

void
unmapnotify(XEvent *e) {
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if((c = wintoclient(ev->window))) {
		if(ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c, False);
	}
}

void
updatebars(void) {
	Monitor *m;
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ButtonPressMask|ExposureMask
	};
	for(m = mons; m; m = m->next) {
		m->barwin = XCreateWindow(dpy, root, m->wx, m->by, m->ww, bh, 0, DefaultDepth(dpy, screen),
		                          CopyFromParent, DefaultVisual(dpy, screen),
		                          CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa);
    m->statuswin = XCreateWindow(dpy, root, stw.x, stw.y, stw.w, stw.h, 0, DefaultDepth(dpy, screen),
                                  CopyFromParent, DefaultVisual(dpy, screen),
                                  CWOverrideRedirect|CWBackPixmap|CWEventMask, &wa); 
		XDefineCursor(dpy, m->barwin, cursor[CurNormal]);
    XDefineCursor(dpy, m->statuswin, cursor[CurNormal]);
		XMapRaised(dpy, m->barwin);
    XMapRaised(dpy, m->statuswin);
	}
}

void
updatebarpos(Monitor *m) {
	m->wy = m->my;
	m->wh = m->mh;
	if(m->showbar) {
		m->wh -= bh;
		m->by = m->topbar ? m->wy : m->wy + m->wh;
		m->wy = m->topbar ? m->wy + bh : m->wy;
	}
	else
		m->by = -bh;
}

Bool
updategeom(void) {
	Bool dirty = False;

#ifdef XINERAMA
	if(XineramaIsActive(dpy)) {
		int i, j, n, nn;
		Client *c;
		Monitor *m;
		XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
		XineramaScreenInfo *unique = NULL;

		for(n = 0, m = mons; m; m = m->next, n++);
		/* only consider unique geometries as separate screens */
		if(!(unique = (XineramaScreenInfo *)malloc(sizeof(XineramaScreenInfo) * nn)))
			die("fatal: could not malloc() %u bytes\n", sizeof(XineramaScreenInfo) * nn);
		for(i = 0, j = 0; i < nn; i++)
			if(isuniquegeom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
		XFree(info);
		nn = j;
		if(n <= nn) {
			for(i = 0; i < (nn - n); i++) { /* new monitors available */
				for(m = mons; m && m->next; m = m->next);
				if(m)
					m->next = createmon();
				else
					mons = createmon();
			}
			for(i = 0, m = mons; i < nn && m; m = m->next, i++)
				if(i >= n
				|| (unique[i].x_org != m->mx || unique[i].y_org != m->my
				    || unique[i].width != m->mw || unique[i].height != m->mh))
				{
					dirty = True;
					m->num = i;
					m->mx = m->wx = unique[i].x_org;
					m->my = m->wy = unique[i].y_org;
					m->mw = m->ww = unique[i].width;
					m->mh = m->wh = unique[i].height;
					updatebarpos(m);
				}
		}
		else { /* less monitors available nn < n */
			for(i = nn; i < n; i++) {
				for(m = mons; m && m->next; m = m->next);
				while(m->clients) {
					dirty = True;
					c = m->clients;
					m->clients = c->next;
					detachstack(c);
					c->mon = mons;
					attach(c);
					attachstack(c);
				}
				if(m == selmon)
					selmon = mons;
				cleanupmon(m);
			}
		}
		free(unique);
	}
	else
#endif /* XINERAMA */
	/* default monitor setup */
	{
		if(!mons)
			mons = createmon();
		if(mons->mw != sw || mons->mh != sh) {
			dirty = True;
			mons->mw = mons->ww = sw;
			mons->mh = mons->wh = sh;
			updatebarpos(mons);
		}
	}
	if(dirty) {
		selmon = mons;
		selmon = wintomon(root);
	}
	return dirty;
}

void
updatenumlockmask(void) {
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for(i = 0; i < 8; i++)
		for(j = 0; j < modmap->max_keypermod; j++)
			if(modmap->modifiermap[i * modmap->max_keypermod + j]
			   == XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c) {
	long msize;
	XSizeHints size;

	if(!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	if(size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	}
	else if(size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	}
	else
		c->basew = c->baseh = 0;
	if(size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	}
	else
		c->incw = c->inch = 0;
	if(size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	}
	else
		c->maxw = c->maxh = 0;
	if(size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	}
	else if(size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	}
	else
		c->minw = c->minh = 0;
	if(size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	}
	else
		c->maxa = c->mina = 0.0;
	c->isfixed = (c->maxw && c->minw && c->maxh && c->minh
	             && c->maxw == c->minw && c->maxh == c->minh);
}

void
updatetitle(Client *c) {
	if(!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if(c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);
}

void
updatestatus(void) {

	gettextprop(root, XA_WM_NAME, vim_command_utils.last_input, sizeof(vim_command_utils.last_input));

  if (!strncmp(vim_command_utils.last_input, "VIMCMD: ", 8)) {
    run_vim_command(vim_command_utils.last_input + 8);
    
  } else {

    update_status();
    drawstatus();
    draw_freestylebar();
    if(draw_status_win)
      drawstw();
 
    drawbar(selmon);
  }

}

void
updatewindowtype(Client *c) {
	Atom state = getatomprop(c, netatom[NetWMState]);
	Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

	if(state == netatom[NetWMFullscreen])
		setfullscreen(c, True);

	if(wtype == netatom[NetWMWindowTypeDialog])
		c->isfloating = True;
}

void
updatewmhints(Client *c) {
	XWMHints *wmh;

	if((wmh = XGetWMHints(dpy, c->win))) {
		if(c == selmon->sel && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		}
		else
			c->isurgent = (wmh->flags & XUrgencyHint) ? True : False;
		if(wmh->flags & InputHint)
			c->neverfocus = !wmh->input;
		else
			c->neverfocus = False;
		XFree(wmh);
	}
}

void
view(const Arg *arg) {
  unsigned int i;
	if((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
		return;
	selmon->seltags ^= 1; /* toggle sel tagset */
  if(arg->ui & TAGMASK) {
      selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
    selmon->prevtag = selmon->curtag;
    if(arg->ui == ~0)
      selmon->curtag = 0;
    else {
      for (i=0; !(arg->ui & 1 << i); i++);
      selmon->curtag = i + 1;
    }
  } else {
    selmon->prevtag= selmon->curtag ^ selmon->prevtag;
    selmon->curtag^= selmon->prevtag;
    selmon->prevtag= selmon->curtag ^ selmon->prevtag;
  }
  selmon->lt[selmon->sellt]= selmon->lts[selmon->curtag];
  selmon->mfact = selmon->mfacts[selmon->curtag];
	focus(NULL);
	arrange(selmon);
}

Client *
wintoclient(Window w) {
	Client *c;
	Monitor *m;

	for(m = mons; m; m = m->next)
		for(c = m->clients; c; c = c->next)
			if(c->win == w)
				return c;
	return NULL;
}

Monitor *
wintomon(Window w) {
	int x, y;
	Client *c;
	Monitor *m;

	if(w == root && getrootptr(&x, &y))
		return recttomon(x, y, 1, 1);
	for(m = mons; m; m = m->next)
    if(w == m->barwin || w == m->statuswin)
			return m;
	if((c = wintoclient(w)))
		return c->mon;
	return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's).  Other types of errors call Xlibs
 * default error handler, which may call exit.  */
int
xerror(Display *dpy, XErrorEvent *ee) {
	if(ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "sdwm: fatal error: request code=%d, error code=%d\n",
			ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee) {
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee) {
	die("sdwm: another window manager is already running\n");
	return -1;
}

void
zoom(const Arg *arg) {
	Client *c = selmon->sel;

	if(!selmon->lt[selmon->sellt]->arrange
	|| (selmon->sel && selmon->sel->isfloating))
		return;
	if(c == nexttiled(selmon->clients))
		if(!c || !(c = nexttiled(c->next)))
			return;
	pop(c);
}

int
main(int argc, char *argv[]) {
	if(argc == 2 && !strcmp("-v", argv[1]))
		die("sdwm-"VERSION", © 2006-2011 sdwm engineers, see LICENSE for details\n");
	else if(argc != 1)
		die("usage: sdwm [-v]\n");
	if(!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if(!(dpy = XOpenDisplay(NULL)))
		die("sdwm: cannot open display\n");
	checkotherwm();
	setup();
	scan();
	run();
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
