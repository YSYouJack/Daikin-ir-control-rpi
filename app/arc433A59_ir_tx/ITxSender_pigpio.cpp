#include "ITxSender_pigpio.h"
#include <pigpio.h>

#ifdef USE_WAVE
#include <vector>
#include <thread>
#include <chrono>
#endif
#include <iostream>

#ifdef USE_WAVE
namespace
{
    void addBit(int pin
        , uint32_t onDuration
        , uint32_t offDuration
        , uint32_t pulseUs
        , uint32_t spaceUs
        , std::vector<gpioPulse_t> &signals)
    {
        uint32_t period = onDuration + offDuration;
        
        // Pulse.
        uint32_t cycles = static_cast<uint32_t>(static_cast<float>(pulseUs) / 26.3f);
	std::cout << "Pulse: " << std::dec << pulseUs << ", Cycle: " << cycles << ", us: " << (onDuration + offDuration) * cycles << std::endl;
        
        for (uint32_t i = 0; i < cycles; ++i) {
            gpioPulse_t on;
            on.gpioOn = (1 << pin);
            on.gpioOff = 0;
            on.usDelay = onDuration;
        
            signals.emplace_back(on);
        
            gpioPulse_t off;
            off.gpioOn = 0;
            off.gpioOff = (1 << pin);
            off.usDelay = offDuration;
        
            signals.emplace_back(off);
        }
        
        // Space
        gpioPulse_t off;
        off.gpioOn = 0;
        off.gpioOff = 0;
        off.usDelay = ((spaceUs + period - 1) / period) * period - offDuration;
	std::cout << "Space: " << off.usDelay << std::endl; 
        
        signals.emplace_back(off);
    }
    
    inline void addStartBit(int pin
        , uint32_t onDuration
        , uint32_t offDuration
        , std::vector<gpioPulse_t> &signals)
    {
        return addBit(pin, onDuration, offDuration, 3400, 1725, signals);
    }
    
    inline void addOneBit(int pin
        , uint32_t onDuration
        , uint32_t offDuration
        , std::vector<gpioPulse_t> &signals)
    {
        return addBit(pin, onDuration, offDuration, 470, 1200, signals);
    }
    
    inline void addZeroBit(int pin
        , uint32_t onDuration
        , uint32_t offDuration
        , std::vector<gpioPulse_t> &signals)
    {
        return addBit(pin, onDuration, offDuration, 470, 400, signals);
    }
    
    inline void addEndBit(int pin
        , uint32_t onDuration
        , uint32_t offDuration
        , std::vector<gpioPulse_t> &signals)
    {
        return addBit(pin, onDuration, offDuration, 470, 14000, signals);
    }
}

ITxSender_pigpio::ITxSender_pigpio()
{
    float period = 1000000.0f / static_cast<int>(m_carrier);
    std::cout << "Period: " << period << std::endl;
    m_onUs = static_cast<uint32_t>(period * 0.5f + 0.5f);
    m_offUs = static_cast<uint32_t>(period * 0.5f + 0.5f);
}
#endif 

ITxSender_pigpio::~ITxSender_pigpio()
{
	close();
}

bool ITxSender_pigpio::open(int gpioPin)
{
	if (0 > gpioInitialise()) {
		return false;
	}

	std::cout << "gpioPin: " << gpioPin << std::endl;


	if (0 != gpioSetMode(gpioPin, PI_OUTPUT)) {
		gpioTerminate();
		return false;
	}

	m_pin = gpioPin;
	return true;
}

void ITxSender_pigpio::close()
{
	if (-1 != m_pin) {
		gpioTerminate();
	}
	m_pin = -1;
}

bool ITxSender_pigpio::send(const std::vector<uint8_t> &signalHeader
	, const std::vector<uint8_t> &signal)
{
	if (-1 == m_pin) {
		return false;
	}
	
	std::cout << "Send" << std::endl;
#ifdef USE_WAVE
    std::vector<gpioPulse_t> irSignal;

    // Start bit.
    addStartBit(m_pin, m_onUs, m_offUs, irSignal);
    
	// Header
	for (auto bit : signalHeader) {
		if (bit) {
            addOneBit(m_pin, m_onUs, m_offUs, irSignal);
		}
		else {
            addZeroBit(m_pin, m_onUs, m_offUs, irSignal);
		}
	}

	// End pulse
    addEndBit(m_pin, m_onUs, m_offUs, irSignal);

	// Start bit.
	addStartBit(m_pin, m_onUs, m_offUs, irSignal);

	// Message
	for (auto bit : signal) {
		if (bit) {
            addOneBit(m_pin, m_onUs, m_offUs, irSignal);
		}
		else {
            addZeroBit(m_pin, m_onUs, m_offUs, irSignal);
		}
	}

	// End pulse
	addEndBit(m_pin, m_onUs, m_offUs, irSignal);
    
    // Create wave.
    gpioWaveClear();

    // Add wave.
    gpioWaveAddGeneric(irSignal.size(), &irSignal[0]);
    int wave = gpioWaveCreate();
    if (wave < 0) {
        return false;
    }
    
    int result = gpioWaveTxSend(wave, PI_WAVE_MODE_ONE_SHOT);
    std::cout << "Result: " << result << std::endl;
    
    while (gpioWaveTxBusy()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
    
    gpioWaveDelete(wave);
    
#else
	// Start bit.
	gpioWrite(m_pin, 1); // pulse
	gpioSleep(PI_TIME_RELATIVE, 0, 3400);
	gpioWrite(m_pin, 0); // space
	gpioSleep(PI_TIME_RELATIVE, 0, 1725);

	// Header
	for (auto bit : signalHeader) {
		if (bit) {
			gpioWrite(m_pin, 1); // pulse
			gpioSleep(PI_TIME_RELATIVE, 0, 470);
			gpioWrite(m_pin, 0); // space
			gpioSleep(PI_TIME_RELATIVE, 0, 1200);
		}
		else {
			gpioWrite(m_pin, 1); // pulse
			gpioSleep(PI_TIME_RELATIVE, 0, 470);
			gpioWrite(m_pin, 0); // space
			gpioSleep(PI_TIME_RELATIVE, 0, 400);
		}
	}

	// End pulse
	gpioWrite(m_pin, 1); // pulse
	gpioSleep(PI_TIME_RELATIVE, 0, 470);

	// Interval between next signal
	gpioWrite(m_pin, 0); // pulse
	gpioSleep(PI_TIME_RELATIVE, 0, 13400);

	// Start bit.
	gpioWrite(m_pin, 1); // pulse
	gpioSleep(PI_TIME_RELATIVE, 0, 3400);
	gpioWrite(m_pin, 0); // space
	gpioSleep(PI_TIME_RELATIVE, 0, 1725);

	// Message
	for (auto bit : signal) {
		if (bit) {
			gpioWrite(m_pin, 1); // pulse
			gpioSleep(PI_TIME_RELATIVE, 0, 470);
			gpioWrite(m_pin, 0); // space
			gpioSleep(PI_TIME_RELATIVE, 0, 1200);
		}
		else {
			gpioWrite(m_pin, 1); // pulse
			gpioSleep(PI_TIME_RELATIVE, 0, 470);
			gpioWrite(m_pin, 0); // space
			gpioSleep(PI_TIME_RELATIVE, 0, 400);
		}
	}

	// End pulse
	gpioWrite(m_pin, 1); // pulse
	gpioSleep(PI_TIME_RELATIVE, 0, 470);
	gpioWrite(m_pin, 0);
#endif
	std::cout << "Exit Send" << std::endl;
	return true;
}
