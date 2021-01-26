#include "oled_input.hpp"

oled_input::oled_input(pin meter_pin, pin button_pin, const char * letters, size_t letters_size, char * buffer, U8X8_SSD1306_128X64_NONAME_HW_I2C & oled)
  : letters(letters), letters_size(letters_size), input(buffer), pos(0), oled(oled), meter_pin(meter_pin), button_pin(button_pin)
{
  
}

void oled_input::print_three_letters(char right, char middle, char left)
{ 
  oled.setCursor(OLED_INPUT_LETTERS_START_X, 5);
  oled.print(right);
  
  oled.setCursor(OLED_INPUT_LETTERS_START_X + 2, 5);
  oled.inverse();
  oled.print(middle);
  oled.noInverse();
  
  oled.setCursor(OLED_INPUT_LETTERS_START_X + 4, 5);
  oled.print(left);
}

void oled_input::input_loop() {
  oled.clear();

  while (true) {
    size_t letter_n = map(analogRead(meter_pin), 0, 1023, 0, letters_size - 1);

    if (letter_n == 0)
    {
      print_three_letters(' ', letters[letter_n], letters[letter_n + 1]);
    }
    else if (letter_n == (letters_size - 1))
    {
      print_three_letters(letters[letter_n - 1], letters[letter_n], ' ');
    }
    else
    {
      print_three_letters(letters[letter_n - 1], letters[letter_n], letters[letter_n + 1]);
    }
  
    bool pressButton1 = (digitalRead(button_pin) == LOW ? false : true);
  
    if (pressButton1)
    {
      if (letter_n == (letters_size - 1))
      {
        break;
      }
      else if (letter_n == 0)
      {
        if (pos != 0)
        {
          pos--;
          input[pos] = '\0';
          oled.clearLine(3);
        }
      }
      else
      {
        input[pos] = letters[letter_n];
        input[pos + 1] = '\0';
        pos++;
      }
    }
  
    if (pos >= 12)
    {
      oled.setCursor(2, 3);
      oled.print(input + (pos - 12));
    }
    else
    {
      oled.setCursor(2, 3);
      oled.print(input);
    }
  
    if (pressButton1) {
      delay(250);
    }
  }

  oled.clearLine(5);
  oled.setCursor(9, 6);
  oled.print("...");

  pos = 0;
}
