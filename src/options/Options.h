/*
 * Options.cpp
 *
 *  Created on: Nov 13, 2013
 *      Author: Jonas Kunze
 *
 *	After calling Options::Initialize all OPTION_* variables are accessible
 *	via one of the available get-methods (e.g. GetString). The value of those
 *	variables can be set by either a settings file or by calling the program
 *	with --Variable=value. The latter will overwrite the settings from the file.
 *
 *	The default settings file is defined by DEFAUL_SETTINGS_FILE and can be
 *	overwritten via calling the program with --configFile=/path/to/file
 */

#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <iostream>
#include <stdint.h>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

namespace po = boost::program_options;

/*
 * Static Options
 */
#define OPTION_HELP (char*)"help"
#define OPTION_VERBOSE (char*)"verbose"
#define OPTION_CONFIG_FILE (char*)"configFile"

#define OPTION_LOAD_FILE (char*)"loadFile"


/*
 * ZMQ
 */
#define OPTION_ZMQ_LISTEN_ADDRESS (char*)"zmqListenAddress"

class INotifiable;
class Options {
public:
	static void PrintVM(boost::program_options::variables_map vm);
	static void Initialize(int argc, char* argv[]);

	static bool Isset(char* parameter);
	static std::string GetString(char* parameter);
	static int GetInt(char* parameter);
	static std::vector<int> GetIntList(char* parameter);
	static std::vector<double> GetDoubleList(char* parameter);
	static float GetFloat(char* parameter);

	static const std::type_info& GetOptionType(std::string key);

	/*
	 * Can be used to access all Descriptions
	 */
	static std::vector<boost::shared_ptr<po::option_description> > GetOptions() {
		return desc.options();
	}

	/*
	 * Configurable Variables
	 */
	static bool VERBOSE;

	static po::options_description desc;

private:
	static po::variables_map vm;
	/*
	 * Notifies all registered objects for an option update
	 */
	static void NotifyAll();

};
#endif /* OPTIONS_H_ */
