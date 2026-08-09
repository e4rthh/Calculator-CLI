#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stack>
#include <cmath>
#include <nlohmann/json.hpp>
#include "calc.cpp"
using json = nlohmann::json; // makes nlohmann::json shorter so we can just write json


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
