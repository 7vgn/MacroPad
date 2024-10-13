"""
This is an example to showcase the use the C++ MacroPad class. The only thing
this does is print the current slider value whenever it changes.
"""

from MacroPad import MacroPad

# Enumerate and print devices
devices = MacroPad.listDevices()
print(devices)

# Choose the first device from the list
mp = MacroPad(next(iter(devices.values())))

# Print active profile and initial slider position
print("Active profile: " + str(mp.getActiveProfile()))
print("Slider position: " + str(mp.getSliderPos()))

# Wait for changes and print
while(True):
	print("Slider position: " + str(mp.waitForSliderChange(-1)))
