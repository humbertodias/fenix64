/*
 *  Fenix - Videogame compiler/interpreter
 *  Current release       : FENIX - PROJECT 1.0 - R 0.84
 *  Last stable release   :
 *  Project documentation : http://fenix.divsite.net
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  Copyright © 1999 José Luis Cebrián Pagüe
 *  Copyright © 2002 Fenix Team
 *
 */

/*
 * FILE        : fnc_exports.c
 * DESCRIPTION : Symbol table management (symbol export/import)
 *               Also includes
 *
 * HISTORY:	0.82 - Performance improvements (hash table), cleaning,
 *                 and ability to export DLL symbols
 */

#include <stdlib.h>

#include <fxi.h>
#include <fnx_loadlib.h>
#include <fmath.h>

static int global_count = 0;
static int global_notfound = 0;

typedef struct _symbol
{
    const char * name;
    void       * addr;
	int          count;

	struct _symbol  * next;
}
symbol;

/* The following struct includes all the symbols exported by default */

/* This macro is used to export a function */
#define FNC(a) { #a, a }
/* This macro is used to export a data pointer */
#define DAT(a) { #a, (void *) &a }

static symbol default_symbols[] = {
    { "file_open"                       , file_open                   },
    { "file_read"                       , file_read                   },
    { "file_write"                      , file_write                  },
    { "file_gets"                       , file_gets                   },
    { "file_puts"                       , file_puts                   },
    { "file_qgets"                      , file_qgets                  },
    { "file_qputs"                      , file_qputs                  },
    { "file_size"                       , file_size                   },
    { "file_pos"                        , file_pos                    },
    { "file_seek"                       , file_seek                   },
    { "file_addp"                       , file_addp                   },
    { "file_close"                      , file_close                  },
    { "file_exists"                     , file_exists                 },
    { "file_add_xfile"                  , file_add_xfile              },
    { "file_eof"                        , file_eof                    },
    { "file_fp"                         , file_fp                     },
    { "string_init"                     , string_init                 },
    { "string_get"                      , (void*)string_get           },
    { "string_dump"                     , string_dump                 },
    { "string_load"                     , string_load                 },
    { "string_new"                      , string_new                  },
    { "string_newa"                     , string_newa                 },
    { "string_use"                      , string_use                  },
    { "string_discard"                  , string_discard              },
    { "string_add"                      , string_add                  },
    { "string_itoa"                     , string_itoa                 },
    { "string_ftoa"                     , string_ftoa                 },
    { "string_ptoa"                     , string_ptoa                 },
    { "string_comp"                     , string_comp                 },
    { "string_char"                     , string_char                 },
    { "string_substr"                   , string_substr               },
    { "string_find"                     , string_find                 },
    { "string_ucase"                    , string_ucase                },
    { "string_lcase"                    , string_lcase                },
    { "string_strip"                    , string_strip                },
    { "string_casecmp"                  , string_casecmp              },
    { "string_pad"                      , string_pad                  },
    { "string_format"                   , string_format               },
    { "string_coalesce"                 , string_coalesce             },
    { "string_concat"                   , string_concat               },
    { "dos_chars"                       , &dos_chars                  },
    { "c_type"                          , &c_type                     },
    { "c_upper"                         , &c_upper                    },
    { "c_lower"                         , &c_lower                    },
    { "convert"                         , convert                     },
    { "init_c_type"                     , init_c_type                 },
    { "dos_to_win"                      , &dos_to_win                 },
    { "win_to_dos"                      , &win_to_dos                 },
    { "flic_open"                       , flic_open                   },
    { "flic_destroy"                    , flic_destroy                },
    { "flic_do_frame"                   , flic_do_frame               },
    { "flic_reset"                      , flic_reset                  },
    { "current_fli"                     , &current_fli                },
    { "current_fli_x"                   , &current_fli_x              },
    { "current_fli_y"                   , &current_fli_y              },
    { "debug"                           , &debug                      },
    { "fxi"                             , &fxi                        },
    { "enable_16bits"                   , &enable_16bits              },
    { "enable_filtering"                , &enable_filtering           },
    { "report_string"                   , &report_string              },
    { "report_graphics"                 , &report_graphics            },
    { "gr_error"                        , gr_error                    },
    { "do_exit"                         , do_exit                     },
    { "dcb_load"                        , dcb_load                    },
    { "getid"                           , getid                       },
    { "path_find"                       , path_find                   },
    { "path_get"                        , path_get                    },
    { "path_set_wall"                   , path_set_wall               },
    { "scr_initialized"                 , &scr_initialized            },
    { "regions"                         , &regions                    },
    { "background"                      , &background                 },
    { "scrbitmap"                       , &scrbitmap                  },
    { "syslib"                          , &syslib                     },
    { "icono"                           , &icono                      },
    { "last_frame_ms"                   , &last_frame_ms              },
	{ "apptitle"                        , &apptitle                   },
    { "key_table"                       , &key_table                  },
    { "keystate"                        , &keystate                   },
    { "keytab_initialized"              , &keytab_initialized         },
    { "scr_width"                       , &scr_width                  },
    { "scr_height"                      , &scr_height                 },
    { "full_screen"                     , &full_screen                },
    { "double_buffer"                   , &double_buffer              },
    { "gr_init"                         , gr_init                     },
    { "gr_set_fps"                      , gr_set_fps                  },
    { "gr_wait_frame"                   , gr_wait_frame               },
    { "gr_advance_timers"               , gr_advance_timers           },
    { "gr_draw_frame"                   , gr_draw_frame               },
	{ "gr_draw_screen"                  , gr_draw_screen              },
    { "keytab_init"                     , keytab_init                 },
    { "keytab_free"                     , keytab_free                 },
    { "vpalette"                        , &vpalette                   },
    { "palette"                         , &palette                    },
    { "colorequiv"                      , &colorequiv                 },
    { "colorghost"                      , &colorghost                 },
    { "background_8bits"                , &background_8bits           },
    { "background_8bits_used"           , &background_8bits_used      },
    { "background_is_black"             , &background_is_black        },
    { "trans_table"                     , &trans_table                },
    { "trans_table_updated"             , &trans_table_updated        },
    { "gr_make_trans_table"             , gr_make_trans_table         },
    { "palette_loaded"                  , &palette_loaded             },
    { "palette_changed"                 , &palette_changed            },
    { "fade_on"                         , &fade_on                    },
/*    { "fade_step"                       , &fade_step                  },*/
    { "gr_read_pal"                     , gr_read_pal                 },
    { "gr_refresh_palette"              , gr_refresh_palette          },
    { "gr_fade_init"                    , gr_fade_init                },
    { "gr_fade_step"                    , gr_fade_step                },
    { "gr_roll_palette"                 , gr_roll_palette             },
    { "gr_find_nearest_color"           , gr_find_nearest_color       },
    { "gr_set_rgb"                      , gr_set_rgb                  },
    { "gr_rgb"                          , gr_rgb                      },
    { "gr_get_rgb"                      , gr_get_rgb                  },
    { "gr_set_colors"                   , gr_set_colors               },
    { "gr_get_colors"                   , gr_get_colors               },
    { "blend_create"                    , blend_create                },
    { "blend_free"                      , blend_free                  },
    { "blend_init"                      , blend_init                  },
    { "blend_translucency"              , blend_translucency          },
    { "blend_intensity"                 , blend_intensity             },
    { "blend_tint"                      , blend_tint                  },
    { "blend_swap"                      , blend_swap                  },
    { "blend_assign"                    , blend_assign                },
    { "blend_apply"                     , blend_apply                 },
    { "blend_grayscale"                 , blend_grayscale             },
    { "gr_load_map"                     , gr_load_map                 },
    { "gr_load_png"                     , gr_load_png                 },
    { "gr_load_pcx"                     , gr_load_pcx                 },
    { "gr_load_fpg"                     , gr_load_fpg                 },
    { "gr_load_pal"                     , gr_load_pal                 },
    { "gr_save_pal"                     , gr_save_pal                 },
	{ "gr_save_png"						, gr_save_png				  },
    { "grlib_new"                       , grlib_new                   },
    { "grlib_destroy"                   , grlib_destroy               },
    { "grlib_add_map"                   , grlib_add_map               },
    { "grlib_unload_map"                , grlib_unload_map            },
    { "bitmap_new"                      , bitmap_new                  },
    { "bitmap_clone"                    , bitmap_clone                },
    { "bitmap_new_syslib"               , bitmap_new_syslib           },
    { "bitmap_get"                      , bitmap_get                  },
    { "bitmap_destroy"                  , bitmap_destroy              },
    { "bitmap_add_cpoint"               , bitmap_add_cpoint           },
    { "bitmap_analize"                  , bitmap_analize              },
    { "bitmap_animate"                  , bitmap_animate              },
    { "bitmap_animate_to"               , bitmap_animate_to           },
    { "region_define"                   , region_define               },
    { "region_union"                    , region_union                },
    { "region_is_empty"                 , region_is_empty             },
    { "region_is_out"                   , region_is_out               },
    { "draw_instance_at"                , draw_instance_at            },
    { "draw_instance"                   , draw_instance               },
    { "instance_update_bbox"            , instance_update_bbox        },
    { "instance_graph"                  , instance_graph              },
    { "scroll_region"                   , scroll_region               },
    { "gr_font_get"                     , gr_font_get                 },
    { "gr_font_load"                    , gr_font_load                },
    { "gr_load_bdf"                     , gr_load_bdf                 },
	{ "gr_font_save"                    , gr_font_save                },
    { "gr_font_new"                     , gr_font_new                 },
    { "gr_font_newfrombitmap"           , gr_font_newfrombitmap       },
    { "gr_font_systemfont"              , gr_font_systemfont          },
    { "gr_font_destroy"                 , gr_font_destroy             },
	{ "gr_text_setcolor"                , gr_text_setcolor            },
	{ "gr_text_getcolor"                , gr_text_getcolor            },
    { "gr_text_new"                     , gr_text_new                 },
    { "gr_text_new_var"                 , gr_text_new_var             },
    { "gr_text_move"                    , gr_text_move                },
    { "gr_text_destroy"                 , gr_text_destroy             },
    { "gr_text_width"                   , gr_text_width               },
    { "gr_text_widthn"                  , gr_text_widthn              },
    { "gr_text_height"                  , gr_text_height              },
    { "gr_text_margintop"               , gr_text_margintop           },
    { "gr_text_put"                     , gr_text_put                 },
    { "gr_text_bitmap"                  , gr_text_bitmap              },
    { "gr_lock_screen"                  , gr_lock_screen              },
    { "gr_unlock_screen"                , gr_unlock_screen            },
    { "gr_clear"                        , gr_clear                    },
    { "gr_clear_as"                     , gr_clear_as                 },
    { "gr_put_pixel"                    , gr_put_pixel                },
    { "gr_get_pixel"                    , gr_get_pixel                },
    { "gr_setcolor"                     , gr_setcolor                 },
    { "gr_vline"                        , gr_vline                    },
    { "gr_hline"                        , gr_hline                    },
    { "gr_line"                         , gr_line                     },
    { "gr_box"                          , gr_box                      },
    { "gr_rectangle"                    , gr_rectangle                },
    { "gr_circle"                       , gr_circle                   },
    { "gr_fcircle"                      , gr_fcircle                  },
    { "gr_blit"                         , gr_blit                     },
    { "gr_get_bbox"                     , gr_get_bbox                 },
    { "gr_rotated_blit"                 , gr_rotated_blit             },
    { "gr_scroll_start"                 , gr_scroll_start             },
    { "gr_scroll_stop"                  , gr_scroll_stop              },
    { "gr_scroll_draw"                  , gr_scroll_draw              },
    { "gr_scroll_active"                , gr_scroll_active            },
    { "gr_scroll_is_fullscreen"         , gr_scroll_is_fullscreen     },
    { "gr_mode7_start"                  , gr_mode7_start              },
    { "gr_mode7_stop"                   , gr_mode7_stop               },
    { "gr_mode7_draw"                   , gr_mode7_draw               },
    { "gr_mode7_active"                 , gr_mode7_active             },
    { "gr_sys_color"                    , gr_sys_color                },
    { "gr_sys_puts"                     , gr_sys_puts                 },
    { "gr_sys_putchar"                  , gr_sys_putchar              },
	{ "gr_convert16_ScreenTo565"        , gr_convert16_ScreenTo565    },
	{ "gr_convert16_565ToScreen"        , gr_convert16_565ToScreen    },
	{ "gr_fade16"                       , gr_fade16                   },
    { "gr_con_printf"                   , gr_con_printf               },
    { "gr_con_putline"                  , gr_con_putline              },
    { "gr_con_show"                     , gr_con_show                 },
    { "gr_con_draw"                     , gr_con_draw                 },
    { "gr_con_getkey"                   , gr_con_getkey               },
    { "gr_con_scroll"                   , gr_con_scroll               },
    { "gr_con_do"                       , gr_con_do                   },
    { "local_strings"                   , &local_strings              },
    { "localstr"                        , &localstr                   },
    { "mainproc"                        , &mainproc                   },
    { "procdef_get"                     , procdef_get                 },
    { "sysproc_get"                     , sysproc_get                 },
    { "sysproc_add"                     , sysproc_add                 },
    { "sysproc_init"                    , sysproc_init                },
    { "globaldata"                      , &globaldata                 },
    { "localdata"                       , &localdata                  },
    { "local_size"                      , &local_size                 },
    { "first_instance"                  , &first_instance             },
    { "last_instance"                   , &last_instance              },
    { "must_exit"                       , &must_exit                  },
    { "instance_getid"                  , instance_getid              },
    { "instance_get"                    , instance_get                },
    { "instance_getfather"              , instance_getfather          },
    { "instance_getson"                 , instance_getson             },
    { "instance_getbigbro"              , instance_getbigbro          },
    { "instance_getsmallbro"            , instance_getsmallbro        },
    { "instance_new"                    , instance_new                },
    { "instance_duplicate"              , instance_duplicate          },
    { "instance_destroy"                , instance_destroy            },
    { "instance_dump"                   , instance_dump               },
    { "instance_dump_all"               , instance_dump_all           },
    { "instance_go"                     , instance_go                 },
    { "instance_go_all"                 , instance_go_all             },
	{ "instance_posupdate"              , instance_posupdate          },
	{ "instance_poschanged"             , instance_poschanged         },
	{ "sound_active"					, &sound_active				  },
    { "sound_init"                      , sound_init                  },
    { "sound_close"                     , sound_close                 },
    { "load_song"                       , load_song                   },
    { "play_song"                       , play_song                   },
    { "unload_song"                     , unload_song                 },
    { "stop_song"                       , stop_song                   },
    { "pause_song"                      , pause_song                  },
    { "resume_song"                     , resume_song                 },
    { "is_playing_song"                 , is_playing_song             },
    { "set_song_volume"                 , set_song_volume             },
    { "load_wav"                        , load_wav                    },
    { "play_wav"                        , play_wav                    },
    { "unload_wav"                      , unload_wav                  },
    { "stop_wav"                        , stop_wav                    },
    { "pause_wav"                       , pause_wav                   },
    { "resume_wav"                      , resume_wav                  },
    { "is_playing_wav"                  , is_playing_wav              },
    { "set_wav_volume"                  , set_wav_volume              },
	{ "set_channel_volume"              , set_channel_volume          },
	{ "reserve_channels"                , reserve_channels            },
    { "screen"                          , &screen                     },
    { "fnc_export"                      , fnc_export                  },
    { "gr_new_object"                   , gr_new_object               },
    { "gr_hide_object"                  , gr_hide_object              },
    { "gr_destroy_object"               , gr_destroy_object           },

	/* Drawing */
	DAT( syscolor16 ),
	DAT( syscolor8 ),
	DAT( fntcolor16 ),
	DAT( fntcolor8 ),
	FNC( gr_setalpha ),
	FNC( gr_bezier ),
	FNC( gr_drawing_new ),
	FNC( gr_drawing_destroy ),
	FNC( gr_drawing_move ),
	DAT( drawing_stipple ),
	DAT( frame_count ),

	/* Palette */
	DAT( nearest_table ),
	FNC( gr_fill_nearest_table ),

	/* Low-level conversion/alpha */
	FNC( gr_alpha16 ),
	FNC( gr_alpha8 ),

	/* Profiler */
	FNC(gprof_init),
	FNC(gprof_begin),
	FNC(gprof_end),
	FNC(gprof_frame),
	FNC(gprof_dump),
	FNC(gprof_reset),
	FNC(gprof_draw),
	FNC(gprof_toggle),

	/* Utility internal functions */
	FNC(gr_key),
	FNC(gr_timer),

	/* Regions */
	FNC(region_destroy),
	FNC(region_new),
	FNC(region_get),

	/* DUMP_TYPE support */
	FNC(gr_mark_rect),
	FNC(gr_mark_instance),

	/* WM related */
	DAT(grab_input) ,
	DAT(exit_status) ,
	DAT(window_status) ,
	DAT(focus_status) ,
	DAT(mouse_status) ,

    { NULL                              , NULL                        }
} ;
#undef FNC
#undef DAT

