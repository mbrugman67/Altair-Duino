;****************************************************
; xt.asm
; Simple file manager
; This is written for use with CP/M
;
; Usage:
; assemble with 'asm':  asm leds
; load: load leds
;
;****************************************************
; subroutines PCHAR, PNIB, PHEX, and CRLF are taken
; from the CP/M manual and modified a bit.
;****************************************************
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
fcblen	equ 	32		; byte size of File Control Block

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

        mvi     c,strout
        lxi     d,result
        call    sys
        call    FIRST
        call    PHEX
        mvi     a,'-'
        call    PCHAR
        call    PFNAME
        call    CRLF

        mvi     c,strout
        lxi     d,nresult
        call    sys
        call    NEXTF
        call    PHEX
        mvi     a,'-'
        call    PCHAR
        call    PFNAME
        call    CRLF

        mvi     c,strout
        lxi     d,nresult
        call    sys
        call    NEXTF
        call    PHEX
        mvi     a,'-'
        call    PCHAR
        call    PFNAME
        call    CRLF



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


;
; MEMCPY
; memory copy from one buffer to another
; IN:
;       A count to copy
;       B/C source
;       D/E destination
MEMCPY:
        push    psw             ; save a/status register
	push	h		; save H/L pair (using l)
	push	b		; save addr of source
	push	d		; save addr of destination
	mov     l,a             ; save count to L
	cpi     0               ; initial compare, make sure there's
mcploop:                        ; work to do
        jz      mcpdone         ; done copying, clean up and return
        ldax    b               ; get next byte from source
        stax    d               ; save to destination
        inx     b               ; next source
        inx     d               ; next destination
        dcr     l               ; decrement copy count
        jmp     mcploop         ; check and do next byte
mcpdone:
        pop     d               ; restore dst addr
        pop     b               ; restore src addr
        pop     h               ; restore H/L pair
        pop     psw             ; resture A and status flags
        ret                     ; get outta here

;
; MEMSET
; set a buffer to a fixed byte value.
; IN:
;       A byte value to set buffer to
;       B count of bytes to set
;       D/E buff
MEMSET:
        push    b               ; save count
	push	d		; save addr of buffer
	cpi     0               ; initial compare, make sure there's
mstloop:                        ; work to do here
        jz      mstdone         ; done setting, cleanup and return
        stax    d               ; set addr to desired values
        inx     d               ; next buffer address
        dcr     b               ; decrement copy count
        jmp     mstloop
mstdone:
        pop     d               ; restore buffer addr
        pop     psw             ; restore count
        ret                     ; get outta here

;
; FIRST
; get first file FCB in directory
; RETURNS:
;       A - result (0xff if no file found)
;       B/C pair - address of file control block
;
FIRST:
        lxi     b,schfcb        ; set B/C to search value for find first
        lxi     d,fcb1          ; set D/E to file control block 2
        mvi     a,fcblen        ; 32 bytes to copy
        call    MEMCPY          ; copy search value to FCB1
        mvi     c,srchf         ; search first file function
        call    sys             ; call CP/M, result in A
        lxi     b,dma1          ; set address of FCB as DMA1
        ret

;
; NEXTF
; get next file name in directory
; RETURNS:
;       A - result (0xff if no file found)
;       B/C pair - address of file control block
;
NEXTF:
        mvi     c,srchn         ; search next file function
        call    sys             ; call CP/M, result in A
        cpi     255             ; compare to 0xff - no file found
        jz      NEXTDONE        ; no file was found!
        rlc                     ;
        rlc                     ; do 5 left shifts on the A
        rlc                     ; register,
        rlc                     ; this multiplies the value in
        rlc                     ; A by 32 to get offset of next FCB
        lxi     h,0             ; load the H/L pair with the value
        mov     l,a             ; in A
        lxi     b,dma1          ; load B/C pair with base address of DMA
        dad     b               ; H/L now contain address of new found FCB
        mov     b,h             ; save that to
        mov     c,l             ; B/C pair
NEXTDONE:
        ret

;
; PFNAME
; Print the filename stored in a FCB
; IN:
;       B/C - address of FCB
PFNAME:
        push    psw             ; save A and PSW registers
        push    h               ; save H/L register pair
        push    b               ; save FCB address
        inx     b               ; name starts at byte 1
        mvi     l,8             ; 8 chars in filename
PFNL1:
        ldax    b               ; get next char in name
        call    PCHAR           ; print to console
        inx     b               ; next byte in FCB
        dcr     l               ; decrement L, will set Z flag on zero
        jnz     PFNL1          ; next char

        mvi     a,'.'           ; name delimeter
        call    PCHAR
        mvi     l, 3            ; 3 chars in extension
PFNL2:
        ldax    b               ; get next char in name
        call    PCHAR           ; print to console
        inx     b               ; next byte in FCB
        dcr     l               ; decrement L, will set Z flag on zero
        jnz     PFNL2          ; next char
        pop     b               ; restore FCB address
        pop     h               ; restore H/L register pair
        pop     a               ; restore A and PSW registers
        ret

;***************************************************
; START OF FIXED DATA AREA
;***************************************************
welcome	db	'A-D 8800 file manager',CR,LF,'$'
nofile  db      'No file found!',CR,LF,'$'
result  db      'Find first file result: 0x','$'
nresult db      'Find next file result: 0x','$'
schfcb  db      0,'???????????',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

;***************************************************
; START OF VARIABLE DATA AREA
;***************************************************
flist   ds      256             ; area for double-null-
                                ; terminated list of filenames
orgstk	ds	2		; original stack pointer
canary  ds      4               ; stack canary
	ds	64		; 32 level stack
newstk				; top of newstk area

	end
