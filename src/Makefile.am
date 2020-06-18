## Process this file with automake to produce Makefile.in

SUBDIRS = bitmaps

bin_PROGRAMS = qvwm qv_deanimate

qvwm_SOURCES = main.cc main.h \
$(WINDOW_SRCS) $(BASE_SRCS) $(EVENT_SRCS) $(RC_SRCS) $(WM_SRCS) \
$(DESKTOP_SRCS) $(MENU_SRCS) $(TASKBAR_SRCS) $(PAGER_SRCS) $(IMAGE_SRCS) \
$(BUTTON_SRCS) $(DIALOG_SRCS) $(TOOLTIP_SRCS) $(AUDIO_SRCS) \
remote_cmd.cc remote_cmd.h

WINDOW_SRCS = qvwm.cc qvwm.h frame.cc titlebar.cc \
window.cc move.cc resize.cc resize_sp.cc rearrange.cc \
focus.cc focus_mgr.cc focus_mgr.h

BASE_SRCS = util.cc util.h message.h ver.h \
timer.cc timer.h function.cc function.h debug.cc debug.h \
hash.h list.h misc.h callback.h \
exec.cc exec.h 

EVENT_SRCS = event.cc event.h colormap.cc configure.cc property.cc \
key.cc key.h 

RC_SRCS = parse.cc parse.h yaccsrc.yy yaccsrc.h lexsrc.ll \
qvwmrc.cc qvwmrc.h 

WM_SRCS = mwm.cc mwm.h gnome.cc gnome.h session.cc session.h \
screen_saver.cc screen_saver.h 

DESKTOP_SRCS = desktop.cc desktop.h icon.cc icon.h switcher.cc switcher.h \
paging.cc paging.h info_display.cc info_display.h accessory.cc accessory.h

MENU_SRCS = menu.cc menu.h menu_func.cc startmenu.cc startmenu.h 

TASKBAR_SRCS = taskbar.cc taskbar.h tbutton.cc tbutton.h \
indicator.cc indicator.h 

PAGER_SRCS = pager.cc pager.h mini.cc mini.h 

IMAGE_SRCS = image.cc image.h pixmap_image.cc pixmap_image.h \
extra_image.cc extra_image.h anim_image.cc anim_image.h

BUTTON_SRCS = button.cc button.h string_button.cc string_button.h \
ctrl_button.cc fbutton.cc fbutton.h sbutton.cc sbutton.h

DIALOG_SRCS = dialog.cc dialog.h system_dialog.cc system_dialog.h \
radio_button.cc radio_button.h radio_set.cc radio_set.h resource.h \
exit_dialog.cc exit_dialog.h confirm_dialog.cc confirm_dialog.h 

TOOLTIP_SRCS = tooltip.cc tooltip.h 

AUDIO_SRCS = audio.cc audio.h audio_wav.cc audio_wav.h audio_au.cc audio_au.h \
audiodev.cc audiodev.h audiodev_alsa.cc audiodev_alsa.h \
audiodev_esd.cc audiodev_esd.h audiodev_oss.cc audiodev_oss.h \
audiodev_sun.cc audiodev_sun.h


qv_deanimate_SOURCES = qv_deanimate.c
qv_deanimate_DEFS=
qv_deanimate_LDADD=

DEFS += -DQVWMDIR=\"@QVWMDIR@\" -DIMGDIR=\"@IMGDIR@\" -DSNDDIR=\"@SNDDIR@\" -DPATH_RM=\"@PATH_RM@\"

DEFS += -DHAVE_CONFIG_H -I$(top_builddir)

# VPATH is not enough for lexsrc.ll and yaccsrc.yy
DEFS += -I$(srcdir)

qvwm_LDADD = @LEXLIB@

AM_YFLAGS = -d

EXTRA_DIST = Imakefile Makefile.std

qv_deanimate:
	$(CC) -o qv_deanimate qv_deanimate.c


