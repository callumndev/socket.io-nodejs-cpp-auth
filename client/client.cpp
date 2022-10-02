#include <iostream>
#include <Windows.h>
#include "ext/sio/src/sio_client.h"
#include <websocketpp/logger/levels.hpp>
#include <thread>


void log(std::string log)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 5);
    std::cout << log << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
}

std::string input(std::string msg)
{
    std::string in;
    std::cout << msg << ": ";
    std::cin >> in;

    return in;
}

void bind_events(sio::socket::ptr socket, bool &exitApp, std::string &exitReason)
{
    socket->on("login", [&](sio::event& ev) {
        sio::message::ptr msg = ev.get_message();

        bool success = msg->get_map()["success"]->get_bool();
        if (!success)
        {
            exitApp = true;
            exitReason = "login failed";
            return;
        }

        log("\n\tAUTHENTICATED!");
        
        std::map<std::string, sio::message::ptr> userData = msg->get_map()["userData"]->get_map();
        bool isAdmin = userData["isAdmin"]->get_bool();
        std::string uname = userData["username"]->get_string();
        
        if (isAdmin)
        {
            log("\n\Welcome admin " + uname + "!");
        }
        else
        {
            log("\n\Welcome user " + uname);
        }
    });
}




int main()
{
    log("PRESS ENTER TO START");
    system("pause > nul");

    bool exitApp = false;
    bool ready = false;
    std::string exitReason;

    std::thread([&]() {
        while (true) {
            if (exitApp) {
                log("exit because " + exitReason);
                MessageBox(NULL, ((std::string)("exit because " + exitReason)).data(), "exit", 0);
                exit(EXIT_FAILURE);
                break;
            }

            Sleep(100);
        }
    }).detach();



    sio::client client;
    client.set_logs_quiet();

    client.set_open_listener([&]() {
        log("socket opened");
        ready = true;
    });
    client.set_close_listener([&](sio::client::close_reason const& reason) {
        log("socket closed");
        exitApp = true;
        exitReason = "socket closed";
    });
    client.set_fail_listener([&]() {
        log("socket fail");
        exitApp = true;
        exitReason = "socket fail";
    });
    client.connect("http://localhost:3000");
    client.set_reconnect_listener([](unsigned, unsigned) {
        log("socket reconnect");
    });

    client.set_reconnect_attempts(0);
    client.set_reconnect_delay(1);
    client.set_reconnect_delay_max(1);

    sio::socket::ptr socket = client.socket();
    bind_events(socket, exitApp, exitReason);


    // Wait for socket to connect
    while (ready == false) {
        Sleep(100);
    }


    // Ready
    std::string username = input("\nUsername");
    std::string password = input("Password");
    log("\n\tUsername: " + username + " | Password: " + password);


    log("\n\tAuthenticating...");


    sio::message::ptr user = sio::object_message::create();
    user->get_map()["username"] = sio::string_message::create(username);
    user->get_map()["password"] = sio::string_message::create(password);



    socket->emit("login", user);



    while (exitApp == false) {
        // Sleep for 100ms
        Sleep(100);
    }
    log("exit");
    system("pause");
}
