# ESP_BRIDGE
This is a proof of concept for creating an Internet networking bridge to an Altair-Duino.

The PoC is communicating over low-level UART serial from an ESP32 to the A-D.

## Altair-Duino configuration
The assumption is that the Altair (or Altair-Duino aka A-D) has an 88-2SIO modules installed.  The first of the two serial ports is going to a serial terminal for user interaction, and the second port is connected to the ESP32.  

For my hardware, I have the host (and 88-2SIO port 1) configured to the RS-232 DB-9 interface at 9600 baud, 8N1.  The second port on the 88-2SIO is connected to the RX/TX LED Port 5 connection at 1200 baud, 8N1.

Also assumed are the following I/O port addresses:

| I/O Port address | Direction | Usage |  
| --- | --- | --- |    
| 16 (&h10) | IN | 88-2SIO first serial port status register |  
| 16 (&h10) | OUT | 88-2SIO first serial port control register |  
| 17 (&h11) | IN | 88-2SIO first serial port data input register |  
| 17 (&h11) | OUT | 88-2SIO first serial port data output register |  
| 18 (&h12) | IN | 88-2SIO second serial port status register |  
| 18 (&h12) | OUT | 88-2SIO second serial port control register |  
| 19 (&h13) | IN | 88-2SIO second serial port data input register |  
| 19 (&h13) | OUT | 88-2SIO second serial port data output register |  

With the A-D, the configuration registers are handled by the config menu (raise `STOP` and `AUX1`).

### Configure host serial
This from the config menu's host serial settings:
```  
Configure host serial settings

(0) USB Programming Port : 115200 baud
(1) Serial (pin 18/19)   : 9600 baud 8N1
(2) USB Native Port      : 115200 baud
(3) Serial (pin A6/A7)   : 9600 baud 8N1
(4) Serial (RXL/TXL)     : 1200 baud 8N1
```  
### Configure serial cards
This is the serial card configuration:
```  
(1) Configure SIO             : Primary (Serial (pin A6/A7))
(2) Configure ACR             : Not mapped
(3) Configure 2SIO port 1     : Primary (Serial (pin A6/A7))
(4) Configure 2SIO port 2     : Serial (RXL/TXL)
```  

Detail for the `SIO` port:
```
Map to host (i)nterface    : Primary (Serial (pin A6/A7))
Simulated (b)aud rate      : 1200
(F)orce baud rate          : no
Example playback (N)ULs    : 4
Use (7) bits               : autodetect
Serial input (u)ppercase   : autodetect
Translate (B)ackspace to   : off
SIO board re(v)ision       : rev1
```  
Again, the `pin A6/A7` serial is the A-D Experimenter DB-9 RS-232 serial port

Detail for `2-SIO port 2`:
```  
Map to host (i)nterface    : Serial (RXL/TXL)
Simulated (b)aud rate      : 1200
(F)orce baud rate          : no
Example playback (N)ULs    : 4
Use (7) bits               : off
Serial input (u)ppercase   : off
Translate (B)ackspace to   : autodetect
```
