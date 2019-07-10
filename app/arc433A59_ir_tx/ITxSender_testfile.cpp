#include "ITxSender_testfile.h"

ITxSender_testfile::~ITxSender_testfile()
{
	close();
}

bool ITxSender_testfile::open(int gpioPin)
{
	m_file.open("testfile.txt");
	if (m_file.is_open()) {
		m_file << "carrier 3800" << gpioPin << std::endl;
		return true;
	}
	else {
		return false;
	}
}

void ITxSender_testfile::close()
{
	if (m_file.is_open()) {
		m_file.close();
	}
}

bool ITxSender_testfile::send(const std::vector<uint8_t> &signalHeader
	, const std::vector<uint8_t> &signal)
{
	if (!m_file.is_open()) {
		return false;
	}

	// Start bit.
	m_file << "pulse 3400" << std::endl << "space 1725" << std::endl;

	// Header
	for (auto bit : signalHeader) {
		if (bit) {
			m_file << "pulse 470" << std::endl << "space 1200" << std::endl;
		} else {
			m_file << "pulse 470" << std::endl << "space 400" << std::endl;
		}
	}

	m_file << "timeout 13400" << std::endl;

	// Start bit.
	m_file << "pulse 3400" << std::endl << "space 1725" << std::endl;

	// Message
	for (auto bit : signal) {
		if (bit) {
			m_file << "pulse 470" << std::endl << "space 1200" << std::endl;
		}
		else {
			m_file << "pulse 470" << std::endl << "space 400" << std::endl;
		}
	}

	// End pulse
	m_file << "pulse 470" << std::endl;
	return true;
}