/**
 * \file input.cpp
 * Implementation for input.h
 */

#include<new>
#include"hardware/adc.h"
#include"input.h"

//-----------------------------------------------------------------------------
// Switch implementation

Switch::Switch(uint pin, uint debounceDuration)
:	pin(pin),
	pressed(false),
	debounceDuration(debounceDuration),
	events()
{
	// Set up debounce buffer
	if(this->debounceDuration < 1 || this->debounceDuration > 31)
		this->debounceDuration = 10;
	buffer = (1 << this->debounceDuration) - 1;

	// Set initial timestamp to now
	tsLastEvent = get_absolute_time();

	// Set up pin (input with pull-up)
	gpio_init(pin);
	gpio_set_dir(pin, false);
	gpio_pull_up(pin);
}

void Switch::update()
{
	// Read the pin and put the value into the debouncing buffer
	buffer = ((buffer << 1) & ((1 << debounceDuration) - 1)) | (gpio_get(pin) ? 1 : 0);

	// Check if the pin has stabilised (all buffer values are equal)
	if(buffer == 0 && !pressed)
	{
		// Press event detected
		// Change state
		pressed = true;
		// Create event and put in queue
		absolute_time_t now = get_absolute_time();
		events.insert(Event{Event::PRESS, static_cast<uint>(absolute_time_diff_us(tsLastEvent, now) / 1000)});
		tsLastEvent = now;
	}
	else if(buffer == (1 << debounceDuration) - 1 && pressed)
	{
		// Release event detected
		// Change state
		pressed = false;
		// Create event and put in queue
		absolute_time_t now = get_absolute_time();
		events.insert(Event{Event::RELEASE, static_cast<uint>(absolute_time_diff_us(tsLastEvent, now) / 1000)});
		tsLastEvent = now;
	}
}

//-----------------------------------------------------------------------------
// RotaryEncoder implementation

RotaryEncoder::RotaryEncoder(uint pinA, uint pinB)
: pinA(pinA), pinB(pinB), state(N), events()
{
	// Set initial timestamp to now
	tsLastEvent = get_absolute_time();

	// Set up pins (inputs with pull-up)
	gpio_init(pinA);
	gpio_set_dir(pinA, false);
	gpio_pull_up(pinA);
	gpio_init(pinB);
	gpio_set_dir(pinB, false);
	gpio_pull_up(pinB);
}

void RotaryEncoder::update()
{
	// Read the pins (Bit 1 = B activated, Bit 0 = A activated. Note that
	// "activated" means connected to GND, i.e. 0.)
	uint pins = (gpio_get(pinA) ? 0 : 1) | (gpio_get(pinB) ? 0 : 2);

	// Finite state machine
	switch(state)
	{
	case N:
		if(pins == 0b01) state = L1;
		else if(pins == 0b10) state = R1;
		break;
	case L1:
		if(pins == 0b11) state = L2;
		else if(pins == 0b00) state = N;
		break;
	case L2:
		if(pins == 0b10) state = L3;
		else if(pins == 0b01) state = L1;
		break;
	case L3:
		if(pins == 0b00)
		{
			state = N;
			// Left turn detected
			absolute_time_t now = get_absolute_time();
			events.insert(Event{Event::LEFT, static_cast<uint>(absolute_time_diff_us(tsLastEvent, now) / 1000)});
			tsLastEvent = now;
		}
		else if(pins == 0b11) state = L2;
		break;
	case R1:
		if(pins == 0b11) state = R2;
		else if(pins == 0b00) state = N;
		break;
	case R2:
		if(pins == 0b01) state = R3;
		else if(pins == 0b10) state = R1;
		break;
	case R3:
		if(pins == 0b00)
		{
			state = N;
			// Right turn detected
			absolute_time_t now = get_absolute_time();
			events.insert(Event{Event::RIGHT, static_cast<uint>(absolute_time_diff_us(tsLastEvent, now) / 1000)});
			tsLastEvent = now;
		}
		else if(pins == 0b11) state = R2;
		break;
	}
}

//-----------------------------------------------------------------------------
// Potentiometer implementation

