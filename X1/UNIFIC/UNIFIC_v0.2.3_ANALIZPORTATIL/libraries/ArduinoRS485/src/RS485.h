/*
  This file is part of the ArduinoRS485 library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _RS485_H_INCLUDED
#define _RS485_H_INCLUDED

#include <Arduino.h>

#ifdef PIN_SERIAL1_TX
#define RS485_DEFAULT_TX_PIN PIN_SERIAL1_TX
#else
#define RS485_DEFAULT_TX_PIN 1 
#endif

#ifdef __AVR__
#define RS845_DEFAULT_DE_PIN 2
#define RS845_DEFAULT_RE_PIN -1
#else
#define RS845_DEFAULT_DE_PIN 26
#define RS845_DEFAULT_RE_PIN 26
#endif

class RS485Class : public Stream {
  public:
    RS485Class(HardwareSerial& hwSerial, int txPin, int dePin, int rePin);

    virtual void begin(unsigned long baudrate);
    virtual void begin(unsigned long baudrate, uint16_t config);
    virtual void end();
    virtual int available();
    virtual int peek();
    virtual int read(void);
    virtual void flush();
    virtual size_t write(uint8_t b);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    virtual operator bool();

    void beginTransmission();
    void endTransmission();
    void receive();
    void noReceive();

    void sendBreak(unsigned int duration);
    void sendBreakMicroseconds(unsigned int duration);

    void setPins(int txPin, int dePin, int rePin);

  private:
    HardwareSerial* _serial;
    int _txPin;
    int _dePin;
    int _rePin;

    bool _transmisionBegun;
    unsigned long _baudrate;
    uint16_t _config;
};

extern RS485Class RS485;

#endif
