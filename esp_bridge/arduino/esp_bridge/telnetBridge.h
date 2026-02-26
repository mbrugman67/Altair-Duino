#ifndef TN_BRIDGE_H_
#define TN_BRIDGE_H_

#include "TelnetClient.h"

class TelnetBridge
{
public:
  TelnetBridge() {}
  ~TelnetBridge() {}

  void begin();
  bool connect(const char* server, const char* user, const char* pass);
  bool update(bool terminate);
  bool isConnected();
  void setPromptChar(char c);

private:
  telnetClient tc;

  char* inbuf;
  bool connected;
  size_t bufPosn;
};

#endif // TN_BRIDGE_H_