/*
 
     ______ _______ _______ ______ ______
    |      |   |   |_     _|   __ |  __  |
    |   ---|       |_|   |_|    __|  __  |
    |______|___|___|_______|___|  |______|
 
            e  m  u  l  a  t  o  r
 
 
           J u s t i n   O b l a k
 
 
 TODO LIST
 ------------------
 - Sound
 - Timers
 - Test Opcodes
 - Test ROMs
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SDL.h"
#include "SDL_mixer.h"

#define USEMEMSTART 0x200
#define USEMEMEND 0xFFF
#define MEMSIZE 0xFFF
#define WIDTH 64
#define HEIGHT 32
#define BPP 32

unsigned char chip8_font[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

const Uint8 key_mapping[16] = {
    SDL_SCANCODE_2, // 0 - 2
    SDL_SCANCODE_Q, // 1 - Q
    SDL_SCANCODE_W, // 2 - W
    SDL_SCANCODE_E, // 3 - E
    SDL_SCANCODE_A, // 4 - A
    SDL_SCANCODE_S, // 5 - S
    SDL_SCANCODE_D, // 6 - D
    SDL_SCANCODE_Z, // 7 - Z
    SDL_SCANCODE_X, // 8 - X
    SDL_SCANCODE_C, // 9 - C
    SDL_SCANCODE_1, // A - 1
    SDL_SCANCODE_3, // B - 3
    SDL_SCANCODE_R, // C - R
    SDL_SCANCODE_V, // E - V
    SDL_SCANCODE_F, // D - F
    SDL_SCANCODE_4  // F - 4
};

struct chip8 {
    unsigned char memory[MEMSIZE];
    unsigned char V[16];
    unsigned short I;
    unsigned short pc;
    unsigned char gfx[WIDTH * HEIGHT];
    unsigned char dt;
    unsigned char st;
    unsigned short stack[16];
    unsigned short sp;
};

typedef struct chip8 chip8;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Surface *surface = NULL;
SDL_Texture *texture = NULL;
Uint32 sdl_gfx[WIDTH * HEIGHT];

void chip8_init(chip8*);
void chip8_loadmem(chip8*, char*);
void chip8_exec(chip8*);
void chip8_draw(chip8*);
void chip8_timers(chip8*);
void chip8_sound(chip8*);

void chip8_init(chip8 *c8) {
    c8->pc = 0x200;
    c8->I = 0;
    c8->sp = 0;
    
    memset(c8->memory, 0, sizeof(c8->memory));
    memset(c8->gfx, 0, sizeof(c8->gfx));
    memset(c8->stack, 0, sizeof(c8->stack));
    memset(c8->V, 0, sizeof(c8->V));
    
    return;
}

void chip8_loadmem(chip8 *c8, char* prog_bin_path) {
    int i;
    long sz = 0;
    FILE *fp;
    
    for (i = 0; i < sizeof(chip8_font); i++) {
        c8->memory[i] = chip8_font[i];
    }
    
    fp = fopen(prog_bin_path, "r");
    
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    
    if (sz > (USEMEMEND - USEMEMSTART)) {
        printf("[-] Error - File too large.\n");
        exit(-1);
    }
    
    fread(c8->memory + USEMEMSTART, sz, sz, fp);
    
    fclose(fp);
}

void chip8_exec(chip8 *c8) {
    unsigned short opcode = (c8->memory[c8->pc] << 8) | c8->memory[c8->pc + 1];
    const Uint8 *state;
    int i;
    
    printf("Executing: 0x%X\n", opcode);
    printf("pc: 0x%X\n", c8->pc);
    printf("I: 0x%X\n", c8->I);
    
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000: // 00E0 - Clear the display.
                    memset(c8->gfx, 0, sizeof(c8->gfx));
                    c8->pc += 2;
                    break;
                case 0x000E: // (RET) 00EE - Return
                    c8->sp--;
                    c8->pc = c8->stack[c8->sp] + 2;
                    c8->stack[c8->sp] &= 0x0000;
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
                    break;
            }
            break;
        case 0x1000: // (JMP) 1nnn - Jump to location nnn.
            c8->pc = opcode & 0x0FFF;
            break;
        case 0x2000: // (CALL) 2nnn - Call subroutine at nnn.
            c8->stack[c8->sp] = c8->pc;
            c8->sp++;
            c8->pc = opcode & 0x0FFF;
            break;
        case 0x3000: // (JE int) 3xkk - Skip next instruction if Vx = kk.
            if (c8->V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                c8->pc += 4;
            }
            else c8->pc += 2;
            break;
        case 0x4000: // (JNE int) 4xkk - Skip next instruction if Vx != kk.
            if (c8->V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                c8->pc += 4;
            }
            else c8->pc += 2;
            break;
        case 0x5000: // (JE reg) 5xy0 - Skip next instruction if Vx = Vy.
            if (c8->V[(opcode & 0x0F00) >> 8] == c8->V[(opcode & 0x00F0) >> 4]) {
                c8->pc += 4;
            }
            else c8->pc += 2;
            break;
        case 0x6000: // (MOV int) 6xkk - Set Vx = kk.
            c8->V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            c8->pc += 2;
            break;
        case 0x7000: // (ADD int) 7xkk - Set Vx = Vx + kk.
            c8->V[(opcode & 0x0F00) >> 8] = c8->V[(opcode & 0x0F00) >> 8] + opcode & 0x00FF;
            c8->pc += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000: // 8xy0 - Set Vx = Vy.
                    c8->V[(opcode & 0x0F00) >> 8] = c8->V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0001: // 8xy1 - Set Vx = Vx OR Vy.
                    c8->V[(opcode & 0x0F00) >> 8] = c8->V[(opcode & 0x0F00) >> 8] | c8->V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0002: // 8xy2 - Set Vx = Vx AND Vy.
                    c8->V[(opcode & 0x0F00) >> 8] = c8->V[(opcode & 0x0F00) >> 8] & c8->V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0003: // 8xy3 - Set Vx = Vx XOR Vy.
                    c8->V[(opcode & 0x0F00) >> 8] = c8->V[(opcode & 0x0F00) >> 8] ^ c8->V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0004: // 8xy4 - Set Vx = Vx + Vy, set VF = carry.
                    if (c8->V[(opcode & 0x0F00) >> 8] + c8->V[(opcode & 0x00F0) >> 4] > 0xFF) c8->V[0xF] = 0x1;
                    else c8->V[0xF] = 0x0;
                    c8->V[(opcode & 0x0F00) >> 8] += c8->V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0005: // 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
                    if (c8->V[(opcode & 0x0F00) >> 8] > c8->V[(opcode & 0x00F0) >> 4]) c8->V[0xF] = 0x1;
                    else c8->V[0xF] = 0x0;
                    c8->V[(opcode & 0x0F00) >> 8] -= c8->V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0006: // 8xy6 - Set Vx = Vx SHR 1.
                    c8->V[(opcode & 0x0F00) >> 8] = c8->V[(opcode & 0x0F00) >> 8] >> 1;
                    break;
                case 0x0007: // 8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
                    if (c8->V[(opcode & 0x0F00) >> 8] < c8->V[(opcode & 0x00F0) >> 4]) c8->V[0xF] = 0x1;
                    else c8->V[0xF] = 0x0;
                    c8->V[(opcode & 0x0F00) >> 8] = c8->V[(opcode & 0x00F0) >> 4] - c8->V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x000E: // 8xyE - Set Vx = Vx SHL 1.
                    c8->V[(opcode & 0x0F00) >> 8] = c8->V[(opcode & 0x0F00) >> 8] << 1;
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
                    break;
            }
            c8->pc += 2;
            break;
        case 0x9000: // (JNE reg) 9xy0 - Skip next instruction if Vx != Vy.
            if (c8->V[(opcode & 0x0F00) >> 8] != c8->V[(opcode & 0x00F0) >> 4]) {
                c8->pc += 4;
            }
            else c8->pc += 2;
            break;
        case 0xA000: // Annn - Set I = nnn.
            c8->I = opcode & 0x0FFF;
            c8->pc += 2;
            break;
        case 0xB000: // (JMP plus offset) Bnnn - Jump to location nnn + V0.
            // push address to stack
            c8->pc = 0x0FFF & c8->V[0];
            break;
        case 0xC000: // Cxkk - Set Vx = random byte AND kk.
            c8->V[0x0F00 >> 8] = rand() & (opcode & 0x00FF);
            c8->pc += 2;
            break;
        case 0xD000: { // Dxyn - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
            // If the sprite is positioned so part of it is outside the coordinates of the display
            // it wraps around to the opposite side of the screen.
            int x = (opcode & 0x0F00) >> 8;
            int y = (opcode & 0x00F0) >> 4;
            for (i = 0; i < (opcode & 0x000F); i++) {
                for (int col = 0; col < 8; col++) {
                    unsigned char mem = c8->memory[c8->I] >> (7 - col) & 1;
                    c8->gfx[(c8->V[x] + col) + (c8->V[y] + i) * WIDTH] ^= mem;
                    if (c8->gfx[(c8->V[x] + col) + (c8->V[y] + i) * WIDTH] == 0x0) c8->V[0xF] = 0x1;
                }
                c8->I++;
            }
            c8->pc += 2;
            break;
        }
        case 0xE000:
            switch (opcode & 0x000F) {
                case 0x000E: // Ex9E - Skip next instruction if key with the value of Vx is pressed.
                    state = SDL_GetKeyboardState(NULL);
                    if (state[key_mapping[c8->V[(opcode & 0x0F00) >> 8]]] == 1) c8->pc += 4;
                    else c8->pc += 2;
                    break;
                case 0x0001: // ExA1 - Skip next instruction if key with the value of Vx is not pressed.
                    state = SDL_GetKeyboardState(NULL);
                    if (state[key_mapping[c8->V[(opcode & 0x0F00) >> 8]]] == 0) c8->pc += 4;
                    else c8->pc += 2;
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
                    break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007: // Fx07 - Set Vx = delay timer value.
                    c8->V[(0x0F00 * opcode) >> 8] = c8->dt;
                    c8->pc += 2;
                    break;
                case 0x000A: {// Fx0A - Wait for a key press, store the value of the key in Vx.
                    state = SDL_GetKeyboardState(NULL);
                    int keypress = 0;
                    for (i = 0; i < sizeof(key_mapping); i++) {
                        if (state[key_mapping[i]] == 1) {
                            c8->V[(0x0F00 & opcode) >> 8] = i;
                            keypress = 1;
                            break;
                        }
                    }
                    if (keypress == 1) c8->pc += 2;
                    break;
                }
                case 0x0015: // Fx15 - Set delay timer = Vx.
                    c8->dt = c8->V[(opcode & 0x0F00) >> 8];
                    c8->pc += 2;
                    break;
                case 0x0018: // Fx18 - Set sound timer = Vx.
                    c8->st = c8->V[(opcode & 0x0F00) >> 8];
                    c8->pc += 2;
                    break;
                case 0x001E: // Fx1E - Set I = I + Vx.
                    c8->I = c8->I + c8->V[(opcode & 0x0F00) >> 8];
                    c8->pc += 2;
                    break;
                case 0x0029: // Fx29 - Set I = location of sprite for digit Vx.
                    c8->I = c8->V[(opcode & 0x0F00) >> 8] * 5;
                    c8->pc += 2;
                    break;
                case 0x0033: // Fx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2.
                    c8->memory[c8->I] = c8->V[(opcode & 0x0F00) >> 8] / 100;
                    c8->memory[c8->I + 1] = (c8->V[(opcode & 0x0F00) >> 8] % 100) / 10;
                    c8->memory[c8->I + 2] = c8->V[(opcode & 0x0F00) >> 8] % 10;
                    c8->pc += 2;
                    break;
                case 0x0055: // Fx55 - Store registers V0 through Vx in memory starting at location I.
                    for (i = 0; i < ((opcode & 0x0F00) >> 8); i++) {
                        c8->memory[c8->I] = c8->V[i];
                        c8->I++;
                    }
                    c8->pc += 2;
                    break;
                case 0x0065: // Fx65 - Read registers V0 through Vx from memory starting at location I.
                    for (i = 0; i < ((opcode & 0x0F00) >> 8); i++) {
                        c8->V[i] = c8->memory[c8->I];
                        c8->I++;
                    }
                    c8->pc += 2;
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode);
                    break;
            }
            break;
        default:
            printf ("Unknown opcode: 0x%X\n", opcode);
            break;
    }
}

void chip8_draw(chip8* c8) {
    int i = 0;
    
    for (i = 0; i < sizeof(c8->gfx); i++) {
        if (c8->gfx[i] == 0) sdl_gfx[i] = 0x00000000;
        else sdl_gfx[i] = 0xFFFFFFFF;
    }
    
    
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, sdl_gfx, 64 * sizeof(Uint32));
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void sdl_init() {
    if(SDL_Init(SDL_INIT_EVERYTHING) == -1) {
        exit(-1);
    }
    
    memset(sdl_gfx, 0, sizeof(sdl_gfx));
    
    window = SDL_CreateWindow("Chip8 OSX", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (texture == NULL || window == NULL || texture == NULL){
        printf("Error - %s\n", SDL_GetError());
        exit(-1);
    }
}

void chip8_timers(chip8 *c8) {
    if (c8->dt > 0) {
        c8->dt--;
    }
    if (c8->st > 0) {
        c8->st--;
    }
}

void chip8_sound(chip8 *c8) {
    
}

int main(int argc, char* args[]) {
    int quit = 0;
    char *prog_bin_path;
    chip8 c8;
    srand((unsigned int)time(NULL));
    
    if (argc < 2) {
        printf("[-] Error - Usage is chip8 [game_path].\n");
        return -1;
    }
    prog_bin_path = args[1];
    
    chip8_init(&c8);
    sdl_init();
    chip8_loadmem(&c8, prog_bin_path);
    
    SDL_Event e;
    while(1) {
        SDL_PollEvent(&e);
        
        if (e.type == SDL_QUIT) quit = 1;
        
        chip8_exec(&c8);
        if (c8.dt > 0 || c8.st > 0) chip8_timers(&c8);
        if (c8.st > 0) chip8_sound(&c8);
        chip8_draw(&c8);
    }
    
    SDL_Delay(2000);
    SDL_Quit();
    
    return 0;
}