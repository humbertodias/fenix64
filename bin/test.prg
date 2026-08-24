// --------------------------------------------------------------------------
// Fenix Tests                                         2000 José Luis Cebrián
// --------------------------------------------------------------------------

GLOBAL option ;

PROCESS Menu()
PRIVATE i, c, map ;
BEGIN
    load_fpg ("test.fpg") ;
    set_fps (30, 4) ;
    write (0, 160,  4, 1, "Fenix tests") ;
    write (0, 160, 14, 1, "Select your option") ;
    write (0, 60,  50, 0, "1. Basic sprite drawing") ;
    write (0, 60,  60, 0, "2. Blending operations") ;
    write (0, 60,  70, 0, "3. Primitive drawing") ;
    write (0, 60,  80, 0, "4. Scroll interactive test") ;
    write (0, 60,  90, 0, "5. Mode 7 interactive test") ;
    write (0, 60, 100, 0, "6. Path-finding interactive test") ;
    write (0, 60, 110, 0, "7. Sound test") ;
    write (0, 60, 150, 0, "0. Exit") ;
    map = new_map (320, 200, 8) ;
    map_clear (0, map, 0) ;
    option = 0 ;
    REPEAT
        IF key(_1): option = 1; END
        IF key(_2): option = 2; END
        IF key(_3): option = 3; END
        IF key(_4): option = 4; END
        IF key(_5): option = 5; END
        IF key(_6): option = 6; END
        IF key(_7): option = 7; END
        IF key(_0): BREAK; END
        c = rgb (rand(32,128), rand(32,64), rand(32,64)) ;
        x = timer/5%100 ;
        IF (x > 50) x = 100-x; END
        xput (0, map, 160, 100, timer*-50, 200+x, 4, 0) ;
        FROM i = 0 TO 100:
            map_put_pixel (0, map, rand(0,320), rand(0, 200), c) ;
            map_put_pixel (0, map, rand(0,320), rand(0, 200), 1) ;
        END
        FRAME ;
    UNTIL option != 0;
    unload_map(0, map) ;
END

PROCESS Header(string title)
BEGIN
    delete_text (0) ;
    write     (0, 160,  4, 1, title) ;
    write     (0, 155, 15, 2, "FPS:") ;
    write_int (0, 165, 15, 0, &fps) ;
    write     (0, 155, 25, 2, "Speed %:") ;
    write_int (0, 165, 25, 0, &speed_gauge) ;
    write     (0, 160, 185, 1, "Press ESC to continue") ;
END

// ----------------------------------------------------------------------
// Simple sprite drawing
// ----------------------------------------------------------------------

PROCESS FallingObjects()
BEGIN
    set_mode (320, 200, 16);
    put_screen (0, 1);

    Header ("400 sprites") ;
    FROM x = 0 TO 400: FallingObject(100, 100, 0, 0); END
    WHILE !key(_ESC): FRAME; END;
    signal (TYPE FallingObject, S_KILL) ;
    WHILE  key(_ESC): FRAME; END;
    Header ("100 rotating sprites") ;
    FROM x = 0 TO 100: FallingObject(101, 100, 0, rand(500,1000)); END
    WHILE !key(_ESC): FRAME; END
    signal (TYPE FallingObject, S_KILL) ;
    WHILE  key(_ESC): FRAME; END;
    Header ("100 sprites with transparency") ;
    FROM x = 0 TO 100: FallingObject(100, 100, 4, 0); END
    WHILE !key(_ESC): FRAME; END
    signal (TYPE FallingObject, S_KILL) ;
    WHILE  key(_ESC): FRAME; END;
    Header ("100 scaled up sprites") ;
    FROM x = 0 TO 100: FallingObject(100, 150, 0, 0); END
    WHILE !key(_ESC): FRAME; END
END

