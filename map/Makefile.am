bin_PROGRAMS=map

.rc.o:
	$(WINDRES) $< -I../../include -o $@

INCLUDES = $(COMMON_CFLAGS)

map_LDADD=$(COMMON_LIBS) -lz $(GIF_LIB) -lpng

SOURCE_FILES=map.c

if HAVE_WINDRES
map_SOURCES = $(SOURCE_FILES) map_resource.rc
else
map_SOURCES = $(SOURCE_FILES)
endif

