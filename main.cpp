#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stack>
#include <cmath>
#include <nlohmann/json.hpp>

using json = nlohmann::json; // makes nlohmann::json shorter so we can just write json

int precedence(const std::string& op) {
    if (op == "+" || op == "-") return 1; //order of operation so + - is lower than * /
    if (op == "*" || op == "/") return 2; 
    if (op == "^") return 3;
    if (op == "sqrt") return 4;
    return 0;
}

bool is_operator(const std::string& t) {
    return t == "+" || t == "-" || t == "*" || t == "/" || t == "^" || t == "sqrt";
}

bool isltr(const std::string& t) {
       return t != "^";
}

std::vector<std::string> tokenize(const std::string& expr) { //function stdvector string for outputing as a vector of string,t he const std::string& expr for const std::string it just says i wont do anything to the string and we input the variable as expr
    std::vector<std::string> tokens;   //tokens vector string
    std::string current;   //string for current to pop

    for (size_t i = 0; i < expr.size(); ) {  //loop for charecter in expr (aka string)
        char c = expr[i];  //changed to this because we can now check in the strings instead of by letters like sqrt(4) wil now be [sqrt(4)] in which we can easily check using to compare string
        while (i < expr.size() && std::isdigit(expr[i])) {    //check numbers faster
             current += expr[i];
                i++;
            }
            if (!current.empty()) {   // if a symbol
                tokens.push_back(current);   //push the current number to the token 
                current.clear();
                continue;  //clear the string using continue to avoid having 4 be checked again in the if below
            }
            if (expr.substr(i, 4) == "sqrt") {
                 tokens.push_back("sqrt");
                 i += 4;
                 continue;
}
            if (c != ' ') {      
                tokens.push_back(std::string(1, c));//push the symbol in 1, c stands for string of 1 charecter
                i++;  
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

    else if (t == ")") {       // put the stuff in parenthesses to the output  by checking for ) then loops every thing before (
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
            while (!ops.empty() &&  // reason being ex 2^3^4+2. the parenthesses gets push to the stack and + so both of them go to the output but the output os now 2 3 4 ^ ^ + and now its correct order instead of 2 3 ^ 4 ^
                   ops.top() != "(" &&
                   precedence(ops.top()) > precedence(t)) {         
                output.push_back(ops.top());
                ops.pop();
            }
        }

        ops.push(t); //for the 1st symbol to be added
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
            if (t == "sqrt") {
                double lhs = stack.top(); stack.pop();
                stack.push(std::sqrt(lhs));
                continue;
            }
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
    std::string line; //string that stores the whole JSON line we receive from Go

    //keep reading input from Go until Go closes the input stream
    //each line is one JSON request
    //example: {"id":1,"method":"eval","expr":"2+2"}
    while (std::getline(std::cin, line)) {

        //create an empty JSON object that we will use to send the answer back to Go
        json response;

        try {

            //take the JSON string that we got from Go
            //and turn it into an actual JSON object that C++ can access
            //for example:
            //line = {"id":1,"method":"eval","expr":"2+2"}
            //request["expr"] will now give us "2+2"
            json request = json::parse(line);


            //get the id from the request
            //the id is used so Go knows which request this response belongs to
            //example:
            //Go sends id 5
            //C++ does the calculation
            //C++ sends id 5 back
            //the 0 means if the request doesn't have an id,
            //just use 0 instead of crashing
            int id = request.value("id", 0);


            //get the method from the JSON request
            //method tells C++ what Go wants it to do
            //right now we only have one method:
            //"eval" = evaluate the math expression
            //the "" means if there is no method, use an empty string
            std::string method = request.value("method", "");


            //put the same id into our response
            //this lets Go match the response to the request it sent
            response["id"] = id;


            //check what method Go requested
            //right now the only thing we support is "eval"
            if (method == "eval") {

                //get the math expression from the JSON request
                //example:
                //"expr": "2+2"
                //expr will now contain:
                //"2+2"
                std::string expr = request.value("expr", "");


                //this is where the JSON RPC connects to our actual calculator
                //we don't need to rewrite the calculator
                //we just take the expression from the JSON
                //and send it through the functions we already made


                //turn the expression into individual tokens
                //example:
                //"2+2"
                //becomes:
                //["2", "+", "2"]
                std::vector<std::string> tokens = tokenize(expr);


                //take the normal tokens and convert them into
                //the order that the evaluator needs
                //example:
                //["2", "+", "2"]
                //becomes:
                //["2", "2", "+"]
                std::vector<std::string> shunters = shuting_yard(tokens);


                //actually calculate the expression
                //example:
                //["2", "2", "+"]
                //becomes:
                //4
                double evals = evaluvation(shunters);


                //tell Go that the calculation was successful
                //true means everything went correctly
                response["ok"] = true;


                //put the calculated answer into the JSON response
                //example:
                //"result": 4
                response["result"] = evals;


            } else {

                //if the method isn't something we recognize,
                //then we can't do what Go requested
                //for example:
                //"method": "banana"
                //we don't have a banana method so we return an error
                response["ok"] = false;


                //tell Go what went wrong
                response["error"] = "Unknown method";
            }


        } catch (const std::exception& e) {

            //if something goes wrong while parsing the JSON
            //or something inside the calculator throws an error,
            //the program will come here instead of immediately crashing
            //e.what() gives us the error message
            //so we can send it back to Go
            response["ok"] = false;


            //put the actual error message into the JSON response
            response["error"] = e.what();
        }


        //convert the response JSON object back into a normal string
        //example:
        //response:
        //{"id":1,"ok":true,"result":4}
        //dump() converts the JSON object into text
        //so we can send it through stdout to Go
        std::cout << response.dump() << std::endl;


        //std::endl also flushes the output
        //this is important because we want Go to receive the response
        //immediately instead of the response sitting in an output buffer
    }


    //when the while loop ends, stdin was closed
    //this normally means the Go program closed its connection
    //so we can safely exit the C++ program
    return 0;
}
