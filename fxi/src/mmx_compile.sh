#!/bin/sh
gcc -TARGET_BEOS -DSTDC_HEADERS -DREGEX_MALLOC `sdl-config --cflags` -IBeOS -g -DTARGET_BeOS  -I../../include  -I../inc -c mmx_scale2x.c -o mmx_scale2x.o
gcc -DTARGET_BEOS -DSTDC_HEADERS -DREGEX_MALLOC `sdl-config --cflags` -IBeOS -g -DTARGET_BeOS  -I../../include  -I../inc -c mmx_main.c -o mmx_main.o