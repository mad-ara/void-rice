/* See LICENSE file for copyright and license details. */

#include "patches/moveresize.c"
#include "patches/push.c"

/* appearance */
static const char *fonts[] = {
    "tewi:style=Regular:size=9",
    "Siji:style=Regular:size=9"
};
static const unsigned int borderpx   = 0;        /* border pixel of windows */
static const unsigned int snap       = 32;       /* snap pixel */
static const int showbar             = 1;        /* 0 means no bar */
static const int topbar              = 1;        /* 0 means bottom bar */
static const unsigned int gappx      = 8;        /* gap pixel between windows */
static const unsigned int barsize    = 10;       /* size of the dwm bar */
static const unsigned int tagspacing = 10;       /* size of icon area */

#define NUMCOLORS 9
static const char colors[NUMCOLORS][MAXCOLORS][9] = {
	// border	 foreground	  background 7C817C
	{ "#ffffff",	 "#FDF7E1", 	  "#2A3334" },  // 0 = normal
	{ "#181818", 	 "#FDF7E1", 	  "#e15443" },  // 1 = selected
	{ "#b43030", 	 "#FDF7E1", 	  "#b23450" },  // 2 = urgent
	{ "#181818", 	 "#FDF7E1", 	  "#394547" },  // 3 = occupied
	{ "#212121", 	 "#ab7438", 	  "#0b0606" },  // 4 = yellow
	{ "#212121", 	 "#475971", 	  "#0b0606" },  // 5 = blue
	{ "#212121", 	 "#694255", 	  "#0b0606" },  // 6 = magenta
	{ "#212121", 	 "#3e6868", 	  "#0b0606" },  // 7 = cyan
	{ "#212121",	 "#cfa696", 	  "#0b0606" },  // 8 = grey
};

/* tagging */
static const char *tags[] = { "", "", "", "\ue269", "", "\ue1e3" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Pale moon"     ,  NULL,       NULL,       1 << 1,       0,           -1 },
	{ "Telegram"      ,  NULL,       NULL,       1 << 4,       0,           -1 },
	{ "zathura"       ,  NULL,       NULL,       1 << 3,       0,           -1 },
	{ "ranger"        ,  NULL,       NULL,       1 << 3,       0,           -1 },
	{ "neomutt"       ,  NULL,       NULL,       1 << 3,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "\ue002",   tile },    /* first entry is default */
	{ "\ue006",   NULL },    /* no layout function means floating behavior */
	{ "\ue000",   monocle }, /* monocle is good for maximizing the preservation and focusing of the window */
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
static const char *dmenucmd[]   = { "dmenu_run", "-i", "-l", "10", "-p", "Search >", "-fn", "tewi:size=11" , "-nb", "#394547", "-nf", "#D8DEE9", "-sb", "#e15443", "-sf", "#2D333D", "-w", "0", "-h", "20", NULL };
static const char *termcmd[]    = { "st", NULL };
static const char *mail[]       = { "st", "-c", "neomutt", "-e", "neomutt", NULL};
static const char *ranger[]     = { "st", "-c", "ranger", "-e", "ranger", NULL};
static const char *dmoji[]      = { "dmoji", NULL};
static const char *cmustatus[]  = { "cmus-status", NULL};
static const char *weechat[]    = { "tmux", "new-window", "-t", "main", "weechat", NULL};
static const char *voldown[]    = { "amixer", "-q", "sset", "Master", "2%-", NULL};
static const char *volup[]      = { "amixer", "-q", "sset", "Master", "2%+", NULL};
static const char *volsupdown[] = { "amixer", "-q", "sset", "Master", "5%-", NULL};
static const char *volsuperup[] = { "amixer", "-q", "sset", "Master", "5%+", NULL};
static const char *muteall[]    = { "amixer", "-q", "sset", "Master", "toggle", NULL};
static const char *cmus[]       = { "st", "-c", "cmus", NULL};
static const char *cmuspause[]  = { "cmus-remote", "--pause", NULL};

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_q,      killclient,     {0} },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },	
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY,            		XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,            		XK_KP_Enter, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_c,      spawn,          {.v = cmustatus } },
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_n,      spawn,          {.v = mail } },
	{ MODKEY,                       XK_s,      spawn,          SHCMD("import /tmp/file.png && xclip -sel clip -t image/png /tmp/file.png && rm /tmp/file.png") },
	{ MODKEY,                       XK_Print,  spawn,          SHCMD("scrot -e 'mv $f ~/Pictures/scrot' && sleep 1 && exec notify-send 'screenshot has been saved in ~/Pictures/scrot'") },
	{ MODKEY,                       XK_x,      spawn,          SHCMD("slock") },
	{ MODKEY,                       XK_equal,  spawn,          {.v = volup } },
	{ MODKEY,                       XK_minus,  spawn,          {.v = voldown } },
	{ MODKEY,                       XK_Up,     moveresize,     {.v = "0x -25y 0w 0h"} },
	{ MODKEY,                       XK_Down,   moveresize,     {.v = "0x 25y 0w 0h"} },
	{ MODKEY,                       XK_Left,   moveresize,     {.v = "-25x 0y 0w 0h"} },
	{ MODKEY,                       XK_Right,  moveresize,     {.v = "25x 0y 0w 0h"} },


	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY|ShiftMask,          	XK_Return, zoom,           {0} },
	{ MODKEY|ShiftMask,             XK_s,      spawn,          SHCMD("scrot 'xclip -selection clipboard -t image/png $f && rm -f $f'") },
	{ MODKEY|ShiftMask,             XK_Print,  spawn,          SHCMD("scrot -se 'mv $f ~/Pictures/scrot/' && sleep 1 && exec notify-send 'screenshot has been saved in ~/Pictures/scrot'") },
	{ MODKEY|ShiftMask,             XK_p,      spawn,          {.v = cmuspause } },
	{ MODKEY|ShiftMask,             XK_w,      spawn,          {.v = dmoji } },
	{ MODKEY|ShiftMask,             XK_m,      spawn,          {.v = muteall } },
	{ MODKEY|ShiftMask,             XK_equal,  spawn,          {.v = volsuperup } },
      	{ MODKEY|ShiftMask,             XK_minus,  spawn,          {.v = volsupdown } },
	{ MODKEY|ShiftMask,             XK_Up,     moveresize,     {.v = "0x 0y 0w -25h"} },
	{ MODKEY|ShiftMask,             XK_Down,   moveresize,     {.v = "0x 0y 0w 25h"} },
	{ MODKEY|ShiftMask,             XK_Left,   moveresize,     {.v = "0x 0y -25w 0h"} },
	{ MODKEY|ShiftMask,             XK_Right,  moveresize,     {.v = "0x 0y 25w 0h"} },
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },

	{ MODKEY|ControlMask,           XK_m,      spawn,          {.v = cmus } },
	{ MODKEY|ControlMask,           XK_j,      pushdown,       {0} },
	{ MODKEY|ControlMask,           XK_k,      pushup,         {0} },

	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
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

