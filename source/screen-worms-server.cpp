#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
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

#define QUEUE_LENGTH 5
#define _POSIX_OPEN_MAX 25
#define PLAYER_TIMEOUT 2
#define BUF_SIZE 548

static const unsigned int crc32_table[] =
{
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
  0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
  0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
  0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
  0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
  0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
  0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
  0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
  0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
  0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
  0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
  0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
  0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
  0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
  0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
  0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
  0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
  0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
  0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
  0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
  0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
  0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
  0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
  0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
  0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
  0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
  0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
  0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
  0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
  0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
  0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
  0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
  0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
  0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
  0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
  0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
  0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
  0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
  0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
  0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
  0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
  0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
  0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

//algorytm crc32
unsigned int crc32 (uint8_t *buf, int len)
{
    unsigned int crc = 0;
    while (len--) {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
        buf++;
    }

    return crc;
}

//struktura gry
typedef struct game_s {
    int game_id;
    int seed;
    int seed_number;
    bool game_started;
    int turning_speed;
    int maxx;
    int maxy;

    pollfd client[25];

    pollfd start_turn;
    struct itimerspec new_turn_timer;

    set<int> active_players; //grajacy gracze
    set<int> not_active_players; //gracze, ktorzy nie wcisneli jeszcze strzalki
    set<int> observers; //obserwatorzy
    set<int> disconnected; //gracze rozlaczeni w trakcie gry
    set<int> all_players; //wszyscy gracze

    map<pair<int, int>, bool> eaten_pixels; //mapa zjedzonych pixeli
    map<string, int> client_index; //indexy klientow o podanym adresie
    map<int, string> player_name; //nazwa gracza o podanym indeksie
    map<int, pair<float, float> > worm_coordinates; //pozycja robaka gracza o danym indeksie
    map<int, vector<uint8_t> > events; //mapa eventow
    map<string, int> playing_players; //indeks gracza o danej nazwie
    map<int, int> turn_direction; //mapa turn_direction
    map<int, int> worm_direction; //kierunek robaka
    map<string, int> session_id_map; //mapa session_id

} game_s;

//spakowanie 8-bajtowej liczby do vectora bajtow
vector<uint8_t> number_to_8bytes(uint64_t number) {
    number = htobe64(number);
    vector<uint8_t>result;
    uint8_t byte1 = (number >> 56) % (1 << 8);
    uint8_t byte2 = (number >> 48) % (1 << 8);
    uint8_t byte3 = (number >> 40) % (1 << 8);
    uint8_t byte4 = (number >> 32) % (1 << 8);
    uint8_t byte5 = (number >> 24) % (1 << 8);
    uint8_t byte6 = (number >> 16) % (1 << 8);
    uint8_t byte7 = (number >> 8) % (1 << 8);
    uint8_t byte8 = number % (1 << 8);

    result.push_back(byte1);
    result.push_back(byte2);
    result.push_back(byte3);
    result.push_back(byte4);
    result.push_back(byte5);
    result.push_back(byte6);
    result.push_back(byte7);
    result.push_back(byte8);

    return result;
}

//spakowanie 4-bajtowej liczby do vectora bajtow
vector<uint8_t> number_to_4bytes(uint32_t number) {
    number = htobe64(number);
    vector<uint8_t>result;
    uint8_t byte1 = (number >> 24) % (1 << 8);
    uint8_t byte2 = (number >> 16) % (1 << 8);
    uint8_t byte3 = (number >> 8) % (1 << 8);
    uint8_t byte4 = number % (1 << 8);

    result.push_back(byte1);
    result.push_back(byte2);
    result.push_back(byte3);
    result.push_back(byte4);

    return result;
}

//spakowanie 1-bajtowej liczby do vectora bajtow
vector<uint8_t> number_to_byte(uint8_t number) {
    number = htobe64(number);
    vector<uint8_t>result;
    uint8_t byte1 = number % (1 << 8);

    result.push_back(byte1);

    return result;
}

//funkcja sprawdzajaca, czy dany string jest liczba
bool check_if_number(char *str) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] > '9' || str[i] < '0') {
            return false;
        }
    }
    return true;
}

//funkcja generujaca game_id
uint32_t rand_gen(game_s *game) {
    (game -> seed_number)++;
    if ((game -> seed_number) == 0) {
        return game -> seed;
    }
    else {
        return ((game -> seed) * 279410273) % 4294967291;
    }
}

