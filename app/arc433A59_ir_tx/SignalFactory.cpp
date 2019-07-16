#include "SignalFactory.h"
#include <array>

#define SIGNAL_FACTORY_DEBUG

#ifdef SIGNAL_FACTORY_DEBUG
#include <iostream>
#include <iomanip>
#endif

namespace
{
	template <std::size_t N>
	std::vector<uint8_t> convertTo(const std::array<uint8_t, N> &in)
	{
		std::vector<uint8_t> out;
		for (auto byte : in) {
			for (int i = 0; i < 8; ++i) {
				out.push_back((byte & (1 << i)) ? 1 : 0);
			}
		}

		return std::move(out);
	}

	// 8 bytes long: 11 da 27 f0 00 00 00 02
	std::vector<uint8_t> makeSignalHeader()
	{
		static std::array<uint8_t, 8> bytes = {
			0x11, 0xda, 0x27, 0xf0, 0x00, 0x00, 0x00, 0x02
		};

		return std::move(convertTo(bytes));
	}
} // anonymous namespace

std::vector<uint8_t> SignalFactory::makeHeader() const
{
	return std::move(makeSignalHeader());
}

std::vector<uint8_t> SignalFactory::makeCmd(const Context &context) const
{
	std::array<uint8_t, 19> bytes;

	// Fix header.
	bytes[0] = 0x11;
	bytes[1] = 0xda;
	bytes[2] = 0x27;
	bytes[3] = 0x00;
	bytes[4] = 0x00;

	// Mode bytes.
	bytes[5] = (context.isOn ? 0x01 : 0x00) | (static_cast<uint8_t>(context.mode) << 4);

	// Temperture bytes.
	bytes[6] = static_cast<uint8_t>(context.temperture << 1 & 0xff);

	// Fix bytes
	bytes[7] = 0x00;

	// Fan/Swing bytes
	bytes[8] = (context.isSwing ? 0x0F : 0x00) | (static_cast<uint8_t>(context.fan) << 4);

	// Fix bytes
	bytes[9] = 0x00;

	// Timer delay bytes. We don't use that.
	bytes[10] = 0x00;
	bytes[11] = 0x00;
	bytes[12] = 0x00;

	// Powerful bytes.
	bytes[13] = (context.isPowerful ? 0x01 : 0x00);

	// Fix bytes
	bytes[14] = 0x00;
	bytes[15] = 0xc0;

	// Econo bytes
	bytes[16] = 0x00;

	// Fix bytes.
	bytes[17] = 0x00;

	// Check sum bytes.
	uint32_t sum = 0;
	for (size_t i = 0; i < 18; ++i) {
		sum += static_cast<uint32_t>(bytes[i]);
	}

	bytes[18] = static_cast<uint8_t>(sum & 0xff);

#ifdef SIGNAL_FACTORY_DEBUG
	for (size_t i = 0; i < bytes.size(); ++i) {
		std::cout << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint32_t>(bytes[i]) << " ";
	}
	std::cout << std::endl;
#endif

	return std::move(convertTo(bytes));
}
