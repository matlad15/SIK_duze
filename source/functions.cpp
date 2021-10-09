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

//crc32 table
uint32_t table[256] =
{
    0x00000000U, 0x77073096U, 0xEE0E612CU, 0x990951BAU, 0x076DC419U,
    0x706AF48FU, 0xE963A535U, 0x9E6495A3U, 0x0EDB8832U, 0x79DCB8A4U,
    0xE0D5E91EU, 0x97D2D988U, 0x09B64C2BU, 0x7EB17CBDU, 0xE7B82D07U,
    0x90BF1D91U, 0x1DB71064U, 0x6AB020F2U, 0xF3B97148U, 0x84BE41DEU,
    0x1ADAD47DU, 0x6DDDE4EBU, 0xF4D4B551U, 0x83D385C7U, 0x136C9856U,
    0x646BA8C0U, 0xFD62F97AU, 0x8A65C9ECU, 0x14015C4FU, 0x63066CD9U,
    0xFA0F3D63U, 0x8D080DF5U, 0x3B6E20C8U, 0x4C69105EU, 0xD56041E4U,
    0xA2677172U, 0x3C03E4D1U, 0x4B04D447U, 0xD20D85FDU, 0xA50AB56BU,
    0x35B5A8FAU, 0x42B2986CU, 0xDBBBC9D6U, 0xACBCF940U, 0x32D86CE3U,
    0x45DF5C75U, 0xDCD60DCFU, 0xABD13D59U, 0x26D930ACU, 0x51DE003AU,
    0xC8D75180U, 0xBFD06116U, 0x21B4F4B5U, 0x56B3C423U, 0xCFBA9599U,
    0xB8BDA50FU, 0x2802B89EU, 0x5F058808U, 0xC60CD9B2U, 0xB10BE924U,
    0x2F6F7C87U, 0x58684C11U, 0xC1611DABU, 0xB6662D3DU, 0x76DC4190U,
    0x01DB7106U, 0x98D220BCU, 0xEFD5102AU, 0x71B18589U, 0x06B6B51FU,
    0x9FBFE4A5U, 0xE8B8D433U, 0x7807C9A2U, 0x0F00F934U, 0x9609A88EU,
    0xE10E9818U, 0x7F6A0DBBU, 0x086D3D2DU, 0x91646C97U, 0xE6635C01U,
    0x6B6B51F4U, 0x1C6C6162U, 0x856530D8U, 0xF262004EU, 0x6C0695EDU,
    0x1B01A57BU, 0x8208F4C1U, 0xF50FC457U, 0x65B0D9C6U, 0x12B7E950U,
    0x8BBEB8EAU, 0xFCB9887CU, 0x62DD1DDFU, 0x15DA2D49U, 0x8CD37CF3U,
    0xFBD44C65U, 0x4DB26158U, 0x3AB551CEU, 0xA3BC0074U, 0xD4BB30E2U,
    0x4ADFA541U, 0x3DD895D7U, 0xA4D1C46DU, 0xD3D6F4FBU, 0x4369E96AU,
    0x346ED9FCU, 0xAD678846U, 0xDA60B8D0U, 0x44042D73U, 0x33031DE5U,
    0xAA0A4C5FU, 0xDD0D7CC9U, 0x5005713CU, 0x270241AAU, 0xBE0B1010U,
    0xC90C2086U, 0x5768B525U, 0x206F85B3U, 0xB966D409U, 0xCE61E49FU,
    0x5EDEF90EU, 0x29D9C998U, 0xB0D09822U, 0xC7D7A8B4U, 0x59B33D17U,
    0x2EB40D81U, 0xB7BD5C3BU, 0xC0BA6CADU, 0xEDB88320U, 0x9ABFB3B6U,
    0x03B6E20CU, 0x74B1D29AU, 0xEAD54739U, 0x9DD277AFU, 0x04DB2615U,
    0x73DC1683U, 0xE3630B12U, 0x94643B84U, 0x0D6D6A3EU, 0x7A6A5AA8U,
    0xE40ECF0BU, 0x9309FF9DU, 0x0A00AE27U, 0x7D079EB1U, 0xF00F9344U,
    0x8708A3D2U, 0x1E01F268U, 0x6906C2FEU, 0xF762575DU, 0x806567CBU,
    0x196C3671U, 0x6E6B06E7U, 0xFED41B76U, 0x89D32BE0U, 0x10DA7A5AU,
    0x67DD4ACCU, 0xF9B9DF6FU, 0x8EBEEFF9U, 0x17B7BE43U, 0x60B08ED5U,
    0xD6D6A3E8U, 0xA1D1937EU, 0x38D8C2C4U, 0x4FDFF252U, 0xD1BB67F1U,
    0xA6BC5767U, 0x3FB506DDU, 0x48B2364BU, 0xD80D2BDAU, 0xAF0A1B4CU,
    0x36034AF6U, 0x41047A60U, 0xDF60EFC3U, 0xA867DF55U, 0x316E8EEFU,
    0x4669BE79U, 0xCB61B38CU, 0xBC66831AU, 0x256FD2A0U, 0x5268E236U,
    0xCC0C7795U, 0xBB0B4703U, 0x220216B9U, 0x5505262FU, 0xC5BA3BBEU,
    0xB2BD0B28U, 0x2BB45A92U, 0x5CB36A04U, 0xC2D7FFA7U, 0xB5D0CF31U,
    0x2CD99E8BU, 0x5BDEAE1DU, 0x9B64C2B0U, 0xEC63F226U, 0x756AA39CU,
    0x026D930AU, 0x9C0906A9U, 0xEB0E363FU, 0x72076785U, 0x05005713U,
    0x95BF4A82U, 0xE2B87A14U, 0x7BB12BAEU, 0x0CB61B38U, 0x92D28E9BU,
    0xE5D5BE0DU, 0x7CDCEFB7U, 0x0BDBDF21U, 0x86D3D2D4U, 0xF1D4E242U,
    0x68DDB3F8U, 0x1FDA836EU, 0x81BE16CDU, 0xF6B9265BU, 0x6FB077E1U,
    0x18B74777U, 0x88085AE6U, 0xFF0F6A70U, 0x66063BCAU, 0x11010B5CU,
    0x8F659EFFU, 0xF862AE69U, 0x616BFFD3U, 0x166CCF45U, 0xA00AE278U,
    0xD70DD2EEU, 0x4E048354U, 0x3903B3C2U, 0xA7672661U, 0xD06016F7U,
    0x4969474DU, 0x3E6E77DBU, 0xAED16A4AU, 0xD9D65ADCU, 0x40DF0B66U,
    0x37D83BF0U, 0xA9BCAE53U, 0xDEBB9EC5U, 0x47B2CF7FU, 0x30B5FFE9U,
    0xBDBDF21CU, 0xCABAC28AU, 0x53B39330U, 0x24B4A3A6U, 0xBAD03605U,
    0xCDD70693U, 0x54DE5729U, 0x23D967BFU, 0xB3667A2EU, 0xC4614AB8U,
    0x5D681B02U, 0x2A6F2B94U, 0xB40BBE37U, 0xC30C8EA1U, 0x5A05DF1BU,
    0x2D02EF8DU
};