//tworzenie zdarzenia - PLAYER_ELIMINATED
void player_eliminated(game_s *game, int ind) {
    (game -> active_players).erase(ind);
    (game -> not_active_players).insert(ind);

    vector<uint8_t> comm;
    uint32_t next = (game -> events).size();
    vector<uint8_t> len = number_to_4bytes(6);
    vector<uint8_t> event_no = number_to_4bytes(next);
    vector<uint8_t> event_type = number_to_byte(2);
    vector<uint8_t> player = number_to_byte(ind);

    copy(len.begin(), len.end(), back_inserter(comm));
    copy(event_no.begin(), event_no.end(), back_inserter(comm));
    copy(event_type.begin(), event_type.end(), back_inserter(comm));
    copy(player.begin(), player.end(), back_inserter(comm));

    //obliczanie crc
    uint32_t crc = crc32(&comm[0], comm.size());
    vector<uint8_t> crc_32 = number_to_4bytes(crc);
    copy(crc_32.begin(), crc_32.end(), back_inserter(comm));

    (game -> events)[next] = comm;
}

//tworzenie zdarzenia - PIXEL
void pixel(game_s *game, int ind) {
    vector<uint8_t> comm;
    uint32_t next = (game -> events).size();
    vector<uint8_t> len = number_to_4bytes(14);
    vector<uint8_t> event_no = number_to_4bytes(next);
    vector<uint8_t> event_type = number_to_byte(1);
    vector<uint8_t> player = number_to_byte(ind);
    vector<uint8_t> maxx = number_to_4bytes((game -> worm_coordinates)[ind].first);
    vector<uint8_t> maxy = number_to_4bytes((game -> worm_coordinates)[ind].second);

    copy(len.begin(), len.end(), back_inserter(comm));
    copy(event_no.begin(), event_no.end(), back_inserter(comm));
    copy(event_type.begin(), event_type.end(), back_inserter(comm));
    copy(player.begin(), player.end(), back_inserter(comm));
    copy(maxx.begin(), maxx.end(), back_inserter(comm));
    copy(maxy.begin(), maxy.end(), back_inserter(comm));

    //obliczanie crc
    uint32_t crc = crc32(&comm[0], comm.size());
    vector<uint8_t> crc_32 = number_to_4bytes(crc);
    copy(crc_32.begin(), crc_32.end(), back_inserter(comm));

    (game -> events)[next] = comm;
}

//tworzenie zdarzenia - NEW_GAME
void new_game(game_s *game) {
    vector<uint8_t> comm;
    uint32_t next = (game -> events).size();

    //pobieranie graczy
    vector<uint8_t> player_name;
    vector<string> names;
    for (auto el : game -> player_name) {
        if ((game -> active_players).find(el.first) != (game -> active_players).end()
            || (game -> not_active_players).find(el.first) != (game -> not_active_players).end()) {

                string p_name = el.second;
                p_name += '\0';
                names.push_back(el.second);
        }
    }

    //sortowanie graczy po nazwie
    sort(names.begin(), names.end());

    vector<uint8_t> event_no = number_to_4bytes(next);
    vector<uint8_t> event_type = number_to_byte(0);
    vector<uint8_t> maxx = number_to_4bytes(game -> maxx);
    vector<uint8_t> maxy = number_to_4bytes(game -> maxy);
    vector<uint8_t> pl_names;

    //tworzenie listy graczy
    for (size_t i = 0; i < names.size(); i++) {
        copy(names[i].begin(), names[i].end(), back_inserter(pl_names));
    }
    vector<uint8_t> len = number_to_4bytes(13 + pl_names.size());


    copy(len.begin(), len.end(), back_inserter(comm));
    copy(event_no.begin(), event_no.end(), back_inserter(comm));
    copy(event_type.begin(), event_type.end(), back_inserter(comm));
    copy(maxx.begin(), maxx.end(), back_inserter(comm));
    copy(maxy.begin(), maxy.end(), back_inserter(comm));
    copy(pl_names.begin(), pl_names.end(), back_inserter(comm));

    //obliczanie crc
    uint32_t crc = crc32(&comm[0], comm.size());
    vector<uint8_t> crc_32 = number_to_4bytes(crc);
    copy(crc_32.begin(), crc_32.end(), back_inserter(comm));


    (game -> events)[next] = comm;
}

