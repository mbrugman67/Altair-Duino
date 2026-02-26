#ifndef WIFI_MGR_H_
#define WIFI_MGR_H_

class WIFIMGR
{
public:
  WIFIMGR() {}
  ~WIFIMGR() {}

  bool connect(const char* ssid, const char* pass);
  bool isConnected();

  String getIPAddress();
  String getGateway();
  String getSubnet();
  String getDNS();
  String getHostname();

private:

};

#endif // WIFI_MGR_H_
