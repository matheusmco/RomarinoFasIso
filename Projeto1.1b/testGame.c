#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_BITMAP *player_bitmap=NULL, *target_bitmap=NULL, *background_bitmap = NULL, *button = NULL;
ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;
ALLEGRO_FONT *generic_font = NULL, *yes_no_font = NULL;
ALLEGRO_TIMER *timer = NULL;
FILE *pFile = NULL;

const int LARGURA_TELA = 640;
const int ALTURA_TELA = 480;

char player_name[20];
int score, tempo;

typedef struct{
    int x;
    int y;
    int height;
    int width;
}TARGET;

typedef struct{
    float x, y;
    int height;
    int width;
    float x_vel;
    float y_vel;
}PLAYER;

bool start();
bool load_files();
bool collide(PLAYER player, TARGET target);
void draw_objects(PLAYER player, TARGET target);
bool finish();


int main(){
    bool retry = true;
    while(retry){
        if(!start() || !load_files()) return -1;
        bool sair = false, type_done = false;
        TARGET target;
        PLAYER player;

        //define tamanho e largura do player e target
        target.height = (const) 23;
        target.width = (const) 30;
        player.height = (const) 60;
        player.width = (const) 50;
        srand(time(NULL));
        target.x = rand()%(LARGURA_TELA - target.width) + 1;
        target.y = rand()%(ALTURA_TELA - target.height) + 1;
        player.y = ALTURA_TELA / 2;
        player.x = LARGURA_TELA / 2;
        player.x_vel = 0.0;
        player.y_vel = 0.0;
        draw_objects(player, target);
        al_flip_display();
        while(!sair){
            if(!al_is_event_queue_empty(fila_eventos)){
                ALLEGRO_EVENT evento;
                al_wait_for_event(fila_eventos, &evento);
                if (evento.type == ALLEGRO_EVENT_KEY_DOWN){
                    switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        player.y_vel -= 0.5;
                        break;
                    case ALLEGRO_KEY_DOWN:
                        player.y_vel += 0.5;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        player.x_vel -= 0.5;
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        player.x_vel += 0.5;
                        break;
                    }
                }
                else if (evento.type == ALLEGRO_EVENT_KEY_UP){
                    switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_UP:
                        player.y_vel = 0;
                        break;
                    case ALLEGRO_KEY_DOWN:
                        player.y_vel = 0;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        player.x_vel = 0;
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        player.x_vel = 0;
                        break;
                    }
                }
                else if(evento.type == ALLEGRO_EVENT_TIMER) tempo--;
                else if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                    sair = true;
                    retry = false;
                }
            }

            if(collide(player, target)){
                target.x = rand()%(LARGURA_TELA - target.width) + 1;
                target.y = rand()%(ALTURA_TELA - target.height) + 1;
                al_draw_bitmap(target_bitmap, target.x, target.y,0);
                al_flip_display();
                score++;
            }

            if(tempo < 0){
                while(!type_done){
                    al_clear_to_color(al_map_rgb(0,0,0));
                    al_draw_textf(generic_font, al_map_rgb(255,255,255), LARGURA_TELA / 2 + 50, ALTURA_TELA / 2 - 100, ALLEGRO_ALIGN_CENTRE,
                                  "Acabou o tempo! Sua pontuacao: %d", score);
                    al_draw_textf(generic_font, al_map_rgb(255, 255, 255), LARGURA_TELA / 2 + 50, ALTURA_TELA / 2 - 50, ALLEGRO_ALIGN_CENTRE,
                                  "Entre com o seu nome: ");
                    if(!al_is_event_queue_empty(fila_eventos)){
                        ALLEGRO_EVENT evento;
                        al_wait_for_event(fila_eventos, &evento);
                        if(evento.type == ALLEGRO_EVENT_KEY_CHAR){
                            if(strlen(player_name) <= 19){
                                char temp[] = {evento.keyboard.unichar, '\0'};
                                if(evento.keyboard.unichar == ' ') strcat(player_name, temp);
                                else if(evento.keyboard.unichar >= '0' && evento.keyboard.unichar <= '9') strcat(player_name, temp);
                                else if(evento.keyboard.unichar >= 'A' && evento.keyboard.unichar <= 'Z') strcat(player_name, temp);
                                else if(evento.keyboard.unichar >= 'a' && evento.keyboard.unichar <= 'z') strcat(player_name, temp);
                            }
                            if(evento.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && strlen(player_name) != 0) player_name[strlen(player_name) - 1] = '\0';
                            if(strlen(player_name) > 0) al_draw_text(generic_font, al_map_rgb(255, 255, 255), LARGURA_TELA / 2,
                                                                     (ALTURA_TELA - al_get_font_ascent(generic_font)) / 2, ALLEGRO_ALIGN_CENTRE,
                                                                     player_name);
                            al_flip_display();
                        }
                        else if(evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ENTER){
                            fprintf(pFile, "%s: ", player_name);
                            fprintf(pFile, "%d\n", score);
                            type_done = true;
                        }
                    }
                }
                al_flip_display();
                sair = true;
            }
            else{
                if((player.x + player.x_vel > 0) && (player.x + player.x_vel < LARGURA_TELA - player.width)) player.x += player.x_vel;
                if((player.y + player.y_vel > 0) && (player.y + player.y_vel < ALTURA_TELA - player.height)) player.y += player.y_vel;
                al_clear_to_color(al_map_rgb(0,0,0));
                draw_objects(player, target);
                al_draw_textf(generic_font, al_map_rgb(255, 255, 255), 50, 50, ALLEGRO_ALIGN_LEFT, "%d", score);
                al_draw_textf(generic_font, al_map_rgb(255, 255, 255), 500, 50, ALLEGRO_ALIGN_RIGHT, "%d", tempo);
                al_flip_display();
            }
        }
        al_clear_to_color(al_map_rgb(255,255,255));
        al_draw_text(generic_font, al_map_rgb(0,0,0), LARGURA_TELA / 2, ALTURA_TELA / 2 - 100, ALLEGRO_ALIGN_CENTRE, "Deseja jogar novamente?");
        al_draw_bitmap(button, 200, 210, 0);
        al_draw_bitmap(button, 400, 210, 0);
        al_draw_text(yes_no_font, al_map_rgb(0,0,0), 225, 220, ALLEGRO_ALIGN_LEFT, "SIM");
        al_draw_text(yes_no_font, al_map_rgb(0,0,0), 425, 220, ALLEGRO_ALIGN_LEFT, "NAO");
        al_flip_display();
        int no_color = 0, yes_color = 0;
        int op = 0;
        bool choosed = false;
        while(!choosed){
            al_draw_text(yes_no_font, al_map_rgb(yes_color,0,0), 225, 220, ALLEGRO_ALIGN_LEFT, "SIM");
            al_draw_text(yes_no_font, al_map_rgb(no_color,0,0), 425, 220, ALLEGRO_ALIGN_LEFT, "NAO");
            al_flip_display();
            if(!al_is_event_queue_empty(fila_eventos)){
                ALLEGRO_EVENT evento;
                al_wait_for_event(fila_eventos, &evento);
                if(evento.type == ALLEGRO_EVENT_KEY_DOWN){
                    switch(evento.keyboard.keycode){
                    case ALLEGRO_KEY_RIGHT:
                        op = 1;
                        no_color = 255;
                        yes_color = 0;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        op = 2;
                        yes_color = 255;
                        no_color = 0;
                        break;
                    case ALLEGRO_KEY_ENTER:
                        choosed = true;
                        break;
                    }
                }
            }
        }
        if (op == 1) retry = false;
        else retry = true;
    }
    finish();
    return 0;
}

