#include "ITxSender.h"
#include "ITxSender_testfile.h"

#ifdef HAVE_PIGPIO
#include "ITxSender_pigpio.h"
#include "ITxSender_pigpiod.h"
#endif

std::unique_ptr<ITxSender> ITxSender::makeInstance(Type type)
{
	switch (type)
	{
	case Type::TestFile:
		return std::make_unique<ITxSender_testfile>();
#ifdef HAVE_PIGPIO
	case Type::Pigpio:
		return std::make_unique<ITxSender_pigpio>();
	case Type::Pigpiod:
		return std::make_unique<ITxSender_pigpiod>();
#endif
	default:
		return nullptr;
	}
}
