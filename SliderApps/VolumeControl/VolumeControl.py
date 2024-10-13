from MacroPad import MacroPad

# Needs pyvolume from https://github.com/thevickypedia/volume-control
# Install it via "pip install volume-control"
import pyvolume

# Get list of all MacroPad devices
devices = MacroPad.listDevices()
# Choose the first one
mp = MacroPad(next(iter(devices.values())))

# Read initial slider value
vol = mp.getSliderPos() * 100 // 255
print("Setting volume to " + str(vol) + "%")
pyvolume.custom(percent = vol)
previousVol = vol

# Set volume whenever slider changes
while(True):
	vol = mp.waitForSliderChange(-1) * 100 // 255
	if vol != previousVol:
		print("Setting volume to " + str(vol) + "%")
		pyvolume.custom(percent = vol)
		previousVol = vol
