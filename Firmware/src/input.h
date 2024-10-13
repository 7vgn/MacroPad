/**
 * \file input.h
 * Classes that manage the input controls (switches, rotary encoders, potentiometers)
 */

#ifndef _INPUT_H
#define _INPUT_H

#include<cstdint>
#include"pico/stdlib.h"

/**
 * \brief An EventQueue can hold a certain amount of events. It uses a ring
 * buffer internally. If the capacity is exceeded, the oldest elements get
 * overwritten.
 */
template<typename Event, uint MAX_CAPACITY>
class EventQueue
{
private:
	/**
	 * \brief Buffer for events
	 */
	Event buffer[MAX_CAPACITY];

	/**
	 * \brief Elements are inserted at head and extrated at tail
	 * \details head==tail can mean an empty or a completely full queue.
	 * numEvents helps distinguish between the two cases.
	 */
	uint head, tail, numEvents;

public:
	/**
	 * \brief Constructs an empty EventQueue
	 */
	EventQueue(): head(0), tail(0), numEvents(0) {}

	/**
	 * \brief Inserts an event into the queue
	 * \param event The event to be inserted.
	 */
	void insert(Event event)
	{
		buffer[head] = event;
		head = (head + 1) % MAX_CAPACITY;
		if(numEvents < MAX_CAPACITY)
			numEvents++;
		else
			tail = (tail + 1) % MAX_CAPACITY;
	}

	/**
	 * \brief Extracts an event from the queue
	 * \return Returns the extracted event. If the queue is empty, an Event
	 * created via the standard constructor is returned.
	 */
	Event extract()
	{
		Event event;
		if(numEvents != 0)
		{
			numEvents--;
			event = buffer[tail];
			tail = (tail + 1) % MAX_CAPACITY;
		}
		return event;
	}

	/**
	 * \brief Calculates the size of the queue
	 * \return The number of events in the queue.
	 */
	inline uint size() const {return numEvents;}
};

/**
 * \brief Size of the event queue
 */
#define SWITCH_EVENT_QUEUE_SIZE 8

/**
 * \brief Represents a switch
 * \details A switch is associated with a pin that connects to GND when
 * pressed. The pin is sampled every 1ms and debounced in software.
 * The switch can be queried in two ways:
 * 1.) The current (debounced) state of the switch can be obtained via
 * isPressed().
 * 2.) The (recent) history of events (presses and releases) can be obtained
 * through getEvents().
 */
class Switch
{
public:
	/**
	 * \brief Type for switch events
	 */
	struct Event
	{
		/**
		 * \brief Was the switch pressed or released?
		 */
		enum {PRESS, RELEASE} type;
		/**
		 * \brief How long did the previous state last (in milliseconds)?
		 */
		uint duration;
	};

private:
	/**
	 * \brief Pin that is connected to the switch
	 */
	uint pin;

	/**
	 * \brief Status of the switch
	 */
	bool pressed;

	/**
	 * \brief Buffer for debouncing
	 */
	uint32_t buffer;

	/**
	 * \brief Debounce duration
	 */
	uint debounceDuration;

	/**
	 * \brief Event queue for this switch
	 * \details Stores press and release events.
	 */
	EventQueue<Event, SWITCH_EVENT_QUEUE_SIZE> events;

	/**
	 * \brief Timestamp of last event
	 */
	absolute_time_t tsLastEvent;

public:
	/**
	 * \brief Constructs a Switch instance
	 * \param pin The pin that the switch is connected to. 
	 * \param debounceDuration Debounce duration for in milliseconds. Must be
	 * between 1 and 31. This determines the size of the debounce buffer: The
	 * pin is sampled once every millisecond. As soon as debounceDuration many
	 * consecutive samples have the same value, the switch is assumed to have
	 * stopped bouncing.
	 */
	Switch(uint pin, uint debounceDuration = 10);

	/**
	 * \brief Getter for pin number
	 * \return Returns the pin number assigned to this switch.
	 */
	inline uint getPin() const {return pin;}