PROCESS FallingObject(graph, size, flags, angleinc)
PRIVATE xspeed, yspeed, inispeed ;
BEGIN
    z      = rand (-5, 125) ;
    x      = rand (15, 305) ;
    y      = - rand (20, 100) ;
    xspeed = rand (-10, 10) ;
    yspeed = rand (-4, 0) ;
    inispeed = rand (10, 15) ;
    angle  = rand (0, 50 * angleinc) ;
    LOOP
        alpha = timer;
        x += xspeed ;
        IF (x > 305 || x < 15) xspeed = -xspeed; END
        y += yspeed++ ;
        IF (yspeed < -14) yspeed = -14; END
        IF (y > 180)
            yspeed = -inispeed ;
            IF (inispeed > 1)
                inispeed--;
            ELSE
                inispeed = 15;
            END
        END
        IF (out_region(ID, 0)) y = -rand(20, 100) ; inispeed = 15 ; END

        angle += angleinc ;
        FRAME ;
    END
END

// ----------------------------------------------------------------------
// Blendop tests
// ----------------------------------------------------------------------

PROCESS BlendedObject (x, y, string text, string text2, int flags, blendop)
BEGIN
    write (0, x, y+32, 4, text) ;
    write (0, x, y+40, 4, text2) ;
    angle = 1000 ;
    size = 150 ;
    graph = 101 ;
    LOOP
        angle += 2000 ;
        FRAME ;
    END
END

PROCESS BlendedObjects()
BEGIN
    graph_mode = mode_16bits ;
    set_mode (m320x200) ;
    put_screen (0, 1) ;
    Header ("Blit flags") ;
    BlendedObject (60, 65, "Normal", "", 0, 0) ;
    BlendedObject (160, 65, "H-Mirror", "", 1, 0) ;
    BlendedObject (260, 65, "V-Mirror", "", 2, 0) ;
    BlendedObject (60, 135, "Normal", "Transparency", 4, 0) ;
    BlendedObject (160, 135, "H-Mirror", "Transparency", 5, 0) ;
    BlendedObject (260, 135, "No key", "", 128, 0) ;
    WHILE !scan_code: FRAME; END
    scan_code = 0 ;
    signal (TYPE BlendedObject, S_KILL) ;
    Header ("Blending operations") ;
    blendop = blendop_new() ;
    blendop_translucency (blendop, 0.70) ;
    BlendedObject (60, 65, "Transparency", "(70%)", 0, blendop) ;
    blendop = blendop_new() ;
    blendop_grayscale (blendop, 1) ;
    BlendedObject (160, 65, "Grayscale", "(Luminance)", 0, blendop) ;
    blendop = blendop_new() ;
    blendop_intensity (blendop, 2.00) ;
    BlendedObject (260, 65, "Lighting", "(50%)", 0, blendop) ;
    blendop = blendop_new() ;
    blendop_tint (blendop, 0.50, 255, 0, 0) ;
    BlendedObject (60, 135, "Red tint", "(50%)", 0, blendop) ;
    blendop = blendop_new() ;
    blendop_intensity (blendop, 0.75) ;
    BlendedObject (160, 135, "Darkening", "(25%)", 0, blendop) ;
    blendop = blendop_new() ;
    blendop_grayscale (blendop, 1) ;
    blendop_intensity (blendop, 3.0) ;
    blendop_swap (blendop);
    BlendedObject (260, 135, "Inverted", "(Grayscale)", 0, blendop) ;
    WHILE !scan_code: FRAME; END
    fade_off() ;
    graph_mode = 0 ;
    set_mode (m320x200) ;
END

// ----------------------------------------------------------------------
// Primitive drawing
// ----------------------------------------------------------------------

PROCESS PrimitiveTests()
PRIVATE
    map, color, i ;
    word POINTER ptr ;
