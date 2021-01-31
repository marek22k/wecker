
#include <Arduino.h>

#include "TM1637Display.h"
#include "segment_display.hpp"
#include "PCF85063TP.h"
#include "eeprom_24c256.hpp"
#include "DHT.h"
#include "U8x8lib.h"
#include "oled_input.hpp"

/* pins */
#define BUTTONS_ROW_1 A1
#define BUTTONS_ROW_2 A0

#define SEGMENT_DISPLAY_DIO A6
#define SEGMENT_DISPLAY_CLK A7

#define TEMP_HUMI A3

#define GREEN_LED 13
#define RED_LED 2

#define BUTTON_TIMEOUT 500

/* I2C addresses */
#define EEPROM_ADDRESS 0b1010001

/* enums */
enum BUTTON {
    /*0*//*1*//* 2 */   /* 3  */    /*4*/ /*5*/ /* 6 */
    CFK, NEXT, OK_EDIT, CHANGE_MODE, PLUS, MINUS, NONE
};

enum MODE {
    MAIN, SET_TIME, ALARM_CLOCK
};

enum UNIT_OF_TEMPERATURE {
    CELSIUS, FAHRENHEIT, KELVIN
};

enum TIME_EDIT {
    HOUR, MINUTE, COMPLETE
};

/* functions */
BUTTON read_buttons();
void next_mode();
void next_temperature_unit();
void mode_main(BUTTON);
void mode_set_time(BUTTON);
void update_temp_humi();
void change_to_main_mode();
void change_red_led_state();
void get_time_from_user(unsigned *, unsigned *, char = ':', int = 23, int = 59);
void get_seconds_from_user(unsigned *, unsigned = 5, int = 59, unsigned = 0);
void get_date_from_user(unsigned *, unsigned *);
void get_year_from_user(unsigned *, unsigned *);
void get_day_from_user(unsigned *);

/* variables */
MODE current_mode = MAIN;
UNIT_OF_TEMPERATURE unit_for_temperature = CELSIUS;
float dht_result[2] = {0, 0};
unsigned long th_update_intervall = 500;  /* temp humi update intervall */
unsigned long th_timestamp = 0;
bool led_red_state = false;

/* objects */
TM1637Display tm1637_display(SEGMENT_DISPLAY_CLK, SEGMENT_DISPLAY_DIO);
segment_display secondary_display(&tm1637_display, 88, 88, 0, true);
PCD85063TP rtc;
eeprom_24c256 eeprom(EEPROM_ADDRESS);
DHT dht(TEMP_HUMI, DHT11);
U8X8_SSD1306_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE);

/*
 * eeprom notes
 * 0 - Clock 1 - Hour
 * 1 - Clock 1 - Minute
 * 2 - Clock 2 - Hour
 * 3 - Clock 2 - Minute
 *
*/

void setup()
{
    Serial.begin(9600);
    Serial.println("Hello!");
    
    pinMode(BUTTONS_ROW_1, INPUT);
    pinMode(BUTTONS_ROW_2, INPUT);
  
    pinMode(SEGMENT_DISPLAY_DIO, OUTPUT);
    pinMode(SEGMENT_DISPLAY_CLK, OUTPUT);
    
    pinMode(GREEN_LED, OUTPUT);
    digitalWrite(GREEN_LED, LOW);
    
    pinMode(RED_LED, OUTPUT);
    digitalWrite(RED_LED, HIGH);
    
    rtc.begin();
    eeprom.init();
    dht.begin();

    oled.setBusClock(100000);
    oled.begin();
    oled.setFont(u8x8_font_victoriamedium8_r);
    
    update_temp_humi();
    th_timestamp = millis();
}

void loop()
{
    BUTTON pressed_button = read_buttons();
    
    if (pressed_button != NONE)
    {
        digitalWrite(GREEN_LED, HIGH);
    }
    
    change_red_led_state();
    
    if ( (th_timestamp - millis()) >= th_update_intervall && current_mode == MAIN )
    {
        update_temp_humi();
        th_timestamp = millis();
    }
    rtc.getTime();
    
    /* code */
    if (pressed_button == CHANGE_MODE)
    {
        next_mode();
    }
    
    if (current_mode == MAIN)
    {
        mode_main(pressed_button);
    }
    else if (current_mode == SET_TIME)
    {
        mode_set_time(pressed_button);
    }
    
    secondary_display.set_number(rtc.hour, 0);
    secondary_display.set_number(rtc.minute, 1);
    secondary_display.refresh_display();
    
    if (pressed_button != NONE)
    {
        digitalWrite(GREEN_LED, LOW);
        unsigned long timestamp = millis();
        while ( (millis() - timestamp) <= BUTTON_TIMEOUT)
        {
            /* code while wait */
        }
    }
}

