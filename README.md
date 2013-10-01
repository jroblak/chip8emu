```
   ______ _______ _______ ______ ______
  |      |   |   |_     _|   __ |  __  |
  |   ---|       |_|   |_|    __|  __  |
  |______|___|___|_______|___|  |______|   
      
      e   m   u   l   a   t   o   r

   
      b y : j u s t i n  o b l a k
      
      

E M U L A T O R
---------------
usage: chip8 [path/to/file.ch8]


A S S E M B L E R
-----------------
usage: ch8asm [path/to/asm.ch8asm]
results: out.ch8 file in directory

asm parser rules:
- up to 30 characters per line
- comments allowed (parser ignores anything after commands)
- data sections indicated by .data:
- data section values separated by commas
- last data section value must have comma

EXAMPLE:

CLS                   // 200
LDI 206               // 202
JP 210                // 204
.data FF,90,90,90,F0, // 209
.data FF,90,90,90,F0, // 20f
LD 0, 3               // 210
LD 1, 3               // 212
DRW 0, 1, 5           // 214
LDKP 0                // 216


asm commands:
CLS - Clear screan
RET - Return from subroutine
SYS - OBSOLETE
JP nnn - Jump to address nnn
CALL nnn - Call subroutine at nnn
SE Vx, byte - Skip next instruction if Vx = byte
SNE Vx, byte - Skip next instruction if Vx != byte
SER Vy, Vy - Skip (Register) next instruction if Vx = Vy
LD Vx, byte - Load; set Vx = byte
ADD Vy, byte - Add Vy = Vx + byte
LDR Vx, Vy - Lost (Register) Vx = Vy
OR Vx, Vy - Vx = Vx | Vy
AND Vx, Vy - Vx = Vx & Vy
XOR Vx, Vy - Vx = Vx ^ Vy
ADDR Vx, Vy - ADD (register) Vx = Vx + Vy
SUB Vx, Vy - SUB Vx = Vx - Vy
SHR Vx, Vy - Vx = Vx >> 1
SUBR Vx, Vy - Vx = Vy - Vx
SHL Vx, Vy - Vx << 1
SNER Vx, Vy -  Skip not equal (register) if Vx != Vy
LDI nnn - Load (I) - sets I to address nnnn
JPO nnn 0 Jump (offset) - Jumps to address nnn + V0
RND Vx, byte - Vx = random byte & Vx mask
DRW Vx, Vy, nibble - Draws nibble-sized sprite pointed to by I at Vx, Vy
SKP Vx
SKNP Vx
LDRD Vx
LDKP Vx
LDDR Vx
LDST Vx
ADDI
LDC
LDBR
LDIR
LDRI

```
