# websocket-simple
This C++ program initializes a WebSocket client connects to a remote WebSocket server and allows the user to send messages interactively from the console.
The connection is handled using a set of function pointers encapsulated in the `ws_functions` struct,which abstracts the implementation details of the WebSocket operations.

Usage:
 * - Run the program. ./my_websocket
 * - Enter messages to send to the WebSocket server.
 * - Type 'exit' to terminate the program.
 
struct ws_functions {
    std::function<void(ws_client&, std::atomic<bool>&)> setup_handlers;
    std::function<ws_client::connection_ptr(ws_client&, const string&, error_code&)> connect;
    std::function<void(ws_client&)> start_perpetual;
    std::function<void(ws_client&)> stop_perpetual;
    std::function<void(ws_client&)> run;
    std::function<void(ws_client&, ws_client::connection_ptr, const string&)> send_message;
}
This structure of function pointer of functons is executed in the main loop. This makes the code look neat and logical to understand the program flow.
