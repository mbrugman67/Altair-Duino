#include <WiFi.h>

#include "wifimgr.h"
#include "globals.h"

bool WIFIMGR::connect(const char *ssid, const char *pass)
{
  uint8_t attempt = 0;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  //WiFi.hostname("ESP32Telnet");

  WiFi.begin(ssid, pass);

  while (!isConnected() && (attempt < 10))
  {
    delay(500);
    ++attempt;
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  return(this->isConnected());
}

bool WIFIMGR::isConnected()
{
  return (WiFi.status() == WL_CONNECTED);
}

String WIFIMGR::getIPAddress()
{
  if (!this->isConnected())
  {
    return (String(""));
  }
  else
  {
    return (WiFi.localIP().toString());
  }
}

String WIFIMGR::getGateway()
{
  if (!this->isConnected())
  {
    return (String(""));
  }
  else
  {
    return (WiFi.gatewayIP().toString());
  }
}

String WIFIMGR::getSubnet()
{
  if (!this->isConnected())
  {
    return (String(""));
  }
  else
  {
    return (WiFi.subnetMask().toString());
  }
}

String WIFIMGR::getDNS()
{
  if (!this->isConnected())
  {
    return (String(""));
  }
  else
  {
    return (WiFi.dnsIP().toString());
  }
}

String WIFIMGR::getHostname()
{
  if (!this->isConnected())
  {
    return (String(""));
  }
  else
  {
    return (String(WiFi.getHostname()));
  }
}
