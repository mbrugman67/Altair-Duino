#include "menu.h"
#include "globals.h"

#define CFG_FILENAME    "/config.json"
#define CFG_WIFI_SSID   "wifi_ssid"
#define CFG_WIFI_PASS   "wifi_password"
#define CFG_TNET_SERVER "tnet_server"
#define CFG_TNET_USER   "tnet_user"
#define CFG_TNET_PASS   "tnet_pass"
#define CFG_TNET_PROMPT "tnet_prompt"


bool MENU::begin()
{
  bool retVal = false;

  slowStr = (char*)ps_malloc(1024);

  if (!LittleFS.begin(true))
  {
    Serial.printf("Could not init filesystem!\r\n");
    return (false);
  }

  File cfgFile = LittleFS.open(CFG_FILENAME, FILE_READ);
  if (!cfgFile)
  {
    Serial.printf("Setting default config\r\n");
    return (this->setDefaultConfig());
  }
  
  size_t fileSize = cfgFile.size();
  if (fileSize < 1)
  {
    cfgFile.close();
    return (this->setDefaultConfig());
  }
  
  char* json = (char*)ps_malloc(fileSize);
  if (!json)
  {
    Serial.printf("Unable to allocate for config json!\r\n");
  }
  else
  {
    size_t bytesRead = cfgFile.read((uint8_t*)json, fileSize);

    if (bytesRead != fileSize)
    {
      Serial.printf("Bad json read - expected %d bytes, but read %d\r\n", fileSize, bytesRead);
    }
    else
    {
      deserializeJson (cfg, json);
      Serial.printf("Loaded %d of config data\r\n", bytesRead);
      retVal = true;
    }
  }
  
  if (json)
  {
    free(json);
  }

  if (cfgFile)
  {
    cfgFile.close();
  }

  return (retVal);
}

const char* MENU::getSSID()
{
  return (cfg[CFG_WIFI_SSID]);
}

const char* MENU::getWifiPassword()
{
  return (cfg[CFG_WIFI_PASS]);
}

const char* MENU::getTelnetServer()
{
  return (cfg[CFG_TNET_SERVER]);
}

const char* MENU::getTelnetUser()
{
  return (cfg[CFG_TNET_USER]);
}

const char* MENU::getTelnetPassword()
{
  return (cfg[CFG_TNET_PASS]);
}

char MENU::getTelnetPrompt()
{
  char tmp[128];
  strncpy(tmp, cfg[CFG_TNET_PROMPT], 127);
  return (tmp[0]);
}

void MENU::update()
{
  if (sserial.available())
  {
    char c = sserial.read();

    switch (c)
    {
      case '1': this->setSSID(c); break;
      case '2': this->setWifiPassword(c); break;
      
      case '3': this->setTelnetServer(c); break;
      case '4': this->setTelnetUser(c); break;
      case '5': this->setTelnetPassword(c); break;
      case '6': this->setTelnetPrompt(c); break;

      case 'C': this->connectToWiFi(c); break;
      case 'S': this->dumpWiFiStatus(c); break;
      
      case 'T': this->connectToTelnet(c); break;
      
      case 'D': this->dumpConfig(c); break;
    }

    if (!tn.isConnected())
    {
      this->showMenu();
    }
  }
}

void MENU::showMenu()
{
  /*
  sserial.printf("\n\n");
  sserial.printf("----- WIFI CONNECTOR MAIN MENU -----\n");
  sserial.printf("1) Set wifi ssid (%s)\n", this->getSSID());
  sserial.printf("2) Set wifi password\n\n");
  sserial.printf("3) Set remote telnet server address (%s)\n", this->getTelnetServer());
  sserial.printf("4) Set remote telnet username (%s)\n", this->getTelnetUser());
  sserial.printf("5) Set remote telnet password\n");
  sserial.printf("6) Set telnet prompt character (%c)\n\n", this->getTelnetPrompt());
  sserial.printf("C) Connect to Wifi network using stored ssid/pass\n");
  sserial.printf("S) Get Wifi status\n\n");
  sserial.printf("T) Connect to telnet server\n\n");
  sserial.printf("D) Dump config data; NOTE - WILL DISPLAY SECRETS!\n");
  sserial.printf("Enter your choice: ");
  */

  //this->sendStr("\n\n");
  this->sendStr("----- WIFI CONNECTOR MAIN MENU -----\n");
  this->sendStr("1) Set wifi ssid (%s)\n", this->getSSID());
  this->sendStr("2) Set wifi password\n\n");
  this->sendStr("3) Set remote telnet server address (%s)\n", this->getTelnetServer());
  this->sendStr("4) Set remote telnet username (%s)\n", this->getTelnetUser());
  this->sendStr("5) Set remote telnet password\n");
  this->sendStr("6) Set telnet prompt character (%c)\n\n", this->getTelnetPrompt());
  this->sendStr("C) Connect to Wifi network using stored ssid/pass\n");
  this->sendStr("S) Get Wifi status\n\n");
  this->sendStr("T) Connect to telnet server\n\n");
  this->sendStr("D) Dump config data; NOTE - WILL DISPLAY SECRETS!\n");
  this->sendStr("Enter your choice: ");
}

void MENU::setSSID(char c)
{
  sserial.printf("%c\n\n", c);
  this->readString("Enter WiFi SSID: ");
  cfg[CFG_WIFI_SSID] = inputstr;
  this->writeConfig();
}

