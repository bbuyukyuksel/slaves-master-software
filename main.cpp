#ifdef _MSC_VER
#include <boost/config/compiler/visualc.hpp>
#endif
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "mqtt_publisher.hpp"
#include "ram_listener.h"


void printInfo()
{
	std::cout << "\n\n------------------------------------------------------------\n";
	std::cout << "Options:\n";
	std::cout << "(R)ead  - (0)Entry / (1) Exit / (A) All Values\n";
	std::cout << "(W)rite - (0)Entry / (1) Exit - Value\n";
	std::cout << "(C)lear\n";
	std::cout << "                                      Burak BUYUKYUKSEL\n";
}

struct Camera {
	std::string name;
	DWORD entry_address;
	DWORD exit_address;
};

int main()
{
	std::string processname;
	std::string mqtt_server = "localhost";
	size_t mqtt_port = 1883;
	size_t mqtt_keep_alive = 60;
	size_t publish_time_ms = 1500;
	size_t mqtt_restart_time_sec = 5;

	// Parse JSON File
	auto cameras = std::vector<Camera>();
	{
		std::string file_content;
		std::ifstream file{ "config.json", std::ios::in };
		{
			std::string content;
			while (file >> content)
			{
				file_content += content;
			}
		}

		std::stringstream ss;
		ss << file_content;

		boost::property_tree::ptree pt;
		boost::property_tree::read_json(ss, pt);

		std::string cam_name;
		std::stringstream sstream;
		

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("ram"))
		{
			assert(v.first.empty()); // array elements have no names

			unsigned int entry_hex_value = 0;
			unsigned int exit_hex_value = 0;

			cam_name = v.second.get("name", "-");
		
			sstream << v.second.get("entry_address", "0");
			sstream >> std::hex >> entry_hex_value;
			
			sstream.str("");
			sstream.clear();

			sstream << v.second.get("exit_address", "0");
			sstream >> std::hex >> exit_hex_value;

			sstream.str("");
			sstream.clear();

			cameras.push_back(Camera{ cam_name, static_cast<DWORD>(entry_hex_value), static_cast<DWORD>(exit_hex_value) });
		}

		processname = pt.get<std::string>("processname", "");
		mqtt_server = pt.get<std::string>("mqtt_server", mqtt_server);
		mqtt_port = pt.get<size_t>("mqtt_port", mqtt_port);
		mqtt_keep_alive = pt.get<size_t>("mqtt_keep_alive", mqtt_keep_alive);
		mqtt_restart_time_sec = pt.get<size_t>("mqtt_restart_time_sec", mqtt_restart_time_sec);
		publish_time_ms = pt.get<size_t>("publish_time_ms", publish_time_ms);
	}
		
	auto mypublisher = MosquittoPublisher("teltonika-ram-listener", mqtt_server.c_str(), mqtt_port, mqtt_keep_alive, mqtt_restart_time_sec);

	RamListener listener;

	auto ok = listener.attachProc(processname.c_str());
	if (!ok)
	{
		std::cerr << "Error\n";
	}
	else {
		std::cout << "MQTT Server     : " << mqtt_server << "\n";
		std::cout << "MQTT Port       : " << mqtt_port << "\n";
		std::cout << "MQTT Keep Alive : " << mqtt_keep_alive << "\n";
		std::cout << "Restart Time    : " << mqtt_restart_time_sec << "s\n";
		std::cout << "Publish Time    : " << publish_time_ms << "ms\n";

		std::cout << "Printing all cameras ... \n";
		std::cout << std::hex;
		for (auto &v : cameras) {
			std::cout << "Cam Name: " << v.name
				<< ", Cam Entry Address: &" << v.entry_address
				<< ", Cam Exit  Address: &" << v.exit_address << "\n";
		}
		std::cout << std::dec;
		while (true) {
			mypublisher.Reconnect();

			std::string topic;
			int entry_value;
			int exit_value;

			std::cout << std::hex;
			for (const auto &v : cameras) {
				entry_value = listener.rpm<int>(v.entry_address);
				exit_value = listener.rpm<int>(v.exit_address);

				topic = "value/" + v.name;
				mypublisher.Publish(topic + "/entry", std::to_string(entry_value));
				mypublisher.Publish(topic + "/exit", std::to_string(exit_value));
			}
			std::this_thread::sleep_for(std::chrono::milliseconds::duration(publish_time_ms));
		}
	}

	return 0;
}