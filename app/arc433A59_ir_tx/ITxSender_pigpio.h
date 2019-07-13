#ifndef AC433_IR_TX_ITXSENDER_PIGPIO_H_
#define AC433_IR_TX_ITXSENDER_PIGPIO_H_

#include "ITxSender.h"

#define USE_WAVE

class ITxSender_pigpio : public ITxSender
{
public:
#ifdef USE_WAVE
    ITxSender_pigpio();
#endif
	~ITxSender_pigpio() override;

	bool open(int gpioPin) override;
	void close() override;
	bool send(const std::vector<uint8_t> &signalHeader
		, const std::vector<uint8_t> &signal) override;

private:
	int m_pin = -1;
#ifdef USE_WAVE
    int m_carrier = 38000; // 38khz
    uint32_t m_onUs;
    uint32_t m_offUs;
#endif
};

#endif // AC433_IR_TX_ITXSENDER_TESTFILE_H_
