#ifndef AC433_IR_TX_ITXSENDER_TESTFILE_H_
#define AC433_IR_TX_ITXSENDER_TESTFILE_H_

#include "ITxSender.h"
#include <fstream>

class ITxSender_testfile : public ITxSender
{
public:
	~ITxSender_testfile() override;

	bool open(int gpioPin) override;
	void close() override;
	bool send(const std::vector<uint8_t> &signalHeader
		, const std::vector<uint8_t> &signal) override;

private:
	std::ofstream m_file;
};

#endif // AC433_IR_TX_ITXSENDER_TESTFILE_H_