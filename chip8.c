/*
   ______ _______ _______ ______ ______
  |      |   |   |_     _|   __ |  __  |
  |   ---|       |_|   |_|    __|  __  |
  |______|___|___|_______|___|  |______|

         e  m  u  l  a  t  o  r


	    J u s t i n   O b l a k
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <SDL/SDL.h>

#define prog_start 0x200
#define prog_end = 0xFFF
#define memory_size = 0xFFF

unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

/*
static int keymap[0x10] = {
    SDLK_0,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_9,
    SDLK_a,
    SDLK_b,
    SDLK_c,
    SDLK_d,
    SDLK_e,
    SDLK_f
};
*/

struct chip8_machine{
    unsigned char memory[memory_size];
    unsigned char V[16];
    unsigned short I;
    unsigned short pc;
    unsigned char gfx[64 * 32];
    unsigned char dt;
    unsigned char st;
    unsigned short stack[16];
    unsigned short sp;
    unsigned char key[16];
};

typedef struct chip8_machine chip8;

void chip8_init(chip8*);
void chip8_loadmem(chip8*, char*);
void chip8_load(chip8*);
void chip8_exec(chip8*);
void chip8_draw(chip8*);

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
	int sz = 0;
	int mem_start = 0x200;
	FILE *fp;

	// Load the Chip8 font sprites into memory
	for (i = 0; i < sizeof(chip8_fontset); i++) {
      c8->memory[i] = chip8_fontset[i];
    }

	// Load the program
	fp = fopen(prog_bin_path, "r");

	// Get the file size
	fseek(fp, 0L, SEEK_END);
	sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	if (sz > (prog_end - prog_start)) {
		printf("[-] Error - File too large.\n");
		exit(-1);
	}

	fread(&(c8->memory[prog_start]), sz, sz, fp);

	fclose(fp);
}

void chip8_exec(chip8 *c8) {
	// Combine next two instructions into opcode
	unsigned short opcode = (c8->memory[c8->pc] << 8) | c8->memory[c8->pc + 1];

	// Debug
	printf("Executing: 0x%X\n", opcode);
	printf("pc: 0x%X\n", c8->pc);

	// Get most significant byte of opcode
	switch (opcode & 0xF000) {
		case 0x0000:
			switch (opcode & 0x000F) {
				case 0x0000: // 00E0 - Clear the display.
					memset(c8->gfx, 0, sizeof(c8->gfx));
					c8->pc += 2;
					break;
				case 0x000E: // (RET) 00EE - Return
					c8->pc = c8->stack[c8->sp] + 2;
					c8->sp--;
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
			// Push current address to stack
			c8->pc = opcode & 0x0FFF;
			break;
		case 0x3000: // (JE int) 3xkk - Skip next instruction if Vx = kk.
			if (c8->V[(opcode & 0x0F000) >> 8] == opcode & 0x00FF) {
				c8->pc += 4;
			}
			else c8->pc += 2;
			break;
		case 0x4000: // (JNE int) 4xkk - Skip next instruction if Vx != kk.
			if (c8->V[(opcode & 0x0F000) >> 8] != opcode & 0x00FF) {
				c8->pc += 4;
			}
			else c8->pc += 2;
			break;
		case 0x5000: // (JE reg) 5xy0 - Skip next instruction if Vx = Vy.
			if (c8->V[(opcode & 0x0F00) >> 8] == c8->V[(opcode & 0x00F0) >> 8]) {
				c8->pc += 4;
			}
			else c8->pc += 2;
			break;
		case 0x6000: // (MOV int) 6xkk - Set Vx = kk.
			c8->V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			break;
		case 0x7000: // (ADD int) 7xkk - Set Vx = Vx + kk.
			c8->V[(opcode & 0x0F00) >> 8] = c8->V[(opcode & 0x0F00) >> 8] + opcode & 0x00FF;
			break;
		case 0x8000:
			switch (0x000F) {
				case 0x0001: // 8xy0 - Set Vx = Vy.
					break;
				case 0x0001: // 8xy1 - Set Vx = Vx OR Vy.
					break;
				case 0x0002: // 8xy2 - Set Vx = Vx AND Vy.
					break;
				case 0x0003: // 8xy3 - Set Vx = Vx XOR Vy.
					break;
				case 0x0004: // 8xy4 - Set Vx = Vx + Vy, set VF = carry.
					break;
				case 0x0005: // 8xy5 - Set Vx = Vx - Vy, set VF = NOT borrow.
					break;
				case 0x0006: // 8xy6 - Set Vx = Vx SHR 1.
					break;
				case 0x0007: // 8xy7 - Set Vx = Vy - Vx, set VF = NOT borrow.
					break;
				case 0x000E: // 8xyE - Set Vx = Vx SHL 1.
					break;
				default:
					printf ("Unknown opcode: 0x%X\n", opcode);
					break;
			}
			break;
		case 0x9000: // (JNE reg) 9xy0 - Skip next instruction if Vx != Vy.
			if (c8->V[(opcode & 0x0F00) >> 8] != c8->V[(opcode & 0x00F0) >> 8]) {
				c8->pc += 4;
			}
			else c8->pc += 2;
			break;
		case 0xA000: // Annn - Set I = nnn.
			c8->I = opcode & 0x0FFF;
			break;
		case 0xB000: // (JMP plus offset) Bnnn - Jump to location nnn + V0.
			// push address to stack
			c8->pc = 0x0FFF & c8->V[0];
			break;
		case 0xC000: // Cxkk - Set Vx = random byte AND kk.
			c8->V[0x0F00 >> 8] = rand() & (opcode & 0x00FF)
			break;
		case 0xD000: // Dxyn - Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
			break;
		case 0xE000:
			switch (opcode & 0x000F) {
				case 0x000E: // Ex9E - Skip next instruction if key with the value of Vx is pressed.
					break;
				case 0x0001: // ExA1 - Skip next instruction if key with the value of Vx is not pressed.
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
					break;
				case 0x000A: // Fx0A - Wait for a key press, store the value of the key in Vx.
					// TODO
					break;
				case 0x0015: // Fx15 - Set delay timer = Vx.
					c8->dt = c8->V[(opcode & 0x0F00) >> 8];
					break;
				case 0x0018: // Fx18 - Set sound timer = Vx.
					c8->st = c8->V[(opcode & 0x0F00) >> 8];
					break;
				case 0x001E: // Fx1E - Set I = I + Vx.
					c8->I = c8->I + c8->V[(opcode & 0x0F00) >> 8];
					break;
				case 0x0029: // Fx29 - Set I = location of sprite for digit Vx.
					// TODO
					break;
				case 0x0033: // Fx33 - Store BCD representation of Vx in memory locations I, I+1, and I+2.
					break;
				case 0x0055: // Fx55 - Store registers V0 through Vx in memory starting at location I.
					break;
				case 0x0065: // Fx65 - Read registers V0 through Vx from memory starting at location I.
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

	c8->pc += 2;
}

int main(int argc, char ** argv) {
	char *prog_bin_path;
	chip8 c8;
	srand (time(NULL));
	
	if (argc != 2) {
		printf("[-] Error - Usage is chip8 [game_path].\n");
		return -1;
	}
	prog_bin_path = argv[1];

    chip8_init(&c8);
	chip8_loadmem(&c8, prog_bin_path);

    for(;;) {
		chip8_exec(&c8);
	}

    return 0;
}
