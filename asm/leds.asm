;****************************************************
; leds.asm
; Simple assembler program to control the LEDs on the
; LED expansion card for the Altair-Duino 8800
; experimenter.  This is written for use with CP/M
;
; Usage:
; assemble with 'asm':  asm leds
; load: load leds
; From the command line:
;  leds <set> <clr>
;     set - set all LEDs on and return to cmdline
;     clr - turn all LEDs off and return to cmdline
;  If no argument given, a menu will be shown allowing
;  each LED to be individually toggled by keyboard.
;
;****************************************************
; subroutines PCHAR, PNIB, PHEX, and CRLF are taken
; from the CP/M manual and modified a bit.
;****************************************************
;
; A-D 8800 Experimenter:
; https://adwaterandstir.com/altair/
;
; LED output register card:
; https://github.com/dhansel/Altair8800-IOBus/tree/main/01-led-output-register
;
;****************************************************
;
; Copyright 2026 M.Brugman
;
; Permission is hereby granted, free of charge, to any person
; obtaining a copy of this software and associated
; documentation files (the “Software”), to deal in the
; Software without restriction, including without limitation
; the rights to use, copy, modify, merge, publish, distribute,
; sublicense, and/or sell copies of the Software, and to permit
; persons to whom the Software is furnished to do so,
; subject to the following conditions:
;
; The above copyright notice and this permission notice shall
; be included in all copies or substantial portions
; of the Software.
;
; THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY
; KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
; WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
; PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
; OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
; OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
; OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
; SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;****************************************************

; Most of the following is just boiler plate, common
; definitions, etc.

;****************************************************
; Entry point for FDOS, BIOS, CP/M system calls
;****************************************************
sys     equ	5		; entry point for system (dos)

;***************************************************
; syscall functions
; Generally, drop this into C and do a call sys to
; do a syscall.  Some funcs require more stuffs.
;***************************************************
reset	equ	0		; reset (reload CP/M)
conin	equ	1		; console input (blocking)
conout	equ	2		; console output function
rdinp	equ	3		; paper tape reader input
pnchout	equ	4		; paper tape puncher
lstout	equ	5		; list output (printer)
dirio	equ	6		; direct console I/O
getio	equ	7		; get IOBYTE
setio	equ	8		; set IOBYTE
strout	equ	9		; string output function
rdcbuf	equ	10		; read console buffer
constat	equ	11		; get console status
version	equ	12		; get CP/M version
rstdsk	equ	13		; reset disk system
seldsk	equ	14		; select disk
fopen   equ	15  		; open file
fclose  equ 	16  		; close file
srchf   equ 	17  		; search first file
srchn   equ 	18  		; search next file
fdel    equ 	19  		; delete file
rdseq   equ 	20  		; read sequential
wrseq   equ 	21  		; write sequential
fmake   equ 	22  		; make file
fname   equ 	23  		; rename file
lvect   equ 	24  		; return login vector
getdsk	equ	25		; return current disk
setdma	equ	26		; set DMA address
alloc	equ	27		; get address (alloc)
pdsk	equ	28		; protect disk
getrov	equ	29		; get R/O vector
setatt	equ	30		; set file attributes
gdprarm	equ	31		; get address (disk params)
gsuer	equ	32		; get/set user code
rdrnd	equ	33		; read random
wrrnd	equ	34		; write random
fsize	equ	35		; compute file size
frset	equ	36		; set random record
rstdrv	equ	37		; reset drive
wrtzer	equ	40		; write random with zero fill

;****************************************************
; CP/M constants/values/addresses
;****************************************************
fcb1	equ	005ch		; first cmdline fcb
dma1	equ	0080h		; first DMA area 128 bytes

;*****************************************************
;* handy constants
;*****************************************************
CR	equ	0dh		; carriage return
LF	equ	0ah		; line feed
LEDS	equ 	255		; I/O port for LED card

;****************************************************
; Start of actual program code here
; .org 100h - TPA area starts at 0x0100
;****************************************************
	org	0100h		; entry point at 100 hex (TPA area)
	lxi	h,0		; zero H/L register pair
	dad	sp		; get stack pointer from CCP to H/L
	shld	orgstk		; save original stack pointer to memory
	lxi	sp,newstk	; set stack pointer to our new stack

	mvi	c,strout	; load reg C with command to print string
	lxi	d,welcome	; point to address of welcome string
	call	sys		; call the system function to print string

