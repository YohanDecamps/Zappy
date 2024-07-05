# ZAPPY

## Purpose

The Zappy project is a simulation in which AI agents gather resources on a map
to level up.

To level up, agents need a specific amount of resources at each stage and must be at the same level or higher than other players.

### Technologies Used

This project consists of three components:

1. **Server**: Manages the game logic (**C**)
2. **GUI**: Displays the state of the game (**C++**)
3. **AI Client**: Connects to the server to participate in the game (**Python**)

There are distinct protocols for server-to-GUI communication and server-to-AI communication.

#### Encapsulation

In the GUI, the networking and protocol components are separated from the rendering component.

```cpp
#pragma once

#include <unistd.h>
#include <netinet/in.h>
#include <string>

class NetworkManager {
public:
    NetworkManager(const std::string &address, int port);
    NetworkManager(int port);
    NetworkManager(NetworkManager &&) = delete;
    NetworkManager(const NetworkManager &) = delete;
    NetworkManager &operator=(NetworkManager &&) = delete;
    NetworkManager &operator=(const NetworkManager &) = delete;
    ~NetworkManager();

    bool connectToServer();
    [[nodiscard]] ssize_t sendData(const std::string &data) const;
    [[nodiscard]] ssize_t receiveData(char *buffer, std::size_t size) const;

    [[nodiscard]] int getSocket() const { return _socket; }

private:
    std::string _address;
    int _socket;
    int _port;
    struct sockaddr_in _addr;
};
```