bool start(){
    score = 0, tempo = 30;
    if(!al_init()){
        printf("Erro ao iniciar o Allegro\n");
        return false;
    }

    janela = al_create_display(LARGURA_TELA, ALTURA_TELA);
    if(!janela){
        printf("Erro ao criar janela\n");
        al_destroy_display(janela);
        return false;
    }

    if(!al_init_image_addon()){
        printf("Erro ao iniciar o addon de imagem\n");
        al_destroy_display(janela);
        return false;
    }

    fila_eventos = al_create_event_queue();
    if(!fila_eventos){
        printf("Erro ao criar fila de eventos\n");
        al_destroy_display(janela);
        return false;
    }

    if(!al_install_keyboard()){
        printf("Erro ao iniciar teclado\n");
        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);
        return false;
    }

    al_init_font_addon();

    if(!al_init_ttf_addon()){
        printf("Falha ao iniciar addon allegro_ttf\n");
        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);
        return false;
    }

    timer = al_create_timer(1.0);
    if(!timer){
        printf("Erro ao criar timer\n");
        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);
        return false;
    }
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
    al_register_event_source(fila_eventos, al_get_display_event_source(janela));

    al_start_timer(timer);
    return true;
}

bool load_files(){
    player_bitmap = al_load_bitmap("player.jpg");
    target_bitmap = al_load_bitmap("ball.png");
    background_bitmap = al_load_bitmap("background.jpg");
    button = al_load_bitmap("button.jpg");
    if(!player_bitmap){
        printf("Erro ao carregar player.jpg\n");
        al_destroy_display(janela);
        al_destroy_timer(timer);
        return false;
    }

    if(!target_bitmap){
        printf("Erro ao carregar ball.png\n");
        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);
        al_destroy_timer(timer);
        return false;
    }

    if(!background_bitmap){
        printf("Erro ao carregar imagem de fundo\n");
        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);
        al_destroy_timer(timer);
    }

    if(!button){
        printf("Erro ao carregar botao\n");
        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);
        al_destroy_timer(timer);
    }

    generic_font = al_load_font("arial.ttf", 20, 0);
    if(!generic_font){
        printf("Falha ao carregar fonte arial.ttf\n");
        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);
        al_destroy_timer(timer);
        return false;
    }

    yes_no_font = al_load_font("arial.ttf", 30, 0);
    if(!yes_no_font){
        printf("Falha ao carregar fonte arial.ttf\n");
        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);
        al_destroy_font(generic_font);
        al_destroy_timer(timer);
        return false;
    }

    pFile = fopen("records.txt", "a+");
    if(!pFile){
        printf("Erro ao carregar arquivo\n");
        al_destroy_display(janela);
        al_destroy_event_queue(fila_eventos);
        al_destroy_font(generic_font);
        al_destroy_font(yes_no_font);
        al_destroy_timer(timer);
        return false;
    }
    return true;
}

bool collide(PLAYER p, TARGET t){
    if(((t.y >= p.y)&&(t.y <= p.y + p.height))&&((t.x >= p.x)&&(t.x <= p.x + p.width))) return true;
    if(((t.y + t.height >= p.y)&&(t.y <= p.y + p.height))&&((t.x + t.width >= p.x)&&(t.x <= p.x + p.width))) return true;
    return false;
}

void draw_objects(PLAYER player, TARGET target){
    al_clear_to_color(al_map_rgb(0,0,0));
    al_draw_bitmap(background_bitmap, 0, 0, 0);
    al_draw_bitmap(player_bitmap, player.x, player.y,0);
    al_draw_bitmap(target_bitmap, target.x, target.y,0);
}

bool finish(){
    al_destroy_display(janela);
    al_destroy_event_queue(fila_eventos);
    al_destroy_font(generic_font);
    al_destroy_timer(timer);
    al_destroy_font(yes_no_font);
    fclose(pFile);
    return true;
}
