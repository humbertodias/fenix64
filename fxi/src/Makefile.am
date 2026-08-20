bin_PROGRAMS = fxi

.rc.o:
	$(WINDRES) $< -I../../include -o $@

INCLUDES = $(COMMON_CFLAGS) $(SDL_CFLAGS) $(SDL_MIXER_CFLAGS) -I../../include -I../inc -DDEBUG

fxi_LDADD = $(SDL_LIBS) $(COMMON_LIBS) -lz -lpng $(SDL_MIXER_LIBS) -lm

SOURCE_FILES = main.c dcbr.c instance.c strings.c \
              i_main.c i_debug.c i_func.c \
	          g_main.c g_maps.c g_blit.c g_texts.c g_pal.c g_draw.c \
		      g_systexts.c g_console.c g_scroll.c g_flic.c g_mode7.c \
		      img_png.c img_pcx.c sound.c pathfind.c fnc_exports.c \
		      g_conversion.c g_font.c g_fpg.c g_blendop.c f_bgload.c \
		      f_cd.c f_sort.c fbm.c fgc.c fpl.c f_joystick.c i_copy.c  \
		      ../../common/dirs.c ../../common/regex.c \
		      ../../common/files.c ../../common/xctype.c \
		      g_profiler.c i_saveload.c \
		      scaler_scanline.c scaler_normal.c scaler_hq2x.c scaler_scale2x.c \
		      mmx_main.c mmx_hspan.c

#hacer un if con mmx para activarlo o no
noinst_HEADERS = ../inc/fxi.h    ../../include/files.h ../../include/xctype.h \
                 ../../include/offsets.h ../../include/pslang.h ../inc/fmath.h

if HAVE_WINDRES
fxi_SOURCES = $(SOURCE_FILES) fxi.rc
else
fxi_SOURCES = $(SOURCE_FILES)
endif

# Legacy GNU regex 0.12 uses K&R function definitions.
regex.o regex.obj: CFLAGS += -std=gnu89
