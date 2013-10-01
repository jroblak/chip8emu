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


```
