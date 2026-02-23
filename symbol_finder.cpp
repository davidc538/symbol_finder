// symbol_finder.cpp
// will use the nm tool to search all files in the current working directory for a symbol
// to compile: g++ symbol_finder.cpp -o symbol_finder -std=c++17
// freely distribute under copypasta license

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <filesystem>
#include <functional>

// copied from https://stackoverflow.com/a/478960/983556
std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}

	return result;
}

std::vector<std::string> split(std::string input, char delimiter) {
	std::vector<std::string> ret;
	std::istringstream input_stream(input);
	std::string s;

	while (std::getline(input_stream, s, delimiter)) {
		ret.push_back(s);
	}
	
	return ret;
}

bool contains(std::string_view haystack, std::string_view needle) {
	bool ret = (haystack.find(needle) != std::string_view::npos);
	return ret;
}

std::vector<std::string> output_contains(std::vector<std::string> output, std::string search) {
	std::vector<std::string> ret;
	for (const std::string& str : output) {
		bool cont = contains(str, search);

		if (cont)
			ret.push_back(str);
	}
	
	return ret;
}

std::string trim(std::string input, char trim) {
	std::string ret = input;

	while (ret[0] == trim) {
		ret = ret.substr(1);
	}

	while (ret[ret.length() - 1] == trim) {
		ret = ret.substr(0, ret.length() - 1);
	}
	
	return ret;
}

std::string subtract_prefix(std::string input, std::string subtr) {
	std::string ret = input;
	size_t index = input.find(subtr);

	if (index == 0)
		ret = ret.substr(subtr.length());

	return ret;
}

bool search_options(int argc, char** argv, const std::string& term) {
	for (int i = 0; i < argc; i++) {
		if (term == std::string(argv[i]))
			return true;
	}
	
	return false;
}

struct nm_options {
	bool demangle;
};

void search_cwd(const nm_options& options, std::string target, std::function<void(std::string, std::string)> on_found) {
	std::string path = std::filesystem::current_path();
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		std::stringstream ss;
		ss << "nm " << entry;

		if (options.demangle)
			ss << " --demangle";
		
		std::string nm_output = exec(ss.str().c_str());
		std::vector<std::string> output_split = split(nm_output, '\n');

		std::vector<std::string> found = output_contains(output_split, target);

		for (const std::string& str : found) {
			std::stringstream ss2;
			ss2 << entry;
			on_found(ss2.str(), str);
		}
	}
}

int main(int argc, char** argv) {
	nm_options options;
	options.demangle = search_options(argc, argv, "--demangle");
	
	if (argc < 2) {
		std::cout << "no search term provided" << std::endl;
		return 0;
	}

	std::string search = argv[1];
	std::cout << "search term: " << search << std::endl;
	search_cwd(options, search, [](std::string filename, std::string symbol) {
		filename = trim(filename, '\"');

		std::stringstream ss;
		ss << std::filesystem::current_path();
		std::string path = ss.str();
		path = trim(path, '\"');

		filename = subtract_prefix(filename, path);
		filename = trim(filename, '/');

		symbol = trim(symbol, ' ');
			
		std::cout << filename << " : " << symbol << std::endl;
	});
}