//tworzenie zdarzenia - GAME_OVER
void game_over(game_s *game) {
    vector<uint8_t> comm;
    uint32_t next = (game -> events).size();
    vector<uint8_t> len = number_to_4bytes(5);
    vector<uint8_t> event_no = number_to_4bytes(next);
    vector<uint8_t> event_type = number_to_byte(3);

    copy(len.begin(), len.end(), back_inserter(comm));
    copy(event_no.begin(), event_no.end(), back_inserter(comm));
    copy(event_type.begin(), event_type.end(), back_inserter(comm));

    //obliczanie crc
    uint32_t crc = crc32(&comm[0], comm.size());
    vector<uint8_t> crc_32 = number_to_4bytes(crc);
    copy(crc_32.begin(), crc_32.end(), back_inserter(comm));

    (game -> events)[next] = comm;
}

//funkcja liczaca przesuniecie robaka
float bin_search(float tangens) {
    float a = 1.0;
    float b = 0.0;
    float eps = 0.00005;
    while (a - b > eps) {
        float s = (a + b) / 2.0;
        if (s / (1.0 - s) > tangens) {
            a = s;
        }
        else {
            b = s;
        }
    }

    return a;
}

//funkcja obslugujaca nowa ture
void new_turn(game_s *game) {
    //czy gra wystartowala
    if (!(game -> game_started)) {
        return;
    }

    poll(&(game -> start_turn), 1, 0);

    //czy czas zaczac nowa ture
    if (!((game -> start_turn).revents & POLLIN)) {
        return;
    }

    //kolejne ruchy graczy
    for (auto el : game -> playing_players) {
        if ((game -> turn_direction)[el.second] == 1) {
            (game -> worm_direction[el.second]) += (game -> turning_speed);
        }
        else if ((game -> turn_direction)[el.second] == 2) {
            (game -> worm_direction[el.second]) -= (game -> turning_speed);
        }

        //obliczanie nowej pozycji robaka
        float x = (game -> worm_coordinates)[el.second].first;
        float y = (game -> worm_coordinates)[el.second].second;
        int old_x = (int)x;
        int old_y = (int)y;
        int d = (game -> worm_direction[el.second]);
        if (d == 0) {
            x += 1.0;
        }
        else if (d == 90) {
            y += 1.0;
        }
        else if (d == 180) {
            x -= 1.0;
        }
        else if (d == 270) {
            y -= 1.0;
        }
        else if (d > 270) {
            d = d - 270;
            float tangens = tan((float)d * 0.0175);
            float result = bin_search(tangens);
            x += result;
            y -= (1.0 - result);
        }
        else if (d > 180) {
            d = d - 180;
            float tangens = tan((float)d * 0.0175);
            float result = bin_search(tangens);
            y -= result;
            x -= (1.0 - result);
        }
        else if (d > 90) {
            d = d - 90;
            float tangens = tan((float)d * 0.0175);
            float result = bin_search(tangens);
            x -= result;
            y += (1.0 - result);
        }
        else {
            float tangens = tan((float)d * 0.0175);
            float result = bin_search(tangens);
            y += result;
            x += (1.0 - result);
        }
        (game -> worm_coordinates)[el.second] = {x, y};
        int _x = (int)x;
        int _y = (int)y;

        //robak nie zmienil piksela
        if (_x == old_x && _y == old_y) {
            continue;
        }
        //jezeli dany piksel jest juz zjedzony
        else if ((game -> eaten_pixels).count({_x, _y})) {
            player_eliminated(game, el.second);
        }
        //jezeli robak wyszedl za plansze
        else if (_x > (game -> maxx) - 1 || _y > (game -> maxy) - 1 || (game -> maxx) < 0 || (game -> maxy) < 0) {
            player_eliminated(game, el.second);
        }
        else {
            (game -> eaten_pixels)[{_x, _y}] = true;
            pixel(game, el.second);
        }

    }
}

//funkcja tworzaca nowa gre
void end_game(game_s *game) {
    //wrzucenie active_players i observers do not_active_players
    set<int>::iterator it;
    for (it = (game -> observers).begin(); it != (game -> observers).end(); it++) {
        (game -> not_active_players).insert(*it);
    }

    for (it = (game -> active_players).begin(); it != (game -> active_players).end(); it++) {
        (game -> not_active_players).insert(*it);
    }

    //oczyszczanie setow
    (game -> observers).clear();
    (game -> active_players).clear();
    (game -> disconnected).clear();

    //rozpoczeto gre
    (game -> game_started) = true;

    //oczyszczanie map
    game -> eaten_pixels.clear();
    game -> worm_coordinates.clear();
    game -> events.clear();
    game -> turn_direction.clear();
    game -> worm_direction.clear();
}

