#ifndef AC433_IR_TX_ITXSENDER_H_
#define AC433_IR_TX_ITXSENDER_H_

#include <cstdint>
#include <vector>
#include <memory>

class ITxSender
{
public:
	enum class Type
	{
		TestFile
#ifdef HAVE_PIGPIO
		, Pigpiod
#endif
	};

	virtual ~ITxSender() {}
	virtual bool open(int gpioPin) = 0;
	virtual void close() = 0;
	virtual bool send(const std::vector<uint8_t> &signalHeader
		, const std::vector<uint8_t> &signal) = 0;

	static std::unique_ptr<ITxSender> makeInstance(Type type);
};

#endif // AC433_IR_TX_ITXSENDER_H_