;*****************************************************************
; anything on the command line?
;*****************************************************************
	lxi	b,dma1		; set B/C reg pair with addr of cmdline
	ldax	b		; load Register A with value at that addr
	cpi 	00h 		; is that value zero?
	jz	indone		; yes, skip over cmdline evaluation
	inx	b		; increment past char count
	inx	b		; increment past space
	lxi	d,argset	; load D/E with addr of 'SET\0'
	call	strcmp		; compare cmdline to 'SET\0'
	jnz	clrleds		; cmdline does not match 'SET\0'
	lxi	d,setmsg	; ope, it DID match - load message addr
	mvi	c,strout	; print string function code
	call	sys 		; print string to console out
	mvi	a,255		; turn all LEDs on
	out	LEDS		; write to I/O port
	jmp	cleanup		; cleanup and exit
clrleds:
	lxi	b,dma1+2	; reset B/C to cmdline + 2 bytes
	lxi	d,argclr	; point to 'CLR\0' string
	call	strcmp		; compare
	jnz	indone		; cmdline not equal, goto mainline
	lxi	d,clrmsg	; message about turning off
	mvi	c,strout	; print string function code
	call	sys		; print string to console out
	mvi	a,0		; turn all LEDs off
	out	LEDS		; write to I/O port
	jmp	cleanup		; all done

;*****************************************************************
; Nothing valid on cmdline, show menu and start taking user input
;*****************************************************************
indone:
	mvi	a,0		; turn them all off to begin
	out	LEDS		; write to port
	call	CRLF		; newline to console out
mainloop:
	lxi	d,current	; point to 'current value' string
	mvi	c,strout	; print string function code
	call	sys		; print string ("current port out value..")
	lda	value		; load current LED output value from memory
	call	phex		; print as hex char
	call	CRLF		; newline
	lxi	d,menu		; point to menu
	mvi	c,strout	; print string function code
	call	sys		; print string (the full menu)
	mvi	c,conin		; console input (one char), blocking
	call	sys		; console in to A register
	sta	choice		; save choice to memory
	call	CRLF 		; newline
	cpi	'x'		; compare to ascii 'x'
	jz	cleanup		; yes, it was 'x', go to exit
	cpi	'0'		; was it less than ascii '0'?
	jm	badentry	; yup, show message and try again
	cpi	'8'		; was entry greater than ascii '7'?
	jp	badentry	; yup, show message and try again
	sui	'0'		; subtract ascii '0' to get 0-7 value
	sta 	choice		; save 0-7 value to memory
	mvi 	a,1		; set register A to 1
	mov	b,a		; put that in B (this will be shifted)
	lda	choice		; user entry 0-7 back in A register
	cpi	0		; get initial compare
shftloop:
	jz	setout		; done sifting? then set the output
	mov	a,b		; get shifted bit value in A
	rlc			; shift left one
	mov	b,a		; save it back to B
	lda	choice		; get count from memory
	sui	1		; decrement count (sets Z flag)
	sta	choice		; save it back to memory
	jmp	shftloop	; go shift again
setout:
	lda	value		; get current LED output value to A
	xra	b		; XOR shifted value with current
	out	LEDS		; write it out
	sta	value		; save new current output to memory
	jmp	mainloop	; here we go again...

badentry:
	lxi	d,badntry	; point to bad entry message
	mvi	c,strout	; print string function code
	call	sys		; print it to console
	jmp	mainloop	; back to try again

;*************************************************
; Final clean up before exit
; restore original stack pointer, etc.
;*************************************************
cleanup:
	lhld	orgstk		; load original stack pointer to H/L pair
	sphl			; set stack pointer from H/L pair
	ret			; back to CCP, end of program


;*************************************************
; Start of subroutines
;*************************************************

;
; PCHAR
; print a character to console out
; IN:
;	reg A - character to print
; all registers preserved

