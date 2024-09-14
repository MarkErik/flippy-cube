/*
  The code controls LED lights mounted on a cube fitted with tilt sensors, and a gyroscope.
  Based on the orientation and movement of the cube, the LEDs either change colour or brightness.
  Author: Mark Altosaar
  https://github.com/MarkErik/flippy-cube

  Incorporates libraries, and their example code from:
  Library to control LED strip: https://github.com/FastLED/FastLED 
  Library to read from gyro: https://github.com/rfetick/MPU6050_light
  Arduino Debounce example: https://docs.arduino.cc/built-in-examples/digital/Debounce/
*/

//GYRO LIBRARIES AND VARIABLES
#include "Wire.h"           //Interface with 6050MPU
#include <MPU6050_light.h>  //Library for reading value from the 6050 sensor
MPU6050 mpu(Wire);

//LED STRIP LIBRARIES AND VARIABLES
#include <FastLED.h>           // Include FastLED library
#define NUM_LEDS 8             // Number of LEDs in the chain
#define DATA_PIN 6             // Data pin for LED control
#define NUM_COLOURS 4          // Number of colours to cycle through depending on the sideways orientation
CRGB leds[NUM_LEDS];           // Array to hold LED Strip color data
CRGB colours[NUM_COLOURS];     // Array to hold the different colours that can be selected based on the side position
int currentColour = 0;         //integer to hold the position of the current colour based on what side the cube is on
int brightness = 255;          //0-255 value, start at max brightness
unsigned long timeBright = 0;  //timer to keep track of how often to check for brightness adjustments

//DEBOUCE VARIABLES
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 55;    // the debounce time; increase if the output flickers

//CUBE ORIENTATION VARIABLES
int facingDown;         //variable to hold whether the cube is facing up or on its side, or down
int rotating;           //value to hold the gyro movement state
int tiltState;          //variable to hold the sum of the 4 tilt sensors
int lastTiltState = 4;  //set the variable as if the cube is facing up at the start
/// Note of the cube sensor pairs
// Pin 10 paired with Pin 14, and Pin 16 paired with Pin 15
// variables to hold pin values from tilt sensor
int pin10;
int pin14;
int pin15;
int pin16;

void setup() {
  //Set the pins for the tilt sensors as inputs
  pinMode(14, INPUT);
  pinMode(15, INPUT);
  pinMode(16, INPUT);
  pinMode(10, INPUT);

  // Pause for 1.2 seconds, so that after turning on the power, the board isn't moved so the gyro can initialize and calibrate
  delay(1200);
  Wire.begin();
  mpu.begin();
  mpu.calcOffsets(true, true);

  // Initialize LEDs
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  //initialize the colour array
  colours[0] = CRGB::White;
  colours[1] = CRGB::Red;
  colours[2] = CRGB::Gold;
  colours[3] = CRGB::Amethyst;

} //end the setup section

void loop() {
  mpu.update();
  rotating = mpu.getGyroZ();

  pin10 = digitalRead(10);
  pin14 = digitalRead(14);
  pin15 = digitalRead(15);
  pin16 = digitalRead(16);


  tiltState = pin10 + pin14 + pin15 + pin16;
  facingDown = !(pin10 || pin14 || pin15 || pin16);

  // check to see if any or all of the tilt sensors changed
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last movement to ignore any noise
  if (tiltState != lastTiltState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state

    // use a timer to delay how often we check for rotation and brightness, otherwise
    // because the loop runs so fast, the brightness quickly goes to 0 or max
    if (millis() - timeBright > 160) {
      if (tiltState == 4) {  //check if cube facing up, so that it can be rotated to control brightness
                             //try to filter out noise from the sensor, and unintentional bumps, instead look for intentional rotation
        if (rotating < -6) {
          brightness += 15;
          if (brightness > 255) {
            brightness = 255;
          }
        } else if (rotating > 6) {
          brightness -= 15;
          if (brightness < 0) {
            brightness = 0;
          }
        }
      }
      timeBright = millis();  //reset brightness timer
    }

    FastLED.setBrightness(brightness);

    // To determine the color, need to check that the pairs are opposite, but may not always work,
    // as when the tilt sensors are on flat their side, they could also end up in a situation where they are opposite,
    // becuase the ball is just rolling around
    if (pin10 == HIGH && pin14 == LOW) {
      currentColour = 0;
    } else if (pin10 == LOW && pin14 == HIGH) {
      currentColour = 1;
    } else if (pin15 == HIGH && pin16 == LOW) {
      currentColour = 2;
    } else if (pin15 == LOW && pin16 == HIGH) {
      currentColour = 3;
    } else {
      currentColour = currentColour;  //keep the color the same if in some weird state
    }

    // Loop through each LED and set it
    for (int dot = 0; dot < NUM_LEDS; dot++) {
      if (facingDown == HIGH) {
        leds[dot] = CRGB::Black;  // Clear the current LED
      } else {
        leds[dot] = colours[currentColour];  // Set the current LED to blue
      }
      FastLED.show();  // Update LEDs
    }                  //end for looping to update LEDs
  }                    // end if debounce timer greater than threshold

  // save the reading. Next time through the loop, it'll be the lastTiltState:
  lastTiltState = tiltState;
} // end the main loop
