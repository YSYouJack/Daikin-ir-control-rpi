#include "ITxSender_pigpio.h"
#include <pigpio.h>

ITxSender_pigpio::~ITxSender_pigpio()
{
	close();
}

bool ITxSender_pigpio::open(int gpioPin)
{
	if (0 > gpioInitialise()) {
		return false;
	}

	if (0 != gpioSetMode(gpioPin, PI_OUTPUT)) {
		gpioTerminate();
		return false;
	}

	m_pin = gpioPin;
	return false;
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

	return true;
}
