
//g++-13 -o my_websock my_websock.cpp -lssl -lcrypto
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/close.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>

using websocketpp::client;
using websocketpp::config::asio_client;
using websocketpp::connection_hdl;
using websocketpp::lib::error_code;
using std::cout;
using std::string;
using std::cin;

using ws_client = client<asio_client>;

struct ws_functions {
    std::function<void(ws_client&, std::atomic<bool>&)> setup_handlers;
    std::function<ws_client::connection_ptr(ws_client&, const string&, error_code&)> connect;
    std::function<void(ws_client&)> start_perpetual;
    std::function<void(ws_client&)> stop_perpetual;
    std::function<void(ws_client&)> run;
    std::function<void(ws_client&, ws_client::connection_ptr, const string&)> send_message;
};

void setup_handlers_impl(ws_client& my_client, std::atomic<bool>& connected) {
    my_client.set_open_handler([&connected](connection_hdl) {
        cout << "Connection opened" << std::endl;
        connected = true;
    });
    my_client.set_message_handler([](connection_hdl, ws_client::message_ptr msg) {
        cout << "Received: " << msg->get_payload() << std::endl;
    });
    my_client.set_close_handler([](connection_hdl) {
        cout << "Connection closed" << std::endl;
    });
    my_client.set_fail_handler([](connection_hdl) {
        cout << "Connection failed" << std::endl;
    });
}

ws_client::connection_ptr connect_impl(ws_client& my_client, const string& uri, error_code& ec) {
    ws_client::connection_ptr con = my_client.get_connection(uri, ec);
    if (!ec) {
        my_client.connect(con);
    }
    return con;
}

/* These functions are thin wrappers around methods of a ws_client object, which is likely a 
WebSocket client based on the naming and usageEach function takes a reference to 
a ws_client instance and calls a specific method on it, providing a simplified interface 
for controlling the client's behavior.

The start_perpetual_impl and stop_perpetual_impl functions start and stop the client's "perpetual" 
mode, which is a common pattern in asynchronous networking libraries to keep the event loop 
running even when there are no active connections. This is useful for long-lived applications 
that need to maintain readiness for new connections or messages.

The run_impl function calls the run method on the client, which typically starts the main 
event loop, processing incoming and outgoing messages and handling network events. 
This is often a blocking call that keeps the application running until the loop is stopped.

The send_message_impl function is responsible for sending a text message over a 
WebSocket connection. It takes a connection pointer and a message string, then 
calls the client's send method. It also provides basic error handling: if the send 
operation fails, it prints an error message to the standard output. This helps with 
debugging and monitoring the health of the WebSocket communication.

Overall, these wrapper functions abstract away direct interaction with the ws_client, 
making the codebase easier to maintain and test. They also provide a single place 
to add logging, error handling, or other cross-cutting concerns in the future.
 */
void start_perpetual_impl(ws_client& my_client) {
    my_client.start_perpetual();
}

void stop_perpetual_impl(ws_client& my_client) {
    my_client.stop_perpetual();
}

void run_impl(ws_client& my_client) {
    my_client.run();
}

void send_message_impl(ws_client& my_client, ws_client::connection_ptr con, const string& message) {
    error_code send_ec;
    my_client.send(con->get_handle(), message, websocketpp::frame::opcode::text, send_ec);
    if (send_ec) {
        cout << "Send error: " << send_ec.message() << std::endl;
    }
}

/**
 * my_websock.cpp
 * Entry point for a simple WebSocket client application.
 * g++-13 -o my_websock my_websock.cpp -lssl -lcrypto
 * This program initializes a WebSocket client, connects to a remote WebSocket server,
 * and allows the user to send messages interactively from the console. The connection
 * is managed using a set of function pointers encapsulated in the `ws_functions` struct,
 * which abstracts the implementation details of the WebSocket operations.
 *
 * Usage:
 * - Run the program.
 * - Enter messages to send to the WebSocket server.
 * - Type 'exit' to terminate the program.
 */
/**
 * @brief Entry point for the WebSocket client application.
 *
 * This function initializes a WebSocket client, sets up event handlers, and connects to a specified WebSocket server.
 * It starts the client's event loop in a separate thread and allows the user to send messages to the server via standard input.
 * The user can type 'exit' to terminate the session, after which the connection is gracefully closed and resources are cleaned up.
 *
 * @return int Returns 0 on successful execution, or -1 if a connection error occurs.
 */
int main() {
    ws_functions ws_ops = {
        setup_handlers_impl,
        connect_impl,
        start_perpetual_impl,
        stop_perpetual_impl,
        run_impl,
        send_message_impl
    };

    ws_client my_client;
    my_client.init_asio();
    std::atomic<bool> connected(false);

    ws_ops.setup_handlers(my_client, connected);

    string uri = "ws://echo.websocket.events";
    error_code ec;
    ws_client::connection_ptr con = ws_ops.connect(my_client, uri, ec);
    if (ec) {
        cout << "Connection error: " << ec.message() << std::endl;
        return -1;
    }

    ws_ops.start_perpetual(my_client);

    std::thread t([&ws_ops, &my_client]() { ws_ops.run(my_client); });

    while (!connected) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    string message;
    while (true) {
        cout << "Enter message to send (or 'exit' to quit): ";
        std::getline(cin, message);
        if (message == "exit") {
            break;
        }
        ws_ops.send_message(my_client, con, message);
    }

    // Gracefully close the connection before stopping the event loop
    error_code close_ec;
    my_client.close(con->get_handle(), websocketpp::close::status::normal, "Client exit", close_ec);
    if (close_ec) {
        cout << "Close error: " << close_ec.message() << std::endl;
    }

    ws_ops.stop_perpetual(my_client);
    t.join();
    return 0;
}