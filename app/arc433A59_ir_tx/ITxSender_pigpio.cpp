#include "ITxSender_pigpio.h"
#include <pigpio.h>

#define USE_WAVE
#ifdef USE_WAVE
#include <vector>
#include <thread>
#include <chrono>
#endif
#include <iostream>

#ifdef USE_WAVE
namespace
{
    void generateBit(int pin
        , uint32_t pulseUs
        , uint32_t spaceUs
        , std::vector<gpioPulse_t> &signals)
    {
        gpioPulse_t pulse;
        pulse.gpioOn = (1 << pin);
        pulse.gpioOff = 0;
        pulse.usDelay = pulseUs;
        
        signals.emplace_back(pulse);
        
        gpioPulse_t space;
        space.gpioOn = 0;
        space.gpioOff = (1 << pin);
        space.usDelay = spaceUs;
        
        signals.emplace_back(space);
    }
    
    inline void addStartBit(int pin, std::vector<gpioPulse_t> &signals)
    {
        return generateBit(pin, 3400, 1725, signals);
    }
    
    inline void addOneBit(int pin, std::vector<gpioPulse_t> &signals)
    {
        return generateBit(pin, 470, 1200, signals);
    }
    
    inline void addZeroBit(int pin, std::vector<gpioPulse_t> &signals)
    {
        return generateBit(pin, 470, 400, signals);
    }
    
    inline void addEndBit(int pin, std::vector<gpioPulse_t> &signals)
    {
        return addBit(pin, 470, 13400, signals);
    }
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
    addStartBit(m_pin, irSignal);
    
	// Header
	for (auto bit : signalHeader) {
		if (bit) {
            addOneBit(m_pin, irSignal);
		}
		else {
            addZeroBit(m_pin, irSignal);
		}
	}

	// End pulse
    addEndBit(m_pin, irSignal);

	// Start bit.
	addStartBit(m_pin, irSignal);

	// Message
	for (auto bit : signal) {
		if (bit) {
            addOneBit(m_pin, irSignal);
		}
		else {
            addZeroBit(m_pin, irSignal);
		}
	}

	// End pulse
	addEndBit(m_pin, irSignal);
    
    // Create wave.
    gpioWaveClear();

    // Add wave.
    gpioWaveAddGeneric(irSignal.size(), &irSignal[0]);
    int waveID = gpioWaveCreate();
    if (wave < 0) {
        return false;
    }
    
    int result = gpioWaveTxSend(waveID, PI_WAVE_MODE_ONE_SHOT);
    std::cout << "Result: " << result << std::endl;
    
    while (gpioWaveTxBusy()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
    
    gpioWaveDelete(waveID);
    
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
