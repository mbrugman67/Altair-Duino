;****************************************************
; stem.asm
; Simple file manager
; This is written for use with CP/M
;
; Usage:
; assemble with 'asm':  asm stem
; load: load stem
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
reset   equ     0		; reset (reload CP/M)
conin	equ     1		; console input (blocking)
conout	equ     2		; console output function
rdinp	equ     3		; paper tape reader input
pnchout	equ     4		; paper tape puncher
lstout	equ     5		; list output (printer)
dirio	equ     6		; direct console I/O
getio	equ     7		; get IOBYTE
setio	equ     8		; set IOBYTE
strout	equ     9		; string output function
rdcbuf	equ     10		; read console buffer
constat equ     11		; get console status
version	equ     12		; get CP/M version
rstdsk	equ     13		; reset disk system
seldsk	equ     14		; select disk
fopen   equ     15  		; open file
fclose  equ     16  		; close file
srchf   equ     17  		; search first file
srchn   equ     18  		; search next file
fdel    equ     19  		; delete file
rdseq   equ     20  		; read sequential
wrseq   equ     21  		; write sequential
fmake   equ     22  		; make file
fname   equ     23  		; rename file
lvect   equ     24  		; return login vector
getdsk	equ     25		; return current disk
setdma	equ     26		; set DMA address
alloc	equ     27		; get address (alloc)
pdsk	equ     28		; protect disk
getrov	equ     29		; get R/O vector
setatt	equ     30		; set file attributes
gdprarm	equ     31		; get address (disk params)
gsuer	equ     32		; get/set user code
rdrnd	equ     33		; read random
wrrnd	equ     34		; write random
fsize	equ     35		; compute file size
frset	equ     36		; set random record
rstdrv	equ     37		; reset drive
wrtzer	equ     40		; write random with zero fill

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
fnbflen equ     452             ; length of filename buffer
fnlen   equ     11              ; length of 8.3 filename (8+3)

;****************************************************
; Start of actual program code here
; .org 100h - TPA area starts at 0x0100
;****************************************************
	org	0100h		; entry point at 100 hex (TPA area)
	lxi	h,0		; zero H/L register pair
	dad	sp		; get stack pointer from CCP to H/L
	shld	orgstk		; save original stack pointer to memory
	lxi	sp,newstk	; set stack pointer to our new stack

        call    cls             ; clear terminal screen
        mvi     d,5             ; move cursor to line 5
        mvi     e,16            ; move cursor to column 16
        call    setcur          ; move cursor
	mvi	c,strout	; load reg C with command to print string
	lxi	d,welcom1	; point to address of welcome #1 string
	call	sys		; call the system function to print string
        mvi     d,6             ; move cursor to line 6
        mvi     e,13            ; move cursor to column 13
        call    setcur          ; move cursor
	mvi	c,strout	; load reg C with command to print string
	lxi	d,welcom2	; point to address of welcome #2 string
	call	sys		; call the system function to print string
        mvi     d,8             ; move cursor to line 8
        mvi     e,11            ; move cursor to column 11
        call    setcur          ; move cursor
	mvi	c,strout	; load reg C with command to print string
	lxi	d,welcom3	; point to address of welcome #3 string
	call	sys		; call the system function to print string

        mvi     c,conin         ; wait for keypress
        call    sys             ; waiting, key will be in A

        call    cls             ; clear screen again
        lxi     d,mworkng       ; message "Enumerating files"...
        mvi     c,strout        ; print string to terminal
        call    sys             ; call CP/M to do it
        mvi     c,getdsk        ; get current disk number (A=0, etc)
        call    sys             ; ask CP/M for it, return in A
        adi     'A'             ; bias for ASCII
        call    pchar           ; print that
        mvi     a,':'           ; trailing colon after drive letter
        call    pchar           ; print to terminal
        call    crlf            ; return/newline

        call    enumdir         ; get the listing for the logged drive
        call    fndisp          ; formatted listing of filenames
        call    crlf
        mvi     c,strout        ; print string
        lxi     d,donemsg       ; "File count" message
        call    sys             ; CP/M
        lhld    fcount          ; get file count
        call    prn16           ; print it as decimal
        call    crlf



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
; pchar
; print a character to console out
; IN:
;	reg A - character to print
; all registers preserved
pchar:
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
; crlf
; output carriage return/linefeed to console out
crlf:
	push	psw		; save A reg
	mvi	a,CR		; set A to carriage return
	call	pchar		; go print it
	mvi	a,LF		; set A to linefeed
	call	pchar		; go print it
	pop	psw		; restore A reg
	ret