/* Scatter array for hash calculations */

#define HASH_MAX 1023

static int scatter[HASH_MAX+1] =
{
      288,   547,   970,   673,   985,   434,   359,   956,
      760,   890,   960,   223,   712,   173,  1008,   182,
      579,   587,   700,    20,   745,   926,   120,    71,
      292,     7,   707,   709,   722,   733,   989,   832,
      147,   520,    73,    23,   482,   873,   892,   249,
      413,   470,   819,   484,   639,   527,   469,   488,
      455,   934,   715,   631,   629,   243,   487,   949,
      371,   271,   804,   497,   686,   426,    66,   209,
      635,    64,    28,   950,   987,   933,   955,   398,
      559,   720,   648,  1007,   523,   420,   952,   914,
      995,     5,   573,   101,    34,   158,   169,    25,
      793,   815,   346,    72,     4,    69,   268,   764,
      727,   657,   829,    78,   666,    68,   794,   105,
      706,   589,   656,   295,    58,   219,    40,   188,
      566,   148,   653,   768,    10,   380,   285,   293,
       37,   406,   613,   417,    59,   825,   967,    50,
       30,   253,   618,   550,   828,   824,   590,   258,
      728,   529,   865,   461,   378,   753,   427,  1005,
      939,   647,   866,   411,   123,   643,   561,   576,
      471,   467,   575,   684,    35,   615,   545,   705,
      564,   893,   779,   432,   911,   823,   581,   901,
      106,   403,   474,   797,   280,    18,   319,   861,
      999,   525,   962,   964,   490,   210,   165,   395,
      774,   667,   773,   638,   230,   453,   654,   161,
       47,   877,   333,   711,   135,   634,   307,   393,
      650,   560,   343,   628,   889,   908,  1016,   788,
      644,     0,   697,   265,   339,   298,   791,   166,
      944,   902,   672,   572,   450,   419,   306,     8,
      846,   945,   662,   305,   382,   136,   694,   191,
      363,   856,   372,   174,   195,   328,   204,   623,
      767,   178,   616,   375,   364,   235,   896,    31,
      762,   181,   602,   231,   923,   830,   348,   299,
      850,   754,   676,   556,   526,   637,   577,   121,
      543,   895,   717,   757,   953,   668,   352,   186,
      775,   906,   534,   221,   863,   565,    75,   798,
       65,   574,     2,   583,   626,   475,   702,    56,
       24,   325,   445,   198,   226,   301,    96,   217,
     1010,   972,   203,   431,   102,   207,   598,   812,
      917,   110,   452,   532,   124,   424,   567,   251,
      867,   276,   171,   982,   356,   857,   542,   864,
     1000,   899,    11,   335,   677,   412,    86,   546,
      404,   224,   738,   459,    89,   273,   978,   772,
     1020,   256,   541,   881,   749,   442,   477,   103,
      267,    43,   358,   386,   151,   721,   321,   849,
      729,   778,   283,    41,   163,    92,   837,   449,
       81,   980,   242,   289,   229,   433,   687,   530,
      799,   805,   642,   983,   162,   337,   679,   703,
      664,  1002,   327,   996,   355,   862,  1023,   503,
      145,   274,   735,   898,   582,    36,   190,   884,
      836,   841,   347,   153,   633,   512,   167,   341,
      234,   255,    27,   391,   502,   212,   897,   366,
       67,   941,   554,   518,   781,   742,   976,   473,
      143,   515,   678,    74,   879,   244,   146,   367,
      189,    61,   659,   516,   447,   751,  1009,   936,
      505,   334,   741,   713,   544,   304,   127,   510,
      536,   507,   465,   894,   496,   392,   905,   218,
      537,   927,   245,   732,   436,   111,   817,   396,
      640,   389,   152,   261,   180,   951,   714,   658,
      122,   282,   826,   743,   683,   548,   820,   860,
      495,    46,    44,   622,     6,   318,   324,    14,
      966,   553,   407,   384,   840,   132,   761,   795,
      150,   125,   957,   883,   806,   977,   476,   834,
      878,   454,   965,   172,   535,   405,   607,   336,
       98,   710,   441,   220,   948,   479,   872,   154,
      323,   630,   259,   279,   539,    82,   551,   748,
      435,   149,    16,   937,   584,   997,   448,   342,
      246,   994,   552,    22,   555,   612,   140,   605,
     1004,   641,   570,   920,   843,   875,    83,   494,
      701,   726,   766,   275,   853,   156,   586,   107,
      423,   533,   277,   508,    60,    97,   311,    76,
     1012,    54,   415,    53,   185,   909,   569,   139,
      880,   606,   813,   519,    38,   718,  1018,   935,
      876,   682,   971,   816,   609,   272,   724,   485,
      157,   524,   131,   215,  1003,   531,   286,   260,
     1011,   690,   930,   322,   900,   159,   344,   213,
      785,   617,   932,   818,   770,   201,   137,   810,
      814,   170,   266,   808,   308,   723,   486,   100,
      254,   206,   759,   142,   763,   809,   973,   549,
      869,   907,   646,   649,   636,   401,   802,   670,
      852,   868,   397,   222,   931,   975,   353,   262,
      141,   309,  1019,   838,   747,  1001,   887,   463,
      155,   752,   704,  1006,   361,   498,   990,   444,
      882,   627,   596,   578,   916,   592,   418,   108,
      480,   400,   278,    90,   922,   739,   294,   340,
      655,   193,   365,   558,   126,   129,   540,   594,
      514,   736,    42,    52,   331,    45,   608,   784,
      744,   800,    62,   765,   164,   192,   969,   451,
      625,    57,   410,   248,   699,   239,   287,   661,
      571,   499,   269,   822,    79,   509,   408,   330,
       77,   263,   685,   698,   312,   771,   776,   284,
      117,   313,   187,   130,   457,   521,   491,   848,
      114,   252,   388,   991,   688,   492,   979,   200,
     1022,   478,   513,   362,    94,   211,   379,   821,
      610,    21,   168,    93,   716,   326,   351,   859,
       33,   777,   783,   790,   562,   281,   214,   756,
      675,   439,    95,   593,    88,    85,   216,   297,
      399,   740,   428,   924,   599,   483,   961,   374,
      787,   369,    51,   870,   755,   517,   619,   674,
       80,   264,   481,   769,   921,   915,   588,   409,
       39,   580,   730,   350,   437,   443,   302,   888,
      257,   652,   708,   383,    91,   963,   680,   247,
      940,   446,    70,   394,   645,   903,   179,    99,
      731,   737,   993,   464,   144,   472,   317,   310,
      665,   746,   986,    26,   918,   528,   315,   115,
      835,   199,   425,   595,     1,   691,   290,   112,
      603,   871,   585,   456,   232,   402,   184,   947,
      354,   506,   422,   959,   981,   796,    15,   368,
      504,   954,   237,   557,   177,   725,   296,   621,
      421,   376,     9,   827,   440,   138,   460,   128,
      414,   208,   855,   851,   489,    87,   522,   240,
      943,   357,   913,    12,   493,  1021,   320,   429,
       32,   988,   968,   462,   373,   416,   387,   854,
      693,   227,   385,   109,   858,    84,   119,   604,
      807,   611,   568,   238,   696,   381,    49,   291,
      624,  1014,   750,    13,   500,   912,   316,   591,
      430,   370,   839,   842,   758,   202,   938,    48,
      669,   803,  1017,   250,   998,   600,   695,   458,
      601,   175,   466,   671,   197,   300,   885,   233,
      689,   847,   538,   160,   651,   833,    17,   925,
      303,   844,   910,     3,   632,   919,    55,   780,
      360,   992,   205,   563,    29,   134,   974,   663,
      660,  1013,   390,   501,   845,   874,   332,   891,
      886,   228,   597,   786,   113,    63,   984,   801,
      614,   719,   681,   620,   734,   958,   692,   377,
      116,   928,   789,   183,   831,   438,   176,   314,
      349,   236,   811,   241,   929,   946,   270,   345,
      225,   133,    19,   104,   194,   904,   196,   782,
      338,   942,   118,  1015,   468,   511,   792,   329
};

