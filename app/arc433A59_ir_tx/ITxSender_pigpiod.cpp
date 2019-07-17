#include "ITxSender_pigpiod.h"
#include <pigpiod_if2.h>

#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

namespace
{
	void addBit(int pin, float carrier, uint32_t pulseUs, uint32_t spaceUs, std::vector<gpioPulse_t> &signals)
	{
		float period = 1000000.0f / carrier;

		// Pulse.
		uint32_t cycles = static_cast<uint32_t>(static_cast<float>(pulseUs) / period + 0.5f);
		float prevTime = 0;

		for (uint32_t i = 0; i < cycles; ++i) {
			gpioPulse_t on;
			on.gpioOn = (1 << pin);
			on.gpioOff = 0;
			on.usDelay = static_cast<uint32_t>((static_cast<float>(i) + 0.5f) * period - prevTime + 0.5f);

			signals.emplace_back(on);

			gpioPulse_t off;
			off.gpioOn = 0;
			off.gpioOff = (1 << pin);
			off.usDelay = static_cast<uint32_t>(static_cast<float>(i + 1) * period - prevTime + 0.5f) - on.usDelay;
			
			signals.emplace_back(off);

			prevTime = static_cast<float>(i + 1) * period;
		}

		// Space
		cycles = static_cast<uint32_t>(static_cast<float>(spaceUs) / 26.3f + 0.5f);
		
		gpioPulse_t off;
		off.gpioOn = 0;
		off.gpioOff = 0;
		off.usDelay = static_cast<uint32_t>(static_cast<float>(cycles) * 26.3f + 0.5f);
		signals.emplace_back(off);
	}

	inline void addStartBit(int pin, float carrier, std::vector<gpioPulse_t> &signals)
	{
		return addBit(pin, carrier, 3400, 1725, signals);
	}

	inline void addOneBit(int pin, float carrier, std::vector<gpioPulse_t> &signals)
	{
		return addBit(pin, carrier, 470, 1200, signals);
	}

	inline void addZeroBit(int pin, float carrier, std::vector<gpioPulse_t> &signals)
	{
		return addBit(pin, carrier, 470, 400, signals);
	}

	inline void addEndBit(int pin, float carrier, std::vector<gpioPulse_t> &signals)
	{
		return addBit(pin, carrier, 470, 14000, signals);
	}
}

ITxSender_pigpiod::ITxSender_pigpiod()
{
}

ITxSender_pigpiod::~ITxSender_pigpiod()
{
	close();
}

bool ITxSender_pigpiod::open(int gpioPin)
{
	m_pigpio = pigpio_start(nullptr, nullptr);
	if (0 > m_pigpio) {
		std::cerr << "Error: Connect to pigpiod daemon failed!" << std::endl;
		return false;
	}

	std::cout << "gpioPin: " << gpioPin << std::endl;

	if (0 != set_mode(m_pigpio, gpioPin, PI_OUTPUT)) {
		std::cerr << "Error: Set output pin failed!" << std::endl;
		pigpio_stop(m_pigpio);
		m_pigpio = -1;
		return false;
	}

	m_pin = gpioPin;
	return true;
}

void ITxSender_pigpiod::close()
{
	if (-1 != m_pin) {
		pigpio_stop(m_pigpio);
	}
	m_pigpio = -1;
	m_pin = -1;
}

bool ITxSender_pigpiod::send(const std::vector<uint8_t> &signalHeader
		, const std::vector<uint8_t> &signal)
{
	if (-1 == m_pin) {
		return false;
	}

	std::vector<gpioPulse_t> irSignal;
#if 1
	// Start bit.
	//addStartBit(m_pin, m_carrier, irSignal);
			addOneBit(m_pin, m_carrier, irSignal);
			addOneBit(m_pin, m_carrier, irSignal);
			addOneBit(m_pin, m_carrier, irSignal);
        /*
	// Header
	for (auto bit : signalHeader) {
		if (bit) {
			addOneBit(m_pin, m_carrier, irSignal);
		}
		else {
			addZeroBit(m_pin, m_carrier, irSignal);
		}
	}
	*/
	// End pulse
	addEndBit(m_pin, m_carrier, irSignal);
#endif
#if 0
	// Start bit.
	addStartBit(m_pin, m_carrier, irSignal);

	// Message
	for (auto bit : signal) {
		if (bit) {
			addOneBit(m_pin, m_carrier, irSignal);
		}
		else {
			addZeroBit(m_pin, m_carrier, irSignal);
		}
	}

	// End pulse
	addEndBit(m_pin, m_carrier, irSignal);
#endif
	// Create wave.
	int err = wave_clear(m_pigpio);
	if (0 > err) {
		std::cerr << "Error: Clear wave " << pigpio_error(err) << "!" << std::endl;
	       	return false;	
	}

	// Add wave.
	err = wave_add_generic(m_pigpio, irSignal.size(), &irSignal[0]);
	if (0 > err) {
		std::cerr << "Error: Add wave " << pigpio_error(err) << "! " << irSignal.size() << std::endl;
	       	return false;	
	}

	int wave = wave_create(m_pigpio);
	if (wave < 0) {
		std::cerr << "Error: Create wave " << pigpio_error(err) << "!" << std::endl;
		return false;
	}

	int result = wave_send_once(m_pigpio, wave);

	while (wave_tx_busy(m_pigpio)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	wave_delete(m_pigpio, wave);

	return true;
}