PCHAR:
	push	h		; save registers that we'll
	push	d		; be using
	push	b
	mvi	c,conout	; CP/M function for char out
	mov	e,a		; char needs to be in Reg e
	call	sys		; call CP/M to print
	pop	b		;
	pop	d		; restore registers
	pop	h		; that were used
	ret

;
; CRLF
; output carriage return/linefeed to console out

CRLF:
	push	psw		; save A reg
	mvi	a,CR		; set A to carriage return
	call	PCHAR		; go print it
	mvi	a,LF		; set A to linefeed
	call	PCHAR		; go print it
	pop	psw		; restore A reg
	ret
;
; PNIB
; Print low nibble in register A
; register A stomped

PNIB:
	ani	0fh		; mask off upper nibble
	cpi	10		; value of low nibble under 10?
	jnc	PNIB10		; nope, it's hex a-f, go there
	adi	'0'		; add 30h to value (for ascii 0-9)
	jmp	PNIBPRN		; print it out
PNIB10:
	adi	'a'-10		; hex a-f, add ascii 'A' offset
PNIBPRN:
	call	PCHAR		; print
	ret
;
; PHEX
; print hex (2 chars)
; IN char to print in register A
PHEX:
	push	psw		; save A register
	rrc			;
	rrc			; start with upper nibble
	rrc			; so rotate bits right
	rrc			; 4 bits
	call	PNIB		; print this upper nibble
	pop	psw		; restore A register
	push	psw		; save it again
	call	PNIB		; print lower nibble
	pop	psw		; final restore before return
	ret

;
; STRLEN
; length of string (null terminated)
; IN:
;	D/E - addr of string
; OUT:
;	B/C - length of string
; stomps A
; stomps B/C
STRLEN:
	push 	d		; save addr of string
	lxi	b,0		; zero out B/C pair
slloop:
	ldax	d		; get char pointed to by D/E
	cpi	00h		; is it NULL (0)?
	jz	sldone		; yup, get out
	inx	b		; increment B/C
	inx	d		; next char pointed to by H/L
	jmp	slloop		; compare next
sldone:
	pop	d		; restore char pointer
	ret

;
; STRCMP
; compare 2 strings
; IN:
;	B/C - addr of string 1
;	D/E - addr of string 2
; OUT:
;	PSW flags (zero, plus, minus).  Zero flag means match
STRCMP:
	push	h		; save H/L pair (using l)
	push	b		; save addr of string 1
	push	d		; save addr of string 2
scloop:
	ldax	b		; get char pointed to by B/C
	mov	l,a		; save it in Reg l
	ldax	d		; get char pointed to by D/E
	cmp	l		; compare 2 characters
	jnz	scdone		; done if they are not the same
	inx	b		; next char of str 1
	inx	d		; next char of str 2
	cpi	0		; null? end of string?
	jnz	scloop		; nope, next char
scdone:
	pop	d		; restore pointer to start of str 1
	pop	b		; restore pointer to start of str 2
	pop	h		; restore Reg H/L pair
	ret			; all done!


;***************************************************
; START OF FIXED DATA AREA
;***************************************************
welcome	db	'A-D 8800 LED card control',LF,'$'
setmsg	db	'Turning all 8 LEDs on',LF,'$'
clrmsg	db	'Turning all 8 LEDs off',LF,'$'
menu	db	'  0) Toggle LED 0',LF
	db	'  1) Toggle LED 1',LF
	db	'  2) Toggle LED 2',LF
	db	'  3) Toggle LED 3',LF
	db	'  4) Toggle LED 4',LF
	db	'  5) Toggle LED 5',LF
	db	'  6) Toggle LED 6',LF
	db	'  7) Toggle LED 7',LF
	db	'  x) Exit program',LF
	db	'Enter choice> ','$'
current	db	'Current port out value (hex): $'
badntry	db	'Bad input value, use 0-7 or x',LF,'$'
argset	db	'SET',0
argclr	db	'CLR',0
value	db	0
choice	db	0

;***************************************************
; START OF VARIABLE DATA AREA
;***************************************************
orgstk	ds	2		; original stack pointer
	ds	64		; 32 level stack
newstk				; top of newstk area

	end
