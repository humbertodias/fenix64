bin_PROGRAMS=fpg

.rc.o:
	$(WINDRES) $< -I../../include -o $@

INCLUDES = $(COMMON_CFLAGS)

fpg_LDADD=$(COMMON_LIBS) -lz $(GIF_LIB) -lpng

SOURCE_FILES=fpg.c

if HAVE_WINDRES
fpg_SOURCES = $(SOURCE_FILES) fpg_resource.rc
else
fpg_SOURCES = $(SOURCE_FILES)
endif