void start_game(game_s *game, uint32_t rounds_per_sec) {
    //stworzenie timera dla odliczania czasu miedzy turami
    (game -> start_turn).fd = timerfd_create(CLOCK_REALTIME, 0);
    (game -> new_turn_timer).it_interval.tv_sec = 0;
    (game -> new_turn_timer).it_interval.tv_nsec = 0;
    (game -> new_turn_timer).it_value.tv_sec = 0;
    (game -> new_turn_timer).it_value.tv_nsec = 1000000000 / rounds_per_sec;
    timerfd_settime((game -> start_turn).fd, 0, &(game -> new_turn_timer), NULL);

    //nowe zdarzenie
    new_game(game);

    //ustawienie game_id
    (game -> seed_number) = -1;
    (game -> game_id) = rand_gen(game);

    //inicjalizacja robakow
    for (auto el : game -> playing_players) {
        float x = (float)(rand_gen(game) % (game -> maxx)) + 0.5;
        float y = (float)(rand_gen(game) % (game -> maxy)) + 0.5;
        (game -> worm_coordinates)[el.second] = {x, y};
        //jestem na zjedzonym pikselu
        if ((game -> eaten_pixels).count({(int)x, (int)y})) {
            player_eliminated(game, el.second);
        }
        else {
            (game -> eaten_pixels)[{(int)x, (int)y}] = true;
            int dir = rand_gen(game) % 360;
            (game -> worm_direction)[el.second] = dir;
            (game -> turn_direction)[el.second] = 0;
            pixel(game, el.second);
        }
    }
}

//usuwanie rozlaczonego gracza
void disconnect_player(game_s *game, int ind) {
    game -> disconnected.insert(ind);
    set<int>::iterator it;
    it = (game -> active_players).find(ind);
    if (it != (game -> active_players).end()) {
        (game -> active_players).erase(it);
    }
    it = (game -> not_active_players).find(ind);
    if (it != (game -> not_active_players).end()) {
        (game -> not_active_players).erase(it);
    }
    it = (game -> observers).find(ind);
    if (it != (game -> observers).end()) {
        (game -> observers).erase(it);
    }
    it = (game -> all_players).find(ind);
    if (it != (game -> all_players).end()) {
        (game -> all_players).erase(it);
    }
}

//przerobienie pierwszych 8 bajtow slowa
uint64_t get_8bytes(char *message) {
    uint64_t number = 0;
    for (size_t i = 0; i < 8; i++) {
        number = number * (1 << 8) + message[i];
    }

    char mes[strlen(message)];
    strcpy(mes, message + 8);
    strcpy(message, mes);

    return be64toh(number);
}

//przerobienie pierwszych 4 bajtow slowa
uint32_t get_4bytes(char *message) {
    uint32_t number = 0;
    for (size_t i = 0; i < 4; i++) {
        number = number * (1 << 8) + message[i];
    }

    char mes[strlen(message)];
    strcpy(mes, message + 4);
    strcpy(message, mes);

    return be32toh(number);
}

//przerobienie bajtu slowa
uint8_t get_byte(char *message) {
    uint8_t number = 0;
    for (size_t i = 0; i < 1; i++) {
        number = number * (1 << 8) + message[i];
    }

    char mes[strlen(message)];
    strcpy(mes, message + 1);
    strcpy(message, mes);

    return be32toh(number);
}

//dodanie nowego gracza
int new_player(game_s *game, string name, bool direct, string ip) {
    //sprawdzenie dostepnego numeru dla gracza
    set<int> s;
    for (size_t i = 0; i < 25; i++) {
        s.insert(i);
    }
    set<int>::iterator it;
    for (it = (game -> all_players).begin(); it != (game -> all_players).end(); it++) {
        s.erase(*it);
    }
    int number;
    if (s.empty()) {
        return -1;
    }
    else {
        number = *s.begin();
    }

    (game -> all_players).insert(number);

    //gracz nacisnal strzalke
    if (direct) {
        (game -> client_index)[ip] = number;
        (game -> player_name)[number] = name;
        (game -> playing_players)[name] = number;
        if (name == "") {
            (game -> observers).insert(number);
        }
        else {
            (game -> active_players).insert(number);
        }
    }
    //gracz nie nacisnal strzalki
    else {
        (game -> client_index)[ip] = number;
        (game -> player_name)[number] = name;
        if (name == "") {
            (game -> observers).insert(number);
        }
        else {
            (game -> not_active_players).insert(number);
        }
    }

    return number;
}

