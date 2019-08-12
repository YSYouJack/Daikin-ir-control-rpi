#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

using namespace std;

const string VERSION = "1.0";

void printHelp()
{
	cout << "The decoder for DAIKIN arc433 series ir remote signal." << endl
		<< "The input format comes from ir-ctl raw receiver." << endl
		<< "The output is the hex decode result." << endl
		<< endl
		<< "The input can be pipe-in or a regular file."
		<< endl
		<< "Usage: arc433A59_ir_decoder --help or arc433A59_ir_decoder -h" << endl
		<< "       arc433A59_ir_decoder --version or arc433A59_ir_decoder -v" << endl
		<< "       arc433A59_ir_decoder [file_path]" << endl;
}

int readFromStream(std::istream &in)
{
	string line;
	bool isPulse = true;
	bool isStartBit = true;
	uint32_t byte = 0;
	int byteShift = 0;
	while (!in.eof()) {
		getline(in, line);
		size_t start = line.find_first_not_of(' ');
		if (string::npos == start) {
			return 0;
		}

		size_t end = line.find_first_of(' ', start);
		if (string::npos == start) {
			return 0;
		}

		assert(end > start);
		size_t len = end - start;
		if (7 == len) {
			if (0 == line.compare(start, len, "carrier")) {
				byte = 0;
				byteShift = 0;
				isStartBit = true;
				isPulse = true;
				continue;
			} else if (0 == line.compare(start, len, "timeout")) {
				byte = 0;
				byteShift = 0;
				isStartBit = true;
				isPulse = true;
				cout << endl;
				continue;
			}
			else {
				cerr << "Error: Unknown keyword! " << line << endl;
				return 1;
			}

		} else if (5 == len) {
			if (0 == line.compare(start, len, "pulse")) {
				if (!isPulse) {
					cerr << "Error: Invalid format! "
						<< "This line should start with \"pulse\" not '" << line << "'!" << endl;
					return 1;
				} else {
					int duration = atoi(line.data() + end);
					if ((3250 <= duration && 3650 >= duration)) {
						isStartBit = true;
						isPulse = false;
						continue;
					} else if (375 > duration) {
						cerr << "Error: Pulse duration too short! " << duration << endl;
						return 1;
					} else if (800 < duration) {
						cerr << "Error: Pulse duration too long! " << duration << endl;
						return 1;
					} else {
						isPulse = false;
						continue;
					}                  
				}
			} else if (0 == line.compare(start, len, "space")) {
				if (isPulse) {
					cerr << "Error: Invalid format! "
						<< "This line should start with \"space\" not " << line << "!" << endl;
					return 1;
				} else {
					int duration = atoi(line.data() + end);
					if (25000 <= duration) {
						byte = 0;
						byteShift = 0;
						isStartBit = true;
						isPulse = true;
						cout << endl;
						continue;
					} else if (1100 <= duration && 1400 >= duration) {
						if (isStartBit) {
							cerr << "Error: Invalid startbit \"space\" duration " << duration << "us!" << endl;
							return 1;
						} else {
							byte |= (1 << byteShift);
							if (++byteShift == 8) {
								cout << setfill('0') << setw(2) << hex << byte << " ";
								byte = 0;
								byteShift = 0;
							}
						}
						isPulse = true;
						continue;
					} else if (200 <= duration && 550 >= duration) {
						if (isStartBit) {
							cerr << "Error: Invalid startbit \"space\" duration " << duration << "us!" << endl;
							return 1;
						} else {
							if (++byteShift == 8) {
								cout << setfill('0') << setw(2) << hex << byte << " ";
								byte = 0;
								byteShift = 0;
							}
						}
						isPulse = true;
						continue;
					} else if (1500 <= duration && 1950 >= duration) {
						if (isStartBit) {
							isStartBit = false;
							isPulse = true;
							byte = 0;
							byteShift = 0;
							continue;
						} else {
							cerr << "Error: Invalid \"space\" duration " << duration << "us!" << endl;
							return 1;
						}
					} else {
						cerr << "Error: Unaccepted space duration." << duration << endl;
						return 1;
					}
				}
			}
		}

		cerr << "Error: Invalid format! " << line << endl;
		return 1;
	}

	cout << endl;    
	return 0;
}

int main(int argc, char **argv)
{
	ios_base::sync_with_stdio(false);

	// Parse input.
	if (3 <= argc) {
		cerr << "Error: Invalid usage!" << endl;
		return 1;
	} else if (2 == argc) {
		string opt = argv[1];
		if ("--help" == opt || "-h" == opt) {
			printHelp();
			return 0;
		} else if ("--version" == opt || "-v" == opt) {
			cout << VERSION << endl;
			return 0;
		} else {
			// Read from file.
			ifstream file(opt);
			if (!file.is_open()) {
				cerr << "Error: Cannot open input file!" << endl;
				return 1;
			} else {
				return readFromStream(file);
			}
		}
	} else {
		// Read from stdin
		return readFromStream(cin);
	}
}
