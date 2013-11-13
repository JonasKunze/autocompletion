/*
 * Options.cpp
 *
 *  Created on: Nov 13, 2013
 *      Author: Jonas Kunze
 */

#include "Options.h"

#include <fstream>
#include <iostream>
#include<boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

po::variables_map Options::vm;
po::options_description Options::desc("Allowed options");

std::map<std::string, std::string> SettingStore;

std::vector<INotifiable*> Options::registeredUpdatables;

#define DEFAUL_SETTINGS_FILE ".settings"

/*
 * Configurable Variables
 */
bool Options::VERBOSE;

void Options::PrintVM(po::variables_map vm) {
	using namespace po;
	for (variables_map::iterator it = vm.begin(); it != vm.end(); ++it) {
		std::cout << it->first << "=";

		const variable_value& v = it->second;
		if (!v.empty()) {
			const std::type_info& type = v.value().type();
			if (type == typeid(::std::string)) {
				std::cout << v.as<std::string>();
			} else if (type == typeid(int)) {
				std::cout << v.as<int>();
			}
		}
		std::cout << std::endl;
	}
}
/**
 * The constructor must be public but should not be called! Use Instance() as factory Method instead.
 */
void Options::Initialize(int argc, char* argv[]) {
	desc.add_options()

	(OPTION_HELP, "Produce help message")

	(OPTION_VERBOSE, "Verbose mode")

	(OPTION_CONFIG_FILE,
			po::value<std::string>()->default_value(DEFAUL_SETTINGS_FILE),
			"Config file to be loaded defining these options")

	(OPTION_LOAD_FILE, po::value<std::string>()->required(),
			"Path to the file that should be loaded in a tsv format like term\tscore")

			;

	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count(OPTION_HELP)) {
		std::cout << desc << "\n";
		exit(EXIT_SUCCESS);
	}

	if (vm.count(OPTION_CONFIG_FILE)) {
		if (!boost::filesystem::exists(
				vm[OPTION_CONFIG_FILE ].as<std::string>())) {
			std::cout << "Config file does not exist: "
					<< vm[OPTION_CONFIG_FILE ].as<std::string>() << std::endl;
		} else {

			std::cout << "======= Reading config file "
					<< vm[OPTION_CONFIG_FILE ].as<std::string>() << std::endl;

			po::store(
					po::parse_config_file<char>(
							vm[OPTION_CONFIG_FILE ].as<std::string>().data(),
							desc), vm);

			// Override file settings with argv settings
			po::store(po::parse_command_line(argc, argv, desc), vm);
		}
	}

	po::notify(vm); // Check the configuration

	std::cout << "======= Running with following configuration:" << std::endl;
	PrintVM(vm);

	VERBOSE = vm.count(OPTION_VERBOSE) > 0;
}

bool Options::Isset(char* parameter) {
	return vm.count(parameter);
}

std::string Options::GetString(char* parameter) {
	std::string str;
	if (SettingStore.find(std::string(parameter)) != SettingStore.end()) {
		str = SettingStore[parameter].c_str();
	} else {
		str = vm[parameter].as<std::string>();
	}

	size_t pos = 0;
	while ((pos = str.find("\\n", pos)) != std::string::npos) {
		str.replace(pos, 2, "\n");
		pos += 1;
	}
	return str;
}

int Options::GetInt(char* parameter) {
	if (SettingStore.find(std::string(parameter)) != SettingStore.end()) {
		return atoi(SettingStore[parameter].c_str());
	}
	return vm[parameter].as<int>();
}

std::vector<int> Options::GetIntList(char* parameter) {
	std::vector<int> values;
	std::string comaSeparatedString = GetString(parameter);

	std::vector<std::string> stringList;
	boost::split(stringList, comaSeparatedString, boost::is_any_of(","));

	for (uint i = 0; i < stringList.size(); i++) {
		std::string str = stringList[i];
		boost::trim(str);
		try {
			if (str.size() > 0) {
				values.push_back(boost::lexical_cast<int>(str));
			}
		} catch (boost::bad_lexical_cast &e) {
			std::cerr
					<< "Unable to cast '" + str
							+ "' to int! Try correct option "
							+ std::string(parameter) << std::endl;
		}
	}

	return values;
}

std::vector<double> Options::GetDoubleList(char* parameter) {
	std::vector<double> values;
	std::string comaSeparatedString = GetString(parameter);

	std::vector<std::string> stringList;
	boost::split(stringList, comaSeparatedString, boost::is_any_of(","));

	for (uint i = 0; i < stringList.size(); i++) {
		std::string str = stringList[i];
		boost::trim(str);
		try {
			if (str.size() > 0) {
				values.push_back(boost::lexical_cast<double>(str));
			}
		} catch (boost::bad_lexical_cast &e) {
			std::cerr
					<< "Unable to cast '" + str
							+ "' to double! Try correct option "
							+ std::string(parameter) << std::endl;
		}
	}

	return values;
}

float Options::GetFloat(char* parameter) {
	if (SettingStore.find(std::string(parameter)) != SettingStore.end()) {
		return atof(SettingStore[parameter].c_str());
	}
	return vm[parameter].as<float>();
}

const std::type_info& Options::GetOptionType(std::string key) {
	return vm[key].value().type();
}
