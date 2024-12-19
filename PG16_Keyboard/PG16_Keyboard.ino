
#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Modulino.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"


#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// Create MPR121 and ModulinoBuzzer instances
Adafruit_MPR121 cap = Adafruit_MPR121();
ModulinoBuzzer buzzer;
ModulinoButtons buttons;
ModulinoKnob knob;
ModulinoDistance distanceSensor;
ModulinoMovement movement;
ModulinoPixels leds;
ArduinoLEDMatrix matrix;

//Extra LED strip colors
ModulinoColor YELLOW(255, 255, 0);
ModulinoColor OFF(0, 0, 0);

// Stack to keep track of pressed keys
int pressedStack[12];  // Stack to hold active keys
int stackIndex = -1;   // Track the current top of the stack
float scaleFactor = 1.0;

// Array to store frequencies for each pad
int padFrequencies[12] = { 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988 };
int adjustedFrequencies[12] = { 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988 };
char noteNames[][12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
int currentOctave = 0;  // Default to octave 0

// Keeps track of the last pads touched and the currently playing frequency
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
bool isPlaying = false;    // Track if the tone is currently playing
int currentFrequency = 0;  // Track the current playing frequency

//Knob setup
bool knobPressed = false;
int lastKnobValue = 0;
bool knobPriority = false;
bool lockout = false;
unsigned long knobLastMovedTime = 0;

//bar graph holder frame
uint8_t frame[8][12] = { 0 };
unsigned long lastDisplayTime;
int graphDelay_ms = 1000;

//Vibrato control
bool vibratoEnabled = false;        // Whether the vibrato effect is active
float vibratoDepth = 0.0;           // Maximum frequency deviation (e.g., ±10 Hz)
float vibratoDepthBackup = 6.0;     //store value when disabled
float vibratoRate = 0.0;            // How fast the frequency oscillates (e.g., 5 Hz)
float vibratoRateBackup = 3.0;      //store value when disabled
unsigned long lastVibratoTime = 0;  // Used for time tracking
float vibratoDepthMax = 20.0;
float vibratoRateMax = 10.0;

//Accelerometer variables
float tiltValue = 0;
float pitchMultiplier = 1.0; //for pitch bend


void playSound(int frequency) {

  //Vibrato calculation
  int vibratoOffset = 0;

  if (vibratoEnabled) {
    // Calculate elapsed time
    unsigned long currentTime = millis();
    float elapsedTime = (currentTime - lastVibratoTime) / 1000.0;  // Convert ms to seconds

    // Calculate vibrato using a sine wave (or use `sin()`)
    vibratoOffset = vibratoDepth * sin(2 * PI * vibratoRate * elapsedTime);

    // Update time tracking (reset every cycle)
    if (elapsedTime > (1.0 / vibratoRate)) {
      lastVibratoTime = currentTime;
    }
  }

  //play tone
  int modulatedFrequency = frequency + vibratoOffset;
  buzzer.tone(modulatedFrequency, 65);  // Short duration to simulate continuous tone
  currentFrequency = frequency;
  isPlaying = true;
}


void stopSound() {
  if (isPlaying) {
    buzzer.noTone();
    isPlaying = false;
    currentFrequency = 0;
  }
}

void barGraph(int min, int max, int value, int height) {

  //Not used (alternative to sine wave visual)
  height = constrain(height, 1, 8);  // Height cannot exceed the matrix height (8 rows)

  // Calculate starting and ending rows for the bar
  int startRow = (8 - height) / 2;     // Center the bar vertically
  int endRow = startRow + height - 1;  // Determine the last row to fill

  // Clear the frame array
  memset(frame, 0, sizeof(frame));

  // Calculate how many columns to fill (0-12)
  int filledColumns = map(value, min, max, 0, 12);
  filledColumns = constrain(filledColumns, 0, 12);  // Ensure within matrix bounds

  // Draw the bar in the frame
  for (int x = 0; x < filledColumns; x++) {
    for (int y = startRow; y <= endRow; y++) {  // set height of graph line
      frame[y][x] = 1;                          // Turn on the pixel at (x, y)
    }
  }
  // Render the frame to the matrix
  matrix.renderBitmap(frame, 8, 12);

  lastDisplayTime = millis();
}

//map function that works with floats
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void sineWaveGraph() {
  // Clear the frame
  memset(frame, 0, sizeof(frame));  // Clear the frame array
  // Map vibratoDepth to sine wave amplitude
  int amplitude = map(vibratoDepth, 0, vibratoDepthMax, 0, 6);
  amplitude = constrain(amplitude, 0, 6);  // Ensure within matrix bounds

  // Use mapFloat to calculate cycles as a float
  float cycles = mapFloat(vibratoRate, 0, vibratoRateMax, 0.0, 4.0);  // Number of cycles

  // Angle step for sine wave
  float step = (2.0 * PI * cycles) / 12.0;

  // Calculate the center of the matrix
  int centerX = 12 / 2;  // Center column (6 for a 12-column matrix)

  // Draw the sine wave symmetrically from the center outwards
  for (int offset = 0; offset <= centerX; offset++) {
    for (int sign = -1; sign <= 1; sign += 2) {  // Calculate left and right columns relative to the center
      int x = centerX + (sign * offset);         // Column index moving symmetrically outward

      if (x >= 0 && x < 12) {                             // Ensure x is within matrix bounds
        float normalizedX = (float)(x - centerX) / 12.0;  // Normalize distance from center
        float angle = 2.0 * PI * cycles * normalizedX;    // Angle for sine calculation
        int yCenter = 4;                                  // Center of the sine wave vertically
        int y = yCenter + round(amplitude * sin(angle));  // Calculate y position

        // Constrain y to stay within matrix bounds
        y = constrain(y, 0, 7);

        // Set the pixel at (x, y) to represent the main sine wave
        frame[y][x] = 1;

        // Ensure 2-row thickness by lighting up 1 row above the main row
        int thickenedY = max(0, y - 1);  // Make sure we don't go below row 0
        frame[thickenedY][x] = 1;        // Light up the row above the main y
      }
    }
  }
  // Render the frame to the matrix
  matrix.renderBitmap(frame, 8, 12);
  lastDisplayTime = millis();
}


// Push a key onto the stack
void pushKey(int key) {
  if (stackIndex < 11) {  // Ensure stack doesn't overflow
    pressedStack[++stackIndex] = key;
  }
}

// Remove a key from the stack
void popKey(int key) {
  for (int i = 0; i <= stackIndex; i++) {
    if (pressedStack[i] == key) {
      // Shift all elements down to remove the key
      for (int j = i; j < stackIndex; j++) {
        pressedStack[j] = pressedStack[j + 1];
      }
      stackIndex--;  // Decrement stack top
      break;
    }
  }
}

void handleButtons() {
  if (buttons.update()) {          // Check for button state changes
    for (int i = 0; i < 3; i++) {  // Iterate through the buttons
      if (buttons.isPressed(i)) {  // Check if button is pressed
        int newOctave = i - 1;     // Map button 0 to -1, button 1 to 0, button 2 to +1

        if (newOctave != currentOctave) {  // Only update if octave changes
          currentOctave = newOctave;

          // Update LEDs: only the current state's LED is on
          buttons.setLeds(currentOctave == -1, currentOctave == 0, currentOctave == 1);

          // Recalculate frequencies for the new octave
          for (int j = 0; j < 12; j++) {
            adjustedFrequencies[j] = padFrequencies[j] * pow(2, currentOctave);
          }

          // Debugging output for octave changes
          Serial.print("Current Octave: ");
          Serial.println(currentOctave);
        }
      }
    }
  }
}

void handleKnob() {
  float speedScale = .25;
  float value = knob.get();

  if (value > vibratoRateMax) {  // Set limits
    value = vibratoRateMax;
    knob.set(value);
  } else if (value < 0) {
    value = 0;
    knob.set(value);
  }

  if (value != lastKnobValue) {  //knob is being turned
    if (vibratoEnabled) {
      knobPriority = true;
      lockout = true;   //stop note name from displaying
      //barGraph (0, vibratoRateMax, value, 4);
      vibratoRate = value;
      lastKnobValue = value;
    }
    sineWaveGraph();
  }
  if (knob.isPressed() && !knobPressed) {  // Toggle vibrato when knob is pressed
    knobPressed = true;
    if (vibratoEnabled) {  //disable
      vibratoEnabled = false;
      //set values to zero temporarily for visualization
      vibratoDepthBackup = vibratoDepth;
      vibratoRateBackup = vibratoRate;
      vibratoDepth = 0;
      vibratoRate = 0;
      Serial.println("Vibrato disabled");
    } else {  //enable
      vibratoEnabled = true;
      //recall backed up values
      vibratoDepth = vibratoDepthBackup;
      vibratoRate = vibratoRateBackup;
      Serial.println("Vibrato enabled");
    }
    sineWaveGraph();
  } else if (!knob.isPressed() && knobPressed) {  //knob released, reset
    knobPressed = false;
  }
}

void handleTof() {
  if (distanceSensor.available()) {
    int measure = distanceSensor.get();
    if (measure < 200) {  //activate within a limited distance from the sensor
      //stop note names from displaying
      knobPriority = true;
      lockout = true;

      vibratoDepth = map((float)measure, 15, 180, 0, vibratoDepthMax);
      sineWaveGraph();
    } else {
      knobPriority = false;
    }
  } else {
    knobPriority = false;
  }
}

void setPixel(int pixel, ModulinoColor color) {
  leds.set(pixel, color, 15);
  leds.show();
}

void handleMovement() {
  movement.update();
  tiltValue = movement.getX(); //get x rotation value from IMU
  //dead zone in center
  if (tiltValue > -0.05 && tiltValue < 0.05) {
    tiltValue = 0;
  }
  pitchMultiplier = mapFloat(tiltValue, -0.3, 0.3, 1.122, 0.891);   //map range of tilt sensor to 1 whole step of pitch bend as a multiplier (2^-1/12 to 2^1/12)

  //Serial.println(tiltValue);
  int barGraphVal = int(mapFloat(tiltValue, -0.3, 0.3, 8.0, 0.0));  //map range to 8 bit led bar
  //update visual display on led bar
  for (int i = 0; i < 8; i++) {
    if (i <= barGraphVal) {
      setPixel(i, BLUE);
    } else {
      setPixel(i, RED);
    }
  }
}

void matrixText(char matrixText[]) {
  //take a char array and display it on the LED matrix
  Serial.println(knobPriority);
  if (!lockout) {   //don't display if knob or TOF are active
    matrix.clear();
    //LED Matrix Note Name
    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);

    // add the text
    matrix.textFont(Font_5x7);
    matrix.beginText(4, 1, 0xFFFFFF);
    matrix.println(matrixText);
    matrix.endText();

    matrix.endDraw();
    lastDisplayTime = millis();
  }
}

