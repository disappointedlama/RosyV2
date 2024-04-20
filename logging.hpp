#pragma once
#include <string>
#include <fstream>
using std::string;
class Logger {
	string file_path;
public:
	Logger() {
		this->file_path = "LOGS.txt";
	}
	Logger(string file_path) {
		this->file_path = file_path;
	}
	void log (string message) {
		if (message.at(message.size() - 1) == '\n') {
			message = message.substr(0, message.size() - 1);
		}
		std::fstream file;
		file.open(file_path, std::ios_base::app);
		file << message << "\n";
		file.close();
	}
	void operator<< (string message) {
		if (message.at(message.size() - 1) == '\n') {
			message = message.substr(0, message.size() - 1);
		}
        std::fstream file;
        file.open(file_path, std::ios_base::app);
        file << message << "\n";
		cout << message << endl;
        file.close();
	}
	void info(string message) {
		if (message.at(message.size() - 1) == '\n') {
			message = message.substr(0, message.size() - 1);
		}
		std::fstream file;
		file.open(file_path, std::ios_base::app);
		file << "INFO\n";
		file << message << "\n";
		cout << message << endl;
		file.close();
	}
	void error(string message) {
		std::fstream file;
		file.open(file_path, std::ios_base::app);
		file << "ERROR\n";
		file << message << "\n";
		cout << message << endl;
		file.close();
	}
};