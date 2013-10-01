#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define MAXLENGTH 3580
#define MAXINSTRUCLEN 4
#define INSTRUCTIONS 35

const char * asm_lookup[INSTRUCTIONS] = {
	"CLS",	
	"RET",
	"SYS",
	"JP",
	"CALL",
	"SE",
	"SNE",
	"SER",
	"LD",
	"ADD",
	"LDR",
	"OR",
	"AND",
	"XOR",
	"ADDR",
	"SUB",
	"SHR",
	"SUBR",
	"SHL",
	"SNER",
	"LDI",
	"JPO",
	"RND",
	"DRW",
	"SKP",
	"SKNP",
	"LDRD",
	"LDKP",
	"LDDR",
	"LDST",
	"ADDI",
	"LDC",
	"LDBR",
	"LDIR",
	"LDRI"
};

unsigned short parse_line(char *line);
char* process_xy(char *line);
char* process_xyk(char *line);
char* process_xyn(char *line);
char* process_xkk(char *line);
FILE* open_file(char *file);

FILE* open_file(char *file) {
    size_t size = 0;
    FILE *fp;

	if ((fp = fopen(file, "rb")) == NULL) {
		perror("File not found.\n");
		exit(-1);
	}

    fseek(fp, 0L, SEEK_END);
    size = (size_t)ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    return fp;
}

unsigned short parse_line(char *line) {
	int i = 0, asm_size = 0, asm_value = -1;
	char *instruc;
	unsigned short opcode;

	for (i = 0; i <= sizeof(line); i++) {
		if (line[i] == 0x20 || line[i] == 0xA || line[i] == 0xD) {
			asm_size = i;
			break;
		}
	}

	instruc = (char *)malloc(asm_size * sizeof(char));
	strncpy(instruc, line, asm_size);

	for (i = 0; i < INSTRUCTIONS; i++) {
		if (strncmp(asm_lookup[i], instruc, asm_size) == 0) {
			asm_value = i;
			break;
		}
	}

	if(asm_value < 0) {
		exit(-1);
	}

	switch (asm_value) {
		case 0:			   // 00E0 - CLS
			opcode = 0x00E0;
			break;
		case 1:			   // 00EE - RET
			opcode = 0x00EE;
			break;
		case 2:			   // 0nnn - SYS addr - DEPRECATED
			opcode = 0x0000;
			break;
		case 3: 		   // 1nnn - JP addr;
			opcode = 0x1000 | strtol(line + asm_size + 1, NULL, 16);
			break;
		case 4:			   // 2nnn - CALL addr
			opcode = 0x2000 | strtol(line + asm_size + 1, NULL, 16);
			break;
		case 5:			   // 3xkk - SE Vx, byte
			opcode = 0x3000 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16);
			break;
		case 6:			   // 4xkk - SNE Vx, byte
			opcode = 0x4000 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16);
			break;
		case 7:			   // 5xy0 - SER Vx, Vy
			opcode = 0x5000 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4;
			break;
		case 8:			   // 6xkk - LD Vx, byte
			opcode = 0x6000 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16);
			break;
		case 9:			   // 7xkk - ADD Vx, byte
			opcode = 0x7000 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16);
			break;
		case 10:		   // 8xy0 - LDR Vx, Vy
			opcode = 0x8000 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4;
			break;
		case 11:		   // 8xy1 - OR Vx, Vy
			opcode = 0x8001 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4;
			break;
		case 12:		   // 8xy2 - AND Vx, Vy
			opcode = 0x8002 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4;
			break;
		case 13:		   // 8xy3 - XOR Vx, Vy
			opcode = 0x8003 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4;
			break;
		case 14:		   // 8xy4 - ADDR Vx, Vy
			opcode = 0x8004 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4;
			break;
		case 15:		   // 8xy5 - SUB Vx, Vy
			opcode = 0x8005 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4;
			break;
		case 16:		   // 8xy6 - SHR Vx
			opcode = 0x8006 | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 17:		   // 8xy7 - SUBN Vx, Vy
			opcode = 0x8007 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4;
			break;
		case 18:		   // 8xyE - SHL Vx
			opcode = 0x800E | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 19:		   // 9xy0 - SNER Vx, Vy
			opcode = 0x9000 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4;
			break;
		case 20:		   // Annn - LDI I, addr
			opcode = 0xA000 | strtol(line + asm_size + 1, NULL, 16);
			break;
		case 21:		   // Bnnn - JPO V0, addr
			opcode = 0xB000 | strtol(line + asm_size + 1, NULL, 16);
			break;
		case 22:		   // Cxkk - RND Vx, byte
			opcode = 0xC000 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16);
			break;
		case 23:		   // Dxyn - DRW Vx, Vy, nibble
			opcode = 0xD000 | strtol(line + asm_size + 1, NULL, 16) << 8 | strtol(line + asm_size + 3, NULL, 16) << 4 |  strtol(line + asm_size + 7, NULL, 16);
			break;
		case 24:		   // Ex9E - SKP Vx
			opcode = 0xE09E | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 25:		   // ExA1 - SKNP Vx
			opcode = 0xE0A1 | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 26:		   // Fx07 - LDRD Vx, (Vx = DT)
			opcode = 0xF007 | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 27:		   // Fx0A - LDKP Vx (Keypress)
			opcode = 0xF00A | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 28:		   // Fx15 - LDDR Vx (DT = Vx)
			opcode = 0xF015 | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 29:		   // Fx18 - LDST Vx, byte (Set ST = Vx)
			opcode = 0xF018 | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 30:		   // Fx1E - ADDI Vx (I = I + Vx)
			opcode = 0xF01E | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 31:		   // Fx29 - LDC Vx (I = Location of char Vx)
			opcode = 0xF029 | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 32:		   // Fx33 - LDBR Vx (Load BCD of Vx to memory)
			opcode = 0xF033 | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 33:		   // Fx55 - LDIR Vx (Store registers through Vx to I)
			opcode = 0xF055 | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		case 34:		   // Fx65 - LDRI Vx (Read registers through Vx from I)
			opcode = 0xF065 | strtol(line + asm_size + 1, NULL, 16) << 8;
			break;
		default:
			exit(-1);
	}

	free(instruc);
    return opcode;
}

int main(int argc, char ** argv) {
    int count = 0;
    char buf[30];
	unsigned short opcode = 0x0000;
	unsigned char op;
    unsigned char opcodes[MAXLENGTH];
	char *output = "out.ch8";
	FILE* fp = NULL;
	FILE* ofp = NULL;

    memset(buf, 0, sizeof(buf));
    memset(opcodes, 0, sizeof(opcodes));

    if (argc < 2) {
        printf("[-] Error - Usage is ch8asm [chip8 assembly file].\n");
        return -1;
    }

    fp = fopen(argv[1], "r");

    while (fgets(buf, 30, fp) != NULL) {
		if (buf[0] == '.') {
			char *c = strchr(buf, ',');
			while(c != NULL) {
				op = strtol(c - 2, NULL, 16);
				*(opcodes + count) = op;
				count += 1;
				c = strchr(c + 1, ',');
			}
		} else {
			opcode = parse_line(buf);
			op = (opcode & 0xFF00) >> 8;
			*(opcodes + count) = op;
			op = opcode & 0x00FF;
			*(opcodes + count + 1) = op;
			count += 2;
		}
    }

	ofp = fopen(output, "wb");
	fwrite(opcodes, 1, count, ofp);

	fclose(ofp);
    fclose(fp);

    return 0;
}
