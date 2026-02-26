#ifndef MENU_H_
#define MENU_H_

#include <ArduinoJson.h>
#include "LittleFS.h"

#define INPUT_STR_LEN 80

class MENU
{
public:
  MENU() {}
  ~MENU() {}

  bool begin();
  void update();

  const char* getSSID();
  const char* getWifiPassword();
  const char* getTelnetServer();
  const char* getTelnetUser();
  const char* getTelnetPassword();
  char getTelnetPrompt();

  void showMenu();

private:
  bool writeConfig();
  bool setDefaultConfig();
  void setSSID(char c);
  void setWifiPassword(char c);
  void setTelnetServer(char c);
  void setTelnetUser(char c);
  void setTelnetPassword(char c);
  void setTelnetPrompt(char c);
  void connectToTelnet(char c);
  void dumpConfig(char c);
  void dumpWiFiStatus(char c);
  void connectToWiFi(char c);

  int readInt(const char* prompt);
  char readChar(const char* prompt);
  const char* readString(const char* prompt);
  
  void sendStr(const char* fmt, ...);

  JsonDocument cfg;

  char inputstr[INPUT_STR_LEN + 1];
  char* slowStr;
};

#endif // MENU_H_