BUTTON read_buttons()
{
    int row_1 = analogRead(BUTTONS_ROW_1);
    int row_2 = analogRead(BUTTONS_ROW_2);
    
    /* eval row 1 */
    if (row_1 >= 670 && row_1 <= 683)
    {
        return OK_EDIT;
    }
    else if (row_1 >= 85 && row_1 <= 100)
    {
        return NEXT;
    }
    else if (row_1 >= 990 && row_1 <= 1010)
    {
        return CFK;
    }
    
    /* eval row 2 */
    if (row_2 >= 670 && row_2 <= 683)
    {
        return MINUS;
    }
    else if (row_2 >= 85 && row_2 <= 100)
    {
        return PLUS;
    }
    else if (row_2 >= 990 && row_2 <= 1010)
    {
        return CHANGE_MODE;
    }
    
    return NONE;
}

inline void next_mode()
{
    if (current_mode == ALARM_CLOCK)
    {
        current_mode = MAIN;
        change_to_main_mode();
    }
    else
    {
        current_mode = static_cast<MODE>( static_cast<short>(current_mode) + 1);
    }
    oled.clear();
}


inline void next_temperature_unit()
{
    if (unit_for_temperature == KELVIN)
        unit_for_temperature = CELSIUS;
    else
        unit_for_temperature = static_cast<UNIT_OF_TEMPERATURE>( static_cast<short>(unit_for_temperature) + 1);
}


void mode_main(BUTTON pressed_button)
{
    /* reactions on buttons */
    if (pressed_button == PLUS || pressed_button == MINUS)
    {
        /* Dimming the display */
        uint8_t light = secondary_display.get_light();
        light += (pressed_button == MINUS ? -1 : +1);
        if ( ! (light > 7) )
        {
            secondary_display.set_light(light);
        }
    }
    else if (pressed_button == OK_EDIT)
    {
        /* Change display state (on/off) */
        secondary_display.turn();
    }
    else if (pressed_button == NEXT)
    {
        update_temp_humi();
    }
    else if (pressed_button == CFK)
    {
        next_temperature_unit();
    }
    
    /* print time and date to the oled display */
    oled.setFont(u8x8_font_profont29_2x3_r);
    oled.setCursor(0, 0);
    /* print hours */
    if (rtc.hour < 10)
        oled.print("0");
    oled.print(rtc.hour);
    
    oled.print(":");
    
    /* print minutes */
    if (rtc.minute < 10)
        oled.print("0");
    oled.print(rtc.minute);

    oled.setFont(u8x8_font_7x14B_1x2_r);
    oled.setCursor(10, 1);
    /* print seconds */
    oled.print(":");
    if (rtc.second < 10)
        oled.print("0");
    oled.print(rtc.second);
    oled.print(" ");

    /* print date */
    oled.setCursor(1, 3);
    
    /* print dayOfMonth */
    if (rtc.dayOfMonth < 10)
        oled.print("0");
    oled.print(rtc.dayOfMonth);
    
    oled.print("/");
    
    /* print month */
    if (rtc.month < 10)
        oled.print("0");
    oled.print(rtc.month);
    
    /* print year */
    oled.print("/");
    oled.print(rtc.year + 1951);
    
    /* print dayOfWeek */
    oled.print(" ");
    switch(rtc.dayOfWeek)
    {
        case MON:
            oled.print("MON");
            break;
        case TUE:
            oled.print("TUE");
            break;
        case WED:
            oled.print("WED");
            break;
        case THU:
            oled.print("THU");
            break;
        case FRI:
            oled.print("FRI");
            break;
        case SAT:
            oled.print("SAT");
            break;
        case SUN:
            oled.print("SUN");
            break;
    }
    
    
    /* print temp and humi to the oled display */
    oled.setCursor(1, 6);
    oled.print(dht_result[0]);
    oled.print("%");
    oled.print(" ");
    
    if (unit_for_temperature == CELSIUS)
    {
        oled.print(dht_result[1]);
        oled.print("C");
    }
    else if (unit_for_temperature == FAHRENHEIT)
    {
        oled.print(dht_result[1] * 1.8 + 32);
        oled.print("F");
    }
    else if (unit_for_temperature == KELVIN)
    {
        oled.print(dht_result[1] + 273.15);
        oled.print("K");
    }
    oled.print(" ");
}

void mode_set_time(BUTTON pressed_button)
{
    oled.setFont(u8x8_font_7x14B_1x2_r);
    oled.setCursor(0, 1);
    oled.print(" Would you like\n  to set the\n    clock?");
    
    if (pressed_button == OK_EDIT)
    {
        unsigned hour;
        unsigned minute;
        unsigned seconds;
        unsigned day_of_week = 0;
        unsigned mon;
        unsigned day;
        unsigned year;
        
        oled.clear();
        get_time_from_user(&hour, &minute);
        
        oled.clear();
        get_seconds_from_user(&seconds);
        
        oled.clear();
        get_day_from_user(&day_of_week);
        
        oled.clear();
        get_date_from_user(&mon, &day);
        
        oled.clear();
        get_year_from_user(&year);
        
        oled.clear();
        
        if (static_cast<int>(year) - 2000 < 0)
            year = 0;
        year -= 2000;
        
        if (mon == 0)
            mon = 1;
        
        if (day == 0)
            day = 1;
        
        rtc.stopClock();
        delay(10);
        rtc.fillByYMD(year, mon, day);
        delay(10);
        rtc.fillByHMS(hour, minute, seconds);
        delay(10);
        rtc.fillDayOfWeek(day_of_week);
        delay(10);
        
        oled.setFont(u8x8_font_7x14B_1x2_r);
        oled.setCursor(0, 1);
        oled.print("  Should the\n    time be\n   set now?");
        while(read_buttons() == NONE);
        rtc.startClock();
        delay(10);
        
        oled.clear();
    }
}

