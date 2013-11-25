/*
 * Options.h
 *
 *  Created on: Nov 13, 2013
 *      Author: Jonas Kunze
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
 * Compile time options
 */
#define MTU 1500
#define LKR_SOURCE_ID 0x24

/*
 * Static Options
 */
#define OPTION_HELP (char*)"help"
#define OPTION_VERBOSE (char*)"verbose"
#define OPTION_CONFIG_FILE (char*)"configFile"

/*
 * File
 */
#define OPTION_LOAD_FILE (char*)"loadFile"

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