//ustawienie gracza z nieaktywnego na aktywnego
void not_active_to_active(game_s *game, string name, int num) {
    if (name == "") {
        return;
    }

    (game -> not_active_players).erase(num);
    (game -> active_players).insert(num);
}

//parsowanie i wysylanie komunikatow do klienta
void parse_message(game_s *game, char *message, sockaddr *recvfrom, int sock) {
    char ip_address[BUF_SIZE];
    string ip;
    if (recvfrom -> sa_family == AF_INET) {
        sockaddr_in ipv4 = *reinterpret_cast<sockaddr_in *>(recvfrom);
        inet_ntop(AF_INET, &(ipv4.sin_addr), ip_address, sizeof(ip_address));
        ip = string(ip_address);
    }
    else {
        sockaddr_in6 ipv6 = *reinterpret_cast<sockaddr_in6 *>(recvfrom);
        inet_ntop(AF_INET, &(ipv6.sin6_addr), ip_address, sizeof(ip_address));
        ip = string(ip_address);
    }

    int64_t session_id = get_8bytes(message);

    uint8_t turn_direction = get_byte(message);

    uint32_t next_expected_event_no = get_4bytes(message);
    
    string name(message);

    //sprawdzenie, czy dany gracz jest nowy
    int num = 0;
    bool is_already = false;
    set<int>::iterator it;
    for (it = (game -> all_players).begin(); it != (game -> all_players).end(); it++) {
        string s = (game -> player_name)[*it];
        if (s == name) {
            is_already = true;
            num = *it;
        }
    }

    if (!is_already) {
        (game -> session_id_map)[ip] = session_id;
    }
    else {
        if (session_id != (game -> session_id_map)[ip]) {
            return;
        }
    }

    //laczenie komunikatow UDP
    char *buffer = (char *)malloc(2 * sizeof(char));
    size_t buffer_size = 1;
    size_t cur_size = 0;
    for (auto el : game -> events) {
        if ((uint32_t)el.first >= next_expected_event_no) {
            vector<uint8_t> event = el.second;
            char ev[event.size() + 1];
            for (size_t j = 0; j < event.size(); j++) {
                ev[j] = event[j];
            }
            while (cur_size + strlen(ev) > buffer_size) {
                buffer_size *= 2;
                buffer = (char *)realloc(buffer, (buffer_size + 1) * sizeof(char));
            }
            cur_size += strlen(ev);
            strcat(buffer, ev);
        }
    }

    //wysylanie komunikatow UDP do klienta
    int beg = 0;
    int end = BUF_SIZE;
    while (beg != end) {
        //skracanie wiadomosci do wyslania
        char buffer2[end - beg + 1];
        memcpy(buffer2, buffer + beg, end - beg);
        buffer2[end - beg] = '\0';
        memcpy(buffer, buffer2, strlen(buffer2));
        buffer[strlen(buffer2)] = '\0';

        //przygotowywania prefiksu wiadomosci do wyslania
        size_t to_send_length = min(BUF_SIZE, (int)cur_size - beg);
        char to_send[to_send_length + 1];
        memcpy(to_send, buffer, to_send_length);
        to_send[to_send_length] = '\0';
        beg += to_send_length;
        //wysylanie komunikatu
        sendto(sock, buffer, strlen(buffer), 0, recvfrom, sizeof(*recvfrom));
    }

    free(buffer);

    //gracz nie nacisnal strzalki
    if (turn_direction == 0) {
        if (!is_already) {
            num = new_player(game, name, false, ip);
        }
    }
    //gracz nacisnal strzalke
    else {
        if (!is_already) {
            num = new_player(game, name, true, ip);
        }
        else {
            not_active_to_active(game, name, num);
        }
    }

    if (num == -1) {
        return;
    }

    //stworzenie timera dla klienta
    for (size_t i = 0; i < (game -> all_players).size(); i++) {
        struct itimerspec new_client_timer;

        (game -> client[i]).fd = timerfd_create(CLOCK_REALTIME, 0);
        new_client_timer.it_interval.tv_sec = 0;
        new_client_timer.it_interval.tv_nsec = 0;
        new_client_timer.it_value.tv_sec = PLAYER_TIMEOUT;
        new_client_timer.it_value.tv_nsec = 0;
        timerfd_settime((game -> client[i]).fd, 0, &(new_client_timer), NULL);
    }

    (game -> turn_direction)[num] = turn_direction;
}

