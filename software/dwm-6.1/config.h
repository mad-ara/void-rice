/* See LICENSE file for copyright and license details. */

#include "patches/moveresize.c"
#include "patches/push.c"

/* appearance */
static const char *fonts[] = {
    "Fira Code:style=Regular:size=10",
    "Siji:style=Regular:size=12"
};
static const unsigned int borderpx   = 1;        /* border pixel of windows */
static const unsigned int snap       = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, 0: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar             = 1;        /* 0 means no bar */
static const int topbar              = 1;        /* 0 means bottom bar */
static const unsigned int gappx      = 8;        /* gap pixel between windows */
static const unsigned int barsize    = 10;       /* size of the dwm bar */
static const unsigned int tagspacing = 10;       /* size of icon area */

#define NUMCOLORS 9
static const char colors[NUMCOLORS][MAXCOLORS][9] = {
    // border	 	foreground	  background
    { "#444444",	 "#ffffff", 	  "#151515" },  // 0 = normal
    { "#BD3538", 	 "#ffffff", 	  "#BD3538" },  // 1 = selected
    { "#151515", 	 "#ffffff", 	  "#da846b" },  // 2 = urgent
    { "#151515", 	 "#ffffff", 	  "#444444" },  // 3 = occupied
    { "#151515", 	 "#151515", 	  "#e7ab65" },  // 4 = yellow
    { "#151515", 	 "#ffffff", 	  "#268ac3" },  // 5 = blue
    { "#151515", 	 "#ffffff", 	  "#9977a2" },  // 6 = magenta
    { "#151515", 	 "#ffffff", 	  "#6d966c" },  // 7 = cyan
    { "#151515",	 "#ffffff", 	  "#AFC563" },  // 8 = grey
};

/* tagging */
static const char *tags[] = { "\ue1f0",  "\ue1a0", "\ue1ec", "\ue1ed", "\ue269", "\ue1d9", "\ue0ad", "\ue1ce", "\ue197" };

static const Rule rules[] = {
    /* xprop(1):
     *	WM_CLASS(STRING) = instance, class
     *	WM_NAME(STRING) = title
     */
    /* class      		instance    title       tags mask     isfloating   monitor */
    { "pcmanfm"   	  ,  NULL,      NULL,       1 << 5,       1,			-1 },
    { "Telegram"      ,  NULL,      NULL,       1 << 6,       0,			-1 },
    { "Firefox"       ,  NULL,      NULL,       1 << 1,       0,			-1 }
};			

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
    /* symbol     arrange function */
    { "\ue0bc",   tile },    /* first entry is default */
    { "\ue0b9",   NULL },    /* no layout function means floating behavior */
    { "\ue0b4",   monocle }, /* monocle is good for maximizing the preservation and focusing of the window */
};

/* key definitions */
/* Mod4Mask == Super key
 * Mod1Mask == Alt key
 * Mod5Mask == Alt Gr
 */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[]   = { "dmenu_run", "-i", "-l", "20", "-p", "Search >", "-fn", "Fira Code:style=Regular:size=10" , "-nb", "#151515", "-nf", "#cfcfcf", "-sb", "#BD3538", "-sf", "#222222", "-w", "0", "-h", "20", NULL };
static const char *termcmd[]    = { "st", NULL };
static const char *voldown[]    = { "pulsemixer", "--change-volume", "-5", NULL};
static const char *volup[]      = { "pulsemixer", "--change-volume", "+5", NULL};
static const char *mute[]    	= { "pulsemixer", "--toggle-mute", NULL};
static const char *filemanager[]= { "pcmanfm", NULL};
static const char *lightdown[]	= {	"xbacklight", "-dec", "5", NULL};
static const char *lightup[]	= {	"xbacklight", "-inc", "5", NULL};