;
; pnib
; Print low nibble in register A
; register A stomped
pnib:
	ani	0fh		; mask off upper nibble
	cpi	10		; value of low nibble under 10?
	jnc	pnib10		; nope, it's hex a-f, go there
	adi	'0'		; add 30h to value (for ascii 0-9)
	jmp	pnibprn		; print it out
pnib10:
	adi	'a'-10		; hex a-f, add ascii 'A' offset
pnibprn:
	call	pchar		; print
	ret

;
; phex
; print hex (2 chars)
; IN char to print in register A
phex:
	push	psw		; save A register
	rrc			;
	rrc			; start with upper nibble
	rrc			; so rotate bits right
	rrc			; 4 bits
	call	pnib		; print this upper nibble
	pop	psw		; restore A register
	push	psw		; save it again
	call	pnib		; print lower nibble
	pop	psw		; final restore before return
	ret

;
; prn16
; Print 16-bit binary value as decimal
; IN:
;       H/L - value to print
; all registers preserved
;
prn16:
        push    h               ; save registers
        push    b
        push    d
        mvi     b, 0            ; Used as a flag to skip printing leading zeroes
        lxi     d,-10000        ; ten thousands place
        call    pdec            ; go print it
        lxi     d,-1000         ; tousands place
        call    pdec            ; go print it
        lxi     d,-100          ; hundreds place
        call    pdec            ; go print it
        lxi     d,-10           ; tens place
        call    pdec            ; go print it
        mov     a,l             ; get remaining ones place
        adi     '0'             ; offset for ascii
        call    pchar           ; and print it
        pop     d               ; restore registers
        pop     b
        pop     h
        ret

;
; prn8
; Print 8-bit binary value as decimal
; IN:
;       L - value to print
; all registers preserved
;
prn8:
        push    h               ; save registers
        push    b
        push    d
        mvi     b, 0            ; Used as a flag to skip printing leading zeroes
        lxi     d,-100          ; hundreds place
        call    pdec            ; go print it
        lxi     d,-10           ; tens place
        call    pdec            ; go print it
        mov     a,l             ; get remaining ones place
        adi     '0'             ; offset for ascii
        call    pchar           ; and print it
        pop     d               ; restore registers
        pop     b
        pop     h
        ret

;
; pdec
; Internal subroutine to print decimal values from
; binary single or multibyte values.  To be called
; from prn16 or prn8 type subroutines
; Uses B register!
;
pdec:
        mvi     c,'0'-1         ; init C