BEGIN
    load_fpg   ("test.fpg") ;
    Header ("Primitive drawing") ;
    graph = new_map (400, 400, 8) ;
    set_center (0, graph, 200, 200) ;
    drawing_map (0, graph) ;
    x = 160 ; y = 100 ; flags = 4 ;
    REPEAT
        drawing_color (rgb (rand(0,255), rand(0,255), rand(0,255))) ;
        draw_fcircle  (rand(0,399), rand(0,399), rand(0,25)) ;
        draw_rect     (rand(0,399), rand(0,399), rand(0,399), rand(0,399)) ;
        draw_line     (rand(0,399), rand(0,399), rand(0,399), rand(0,399)) ;
        draw_circle   (rand(0,399), rand(0,399), rand(0,100)) ;
        FRAME ;
        angle += 1000 ;
    UNTIL scan_code;
    unload_map (0, graph) ;
END

// ----------------------------------------------------------------------
// Scrolling
// ----------------------------------------------------------------------

PROCESS ScrollTest()
BEGIN
    define_region (1,   0,   0, 160, 100) ;
    define_region (2,   0, 100, 160, 100) ;
    define_region (3, 160,   0, 160, 100) ;
    define_region (4, 160, 100, 160, 100) ;
    start_scroll(0, 0, 200, 1, 1, 15);
    start_scroll(1, 0, 200, 1, 2, 15);
    start_scroll(2, 0, 200, 1, 3, 15);
    start_scroll(3, 0, 200, 1, 4, 15);
    scroll[0].camera = ScrollSphere (160, 100, 100) ;
    scroll[1].camera = ScrollTriangle (scroll[0].camera, 0) ;
    scroll[2].camera = ScrollTriangle (scroll[0].camera, 120000) ;
    scroll[3].camera = ScrollTriangle (scroll[0].camera, 240000) ;
    scroll[0].ratio  = 400 ;
    scroll[1].ratio  = 800 ;
    scroll[2].ratio  = 50 ;
    scroll[3].ratio  = 2000 ;
    scroll[0].speed  = 3 ;
    scroll[0].flags1 = 4 ;
    scroll[1].flags1 = 4 ;
    scroll[2].flags1 = 4 ;
    scroll[3].flags1 = 4 ;
    scroll[0].flags2 = 129 ;
    scroll[1].flags2 = 129 ;
    scroll[2].flags2 = 129 ;
    scroll[3].flags2 = 129 ;
    Header ("Scroll test") ;
    REPEAT: FRAME; UNTIL key(_esc);
    fade_off() ;
    stop_scroll(0) ;
    stop_scroll(1) ;
    stop_scroll(2) ;
    stop_scroll(3) ;
END

PROCESS ScrollSphere (x, y, graph)
BEGIN
    priority = 100 ;
    ctype = c_scroll;
    LOOP
        IF (key(_right)) x += 4 ; END
        IF (key(_left))  x -= 4 ; END
        IF (key(_down))  y += 4 ; END
        IF (key(_up))    y -= 4 ; END
        IF (key(_space)) x = y = 0 ; END
        FRAME;
    END
END

PROCESS ScrollTriangle (follow, angle)
BEGIN
    priority = 101 ;
    graph = 102 ;
    ctype = c_scroll ;
    LOOP
        x = follow.x + get_distx (angle, 40) ;
        y = follow.y + get_disty (angle, 40) ;
        angle += 185000 ;
        FRAME ;
        angle -= 180000 ;
    END
END

// ----------------------------------------------------------------------
// Mode 7
// ----------------------------------------------------------------------

PROCESS Mode7Test()
PRIVATE
    a, b ;
