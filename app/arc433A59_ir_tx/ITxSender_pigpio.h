#ifndef AC433_IR_TX_ITXSENDER_PIGPIO_H_
#define AC433_IR_TX_ITXSENDER_PIGPIO_H_

#include "ITxSender.h"

class ITxSender_pigpio : public ITxSender
{
public:
	~ITxSender_pigpio() override;

	bool open(int gpioPin) override;
	void close() override;
	bool send(const std::vector<uint8_t> &signalHeader
		, const std::vector<uint8_t> &signal) override;

private:
	int m_pin = -1;
};

#endif // AC433_IR_TX_ITXSENDER_TESTFILE_H_