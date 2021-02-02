#ifndef SEGMENT_DISPLAY_HPP
#define SEGMENT_DISPLAY_HPP

#include <Arduino.h>
#include <TM1637Display.h>

class segment_display
{
  public:
    typedef unsigned char pin;
  
  protected:
      TM1637Display * display;
      volatile int current_number[2];
      int old_number[2];
      volatile bool colon;
      uint8_t current_light;
      uint8_t last_light;
      bool on;
      bool last_on;

  public:
    segment_display(TM1637Display *, int, int, uint8_t, bool = false, bool = true);
    void set_number(int, size_t);
    void set_light(uint8_t);
    void set_colon(bool);
    int get_number(size_t);
    uint8_t get_light();
    bool get_colon();
    void refresh_display();
    void turn_off();
    void turn_on();
    void turn();
};

#endif