BEGIN
    put_screen (0, 1) ;
    Header ("Mode 7 - Press cursor keys, Q, W, A, Z") ;
    start_mode7 (0, 0, 1, 0, 0, 64) ;
    start_mode7 (1, 0, 0, 200, 0, 64) ;

    m7[0].distance = 0 ;
    m7[0].camera = id ;
    m7[0].height = 90 ;

    m7[1].distance = 0 ;
    m7[1].camera = id ;
    m7[1].height = 80 ;
    m7[1].flags  = 4 ;

    resolution = 100 ;
    FROM a = 0 TO 64 Step 16:
        FROM b = 0 TO 64 Step 16:
            Mode7Object (a, b, 100) ;
        END
    END
    FRAME;
    LOOP
        m7[0].z = m7[0].height ;
        m7[1].z = m7[1].height ;
        IF (m7.height < 0)
            m7[0].z = -m7[0].height ;
            m7[1].z = -m7[1].height ;
        END
        IF (key(_up))   advance(200) ; END
        IF (key(_down)) advance(-200) ; END
        IF (key(_left)) angle += 2000 ; END
        IF (key(_right)) angle -= 2000 ;END
        IF (key(_a)) m7.height+=2 ; m7[1].height+=2 ; END
        IF (key(_z)) m7.height-=2 ; m7[1].height-=2 ; END
        IF (key(_q)) xadvance(angle+90000, 200) ; END
        IF (key(_w)) xadvance(angle-90000, 200) ; END
        scan_code = 0 ;
        FRAME ;
        IF key(_esc): BREAK; END
    END
    fade_off() ;
    stop_mode7(0) ;
    stop_mode7(1) ;
END

PROCESS Mode7Object (x, y, graph)
PRIVATE
    speed = -1 ;
BEGIN
    ctype = c_m7 ;
    cnumber = 1 ;
    height = rand (0, 16) ;
    resolution = 4 ;
    x *= 4 ; y *= 4 ;
    LOOP
        IF height > 0 && speed > -8: speed-- ; END
        height += speed ;
        IF (height < 0)
            height = 0 ;
            speed = rand(5, 15) ;
        END
        FRAME;
    END
END

// ----------------------------------------------------------------------
// Path-finding
// ----------------------------------------------------------------------

CONST
    width   = 64 ;
    height  = 32 ;

GLOBAL
    int     graphic ;
    int     title ;
    int     wall_color ;
    int     start_color ;
    int     end_color ;
    int     color_ruta ;
    int     last_x, last_y ;
    int     start_x, start_y ;
    int     end_x, end_y ;

PROCESS ClearRoute ()
PRIVATE
    byte POINTER ptr ;
    int pitch ;
BEGIN
    ptr = map_buffer (0, graphic) ;
    pitch = graphic_info(0, graphic, G_PITCH) ;
    FROM y = 0 TO HEIGHT:
        ptr = map_buffer(0,graphic) ;
        ptr += pitch * y ;
        FROM x = 0 TO WIDTH:
            IF [ptr] == color_ruta: [ptr] = 0; END
            ptr++ ;
        END
    END
END

PROCESS Reset()
BEGIN
    drawing_color (0) ;
    draw_box (0, 0, WIDTH, HEIGHT) ;
    drawing_color (wall_color) ;
    draw_rect (0, 0, WIDTH, HEIGHT) ;
    start_x = WIDTH/4 ;
    end_x = WIDTH*3/4 ;
    start_y = HEIGHT/2 ;
    end_y = HEIGHT/2 ;
    map_put_pixel (0, graphic, start_x, start_y, start_color) ;
    map_put_pixel (0, graphic, end_x, end_y, end_color) ;
END

