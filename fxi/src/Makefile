# Fenix - Compilador/intérprete de videojuegos
# Copyright (C) 1999 José Luis Cebrián Pagüe
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

ifndef target
error:
	@echo "ERROR: Debe compilar con make target=N"
	@echo "       (N puede ser MAC, linux o WIN32)"
endif

# Opciones por defecto
debug = true
SDL = /usr/local/include/SDL

ifeq ($(target) ,BeOS)
CFLAGS += -I/boot/develop/tools/gnupro/include -DTARGET_BEOS -DSTDC_HEADERS -DREGEX_MALLOC
EXTRA_OBJS += mmx_scale2x.o mmx_main.o
endif

ifeq ($(target) ,linux)
CFLAGS += -Wall -DTARGET_linux
EXTRA_OBJS += mmx_scale2x.o mmx_main.o
endif

ifeq ($(target) ,MAC)
CFLAGS += -Wall -O2 -g -DTARGET_MAC
LIBS += -L/sw/lib -L/usr/local/lib -L/usr/lib -lz -ldl -framework quicktime
endif

# Usar target=win32 para compilar el .EXE

ifeq ($(target), win32)
    TARGET = ../../bin/fxi.exe
    CFLAGS += -I/win32/include -I$(SDL)/include -I$(SDL)/include/SDL -D_WINDOWS
    LIBS += -lmingw32 -lpng -lz -L$(SDL)/lib -lSDL -mwindows
else 
  ifeq ($(target), BeOS)
    TARGET = ../../bin/fxi
    CFLAGS += `sdl-config --cflags` -IBeOS
    LIBS += `sdl-config --libs` -lpng -lSDL_mixer -lz -ldl -lglob -LBe_compat -lbe_compat -lbe
  else
    TARGET = ../../bin/fxi
    CFLAGS += `sdl-config --cflags` -L../../FreeType/include
    LIBS += `sdl-config --libs` -lpng -lm -lSDL_mixer -lvorbisfile -lvorbis -lz
  endif
endif

ifeq ($(target) ,MAC)
LIBS += `sdl-config --libs` -L/usr/local/lib -lfreetype 
CFLAGS += -I/usr/include/sys
endif

# Usar efence=true para enlazar con la librería Electric Fence

ifeq ($(efence), true)
    LIBS += -lefence
endif

# Usar debug=true para mode depurado

ifeq ($(debug), true)
    CFLAGS += -g
else
    CFLAGS += -O3 -finline-functions -ffast-math
endif

HEADERS = ../inc/fxi.h    ../../include/files.h ../../include/xctype.h \
	  ../../include/offsets.h ../../include/pslang.h ../inc/fmath.h

CFLAGS += -DTARGET_$(target) 
CFLAGS += -I../../include -I/sw/include -I../inc -I/sw/include/freetype2 

$(TARGET): main.o dcbr.o instance.o strings.o \
	i_main.o i_debug.o i_func.o \
	g_main.o g_maps.o g_blit.o g_texts.o g_pal.o g_draw.o \
	g_systexts.o g_console.o g_scroll.o g_flic.o g_mode7.o \
	img_png.o img_pcx.o sound.o pathfind.o fnc_exports.o\
	g_conversion.o g_font.o pathfind.o g_fpg.o g_blendop.o\
	$(EXTRA_OBJS) \
	../../common/dirs.o ../../common/regex.o \
	../../common/files.o ../../common/xctype.o g_profiler.o i_saveload.o
	cc  $(LDEFS) -o $@ $^ $(LIBS)

# Módulo principal

main.o: main.c $(HEADERS)

dcbr.o: dcbr.c $(HEADERS)

# Gestores de datos

instance.o: instance.c $(HEADERS)

strings.o: strings.c $(HEADERS)

# Intérprete

i_main.o: i_main.c $(HEADERS)

i_debug.o: i_debug.c $(HEADERS)

i_func.o: i_func.c $(HEADERS) ../inc/fmath.h ../../include/sysprocs.h

i_saveload.o: i_saveload.c $(HEADERS)

# Rutinas gráficas

g_main.o: g_main.c $(HEADERS)

g_maps.o: g_maps.c $(HEADERS)

g_blit.o: g_blit.c $(HEADERS) ../inc/fmath.h

g_texts.o: g_texts.c $(HEADERS)

g_systexts.o: g_systexts.c $(HEADERS)

g_console.o: g_console.c $(HEADERS)

g_pal.o: g_pal.c $(HEADERS) ../inc/font.h

g_scroll.o: g_scroll.c $(HEADERS)

g_flic.o: g_flic.c $(HEADERS)

g_mode7.o: g_mode7.c $(HEADERS) ../inc/fmath.h

g_draw.o: g_draw.c $(HEADERS)

g_conversion.o: g_conversion.c $(HEADERS)

g_font.o: g_font.c $(HEADERS)

g_fpg.o: g_fpg.c $(HEADERS)

g_blendop.o: g_blendop.c $(HEADER)

g_profiler.o: g_profiler.c $(HEADER)

mmx_scale2x.o: mmx_scale2x.c $(HEADER)

# Rutinas de carga de imágenes

img_png.o: img_png.c $(HEADERS)

img_pcx.o: img_pcx.c $(HEADERS)

# Módulo de sonido

sound.o: sound.c $(HEADERS)

# Modulo de librerias

fnc_exports.o: fnc_exports.c $(HEADERS)

# otras

pathfind.o: pathfind.c $(HEADERS)

# Rutinas de utilidad

../../common/files.o: ../../common/files.c ../../include/files.h

../../common/xctype.o: ../../common/xctype.c ../../include/xctype.h

../../common/regex.o: ../../common/regex.c ../../include/regex.h

../../common/dirs.o: ../../common/dirs.c ../../include/regex.h

# Limpieza

clean:
	rm -f *.o core ../../common/*.o

