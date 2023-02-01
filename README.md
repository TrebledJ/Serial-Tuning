# Serial Tuning

![Get some sense Robin!](docs/gesundheit.jpg)

## Features

* [x] Tune variables without restarting the program, using Arduino's serial monitor (`Serial`).
* [x] Tune a plethora of types: integers, floating-points, strings.
* [x] Works on boards based on the Arduino framework (e.g. ESP32).
* [x] Print variables.
* [x] Works with custom types.
* [x] Report bad input and parse errors through opt-in logging.

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