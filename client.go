package main //tells the compiler that the package should compile as an executable program rather than a shared library.

//import library
import (
	"fmt" //used to print debug messages and format output.

	"io" //used for the io.WriteCloser type returned by cmd.StdinPipe().

	"os" //used for command-line arguments and interacting with operating system features.

	"os/exec" //used for running the C++ calculator as a separate child process.

	"bufio" //used for reading the C++ calculator output one line at a time.

	"encoding/json" //used for converting Go data into JSON and converting JSON from C++ back into Go data.

  // "strings" //used for processing and working with strings.

	"sync" //used to prevent multiple RPC requests from being sent at the same time.

	// tea "tea.charm.land/bubbletea/v2" //Bubble Tea main framework for building the TUI, currently not being used yet.

	// "charm.land/lipgloss/v2" //Lip Gloss library for styling text, borders, and colors, currently not being used yet.

	// "charm.land/bubbles/v2/textinput" //Text input component for the calculator UI, currently not being used yet.

	// "charm.land/bubbles/v2/spinner" //Spinner component for showing loading states, currently not being used yet.
)

//Request is the JSON message that Go sends to the C++ calculator.
type Request struct {
	ID int `json:"id"` //stores the request ID so we can match the response to the request that was sent.
	Method string `json:"method"` //stores what operation we want the C++ calculator to perform, such as "eval".
	Expr string `json:"expr"` //stores the actual math expression that we want the C++ calculator to calculate.
}


//Response is the JSON message that the C++ calculator sends back to Go.
type Response struct {
	ID int `json:"id"` //stores the same request ID that was sent so Go knows which request this response belongs to.
	OK bool `json:"ok"` //stores whether the calculation was successful or not.
	Result float64 `json:"result"` //stores the calculated number when the calculation succeeds.
	Error string `json:"error"` //stores the error message when something goes wrong.
}


//Client stores everything that Go needs to communicate with the C++ calculator.
type Client struct {
	cmd *exec.Cmd //stores the C++ process so we can start it and wait for it to finish later.

	stdin io.WriteCloser //stores the pipe that Go uses to send JSON requests to the C++ program through stdin.

	stdout *bufio.Reader //stores the reader that Go uses to read JSON responses from the C++ program through stdout.

	nextID int //stores the ID that will be given to the next request so every request can have its own ID.

	mu sync.Mutex //locks the client while a request is being sent and received so multiple requests do not mix together.
}



//NewClient starts the C++ calculator and creates the communication pipes between Go and C++.
func NewClient(path string) (*Client, error) {
	cmd := exec.Command(path) //creates a command that will start the C++ calculator executable using the path we provide.

	stdin, err := cmd.StdinPipe() //creates a pipe from Go into the C++ program so Go can send requests to C++ through stdin.
	if err != nil { //checks if creating the stdin pipe failed.
		return nil, fmt.Errorf("failed to create stdin pipe: %w", err) //returns the error so the caller knows that the C++ connection could not be created.
	}

	stdout, err := cmd.StdoutPipe() //creates a pipe from the C++ program back into Go so Go can read the responses through stdout.
	if err != nil { //checks if creating the stdout pipe failed.
		stdin.Close() //closes the stdin pipe because we cannot use the calculator if we cannot read its output.
		return nil, fmt.Errorf("failed to create stdout pipe: %w", err) //returns the error so the caller knows that the C++ connection could not be created.
	}

	err = cmd.Start() //starts the C++ calculator process with the stdin and stdout pipes connected to Go.
	if err != nil { //checks if the C++ program failed to start.
		stdin.Close() //closes the stdin pipe because the C++ process was not successfully started.
		stdout.Close() //closes the stdout pipe because the C++ process was not successfully started.
		return nil, fmt.Errorf("failed to start calculator: %w", err) //returns the error so the caller knows that the C++ calculator could not be started.
	}

	return &Client{ //creates and returns a Client containing everything needed to communicate with the running C++ calculator.
		cmd: cmd, //stores the C++ process inside the Client.
		stdin: stdin, //stores the Go-to-C++ pipe inside the Client.
		stdout: bufio.NewReader(stdout), //wraps the C++ stdout pipe in a buffered reader so we can read one JSON line at a time.
		nextID: 1, //starts request IDs at 1 because 0 is not needed for our requests.
	}, nil //returns the new Client and nil because no error happened.
}


