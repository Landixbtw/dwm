/* See LICENSE file for copyright and license details. */

#include <X11/XF86keysym.h>


/* appearance */
static const unsigned int borderpx  = 3;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "GeistMono NF:size=12", "JetBrainsMonoNL NFM:size=12" };
static const char dmenufont[]       = "GeistMono NF:size=12";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char col_warm[]        = "#ebcb8b"; // Warm Orange for the status bar and selected tag background
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
	// [SchemeSel]  = { col_gray4, col_warm,  col_warm  },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class        instance    title                           tags mask       isfloating   monitor */
	{ "Discord",    NULL,       "Vesktop",                      4,                  0,          -1 },
    { "Firefox",    NULL,       "Firefox Preferences",          1 << 8,             1,       -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
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
static const char *dmenucmd[] = { "dmenu_run", "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "ghostty", NULL };
// static const char *termcmd[]  = { "st", NULL };
// static const char *termcmd[]  = { "wezterm", NULL };
static const char *flameshotcmd[] = {"flameshot", "gui", "-c", "-p", "/home/ole/Pictures/" ,NULL};
static const char *webbrowsercmd[] = { "chromium" , NULL };


/* Your volume and brightness commands */
/* Volume control */

static const char *volume_up[] = { "/bin/sh", "-c", 
    "VOLUME_STEP=5 && "
    "BAR_LENGTH=10 && "
    "amixer -q sset Master ${VOLUME_STEP}%+ && "
    "VOL=$(amixer get Master | grep -Po '(?<=\\[)[0-9]+(?=%\\])' | head -1) && "
    "FILLED=$(seq -s '━' $(($VOL * BAR_LENGTH / 100)) | sed 's/[0-9]//g') && "
    "EMPTY=$(seq -s '─' $((BAR_LENGTH - $VOL * BAR_LENGTH / 100)) | sed 's/[0-9]//g') && "
    "notify-send -i /usr/share/icons/Papirus/48x48/status/notification-audio-volume-high.svg -r 9999 -t 500 'Volume' \"$FILLED$EMPTY $VOL%\"", NULL 
};



static const char *volume_down[] = { "/bin/sh", "-c", 
    "VOLUME_STEP=5 && "
    "BAR_LENGTH=10 && "
    "amixer -q sset Master ${VOLUME_STEP}%- && "
    "VOL=$(amixer get Master | grep -Po '(?<=\\[)[0-9]+(?=%\\])' | head -1) && "
    "FILLED=$(seq -s '━' $(($VOL * BAR_LENGTH / 100)) | sed 's/[0-9]//g') && "
    "EMPTY=$(seq -s '─' $((BAR_LENGTH - $VOL * BAR_LENGTH / 100)) | sed 's/[0-9]//g') && "
    "ICON=/usr/share/icons/Papirus/48x48/status/notification-audio-volume-medium.svg && "
    "if [ $VOL -eq 0 ]; then "
    "  ICON=/usr/share/icons/Papirus/48x48/status/notification-audio-volume-low.svg; "
    "fi && "
    "notify-send -i $ICON -r 9999 -t 500 'Volume' \"$FILLED$EMPTY $VOL%\"", NULL 
};


static const char *volume_toggle[] = { "/bin/sh", "-c", 
    "amixer -q sset Master toggle && "
    "MUTED=$(amixer get Master | grep '\\[off\\]' | wc -l) && "
    "if [ $MUTED -eq 0 ]; then "
    "  notify-send -i /usr/share/icons/Papirus/48x48/status/notification-audio-volume-high.svg -r 9999 -t 500 'Volume' 'Unmuted'; "
    "else "
    "  notify-send -i /usr/share/icons/Papirus/48x48/status/notification-audio-volume-muted.svg -r 9999 -t 500 'Volume' 'Muted'; "
    "fi", NULL 
};



static const char *brightness_up[] = { "/bin/sh", "-c", 
    "BRIGHTNESS_STEP=5 && "
    "BAR_LENGTH=10 && "
    "BRIGHTNESS=$(xbacklight -get | cut -d '.' -f 1) && "
    "[ $BRIGHTNESS -le $((100 - BRIGHTNESS_STEP)) ] && xbacklight -inc $BRIGHTNESS_STEP || xbacklight -set 100 && "
    "BRIGHTNESS=$(xbacklight -get | cut -d '.' -f 1) && "
    "FILLED=$(seq -s '━' $(($BRIGHTNESS * BAR_LENGTH / 100)) | sed 's/[0-9]//g') && "
    "EMPTY=$(seq -s '─' $((BAR_LENGTH - $BRIGHTNESS * BAR_LENGTH / 100)) | sed 's/[0-9]//g') && "
    "notify-send -i /usr/share/icons/Papirus/48x48/status/notification-display-brightness-high.svg -r 9998 -t 500 'Brightness' \"$FILLED$EMPTY $BRIGHTNESS%\"", NULL 
};

static const char *brightness_down[] = { "/bin/sh", "-c", 
    "BRIGHTNESS_STEP=5 && "
    "BAR_LENGTH=10 && "
    "BRIGHTNESS=$(xbacklight -get | cut -d '.' -f 1) && "
    "[ $BRIGHTNESS -ge $BRIGHTNESS_STEP ] && xbacklight -dec $BRIGHTNESS_STEP || xbacklight -set 0 && "
    "BRIGHTNESS=$(xbacklight -get | cut -d '.' -f 1) && "
    "FILLED=$(seq -s '━' $(($BRIGHTNESS * BAR_LENGTH / 100)) | sed 's/[0-9]//g') && "
    "EMPTY=$(seq -s '─' $((BAR_LENGTH - $BRIGHTNESS * BAR_LENGTH / 100)) | sed 's/[0-9]//g') && "
    "notify-send -i /usr/share/icons/Papirus/48x48/status/notification-display-brightness-medium.svg -r 9998 -t 500 'Brightness' \"$FILLED$EMPTY $BRIGHTNESS%\"", NULL 
};

static const char *kbd_brightness_up[]  = { "bash", "-c", 
    "cur=$(cat /sys/class/leds/tpacpi::kbd_backlight/brightness); max=$(cat /sys/class/leds/tpacpi::kbd_backlight/max_brightness); if [ $cur -lt $max ]; then echo $((cur+1)) > /sys/class/leds/tpacpi::kbd_backlight/brightness; fi", NULL };


static const char *kbd_brightness_down[] = { "bash", "-c", 
    "cur=$(cat /sys/class/leds/tpacpi::kbd_backlight/brightness); if [ $cur -gt 0 ]; then echo $((cur-1)) > /sys/class/leds/tpacpi::kbd_backlight/brightness; fi", NULL };

static const char *roficmd[] = { "rofi", "-show", "drun", "-show-icons" };


static const Key keys[] = {
	/* modifier                     key        function        argument */
	// { MODKEY,                       XK_d,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_d,      spawn,          {.v = roficmd} },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ ControlMask,                  XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	// { ControlMask ,                 XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_k,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
    { MODKEY|ShiftMask,             XK_s,       spawn,          {.v = flameshotcmd } },
    { MODKEY,                       XK_f,       spawn,          {.v = webbrowsercmd } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },

    { 0,                            XF86XK_MonBrightnessUp,     spawn,          {.v = brightness_up } },
    { 0,                            XF86XK_MonBrightnessDown,   spawn,          {.v = brightness_down } },
    { 0,                            XF86XK_AudioRaiseVolume,    spawn,          {.v = volume_up } },
    { 0,                            XF86XK_AudioLowerVolume,    spawn,          {.v = volume_down } },
    { 0,                            XF86XK_AudioMute,           spawn,          {.v = volume_toggle } },
    { 0,                            XF86XK_KbdBrightnessUp,     spawn,          {.v = kbd_brightness_up } },
    { 0,                            XF86XK_KbdBrightnessDown,   spawn,          {.v = kbd_brightness_down } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

