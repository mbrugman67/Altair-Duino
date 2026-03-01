;
; This is part of the proof of concept of the serial <-> wifi adapter
;
; this is meant to be hand-switched in to the Altair
; Yes, I know that this is just a dump serial echo, but it's still fun :)


.org 0000           ; start at address zero

IN 0x10             ; in port 16 (stat register of first port on 88-2SIO)
ANI 0x01            ; immediate logical AND with 1 (bit 0 true means data at RX)
JZ 0x000e           ; no data - go check status of second port on 88-2SIO
IN 0x11             ; get byte at first serial port's RX register
OUT 0x13            ; shove that byte out to the TX register of second serial port
NOP
NOP
NOP
IN 0x12             ; in port 18 (stat register of second port on 88-2SIO)
ANI 0x01            ; check if data available at RX register
JZ 0x0000           ; no data at port - go back to start
IN 0x13             ; get the byte at RX register of second serial port
OUT 0x11            ; and shove that byte right back out to TX on serial port 1
JMP 0x0000          ; back to start

; switches (start at address zero):
; 333 020 346 001 312 016 000 333 021 323 023 000 000 000
; 333 022 346 001 312 000 000 333 023 323 021 303 000 000
;
