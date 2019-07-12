#ifndef AC433_IR_TX_SIGNALFACTORY_H_
#define AC433_IR_TX_SIGNALFACTORY_H_

#include <cstdint>
#include <vector>

class SignalFactory
{
public:
#if 0
	enum class Mode : uint8_t
	{
		Auto   = 0x00
		, Dry  = 0x02
		, Cold = 0x03
		, Heat = 0x04
		, Fan  = 0x06
	};

	enum class Fan : uint8_t
	{
		Auto     = 0x0A
		, Silent = 0x0B
		, Mode1  = 0x03
		, Mode2  = 0x04
		, Mode3  = 0x05
		, Mode4  = 0x06
		, Mode5  = 0x07
	};

	struct Context
	{
		int temperture;
		bool isOn; 
		bool isSwing;
		bool isPowerful;
		bool isComfort;
		Mode mode;
		Fan fan;
	};

	std::vector<uint8_t> makeHeader() const;
	std::vector<uint8_t> makeCmd(const Context &context) const;
#else
	enum class Signal
	{
		Header
		, On
		, Off
	};

	std::vector<uint8_t> makeSignal(Signal signal) const;
#endif
};
#endif // AC433_IR_TX_SIGNALFACTORY_H_
