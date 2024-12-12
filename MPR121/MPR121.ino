#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <Modulino.h>

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// Create MPR121 and ModulinoBuzzer instances
Adafruit_MPR121 cap = Adafruit_MPR121();
ModulinoBuzzer buzzer;

// Stack to keep track of pressed keys
int pressedStack[12];   // Stack to hold active keys
int stackIndex = -1;    // Track the current top of the stack

// Keeps track of the last pads touched and the currently playing frequency
uint16_t lasttouched = 0;
uint16_t currtouched = 0;
bool isPlaying = false;    // Track if the tone is currently playing
int currentFrequency = 0;  // Track the current playing frequency

// Array to store frequencies for each pad
int padFrequencies[12] = {440, 494, 523, 587, 659, 698, 784, 880, 988, 1047, 1175, 1319};

void setup() {
  Serial.begin(9600);
  delay(2000);  // Give time for Serial to initialize

  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  // Initialize Wire1 for the Qwiic connector and MPR121
  Wire1.begin();
  if (!cap.begin(0x5A, &Wire1)) {  // Use &Wire1 for the Qwiic port
    Serial.println("MPR121 not found, check wiring and I2C address!");
    while (1);
  }
  Serial.println("MPR121 found!");

  // Initialize the buzzer
  Modulino.begin();
  buzzer.begin();
}


void playSound(int frequency) {
    buzzer.tone(frequency, 49);                       // Short duration to simulate continuous tone
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

void loop() {
  // Get the currently touched pads
  currtouched = cap.touched();

  // Check each pad to update the stack
  for (uint8_t i = 0; i < 12; i++) {
    if (currtouched & _BV(i)) {  // Check if pad i is currently pressed
      if (!(lasttouched & _BV(i))) {  // Only push if it was not already pressed
        pushKey(i);  // Add newly pressed key to the stack
      }
    } else if (lasttouched & _BV(i)) {  // Pad was just released
      popKey(i);  // Remove released key from the stack
    }
  }

  // Play the sound of the most recent key in the stack (last pressed)
  if (stackIndex >= 0) {
    int lastKey = pressedStack[stackIndex];
    if (padFrequencies[lastKey] != currentFrequency) {  // Play new frequency if it changed
      playSound(padFrequencies[lastKey]);
    } else {
      // Continue playing the current frequency if no changes
      playSound(currentFrequency);
    }
  } else {
    stopSound();  // Stop sound if no keys are pressed
  }

  // Reset our state
  lasttouched = currtouched;
  delay(10);  // Small delay for smooth playback
}