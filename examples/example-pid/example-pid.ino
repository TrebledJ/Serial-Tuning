// PD-controller example. This simulates a moving body chasing a point on a single-dimension, frictionless surface.
//
// Try to tune the system so that the body reaches the target in the shortest time.
//
// In the serial monitor, you can type stuff like:
//      tar=10          Set the target to 10.
//      kp=2            Set the P gain to 2.
//      kd=0.01         Set the D gain to 0.01.
//
// You can "reset" the system by changing the target.
//

#include "tuning.h"

#include <Arduino.h>
#include <stdio.h>


float kp = 0.1;
// float ki;
float kd = 0.1;

float target = 0;
float x;
float vx;
const float dt = 0.1;

uint32_t start_time = 0;
bool running = true;

TuneSet<> tuning;

void setup()
{
    tuning.add("kp", kp);
    // tuning.add("ki", ki);
    tuning.add("kd", kd);
    tuning.add("tar", target);

    Serial.begin(115200);

    x = 10;
    start_time = millis();
}

float control()
{
    float err = x - target;
    float vx_new = -kp * err + kd * vx;
    return vx_new;
}

bool arrived()
{
    return abs(x - target) < 0.00001;
}

void sim()
{
    vx = control();
    x += vx * dt;
}

void loop()
{
    tuning.readSerial();

    static uint32_t lt = 0;
    if (millis() - lt > dt * 1000) {
        if (running) {
            sim();

            char buffer[256] = {0};
            snprintf(buffer, sizeof(buffer), "kp:%.3f,kd:%.3f,x:%.5f,dx:%.5f", kp, kd, x, vx);
            Serial.println(buffer);

            if (arrived()) {
                Serial.printf("finished in %.3f seconds\n", (millis() - start_time) / 1000.0);
                running = false;
            }

        } else {
            if (!arrived()) {
                // Starting...
                start_time = millis();
                running = true;
            }
        }
        lt = millis();
    }
}
