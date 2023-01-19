// 
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


// Declare a TuneSet. This is a generic class. We specify `<>` to use the
// default template parameters, which should work well for most use cases.
TuneSet<> tuning;

// These are the variables we will be tuning.
float kp = 0.1;
float kd = 0.1;
float target = 0;

// Simulation variables.
float x;              // x-displacement.
float vx;             // x-velocity.
const float dt = 0.1; // Time interval between simulation steps, in secons.
uint32_t start_time = 0;
bool running = true;


void setup()
{
    // Add our variables to the tuning set.
    tuning.add("kp", kp);
    tuning.add("kd", kd);
    tuning.add("tar", target);

    // Start the serial port.
    Serial.begin(115200);

    x = 10;
    start_time = millis();
}

// Controller function. Assume we only have access to x, target, and vx; and we only output the next vx.
float control()
{
    float err = x - target;
    float vx_new = -kp * err + kd * vx;
    return vx_new;
}

// Model the PD system.
void sim()
{
    vx = control();
    x += vx * dt;
}

bool arrived()
{
    return abs(x - target) < 0.00001;
}

void loop()
{
    // This handler receives data from Serial, parses it, then prints or assigns the correct variable.
    // It also does other things, depending on the settings and options defined in your tuning profile.
    tuning.readSerial();

    static uint32_t lt = 0;
    if (millis() - lt > dt * 1000) {
        if (running) {
            sim();

            // Print out our tuning and simulation variables.
            char buffer[256] = {0};
            snprintf(buffer, sizeof(buffer), "kp:%.3f,kd:%.3f,tar:%.5f,x:%.5f,dx:%.5f", kp, kd, target, x, vx);
            Serial.println(buffer);

            if (arrived()) {
                // Finished!
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
