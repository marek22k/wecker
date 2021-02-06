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

#include "segment_display.hpp"

segment_display::segment_display(TM1637Display * display, int number_1, int number_2, uint8_t light, bool colon, bool on)
  : display(display), colon(colon), current_light(light), last_light(light), on(on)
{
  this->current_number[0] = number_1;
  this->current_number[1] = number_2;
  old_number[0] = 0;
  old_number[1] = 0;
  this->refresh_display();
}

void segment_display::set_number(int number, size_t part)
{
  this->current_number[part] = number;
}

void segment_display::set_light(uint8_t light)
{
  this->current_light = light;
}

void segment_display::set_colon(bool show)
{
  this->colon = show;
}

int segment_display::get_number(size_t part)
{
  return this->current_number[part];
}

uint8_t segment_display::get_light()
{
  return this->current_light;
}

bool segment_display::get_colon()
{
  return this->colon;
}

void segment_display::refresh_display()
{
  if (
    this->last_light != this->current_light ||
    this->last_on != this->on ||
    this->old_number[0] != this->current_number[0] ||
    this->old_number[1] != this->current_number[1]
  )
  {
    this->old_number[0] = this->current_number[0];
    this->old_number[1] = this->current_number[1];
    this->last_light = this->current_light;
    this->last_on = this->on;
    this->display->setBrightness(this->current_light, this->on);
    this->display->showNumberDecEx(this->current_number[0], (this->colon ? 64 : 0), false, 2, 0);
    this->display->showNumberDecEx(this->current_number[1], (this->colon ? 64 : 0), false, 2, 2);
  }
}

void segment_display::turn_on()
{
    this->on = true;
}

void segment_display::turn_off()
{
    this->on = true;
}

void segment_display::turn()
{
    this->on = ! this->on;
}
