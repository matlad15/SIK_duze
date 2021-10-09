#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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

#define BUF_SIZE 1024

//crc32 algorithm
uint32_t crc32(char* data, uint32_t len, uint32_t crc);

//check if player_name is valid player name
bool check_player_name(string player_name);

//check if player1 is lexicographically lesser than player2
bool check_order(string player1, string player2);

//pack number to string (char *)
char *number_to_bytes(char *comm, size_t *pos, uint64_t number, uint64_t cnt);

//check if str is valid number
bool check_if_number(string str);

//change char *message to number
uint64_t get_bytes(char *message, size_t len, size_t *pos);

//send communication to server
void send_comm(int sock_udp, uint64_t session_id, int turn_direction,  
                uint32_t expected_event_no, string player_name);

//change number to string
string number_to_string(uint32_t number);

//remove prefix from string (char *)
char *remove_prefix(char *mes, uint64_t b, uint64_t *l);

//copy string to another string
string copy_string(string b);

//copy map to another map
void copy_map(map<int, string> *players, map<int, string> &copy_players);

//change char to string
string char_to_string(char *c);

//copy string (char *) to another string (char *)
void copy_char(char **str1, const char *str2, uint32_t len);

//parse communication from server
void parsecomm(int sock_tcp, char *mes, size_t len, uint32_t *expected_event_no, map<int, string> *players, 
    uint32_t *game_id, uint32_t *max_x, uint32_t *max_y);

#endif /* FUNCTIONS_H */