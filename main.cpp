#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stack>
#include <cmath>
int precedence(const std::string& op) {
    if (op == "+" || op == "-") return 1; //order of operation so + - is lower than * /
    if (op == "*" || op == "/") return 2;
    if (op == "^") return 3;
    if (op == "sqrt") return 4;
    return 0;
}

bool is_operator(const std::string& t) {
    return t == "+" || t == "-" || t == "*" || t == "/" || t == "^";
}

bool isltr(const std::string& t) {
       return t != "^";
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

            if (c != ' ' && c != 's' && c != 'q' && c != 'r' && c !='t') {      
                tokens.push_back(std::string(1, c));   //push the symbol in 1, c stands for string of 1 charecter
            } else if (c != ' ') {
                current += c;
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

    for (const auto& t : tokens) {

    if (std::isdigit(t[0])) {
        output.push_back(t);
    }

    else if (t == "(") {      //if its a ( then just go to ops
        ops.push(t);
    }

    else if (t == ")") {       // put the stuff in parenthesses to the output
        while (!ops.empty() && ops.top() != "(") {
            output.push_back(ops.top());
            ops.pop();
        }
        ops.pop();   
    }

    else if (is_operator(t)) {
            if (isltr(t)) {     //check if its left to right operation if it is then just loop normally
            while (!ops.empty() &&
                   ops.top() != "(" &&
                   precedence(ops.top()) >= precedence(t)) {      //check if its empty and if its a higher weighted operation
                output.push_back(ops.top());       //if it is then push to output
                ops.pop();             
            }
        } else { // because of ltr condition if its both expo then it just gets put into the stack 
            while (!ops.empty() &&  // reason being ex 2^3^4+2. the parenthesses gets push to the stack and + so both of them go to the output but the output os now 2 3 4 ^ ^ + and now its correct order instead of 2 3 ^ 2 ^
                   ops.top() != "(" &&
                   precedence(ops.top()) > precedence(t)) {         
                output.push_back(ops.top());
                ops.pop();
            }
        }

        ops.push(t);
    }
}

while (!ops.empty()) {
    output.push_back(ops.top());
    ops.pop();
}

return output;
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
            else if (t == "-") result = rhs - lhs;
            else if (t == "/") result = rhs / lhs;
            else if (t == "^") result = std::pow (rhs, lhs);
            stack.push(result);  // push the result so we can continue finding vlaues

        }
    }
    return stack.top();  //return the value
}

int main() {
    std::string expr;

    while (std::getline(std::cin, expr)) {

        std::vector<std::string> tokens = tokenize(expr);
        std::vector<std::string> shunters = shuting_yard(tokens);
        double evals = evaluvation(shunters);

        std::cout << evals << '\n';
    }

    return 0;
}
