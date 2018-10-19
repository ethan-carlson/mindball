# Mindball

Mindball is a "competitive meditation" game where you win by being the most relaxed.  This is meant to be a single-player version played using the NeuroSky MindWave headset.

The general engineering structure is as follows:
  -A MindWave headset is paired with a laptop (Mac, in my case, but it should work on any OS on which ThinkGear is supported)
  -The headset passes data to the ThinkGear program listening on the bluetooth com port
  -Thinkgear makes the data available on localhost, where locally running code can snag it
  -The JSON data is parsed and manipulated, then passed back out over Bluetooth to and Arduino with a BlueSMiRF attached
  -The Arduino is running software serial to receive data from the BlueSMiRF, which allows for standard debugging over USB
  -Game code runs on the arduino
  -At the time of this writing, the output is a neopixel display
  -Future work will create a stepper motor output with a linear actuator assembly to allow for the real game to be played

The Processing code is a stripped down version of the the Brain Grapher made by Eric Blue:
#  url   = http://eric-blue.com/2011/07/13/neurosky-brainwave-visualizer/
#  email = ericblue76 (at) gmail (dot) com
# Project: Neurosky Brainwave Visualizer

I used his code to get brainwave JSON data off the MindWave and do some math to get the "game value" that I was interested in.  Right now the game value is the sum of of the natural logs of the alpha waves divided by the sum of the natural logs of the beta waves, times 100.  The ln is used to make the wave values nominally comperable, so that % movement in one does not outweigh the other by too much.

This value is calculated in the Processing code in order to allow the bluetooth pipe to send a single number, meaning no parsing code is necessary on the Arduino.
