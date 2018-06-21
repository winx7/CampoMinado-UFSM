//c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//allegro
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>


int ScrW = 512, ScrH = 512;

ALLEGRO_DISPLAY * scr;

ALLEGRO_FONT * m_font, * i_font, * t_font, *counter_f;

ALLEGRO_BITMAP * bg_glow;
ALLEGRO_BITMAP * bframe;
ALLEGRO_BITMAP * sq;

ALLEGRO_EVENT_QUEUE * eq;

ALLEGRO_TIMER * timer;

static ALLEGRO_COLOR white;
//float animCoef;

ALLEGRO_SAMPLE * bhover;
ALLEGRO_SAMPLE * bclick;
ALLEGRO_SAMPLE * back;
ALLEGRO_SAMPLE * start;
ALLEGRO_SAMPLE * quit;

ALLEGRO_SAMPLE_ID * bhI;

//Funções Globais

int i, j, k;
int setup(char*);

bool redraw;
bool hovering = false;
bool clickable = true;

int selX = 0, selY = 1024;

float ra, ga, ba, bga;
double drtime;

void drawInitMenu();
void initMAxLogic(ALLEGRO_EVENT);
bool initMBuLogic(ALLEGRO_EVENT);

void drawDiffMenu();
void difMAxLogic(ALLEGRO_EVENT);
bool difMBuLogic(ALLEGRO_EVENT);

bool createMineField(int, int, int);

void drawGame();
void gameMAxLogic(ALLEGRO_EVENT);
bool gameMBuLogic(ALLEGRO_EVENT);

void closeAll();

int err(unsigned char);

//ponteiros de funcao

void(*curDFunc)();
void(*curMAxLFunc)(ALLEGRO_EVENT);
bool(*curMBuLFunc)(ALLEGRO_EVENT);


// lógica do jogo em si

typedef struct mfield {

    short unsigned int m;
    short unsigned n;
    short unsigned int mnum;

    bool mines[64][64];
    signed char plyfield[64][64];

} mfield;

mfield minefield;

int setup(char title[]) {
    if (!al_init())
        return err(0);

    if (!al_init_image_addon())
        return err(1);

    if (!al_install_keyboard())
        return err(2);

    if (!al_install_mouse())
        return err(3);

    timer = al_create_timer(1.0 / 100.0);
    if (!timer)
        return err(4);

    eq = al_create_event_queue();

    if (!eq)
        return err(5);

    if (!al_init_primitives_addon())
        return err(6);

    if (!al_install_audio())
        return err(7);

    if (!al_init_acodec_addon())
        return err(8);

    if (!al_reserve_samples(8))
        return err(9);

    bhover = al_load_sample("sfx/buttonhover.wav");
    bclick = al_load_sample("sfx/buttonclick.wav");
    back = al_load_sample("sfx/back.wav");
    start = al_load_sample("sfx/start.wav");
    quit = al_load_sample("sfx/quit.wav");

    bhI = al_create_sample_instance(bhover);

    //al_register_event_source(eq, al_get_display_event_source(&scr)); // tá dando crash
    //al_register_event_source(eq, al_get_display_event_source(&scr));

    al_register_event_source(eq, al_get_timer_event_source(timer));

    al_register_event_source(eq, al_get_mouse_event_source());

    al_init_font_addon();

    al_init_ttf_addon();

    t_font = al_load_ttf_font("res/square-deal.ttf", 48, 0);
    counter_f = al_load_ttf_font("res/square-deal.ttf", 24, 0);
    m_font = al_load_ttf_font("res/alterebro-pixel-font.ttf", 36, 0);
    i_font = al_load_ttf_font("res/alterebro-pixel-font.ttf", 24, 0);

    scr = al_create_display(ScrW, ScrH);

    al_set_window_title(scr, title);
    //al_set_display_flag(scr, ALLEGRO_NOFRAME, true);

    bg_glow = al_load_bitmap("gfx/bg_glow.png");
    bframe = al_load_bitmap("gfx/glass.png");
    sq = al_load_bitmap("gfx/sq.png");

    al_flip_display();
    al_start_timer(timer);

    return 1;
}

