/**
 * The MIT License (MIT)
 * Copyright (c) 2015 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "NTPClient.h"

#define DEBUG_Client 1

#define SEVENZYYEARS 2208988800UL

namespace NTPClient {
Client::Client(UDP &udp) { this->_udp = &udp; }

Client::Client(UDP &udp, const char *poolServerName) {
  this->_udp = &udp;
  this->_poolServerName = poolServerName;
}

Client::Client(UDP &udp, const char *poolServerName, int updateInterval) {
  this->_udp = &udp;
  this->_poolServerName = poolServerName;
  this->_updateInterval = updateInterval;
}

void Client::setUpdateInterval(int updateInterval) {
  this->_updateInterval = updateInterval;
}

void Client::begin() { this->begin(Client::defaultLocalPort); }

void Client::begin(unsigned short port) {
  this->_port = port;

  this->_udp->begin(this->_port);

  this->_udpSetup = true;

  #ifdef DEBUG_Client
      Serial.println("Client: Started on port" + String(this->_port));
  #endif
}

void Client::asyncUpdate(bool force) {
  // skip if already waiting for update
  if (this->isUpdating()) {
    this->processAsyncUpdate();
    return;
  }

  // Update after _updateInterval, if no update or if forced update
  if ((millis() - this->_lastUpdate >= this->_updateInterval) ||
      this->_lastUpdate == 0 || force) {

    if (!this->_udpSetup)
      this->begin(); // setup the UDP client if needed

#ifdef DEBUG_Client
    Serial.println("Client: Update requested");
#endif

    this->sendNTPPacket();

    this->_updateStart = millis();
  }
}

bool Client::isUpdating() { return this->_updateStart != 0; }

/**
 * Process asynchronous update. Call this function from the loop()
 */
bool Client::processAsyncUpdate() {
  // if not waiting for update - skip
  if (!this->isUpdating()) {
    return false;
  }

  // check for timeout
  int waitTime = millis() - this->_updateStart;
  if (waitTime > this->_timeout) {
#ifdef DEBUG_Client
    Serial.println("Client: timeout");
#endif
    // timeout, cancel current update
    this->_lastUpdate = this->_updateStart;
    this->_updateStart = 0;
    return false;
  }

  // try to receive UDP packet
  int cb = this->_udp->parsePacket();
  if (cb == 0) {
    return false;
  }

  //
  this->_udp->read(this->_packetBuffer, Client::packetSize);

  unsigned long highWord =
      word(this->_packetBuffer[40], this->_packetBuffer[41]);
  unsigned long lowWord =
      word(this->_packetBuffer[42], this->_packetBuffer[43]);

  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;

  //
  this->_lastUpdate = this->_updateStart;
  this->_updateStart = 0;
  this->_currentEpoc = secsSince1900 - SEVENZYYEARS;

#ifdef DEBUG_Client
  Serial.println("Client: Update received:");
  Serial.println(String("Client: ") + this->_currentEpoc);
#endif

  return true;
}

unsigned long Client::getEpochTime() {
  return this->_currentEpoc + // Epoc returned by the NTP server
         ((millis() - this->_lastUpdate) / 1000); // Time since last update
}

int Client::getDay() {
  return (((this->getEpochTime() / 86400L) + 4) % 7); // 0 is Sunday
}

int Client::getHours() { return ((this->getEpochTime() % 86400L) / 3600); }

int Client::getMinutes() { return ((this->getEpochTime() % 3600) / 60); }

int Client::getSeconds() { return (this->getEpochTime() % 60); }

void Client::end() {
  this->_udp->stop();

  this->_udpSetup = false;
}

void Client::sendNTPPacket() {
  // set all bytes in the buffer to 0
  memset(this->_packetBuffer, 0, Client::packetSize);

  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  this->_packetBuffer[0] = 0b11100011; // LI, Version, Mode
  this->_packetBuffer[1] = 0;          // Stratum, or type of clock
  this->_packetBuffer[2] = 6;          // Polling Interval
  this->_packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  this->_packetBuffer[12] = 49;
  this->_packetBuffer[13] = 0x4E;
  this->_packetBuffer[14] = 49;
  this->_packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  // NTP requests are to port 123
  this->_udp->beginPacket(this->_poolServerName, 123);
  this->_udp->write(this->_packetBuffer, Client::packetSize);
  this->_udp->endPacket();
}

unsigned long Client::getLastUpdate() { return this->_lastUpdate; }

/**
 *
 */
String TimeFormatter::getShortTime(unsigned long epochTime) {
  unsigned long hours = (epochTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (epochTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  return hoursStr + ":" + minuteStr;
}

} // namespace NTPClient