PROCESS PathFinding()
BEGIN
    mouse.graph = 200 ;
    graphic = new_map (WIDTH, HEIGHT, 8) ;
    wall_color = rgb (196, 196, 196) ;
    start_color = rgb (255, 0, 0) ;
    end_color = rgb (0, 0, 255) ;
    color_ruta = rgb (128, 64, 128) ;
    drawing_map (0, graphic) ;
    Reset() ;
    title = write (0, 160, 4, 1, "Path finding") ;
    write (0, 160, 180, 1, "Draw walls with the mouse") ;
    write (0, 160, 190, 1, "(S) start (E) end (R) reset (SPACE) go") ;
    last_x = mouse.x / 4 ;
    last_y = mouse.y / 4 ;
    WHILE !key(_esc):
        clear_screen() ;
        xput (0, graphic, 160, 100, 0, 400, 0, 0) ;
        x = mouse.x / 4 - (40-WIDTH/2);
        y = mouse.y / 4 - (25-HEIGHT/2);

        IF mouse.left:
            drawing_color (wall_color) ;
            draw_line (x, y, last_x, last_y) ;
            map_put_pixel (0, graphic, x, y, wall_color) ;
        END
        IF mouse.right:
            drawing_color (0) ;
            draw_line (x, y, last_x, last_y) ;
            map_put_pixel (0, graphic, x, y, 0) ;
        END
        IF key(_s):
            ClearRoute() ;
            map_put_pixel (0, graphic, start_x, start_y, 0) ;
            start_x = x ; start_y = y ;
            map_put_pixel (0, graphic, x, y, start_color) ;
        END
        IF key(_r):
            Reset() ;
        END
        IF key(_e):
            ClearRoute() ;
            map_put_pixel (0, graphic, end_x, end_y, 0) ;
            end_x = x ; end_y = y ;
            map_put_pixel (0, graphic, x, y, end_color) ;
        END
        IF key(_space):
            FRAME ;
            ClearRoute() ;
            IF !path_find(0, graphic, start_x, start_y,
                end_x, end_y, PF_NODIAG):
                delete_text(title) ;
                title = write (0, 160, 6, 1, "The path is blocked") ;
            ELSE:
                FRAME ;
                clear_screen() ;
                delete_text(title) ;
                title = write (0, 160, 6, 1, "Path found in " + (speed_gauge*60/100) + " ms") ;
                set_fps (60, 0) ;
                WHILE path_getxy(&x, &y):
                    map_put_pixel (0, graphic, x, y, color_ruta) ;
                    xput (0, graphic, 160, 100, 0, 400, 128, 0) ;
                    FRAME ;
                END
                set_fps (60, 0) ;
                map_put_pixel (0, graphic, start_x, start_y, start_color) ;
                map_put_pixel (0, graphic, end_x, end_y, end_color) ;
            END
            WHILE key(_space): FRAME; END
        END
        IF key(_esc): BREAK; END
        last_x = x ;
        last_y = y ;
        FRAME ;
    END
    unload_map (0, graphic) ;
    fade_off() ;
END

// ------------------------------------------------------------------------
// Sound test
// ------------------------------------------------------------------------

PROCESS SoundTest()
PRIVATE
    int pat, pos, vol, time, md ;
BEGIN
    header ("Sound test") ;
    md = load_song("game.s3m") ;
    play_song (md,-1) ;
    vol = 128 ;
    WHILE is_playing_song() AND scan_code != _esc:
        FRAME ;
    END
    WHILE is_playing_song() AND vol > 0:
        FRAME ;
        set_song_volume ( vol -= 2) ;
    END
    stop_song() ;
    unload_song(md) ;
END

// ----------------------------------------------------------------------

PRIVATE proc ;
BEGIN
    FULL_SCREEN = FALSE ;
    LOOP
        proc = Menu() ;
        WHILE exists(proc): FRAME; END
        fade_off() ;
        put_screen(0, 1) ;
        delete_text(0) ;
        fade_on() ;
        IF option == 0: BREAK; END
        SWITCH (option)
            CASE 1: proc = FallingObjects(); END
            CASE 2: proc = BlendedObjects(); END
            CASE 3: proc = PrimitiveTests(); END
            CASE 4: proc = ScrollTest(); END
            CASE 5: proc = Mode7Test(); END
            CASE 6: proc = PathFinding(); END
            CASE 7: proc = SoundTest(); END
        END
        WHILE exists(proc): FRAME; END
        fade_off() ;
        let_me_alone() ;
        clear_screen() ;
        delete_text(0) ;
        fade_on() ;
    END
END