	/**
	 * \brief Updates the internal state of the instance
	 * \details This method must be called in 1ms intervals in order for the
	 * switch to be monitored.
	 */
	void update();

	/**
	 * \brief Returns the current state of the switch
	 * \return True if the switch is pressed, false otherwise. 
	 */
	inline bool isPressed() const {return pressed;}

	/**
	 * \brief Returns the event queue
	 * \return The event queue for this switch.
	 */
	inline EventQueue<Event, SWITCH_EVENT_QUEUE_SIZE>& getEvents() {return events;}
};

/**
 * \brief Size of the event queue
 */
#define ROTENC_EVENT_QUEUE_SIZE 8

/**
 * \brief Represents a rotary encoder
 * \details A rotary encoder is associated with two pins that connect to GND
 * in a certain sequency when turned. The pins are sampled every 1ms.
 * A finite state machine is used to ensure the sequence is read correctly and
 * without bouncing.
 * The events (left turn, right turn) are stored in an EventQueue which can be
 * obtained through getEvents().
 */
class RotaryEncoder
{
public:
	/**
	 * \brief Type for rotary encoder events
	 */
	struct Event
	{
		/**
		 * \brief Was the rotary encoder turned left or right?
		 */
		enum {LEFT, RIGHT} type;
		/**
		 * \brief How long did the previous state last (in milliseconds)?
		 */
		uint duration;
	};

private:
	/**
	 * \brief Pins that are connected to the rotary encoder
	 */
	uint pinA, pinB;

	/**
	 * \brief Finite state machine for decoding
	 */
	enum
	{
		// Rotary encoder is in neutral state with neither pin connected to GND
		N,
		// Rotary encoder is currently turning left, going through a 2-bit
		// Hamming sequence. The state reflects which stage of the sequence
		// we're in.
		L1, L2, L3,
		// Rotary encoder is currently turning right, going through a 2-bit
		// Hamming sequence. The state reflects which stage of the sequence
		// we're in.
		R1, R2, R3
	} state;

	/**
	 * \brief Event queue for this switch
	 * \details Stores press and release events.
	 */
	EventQueue<Event, ROTENC_EVENT_QUEUE_SIZE> events;

	/**
	 * \brief Timestamp of last event
	 */
	absolute_time_t tsLastEvent;

public:
	/**
	 * \brief Constructs a RotaryEncoder instance
	 * \param pinA,pinB The pins that the rotary encoder is connected to.
	 */
	RotaryEncoder(uint pinA, uint pinB);

	/**
	 * \brief Getter for Pin A number
	 * \return Returns the first pin number assigned to this rotary encoder.
	 */
	inline uint getPinA() const {return pinA;}

	/**
	 * \brief Getter for Pin B number
	 * \return Returns the second pin number assigned to this rotary encoder.
	 */
	inline uint getPinB() const {return pinB;}

	/**
	 * \brief Updates the internal state of the instance
	 * \details This method must be called in 1ms intervals in order for the
	 * rotary encoder to be monitored.
	 */
	void update();

	/**
	 * \brief Returns the event queue
	 * \return The event queue for this switch.
	 */
	inline EventQueue<Event, ROTENC_EVENT_QUEUE_SIZE>& getEvents() {return events;}
};

/**
 * \brief Size of the event queue
 */
#define POTI_EVENT_QUEUE_SIZE 8

/**
 * \brief Represents a potentiometer
 * \details A potentiometer is associated with one of the ADC-capable pins.
 * The ADC outputs a 12-bit raw value which is downscaled to the interval
 * 0..255 by this class. Some hysteresis is applied to avoid jitter.
 */
class Potentiometer
{
public:
	/**
	 * \brief Type for pototentiometer events
	 */
	struct Event
	{
		/**
		 * \brief How far did the potentiometer go up or down?
		 */
		int delta;
		/**
		 * \brief What is the potentiometer's position after the event?
		 */
		uint8_t position;
		/**
		 * \brief How long did the previous state last (in milliseconds)?
		 */
		uint duration;
	};

private:
	/**
	 * \brief Pin that is connected to the potentiometer
	 * \details Must be 26..29 since these are the only ADC-capable pins.
	 */
	uint pin;

