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
