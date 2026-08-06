#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stack>

int precedence(const std::string& op) {
    if (op == "+" || op == "-") return 1; //order of operation so + - is lower than * /
    if (op == "*" || op == "/") return 2;
    return 0;
}

bool is_operator(const std::string& t) {
    return t == "+" || t == "-" || t == "*" || t == "/";
}

bool isltr(const std::string& t) {
    if (t == "+" || t == "-" || t == "*" || t == "/") return true;
}

std::vector<std::string> tokenize(const std::string& expr) { //function stdvector string for outputing as a vector of string,t he const std::string& expr for const std::string it just says i wont do anything to the string and we input the variable as expr
    std::vector<std::string> tokens;   //tokens vector string
    std::string current;   //string for current to pop

   for (char c : expr) {  //loop for charecter in expr (aka string)
        if (std::isdigit(c)) {   //check if thats a digit
            current += c; //put to current
        } else {
            if (!current.empty()) {   // if a symbol
                tokens.push_back(current);   //push the current number to the token 
                current.clear();  //clear the string
            }

            if (c != ' ') {      
                tokens.push_back(std::string(1, c));   //push the symbol in 1, c stands for string of 1 charecter
            }
        }
    }

    if (!current.empty()) {      //for leftovers to push to tokens
        tokens.push_back(current);
    }

    return tokens;
}

std::vector<std::string> shuting_yard(const std::vector<std::string>& tokens) {  
    std::vector<std::string>output;      //string vector
    std::stack<std::string>ops;         //stack

    for (const auto& t : tokens) {    //checks the value but dont change anything to the actual string
        if (std::isdigit(t[0])) {     //check if digit only 1st index is enough
            output.push_back(t); // if digit then just go in the string

        } else if (is_operator(t)) {    //check if its an opeartion now keything is we compare the 1st index of the vector that keeps the operation to a new one if old is higher we push it if not then we push the newest one
            if (isltr(t)) {
            while (!ops.empty() && precedence(ops.top()) >= precedence(t)) {   //check if string isnt empty and check if the thing inside vector has higher value
                output.push_back(ops.top()); //push if its higher
                ops.pop();   //remove the first element
            }   
        }   else  {
            while (!ops.empty() && precedence(ops.top()) > precedence(t)) {
                output.push_back(ops.top());
                ops.pop();
            }
            ops.push(t);
        } 
    }
    while (!ops.empty()) {
    output.push_back(ops.top()); //push the last one

    ops.pop();
    }
    return output;
  }
}
double evaluvation (const std::vector<std::string>& tokens) {
    std::stack<double> stack; //stack

    for (const auto& t : tokens) {
         if (std::isdigit(static_cast<unsigned char>(t[0]))) {    //is digit can only check for unsigned char so we just change its type to that
            stack.push(std::stod(t));   //push to stack
        } else {
            double lhs = stack.top(); stack.pop();   //left hand side
            double rhs = stack.top(); stack.pop();   //right hand side
            double result = 0;
            if (t == "*") result = lhs * rhs;
            else if (t == "+") result = lhs + rhs;
            else if (t == "-") result = lhs - rhs;
            else if (t == "/") result = lhs / rhs;
            stack.push(result);  // push the result so we can continue finding vlaues

        }
    }
    return stack.top();  //return the value
}

int main() {
    std::string expr;
    std::getline(std::cin, expr);

    std::vector<std::string> tokens = tokenize(expr);
    std::vector<std::string> shunters = shuting_yard(tokens);
    double evals = evaluvation(shunters);

    std::cout << evals << '\n';

    return 0;

}