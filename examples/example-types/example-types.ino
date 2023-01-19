// 
// Serial Tuning example of working with various tuning types.
// 

#include "tuning.h"

#include <Arduino.h>
#include <stdio.h>


uint8_t u8;
int16_t i16;
uint32_t u32;
int64_t i64;
float f;
double d;
String s;

TuneSet<> tuning;

void setup()
{
    // Add our tuning variables.
    // We can use the `TUNE` macro to add tuning variables with the same labels as the variable names.
    // tuning.add("u8", u8); // Equivalent to below.
    tuning.TUNE(u8);
    tuning.TUNE(i16);
    tuning.TUNE(u32);
    tuning.TUNE(i64);
    tuning.TUNE(f);
    tuning.TUNE(d);
    tuning.TUNE(s);

    Serial.begin(115200);
}

void loop()
{
    // Handle serial commands.
    tuning.readSerial();

    char buffer[256] = {0};
    snprintf(buffer, sizeof(buffer), "u8:%hhu,i16:%hd,u32:%lu,i64:%lld,f:%.9f,d:%.9lf,s:%s", u8, i16, u32, i64, f, d, s.c_str());
    Serial.println(buffer);

    delay(200);
}