//crc32 algorithm https://gist.github.com/iwanders/8e1cb7b92af2ccf8d1a73450d771f483?fbclid=IwAR2bTt71Pq0XDunfLJLWSW4nRoKOvc-zqxgFyVL9-VeMouXfHi04k6dN284
uint32_t crc32(char* data, uint32_t len, uint32_t crc = 0) {
    crc = crc ^ 0xFFFFFFFFU;
    for (uint32_t i = 0; i < len; i++)
    {
        crc = table[(unsigned char)(*data) ^ (crc & 0xFF)] ^ (crc >> 8);
        data++;
    }
    crc = crc ^ 0xFFFFFFFFU;
    return crc;
}

//check if player_name is valid player name
bool check_player_name(string player_name) {
    if (player_name.size() > 20) {
        return false;
    }

    for (size_t i = 0; i < player_name.size(); i++) {
        if (player_name[i] < 33 || player_name[i] > 126) {
            return false;
        }
    }

    return true;
}

//check if player1 is lexicographically lesser than player2
bool check_order(string player1, string player2) {
    for (size_t i = 0; i < min(player1.size(), player2.size()); i++) {
        if (player1[i] < player2[i]) {
            return true;
        }
    }

    if (player1.size() < player2.size()) {
        return true;
    }

    return false;
}