int main() {

    if (!setup("Campo Minado")) // se não conseguir inicializar o jogo, cancela tudo
        return -1;

    srand(time(NULL));
//pra reduzir o numero de comparações no loop do jogo
    white = al_map_rgb(255, 255, 255);
    //ALLEGRO_EVENT ev;
    curDFunc = &drawInitMenu; //começa com as funcoes do menu
    curMAxLFunc = &initMAxLogic;
    curMBuLFunc = &initMBuLogic;

    bool quit = true; // pra n ter que colocar um ! no while

    while (quit) {

        ALLEGRO_EVENT ev;
        al_wait_for_event(eq, & ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            redraw = true;

        } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            quit = false;

        } else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {

            curMAxLFunc(ev);

        } else if (ev.type == ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY) {

            hovering = false;
            curMAxLFunc(ev);

        } else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {

            quit = curMBuLFunc(ev);

        }

        if (redraw && al_is_event_queue_empty(eq)) {
            redraw = false;
            curDFunc();
        }
    }

    closeAll();

    return 0;
}



void drawInitMenu() {

    drtime = al_get_time();
    al_clear_to_color(al_map_rgb(0, 0, 0));

    bga = (cos((2 * drtime) + 2)/4)+0.75;

    al_color_hsl_to_rgb((sin(drtime/4)*180)+180, 1.0, 0.3, &ra, &ga, &ba);

    al_draw_tinted_bitmap(bg_glow, al_map_rgba_f(ra * bga, ga * bga, ba * bga, bga), 0, 0, NULL);

    al_draw_text(t_font, white, ScrW / 2, 128 - sin(drtime * 2) * 9, ALLEGRO_ALIGN_CENTRE, "campo minado");

    al_draw_text(m_font, white, ScrW / 2, 238, ALLEGRO_ALIGN_CENTER, "Iniciar");
    al_draw_text(m_font, white, ScrW / 2, 274, ALLEGRO_ALIGN_CENTER, "Placar");
    al_draw_text(m_font, white, ScrW / 2, 310, ALLEGRO_ALIGN_CENTER, "Opções");

    al_draw_text(m_font, white, ScrW / 2, 398, ALLEGRO_ALIGN_CENTER, "Sair");

    al_draw_text(m_font, al_map_rgb(255 - (255*ra), 255 - (255*ga), 255 - (255*ba)), (ScrW / 2) - sin(drtime) * 280, 458, ALLEGRO_ALIGN_CENTER, "Desenvolvido por Juarez Corneli Filho, Andrey Avila, Gabriel Tanski e Gabriel Jaymes - Telecom 2018/1 UFSM");

    al_draw_text(m_font, white, ScrW / 2, selY, ALLEGRO_ALIGN_CENTER, "#                            #");
    al_draw_rectangle(ScrW / 2 - 128, selY, ScrW / 2 + 128, selY + 36, white, 2);

    al_flip_display();

}

