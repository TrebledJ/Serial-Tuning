#include "tuning.h"

#include <Arduino.h>
#include <stdio.h>


uint8_t a;
int16_t b;
uint32_t c;
int64_t d;
float e;
double f;

TuneSet<> tuning;

void setup()
{
    tuning.add("u8", a);
    tuning.add("i16", b);
    tuning.add("u32", c);
    tuning.add("i64", d);
    tuning.add("f", e);
    tuning.add("d", f);

    Serial.begin(115200);
}

void loop()
{
    tuning.readSerial();

    char buffer[256] = {0};
    snprintf(buffer, sizeof(buffer), "u8:%hhu,i16:%hd,u32:%lu,i64:%lld,f:%.9f,d:%.9lf", a, b, c, d, e, f);
    Serial.println(buffer);

    delay(200);
}
