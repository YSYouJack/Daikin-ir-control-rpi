#include "ITxSender_pigpiod.h"
#include <pigpiod_if2.h>

#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

#define USE_WAVE_CHAIN
#ifdef USE_WAVE_CHAIN
#include <algorithm>
#endif

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
	
#ifdef USE_WAVE_CHAIN
	int createStartWave(int pigpio, int pin, float carrier)
	{
		std::vector<gpioPulse_t> signals;
		addStartBit(pin, carrier, signals);
		
		int err = wave_add_generic(pigpio, signals.size(), &signals[0]);
		
		if (0 > err) {
			std::cerr << "Error: Add start-bit wave " << pigpio_error(err) << "! " << std::endl;
			return err;
		}

		int wave = wave_create(m_pigpio);
		if (wave < 0) {
			std::cerr << "Error: Create start-bit wave " << pigpio_error(err) << "!" << std::endl;
		}
		
		return wave;
	}
	
	int createEndWave(int pigpio, int pin, float carrier)
	{
		std::vector<gpioPulse_t> signals;
		addEndBit(pin, carrier, signals);
		
		int err = wave_add_generic(pigpio, signals.size(), &signals[0]);
		
		if (0 > err) {
			std::cerr << "Error: Add end-bit wave " << pigpio_error(err) << "! " << std::endl;
			return err;
		}

		int wave = wave_create(pigpio);
		if (wave < 0) {
			std::cerr << "Error: Create end-bit wave " << pigpio_error(err) << "!" << std::endl;
		}
		
		return wave;
	}
	
	std::vector<int> createByteWaves(int pigpio
		, int pin
		, float carrier
		, const std::vector<uint8_t> &signals)
	{
		assert(signals.size() % 8 == 0);
		
		std::vector<int> waves;
		size_t byteCnt = signals.size() / 8;
		for (size_t i = 0; i < byteCnt; ++i) {
			
			// Add to pulse sequences
			std::vector<gpioPulse_t> pulses;
			for (size_t j = 0; j < 8; ++j) {
				size_t id = i * 8 + j;
				if (signals[id]) {
					addOneBit(m_pin, m_carrier, pulses);
				} else {
					addZeroBit(m_pin, m_carrier, pulses);
				}
			}
			
			// Add wave
			int err = wave_add_generic(pigpio, pulses.size(), &pulses[0]);
			if (0 > err) {
				std::cerr << "Error: Add bytes wave at " << i << "th bytes! Because " 
				          << pigpio_error(err) << "!" << std::endl;
						  
				for (auto w: waves) {
					wave_delete(pigpio, w);
				}
				return std::vector<int>();
			}
			
			// Create wave
			int wave = wave_create(pigpio);
			if (wave < 0) {
				std::cerr << "Error: Create bytes wave at " << i << "th bytes! Because " 
				          << pigpio_error(err) << "!" << std::endl;
						  
				for (auto w: waves) {
					wave_delete(pigpio, w);
				}
				return std::vector<int>();
			}
			
			waves.push_back(wave);
		}
		
		return std::move(waves);
	}
	
#endif 	
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
#ifdef USE_WAVE_CHAIN
	if (-1 == m_pin) {
		return false;
	}
	
	// Create Start\End bit wave.
	int startWaveId = createStartWave(m_pigpio, m_pin, m_carrier);
	if (0 > startWaveId) {
		return false;
	}
	
	int endWaveId = createEndWave(m_pigpio, m_pin, m_carrier);
	if (0 > endWaveId) {
		wave_clear(m_pigpio);
		return false;
	}
	
	// Create bytes waves.
	std::vector<int> headerWaveIds = createByteWaves(m_pigpio, m_pin, m_carrier, signalHeader);
	if (headerWaveIds.empty()) {
		std::cerr << "Create header wave failed!"  << std:endl;
		wave_clear(m_pigpio);
		return false;
	}
	
	std::vector<int> signalWaveIds = createByteWaves(m_pigpio, m_pin, m_carrier, signal);
	if (signalWaveIds.empty()) {
		std::cerr << "Create signal wave failed!"  << std:endl;
		wave_clear(m_pigpio);
		return false;
	}
	
	// Create wave chains.
	std::vector<char> waveChain;
	
	waveChain.emplace_back(static_cast<char>(startWaveId));
	for (auto w : headerWaveIds) {
		waveChain.emplace_back(w);
	}
	waveChain.emplace_back(static_cast<char>(endWaveId));
	
	waveChain.emplace_back(static_cast<char>(startWaveId));
	for (auto w : signalWaveIds) {
		waveChain.emplace_back(w);
	}
	waveChain.emplace_back(static_cast<char>(endWaveId));
	
	// Send wave chain.
	int err = wave_chain(m_pigpio, &waveChain[0], waveChain.size());
	if (0 > err) {
		std::cerr << "Error: Send wave chain failed " << pigpio_error(err) << "!" << std::endl;
		wave_clear(m_pigpio);
		return false;
	}
	
	while (wave_tx_busy(m_pigpio)) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
	wave_clear(m_pigpio);
	return true;
#else	
	if (-1 == m_pin) {
		return false;
	}

	std::vector<gpioPulse_t> irSignal;

	// Start bit.
	addStartBit(m_pin, m_carrier, irSignal);
	
	// Header
	for (auto bit : signalHeader) {
		if (bit) {
			addOneBit(m_pin, m_carrier, irSignal);
		}
		else {
			addZeroBit(m_pin, m_carrier, irSignal);
		}
	}
	
	// End pulse
	addEndBit(m_pin, m_carrier, irSignal);

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
#endif
}
