
# Countdown Timer for the LED-5604 and Ardunio

A countdown timer built to run on the Ardunio Nano and display on the LED-5604 7-Segment.




## Documentation

This code shows how you could use the Arduino SPI 
library to interface with the LED-5604

There are functions for clearing the display.

The values for each segment are defined in the arrays.
In order to display a decimal you must call the decimal function.

The SPI.transfer() function is used to send a byte of the
SPI wires. Notice that each SPI transfer(s) is prefaced by
writing the SS pin LOW and closed by writing it HIGH.

Each of the custom functions handle the ssPin writes as well
as the SPI.transfer()'s.

## Circuit

| Ardunio           |7-Segment                                                           |
| ----------------- | ------------------------------------------------------------------ |
| 5V | VCC |
| GND | GND |
| 8 | SS |
| 11 | SDI |
|13|SCK|




## Buttons

|button|pin   |
|------|------|
|select|2|
|up|3|
|down|4|
|start/stop|5|

## Modes

|mode|id|
|----|--|
|timer|0|
|duration menu|1|
|minute setting|3|
|hour setting|4|
|adjust minute|6|
|adjust hour|7|

## Versions

#### 4.1

- Changed default hour interval to 1 min. Added a new debounce delay for
         Up/Down that is fasster. This allows the time to be set quicker without
         allowing accidental button press for the other buttons.

#### 4.0

- Changed the display to minutes when there is only one hour left
         in the countdown.
         Holding up/down buttons for 1.5 seconds increses the amount at
         which the setting moves.
- Fixed - Timer code now stops counting down when the timer reaches
         zero.

#### 3.1

- Added the output for the buzzer and light when the time reaches zero.

#### 3.0

- Production version, Cleaned up code.
## Author

[@Daniel Garcia](https://www.github.com/agent5905)


## License

[MIT](https://choosealicense.com/licenses/mit/)

