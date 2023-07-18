#pragma once
#include <string>
#include <fstream>

class Logger {
	std::string file_path;
public:
	Logger() {
		this->file_path = "LOGS.txt";
	}
	Logger(std::string file_path) {
		this->file_path = file_path;
	}
	void log (std::string message) {
		if (message.at(message.size() - 1) == '\n') {
			message = message.substr(0, message.size() - 1);
		}
		std::fstream file;
		file.open(file_path, std::ios_base::app);
		file << message << "\n";
		file.close();
	}
	void operator<< (std::string message) {
		if (message.at(message.size() - 1) == '\n') {
			message = message.substr(0, message.size() - 1);
		}
        std::fstream file;
        file.open(file_path, std::ios_base::app);
        file << message << "\n";
		std::cout << message << std::endl;
        file.close();
	}
	void info(std::string message) {
		if (message.at(message.size() - 1) == '\n') {
			message = message.substr(0, message.size() - 1);
		}
		std::fstream file;
		file.open(file_path, std::ios_base::app);
		file << "INFO\n";
		file << message << "\n";
		std::cout << message << std::endl;
		file.close();
	}
	void error(std::string message) {
		std::fstream file;
		file.open(file_path, std::ios_base::app);
		file << "ERROR\n";
		file << message << "\n";
		std::cout << message << std::endl;
		file.close();
	}
};