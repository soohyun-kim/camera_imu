The correct procedures in using the camera and the board in conjunction:

1.)  Ensure that the power switch located in the lower half othe board is set to off (right position).  This ensures that the board and camera boot simultaneously as to allow for the PWM signal to be introduced during the camera's grace period.
2.)  Power on the camera.  This may be done either by simply plugging in the corded power supply to the board, or pressing the camera's power button (when on battery power).
3.)  Observe the LED.  A solid green indicates all systems ready, to enter sensor calibration automatically.  A flashing magenta indicates that the sensor has not been detected.  A flashing yellow indicates the absence of an SD card.
4.)  The sensor will automatically enter calibration, indicated by a flashing green.  Do not touch the rig until flashing finishes.
5.)  Once calibration has finished, the LED will change to a solid blue, and the seven-segment display will now light on, indicating the number of record files created on the SD card.  The camera rig is now ready to record.
6.)  Record by pressing the RECORD button ONCE.  The LED will change to a solid red as indication.  Pressing it again will stop record.  Refer to the camera manual for general operation of the camera.
7.)  Press and HOLD the UNFREEZE button to initiate camera motion, and release when motion is finished.  This is to ensure that the corresponding virtual camera does not drift while the real counterpart remains motionless.
8.)  If sensor recalibration is needed, power cycle the board (use the power switch).  Do not power down the camera.

DO NOT:

	- Power the camera with the board's power switch in the OFF position.
	- Recalibrate the sensor without at least one REC start/stop cycle has been completed.


Engineering notes:

I still am not in full understanding of the PWM specs of this camera; there isn't much documentation for it at all.  Apparently it's tailored for that of servos which, according to Wikipedia, works in a way such that the absolute pulse width duration is only important, not the duty cycle; I dread experimenting with such a configuration.

In the current code it is written such that the PWM frequency is set to 50 Hz (the traditional servo frequency), and record start triggers when the standby duty cycle value of 51 (range 0-255, uint8, corresponding to 0-100% duty cycle) crosses a threshold value of 52 to end on 53; inversely, record stops on descent from 53 crossing 52 to 51.  This is the most reliable method discovered by my own experimentation.  This change cannot be abrupt; an immediate change between the two end values will not be registered by the camera.  The duty cycle value must lie on 52 for a finite duration of time; this is implemented as such in the code.

Further complicating the process is BlackMagic's decision to have the REC start/stop control accept a simple "switch" signal as well as a PWM signal.  In BlackMagic fashion, this is also little-documented, but experimentation revealed that REC start/stop toggles on a falling edge to ground.  This is problematic, as an independent power cycle of the Teensy temporarily drives all digital outputs low, which is then interpreted by the camera as a record toggle.  Simply put: a sensor recalibration results in an undesirable record toggle and subsequent index mismatching of the footage file and the camera rotation file.

This dual-purpose control creates complications in PWM mode as well.  The manual states (as one of the very few details listed about the specs) that the camera first operates in switch mode, expecting a switch input, until a PWM signal is detected, after which only the PWM signal is accepted; to return to switch mode, the camera must be power cycled.  Unfortunately, this results in the camera erronously interpreting the very first falling edge of the PWM signal as a switch toggle; only after is the PWM signal acknowledged.  In following, the camera always starts record at board startup, the instant the PWM signal is detected.

A workaround to this complication was quickly conceived when, following more experimentation, it was discovered that the camera has a brief "grace period" upon startup, when the SD card is acknowledged and characterized, in which external REC signals are ignored; introducing the PWM signal during this period will not trigger a REC start.  So, the code is written to introduce the PWM signal during this grace period, and it appears to work well...

...except on one condition.  A rather bizarre behavior, the camera only fully characterizes the PWM signal once one REC start/stop cycle has been completed in the defined fashion.  Only after at least one REC start/stop cycle has been administered does the camera fully "understand" the nature of the command and trigger record in the desired behavior, even when independently power cycling the board.  Failure to do this will result in the camera somehow entering record upon reboot of the Teensy after a power cycle.  Again - very bizarre and I fail to understand it in the slightest way...