	/**
	 * \brief Position of the potentiometer
	 */
	uint8_t position;

	/**
	 * \brief Minimum and maximum raw ADC value
	 */
	uint16_t adcMin, adcMax;

	/**
	 * \brief Hysteresis for raw ADC value
	 */
	uint16_t hysteresis;

	/**
	 * \brief Event queue for this switch
	 * \details Stores press and release events.
	 */
	EventQueue<Event, POTI_EVENT_QUEUE_SIZE> events;

	/**
	 * \brief Timestamp of last event
	 */
	absolute_time_t tsLastEvent;

	/**
	 * \brief Convert a raw ADC value to a position.
	 * \param raw A raw ADC value in the range 0..4095.
	 * \return The corresponding position in the range 0..255.
	 */
	inline uint8_t rawToPos(uint raw)
	{
		if(raw < adcMin) raw = adcMin;
		if(raw > adcMax) raw = adcMax;
		return static_cast<uint8_t>((raw - adcMin) * 256 / (adcMax - adcMin + 1));
	}

public:
	/**
	 * \brief Constructs a Potentiometer instance
	 * \param pin The pin that the potentiometer is connected to. Must be an
	 * ADC-capable pin (26..29).
	 * \param adcMin,adcMax Minimum and maximum ADC values. The potentiometer's
	 * raw values will probably not go all the way down to 0 or all the way up
	 * to 4095.
	 * \param hysteresis Hysteresis for raw ADC value.
	 * Example: Assume raw values 632..638 result in position 40 and the
	 * poti is moved from 670 down into this interval. If the value falls to or
	 * below 638-POTI_ADC_HYSTERESIS, position is set to 40. If it stays above
	 * 638-POTI_ADC_HYSTERESIS, position stops at 41.
	 */
	Potentiometer(uint pin, uint16_t adcMin, uint16_t adcMax, uint16_t hysteresis);

	/**
	 * \brief Getter for pin number
	 * \return Returns the pin number assigned to this potentiometer.
	 */
	inline uint getPin() const {return pin;}

	/**
	 * \brief Updates the internal state of the instance
	 * \details This method must be called in 1ms intervals in order for the
	 * potentiometer to be monitored.
	 */
	void update();

	/**
	 * \brief Returns the current position of the potentiometer
	 * \return The current position of the potentiometer in the interval
	 * [0,255].
	 */
	inline uint8_t getPosition() const {return position;}

	/**
	 * \brief Returns the event queue
	 * \return The event queue for this potentiometer.
	 */
	inline EventQueue<Event, POTI_EVENT_QUEUE_SIZE>& getEvents() {return events;}
};

/**
 * \brief Aggregates all the input controls
 * \details This class contains a collection of all the input controls and
 * ensures that their update() methods are called whenever necessary.
 * This class follows the SINGLETON pattern. The unique instance is created
 * by create() and can be obtained via getInstance().
 *
 */
class InputMonitor
{
public:
	/**
	 * \brief Input monitor mode
	 * \details The input monitor can operate in three modes:
	 */
	enum class Mode
	{
		/**
		 * Input controls are not monitored
		 */
		STOPPED,
		/**
		 * Switches and rotary encoders are not constantly monitored but an
		 * an interrupt is set up to wake the controller when any input is
		 * detected. No debouncing is done in this mode and eventy are not
		 * logged by the Switch and RotaryEncoder classes, i.e. they won't
		 * appear in their event queues.
		 * Potentiometers are not monitored at all since the ADC is not
		 * operating during sleep.
		 */
		SLEEPING,
		/**
		 * All input controls are monitored using a 1ms timer.
		 */
		RUNNING
	};

private:
	/**
	 * \brief Stores the single instance of this class
	 */
	static InputMonitor* instance;

	/**
	 * \brief Alarm used during Mode::RUNNING
	 */
	alarm_id_t runningAlarm;

