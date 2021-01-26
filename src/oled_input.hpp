#ifndef OLED_INPUT_HPP
#define OLED_INPUT_HPP

#include <stddef.h>
#include <U8x8lib.h>

#define OLED_INPUT_LETTERS_START_X 5

class oled_input {

  public:
    typedef unsigned char pin;

  protected:
    const char * letters; // = "<ABCDEFGHIJKLMNOPQRSTUVWXYZ>";
    const size_t letters_size; // = 28;
    char * input; // = "";
    size_t pos; // = 0;
    U8X8_SSD1306_128X64_NONAME_HW_I2C & oled;
    pin meter_pin;
    pin button_pin;
    

    void print_three_letters(char, char, char);

  public:
    oled_input(pin, pin, const char *, size_t, char *, U8X8_SSD1306_128X64_NONAME_HW_I2C &);
    void input_loop();
  
};

#endif
