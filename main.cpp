#include <iostream>   // cin / cout
#include <string>     // getline
#include <stack>      // shunting-yard
#include <sstream>    // tokenizing the input
#include <stdexcept>  // throwing errors (div by zero etc)
#include <math.h>



int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        // parse + eval line
        // for now just echo it back
        std::cout << line << "\n";
        std::cout.flush(); // important for piping
    }
}

void std::vector<std::string> tokenize(const std::string& expr);