/* The hash table itself */

symbol * hash_table[HASH_MAX+1];

/*
 *  FUNCTION : symbol_hash
 *
 *  Calculate a hash value for a given symbol. This should be fast.
 *
 *  PARAMS :
 *		name		ASCIIZ string with the symbol name
 *
 *  RETURN VALUE :
 *      Integer hash value from 0 to HASH_MAX
 *
 */

static int symbol_hash (const char * name)
{
	int result = 0;

	while (*name)
		result ^= scatter[(int)*name++];

	return result;
}

/*
 *  FUNCTION : fnc_export
 *
 *  Add a new symbol to the symbol tables
 *
 *  PARAMS :
 *		name		Name of the symbol to search
 *		addr		Symbol address (to be returned by fnc_import)
 *
 *  RETURN VALUE :
 *      None
 *
 */

void fnc_export (const char * name, void * addr)
{
	symbol * s = malloc(sizeof(symbol));
	int hash = symbol_hash(name);

	if (s != NULL)
	{
		s->name = name;
		s->count = 0;
		s->addr = addr;
		s->next = hash_table[hash];
		hash_table[hash] = s;
	}
}

/*
 *  FUNCTION : fnc_import
 *
 *  Search for a symbol in the symbol table and return its address
 *  This search is case sensitive and all characters are meaningful
 *
 *  PARAMS :
 *		name		Name of the symbol to search
 *
 *  RETURN VALUE :
 *      The symbol pointer or NULL if the symbol was not found
 *
 */

