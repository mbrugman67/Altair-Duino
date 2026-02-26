#include "telnetBridge.h"
#include "globals.h"

void TelnetBridge::begin()
{
  inbuf = nullptr;
  connected = false;
  bufPosn = 0;
  tc.setPromptChar('$');
}

void TelnetBridge::setPromptChar(char c)
{
  tc.setPromptChar(c);
}

bool TelnetBridge::connect(const char* server, const char* user, const char* pass)
{
  connected = false;
  
  if (!wifiMgr.isConnected())
  {
    sserial.printf("Wifi must be connected before telnet can be used!\n");
    return (false);
  }

  Serial.printf("Attempting to connect to telnet server %s as user %s\n", server, user);

  if (!tc.login(server, user, pass))
  {
    Serial.printf("That didn't work.\r\n");
    sserial.printf("Did not connect!\r\n");
  }
  else
  {
    Serial.printf("Connected!\r\n");
    connected = true;

    if (!inbuf)
    {
      inbuf = (char*)ps_malloc(1024);
    }

    if (!inbuf)
    {
      Serial.printf("Didn't allocate inbuf!\r\n");
      tc.disconnect();
      connected = false;
    }
    else
    {
      Serial.printf("inbuf allocated at %p\r\n", inbuf);
    }
  }

  return (connected);
}

bool TelnetBridge::isConnected()
{
  return (connected);
}

bool TelnetBridge::update(bool terminate)
{
  if (connected)
  {
    if (!tc.isTCPConnected())
    {
      connected = false;
      return (false);
    }

    if (terminate)
    {
      tc.disconnect();
      connected = false;
      Serial.printf("Got termination request\r\n");
    }
    while(sserial.available())
    {
      char c = sserial.read();

      if (c == 0x1d)
      {
        connected = false;
        //sserial.flush();
        tc.disconnect();

        sserial.printf("\n\nDISCONNECTED FROM REMOTE SERVER\n");
        serialMenu.showMenu();
      }

      /*
      if (isprint(c))
      {
        Serial.printf("%c", c);
      }
      else
      {
        Serial.printf(">0x%02x<", (int)c);
      }
      */

      sserial.printf("%c", c);
      
      if (c == 0x0d)// || c == '\n')
      {
        inbuf[bufPosn] = '\0';
        tc.sendCommand(inbuf);
        //tc.send(inbuf);

        //Serial.printf("Sending> %s\r\n", inbuf);

        bufPosn = 0;
      }
      else
      {
        inbuf[bufPosn++] = c;
      }
    }
  }

  return (connected);
}