inline void update_temp_humi()
{
    dht.readTempAndHumidity(dht_result);
}

inline void change_to_main_mode()
{
    update_temp_humi();
}

inline void change_red_led_state()
{
    digitalWrite(RED_LED, (led_red_state ? HIGH : LOW));
    led_red_state = ! led_red_state;
}

void get_time_from_user(unsigned * hour, unsigned * minute, char colon, int max1, int max2)
{
    int tmp_hour = 0;
    int tmp_minute = 0;
    
    int edit_mode = 1;
    /*
     * -1 = complete
     * 1 = hour
     * 2 = minute
     */
    
    oled.setFont(u8x8_font_profont29_2x3_r);
    
    while (edit_mode != -1)
    {
        BUTTON pressed_button = read_buttons();
        
        if (pressed_button == NEXT)
        {
            edit_mode = (edit_mode == 1 ? 2 : 1);
        }
        else if (pressed_button == OK_EDIT)
        {
            edit_mode = -1;
        }
        else if (pressed_button == PLUS || pressed_button == MINUS)
        {
            unsigned val = (pressed_button == MINUS ? -1 : +1);
            if (edit_mode == 1)
            {
                tmp_hour += val;
                if (tmp_hour < 0 || tmp_hour > max1)
                    tmp_hour -= val;
            }
            else /* if (edit_mode == 2) */
            {
                tmp_minute += val;
                if (tmp_minute < 0 || tmp_minute > max2)
                    tmp_minute -= val;
            }
        }
            
        oled.setCursor(3, 2);
        if (edit_mode == 1)
            oled.inverse();
        
        if (tmp_hour < 10)
            oled.print("0");
        oled.print(tmp_hour);
        
        if (edit_mode == 1)
            oled.noInverse();
        
        oled.print(colon);
        
        if (edit_mode == 2)
            oled.inverse();
        
        if (tmp_minute < 10)
            oled.print("0");
        oled.print(tmp_minute);
        
        if (edit_mode == 2)
            oled.noInverse();
        
        if (pressed_button != NONE)
            delay(BUTTON_TIMEOUT);
    }
    
    * hour = tmp_hour;
    * minute = tmp_minute;
}

void get_seconds_from_user(unsigned * seconds, unsigned startpos, int max, unsigned startval)
{
    int tmp_seconds = startval;
    
    bool complete = false;
    
    oled.setFont(u8x8_font_profont29_2x3_r);
    
    oled.inverse();
    while (! complete)
    {
        BUTTON pressed_button = read_buttons();
        
        if (pressed_button == OK_EDIT)
        {
            complete = true;
        }
        else if (pressed_button == PLUS || pressed_button == MINUS)
        {
            unsigned val = (pressed_button == MINUS ? -1 : +1);
            tmp_seconds += val;
            if (tmp_seconds < 0 || tmp_seconds > max)
                tmp_seconds -= val;
        }
            
        oled.setCursor(startpos, 2);
        
        if (tmp_seconds < 10)
            oled.print("0");
        oled.print(tmp_seconds);
        
        if (pressed_button != NONE)
            delay(BUTTON_TIMEOUT);
    }
    oled.noInverse();
    
    * seconds = tmp_seconds;
}

inline void get_year_from_user(unsigned * year)
{
    get_seconds_from_user(year, 3, 9999, 2000);
}

inline void get_date_from_user(unsigned * mon, unsigned * day)
{
    get_time_from_user(mon, day, '/', 12, 31);
}

void get_day_from_user(unsigned * day)
{
    int tmp_day = 1;
    
    bool complete = false;
    
    oled.setFont(u8x8_font_profont29_2x3_r);
    
    oled.inverse();
    while (! complete)
    {
        BUTTON pressed_button = read_buttons();
        
        if (pressed_button == OK_EDIT)
        {
            complete = true;
        }
        else if (pressed_button == PLUS || pressed_button == MINUS)
        {
            unsigned val = (pressed_button == MINUS ? -1 : +1);
            tmp_day += val;
            if (tmp_day < 0 || tmp_day > 6)
                tmp_day -= val;
        }
            
        oled.setCursor(3, 2);
        
        switch(tmp_day)
        {
            case MON:
                oled.print("MON");
                break;
            case TUE:
                oled.print("TUE");
                break;
            case WED:
                oled.print("WED");
                break;
            case THU:
                oled.print("THU");
                break;
            case FRI:
                oled.print("FRI");
                break;
            case SAT:
                oled.print("SAT");
                break;
            case SUN:
                oled.print("SUN");
                break;
        }
        
        if (pressed_button != NONE)
            delay(BUTTON_TIMEOUT);
    }
    oled.noInverse();
    
    * day = tmp_day;
}