void * fnc_import (const char * name)
{
    symbol * ptr = hash_table[symbol_hash(name)] ;

    while (ptr != NULL)
	{
		if (strcmp (ptr->name, name) == 0)
		{
			ptr->count++;
			global_count++;
			return ptr->addr;
		}
        ptr = ptr->next;
    }

	global_notfound++;

	gr_con_printf ("[FXI] Warning: symbol '%s' not found!", name);
	return NULL;
}

/*
 *  FUNCTION : fnc_init
 *
 *  Initialize the exported symbol table with all the default
 *  symbols. This function must be called before any call to fnc_export
 *
 *  PARAMS :
 *		None
 *
 *  RETURN VALUE :
 *      None
 *
 */

void fnc_init()
{
	int n;
	int v;
	symbol * ptr = default_symbols;

	for (n = 0 ; n <= HASH_MAX ; n++)
		hash_table[n] = 0;

	while (ptr->name)
	{
		v = symbol_hash (ptr->name);
		ptr->next = hash_table[v];
		hash_table[v] = ptr;
		ptr++;
	}
}

/*
 *  FUNCTION : fnc_show_information
 *
 *  Examine the export information collected by fnc_export and
 *  show in the console any debug information that could be
 *  useful about non-found symbols and symbols not used by the DLLs
 *
 *  PARAMS :
 *		None
 *
 *  RETURN VALUE :
 *      None
 *
 */

