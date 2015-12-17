bin_PROGRAMS = fxc

.rc.o:
	$(WINDRES) $< -I../../include -o $@

INCLUDES = $(COMMON_CFLAGS) $(SDL_CFLAGS) -I../../include -I../inc

fxc_LDADD = $(SDL_LIBS) $(COMMON_LIBS) -lz

SOURCE_FILES = main.c main_div.c token.c identifier.c segment.c \
              constants.c strings.c varspace.c procedure.c codeblock.c \
              c_main.c c_data.c c_code.c c_debug.c dcbw.c \
              messages.c error.c \
              sysstub.c typedef.c ../../common/files.c ../../common/xctype.c

if HAVE_WINDRES
fxc_SOURCES = $(SOURCE_FILES) fxc.rc
else
fxc_SOURCES = $(SOURCE_FILES)
endif
