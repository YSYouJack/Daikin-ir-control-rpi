#ifndef AC433_IR_TX_ITXSENDER_PIGPIOD_H_
#define AC433_IR_TX_ITXSENDER_PIGPIOD_H_

#include "ITxSender.h"

class ITxSender_pigpiod : public ITxSender
{
	public:
		ITxSender_pigpiod();
		~ITxSender_pigpiod() override;

		bool open(int gpioPin) override;
		void close() override;
		bool send(const std::vector<uint8_t> &signalHeader
				, const std::vector<uint8_t> &signal) override;

	private:
		int m_pigpio = -1;
		int m_pin = -1;
		float m_carrier = 38000.0f; // 38khz
};

#endif // AC433_IR_TX_ITXSENDER_PIGPIOD_H_
