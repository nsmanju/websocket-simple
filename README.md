# websocket-simple
This C++ program initializes a WebSocket client connects to a remote WebSocket server and allows the user to send messages interactively from the console.
The connection is handled using a set of function pointers encapsulated in the `ws_functions` struct,which abstracts the implementation details of the WebSocket operations.

Usage:
 * - Run the program. ./my_websocket
 * - Enter messages to send to the WebSocket server.
 * - Type 'exit' to terminate the program.
 
