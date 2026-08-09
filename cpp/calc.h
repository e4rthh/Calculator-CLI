#pragma once               //tells the copiler to compile once
#include <vector>
#include <string>

std::vector<std::string> tokenize(const std::string& expr);
std::vector<std::string> shuting_yard(const std::vector<std::string>& tokens);
double evaluvation(const std::vector<std::string>& tokens);                     //header file