#pragma once

// #define DEBUG_NTPClient

#include "Arduino.h"

#include <Udp.h>

namespace NTPClient {

class Client {

public:
  const char *_poolServerName = "pool.ntp.org"; // Default time server

private:
  //
  static const unsigned int packetSize = 48;
  static const unsigned short defaultLocalPort = 137;

  // networking
  UDP *_udp = NULL;
  bool _udpSetup = false;
  unsigned short _port = defaultLocalPort;
  byte _packetBuffer[packetSize] = {0};

  //
  unsigned int _updateInterval = 60000; // In ms

  unsigned long _updateStart = 0;
  unsigned long _lastUpdate = 0;  // In ms
  unsigned long _timeout = 1000;  // In ms
  unsigned long _currentEpoc = 0; // In s, UTC

public:
  Client(UDP &udp);
  Client(UDP &udp, const char *poolServerName);
  Client(UDP &udp, const char *poolServerName, int updateInterval);

  /**
   * Starts the underlying UDP client with the default local port
   */
  void begin();

  /**
   * Starts the underlying UDP client with the specified local port
   */
  void begin(unsigned short port);

  /**
   * Stops the underlying UDP client
   */
  void end();

  /**
   * This should be called in the main loop of your application. By default an
   * update from the NTP Server is only made every 60 seconds. This can be
   * configured in the NTPClient constructor.
   *
   * @return true on success, false on failure
   */
  void asyncUpdate(bool force = false);

  /**
   * @return true if currently updating
   */
  bool isUpdating();

  /**
   * Get last update timestamp
   *
   * @return last update timestamp
   */
  unsigned long getLastUpdate();

  /**
   * Set the update interval to another frequency. E.g. useful when the
   * timeOffset should not be set in the constructor
   *
   * @param updateInterval update interval in milliseconds
   */
  void setUpdateInterval(int updateInterval);

  /**
   * @return time in seconds since Jan. 1, 1970
   */
  unsigned long getEpochTime();

  int getDay();
  int getHours();
  int getMinutes();
  int getSeconds();

private:
  bool processAsyncUpdate();
  void sendNTPPacket();
};

class TimeFormatter {
public:
  /**
   * @return time formatted `hh:mm`
   */
  static String getShortTime(unsigned long time);
};

} // namespace NTPClient