//pack number to string (char *)
char *number_to_bytes(char *comm, size_t *pos, uint64_t number, uint64_t cnt) {

    if (cnt == 8) {
        number = htobe64(number);
        memcpy(comm + *pos, &number, 8);
    }
    else if (cnt == 4) {
        number = htonl(number);
        memcpy(comm + *pos, &number, 4);
    }
    else if (cnt == 1) {
        number = (htons(number) >> 8);
        memcpy(comm + *pos, &number, 1);
    }

    *pos += cnt;

    return comm;
}

//check if str is valid number
bool check_if_number(string str) {
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] > '9' || str[i] < '0') {
            return false;
        }
    }
    return true;
}

//change char *message to number
uint64_t get_bytes(char *message, size_t len, size_t *pos) {
    uint64_t number = 0;
    memcpy(&number, message + *pos, len);

    if (len == 8) {
        *pos += 8;
        return be64toh(number);
    }
    else if (len == 4) {
        *pos += 4;
        return ntohl(number);
    }
    else {
        *pos += 1;
        return (ntohs(number) >> 8);
    }
}

//send communication to server
void send_comm(int sock_udp, uint64_t session_id, int turn_direction,  
                uint32_t expected_event_no, string player_name) {

    size_t pos = 0;

    char *comm = (char *)malloc(sizeof(char) * (BUF_SIZE + 1));
    comm = number_to_bytes(comm, &pos, session_id, 8);
    comm = number_to_bytes(comm, &pos, turn_direction, 1);
    comm = number_to_bytes(comm, &pos, expected_event_no, 4);

    for (size_t i = 0; i < player_name.size(); i++) {
        comm[i + 13] = player_name[i];
    }
    comm[13 + player_name.size()] = '\0';

    if ((size_t)write(sock_udp, comm, 13 + player_name.size()) != 13 + player_name.size()) {
        fprintf(stderr, "dsadsadsa");
    }
}

//change number to string
string number_to_string(uint32_t number) {
    string res = "";
    vector<int> vec;
    while (number != 0) {
        vec.push_back(number % 10);
        number /= 10;
    }

    for (int i = vec.size() - 1; i >= 0; i--) {
        res += (vec[i] + '0');
    }

    return res;
}

//remove prefix from string (char *)
char *remove_prefix(char *mes, uint64_t b, uint64_t *l) {
    *l -= b;
    memmove(mes, mes + b, *l);
    mes[*l] = '\0';
    return mes;
}

//copy string to another string
string copy_string(string b) {
    string a = "";
    for (size_t i = 0; i < b.size(); i++) {
        a += b[i];
    }

    return a;
}

//copy map to another map
void copy_map(map<int, string> *players, map<int, string> &copy_players) {
    for (auto it = copy_players.begin(); it != copy_players.end(); it++) {
        (*players)[it -> first] = copy_string(copy_players[it -> first]);
    }

    copy_players.clear();
}

//change char to string
string char_to_string(char *c) {
    string s = "";
    for (size_t i = 0; i < strlen(c); i++) {
        s += c[i];
    }
    return s;
}

//copy string (char *) to another string (char *)
void copy_char(char **str1, const char *str2, uint32_t len) {
    for (size_t i = 0; i < len; i++) {
        (*str1)[i] = str2[i];
    }

    (*str1)[len] = '\0';

}

