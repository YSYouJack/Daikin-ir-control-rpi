#include <iostream>
#include <exception>
#include <string>
#include <boost/program_options.hpp>

#include "ITxSender.h"
#include "SignalFactory.h"

namespace bpo = boost::program_options;

int main(int argc, char **argv)
{
	int gpio;
	int temperture;
	std::string power;
	std::string mode;
	std::string fan;
	std::string engine;

	bpo::options_description bOptions("arc433A59_ir_tx Options");
	bOptions.add_options()
		("help", "Produce help message")
		("gpio,g", bpo::value<int>(&gpio)->default_value(4), "GPIO pin number. [1-29]")
		("power,p", bpo::value<std::string>(&power)->default_value("on"), "Power mode. [on|off]")
		("comfort", "Comfort mode")
		("powerful", "Powerful mode")
		("temperture,t", bpo::value<int>(&temperture)->default_value(25), "Temperture. [12-30]")
		("mode,m", bpo::value<std::string>(&mode)->default_value("auto"), "Mode. [auto|dry|cold|heat|fan]")
		("fan,f", bpo::value<std::string>(&fan)->default_value("auto"), "Fan. [auto|silent|1|2|3|4|5]")
		("swing,s", "Swing.")
		("engine,e", bpo::value<std::string>(&engine)->default_value("file"), "[file|pigpiod]");

	bpo::variables_map vMap;
	try {
		bpo::store(bpo::parse_command_line(argc, argv, bOptions), vMap);
	}
	catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	bpo::notify(vMap);

	// Print help.
	if (vMap.count("help")) {
		std::cout << bOptions << std::endl;
		return 0;
	}

	// Read input and verified.
	SignalFactory::Context context;
	if (1 > gpio || 29 < gpio) {
		std::cerr << "Error: Invalid gpio " << gpio << "." << std::endl;
		return 1;
	}

	if ("on" == power) {
		context.isOn = true;
	} else if ("off" == power) {
		context.isOn = false;
	} else {
		std::cerr << "Error: Invalid power value " << power << "." << std::endl;
		return 1;
	}

	context.isComfort = (0 < vMap.count("comfort"));
	context.isPowerful = (0 < vMap.count("powerful"));

	if (12 <= temperture && 30 >= temperture) {
		context.temperture = temperture;
	} else {
		std::cerr << "Error: Invalid temperture " << temperture << "." << std::endl;
		return 1;
	}

	if ("auto" == mode) {
		context.mode = SignalFactory::Mode::Auto;
	}
	else if ("dry" == mode) {
		context.mode = SignalFactory::Mode::Dry;
		context.temperture = 96;
	}
	else if ("cold" == mode) {
		context.mode = SignalFactory::Mode::Cold;
	}
	else if ("heat" == mode) {
		context.mode = SignalFactory::Mode::Heat;
	}
	else if ("fan" == mode) {
		context.mode = SignalFactory::Mode::Fan;
	}
	else {
		std::cerr << "Error: Invalid mode value " << mode << "." << std::endl;
		return 1;
	}

	if ("auto" == fan) {
		context.fan = SignalFactory::Fan::Auto;
	}
	else if ("silent" == fan) {
		context.fan = SignalFactory::Fan::Silent;
	}
	else if ("1" == fan) {
		context.fan = SignalFactory::Fan::Mode1;
	}
	else if ("2" == fan) {
		context.fan = SignalFactory::Fan::Mode2;
	}
	else if ("3" == fan) {
		context.fan = SignalFactory::Fan::Mode3;
	}
	else if ("4" == fan) {
		context.fan = SignalFactory::Fan::Mode4;
	}
	else if ("5" == fan) {
		context.fan = SignalFactory::Fan::Mode5;
	}
	else {
		std::cerr << "Error: Invalid fan value " << fan << "." << std::endl;
		return 1;
	}

	context.isSwing = (0 < vMap.count("swing"));

	// Initailize the sender.
        std::unique_ptr<ITxSender> txSender;
	if ("pigpiod" == engine) {
#ifdef HAVE_PIGPIO
		txSender = ITxSender::makeInstance(ITxSender::Type::Pigpiod);
#else
		std::cerr << "Error: Cannot find pigpio on system!" << std::endl;
		return 1;
#endif
	} else if ("file" == engine) {
		txSender = ITxSender::makeInstance(ITxSender::Type::TestFile);
	} else {
		std::cerr << "Error: Invalie engine value " << engine << "." << std::endl;
		return 1;
	}

	if (!txSender) {
		std::cerr << "Error: Cannot create tx sender instance!" << std::endl;
		return 1;
	}

	if (!txSender->open(gpio)) {
		std::cerr << "Error: Cannot set gpio pin to output!" << std::endl;
		return 1;
	}

	// Get signal.
	SignalFactory signalFactory;
	auto sHeader = signalFactory.makeHeader();
	auto sCmd = signalFactory.makeCmd(context);

	if (!txSender->send(sHeader, sCmd)) {
		std::cerr << "Error: Cannot send OFF signal!" << std::endl;
		return 1;
	}

	return 0;
}
