#include "functions.h"
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <cstring>
#include <iostream>
#include <utility>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <regex>
#include <inttypes.h>
#include <errno.h>
#include <netinet/tcp.h>

using std::cout;
using std::endl;
using std::string;
using std::map;
using std::vector;
using std::set;
using std::pair;
using std::min;
using std::max;
using std::copy;
using std::back_inserter;
using std::sort;
using std::regex;
using std::smatch;

int main(int argc, char *argv[]) {
    uint64_t session_id = std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
    int opt;
    string game_server = "";
    string player_name = "";
    string game_server_port = "";
    string gui_server = "";
    string gui_server_port = "";;
    gui_server_port = "20210";
    game_server_port = "2021";
    gui_server = "localhost";
    if (argc < 2) {
        fprintf(stderr, "bad propgram arguments");
        exit(1);
    }
    game_server = char_to_string(argv[1]);
    while ((opt = getopt(argc, argv, "n:p:i:r:")) != -1) {
        switch(opt) {
            case 'n':
                player_name = char_to_string(optarg);
            break;
            case 'p':
                game_server_port = char_to_string(optarg);
            break;
            case 'i':
                gui_server = char_to_string(optarg);
            break;
            case 'r':
                gui_server_port = char_to_string(optarg);
            break;
            default:
                fprintf(stderr, "bad propgram arguments");
                exit(1);
            break;
        }
    }

    if (!check_if_number(game_server_port) || !check_if_number(gui_server_port) || !check_player_name(player_name)) {
        fprintf(stderr, "bad propgram arguments");
        exit(1);
    }

    map<int, string> players;

    struct addrinfo addr_hints;
    struct addrinfo *addr_result;
    memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_flags = 0;
    addr_hints.ai_family = AF_UNSPEC;
    addr_hints.ai_socktype = SOCK_DGRAM;
    addr_hints.ai_protocol = IPPROTO_UDP;
    addr_hints.ai_addrlen = 0;
    addr_hints.ai_addr = NULL;
    addr_hints.ai_canonname = NULL;
    addr_hints.ai_next = NULL;

    if (getaddrinfo(game_server.c_str(), game_server_port.c_str(), &addr_hints, &addr_result) != 0) {
        fprintf(stderr, "bad propgram arguments");
        exit(1);
    }

    int sock_udp;
    sock_udp = socket(addr_result -> ai_family, addr_result -> ai_socktype, addr_result -> ai_protocol);
    if (sock_udp < 0) {
        fprintf(stderr, "socket error");
        exit(1);
    }

    struct timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 200;
    setsockopt(sock_udp, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    int no = 0;
    setsockopt(sock_udp, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no));

    if (connect(sock_udp, addr_result -> ai_addr, addr_result -> ai_addrlen) < 0) {
        fprintf(stderr, "connect errora");
        exit(1);
    }

    freeaddrinfo(addr_result);

    struct addrinfo addrhints;
    struct addrinfo *addrresult;
    memset(&addrhints, 0, sizeof(struct addrinfo));
    addrhints.ai_family = AF_UNSPEC;
    addrhints.ai_socktype = SOCK_STREAM;
    addrhints.ai_protocol = IPPROTO_TCP;
    addrhints.ai_addrlen = 0;
    addrhints.ai_addr = NULL;
    addrhints.ai_canonname = NULL;
    addrhints.ai_next = NULL;

    if (getaddrinfo(gui_server.c_str(), gui_server_port.c_str(), &addrhints, &addrresult) != 0) {
        fprintf(stderr, "bad propgram arguments");
        exit(1);
    }


    int sock_tcp;
    sock_tcp = socket(addrresult -> ai_family, addrresult -> ai_socktype, addrresult -> ai_protocol);
    if (sock_tcp < 0) {
        fprintf(stderr, "socket error");
        exit(1);
    }

    setsockopt(sock_tcp, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    no = 0;
    setsockopt(sock_tcp, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no));

    int nagle = 1;
    setsockopt(sock_tcp, IPPROTO_TCP, TCP_NODELAY, (char *)&nagle, sizeof(int));

    if (connect(sock_tcp, addrresult -> ai_addr, addrresult -> ai_addrlen) < 0) {
        fprintf(stderr, "connect error");
        exit(1);
    }

    freeaddrinfo(addrresult);

    struct pollfd gui_client;
    gui_client.fd = sock_tcp;
    gui_client.events = POLLIN;
    gui_client.revents = 0;

    struct pollfd client_timer;
    client_timer.fd = timerfd_create(CLOCK_REALTIME, 0);
    struct itimerspec timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_nsec = 30000000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_nsec = 0;
    timerfd_settime(client_timer.fd, 0, &timer, NULL);
    client_timer.events = POLLIN;
    client_timer.revents = 0;
    
    uint32_t expected_event_no = 0;
    uint32_t game_id = 0;
    uint32_t max_x;
    uint32_t max_y;
    int turn_direction = 0;
    bool left_key = false;
    bool right_key = false;

    while (true) {
        int ret = poll(&client_timer, 1, -1);
        if (ret > 0) {
            if (client_timer.revents & POLLIN) {
                send_comm(sock_udp, session_id, turn_direction, expected_event_no, player_name);
            }
            timerfd_settime(client_timer.fd, 0, &timer, NULL);
            client_timer.events = POLLIN;
            client_timer.revents = 0;
        }

        char *buf = (char *)malloc(sizeof(char) * BUF_SIZE);
        int len = read(gui_client.fd, buf, BUF_SIZE);
        if (len == 0) {
            fprintf(stderr, "tcp connection closed");
            exit(1);
        }
        else if (len < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "read error");
                exit(1);
            }
        }

        string buffer(buf);
        smatch sm;
        static const regex e ("(LEFT|RIGHT)(_KEY_)(DOWN|UP)(\n)");
        while (regex_search(buffer, sm, e)) {
            if (sm[0].compare("LEFT_KEY_DOWN\n") == 0) {
                turn_direction = 2;
                left_key = true;
            }
            else if (sm[0].compare("LEFT_KEY_UP\n") == 0) {
                left_key = false;
                if (right_key == true) {
                    turn_direction = 1;
                }
                else {
                    turn_direction = 0;
                }
            }
            else if (sm[0].compare("RIGHT_KEY_DOWN\n") == 0) {
                turn_direction = 1;
                right_key = true;
            }
            else if (sm[0].compare("RIGHT_KEY_UP\n") == 0) {
                right_key = false;
                if (left_key == true) {
                    turn_direction = 2;
                }
                else {
                    turn_direction = 0;
                }
            }
            buffer = sm.suffix().str();
        }

        gui_client.events = POLLIN;
        gui_client.revents = 0;
        free(buf);

        char *buff = (char *)malloc(sizeof(char) * (BUF_SIZE + 1));
        len = read(sock_udp, buff, BUF_SIZE);
        if (len < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                fprintf(stderr, "read error");
                exit(1);
            }
        }
        if (len > 0) {
            parsecomm(sock_tcp, buff, len, &expected_event_no, &players, &game_id, &max_x, &max_y);
        }
        free(buff);
    }
}