void initMAxLogic(ALLEGRO_EVENT ev) {
    if (ev.mouse.y <= 272 && ev.mouse.y >= 240) {
        if (!hovering) {
            al_stop_sample(bhI);
            hovering = true;
            al_play_sample(bhover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        }
        selY = 238;
    } else if (ev.mouse.y <= 308 && ev.mouse.y >= 276) {
        if (!hovering) {
            al_stop_sample(bhI);
            hovering = true;
            al_play_sample(bhover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        }
        selY = 274;
    } else if (ev.mouse.y <= 344 && ev.mouse.y >= 316) {
        if (!hovering) {
            hovering = true;
            al_stop_sample(bhI);
            al_play_sample(bhover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        }
        selY = 310;
    } else if (ev.mouse.y <= 434 && ev.mouse.y >= 400) {
        if (!hovering) {
            al_stop_sample(bhI);
            hovering = true;
            al_play_sample(bhover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        }
        selY = 398;
    } else {
        hovering = false;
        selY = 1024;
    }
}

bool initMBuLogic(ALLEGRO_EVENT ev) {

    if(!hovering)
        return true;
    if (ev.mouse.y <= 272 && ev.mouse.y >= 240) {
        al_play_sample(bclick, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        if(clickable)
        {
            curDFunc = &drawDiffMenu;
            curMAxLFunc = &difMAxLogic;
            curMBuLFunc = &difMBuLogic;
            selY = 1024;
            hovering = false;
        }
    } else if (ev.mouse.y <= 308 && ev.mouse.y >= 276) {
        al_play_sample(bclick, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
    } else if (ev.mouse.y <= 344 && ev.mouse.y >= 316) {
        al_play_sample(bclick, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
    } else if (ev.mouse.y <= 434 && ev.mouse.y >= 400) {
        al_play_sample(back, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        return false;
    } else {
        hovering = false;
        selY = 1024;
        return true;
    }
}

void drawDiffMenu() {

    drtime = al_get_time();
    al_clear_to_color(al_map_rgb(0, 0, 0));

    /*bga = sin(fmod(al_get_time() / 2, 3.14)) / 4.0 + 0.2;
    ra = max(sin(drtime), 0);
    ga = (abs(cos(drtime)) - sin(drtime - 1.57)) / 2 + 0.5;
    ba = max(sin(drtime + 4.71), 0);*/

    bga = (cos((2 * drtime) + 2)/4)+0.75;

    al_color_hsl_to_rgb((sin((drtime+3)/4)*180)+180, 1, 0.3, &ra, &ga, &ba);

    al_draw_tinted_bitmap(bg_glow, al_map_rgba_f(ra, ga, ba, bga), 0, 0, NULL);

    //al_draw_text(t_font, white, ScrW / 2, 41 - sin(drtime * 2) * 9, ALLEGRO_ALIGN_CENTRE, "Campo Minado");
    al_draw_text(m_font, white, ScrW / 2, 124, ALLEGRO_ALIGN_CENTER, "Escolha a dificuldade");

    al_draw_text(m_font, white, ScrW / 2, 178, ALLEGRO_ALIGN_CENTER, "Fácil");
    al_draw_text(m_font, white, ScrW / 2, 214, ALLEGRO_ALIGN_CENTER, "Intermediário");
    al_draw_text(m_font, white, ScrW / 2, 250, ALLEGRO_ALIGN_CENTER, "Difícil");
    al_draw_text(m_font, white, ScrW / 2, 286, ALLEGRO_ALIGN_CENTER, "Zanetti vs Lian");

    al_draw_text(m_font, white, ScrW / 2, 458, ALLEGRO_ALIGN_CENTER, "Voltar");
    //al_draw_text(m_font, white, (ScrW / 2) - cos(drtime) * 280, 458, ALLEGRO_ALIGN_CENTER, "Desenvolvido por Juarez Corneli Filho, Andrey Avila, Gabriel Tanski e Gabriel Jaymes - Telecom 2018/1 UFSM");

    al_draw_text(m_font, white, ScrW / 2, selY, ALLEGRO_ALIGN_CENTER, "x                            x");
    al_draw_rectangle(ScrW / 2 - 128, selY, ScrW / 2 + 128, selY + 36, white, 2);

    al_flip_display();

}

void difMAxLogic(ALLEGRO_EVENT ev) {

    if (ev.mouse.y <= 212 && ev.mouse.y >= 180) {
        if (!hovering) {
            al_stop_sample(bhI);
            hovering = true;
            al_play_sample(bhover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        }
        selY = 178;
    } else if (ev.mouse.y <= 248 && ev.mouse.y >= 216) {
        if (!hovering) {
            al_stop_sample(bhI);
            hovering = true;
            al_play_sample(bhover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        }
        selY = 214;
    } else if (ev.mouse.y <= 284 && ev.mouse.y >= 256) {
        if (!hovering) {
            hovering = true;
            al_stop_sample(bhI);
            al_play_sample(bhover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        }
        selY = 250;
    } else if (ev.mouse.y <= 320 && ev.mouse.y >= 288) {
        if (!hovering) {
            al_stop_sample(bhI);
            hovering = true;
            al_play_sample(bhover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        }
        selY = 286;
    } else if (ev.mouse.y <= 494 && ev.mouse.y >= 460) {
        if (!hovering) {
            al_stop_sample(bhI);
            hovering = true;
            al_play_sample(bhover, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        }
        selY = 458;
    } else {
        hovering = false;
        selY = 1024;
    }
}

bool difMBuLogic(ALLEGRO_EVENT ev) {

    if(!hovering)
        return true;
    if (ev.mouse.y <= 212 && ev.mouse.y >= 180) {
        al_play_sample(start, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);

        //if(!createMineField(9, 9, 10))

        createMineField(9, 9, 10);
        //return err(0);

        curDFunc = &drawGame;
        curMAxLFunc = &gameMAxLogic;
        curMBuLFunc = &gameMBuLogic;
        selY = 1024;
        hovering = false;
    } else if (ev.mouse.y <= 248 && ev.mouse.y >= 216) {
        al_play_sample(start, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        createMineField(16, 16, 40);
        //return err(0);

        curDFunc = &drawGame;
        curMAxLFunc = &gameMAxLogic;
        curMBuLFunc = &gameMBuLogic;
        selY = 1024;
        hovering = false;
    } else if (ev.mouse.y <= 284 && ev.mouse.y >= 256) {
        al_play_sample(start, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        createMineField(16, 30, 99);
        //return err(0);

        curDFunc = &drawGame;
        curMAxLFunc = &gameMAxLogic;
        curMBuLFunc = &gameMBuLogic;

        selY = 1024;
        hovering = false;
    } else if (ev.mouse.y <= 320 && ev.mouse.y >= 288) {
        al_play_sample(start, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        createMineField(1, 1, 1);
        //return err(0);

        curDFunc = &drawGame;
        curMAxLFunc = &gameMAxLogic;
        curMBuLFunc = &gameMBuLogic;
        selY = 1024;
        hovering = false;
    } else if (ev.mouse.y <= 494 && ev.mouse.y >= 460) {
        al_play_sample(back, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);

        curDFunc = &drawInitMenu;
        curMAxLFunc = &initMAxLogic;
        curMBuLFunc = &initMBuLogic;
        selY = 1024;
        hovering = false;

    } else {
        hovering = false;
        selY = 1024;
        return true;
    }
}


bool createMineField(int m, int n, int num) {

    minefield.m = m;
    minefield.n = n;

    unsigned short int rX, rY;


    for(i = 0; i<m; i++)
        for(j=0; j<n; j++)
            minefield.mines[i][j] = false;

    for(i = 0; i<m; i++)
        for(j=0; j<n; j++)
            minefield.plyfield[i][j] = -1;

    for(i = 0; i<(num); i++)
    {
        rX = rand()%m;
        rY = rand()%n;

        if(minefield.mines[rX][rY] == false)
            minefield.mines[rX][rY] = true;
        else
            i--;
    }

    return true;
}

void drawGame() {

    drtime = al_get_time();
    al_clear_to_color(al_map_rgb(0, 0, 0));

    /*bga = sin(fmod(al_get_time() / 2, 3.14)) / 4.0 + 0.2;
    ra = max(sin(drtime), 0);
    ga = (abs(cos(drtime)) - sin(drtime - 1.57)) / 2 + 0.5;
    ba = max(sin(drtime + 4.71), 0);*/

    bga = (cos((2 * drtime) + 2)/4)+0.75;

    al_color_hsl_to_rgb((sin((drtime-4)/8)*180)+180, 1, 0.3, &ra, &ga, &ba);

    al_draw_tinted_bitmap(bg_glow, al_map_rgba_f(ra, ga, ba, bga), 0, 0, NULL);

    al_draw_textf(t_font, white, ScrW / 2, 18, ALLEGRO_ALIGN_CENTER, ".");
    al_draw_textf(t_font, white, ScrW / 2, 32, ALLEGRO_ALIGN_CENTER, "02.59");
    al_draw_textf(counter_f, white, ScrW / 5, 40, ALLEGRO_ALIGN_CENTER, "n jogadas");
    al_draw_textf(counter_f, white, ScrW * 0.8, 40, ALLEGRO_ALIGN_CENTER, "n minas");

    for(i = 0; i < minefield.m; i++)
    {
        for(j = 0; j < minefield.n; j++) {

            if(minefield.mines[i][j])
            {
                al_draw_bitmap(sq, ScrW/2 - (minefield.n * 8) + (j*16) , ScrH/2 - (minefield.m * 8) + (i*16), NULL);
                al_draw_filled_circle(ScrW/2 - (minefield.n * 8) + (j*16) + 8, ScrH/2 - (minefield.m * 8) + (i*16) + 8, 8, white);
            } else {
                al_draw_bitmap(sq, ScrW/2 - (minefield.n * 8) + (j*16) , ScrH/2 - (minefield.m * 8) + (i*16), NULL);
            }
        }
    }


    /*al_draw_text(m_font, white, ScrW / 2, 124, ALLEGRO_ALIGN_CENTER, "Escolha a dificuldade");

    al_draw_text(m_font, white, ScrW / 2, 178, ALLEGRO_ALIGN_CENTER, "Fácil");
    al_draw_text(m_font, white, ScrW / 2, 214, ALLEGRO_ALIGN_CENTER, "Intermediário");
    al_draw_text(m_font, white, ScrW / 2, 250, ALLEGRO_ALIGN_CENTER, "Difícil");
    al_draw_text(m_font, white, ScrW / 2, 286, ALLEGRO_ALIGN_CENTER, "Zanetti vs Lian");

    al_draw_text(m_font, white, ScrW / 2, 458, ALLEGRO_ALIGN_CENTER, "Voltar");
    //al_draw_text(m_font, white, (ScrW / 2) - cos(drtime) * 280, 458, ALLEGRO_ALIGN_CENTER, "Desenvolvido por Juarez Corneli Filho, Andrey Avila, Gabriel Tanski e Gabriel Jaymes - Telecom 2018/1 UFSM");
      */
    al_draw_text(m_font, white, ScrW / 2, selY, ALLEGRO_ALIGN_CENTER, "x                            x");
    al_draw_rectangle(ScrW / 2 - 128, selY, ScrW / 2 + 128, selY + 36, white, 2);

    al_flip_display();

}

void gameMAxLogic(ALLEGRO_EVENT ev) {

    printf("Quad: %i x %i eq %i %i\n", ev.mouse.x - (ScrW/2) + (minefield.n*8), ev.mouse.y - (ScrH/2) + (minefield.m*8),
           (ev.mouse.x - (ScrW/2) + (minefield.n*8))/16, (ev.mouse.y - (ScrH/2) + (minefield.m*8))/16);

    ;

}

bool gameMBuLogic(ALLEGRO_EVENT ev) {

    if(!hovering)
        return true;
    if (ev.mouse.y <= 272 && ev.mouse.y >= 240) {
        al_play_sample(bclick, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        if(clickable)
        {
            curDFunc = &drawDiffMenu;
            curMAxLFunc = &difMAxLogic;
            curMBuLFunc = &difMBuLogic;
            selY = 1024;
            hovering = false;
        }
    } else if (ev.mouse.y <= 308 && ev.mouse.y >= 276) {
        al_play_sample(bclick, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
    } else if (ev.mouse.y <= 344 && ev.mouse.y >= 316) {
        al_play_sample(bclick, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
    } else if (ev.mouse.y <= 434 && ev.mouse.y >= 400) {
        al_play_sample(back, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
        return false;
    } else {
        hovering = false;
        selY = 1024;
        return true;
    }
}


void closeAll() {
    /*
    ALLEGRO_EVENT_QUEUE * eq;

    ALLEGRO_TIMER * timer;

    static ALLEGRO_COLOR white;
    //float animCoef;

    ALLEGRO_SAMPLE * bhover;
    ALLEGRO_SAMPLE * bclick;
    ALLEGRO_SAMPLE * back;
    ALLEGRO_SAMPLE * start;
    ALLEGRO_SAMPLE * quit;

    ALLEGRO_SAMPLE_ID * bhI;
    */

    al_destroy_bitmap(bg_glow);
    al_destroy_bitmap(bframe);
    al_destroy_font(m_font);
    al_destroy_font(i_font);
    al_destroy_font(t_font);
    al_destroy_sample(bhover);
    al_destroy_sample(bclick);
    al_destroy_sample(back);
    al_destroy_sample(start);
    al_destroy_sample(quit);
    al_destroy_timer(timer);
    al_destroy_display(scr);
    al_destroy_event_queue(eq);
}

int err(unsigned char id) {

    switch (id) {
    case 0:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde inicializar o Allegro.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    case 1:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde inicializar o módulo de imagens.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    case 2:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde inicializar o módulo do teclado.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    case 3:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde inicializar o módulo do mouse.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    case 4:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde inicializar o módulo do cronômetro.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    case 5:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde inicializar o módulo da fila de eventos.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    case 6:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde inicializar o módulo de desenho de primitivas.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    case 7:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde inicializar o módulo de áudio.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    case 8:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde inicializar o módulo dos codecs.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    case 9:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "O programa não pôde reservar amostras de áudio.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;

    default:
        al_show_native_message_box(
            scr,
            "Erro",
            "",
            "Erro indefinido.",
            NULL,
            ALLEGRO_MESSAGEBOX_ERROR
        );
        break;
    }

    return false;
}