Potentiometer::Potentiometer(uint pin, uint16_t adcMin, uint16_t adcMax, uint16_t hysteresis)
: pin(pin), position(0), events(), adcMin(adcMin), adcMax(adcMax), hysteresis(hysteresis)
{
	// Check parameter bounds
	if(adcMax > 4095) adcMax = 4095;
	if(adcMin > adcMax) adcMin = adcMax;
	if(hysteresis > (adcMax - adcMin + 1) / 256)
		hysteresis = (adcMax - adcMin + 1) / 256;

	// Set initial timestamp to now
	tsLastEvent = get_absolute_time();

	// Set up pin (digital input buffer disabled)
	adc_init();
	adc_gpio_init(pin);
	adc_select_input(pin - 26);
}

void Potentiometer::update()
{
	// Read the analog pin value
	adc_select_input(pin - 26);
	uint16_t newRaw = adc_read();

	uint8_t oldPos = position;
	uint8_t newPos = rawToPos(newRaw);

	// Apply hysteresis
	if(newPos < position)
	{
		// Moved down
		if(newPos == 255 || rawToPos(newRaw + hysteresis) == newPos)
			position = newPos;
		else
			position = newPos + 1;
	}
	else if(newPos > position)
	{
		// Moved up
		if(newPos == 0 || rawToPos(newRaw - hysteresis) == newPos)
			position = newPos;
		else
			position = newPos - 1;
	}

	// Create event
	if(position != oldPos)
	{
		uint now = get_absolute_time();
		events.insert(Event{(int)newPos - (int)oldPos, newPos, static_cast<uint>(absolute_time_diff_us(tsLastEvent, now) / 1000)});
		tsLastEvent = now;
	}
}

//-----------------------------------------------------------------------------
// InputMonitor implementation

/// Memory for the InputMonitor singleton
alignas(InputMonitor) uint8_t inputMonitorMemory[sizeof(InputMonitor)];

InputMonitor* InputMonitor::instance = nullptr;

void InputMonitor::create()
{
	if(instance == nullptr)
		instance = new(inputMonitorMemory) InputMonitor();
}

InputMonitor::InputMonitor()
:	switches{Switch(16), Switch(17), Switch(18), Switch(19), Switch(20), Switch(21), Switch(11), Switch(8), Switch(27)},
	rotaryEncoders{RotaryEncoder(13, 12), RotaryEncoder(10, 9), RotaryEncoder(22, 26)},
	potentiometers{Potentiometer(28, 11, 4077, 3)},
	mode(Mode::STOPPED),
	sleepingInput(false)
{
	// Prepare IRQs on all switch & rotary encoder pins (but don't enable them yet)
	gpio_set_irq_callback(sleepingGpioCb);
	irq_set_enabled(IO_IRQ_BANK0, false);
	for(const Switch& sw : switches)
		gpio_set_irq_enabled(sw.getPin(), GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
	for(const RotaryEncoder& re : rotaryEncoders)
	{
		gpio_set_irq_enabled(re.getPinA(), GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
		gpio_set_irq_enabled(re.getPinB(), GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
	}
}

void InputMonitor::setMode(Mode mode)
{
	// No change
	if(mode == this->mode)
		return;

	// Stop previous mode
	switch(this->mode)
	{
		case Mode::SLEEPING:
			irq_set_enabled(IO_IRQ_BANK0, false);
			break;
		case Mode::RUNNING:
			cancel_alarm(runningAlarm);
			break;
	}

	// Start new mode
	switch(mode)
	{
		case Mode::SLEEPING:
			sleepingInput = false;
			irq_set_enabled(IO_IRQ_BANK0, true);
			break;
		case Mode::RUNNING:
			runningAlarm = add_alarm_in_ms(0, runningAlarmCb, this, true);
			break;
	}

	this->mode = mode;
}

int64_t InputMonitor::runningAlarmCallback(alarm_id_t id, void* user_data)
{
	// Update all input controls
	for(Switch& sw : switches)
		sw.update();
	for(RotaryEncoder& re : rotaryEncoders)
		re.update();
	for(Potentiometer& pt : potentiometers)
		pt.update();

	// Fire again 1000us after this one was fired
	return -1000;
}

void InputMonitor::sleepingGpioCallback(uint gpio, uint32_t event_mask)
{
	sleepingInput = true;
}
