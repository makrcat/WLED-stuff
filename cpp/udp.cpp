#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

struct LED {
    unsigned char r, g, b;
    LED() : r(0), g(0), b(0) {}      
};

int main() {
    const int NUM_LEDS = 20;
    LED leds[NUM_LEDS];
    unsigned char frame[NUM_LEDS * 3];

    const char* WLED_IP = "192.168.1.100"; 
    const int WLED_PORT = 21324;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "Failed to create socket\n";
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(WLED_PORT);
    inet_pton(AF_INET, WLED_IP, &addr.sin_addr);

    for (int i = 0; i < NUM_LEDS; i++) {

        if (i != 0) {
            frame[3 * i - 3] = 0;
            frame[3 * i - 2] = 0;
            frame[3 * i - 1] = 0;
        }

        frame[3 * i] = leds[i].r;
        frame[3 * i + 1] = leds[i].g;
        frame[3 * i + 2] = leds[i].b;

        sendto(sock, frame, sizeof(frame), 0, (struct sockaddr*)&addr, sizeof(addr));

        this_thread::sleep_for(chrono::milliseconds(100));
    }
    close(sock);
}
