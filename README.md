# Light Timer Project

## About
This is a repository with a microcontroller firmware of a timer device implemented in a table light. The device was built using a Microchip PIC12F675 microcontroller. 
The code was written in C language and was used the MPLABX to compile and build the program. It was built a PCB to be added to the table light.
The code comments is in portuguese language. There are .pdf files with the schematics and pcb layout attached.

## How it works
The device has only a push button to turn the light on and off, and change the operation modes. 

### 1- Turning the light on with timer mode on:
A single press of the button, the device wake up and turn the light on with a timer of 40 minutes. You can change the color light pressing the button. 
There are 7 different colors.

Note: When the timer gets the last counting minute, the light will blink indicating that will turn the light off soon. 

### 2- Turning the light on with timer mode off:
If the button is pressed more than 15 seconds on the timer mode on, the timer mode is switched off, but the light continues on with no time restriction.

### 3- Turning the light off:
If the button is pressed more than 15 seconds on the timer mode off, the light is turned off and the device goes to the standby mode.


## VIDEO (In portuguese language)
[![The timer light working](http://img.youtube.com/vi/0DHHudRgUJE/0.jpg)](http://www.youtube.com/watch?v=0DHHudRgUJE "Timer Light Video")