//Eval sends a math expression to the C++ calculator and returns the calculated result.
func (c *Client) Eval(expr string) (float64, error) {
	c.mu.Lock() //locks the Client so another request cannot be sent while this request is waiting for its response.
	defer c.mu.Unlock() //automatically unlocks the Client when this function finishes.

	id := c.nextID //gets the current request ID and saves it for this request.
	c.nextID++ //increases the ID so the next request gets a different ID.

	request := Request{ //creates the JSON request that will be sent to the C++ calculator.
		ID: id, //puts the unique request ID into the JSON request.
		Method: "eval", //tells C++ that we want it to evaluate a math expression.
		Expr: expr, //puts the expression from the user into the JSON request.
	}

	err := json.NewEncoder(c.stdin).Encode(request) //converts the Go Request into JSON and sends it through stdin to the C++ calculator.
	if err != nil { //checks if sending the JSON request failed.
		return 0, fmt.Errorf("failed to send request: %w", err) //returns an error because C++ did not receive the request correctly.
	}

	var response Response //creates an empty Response where we will store the JSON response from C++.

	err = json.NewDecoder(c.stdout).Decode(&response) //reads one JSON response from C++ and converts it into the Go Response struct.
	if err != nil { //checks if reading or decoding the C++ response failed.
		return 0, fmt.Errorf("failed to read response: %w", err) //returns an error because Go could not understand the response from C++.
	}

	if response.ID != id { //checks that the response ID matches the request ID we just sent.
		return 0, fmt.Errorf("request ID mismatch: expected %d, got %d", id, response.ID) //returns an error if the response belongs to a different request.
	}

	if !response.OK { //checks if the C++ calculator reported that the calculation failed.
		return 0, fmt.Errorf("calculator error: %s", response.Error) //returns the error message that C++ sent back to Go.
	}

	return response.Result, nil //returns the calculated number to the Go program and nil because everything worked.
}


//Close closes the connection to the C++ calculator and waits for the C++ process to finish.
func (c *Client) Close() error {
	err := c.stdin.Close() //closes the stdin pipe which tells the C++ calculator that Go will not send any more requests.
	if err != nil { //checks if closing the stdin pipe failed.
		return fmt.Errorf("failed to close calculator stdin: %w", err) //returns the error if the pipe could not be closed.
	}

	err = c.cmd.Wait() //waits for the C++ calculator process to exit after its stdin has been closed.
	if err != nil { //checks if the C++ calculator exited with an error.
		return fmt.Errorf("calculator exited with error: %w", err) //returns the error if the C++ process did not exit normally.
	}

	return nil //returns nil because the C++ calculator closed successfully.
}


func main() {
	client, err := NewClient("./calculator") //starts the C++ calculator executable and creates the RPC client that will communicate with it.
	if err != nil { //checks if the C++ calculator could not be started.
		fmt.Println(err) //prints the error so we know why the calculator could not start.
		os.Exit(1) //stops the Go program with an error status because there is no backend to communicate with.
	}

	defer client.Close() //makes sure the C++ calculator is closed when the Go program finishes.

	result, err := client.Eval("-2+2.54") //sends a test expression to C++ through JSON RPC and waits for the calculated result.
	if err != nil { //checks if the calculation or communication failed.
		fmt.Println(err) //prints the error returned from the RPC client.
		os.Exit(1) //stops the program because the RPC test failed.
	}

	fmt.Println(result) //prints the result received from the C++ calculator.
}
