#pragma once

// #define DEBUG_NTPClient

#include "Arduino.h"

#include <Udp.h>

#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337

class NTPClient {
private:
  const char *_poolServerName = "pool.ntp.org"; // Default time server

  UDP *_udp = NULL;
  bool _udpSetup = false;

  int _port = NTP_DEFAULT_LOCAL_PORT;

  int _timeOffset = 0; // In s

  unsigned int _updateInterval = 60000; // In ms

  unsigned long _currentEpoc = 0; // In s

  unsigned long _updateStart = 0;
  unsigned long _lastUpdate = 0; // In ms
  unsigned long _timeout = 1000; // In ms

  byte _packetBuffer[NTP_PACKET_SIZE] = {0};

public:
  // time formatting enums
  enum TimeFormat {
    TimeFormatShort, // hh:mm
    TimeFormatLong   // hh:mm:ss
  };

public:
  NTPClient(UDP &udp);
  NTPClient(UDP &udp, int timeOffset);
  NTPClient(UDP &udp, const char *poolServerName);
  NTPClient(UDP &udp, const char *poolServerName, int timeOffset);
  NTPClient(UDP &udp, const char *poolServerName, int timeOffset,
            int updateInterval);

  /**
   * Starts the underlying UDP client with the default local port
   */
  void begin();

  /**
   * Starts the underlying UDP client with the specified local port
   */
  void begin(int port);

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
   * This will force the update from the NTP Server.
   *
   * @return true on success, false on failure
   */
  // bool forceUpdate();

  int getDay();
  int getHours();
  int getMinutes();
  int getSeconds();

  /**
   * Changes the time offset. Useful for changing timezones dynamically
   *
   * @param timeOffset offset in seconds
   */
  void setTimeOffset(int timeOffset);

  /**
   * Set the update interval to another frequency. E.g. useful when the
   * timeOffset should not be set in the constructor
   *
   * @param updateInterval update interval in milliseconds
   */
  void setUpdateInterval(int updateInterval);

  /**
   * @return time formatted like `hh:mm:ss` or `hh:mm`
   */
  String getFormattedTime(TimeFormat mode = TimeFormat::TimeFormatLong);

  /**
   * @return time in seconds since Jan. 1, 1970
   */
  unsigned long getEpochTime();

  /**
   * Stops the underlying UDP client
   */
  void end();

private:
  /**
   * Process async update
   */
  bool processAsyncUpdate();

  void sendNTPPacket();
};