static Key keys[] = {
    /* modifier             key        		function        argument */
    { MODKEY,               XK_0,      		view,           {.ui = ~0 } },
    { MODKEY,				XK_a,			spawn,			SHCMD("gscreenshot-cli -f ~/screenshots/ -s && notify-send \"`date`\" \"Screenshot saved\"") },
    { MODKEY,             	XK_b,      		togglebar,      {0} },
    { MODKEY,               XK_d,      		incnmaster,     {.i = -1 } },
    { MODKEY,               XK_e,      		spawn,          {.v = filemanager } },
    { MODKEY,               XK_f,      		setlayout,      {.v = &layouts[1]} },
    { MODKEY,               XK_h,      		setmfact,       {.f = -0.05} },
    { MODKEY,               XK_i,      		incnmaster,     {.i = +1 } },
    { MODKEY,             	XK_j,      		focusstack,     {.i = +1 } },
    { MODKEY,               XK_k,      		focusstack,     {.i = -1 } },
    { MODKEY,               XK_l,      		setmfact,       {.f = +0.05} },
    { MODKEY,               XK_m,      		setlayout,      {.v = &layouts[2]} },
    { MODKEY,               XK_p,      		spawn,          {.v = dmenucmd } },
    { MODKEY,             	XK_q,      		killclient,     {0} },
    { MODKEY,               XK_s,      		spawn,          SHCMD("gscreenshot-cli -f ~/screenshots/ && notify-send \"`date`\" \"Screenshot saved\"") },
    { MODKEY,               XK_t,      		setlayout,      {.v = &layouts[0]} },
    { MODKEY,               XK_x,      		spawn,          SHCMD("physlock") },
    { MODKEY,               XK_comma,  		focusmon,       {.i = -1 } },
    { MODKEY,               XK_equal,  		spawn,          {.v = volup } },
    { MODKEY,               XK_minus,  		spawn,          {.v = voldown } },
    { MODKEY,               XK_period, 		focusmon,       {.i = +1 } },
    { MODKEY,               XK_space,  		setlayout,      {0} },	
    { MODKEY,            	XK_KP_Enter, 	spawn,          {.v = termcmd } },
    { MODKEY,            	XK_Return, 		spawn,          {.v = termcmd } },
    { MODKEY,               XK_Tab,    		view,           {0} },
    { MODKEY,               XK_Up,     		moveresize,     {.v = "0x -25y 0w 0h"} },
    { MODKEY,               XK_Down,   		moveresize,     {.v = "0x 25y 0w 0h"} },
    { MODKEY,               XK_Left,   		moveresize,     {.v = "-25x 0y 0w 0h"} },
    { MODKEY,               XK_Right,  		moveresize,     {.v = "25x 0y 0w 0h"} },
    { MODKEY,				0x1008ff02,		spawn,			{.v = lightup }},
    { MODKEY,				0x1008ff03,		spawn, 			{.v = lightdown }},
    { MODKEY,				0x1008ff11,		spawn,			{.v = voldown }},
    { MODKEY,				0x1008ff13,		spawn,			{.v = volup }},

    { MODKEY|ShiftMask,     XK_0,      		tag,            {.ui = ~0 } },
    { MODKEY|ShiftMask,		XK_a,			spawn,			SHCMD("gscreenshot-cli -f ~/screenshots/ -s -c && notify-send \"`date`\" \"Screenshot copied to clipboard\"") },
    { MODKEY|ShiftMask,     XK_m,      		spawn,          {.v = mute } },
    { MODKEY|ShiftMask,     XK_q,      		quit,           {0} },
    { MODKEY|ShiftMask,     XK_s,      		spawn,          SHCMD("gscreenshot-cli -c && notify-send \"`date`\" \"Screenshot copied to clipboard\"") },
    { MODKEY|ShiftMask,     XK_comma,		tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,     XK_period, 		tagmon,         {.i = +1 } },
    { MODKEY|ShiftMask,     XK_space,  		togglefloating, {0} },
    { MODKEY|ShiftMask,     XK_Return, 		zoom,           {0} },
    { MODKEY|ShiftMask,     XK_Up,     		moveresize,     {.v = "0x 0y 0w -25h"} },
    { MODKEY|ShiftMask,     XK_Down,   		moveresize,     {.v = "0x 0y 0w 25h"} },
    { MODKEY|ShiftMask,     XK_Left,   		moveresize,     {.v = "0x 0y -25w 0h"} },
    { MODKEY|ShiftMask,     XK_Right,  		moveresize,     {.v = "0x 0y 25w 0h"} },

    { MODKEY|ControlMask,   XK_j,      		pushdown,       {0} },
    { MODKEY|ControlMask,   XK_k,      		pushup,         {0} },
    

    TAGKEYS(                XK_1,      		                0)
    TAGKEYS(                XK_2,      		                1)
    TAGKEYS(                XK_3,      		                2)
    TAGKEYS(                XK_4,      		                3)
    TAGKEYS(                XK_5,                      		4)
    TAGKEYS(                XK_6,                      		5)
    TAGKEYS(                XK_7,                      		6)
    TAGKEYS(                XK_8,                      		7)
    TAGKEYS(                XK_9,                      		8)
};

/* button definitions */
/* click can be ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
    /* click                event mask      button          function        argument */
    { ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
    { ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
    { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
    { ClkTagBar,            0,              Button1,        view,           {0} },
    { ClkTagBar,            0,              Button3,        toggleview,     {0} },
    { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
    { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

