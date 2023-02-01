# Serial Tuning

![Get some sense Robin!](docs/gesundheit.jpg)

## Features

* [x] Tune variables without restarting the program, using Arduino's serial monitor (`Serial`).
* [x] Tune a plethora of types: integers, floating-points, strings.
* [x] Works on boards based on the Arduino framework (e.g. ESP32).
* [x] Print variables.
* [x] Works with custom types.
* [x] Report bad input and parse errors through opt-in logging.
* [x] Parse commands from `String` input (doesn't necessarily use `Serial` RX).
* [x] Optional callback when a variable is set.


## Example

```cpp
// example-pid/example-pid.ino

// Declare our tuning variables! These are the things we'll be controlling over the Serial Monitor input.
float kp = 0.1; // Default values here will work as usual.
float kd = 0.1;
float target = 0;

// Declare a tuning set. This will store the variables we want to tune, along with their associated labels.
TuneSet<> tuning;

void setup()
{
    // Add our tuning variables.
    tuning.add("kp", kp); // Associate "kp" label with the `kp` variable.
    tuning.add("kd", kd);
    tuning.add("tar", target); // You can use different labels, as long as the labels don't contain spaces or the equal-sign.

    // Start the serial port. The baud rate is up to you, but it should agree on both ends.
    Serial.begin(115200);

    // ... more setup code ...
}

void loop()
{
    // Read and parse input. Variables will be updated afterward.
    tuning.readSerial();

    // ... more loopy code ...
}
```

You can test and play around with the following input:

```sh
kp=2        # Changes `kp` from 0.1 to 2.
tar=10      # Changes `target` from 0 to 10.
kp=0.001    # Changes `kp` from 2 to 0.001.
kp          # Prints back "kp=0.001000".
```

Check out more examples in [*examples*](examples).


### Callback Example

```cpp
// Declare some tuning variables in file scope.
float kp;

#define NUM_OPTIONS 20
uint8_t options[NUM_OPTIONS];

// Declare a TuneSet, as usual.
TuneSet<> tuneset;

// Define a callback function which takes a void* parameter.
void tuningOnUpdate(void* ptr) {
    if (ptr == &kp) {
        Serial.println("kp updated");
        // Save to SD?
        // Notify over WiFi?
        // Clamp within range?
    } else if (&options[0] <= ptr && ptr <= &options[NUM_OPTIONS - 1]) {
        Serial.println("option updated");
        // Do stuff.
    }
}

void setup() {
    // Add our tuning variables, as usual.
    tuneset.add("kp", kp);
    for (int i = 0; i < 20; i++) {
        tuneset.add(String("option") + i, &options[i]);
    }

    // Register the callback.
    tuneset.onUpdate(tuningOnUpdate);
}

void loop() {
    // Poll for commands, as usual.
    tuneset.readSerial();

    delay(10);
}
```


### Custom Reader Example

An example demonstrating how to construct a `Reader` for custom types. A custom `Writer` follows similarly, except you go the opposite direction: translating custom types to `String`s. Remember to also define a `SERIAL_TUNING_TYPE_LIST` in your `tuning_profile.h`!

```cpp
// vec2.h

// Have a custom type in the first place.
struct Vec2 { float x, y; };
```

```cpp
// tuning_profile.h

// Define a `SERIAL_TUNING_TYPE_LIST` macro.
// Add your custom type to the type list.
// Make sure to stick to the format, and don't forget backslashes if necessary.
#define SERIAL_TUNING_TYPE_LIST(X) \
    X(int8_t)                      \
    X(int16_t)                     \
    X(int32_t)                     \
    X(int64_t)                     \
    X(uint8_t)                     \
    X(uint16_t)                    \
    X(uint32_t)                    \
    X(uint64_t)                    \
    X(float)                       \
    X(double)                      \
    X(String)                      \
    X(Vec2)

// Include the definition for Vec2, so that Serial Tuning knows how to call it in a function.
#include "vec2.h"
```

```cpp
// main.cpp

#include "vec2.h"

// Inherit `DefaultReader`.
class MyReader : public DefaultReader {
public:
    // Inherit default `read()` functions.
    using DefaultReader::read;

    // Overload `read()` for your custom type.
    template <typename T, enable_if_t<std::is_same<T, Vec2>::value, int> = 0>
    static Vec2 read(const String& value)
    {
        // Parse 2 floats from the string.
        int i = value.indexOf(',');
        if (i == -1) {
            // Couldn't find the delimiter, so assume second float is 0.
            return Vec2{value.toFloat(), 0.0f};
        }
        return Vec2{value.substring(0, i).toFloat(), value.substring(i + 1).toFloat()};
    }
};

// Declare a type alias for convenience.
using MyTuneSet = TuneSet<SERIAL_TUNING_DEFAULT_MAX_ITEMS, MyReader, DefaultWriter>;

// Use as usual.
MyTuneSet tuneset;

float kp;
Vec2 v;

void setup() {
    tuneset.add("kp", kp);
    tuneset.add("v", v);
}
```



## Roadmap

* [ ] Work with other UART ports, not just the default `Serial`.


<!-- 
## Dev Notes

### Publish on PlatformIO

* Modify library.json, update the version.
* Run `pio pkg publish` in a PlatformIO terminal.

### Publish on Arduino Package List

* Modify library.properties, update the version.
* Make a new GitHub release.

-->