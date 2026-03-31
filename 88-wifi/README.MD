# 88-WIFI
This is a wifi expansion bus card for the Altair-Duino 8800 computer kit.  The card will provide simple wifi connectivity to the Altair 8800 Experimenter's kit with an eye for:
+ Simple connectivity to an AP 
  + Initial plan is for the device to only be a client, not to act as an AP
+ Initial WiFi functionality for:
  + FTP - provide basic FTP functionality to allow easy file transer to/from an FTP server
  + Telnet - basic telnet connectivity to servers

Later possible functionality could include generic TCP/UDP socket interface.

## Hardware
The card is a carrier and bus interface to an ESP32 dev board; the ESP32 will do the heavy lifting of buffering data between the slower A-D and the wireless network.

## Software/Firmware

### I/O Ports
The bus card will use 4 I/O ports:
| PORT ADDR | DIRECTION | DESCRIPTION |  
| --- | --- | --- |  
| BASEADDR | OUT | Status word (one byte) |  
| BASEADDR | IN | Control word (one byte) |  
| BASEADDR + 1 | OUT | Command Response word (one byte) |  
| BASEADDR + 1 | IN | Command word (one byte) |  
| BASEADDR + 2 | OUT | Command data out |  
| BASEADDR + 2 | IN | Command data in |  
| BASEADDR + 3 | OUT | Data out |  
| BASEADDR + 3 | IN | Data in |  

`Direction` is with respect to the A-D; so `OUT` is out from A-D to the expansion card.  The respects the `IN` and `OUT` Intel 8080 assembly commands.

#### Status Word
```  
BIT POSN (7 is MSB):
 7 6 5 4 3 2 1 0
 | | | | | | | |
 | | | | | | | +-- Board Error (1 = error, 0 = OK)
 | | | | | | +---- 1 = WiFi Connected, 0 = not connected
 | | | | | +------ 1 = Busy, 0 = idle
 | | | | +-------- 1 = Command data available, 0 = no cmd data
 | | | +---------- 1 = WiFi data available, 0 = no data
 | | +------------ 1 = TX buffers full, 0 = TX buffer available
 | +-------------- 1 = RX buffers full, 0 = RX buffer available
 +---------------- Reserved
 ```  

#### Control Word
```  
BIT POSN (7 is MSB):
 7 6 5 4 3 2 1 0
 | | | | | | | |
 | | | | | | | +-- 1 = Reset WiFi connector (soft reset of ESP32)
 | | | | | | +---- 1 = Connect to AP
 | | | | | +------ 1 = AP Scan
 | | | | +-------- 1 = Disconnect (flushes buffers)
 | | | +---------- 1 = 
 | | +------------
 | +--------------
 +----------------
 ```
