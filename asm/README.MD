# Playing around with assembly on the A-D 8800
Here are some programs written in assembly.

## leds
This is to control the LED register output card for the A-D 8800 experimenter.  For info on that particular card, please see [LED output card documentation](https://github.com/dhansel/Altair8800-IOBus/tree/main/01-led-output-register).  Basically, it is an output-only bus card with 8 LEDs sitting at bus address 255.  Using BASIC, one can issue a line like `out 255, 1` to turn on LED 1, `out 255, 255` to turn them all on.

It's been a while since I've written any assembly of consequence, so this seemed like a pretty good thing to do as practice.

### Assembling
This is written to run in `CP/M 2.2`. A disk image with CP/M, the assembler, debugger, editor, and everything else that's needed is provided on the SD card for the A-D.  Note: [SD card image can be found here](https://adwaterandstir.com/instructions/)

Follow the A-D instructions on mounting the CP/M disk and booting CP/M itself.  From there, one can either type in the whole program by hand or transfer by `XMODEM` or other method to the disk.

There should be just one file starting with `leds` on the disk:
```  
B>dir leds.*
B: LEDS     ASM
```  

Once there, assemble it using `asm`:
```  
B>asm leds
CP/M ASSEMBLER - VER 2.0
03AA
004H USE FACTOR
END OF ASSEMBLY
```   
The assembler finished with no errors - yay!!  There are two new files on the disk: `leds.prn` and `leds.hex`:
```  
B>dir leds.*
B: LEDS     ASM : LEDS     PRN : LEDS     HEX
```  
`leds.prn` is the "printer" output from the assembler.  It is the output of the assembler listed with the original source - this is like a listing file from a c compiler.  For example, looking at lines 200 to 224 of the prn file with the `ed` editor looks like this:
```  
B>ed leds.prn
*#a200:25t
                shftloop:
 0197 CAA801            jz      setout          ; done sifting? then set the output
 019A 78                mov     a,b             ; get shifted bit value in A
 019B 07                rlc                     ; shift left one
 019C 47                mov     b,a             ; save it back to B
 019D 3A6703            lda     choice          ; get count from memory
 01A0 D601              sui     1               ; decrement count (sets Z flag)
 01A2 326703            sta     choice          ; save it back to memory
 01A5 C39701            jmp     shftloop        ; go shift again
                setout:
 01A8 3A6603            lda     value           ; get current LED output value to A
 01AB A8                xra     b               ; XOR shifted value with current
 01AC D3FF              out     LEDS            ; write it out
 01AE 326603            sta     value           ; save new current output to memory
 01B1 C35701            jmp     mainloop        ; here we go again...
                
                badentry:
 01B4 113F03            lxi     d,badntry       ; point to bad entry message
 01B7 0E09              mvi     c,strout        ; print string function code
 01B9 CD0500            call    sys             ; print it to console
 01BC C35701            jmp     mainloop        ; back to try again
                
                ;*************************************************
                ; Final clean up before exit
                ; restore original stack pointer, etc.
*q

Q-(Y/N)?y

```  
`leds.hex` is an Intel hex file of the memory image of the assembled program.  Same kind of thing that a microcontroller cross compiler will generate for loading into Flash memory:
```  
B>type leds.hex
:100100002100003922680331AA030E09112502CD0E
:1001100005000180000AFE00CA50010303115E03BE
:10012000CD1002C235011140020E09CD05003EFF7F
:10013000D3FFC3BF01018200116203CD1002C25080
:10014000011157020E09CD05003E00D3FFC3BF01C8
:100150003E00D3FFCDD1011120030E09CD05003A99
:100160006603CDF001CDD101116F020E09CD05005E
:100170000E01CD0500326703CDD101FE78CABF0163
:10018000FE30FAB401FE38F2B401D6303267033ED5
:1001900001473A6703FE00CAA8017807473A670398
:1001A000D601326703C397013A6603A8D3FF3266CC
:1001B00003C35701113F030E09CD0500C357012AA0
:1001C0006803F9C9E5D5C50E025FCD0500C1D1E1CF
:1001D000C9F53E0DCDC4013E0ACDC401F1C9E60FFB
:1001E000FE0AD2EA01C630C3EC01C657CDC401C92C
:1001F000F50F0F0F0FCDDE01F1F5CDDE01F1C9D501
:100200000100001AFE00CA0E020313C30302D1C983
:10021000E5C5D50A6F1ABDC221020313FE00C21341
:1002200002D1C1E1C9412D442038383030204C453D
:1002300044206361726420636F6E74726F6C0A2471
:100240005475726E696E6720616C6C2038204C4565
:100250004473206F6E0A245475726E696E67206154
:100260006C6C2038204C454473206F66660A24204D
:1002700020302920546F67676C65204C454420303E
:100280000A2020312920546F67676C65204C454453
:1002900020310A2020322920546F67676C65204C7A
:1002A000454420320A2020332920546F67676C654B
:1002B000204C454420330A2020342920546F67679E
:1002C0006C65204C454420340A2020352920546F89
:1002D00067676C65204C454420350A20203629206C
:1002E000546F67676C65204C454420360A202037E0
:1002F0002920546F67676C65204C454420370A20DD
:1003000020782920457869742070726F6772616D5A
:100310000A456E7465722063686F6963653E2024C8
:1003200043757272656E7420706F7274206F75748D
:100330002076616C75652028686578293A2024420A
:10034000616420696E7075742076616C75652C200F
:1003500075736520302D37206F7220780A2453453D
:080360005400434C5200000060
:0000000000
```
### Creating a loadable image
The `leds.hex` could be run in the `ddt` debugger, but let's just create the executable image so it can be run fro the command line:
```  
B>load leds

FIRST ADDRESS 0100
LAST  ADDRESS 0367
BYTES READ    0268
RECORDS WRITTEN 05
```  
Now there is another file in the directory:
```   
B>dir leds.*
B: LEDS     ASM : LEDS     PRN : LEDS     HEX : LEDS     COM
```  
`leds.com` is the executable image of the program!
### Running the program
We now have an executable that can be called from the command line like any other program.  This one accepts zero or one command-line argument, so it can be called in one of these ways.

Adding a 'clr' to the command will turn off all 8 LEDs:
```
B>leds clr
A-D 8800 LED card control
Turning all 8 LEDs off
```  
Adding a 'set' to the command will turn them all on:
```  
B>leds set
A-D 8800 LED card control
Turning all 8 LEDs on
```  
Starting the program with no parameters will give an interactive mode:
``` 
B>leds
A-D 8800 LED card control

Current port out value (hex): 00
  0) Toggle LED 0
  1) Toggle LED 1
  2) Toggle LED 2
  3) Toggle LED 3
  4) Toggle LED 4
  5) Toggle LED 5
  6) Toggle LED 6
  7) Toggle LED 7
  x) Exit program
Enter choice> 
```  
Pressing `0` through `7` will toggle the state of that LED on the card (leaving the rest as they are).  pressing `x` will return to CP/M.  The current value of the output register is shown, and will change as a key is pressed.  

Here, the `2` is pressed, which turned on the 3rd LED:
```  
Enter choice> 2
Current port out value (hex): 04
  0) Toggle LED 0
  1) Toggle LED 1
  2) Toggle LED 2
  3) Toggle LED 3
  4) Toggle LED 4
  5) Toggle LED 5
  6) Toggle LED 6
  7) Toggle LED 7
  x) Exit program
Enter choice> 
```
