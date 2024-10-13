from PyQt5.QtCore import *
from krita import *
from MacroPad.MacroPad import MacroPad

class MacroPadDocker(DockWidget):
	def __init__(self):
		super().__init__()

		# Create Timer that checks slider position
		self.timer = QTimer()
		self.timer.timeout.connect(self.update)
		
		# Create Docker
		self.setWindowTitle("MacroPad")
		# Main widget
		mainWidget = QWidget(self)
		self.setWidget(mainWidget)
		mainLayout = QVBoxLayout()
		mainWidget.setLayout(mainLayout)
		# Scan button and device dropdown
		self.btnScan = QPushButton("Scan", mainWidget)
		self.btnScan.clicked.connect(self.scanDevices)
		self.ddDevices = QComboBox(mainWidget)
		self.scanDevices()
		deviceLayout = QHBoxLayout()
		deviceLayout.addWidget(self.ddDevices, 1)
		deviceLayout.addWidget(self.btnScan, 0)
		mainLayout.addLayout(deviceLayout)
		# Enable checkbox
		self.cbEnable = QCheckBox("Enable", self)
		self.cbEnable.stateChanged.connect(self.enableChecked)
		mainLayout.addWidget(self.cbEnable)
	
	def __del__(self):
		self.stopMonitoring()
	
	def enableChecked(self, state):
		if(state == 0):
			self.stopMonitoring()
		elif(state == 2):
			self.startMonitoring()
		pass

	def canvasChanged(self, canvas):
		pass
	
	def scanDevices(self):
		devices = MacroPad.listDevices()
		self.ddDevices.clear()
		self.ddDevices.addItem("<Select Device>", None)
		for serial, path in devices.items():
			self.ddDevices.addItem(serial, path)
	
	@pyqtSlot()
	def update(self):
		pos = self.macroPad.getSliderPos()
		if(pos != self.previousPos):
			# Update previous position
			self.previousPos = pos
			# Convert slider position to brush size
			# FIXME: This is rather crude at the moment. We probably want something more logarithmic
			brushSize = pos / 4
			# Set brush size
			Krita.instance().activeWindow().activeView().setBrushSize(brushSize)
	
	def startMonitoring(self):
		# Check if a device is selected
		path = self.ddDevices.itemData(self.ddDevices.currentIndex())
		if(path is None):
			# Inform the user
			msgBox = QMessageBox()
			msgBox.setText("No device selected")
			msgBox.exec()
			# Uncheck the box
			self.cbEnable.setCheckState(0)
		else:
			# Create MacroPad instance
			self.macroPad = MacroPad(path)
			# Disable device selection (no changing device while monitoring)
			self.ddDevices.setEnabled(False)
			self.btnScan.setEnabled(False)
			# Initialise previous slider position
			self.previousPos = -1
			# Start timer
			self.timer.start(100)

	def stopMonitoring(self):
		# Stop timer
		if(self.timer.isActive()):
			self.timer.stop()
		# Delete MacroPad instance
		if(self.macroPad):
			del self.macroPad
		# Re-enable device selection
		self.ddDevices.setEnabled(True)
		self.btnScan.setEnabled(True)

Krita.instance().addDockWidgetFactory(DockWidgetFactory("MacroPadPlugIn", DockWidgetFactoryBase.DockRight, MacroPadDocker))
