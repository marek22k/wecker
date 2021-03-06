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

#include "eeprom_24c256.hpp"

eeprom_24c256::eeprom_24c256(uint8_t device_address)
  : device_address(device_address)
{
  
}

void eeprom_24c256::init()
{
  Wire.begin();
}

bool eeprom_24c256::write(unsigned address, byte data)
{
  Wire.beginTransmission(this->device_address);

  Wire.write( address >> 8 );
  Wire.write( address & 0xFF );
  
  Wire.write(data);

  byte transmission_status = Wire.endTransmission();

  bool return_status = (transmission_status == 0 ? true : false);

  return return_status;
}

bool eeprom_24c256::read(unsigned address, byte * data)
{
  Wire.beginTransmission(this->device_address);

  Wire.write( address >> 8 );
  Wire.write( address & 0xFF );

  byte transmission_status = Wire.endTransmission();
  if (transmission_status != 0)
    return false;

  Wire.requestFrom(this->device_address, (size_t) 2);

  if (! Wire.available())
    return false;
  
  *data = Wire.read();

  return true;
}

bool eeprom_24c256::update(unsigned address, byte data)
{
  byte previous_data;
  bool read_status = this->read(address, &previous_data);
  if (! read_status)
    return false;

  if (previous_data == data)
    return true;
  
  return this->write(address, data);
}

bool eeprom_24c256::write(unsigned address, const byte * data, size_t len)
{
  Wire.beginTransmission(this->device_address);

  Wire.write( address >> 8 );
  Wire.write( address & 0xFF );

  for (size_t i = 0; i < len; i++)
    Wire.write(data[i]);

  byte transmission_status = Wire.endTransmission();

  bool return_status = (transmission_status == 0 ? true : false);

  return return_status;
}

bool eeprom_24c256::read(unsigned address, byte * data, size_t len)
{
  Wire.beginTransmission(this->device_address);

  Wire.write( address >> 8 );
  Wire.write( address & 0xFF );

  byte transmission_status = Wire.endTransmission();
  if (transmission_status != 0)
    return false;

  Wire.requestFrom(this->device_address, (size_t) len);

  for (size_t i = 0; i < len; i++)
  {
    if (! Wire.available() )
      return false;
    
    data[i] = Wire.read();
  }

  return true;
}

bool eeprom_24c256::update(unsigned address, const byte * data, byte * buffer, size_t len, unsigned long delay_time)
{
  bool read_status = this->read(address, buffer, len);
  if (! read_status)
    return false;

  for (size_t i = 0; i < len; i++)
  {
    if (buffer[i] != data[i])
    {
      bool write_status = this->write(address + i, data[i]);
      if (! write_status)
        return false;
      delay(delay_time);
    }
  }

  return true;
}

bool eeprom_24c256::update(unsigned address, const byte * data, size_t len, unsigned long delay_time)
{
  byte * buffer = new byte[len];
  bool return_status = this->update(address, data, buffer, len, delay_time);
  delete[] buffer;

  return return_status;
}