int main(int argc, char *argv[]) {
    //parsowanie argumentow programu
    int opt;
    int port_num = 2021;
    uint32_t rand_gen = time(NULL);
    int turning_speed = 6;
    int rounds_per_sec = 50;
    int width = 640;
    int height = 480;
    while ((opt = getopt(argc, argv, "p:s:t:v:w:h:")) != -1) {
        switch(opt) {
            case 'p':
                if (!check_if_number(optarg)) {
                    fprintf(stderr, "bad propgram arguments");
                    exit(1);
                }
                port_num = atoi(optarg);
            break;
            case 's':
                if (!check_if_number(optarg)) {
                    fprintf(stderr, "bad propgram arguments");
                    exit(1);
                }
                rand_gen = atoi(optarg);
            break;
            case 't':
                if (!check_if_number(optarg)) {
                    fprintf(stderr, "bad propgram arguments");
                    exit(1);
                }
                turning_speed = atoi(optarg);
            break;
            case 'v':
                if (!check_if_number(optarg)) {
                    fprintf(stderr, "bad propgram arguments");
                    exit(1);
                }
                rounds_per_sec = atoi(optarg);
            break;
            case 'w':
                if (!check_if_number(optarg)) {
                    fprintf(stderr, "bad propgram arguments");
                    exit(1);
                }
                width = atoi(optarg);
            break;
            case 'h':
                if (!check_if_number(optarg)) {
                    fprintf(stderr, "bad propgram arguments");
                    exit(1);
                }
                height = atoi(optarg);
            break;
            default:
                fprintf(stderr, "bad propgram arguments");
                exit(1);
            break;
        }
    }

    //struktura gry
    game_s game {};
    game.game_id = 0;
    game.seed = rand_gen;
    game.seed_number = -1;
    game.game_started = false;
    game.turning_speed = turning_speed;
    game.maxx = width;
    game.maxy = height;
    game.active_players.clear();
    game.not_active_players.clear();
    game.all_players.clear();
    game.observers.clear();
    game.disconnected.clear();
    game.eaten_pixels.clear();
    game.client_index.clear();
    game.player_name.clear();
    game.worm_coordinates.clear();
    game.events.clear();
    game.playing_players.clear();
    game.turn_direction.clear();
    game.worm_direction.clear();

    //tworzenie socketu
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);

    //ustawienie czasu oczekiwania na recvfrom
    struct timeval tv {};
    tv.tv_sec = 0;
    tv.tv_usec = 10;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    //ustawienie flagi IPV6_V6ONLY
    int no = 0;
    setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no));

    //bindowanie socketu
    struct sockaddr_in6 server_address;
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(port_num);
    bind(sock, (struct sockaddr *)&server_address, (socklen_t)sizeof(server_address));

    //glowna petla
    while (true) {
        //rozpoczecie nowej gry
        if (!game.game_started && game.active_players.size() > 1 && game.not_active_players.size() == 0) {
            start_game(&game, rounds_per_sec);
        }

        //nastepna tura
        new_turn(&game);

        //zakonczenie gry
        if (game.game_started && game.active_players.size() < 2) {
            end_game(&game);
        }

        //usuwanie odlaczonych klientow
        int ret = poll(&game.client[0], 25, 0);
        if (ret > 0) {
            for (size_t i = 0; i < 25; i++) {
                if (game.client[i].revents & POLLIN) {
                    disconnect_player(&game, i);
                }
            }
        }

        //otrzymywanie wiadomosci od klienta
        char buf[BUF_SIZE + 1];
        struct sockaddr recv_from;
        int recv_from_len = sizeof(recv_from);
        recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr *)&recv_from, (socklen_t *)&recv_from_len);
        parse_message(&game, buf, &recv_from, sock);
    }
}