void setup() {
  Serial.begin(9600);
  delay(2000);  // Give time for Serial to initialize
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");
  // Initialize the buzzer
  Modulino.begin();
  buzzer.begin();
  buttons.begin();
  knob.begin();
  movement.begin();
  leds.begin();
  matrix.begin();

  knob.set(vibratoRate);

  Serial.println("Initializing ToF Sensor...");
  if (!distanceSensor.begin()) {
    Serial.println("Failed to initialize ToF sensor! Check wiring and power.");
    while (1)
      ;  // Halt if sensor initialization fails
  }
  Serial.println("ToF Sensor initialized successfully.");

  buttons.setLeds(false, true, false);

  // Initialize Wire1 for the Qwiic connector and MPR121
  Wire1.begin();
  if (!cap.begin(0x5A, &Wire1)) {  // Use &Wire1 for the Qwiic port
    Serial.println("MPR121 not found, check wiring and I2C address!");
    while (1)
      ;
  }
  Serial.println("MPR121 found!");

  //clear pixel strip
  for (int i = 0; i < 8; i++) {
    setPixel(i, OFF);
  }
}

void loop() {
  // Get the currently touched pads
  currtouched = cap.touched();

  // Run all sensor handlers
  handleButtons();
  handleTof();
  handleKnob();
  handleMovement();

  // Check each pad to update the stack
  for (uint8_t i = 0; i < 12; i++) {
    if (currtouched & _BV(i)) {       // Check if pad i is currently pressed
      if (!(lasttouched & _BV(i))) {  // Only push if it was not already pressed
        pushKey(i);                   // Add newly pressed key to the stack
      }
    } else if (lasttouched & _BV(i)) {  // Pad was just released
      popKey(i);                        // Remove released key from the stack
    }
  }

  // Play the sound of the most recent key in the stack (last pressed)
  if (stackIndex >= 0) {
    int lastKey = pressedStack[stackIndex];
    if (adjustedFrequencies[lastKey] != currentFrequency) {  // Play new frequency if it changed
      playSound(adjustedFrequencies[lastKey] * scaleFactor * pitchMultiplier);
      matrixText(noteNames[lastKey]);  //display current note name
    } else {
      // Continue playing the current frequency if no changes
      playSound(currentFrequency * scaleFactor * pitchMultiplier);  //includes pitch bend multiplier
    }
  } else {
    stopSound();  // Stop sound if no keys are pressed
  }

  //keep latest displayed info on the screen for a period of 1 second
  if (millis() - lastDisplayTime > graphDelay_ms) {
    matrix.clear();
    if (!knobPriority) {
      lockout = false;  //reset note name lockout after time elapsed
    }
  }

  // Reset our state
  lasttouched = currtouched;
  delay(10);  // Small delay for smooth playback
}