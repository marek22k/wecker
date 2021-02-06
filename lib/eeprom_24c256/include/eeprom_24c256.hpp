
/*
    Copyright (C) 2020 Marek Küthe

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef EEPROM_24C256
#define EEPROM_24C256

#include <Arduino.h>
#include <Wire.h>

class eeprom_24c256
{
  protected:
     uint8_t device_address;

  public:
    eeprom_24c256(uint8_t);
    void init();

    bool write(unsigned address, byte data);
    bool read(unsigned address, byte * data);
    bool update(unsigned address, byte data);
    
    bool write(unsigned address, const byte * data, size_t len);
    bool read(unsigned address, byte * data, size_t len);
    bool update(unsigned address, const byte * data, byte * buffer, size_t len, unsigned long delay_time);
    bool update(unsigned address, const byte * data, size_t len, unsigned long delay_time);
};

#endif