void fnc_show_information()
{
	int n, m, count;
	int max_use = 0;
	int bucket_use[HASH_MAX+1];

	/* If no symbol imported by any DLL, do nothing */

	if (global_count == 0)
		return;

	/* Report all symbols that were not imported by DLL
	 * because we are importing all symbols currently and
	 * this helps to find exporting symbols missing from fxdll.h.
	 * In the future this should not be done
	 */

	memset (bucket_use, 0, sizeof(bucket_use));

	for (n = 0 ; n <= HASH_MAX ; n++)
	{
		symbol * ptr = hash_table[n];

		while (ptr != NULL)
		{
			if (ptr->count == 0)
				gr_con_printf ("[FXI] Warning: '%s' not imported!", ptr->name);

			bucket_use[n]++;
			if (max_use < bucket_use[n])
				max_use = bucket_use[n];

			ptr = ptr->next;
		}
	}

	gr_con_printf ("[FXI] %4d symbols imported by DLL", global_count);

	/* Show information about hash table proficiency */

	for (n = 1 ; n <= max_use ; n++)
	{
		for (m = count = 0 ; m <= HASH_MAX ; m++)
		{
			if (bucket_use[m] == n)
				count++;
		}

		if (count > 0)
			gr_con_printf ("[FXI] %4d buckets of size %d", count, n);
	}

	/* Not found symbols are reported by fnc_import */

	if (global_notfound)
		gr_con_printf ("[FXI] %4d symbols not found", global_notfound);
}