	/**
	 * \brief Helper method for alarm callback
	 */
	static int64_t runningAlarmCb(alarm_id_t id, void* user_data) {return instance->runningAlarmCallback(id, user_data);}

	/**
	 * \brief Alarm callback function
	 */
	int64_t runningAlarmCallback(alarm_id_t id, void* user_data);

	/**
	 * \brief Helper method for GPIO callback
	 */
	static void sleepingGpioCb(uint gpio, uint32_t event_mask) {instance->sleepingGpioCallback(gpio, event_mask);}

	/**
	 * \brief Alarm callback function
	 */
	void sleepingGpioCallback(uint gpio, uint32_t event_mask);

	/**
	 * \brief Array of switches
	 */
	Switch switches[9];

	/**
	 * \brief Array of rotary encoder
	 */
	RotaryEncoder rotaryEncoders[3];

	/**
	 * \brief Array of potentiometers
	 */
	Potentiometer potentiometers[1];

	/**
	 * \brief Operating mode
	 */
	Mode mode;

	/**
	 * \brief Remember whether there has been input in Mode::SLEEPING
	 */
	bool sleepingInput;

	/**
	 * \brief Private constructor for SINGLETON pattern
	 */
	InputMonitor();

public:
	/**
	 * \brief Creates the single instance
	 * \details This method needs to be called from the correct core since it
	 * registers GPIO IRQs. The interrupt will wake up this core.
	 */
	static void create();

	/**
	 * \brief Getter for the unique instance
	 * \details create() must have been called before this method is ever
	 * invoked.
	 * \return Returns the unique instance of this class.
	 */
	static inline InputMonitor& getInstance() {assert(instance != nullptr); return *instance;}

	/**
	 * \brief Set operating mode
	 * \details This method needs to be called from the correct core since it
	 * enables/disables GPIO IRQs. The interrupt will wake up this core.
	 * \param mode The new mode that the input monitor is supposed to be
	 * operating in.
	 */
	void setMode(Mode mode);

	/**
	 * \brief Has there been input in sleeping mode?
	 * \return Returns true if input occurred since the last time the mode was
	 * switched to Mode::SLEEPING. Has no meaning when in other modes.
	 */
	inline bool hasInput() const {return sleepingInput;}

	/**
	 * \brief Number of switches
	 * \return Returns the number of switches.
	 */
	inline uint getNumSwitches() const {return sizeof(switches) / sizeof(Switch);}

	/**
	 * \brief Getter for switches
	 * \param index Index of the switch. Must be between 0 and
	 * getNumSwitches() - 1.
	 * \return Returns a reference to the switch.
	 */
	inline Switch& getSwitch(uint index)
	{
		assert(index < sizeof(switches) / sizeof(Switch));
		return switches[index];
	}

	/**
	 * \brief Number of rotary encoders
	 * \return Returns the number of rotary encoders.
	 */
	inline uint getNumRotaryEncoders() const {return sizeof(rotaryEncoders) / sizeof(RotaryEncoder);}

	/**
	 * \brief Getter for rotary encoders
	 * \param index Index of the rotary encoder. Must be between 0 and
	 * getNumRotaryEncoders() - 1.
	 * \return Returns a reference to the rotary encoder.
	 */
	inline RotaryEncoder& getRotaryEncoder(uint index)
	{
		assert(index < sizeof(rotaryEncoders) / sizeof(RotaryEncoder));
		return rotaryEncoders[index];
	}

	/**
	 * \brief Number of potentiometers
	 * \return Returns the number of potentiometers.
	 */
	inline uint getNumPotentiometers() const {return sizeof(potentiometers) / sizeof(Potentiometer);}

	/**
	 * \brief Getter for potentiometers
	 * \param index Index of the potentiometer. Must be between 0 and
	 * getNumPotentiometers() - 1.
	 * \return Returns a reference to the potentiometer.
	 */
	inline Potentiometer& getPotentiometer(uint index)
	{
		assert(index < sizeof(potentiometers) / sizeof(Potentiometer));
		return potentiometers[index];
	}
};

#endif // _INPUT_H