void MENU::setWifiPassword(char c)
{
  sserial.printf("%c\n\n", c);
  this->readString("Enter WiFi password: ");
  cfg[CFG_WIFI_PASS] = inputstr;
  this->writeConfig();
}

void MENU::setTelnetServer(char c)
{
  sserial.printf("%c\n\n", c);
  this->readString("Enter telnet server (IP or hostname): ");
  cfg[CFG_TNET_SERVER] = inputstr;
  this->writeConfig();
}

void MENU::setTelnetUser(char c)
{
  sserial.printf("%c\n\n", c);
  this->readString("Enter telnet username: ");
  cfg[CFG_TNET_USER] = inputstr;
  this->writeConfig();
}

void MENU::setTelnetPassword(char c)
{
  sserial.printf("%c\n\n", c);
  this->readString("Enter telnet password: ");
  cfg[CFG_TNET_PASS] = inputstr;
  this->writeConfig();
}

void MENU::setTelnetPrompt(char c)
{
  sserial.printf("%c\n\n", c);
  char s[2];
  s[1] = '\0';
  s[0] = this->readChar("Enter telnet prompt character: ");
  cfg[CFG_TNET_PROMPT] = s;
  this->writeConfig();
}

void MENU::connectToTelnet(char c)
{
  sserial.printf("%c\n\n", c);
  tn.setPromptChar(this->getTelnetPrompt());
  tn.connect(this->getTelnetServer(), this->getTelnetUser(), this->getTelnetPassword());
}

void MENU::dumpConfig(char c)
{
  sserial.printf("%c\n\n", c);
  serializeJsonPretty(cfg, sserial);
  sserial.printf("\n");
}

void MENU::connectToWiFi(char c)
{
  sserial.printf("%c\n\n", c);

  sserial.printf("Attempting to connect to Wifi...\n");
  if (wifiMgr.connect(this->getSSID(), this->getWifiPassword()))
  {
    sserial.printf("Connected!\n");
  }
  else
  {
    sserial.printf("Could not connect :/\nCheck SSID and password.\n");
  }
}

void MENU::dumpWiFiStatus(char c)
{
  sserial.printf("%c\n\n", c);
  if (!wifiMgr.isConnected())
  {
    sserial.printf("Wifi not connected\n");
  }
  else
  {
    sserial.printf("Wifi is connected:\n");
    sserial.printf("  IP Address  %s\n", wifiMgr.getIPAddress().c_str());
    sserial.printf("  Subnet Mask %s\n", wifiMgr.getSubnet().c_str());
    sserial.printf("  Gateway IP  %s\n", wifiMgr.getGateway().c_str());
    sserial.printf("  DNS Server  %s\n", wifiMgr.getDNS().c_str());
  }
}

bool MENU::setDefaultConfig()
{
  cfg[CFG_WIFI_SSID] = "your_wifi_ssid";
  cfg[CFG_WIFI_PASS] = "your_wifi_password";
  cfg[CFG_TNET_SERVER] = "server.com";
  cfg[CFG_TNET_USER] = "user";
  cfg[CFG_TNET_PASS] = "password";
  cfg[CFG_TNET_PROMPT] = "$";

  return (this->writeConfig());
}

bool MENU::writeConfig()
{
  char* json = (char*)ps_malloc(1024);
  if (!json)
  {
    Serial.printf("Could not allocate for writing config!\r\n");
    return (false);
  }

  memset(json, '\0', 1024);
  serializeJson(cfg, json, 1023);

  File cfgFile = LittleFS.open(CFG_FILENAME, FILE_WRITE);
  if (!cfgFile)
  {
    free (json);
    return (false);
  }

  size_t bytesWritten = cfgFile.write((uint8_t*)json, strlen(json));
  cfgFile.close();

  Serial.printf("Wrote %d bytes of config\r\n", bytesWritten);

  free (json);
  return (true);
}

/*****************************************************
 * Read a string from serial and convert to an integer
******************************************************/
int MENU::readInt(const char* prompt)
{
  this->readString(prompt);
  return ((int)strtol(inputstr, NULL, 10));
}

/*****************************************************
* Read a single char from serial
******************************************************/
char MENU::readChar(const char* prompt)
{
  static char c;

  sserial.printf(prompt);

  while (!sserial.available());

  c = sserial.read();

  return (c);
}

/*****************************************************
* Read a string
******************************************************/
const char* MENU::readString(const char* prompt)
{
  int posn = 0;

  sserial.printf(prompt);

  while (posn < INPUT_STR_LEN)
  {
    while (!sserial.available());

    char c = sserial.read();
    if (c == '\r')
    {
      break;
    }
    else if (c == '\b' || c == 0x7f)
    {
      if (posn)
      {
        sserial.printf("\b \b");
        --posn;
      }
    }
    else
    {
      if (isprint(c))
      {
        inputstr[posn] = c;
        sserial.print(c);
        ++posn;
      }
      else
      {
        sserial.printf(" >>0x%2x<< ", c);
      }
    }
  }

  inputstr[posn] = '\0';

  sserial.printf("\r\n");

  return (inputstr);
}

void MENU::sendStr(const char* fmt, ...)
{
  size_t size = 1024;
  va_list ap;

  va_start(ap, fmt);

  ssize_t n = vsnprintf(slowStr, size, fmt, ap);
  va_end (ap);

  if (n > -1 && n < size)
  {
    for (size_t ii = 0; ii < strlen(slowStr); ++ii)
    {
      sserial.printf("%c", slowStr[ii]);
      delay(1);
    }
  }
}


