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

debug = true

ifeq ($(target), linux)
CFLAGS += -Wall -DENGLISH -DLINUX
LIBS += -ldl
endif

# Los directorios definidos para las librerias son los de FINK

ifeq ($(target), MAC)
CFLAGS += -Wall -O2 -g
LIBS += -L/usr/local/lib -ldl
endif

ifeq ($(target), BeOS)
CFLAGS += `sdl-config --cflags` -I/boot/home/config/include
LIBS += -ldl
endif

# Usar debug=true para activar versión de depurado

ifeq ($(debug), true)
CFLAGS += -DDEBUG -g
else
CFLAGS += -O2 
endif

# Usar efence=true para activar la librería Electric Fence

ifeq ($(efence), true)
LIBS += -lefence
endif

# Usar taget=win32 para compilar el .EXE de Windows

ifeq ($(target), win32)
TARGET = ../../bin/fxc.exe
else
TARGET = ../../bin/fxc
endif

# Cabeceras comunes

HEADERS = ../inc/fxc.h   ../inc/codeblock.h ../inc/compiler.h \
          ../../include/files.h ../../include/xctype.h \
  	  ../../include/pslang.h ../../include/offsets.h \
          ../../include/dcb.h ../../include/files_st.h \
          ../../include/pslang.h ../../include/xstrings.h \
          ../../include/typedef.h ../../include/typedef_st.h \
          ../../include/xctype_st.h ../../include/fnx_loadlib.h

CFLAGS += -DTARGET_$(target)
CFLAGS += -I/usr/include/SDL -I../../include -I../inc

ifeq ($(target), MAC)
CFLAGS += -I/usr/local/include/SDL -I/sw/include
endif

# Target

$(TARGET): main.o main_div.o token.o identifier.o segment.o \
       constants.o strings.o varspace.o procedure.o codeblock.o \
       c_main.o c_fixed.o c_data.o c_code.o c_debug.o dcbw.o \
       messages.o \
       sysstub.o typedef.o ../../common/files.o ../../common/xctype.o 
	$(CC) -o $@ $^ $(LIBS) -lz

# Módulos principales

main.o: main.c $(HEADERS)

main_div.o: main_div.c $(HEADERS)

# Tokenizador

token.o: token.c $(HEADERS)

# Gestores de datos

identifier.o: identifier.c $(HEADERS)

constants.o: constants.c $(HEADERS)

strings.o: strings.c $(HEADERS)

varspace.o: varspace.c $(HEADERS)

procedure.o: procedure.c $(HEADERS)

codeblock.o: codeblock.c $(HEADERS)

typedef.o: typedef.c $(HEADERS)

sysstub.o: sysstub.c $(HEADERS) ../../include/sysprocs.h

segment.o: segment.c $(HEADERS) 

dcbw.o: dcbw.c $(HEADERS)

dcbw_old.o: dcbw_old.o $(HEADERS)

strings_old.o: strings_old.c $(HEADERS)

# Compilador

c_main.o: c_main.c $(HEADERS)

c_fixed.o: c_fixed.c $(HEADERS)

c_data.o: c_data.c $(HEADERS)

c_code.o: c_code.c $(HEADERS)

c_debug.o: c_debug.c $(HEADERS)

# Rutinas de utilidad

../../common/files.o: ../../common/files.c ../../include/files.h

../../common/xctype.o: ../../common/xctype.c ../../include/xctype.h

# Limpieza/empaquetado/etc.

messages.o: messages.c $(HEADERS)

clean:
	rm -f *.o core ../common/*.o

