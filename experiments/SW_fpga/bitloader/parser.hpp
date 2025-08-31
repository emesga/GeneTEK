/** This script reads the keys.json file to extract the keys, 
 * then parses the design_1.hwh file to find the corresponding values, '
 * and finally writes these key-value pairs to an output.json file 
 * in JSON format without relying on any external libraries.


Usage:  g++ -std=c++11 -o parse_hwh parser.cpp
        ./parse_hwh
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

typedef struct clock_regs_t {
    uint8_t clk_idx;
    uint8_t div0;
    uint8_t div1;
    bool en;
    uint8_t clk_src;
    uint8_t fbdiv;
    uint8_t div2;
    uint8_t pre_src;
} clock_regs_t;

// Function to trim whitespace from the start and end of a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Function to parse a JSON-like file and extract keys
void parseKeysJson(const std::string& filename, std::unordered_map<std::string, std::string>& keysMap) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening keys.json file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t colonPos = line.find(":");
        size_t quotePos = line.find("\"");

        if (colonPos != std::string::npos && quotePos != std::string::npos) {
            std::string key = trim(line.substr(quotePos + 1, colonPos - quotePos - 3));
            keysMap[key] = ""; // Initialize with empty string
        }
    }

    file.close();
}

// Function to extract the VALUE attribute from a given line
std::string extractValue(const std::string& line) {
    size_t valuePos = line.find("VALUE=\"");
    if (valuePos != std::string::npos) {
        size_t start = valuePos + 7;
        size_t end = line.find("\"", start);
        if (end != std::string::npos) {
            return line.substr(start, end - start);
        }
    }
    return "";
}

// Function to get the values of clocks
void getRegs(clock_regs_t * regs, std::unordered_map<std::string, std::string>& keysMap) {
    
    // Fill regs with the information from keysMap
    for (int i = 0; i < 4; i++) {
        regs[i].clk_idx = i;
        std::string enable_str = "PSU__FPGA_PL" + std::to_string(i) + "_ENABLE";
        regs[i].en = std::stoi(keysMap[enable_str]) == 1;
        if (!regs[i].en) {
            continue;
        }
        std::string plRefCtrlKey = "PSU__CRL_APB__PL" + std::to_string(i) + "_REF_CTRL__";
        regs[i].div0 = std::stoi(keysMap[plRefCtrlKey + "DIVISOR0"]);
        regs[i].div1 = std::stoi(keysMap[plRefCtrlKey + "DIVISOR1"]);
        std::string clk_src_str = keysMap[plRefCtrlKey + "SRCSEL"];
        if (clk_src_str == "IOPLL") {
            regs[i].clk_src = 0;
        } else if (clk_src_str == "RPLL") {
            regs[i].clk_src = 2;
        } else if (clk_src_str == "APLL") {
            regs[i].clk_src = 3;
        } else {
            throw std::runtime_error("Invalid clock source");
        }
        std::string pllRefCtrlKey = "PSU__CRL_APB__" + clk_src_str + "_CTRL__";
        regs[i].fbdiv = std::stoi(keysMap[pllRefCtrlKey + "FBDIV"]);
        regs[i].div2 = std::stoi(keysMap[pllRefCtrlKey + "DIV2"]);
        if (keysMap[pllRefCtrlKey + "SRCSEL"] != "PSS_REF_CLK") {
            throw std::runtime_error("Invalid clock source");
        }
        regs[i].pre_src = 0;
    }
}
// Function to parse the .hwh file and extract key-value pairs
void parseFile(const std::string& designFilename, const std::string& keysFilename, std::unordered_map<std::string, std::string>& keysMap) {
    // Parse the keys.json file to get the keys
    parseKeysJson(keysFilename, keysMap);

    std::ifstream file(designFilename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << designFilename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        for (auto& key : keysMap) {
            if (line.find(key.first) != std::string::npos) {
                std::string value = extractValue(line);
                if (!value.empty()) {
                    key.second = value;
                }
            }
        }
    }

    file.close();
}

// Function to write the JSON output to a file
void writeJsonOutput(const std::string& filename, const std::unordered_map<std::string, std::string>& keysMap) {
    std::ofstream jsonFile(filename);
    if (!jsonFile.is_open()) {
        std::cerr << "Error creating JSON output file" << std::endl;
        return;
    }

    jsonFile << "{\n";
    for (auto it = keysMap.begin(); it != keysMap.end(); ++it) {
        jsonFile << "  \"" << it->first << "\" : \"" << it->second << "\"";
        if (std::next(it) != keysMap.end()) {
            jsonFile << ",";
        }
        jsonFile << "\n";
    }
    jsonFile << "}\n";

    jsonFile.close();
}

// int main() {
//     // Map to store the keys and their extracted values
//     std::unordered_map<std::string, std::string> keysMap;

//     // Parse the design_1.hwh file to extract the key-value pairs using keys.json as reference
//     parseFile("design_1.hwh", "keys.json", keysMap);

//     // Write the JSON output to a file
//     writeJsonOutput("output.json", keysMap);

//     std::cout << "JSON output written to output.json" << std::endl;

//     return 0;
// }