//parse communication from server
void parsecomm(int sock_tcp, char *mes, size_t len, uint32_t *expected_event_no, map<int, string> *players, 
    uint32_t *game_id, uint32_t *max_x, uint32_t *max_y) {

    size_t pos = 0;
    while (len - pos > 0) {

        string mes_to_gui = "";

        if (len - pos < 4) {
            fprintf(stderr, "server sent bad datagram");
            exit(1);
        }
        uint32_t game = get_bytes(mes, 4, &pos);

        if (*expected_event_no == 0)  {
            *game_id = max(*game_id, game);
        }

        while (len - pos > 0) {
            if (len - pos < 4) {
                fprintf(stderr, "server sent bad datagram");
                exit(1);
            }

            uint64_t l = get_bytes(mes, 4, &pos);

            if (l < 5) {
                fprintf(stderr, "server sent bad datagram");
                exit(1);
            }

            char *crc32s = (char *)malloc(sizeof(char) * (l + 4 + 1));
            copy_char(&crc32s, mes + pos - 4, l + 4);


            if (len - pos < 4 + l) {
                fprintf(stderr, "server sent bad datagram");
                exit(1);
            }

            uint32_t event_no = get_bytes(mes, 4, &pos);

            uint32_t event_type = get_bytes(mes, 1, &pos);

            char *event_data = (char *)malloc(sizeof(char) * (l - 5 + 1));
            copy_char(&event_data, mes + pos, l - 5);
            pos += (l - 5);

            uint32_t crc32v = get_bytes(mes, 4, &pos);
            uint32_t crc32value = crc32(crc32s, l + 4);
            free(crc32s);

            if (crc32value != crc32v) {
                return;
            }


            size_t new_pos = 0;

            if (event_type == 3) {
                if (l - 5 > 0) {
                    fprintf(stderr, "server sent bad datagram");
                    exit(1);
                }
                *game_id = 0;
                *expected_event_no = 0;
            }
            else if (event_type == 2) {
                if (l - 5 != 1) {
                    fprintf(stderr, "server sent bad datagram");
                    exit(1);
                }

                uint8_t player_nr = get_bytes(event_data, 1, &new_pos);
                if ((*players).count(player_nr) == 0) {
                    fprintf(stderr, "server sent bad datagram");
                    exit(1);
                }
                mes_to_gui = "PLAYER_ELIMINATED ";
                mes_to_gui += (*players)[player_nr];
            }
            else if (event_type == 1) {
                if (l - 5 != 9) {
                    fprintf(stderr, "server sent bad datagram");
                    return;
                }
                uint64_t player_nr = get_bytes(event_data, 1, &new_pos);
                if ((*players).count(player_nr) == 0) {
                    fprintf(stderr, "server sent bad datagram");
                    exit(1);
                }

                uint64_t xx = get_bytes(event_data, 4, &new_pos);
                uint64_t yy = get_bytes(event_data, 4, &new_pos);

                if (xx >= *max_x || yy >= *max_y) {
                    fprintf(stderr, "server sent bad datagram");
                    exit(1);
                }

                mes_to_gui = "PIXEL ";
                mes_to_gui += number_to_string(xx);
                mes_to_gui += " ";
                mes_to_gui += number_to_string(yy);
                mes_to_gui += " ";
                mes_to_gui += (*players)[player_nr];
            }
            else if (event_type == 0) {
                map<int, string>copy_players;
                mes_to_gui = "NEW_GAME ";
                int cnt = 0;
                uint32_t max_xx = get_bytes(event_data, 4, &new_pos);
                uint32_t max_yy = get_bytes(event_data, 4, &new_pos);
                mes_to_gui += number_to_string(max_xx);
                mes_to_gui += " ";
                mes_to_gui += number_to_string(max_yy);
                string player1, player2;
                player1 = "";
                player2 = "";
                for (size_t i = 8; i < l - 5; i++) {
                    if (event_data[i] == '\0') {
                        if (!check_order(player1, player2) || !check_player_name(player2)) {
                            fprintf(stderr, "server sent bad datagram");
                            exit(1);
                        }
                        else {
                            player1 = copy_string(player2);
                            mes_to_gui += " ";
                            mes_to_gui += player1;
                            string str = copy_string(player1);
                            copy_players[cnt] = str;
                            cnt++;
                            player2 = "";
                        }
                    }
                    else {
                        player2 += event_data[i];
                    }
                }
                
                if (player2 != "") {
                    fprintf(stderr, "server sent bad datagram");
                    exit(1);
                }

                if (game == *game_id && *expected_event_no == event_no && event_no == 0) {
                    *max_x = max_xx;
                    *max_y = max_yy;
                    (*players).clear();
                    copy_map(players, copy_players);
                }
                else if (game != *game_id && event_no == 0) {
                    *game_id = game;
                    *expected_event_no = 0;
                    *max_x = max_xx;
                    *max_y = max_yy;
                    (*players).clear();
                    copy_map(players, copy_players);
                }

                if ((*players).size() < 2) {
                    fprintf(stderr, "server sent bad datagram");
                    exit(1);
                }
            }
            else {
                (*expected_event_no)++;
            }
            if (game == *game_id && *expected_event_no == event_no) {
                (*expected_event_no)++;
                mes_to_gui += '\n';
                if ((size_t)write(sock_tcp, mes_to_gui.c_str(), mes_to_gui.size()) != mes_to_gui.size()) {
                    fprintf(stderr, "DSADSA");
                    exit(1);
                }
            }
        }
    }
}