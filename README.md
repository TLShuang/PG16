# Suitcase Synth Modulino


[LEARN ABOUT OUR PROCESS THROUGH THIS PROJECT IN THE POTENTIALLY GENIUS SERIES](https://youtu.be/-QRer8uG97s?si=7fIsf5wegiuD1a32)


![full build](https://raw.githubusercontent.com/TLShuang/PG16/refs/heads/main/Images%20and%20Videos/fullBuilds.jpg)

## Overview
Building on Plug and Make Kit, the Suitcase Synth Modulino is a portable, modular sound synthesizer that combines capacitive touch, sensor-driven sound manipulation, and visual feedback. Built for creativity and interaction, it allows users to explore monophonic sound creation, pitch bending, vibrato effects, and more, through intuitive controls and modular components.

---

## Features
- **Capacitive Touch Keys**: Play 12 notes with adjustable octaves.
- **Modular Interaction**:
  - **Knob Modulino**: Adjust vibrato speed and enable/disable vibrato.
  - **ToF Distance Sensor Modulino**: Dynamically control vibrato depth.
  - **Accelerometer Modulino**: Tilt control for pitch bending.
- **Visual Feedback**:
  - LED Matrix displays note names and sine wave visualizations.
  - LED strip provides dynamic color changes based on input.
- **Sound Engine**: Generates sounds via a monophonic buzzer with modulation effects like vibrato and pitch bending.
- **Octave Control**: Select octave range using buttons with LED indicators.

---

## Setup
### Physical Setup and Components
- Plug and Make Kit with modulinos
- Modulino Buzzer, Buttons, Knob, Distance, Movement, and Pixels
- Power supply (USB or battery)
- Adafruit MPR121 Capacitive Touch Sensor
- TL synth PCB or any capacitive items
- 3D printer

### Software Setup
We used the following software to design this project. To recreate, you may use the following or use the provided file directly. You may also choose softwares that you feel comfortable using.
- Fusion 360 (Schematic and PCB)
- Solidworks (Enclosure Design)
- Bambu Studio (3D Printing Slicer)
- Arduino IDE (Programming)

### Assembly
1. Connect the MPR121 sensor on PCB for capacitive touch input. You can directly solder the MPR121 with male headers or use female headers.
2. Attach all Modulino components to the microcontroller and on the mainboard or plateboard. You can use threaded M3 heats insert or thread forming screws to attach them on to your 3D print.
3. Connect all the used Modulino with the Qwiic connectors provided in the kit.
4. Upload the provided code to your Arduino.

---

## How It Works
1. **Playing Notes**: Touch the capacitive keys to produce sounds.
2. **Octave Selection**: Use the buttons to switch between octaves. LEDs indicate the current octave.
3. **Sound Manipulation**:
   - Rotate the knob to adjust vibrato rate.
   - Move your hand over the ToF sensor to change vibrato depth.
   - Tilt the accelerometer to bend the pitch.
4. **Visual Feedback**: Note names, sine wave patterns, and color changes synchronize with your interaction.

---

## Code Highlights
- **Capacitive Touch**: Detects note presses and controls sound generation.
- **Vibrato Effect**: Adds a modulated frequency offset based on knob and ToF sensor input.
- **Pitch Bending**: Dynamically modifies pitch using accelerometer data.
- **Visuals**: LED Matrix displays note and sine wave data; LED strip reacts to pitch and vibrato changes.

---

## Usage
1. Turn on the Suitcase Synth Modulino.
2. Touch the keys to play notes.
3. Use the knob, distance sensor, and accelerometer for live sound effects.
4. Observe the visual feedback for an immersive experience.

---

## Applications
- Interactive sound installations.
- Music education and experimentation.
- Open-source hardware exploration.

---

## Future Expansion
The modular design allows users to add new sensors or outputs, enabling endless creative possibilities.

## License
This project is open-source and part of the Arduino Plug and Make Kit ecosystem.