pdec2:
        inr     c               ; increment C
        dad     d               ; add negated D reg (subtract)
        jc      pdec2           ; still more to subtract (no negative yet)
        mov     a,d             ; we need to complement (re-negate to positive)
        cma                     ; complement (change sign)
        mov     d,a             ; store it back
        mov     a,e             ; lower byte, too
        cma                     ; complement that one
        mov     e,a             ; now D/E contain positive value
        inx     d               ; 2's complement, gotta add one
        dad     d               ; add that "one too many" back
        mov     a,c             ; get that count (remember, it's ascii now)
        cpi     '0'             ; check for zero
        jz      pdecz           ; check if this is a leading zero
        mvi     b,0ffh          ; set flag - this is non zero
        call    pchar           ; print it
        jmp     pdecr           ; done here
pdecz:                          ; checking for leading zeroes
        mov     a,b             ; get the flag
        cpi     0               ; has it been set?
        jz      pdecr           ; it's a leading zero, just return
        mov     a,c             ; get the zero back!
        call    pchar           ; not a leading zero, print
pdecr:
        ret

;
; byt2asc
; Convert byte to hex ascii numeric (e.g., 0x80 -> 313238
; IN
;       A - byte to Convert
;       H/L - buffer pointer to put the bytes.  Buffer will
;             be incremented to the 'next' location
byt2asc
        push    d               ; save regs on entry
        push    b               ;
        mvi     e,0             ; leading zero flag
        mvi     d,100           ; hundreds place first
b2asc1:
        mvi     c,'0'-1         ; starting value
b2asc2:
        inr     c               ; increment
        sub     d               ; subtract off 100/10 value
        jnc     b2asc2          ; still positive
        add     d               ; add back to get positive
        mov     b,a             ; modulo (remainder)
        mov     a,c             ; 100/10 value
        cpi     '1'             ; compare to ascii 1
        jnc     b2asc3          ; less than 1 (it is zero)
        mov     a,e             ; get zero flag
        ora     a               ; or sets zero flag
        mov     a,c             ; get the byte again
        jz      b2asc4          ; this zero is a leading zero
b2asc3:
        mov     m,a             ; stuff the ascii byte into buffer
        inx     h               ; increment buffer
        mvi     e,0ffh          ; add number, set leading zero flag
b2asc4:
        mov     a,d             ; get 100/10 place value
        sui     90              ; go from 100 to 10
        mov     d,a             ; put it back
        mov     a,b             ; get modulo from previous
        jnc     b2asc1          ; still more to subtract off
        adi     '0'             ; ascii bias
        mov     m,a             ; store it to buffer
        inx     h               ; increment buffer
        pop     b               ; restore
        pop     d               ; registers
        ret

;
; strlen
; length of string (null terminated)
; IN:
;	D/E - addr of string
; OUT:
;	B/C - length of string
; stomps A
; stomps B/C
strlen:
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
; strcmp
; compare 2 strings
; IN:
;	B/C - addr of string 1
;	D/E - addr of string 2
; OUT:
;	PSW flags (zero, plus, minus).  Zero flag means match
strcmp:
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
; memcpy
; memory copy from one buffer to another
; IN:
;       A count to copy
;       B/C source
;       D/E destination
memcpy:
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
; memset
; set a buffer to a fixed byte value.
; THIS VERSION ONLY DOES UP TO 255 BYTES!
; IN:
;       A byte value to set buffer to
;       B count of bytes to set
;       D/E buff
memset:
	push	d		; save addr of buffer
	push    psw             ; save set byte
        push    b               ; save count
	mov     a,b
	cpi     0
	pop     b
	pop     psw
	push    b
mstloop:
        jz      mstdone         ; done setting, cleanup and return
        stax    d               ; set addr to desired values
        inx     d               ; next buffer address
        dcr     b               ; decrement copy count
        jmp     mstloop
mstdone:
        pop     b               ; restore count
        pop     d               ; restore addr
        ret                     ; get outta here

;
; first
; get first file FCB in directory
; RETURNS:
;       A - result (0xff if no file found)
;       B/C pair - address of file control block
;
first:
        lxi     b,schfcb        ; set B/C to search value for find first
        lxi     d,fcb1          ; set D/E to file control block 2
        mvi     a,fcblen        ; 32 bytes to copy
        call    memcpy          ; copy search value to FCB1
        mvi     c,srchf         ; search first file function
        call    sys             ; call CP/M, result in A
        lxi     b,dma1          ; set address of FCB as DMA1
        ret

;
; nextf
; get next file name in directory
; RETURNS:
;       A - result (0xff if no file found)
;       B/C pair - address of file control block
;
nextf:
        mvi     c,srchn         ; search next file function
        call    sys             ; call CP/M, result in A
        cpi     255             ; compare to 0xff - no file found
        jz      nextdone        ; no file was found!
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
nextdone:
        ret

;
; pfname
; Print the filename stored in a FCB
; IN:
;       B/C - address of FCB
pfname:
        push    psw             ; save A and PSW registers
        push    h               ; save H/L register pair
        push    b               ; save FCB address
        inx     b               ; name starts at byte 1
        mvi     l,8             ; 8 chars in filename
pfnl1:
        ldax    b               ; get next char in name
        cpi     ' '             ; is it a space?
        jz      pfnskp          ; don't print a spacee
        call    pchar           ; print to console
pfnskp
        inx     b               ; next byte in FCB
        dcr     l               ; decrement L, will set Z flag on zero
        jnz     pfnl1           ; next char

        mvi     a,'.'           ; name delimeter
        call    pchar
        mvi     l, 3            ; 3 chars in extension
pfnl2:
        ldax    b               ; get next char in name
        call    pchar           ; print to console
        inx     b               ; next byte in FCB
        dcr     l               ; decrement L, will set Z flag on zero
        jnz     pfnl2           ; next char
        pop     b               ; restore FCB address
        pop     h               ; restore H/L register pair
        pop     psw             ; restore A and PSW registers
        ret

;
; cls
; Clear terminal using ANSII escape sequence
; sequence is ^[2J
;
cls:
        push    h               ; save H/L
        push    b               ; save B/C
        lxi     h,esctmp        ; get buffer
        mvi     m,01bh          ; escape character
        inx     h               ; next byte
        mvi     m,'['           ; CSI (Control Sequence Introducer)
        inx     h               ; next byte
        mvi     m,'2'           ; STILL
        inx     h               ;  SETTING
        mvi     m,'J'           ;   ANSI
        inx     h               ;    SEQUENCE....
        mvi     m,'$'           ; terminator for CP/M send string syscall
        lxi     d,esctmp        ; set buffer
        mvi     c,strout        ; set function
        call    sys             ; call CP/M
        mvi     d,1             ; move cursor to
        mvi     e,1             ; top left
        call    setcur          ; after clearning screen
        pop     b               ; restore
        pop     h               ; restore
        ret

;
; setcur
; Set cursor position on screen
; sequence is ^[{line};{col}H
; IN:
;       D - line
;       E - col
setcur:
        push    psw             ; save A and PSW
        push    b               ; save B/C pair
        push    d               ; save D/E pair
        push    h               ; save H/L pair
        lxi     h,esctmp        ; get buffer for building sequence
        mvi     m,01bh          ; escape
        inx     h               ; next byte
        mvi     m,'['           ; CSI (Control Sequence Introducer)
        inx     h               ; next byte
        mov     a,d             ; get the desired line
        call    byt2asc         ; add to the buffer
        mvi     m,';'           ; separator
        inx     h               ; next byte
        mov     a,e             ; get desired column
        call    byt2asc         ; add to buffer
        mvi     m,'H'           ; finish off the command
        inx     h               ; next byte
        mvi     m,'$'           ; for CP/M command
        lxi     d,esctmp        ; back to start of buffer of CP/m
        mvi     c,strout        ; syscall for string out to the terminal
        call    sys             ; let CP/M do the thing
        pop     h               ; restore H/L
        pop     d               ; restore D/E
        pop     b               ; restore B/C
        pop     psw             ; restore A and PSW
        ret                     ; bye!

;
; enumdir
; Enumerate directory - get all filenames on the currently
; logged disk
; RETURNS
;       fcount - number of files found (16-bit integer)
; Stomps on most registers :/
enumdir:
	lhld    flist           ; get the address file bufffer
        shld    flposn          ; save that as pointer (start at beginning)
        mvi     l,0             ; file count is a 16-bit integer
        mvi     h,0             ; start with it zeroed out
        shld    fcount          ; and store it

        call    first           ; find first file, FCB is in B/C
        cpi     0ffh            ; compare a to 255; 255 means no file found
        jnz     frstok          ; we found a file!
        mvi     c,strout        ; gonna print a string
        lxi     d,nofiles       ; msg: no files on disk
        jmp     cleanup         ; we're all done here
frstok:
        inx     b               ; B/C holds FCB1, filename starts at second byte
        lxi     d,flposn        ; get next address of filename buffer
        mvi     a,fnlen         ; set copy len to be 8+3 (file name + ext in FCB)
        call    memcpy          ; copy filename to buffer
        lxi     h,flposn        ; get the buffer pointer again
        mvi     b,0             ; load B/C with filename lenght
        mvi     c,fnlen         ; ''
        dad     b               ; add filename length to buffer pointer
        shld    flposn          ; save it
        ;call    pfname          ; print the first file name
        ;call    crlf            ; newline/return
        lhld    fcount          ; get file count
        inx     h               ; increment
        shld    fcount          ; save
nxtfl:
        call    nextf           ; look for another file
        cpi     0ffh            ; no more files found
        jz      enumret         ; so break out of loop
        inx     b               ; B/C holds FCB1, filename starts at second byte
        lxi     d,flposn        ; get next address of filename buffer
        mvi     a,fnlen         ; set copy len to be 8+3 (file name + ext in FCB)
        call    memcpy          ; copy filename to buffer
        lxi     h,flposn        ; get the buffer pointer again
        mvi     b,0             ; load B/C with filename lenght
        mvi     c,fnlen         ; ''
        dad     b               ; add filename length to buffer pointer
        shld    flposn          ; save it
        ;call    pfname          ; print this filename
        ;call    crlf            ; newline/return
        lhld    fcount          ; get file count
        inx     h               ; increment
        shld    fcount          ; save
        jmp     nxtfl           ; go try again
enumret:
        ret                     ; all done!

;
; fndisp
; Formatted display of filenames
;
fndisp
        push    psw             ; save A and PSW
        push    b               ; save B/C
        push    d               ; save D/E
        push    h               ; save H/L
        lhld    fcount          ; get file count
        mov     a,l             ; count to a
        cpi     0               ; is it zero (no files?)
        jz      fnddone         ; then we're done
        mov     c,a             ; save file count
        mov     d,4             ; first line to use - 1
        lhld    flist           ; get buffer address
fndlp:
        inr     d               ; next line
        mvi     e,3             ; start in column 3
        call    setcur          ; move cursor there
        mvi     a,'['           ;
        call    pchar           ;
        mvi     a,' '           ;
        call    pchar           ;
        mvi     a,']'           ;
        call    pchar           ;
        mvi     a,' '           ;
        call    pchar           ;
        mvi     b,8             ; 8 characters in filename portion
fndlp1:
        mov     a,m             ; get next char
        inx     h               ; increment buffer pointer
        cpi     ' '             ; is it a space?
        jz      fndskp          ; don't print a space
        call    pchar           ; print non-space char
fndskp:
        dcr     b               ; decrement count
        mov     a,b             ; move to A to check
        jnz     fndlp1          ; more to print
        mvi     a,'.'           ; delimiter
        call    pchar           ; print it
        mvi     b,3             ; 3 chars in extension portion
fndlp2:
        mov     a,m             ; get next char
        inx     h               ; increment buffer pointer
        call    pchar           ; print it
        dcr     b               ; decrement char count
        mov     a,b             ; move for compare
        cpi     0               ; done?
        jnz     fndlp2          ; nope, guess not :/
        dcr     c               ; decrement file count
        mov     a,c             ; move for compare
        ;jnz     fndlp           ; go do next
fnddone:
        pop     h               ; restore H/L
        pop     d               ; restore D/E
        pop     b               ; restore B/C
        pop     psw             ; restore A and PSW
        ret                     ; all done!

;***************************************************
; START OF FIXED DATA AREA
;***************************************************
welcom1	db	'Altair 8800 File Manager$'
welcom2 db      '2026 Mattydyne Heavy Industries$'
welcom3 db      '"H" for help, any other key to start',CR,LF,'$'
nofiles db      'No files found!',CR,LF,'$'
donemsg db      'File count: $'
schfcb  db      0,'???????????',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
mworkng db      'Enumerating files in drive $'

;***************************************************
; START OF VARIABLE DATA AREA
;***************************************************
fcount  dw      1               ; file count addr
flposn  dw      1               ; position in file name array
flist   ds      512             ; area for list of filenames
esctmp  ds      24              ; temp area for building escape sequences
orgstk	ds	2		; original stack pointer
canary  ds      4               ; stack canary
	ds	64		; 32 level stack
newstk				; top of newstk area

	end




;getsz   db      01bh,'[999,999h',01bh,'[6n$'
;msg1    db      'Getting console size (line,column): $'
        ;mvi     c,strout
        ;lxi     d,msg1
        ;call    sys

        ; get size
        ;mvi     c,strout
        ;lxi     d,getsz
        ;call    sys

;szloop:
        ;mvi     b,0
        ;mvi     c,conin
        ;call    sys

        ;cpi     'R'
        ;jz      szlp1
        ;call    pchar
        ;mvi     a,' '
        ;call    pchar
        ;jmp     szloop


;szlp1:
        ;call    crlf
